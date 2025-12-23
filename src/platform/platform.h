#pragma once
#include <string>

namespace Engine {

class Platform {
public:
    static void init();
    static void shutdown();

    static std::string getBasePath();
    static std::string getResourcePath(const std::string& relativePath);

    static bool isWindows();
    static bool isMacOS();
    static bool isLinux();

private:
    static std::string s_basePath;
};

} // namespace Engine

