#pragma once

#include <string>

struct SaveData {
    int totalWins = 0;
    int gamesPlayed = 0;
    int bestWaveReached = 0;
};

class SaveSystem {
  public:
    explicit SaveSystem(std::string path);

    SaveData Load() const;
    void Save(const SaveData& data) const;

  private:
    std::string path_;
};
