#pragma once

#include <string>
#include <unordered_map>

struct GameConfig {
    int screenWidth = 1280;
    int screenHeight = 720;

    int lanes = 5;
    int cols = 9;
    float boardX = 210.0f;
    float boardY = 130.0f;
    float cellWidth = 96.0f;
    float cellHeight = 98.0f;

    int startingSun = 150;
    float fallingSunInterval = 6.0f;
    int fallingSunValue = 25;
    int totalWaves = 6;
    float waveIntermission = 5.0f;
    float victoryDelay = 1.5f;
};

class ConfigLoader {
  public:
    static GameConfig Load(const std::string& path);

  private:
    static std::unordered_map<std::string, std::string> ParseFile(const std::string& path);
};
