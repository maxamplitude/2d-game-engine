#pragma once
#include <string>
#include <vector>
#include <optional>
#include <cstdint>

namespace Engine {

class FileSystem {
public:
    static std::optional<std::vector<uint8_t>> loadBinaryFile(const std::string& path);
    static std::optional<std::string> loadTextFile(const std::string& path);
    static bool fileExists(const std::string& path);
    static size_t getFileSize(const std::string& path);
    static std::string getDirectory(const std::string& path);
    static std::string getFilename(const std::string& path);
    static std::string getExtension(const std::string& path);
    static std::string combinePath(const std::string& a, const std::string& b);
};

} // namespace Engine

