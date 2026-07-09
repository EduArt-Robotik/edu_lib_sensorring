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

enum class Mode {
  EnterBootloader,
  FlashDetectedBootloader,
  EnterBootloaderAndFlash,
  AutoAll,
};

void printUsage(const char* executable) {
  std::cerr << "Usage: " << executable << " -m <enter|flash|enter-flash|auto-all> -t <socketcan|usbtingo> -i <interface-name> [-n <board-index>] [-f <firmware.hex>]\n"
            << "Modes:\n"
            << "  enter        : switch one board to bootloader mode (requires -n)\n"
            << "  flash        : detect one board already in bootloader mode and flash it (requires -f)\n"
            << "  enter-flash  : switch one board to bootloader mode and flash it directly (requires -n and -f)\n"
            << "  auto         : automatically update all boards on one interface (requires -f)\n"
            << "Examples:\n"
            << "  " << executable << " -m enter -t socketcan -i can0 -n 0\n"
            << "  " << executable << " -m flash -t socketcan -i can0 -f ./firmware.hex\n"
            << "  " << executable << " -m enter-flash -t socketcan -i can0 -n 1 -f ./firmware.hex\n"
            << "  " << executable << " -m auto -t socketcan -i can0 -f ./firmware.hex\n";
}

bool parseMode(const std::string& input, Mode& mode) {
  if (input == "enter") {
    mode = Mode::EnterBootloader;
    return true;
  }

  if (input == "flash") {
    mode = Mode::FlashDetectedBootloader;
    return true;
  }

  if (input == "enter-flash") {
    mode = Mode::EnterBootloaderAndFlash;
    return true;
  }

  if (input == "auto") {
    mode = Mode::AutoAll;
    return true;
  }

  return false;
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
    std::string mode_str;
    std::string interface_type_str;
    std::string interface_name;
    std::string firmware_path;
    std::optional<unsigned int> board_index;

    for (int i = 1; i < argc; i += 2) {
      if (i + 1 >= argc) {
        std::cerr << "Missing value for argument: " << argv[i] << '\n';
        printUsage(argv[0]);
        return EXIT_FAILURE;
      }

      const std::string arg   = argv[i];
      const std::string value = argv[i + 1];

      if (arg == "-m") {
        mode_str = value;
      } else if (arg == "-t") {
        interface_type_str = value;
      } else if (arg == "-i") {
        interface_name = value;
      } else if (arg == "-n") {
        try {
          board_index = static_cast<unsigned int>(std::stoul(value));
        } catch (const std::exception&) {
          std::cerr << "Invalid board index: " << value << '\n';
          printUsage(argv[0]);
          return EXIT_FAILURE;
        }
      } else if (arg == "-f") {
        firmware_path = value;
      } else {
        std::cerr << "Unknown argument: " << arg << '\n';
        printUsage(argv[0]);
        return EXIT_FAILURE;
      }
    }

    Mode mode = Mode::EnterBootloader;
    if (!parseMode(mode_str, mode)) {
      std::cerr << "Unsupported mode: " << mode_str << '\n';
      printUsage(argv[0]);
      return EXIT_FAILURE;
    }

    if (interface_type_str.empty() || interface_name.empty()) {
      std::cerr << "Missing required interface arguments (-t and -i).\n";
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

    firmware_update::FirmwareUpdater updater;
    const auto log_callback = [](const std::string& msg) {
      std::cout << msg << '\n';
    };

    if (mode == Mode::EnterBootloader) {
      if (!board_index.has_value()) {
        std::cerr << "Mode 'enter' requires -n <board-index>.\n";
        printUsage(argv[0]);
        return EXIT_FAILURE;
      }
      return updater.enterSingleBoardBootloader(interface, *board_index, log_callback) ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    if (mode == Mode::FlashDetectedBootloader) {
      if (firmware_path.empty()) {
        std::cerr << "Mode 'flash' requires -f <firmware.hex>.\n";
        printUsage(argv[0]);
        return EXIT_FAILURE;
      }

      if (interface.type != com::InterfaceType::SocketCan) {
        std::cerr << "Mode 'flash' currently supports only socketcan interfaces.\n";
        return EXIT_FAILURE;
      }

      const auto node_id = updater.detectBootloaderNodeWithRetries(interface);
      if (!node_id.has_value()) {
        std::cerr << "No board detected in bootloader mode on " << interface.type << " " << interface.name << ".\n";
        return EXIT_FAILURE;
      }

      std::cout << "Detected bootloader node " << static_cast<unsigned int>(*node_id) << ". Flashing...\n";
      return updater.flashSingleBoard(interface, *node_id, firmware_path, log_callback) ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    if (mode == Mode::EnterBootloaderAndFlash) {
      if (!board_index.has_value() || firmware_path.empty()) {
        std::cerr << "Mode 'enter-flash' requires both -n <board-index> and -f <firmware.hex>.\n";
        printUsage(argv[0]);
        return EXIT_FAILURE;
      }

      if (interface.type != com::InterfaceType::SocketCan) {
        std::cerr << "Mode 'enter-flash' currently supports only socketcan interfaces.\n";
        return EXIT_FAILURE;
      }

      if (!updater.enterSingleBoardBootloader(interface, *board_index, log_callback)) {
        return EXIT_FAILURE;
      }

      const auto node_id = updater.detectBootloaderNodeWithRetries(interface);
      if (!node_id.has_value()) {
        std::cerr << "Board entered bootloader mode, but no bootloader node could be detected.\n";
        return EXIT_FAILURE;
      }

      std::cout << "Detected bootloader node " << static_cast<unsigned int>(*node_id) << ". Flashing...\n";
      return updater.flashSingleBoard(interface, *node_id, firmware_path, log_callback) ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    if (mode == Mode::AutoAll) {
      if (firmware_path.empty()) {
        std::cerr << "Mode 'auto' requires -f <firmware.hex>.\n";
        printUsage(argv[0]);
        return EXIT_FAILURE;
      }

      std::cout << "Running automatic sequential flash on interface " << interface.type << " " << interface.name << ".\n";
      std::cout << "Firmware file: " << firmware_path << '\n';
      return updater.flashAllBoardsSequential(interface, firmware_path, log_callback) ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    std::cerr << "Unsupported mode.\n";
    return EXIT_FAILURE;
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_FAILURE;
}
