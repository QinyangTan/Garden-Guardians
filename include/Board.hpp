#pragma once

#include <optional>
#include <utility>

#include "GameConfig.hpp"
#include "raylib.h"

class Board {
  public:
    explicit Board(GameConfig config);

    [[nodiscard]] Rectangle GridBounds() const;
    [[nodiscard]] Rectangle CellRect(int row, int col) const;
    [[nodiscard]] Vector2 CellCenter(int row, int col) const;
    [[nodiscard]] std::optional<std::pair<int, int>> CellFromPoint(Vector2 point) const;
    [[nodiscard]] bool InBounds(int row, int col) const;
    [[nodiscard]] float LeftDangerX() const;
    [[nodiscard]] float ZombieSpawnX() const;
    [[nodiscard]] const GameConfig& Config() const;

    void Draw() const;

  private:
    GameConfig config_;
};
