#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include <sensorring/SensorRingFactory.hpp>

using namespace eduart::sensorring;

namespace {

void printUsage(const char* executable) {
  std::cerr << "Usage: " << executable << " -t <socketcan|usbtingo> -i <interface-name> -n <board-index>\n"
            << "Example:\n"
            << "  " << executable << " -t socketcan -i can0 -n 0\n"
            << "  " << executable << " -t usbtingo -i 0 -n 1\n";
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

bool tryEnterBootloaderOnBoard(const com::ComInterfaceID& interface, unsigned int board_index) {
  ring::SensorRingFactory factory(ring::ValidationMode::Relaxed);
  factory.addInterface(interface);

  const auto enumeration = factory.enumerate();
  std::size_t board_count = 0;

  for (const auto& [iface, boards] : enumeration) {
    (void)iface;
    board_count += boards.size();
  }

  if (board_index >= board_count) {
    std::cerr << "Requested board index " << board_index << " is out of range for current board set.\n";
    return false;
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

  const bool ok = boards.at(board_index)->enterBootloader();
  if (!ok) {
    std::cerr << "Failed to enter bootloader mode on board " << board_index << " on " << interface.type << " " << interface.name << ".\n";
    return false;
  }

  std::cout << "Board " << board_index << " switched to bootloader mode on " << interface.type << " " << interface.name << ".\n";
  return true;
}

} // namespace

int main(int argc, char* argv[]) {
  try {
    std::vector<com::ComInterfaceID> interfaces_to_try;
    unsigned int board_index = 0;

    if (argc == 1) {
      interfaces_to_try.push_back(com::ComInterfaceID{ com::InterfaceType::UsbTingo, "0" });
      interfaces_to_try.push_back(com::ComInterfaceID{ com::InterfaceType::SocketCan, "can0" });
      std::cout << "No interface arguments provided. Trying usbtingo 0, then socketcan can0 (board index 0).\n";
    } else {
      std::string interface_type_str;
      std::string interface_name;
      bool has_type = false;
      bool has_name = false;
      bool has_board_index = false;

      for (int i = 1; i < argc; i += 2) {
        if (i + 1 >= argc) {
          std::cerr << "Missing value for argument: " << argv[i] << '\n';
          printUsage(argv[0]);
          return EXIT_FAILURE;
        }

        const std::string arg = argv[i];
        const std::string value = argv[i + 1];

        if (arg == "-t") {
          interface_type_str = value;
          has_type = true;
        } else if (arg == "-i") {
          interface_name = value;
          has_name = true;
        } else if (arg == "-n") {
          try {
            board_index = static_cast<unsigned int>(std::stoul(value));
          } catch (const std::exception&) {
            std::cerr << "Invalid board index: " << value << '\n';
            printUsage(argv[0]);
            return EXIT_FAILURE;
          }
          has_board_index = true;
        } else {
          std::cerr << "Unknown argument: " << arg << '\n';
          printUsage(argv[0]);
          return EXIT_FAILURE;
        }
      }

      if (!has_type || !has_name || !has_board_index) {
        std::cerr << "Missing required arguments.\n";
        printUsage(argv[0]);
        return EXIT_FAILURE;
      }

      com::ComInterfaceID interface;
      if (!parseInterfaceType(interface_type_str, interface.type)) {
        std::cerr << "Unsupported interface type: " << interface_type_str << '\n';
        printUsage(argv[0]);
        return EXIT_FAILURE;
      }
      interface.name = interface_name;
      interfaces_to_try.push_back(interface);
    }

    for (const auto& interface : interfaces_to_try) {
      if (tryEnterBootloaderOnBoard(interface, board_index)) {
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
