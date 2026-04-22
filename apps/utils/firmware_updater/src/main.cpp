#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include <sensorring/SensorRingFactory.hpp>

using namespace eduart::sensorring;

namespace {

void printUsage(const char* executable) {
  std::cerr << "Usage: " << executable << " <socketcan|usbtingo> <interface-name>\n"
            << "Example:\n"
            << "  " << executable << " socketcan can0\n"
            << "  " << executable << " usbtingo 0\n";
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

bool tryEnterBootloaderOnFirstBoard(const com::ComInterfaceID& interface) {
  ring::SensorRingFactory factory(ring::ValidationMode::Relaxed);
  factory.addInterface(interface);

  const auto enumeration = factory.enumerate();
  std::size_t board_count = 0;
  for (const auto& [iface, boards] : enumeration) {
    (void)iface;
    board_count += boards.size();
  }

  std::cout << "Discovered " << board_count << " board(s) on " << interface.type << " " << interface.name << ".\n";
  if (board_count == 0) {
    return false;
  }

  auto ring = factory.build();
  if (!ring) {
    std::cerr << "Failed to build SensorRing from discovered boards on " << interface.type << " " << interface.name << ".\n";
    return false;
  }

  const auto buses = ring->getSensorBuses();
  if (buses.empty()) {
    std::cerr << "No sensor bus available after build on " << interface.type << " " << interface.name << ".\n";
    return false;
  }

  const auto boards = buses.front()->getSensorBoards();
  if (boards.empty()) {
    std::cerr << "No sensor board available after build on " << interface.type << " " << interface.name << ".\n";
    return false;
  }

  const bool ok = boards.front()->enterBootloader();
  if (!ok) {
    std::cerr << "Failed to enter bootloader mode on first board on " << interface.type << " " << interface.name << ".\n";
    return false;
  }

  std::cout << "First board switched to bootloader mode on " << interface.type << " " << interface.name << ".\n";
  return true;
}

} // namespace

int main(int argc, char* argv[]) {
  if (!(argc == 1 || argc == 3)) {
    printUsage(argv[0]);
    return EXIT_FAILURE;
  }

  try {
    std::vector<com::ComInterfaceID> interfaces_to_try;

    if (argc == 1) {
      interfaces_to_try.push_back(com::ComInterfaceID{ com::InterfaceType::UsbTingo, "0" });
      interfaces_to_try.push_back(com::ComInterfaceID{ com::InterfaceType::SocketCan, "can0" });
      std::cout << "No interface argument provided. Trying usbtingo 0, then socketcan can0.\n";
    } else {
      com::ComInterfaceID interface;
      if (!parseInterfaceType(argv[1], interface.type)) {
        std::cerr << "Unsupported interface type: " << argv[1] << '\n';
        printUsage(argv[0]);
        return EXIT_FAILURE;
      }
      interface.name = argv[2];
      interfaces_to_try.push_back(interface);
    }

    for (const auto& interface : interfaces_to_try) {
      if (tryEnterBootloaderOnFirstBoard(interface)) {
        return EXIT_SUCCESS;
      }
    }

    std::cerr << "No suitable board found to enter bootloader mode.\n";
    return EXIT_FAILURE;
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_FAILURE;
}
