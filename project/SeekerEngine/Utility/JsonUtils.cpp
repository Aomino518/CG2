#include "JsonUtils.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <assert.h>
#include "Logger.h"

namespace JsonUtils {
    std::string JsonUtils::ReadFilePath(const std::string& path)
    {
        std::ifstream ifs(path);
        if (!ifs.is_open()) {
            Logger::Write(Logger::LogLevel::Error, "Failed to open file" + path);
            assert(!"Failed to open file");
        }

        std::stringstream buffer;
        buffer << ifs.rdbuf();
        return buffer.str();
    }

    void JsonUtils::WriteTextFile(const std::string& path, const std::string& text)
    {
        std::ofstream ofs(path);
        if (!ofs.is_open()) {
            Logger::Write(Logger::LogLevel::Error, "Failed to open file" + path);
            assert(!"Failed to open file");
        }

        ofs << text;
    }
}
