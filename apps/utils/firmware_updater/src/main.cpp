#include <chrono>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <sensorring/SensorRingFactory.hpp>
#include <sensorring/firmware/FirmwareUpdater.hpp>
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
            << "  enter        : switch one app board to bootloader mode (requires -n)\n"
            << "  flash        : detect one board already in bootloader mode and flash it (requires -f)\n"
            << "  enter-flash  : switch one app board to bootloader mode and flash it directly (requires -n and -f)\n"
            << "  auto-all     : automatically update all boards on one interface (requires -f)\n"
            << "Examples:\n"
            << "  " << executable << " -m enter -t socketcan -i can0 -n 0\n"
            << "  " << executable << " -m flash -t socketcan -i can0 -f ./firmware.hex\n"
            << "  " << executable << " -m enter-flash -t socketcan -i can0 -n 1 -f ./firmware.hex\n"
            << "  " << executable << " -m auto-all -t socketcan -i can0 -f ./firmware.hex\n";
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

  if (input == "auto-all") {
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

bool enterBootloaderOnBoard(const firmware_update::FirmwareUpdater& updater, const com::ComInterfaceID& interface, unsigned int board_index) {
  SensorRingFactory factory(ValidationMode::Relaxed);
  factory.addInterface(interface);

  const auto enumeration  = factory.enumerate();
  std::size_t board_count = 0U;
  for (const auto& [iface, boards] : enumeration) {
    (void)iface;
    board_count += boards.size();
  }

  if (board_index >= board_count) {
    std::cerr << "Requested board index " << board_index << " is out of range for current board set.\n";
    return false;
  }

  std::cout << "Discovered " << board_count << " board(s) on " << interface.type << " " << interface.name << ".\n";
  if (board_count == 0U) {
    return false;
  }

  const bool ok = updater.enterSingleBoardBootloader(interface, board_index);
  if (!ok) {
    std::cerr << "Failed to enter bootloader mode on board " << board_index << " on " << interface.type << " " << interface.name << ".\n";
    return false;
  }

  std::cout << "Board " << board_index << " switched to bootloader mode on " << interface.type << " " << interface.name << ".\n";
  return true;
}

std::optional<std::uint8_t> detectBootloaderNodeWithRetries(const firmware_update::FirmwareUpdater& updater, const com::ComInterfaceID& interface, unsigned int retries, std::chrono::milliseconds retry_delay) {
  for (unsigned int attempt = 0; attempt < retries; ++attempt) {
    const auto node_id = updater.detectBootloaderNode(interface);
    if (node_id.has_value()) {
      return node_id;
    }
    std::this_thread::sleep_for(retry_delay);
  }

  return std::nullopt;
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

    constexpr unsigned int bootloader_detect_retries = 10U;
    const auto bootloader_detect_retry_delay         = std::chrono::milliseconds(200);
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
      return enterBootloaderOnBoard(updater, interface, *board_index) ? EXIT_SUCCESS : EXIT_FAILURE;
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

      const auto node_id = detectBootloaderNodeWithRetries(updater, interface, bootloader_detect_retries, bootloader_detect_retry_delay);
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

      if (!enterBootloaderOnBoard(updater, interface, *board_index)) {
        return EXIT_FAILURE;
      }

      const auto node_id = detectBootloaderNodeWithRetries(updater, interface, bootloader_detect_retries, bootloader_detect_retry_delay);
      if (!node_id.has_value()) {
        std::cerr << "Board entered bootloader mode, but no bootloader node could be detected.\n";
        return EXIT_FAILURE;
      }

      std::cout << "Detected bootloader node " << static_cast<unsigned int>(*node_id) << ". Flashing...\n";
      return updater.flashSingleBoard(interface, *node_id, firmware_path, log_callback) ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    if (mode == Mode::AutoAll) {
      if (firmware_path.empty()) {
        std::cerr << "Mode 'auto-all' requires -f <firmware.hex>.\n";
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
