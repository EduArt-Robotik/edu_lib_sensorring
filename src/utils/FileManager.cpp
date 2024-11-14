#include "utils/FileManager.hpp"

#include <fstream>
#include <iostream>
#include <filesystem>

namespace filemanager{

//==================================================
// VectorHandler
//==================================================

template<typename T>
bool VectorHandler<T>::saveVectorToFile(const std::string filename, const std::vector<T> &vec){
    return saveVectorToFile("", filename, vec);
};

template<typename T>
bool VectorHandler<T>::saveVectorToFile(const std::string filepath, const std::string filename, const std::vector<T> &vec){
    
    if(vec->empty()) return false;
    
    std::string full_path = std::filesystem::path(filepath) / filename;
    std::ofstream file(full_path, std::ios::trunc);
    
    if(!file) return false;
    
    for(auto element : *vec){
        file << element << " ";
    }

    file.close();

    return true;
};

template<typename T>
bool VectorHandler<T>::readVectorFromFile(const std::string filename, std::vector<T> &vec){
    return readVectorFromFile("", filename, vec);
};

template<typename T>
bool VectorHandler<T>::readVectorFromFile(const std::string filepath, const std::string filename, std::vector<T> &vec){
    
    vec.erase();
    std::string full_path = std::filesystem::path(filepath) / filename;
    std::ifstream file(full_path);
    
    if(!file) return false;

    T value;
    while(file >> value){
        vec.push_back(value);
    }

    if(vec.empty()) return false;

    file.close();
    return true;
};


//==================================================
// ArrayHandler
//==================================================

template<typename T, std::size_t l>
bool ArrayHandler<T, l>::saveArrayToFile(const std::string filename, const std::array<T, l> &arr){
    return saveArrayToFile("", filename, arr);
};

template<typename T, std::size_t l>
bool ArrayHandler<T, l>::saveArrayToFile(const std::string filepath, const std::string filename, const std::array<T, l> &arr){
    if(arr.empty()) return false;
    
    std::string full_path = std::filesystem::path(filepath) / filename;
    std::ofstream file(full_path, std::ios::trunc);
    
    if(!file) return false;
    
    for(auto element : arr){
        file << element << " ";
    }

    file.close();

    return true;
};

template<typename T, std::size_t l>
bool ArrayHandler<T, l>::readArrayFromFile(const std::string filename, std::array<T, l> &arr){
    return readArrayFromFile("", filename, arr);
};

template<typename T, std::size_t l>
bool ArrayHandler<T, l>::readArrayFromFile(const std::string filepath, const std::string filename, std::array<T, l> &arr){
        
    arr.fill(0);

    std::string full_path = std::filesystem::path(filepath) / filename;
    std::ifstream file(full_path);
    
    if(!file) return false;

    T value;
    size_t i = 0;
    while(file >> value){
        arr[i] = value;
        i++;
    }

    if(i==0) return false;

    file.close();
    return true;
};


//==================================================
// StructHandler
//==================================================

template<typename T>
bool StructHandler<T>::saveStructToFile(const std::string filename, const T &str){
    return saveStructToFile("", filename, str);
};

template<typename T>
bool StructHandler<T>::saveStructToFile(const std::string filepath, const std::string filename, const T &str){

    std::string full_path = std::filesystem::path(filepath) / filename;
    std::ofstream file(full_path, std::ios::trunc);
    
    if(!file) return false;
    
    file.write(reinterpret_cast<const char*>(&str), sizeof(T));
    file.close();
    
    return file.good();
};

template<typename T>
bool StructHandler<T>::readStructFromFile(const std::string filename, T &str){
    return readStructFromFile("", filename, str);
};

template<typename T>
bool StructHandler<T>::readStructFromFile(const std::string filepath, const std::string filename, T &str){

    std::string full_path = std::filesystem::path(filepath) / filename;
    std::ifstream file(full_path);
    
    if(!file) return false;

    file.read(reinterpret_cast<char*>(&str), sizeof(T));
    file.close();
    
    return file.good();
};

}; // namespace FileManager