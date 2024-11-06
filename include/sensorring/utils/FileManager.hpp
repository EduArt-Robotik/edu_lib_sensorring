#pragma once

#include <string>
#include <array>
#include <vector>

#include "CustomTypes.hpp"
#include "heimann_htpa32.hpp"

namespace filemanager{

template <typename T>
class VectorHandler{
    public:
        VectorHandler() = delete;

        static bool saveVectorToFile(const std::string filename, const std::vector<T> &vec);
        static bool saveVectorToFile(const std::string filepath, const std::string filename, const std::vector<T> &vec);
        static bool readVectorFromFile(const std::string filename, std::vector<T> &vec);
        static bool readVectorFromFile(const std::string filepath, const std::string filename, std::vector<T> &vec);
};

template <typename T, std::size_t l>
class ArrayHandler{
    public:
        ArrayHandler() = delete;

        static bool saveArrayToFile(const std::string filename, const std::array<T, l> &arr);
        static bool saveArrayToFile(const std::string filepath, const std::string filename, const std::array<T, l> &arr);
        static bool readArrayFromFile(const std::string filename, std::array<T, l> &arr);
        static bool readArrayFromFile(const std::string filepath, const std::string filename, std::array<T, l> &arr);
};

template <typename T>
class StructHandler{
    public:
        StructHandler() = delete;

        static bool saveStructToFile(const std::string filename, const T &str);
        static bool saveStructToFile(const std::string filepath, const std::string filename, const T &str);
        static bool readStructFromFile(const std::string filename, T &str);
        static bool readStructFromFile(const std::string filepath, const std::string filename, T &str);
};

// concrete types that are used in the program
template class filemanager::ArrayHandler<double, THERMAL_RESOLUTION>;
template class filemanager::StructHandler<heimannsensor::HTPA32Eeprom>;

}; // namespace filemanager