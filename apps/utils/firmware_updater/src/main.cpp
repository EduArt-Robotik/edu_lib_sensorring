#include <cstdlib>
#include <iostream>
#include <sensorring/firmware_update/FirmwareUpdater.hpp>
#include <string>
#include <vector>

using namespace eduart::sensorring;

namespace {

void printUsage(const char* executable) {
  std::cerr << "Usage: " << executable << " <socketcan|usbtingo> <interface-name> <firmware.hex>\n"
            << "Example:\n"
            << "  " << executable << " socketcan can0 ./firmware.hex\n";
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
  if (!(argc == 1 || argc == 4)) {
    printUsage(argv[0]);
    return EXIT_FAILURE;
  }

  try {
    std::vector<com::ComInterfaceID> interfaces_to_try;
    std::string hex_file_path;

    if (argc == 1) {
      interfaces_to_try.push_back(com::ComInterfaceID{ com::InterfaceType::UsbTingo, "0" });
      interfaces_to_try.push_back(com::ComInterfaceID{ com::InterfaceType::SocketCan, "can0" });
      std::cerr << "No firmware file argument provided.\n";
      printUsage(argv[0]);
      return EXIT_FAILURE;
    } else {
      com::ComInterfaceID interface;
      if (!parseInterfaceType(argv[1], interface.type)) {
        std::cerr << "Unsupported interface type: " << argv[1] << '\n';
        printUsage(argv[0]);
        return EXIT_FAILURE;
      }
      interface.name = argv[2];
      hex_file_path  = argv[3];
      interfaces_to_try.push_back(interface);
    }

    std::cout << "Discovered " << interfaces_to_try.size() << " candidate interface(s).\n";
    std::cout << "Firmware file: " << hex_file_path << '\n';

    firmware_update::FirmwareUpdater updater;
    auto log_callback = [](const std::string& msg) {
      std::cout << msg << '\n';
    };

    for (std::size_t i = 0; i < interfaces_to_try.size(); ++i) {
      const auto& interface = interfaces_to_try[i];
      std::cout << "[" << (i + 1) << "/" << interfaces_to_try.size() << "] Trying interface " << interface.type << " " << interface.name << "...\n";
      if (updater.flashAllBoardsSequential(interface, hex_file_path, log_callback)) {
        return EXIT_SUCCESS;
      }
      std::cout << "Interface " << interface.type << " " << interface.name << " failed.\n";
    }

    std::cerr << "Firmware update failed on all candidate interfaces.\n";
    return EXIT_FAILURE;
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_FAILURE;
}
