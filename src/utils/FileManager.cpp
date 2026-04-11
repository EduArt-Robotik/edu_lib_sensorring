#include "utils/FileManager.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>

#include "device/hardware/htpa32/HTPA32_Eeprom.hpp"
#include "platform/Platform.hpp"
#include "sensorring/logger/Logger.hpp"
#include "sensorring/measurement/Image.hpp"

namespace eduart {

namespace sensorring {

namespace filemanager {

//==================================================
// concrete types that are used in the program
//==================================================

template class filemanager::ArrayHandler<double, THERMAL_RESOLUTION>;
template class filemanager::StructHandler<device::htpa32::HTPA32_Eeprom>;

//==================================================
// PathHandler
//==================================================

bool PathHandler::checkDirectory(std::string path) {

  return checkDirectory(std::filesystem::path(path));
}

bool PathHandler::checkDirectory(std::filesystem::path path) {
  if (!path.empty()) {
    if (!std::filesystem::exists(path)) {
      logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, std::string("Directory " + path.u8string() + " doesn't exist, creating the directory now."));
      if (!std::filesystem::create_directories(path)) {
        if (!std::filesystem::exists(path)) {
          logger::Logger::getInstance()->log(logger::LogVerbosity::Warning, std::string("Creating directory " + path.u8string() + " failed!"));
          return false;
        }
      }
    }
  } else {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "File Manager: No directory specified, skip operation.");
    return false;
  }

  return true;
}

std::filesystem::path PathHandler::resolvePath(const std::string path) {
  std::filesystem::path resolved_path;

  // absolute path -> use it directly
  if (std::filesystem::path(path).is_absolute()) {
    resolved_path = std::filesystem::path(path);
  } else {

    // relative path -> resolve it against home directory
    auto home = getEnvVar("HOME");
    if (!home.empty()) {
      // unix
      std::filesystem::path home_dir(home);
      resolved_path = home_dir / path;
    } else {
      // windows
      auto user_profile = getEnvVar("USERPROFILE");
      if (!user_profile.empty()) {
        std::filesystem::path user_profile_dir(user_profile);
        resolved_path = user_profile_dir / path;
      } else {
        // use relative path
        resolved_path = std::filesystem::current_path() / path;
      }
    }
  }

  return resolved_path;
}

//==================================================
// VectorHandler
//==================================================

template <typename T> bool VectorHandler<T>::saveVectorToFile(const std::string filename, const std::vector<T>& vec) {
  return saveVectorToFile("", filename, vec);
}

template <typename T> bool VectorHandler<T>::saveVectorToFile(const std::string filepath, const std::string filename, const std::vector<T>& vec) {
  if (vec->empty())
    return false;
  if (!PathHandler::checkDirectory(filepath))
    return false;

  std::filesystem::path full_path = PathHandler::resolvePath(filepath) / filename;
  std::ofstream file(full_path, std::ios::trunc);

  if (!file)
    return false;

  for (auto element : *vec) {
    file << element << " ";
  }

  file.close();

  return true;
}

template <typename T> bool VectorHandler<T>::readVectorFromFile(const std::string filename, std::vector<T>& vec) {
  return readVectorFromFile("", filename, vec);
}

template <typename T> bool VectorHandler<T>::readVectorFromFile(const std::string filepath, const std::string filename, std::vector<T>& vec) {

  vec.erase();
  std::filesystem::path full_path = PathHandler::resolvePath(filepath) / filename;
  std::ifstream file(full_path);

  if (!file)
    return false;

  T value;
  while (file >> value) {
    vec.push_back(value);
  }

  if (vec.empty())
    return false;

  file.close();
  return true;
}

//==================================================
// ArrayHandler
//==================================================

template <typename T, std::size_t l> bool ArrayHandler<T, l>::saveArrayToFile(const std::string filename, const std::array<T, l>& arr) {
  return saveArrayToFile("", filename, arr);
}

template <typename T, std::size_t l> bool ArrayHandler<T, l>::saveArrayToFile(const std::string filepath, const std::string filename, const std::array<T, l>& arr) {
  if (arr.empty())
    return false;

  auto path = PathHandler::resolvePath(filepath);
  if (!PathHandler::checkDirectory(path)) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "File Manager: Directory could not be verified, skip operation.");
    return false;
  }

  path /= filename;

  std::ofstream file(path, std::ios::trunc);

  if (!file)
    return false;

  for (auto element : arr) {
    file << element << " ";
  }

  file.close();

  return true;
}

template <typename T, std::size_t l> bool ArrayHandler<T, l>::readArrayFromFile(const std::string filename, std::array<T, l>& arr) {
  return readArrayFromFile("", filename, arr);
}

template <typename T, std::size_t l> bool ArrayHandler<T, l>::readArrayFromFile(const std::string filepath, const std::string filename, std::array<T, l>& arr) {

  arr.fill(0);

  auto path = PathHandler::resolvePath(filepath) / filename;
  std::ifstream file(path);

  if (!file)
    return false;

  T value;
  size_t i = 0;
  while (file >> value) {
    arr[i] = value;
    i++;
  }

  if (i == 0)
    return false;

  file.close();
  return true;
}

//==================================================
// StructHandler
//==================================================

template <typename T> bool StructHandler<T>::saveStructToFile(const std::string filename, const T& str) {
  return saveStructToFile("", filename, str);
}

template <typename T> bool StructHandler<T>::saveStructToFile(const std::string filepath, const std::string filename, const T& str) {
  auto path = PathHandler::resolvePath(filepath);
  if (!PathHandler::checkDirectory(path)) {
    logger::Logger::getInstance()->log(logger::LogVerbosity::Debug, "File Manager: Directory could not be verified, skip operation.");
    return false;
  }

  path /= filename;
  std::ofstream file(path, std::ios::trunc);

  if (!file)
    return false;

  file.write(reinterpret_cast<const char*>(&str), sizeof(T));
  file.close();

  return file.good();
}

template <typename T> bool StructHandler<T>::readStructFromFile(const std::string filename, T& str) {
  return readStructFromFile("", filename, str);
}

template <typename T> bool StructHandler<T>::readStructFromFile(const std::string filepath, const std::string filename, T& str) {

  auto path = PathHandler::resolvePath(filepath) / filename;
  std::ifstream file(path);

  if (!file)
    return false;

  file.read(reinterpret_cast<char*>(&str), sizeof(T));
  file.close();

  return file.good();
}

} // namespace filemanager

} // namespace sensorring

} // namespace eduart