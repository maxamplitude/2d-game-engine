#include "platform.h"
#include "logging.h"
#include <filesystem>

#ifdef _WIN32
    #include <windows.h>
#elif __APPLE__
    #include <mach-o/dyld.h>
#else
    #include <unistd.h>
#endif

namespace Engine {

std::string Platform::s_basePath;

void Platform::init() {
    char path[1024] = {};

#ifdef _WIN32
    GetModuleFileNameA(nullptr, path, sizeof(path));
#elif __APPLE__
    uint32_t size = sizeof(path);
    _NSGetExecutablePath(path, &size);
#else
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len != -1) {
        path[len] = '\0';
    }
#endif

    s_basePath = std::filesystem::path(path).parent_path().string();
    Log::info("Platform initialized. Base path: {}", s_basePath);
}

void Platform::shutdown() {
    Log::info("Platform shutdown");
}

std::string Platform::getBasePath() {
    return s_basePath;
}

std::string Platform::getResourcePath(const std::string& relativePath) {
    return (std::filesystem::path(s_basePath) / relativePath).string();
}

bool Platform::isWindows() {
#ifdef _WIN32
    return true;
#else
    return false;
#endif
}

bool Platform::isMacOS() {
#ifdef __APPLE__
    return true;
#else
    return false;
#endif
}

bool Platform::isLinux() {
#ifdef __linux__
    return true;
#else
    return false;
#endif
}

} // namespace Engine

