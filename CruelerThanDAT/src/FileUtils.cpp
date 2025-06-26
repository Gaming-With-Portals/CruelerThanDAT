#include "pch.hpp"
#include "FileUtils.h"
bool ReadFileIntoVector(const std::string& filePath, std::vector<char>& data) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return false;
    }

    // Get file size and resize the vector to hold the file data
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    data.resize(fileSize);

    // Read the file into the vector
    file.read(reinterpret_cast<char*>(data.data()), fileSize);
    return true;
}
