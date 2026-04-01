#include "SaveSystem.hpp"

#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <utility>

namespace {
std::string Trim(const std::string& value) {
    std::size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])) != 0) {
        ++start;
    }
    std::size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1])) != 0) {
        --end;
    }
    return value.substr(start, end - start);
}
}  // namespace

SaveSystem::SaveSystem(std::string path) : path_(std::move(path)) {}

SaveData SaveSystem::Load() const {
    SaveData data {};
    std::ifstream file(path_);
    if (!file.is_open()) {
        return data;
    }

    std::unordered_map<std::string, std::string> values;
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
        values[Trim(line.substr(0, eqPos))] = Trim(line.substr(eqPos + 1));
    }

    auto readInt = [&values](const std::string& key, int fallback) {
        const auto it = values.find(key);
        if (it == values.end()) {
            return fallback;
        }
        std::stringstream parser(it->second);
        int out = fallback;
        parser >> out;
        return out;
    };

    data.totalWins = readInt("totalWins", 0);
    data.gamesPlayed = readInt("gamesPlayed", 0);
    data.bestWaveReached = readInt("bestWaveReached", 0);
    return data;
}

void SaveSystem::Save(const SaveData& data) const {
    std::filesystem::path filePath(path_);
    if (!filePath.parent_path().empty()) {
        std::error_code ec;
        std::filesystem::create_directories(filePath.parent_path(), ec);
    }

    std::ofstream out(path_);
    if (!out.is_open()) {
        return;
    }

    out << "# Garden Guardians save data\n";
    out << "totalWins=" << data.totalWins << "\n";
    out << "gamesPlayed=" << data.gamesPlayed << "\n";
    out << "bestWaveReached=" << data.bestWaveReached << "\n";
}
