#include "Ethernet2Can.hpp"

#include <cerrno>
#include <array>
#include <chrono>
#include <cstring>
#include <exception>
#include <sensorring_transport/Protocol.hpp>
#include <sensorring_transport/can/CanCodec.hpp>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include "Ethernet2CanFrame.hpp"
#include "sensorring/logger/Logger.hpp"

namespace eduart {
namespace sensorring {
namespace com {

namespace {
constexpr auto LISTENER_TIMEOUT = std::chrono::milliseconds(10);

#ifdef _WIN32
using NativeSocket = SOCKET;
constexpr NativeSocket INVALID_SOCKET_HANDLE = INVALID_SOCKET;
int socketError() { return WSAGetLastError(); }
void closeSocket(NativeSocket socket) { closesocket(socket); }
bool interruptedSocketError(int error) { return error == WSAEINTR; }

bool initializeSockets() {
  static const bool initialized = []() {
    WSADATA data{};
    return WSAStartup(MAKEWORD(2, 2), &data) == 0;
  }();
  return initialized;
}
#else
using NativeSocket = int;
constexpr NativeSocket INVALID_SOCKET_HANDLE = -1;
int socketError() { return errno; }
void closeSocket(NativeSocket socket) { close(socket); }
bool interruptedSocketError(int error) { return error == EINTR; }
bool initializeSockets() { return true; }
#endif

NativeSocket nativeSocket(std::intptr_t socket) {
  return static_cast<NativeSocket>(socket);
}
} // namespace

Ethernet2Can::Ethernet2Can(const Ethernet2CanParams& params)
    : CanInterface({ InterfaceType::Ethernet2Can, params.interfaceName() }, params)
    , _ethernet_params(params)
    , _socket(static_cast<std::intptr_t>(INVALID_SOCKET_HANDLE))
    , _assembler(transport::can::CanCodec::MAX_PAYLOAD_PER_FRAME)
    , _reassembler([this](const transport::TransportFrame& frame) {
      ComEndpoint endpoint{ static_cast<Direction>(frame.direction), frame.boardAddress, frame.deviceId };
      dispatchMessage(endpoint, frame.command, frame.data);
    }) {
  if (!openInterface()) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Unable to open Ethernet CAN interface " + _id.name + ".");
    return;
  }
  startListener();
}

Ethernet2Can::~Ethernet2Can() {
  stopListener();
  closeInterface();
}

bool Ethernet2Can::openInterface() {
  if (_socket != static_cast<std::intptr_t>(INVALID_SOCKET_HANDLE)) {
    return true;
  }
  if (_ethernet_params.host.empty() || _ethernet_params.port == 0 || !initializeSockets()) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Invalid Ethernet CAN gateway configuration for " + _id.name + ".");
    return false;
  }

  addrinfo hints{};
  hints.ai_family   = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  addrinfo* addresses = nullptr;
  const std::string service = std::to_string(_ethernet_params.port);
  const int resolve_result  = getaddrinfo(_ethernet_params.host.c_str(), service.c_str(), &hints, &addresses);
  if (resolve_result != 0 || !addresses) {
#ifdef _WIN32
    const std::string reason = std::to_string(resolve_result);
#else
    const std::string reason = gai_strerror(resolve_result);
#endif
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Unable to resolve Ethernet2CAN gateway " + _ethernet_params.host + ": " + reason);
    if (addresses) {
      freeaddrinfo(addresses);
    }
    return false;
  }

  const NativeSocket socket_handle = ::socket(addresses->ai_family, addresses->ai_socktype, addresses->ai_protocol);
  if (socket_handle == INVALID_SOCKET_HANDLE) {
    freeaddrinfo(addresses);
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Unable to create UDP socket for Ethernet2CAN interface " + _id.name + ".");
    return false;
  }

  sockaddr_in local_address{};
  local_address.sin_family      = AF_INET;
  local_address.sin_addr.s_addr = htonl(INADDR_ANY);
  local_address.sin_port        = htons(_ethernet_params.local_port);
  if (bind(socket_handle, reinterpret_cast<const sockaddr*>(&local_address), sizeof(local_address)) != 0
      || connect(socket_handle, addresses->ai_addr, static_cast<int>(addresses->ai_addrlen)) != 0) {
    const int error = socketError();
    closeSocket(socket_handle);
    freeaddrinfo(addresses);
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "Unable to bind/connect UDP socket for Ethernet CAN interface " + _id.name + " (socket error " + std::to_string(error) + ").");
    return false;
  }

  freeaddrinfo(addresses);
  _socket               = static_cast<std::intptr_t>(socket_handle);
  _communication_error = false;
  return true;
}

bool Ethernet2Can::send(ComEndpoint target, std::uint8_t command, const std::vector<std::uint8_t>& data) {
  const auto frames = _assembler.assemble(
      static_cast<transport::Direction>(target.direction), target.boardAddress, target.deviceId, command, data);
  for (const auto& frame : frames) {
    const auto can_frame = transport::can::CanCodec::encode(frame);
    if (!sendCanFrame(can_frame.id, can_frame.data)) {
      _communication_error = true;
      return false;
    }
  }
  _communication_error = false;
  return true;
}

bool Ethernet2Can::sendCanFrame(std::uint32_t can_id, const std::vector<std::uint8_t>& data) {
  const auto datagram = ethernet::encodeDatagram(_ethernet_params.can_bus_index, can_id, data);
  if (datagram.empty() || _socket == static_cast<std::intptr_t>(INVALID_SOCKET_HANDLE)) {
    return false;
  }

  LockGuard guard(_mutex);
  const auto sent = ::send(nativeSocket(_socket), reinterpret_cast<const char*>(datagram.data()), static_cast<int>(datagram.size()), 0);
  if (sent < 0 || static_cast<std::size_t>(sent) != datagram.size()) {
    const int error = socketError();
    logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "UDP send failed on Ethernet CAN interface " + _id.name + " (socket error " + std::to_string(error) + ").");
    return false;
  }
  return true;
}

bool Ethernet2Can::listener() {
  if (_socket == static_cast<std::intptr_t>(INVALID_SOCKET_HANDLE)) {
    return false;
  }

  _shut_down_listener  = false;
  _listener_is_running = true;
  logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Starting Ethernet CAN listener on interface " + _id.name);

  const NativeSocket socket_handle = nativeSocket(_socket);
  std::array<std::uint8_t, 2048> buffer{};
  while (!_shut_down_listener) {
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(socket_handle, &read_set);
    timeval timeout{};
    timeout.tv_sec  = static_cast<long>(LISTENER_TIMEOUT.count() / 1000);
    timeout.tv_usec = static_cast<long>((LISTENER_TIMEOUT.count() % 1000) * 1000);
#ifdef _WIN32
    const int selected = select(0, &read_set, nullptr, nullptr, &timeout);
#else
    const int selected = select(socket_handle + 1, &read_set, nullptr, nullptr, &timeout);
#endif
    if (selected == 0) {
      continue;
    }
    if (selected < 0) {
      const int error = socketError();
      if (interruptedSocketError(error)) {
        continue;
      }
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "UDP select failed on Ethernet CAN interface " + _id.name + " (socket error " + std::to_string(error) + ").");
      break;
    }

    const auto received = ::recv(socket_handle, reinterpret_cast<char*>(buffer.data()), static_cast<int>(buffer.size()), 0);
    if (received < 0) {
      const int error = socketError();
      if (interruptedSocketError(error)) {
        continue;
      }
      logger::Logger::getInstance()->log(logger::LogVerbosity::Error, "UDP receive failed on Ethernet CAN interface " + _id.name + " (socket error " + std::to_string(error) + ").");
      break;
    }

    std::uint32_t can_id = 0;
    std::vector<std::uint8_t> can_data;
    if (!ethernet::decodeDatagram(buffer.data(), static_cast<std::size_t>(received), _ethernet_params.can_bus_index, can_id, can_data)) {
      continue;
    }

    try {
      const auto frame = transport::can::CanCodec::decode(can_id, can_data.data(), can_data.size());
      _reassembler.processFrame(frame);
    } catch (const std::exception& error) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, "Ignoring invalid CAN frame on Ethernet CAN interface " + _id.name + ": " + error.what());
    }
  }

  _listener_is_running = false;
  logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "Stopping Ethernet CAN listener on interface " + _id.name);
  return true;
}

bool Ethernet2Can::closeInterface() {
  if (_socket == static_cast<std::intptr_t>(INVALID_SOCKET_HANDLE)) {
    return true;
  }
  const NativeSocket socket_handle = nativeSocket(_socket);
  _socket = static_cast<std::intptr_t>(INVALID_SOCKET_HANDLE);
  shutdown(socket_handle, 2);
  closeSocket(socket_handle);
  return true;
}

bool Ethernet2Can::repairInterface() {
  stopListener();
  closeInterface();
  if (!openInterface()) {
    return false;
  }
  return startListener();
}

} // namespace com
} // namespace sensorring
} // namespace eduart
