#include <chrono>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <sensorring/SensorRingFactory.hpp>
#include <sensorring/firmware/FirmwareUpdater.hpp>
#include <sensorring/interface/InterfaceParams.hpp>
#include <string>
#include <thread>

using namespace eduart::sensorring;

namespace {

// std::vector can't be constexpr so using std::array instead
static constexpr std::array<std::string_view, 3> KNOWN_SOCKETCAN_INTERFACES = { "can0", "eduart-can0", "eduart-can1" };
static constexpr std::array<std::string_view, 1> KNOWN_USBTINGO_INTERFACES  = { "0x0" };

void printUsage(const char* executable) {
  std::cerr << "Usage: " << executable << " -t <socketcan|usbtingo> -i <interface-name> -r\n"
            << " Arguments:\n"
            << " -t <socketcan|usbtingo> interface type \n"
            << " -i <interface-name> interface name (e.g. can0, can1, 0x0)\n"
            << " -r reset the devices before discovery\n"
            << " Without a specified interface, the tool will test the following known interfaces:\n"
            << "  socketcan can0\n"
            << "  socketcan eduart-can0\n"
            << "  socketcan eduart-can1\n"
            << "  usbtingo 0x0 (auto discovery)\n";
}

bool parseInterfaceType(const std::string& input, com::InterfaceType& type) {
  if (input == "socketcan") {
    type = com::InterfaceType::SocketCan;
    return true;
  }

  if (input == "usbtingo") {
    type = com::InterfaceType::UsbTingo;
    return true;
  }

  return false;
}

} // namespace

int main(int argc, char* argv[]) {
  try {

    std::optional<std::string> interface_type_str;
    std::optional<std::string> interface_name;
    bool reset_devices = false;

    for (int i = 1; i < argc; i += 2) {

      const std::string arg = argv[i];

      // Parameters without value
      if (arg == "-r") {
        reset_devices = true;
        i -= 1; // -r does not have a value, so adjust the index
        continue;
      }

      if (i + 1 >= argc) {
        std::cerr << "Missing value for argument: " << argv[i] << '\n';
        printUsage(argv[0]);
        return EXIT_FAILURE;
      }

      // Parameters with value
      const std::string value = argv[i + 1];
      if (arg == "-t") {
        interface_type_str = value;
      } else if (arg == "-i") {
        interface_name = value;
      } else {
        std::cerr << "Unknown argument: " << arg << '\n';
        printUsage(argv[0]);
        return EXIT_FAILURE;
      }
    }

    if (!interface_type_str.has_value() != !interface_name.has_value()) {
      std::cerr << "Missing required interface arguments. -t and -i must both be specified.\n";
      printUsage(argv[0]);
      return EXIT_FAILURE;
    }

    // Create the SensorRing via auto-discovery
    SensorRingFactory factory;

    com::InterfaceType interface_type;
    if (interface_type_str.has_value()) {
      if (!parseInterfaceType(interface_type_str.value(), interface_type)) {
        std::cerr << "Unsupported interface type: " << interface_type_str.value() << '\n';
        printUsage(argv[0]);
        return EXIT_FAILURE;
      }

      switch (interface_type) {
      case com::InterfaceType::SocketCan:
        factory.addInterface(com::SocketCanParams{ std::string(interface_name.value()) });
        break;
      case com::InterfaceType::UsbTingo:
        factory.addInterface(com::UsbTingoParams{ std::string(interface_name.value()) });
        break;
      default:
        std::cerr << "Unsupported interface type: " << interface_type_str.value() << '\n';
        return EXIT_FAILURE;
      }
    } else {
      for (const auto& name : KNOWN_SOCKETCAN_INTERFACES) {
        com::SocketCanParams can_interface{ std::string(name) };
        factory.addInterface(can_interface);
      }

      for (const auto& name : KNOWN_USBTINGO_INTERFACES) {
        com::UsbTingoParams usbtingo_interface{ std::string(name) };
        factory.addInterface(usbtingo_interface);
      }
    }

    if (reset_devices) {
      std::cout << "Resetting devices..." << std::endl;
      factory.enumerate(); // Have to enumerate first to ensure the interfaces are initialized before sending reset commands
      board::SensorBoard::resetBoards();
      std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    factory.enumerate();
    std::cout << factory.printTopology() << std::endl;

  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
