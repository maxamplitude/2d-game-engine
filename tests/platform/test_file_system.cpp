#include <catch2/catch_test_macros.hpp>
#include "platform/file_system.h"
#include <fstream>
#include <cstdio>

using namespace Engine;

TEST_CASE("FileSystem text read/write roundtrip", "[platform][filesystem]") {
    const std::string path = "fs_test_temp.txt";
    {
        std::ofstream out(path);
        out << "hello\nworld";
    }

    REQUIRE(FileSystem::fileExists(path));
    auto text = FileSystem::loadTextFile(path);
    REQUIRE(text.has_value());
    REQUIRE(text->find("world") != std::string::npos);
    REQUIRE(FileSystem::getFileSize(path) == text->size());

    std::remove(path.c_str());
}

TEST_CASE("FileSystem path helpers", "[platform][filesystem]") {
    std::string combined = FileSystem::combinePath("root/dir", "file.ext");
    REQUIRE(combined.find("file.ext") != std::string::npos);
    REQUIRE(FileSystem::getFilename(combined) == "file.ext");
    REQUIRE(FileSystem::getExtension(combined) == ".ext");
}

