#include "GameConfig.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

namespace {
std::string Trim(const std::string& text) {
    std::size_t begin = 0;
    while (begin < text.size() && std::isspace(static_cast<unsigned char>(text[begin])) != 0) {
        ++begin;
    }

    std::size_t end = text.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1])) != 0) {
        --end;
    }
    return text.substr(begin, end - begin);
}

template <typename T>
void AssignIfPresent(const std::unordered_map<std::string, std::string>& values, const std::string& key, T& outValue) {
    auto iter = values.find(key);
    if (iter == values.end()) {
        return;
    }
    std::stringstream parser(iter->second);
    parser >> outValue;
}
}  // namespace

GameConfig ConfigLoader::Load(const std::string& path) {
    GameConfig config {};
    const auto values = ParseFile(path);

    AssignIfPresent(values, "screenWidth", config.screenWidth);
    AssignIfPresent(values, "screenHeight", config.screenHeight);
    AssignIfPresent(values, "lanes", config.lanes);
    AssignIfPresent(values, "cols", config.cols);
    AssignIfPresent(values, "boardX", config.boardX);
    AssignIfPresent(values, "boardY", config.boardY);
    AssignIfPresent(values, "cellWidth", config.cellWidth);
    AssignIfPresent(values, "cellHeight", config.cellHeight);
    AssignIfPresent(values, "startingSun", config.startingSun);
    AssignIfPresent(values, "fallingSunInterval", config.fallingSunInterval);
    AssignIfPresent(values, "fallingSunValue", config.fallingSunValue);
    AssignIfPresent(values, "totalWaves", config.totalWaves);
    AssignIfPresent(values, "waveIntermission", config.waveIntermission);
    AssignIfPresent(values, "victoryDelay", config.victoryDelay);

    return config;
}

std::unordered_map<std::string, std::string> ConfigLoader::ParseFile(const std::string& path) {
    std::unordered_map<std::string, std::string> values;
    std::ifstream file(path);
    if (!file.is_open()) {
        return values;
    }

    std::string line;
    while (std::getline(file, line)) {
        const auto hashPos = line.find('#');
        if (hashPos != std::string::npos) {
            line = line.substr(0, hashPos);
        }

        const auto eqPos = line.find('=');
        if (eqPos == std::string::npos) {
            continue;
        }

        const std::string key = Trim(line.substr(0, eqPos));
        const std::string value = Trim(line.substr(eqPos + 1));
        if (!key.empty() && !value.empty()) {
            values[key] = value;
        }
    }
    return values;
}
