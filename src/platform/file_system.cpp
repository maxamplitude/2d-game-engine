#include "file_system.h"
#include "logging.h"
#include <fstream>
#include <filesystem>

namespace Engine {

std::optional<std::vector<uint8_t>> FileSystem::loadBinaryFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        Log::error("Failed to open binary file: {}", path);
        return std::nullopt;
    }

    const std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        Log::error("Failed to read binary file: {}", path);
        return std::nullopt;
    }

    return buffer;
}

std::optional<std::string> FileSystem::loadTextFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        Log::error("Failed to open text file: {}", path);
        return std::nullopt;
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    return content;
}

bool FileSystem::fileExists(const std::string& path) {
    return std::filesystem::exists(path);
}

size_t FileSystem::getFileSize(const std::string& path) {
    return std::filesystem::file_size(path);
}

std::string FileSystem::getDirectory(const std::string& path) {
    return std::filesystem::path(path).parent_path().string();
}

std::string FileSystem::getFilename(const std::string& path) {
    return std::filesystem::path(path).filename().string();
}

std::string FileSystem::getExtension(const std::string& path) {
    return std::filesystem::path(path).extension().string();
}

std::string FileSystem::combinePath(const std::string& a, const std::string& b) {
    return (std::filesystem::path(a) / b).string();
}

} // namespace Engine

