#include "Board.hpp"

#include "raylib.h"

Board::Board(GameConfig config) : config_(config) {}

Rectangle Board::GridBounds() const {
    return Rectangle {config_.boardX, config_.boardY, config_.cellWidth * config_.cols, config_.cellHeight * config_.lanes};
}

Rectangle Board::CellRect(int row, int col) const {
    return Rectangle {
        config_.boardX + static_cast<float>(col) * config_.cellWidth,
        config_.boardY + static_cast<float>(row) * config_.cellHeight,
        config_.cellWidth,
        config_.cellHeight
    };
}

Vector2 Board::CellCenter(int row, int col) const {
    const Rectangle rect = CellRect(row, col);
    return Vector2 {rect.x + rect.width * 0.5f, rect.y + rect.height * 0.5f};
}

std::optional<std::pair<int, int>> Board::CellFromPoint(Vector2 point) const {
    const Rectangle bounds = GridBounds();
    if (!CheckCollisionPointRec(point, bounds)) {
        return std::nullopt;
    }

    const int col = static_cast<int>((point.x - config_.boardX) / config_.cellWidth);
    const int row = static_cast<int>((point.y - config_.boardY) / config_.cellHeight);
    if (!InBounds(row, col)) {
        return std::nullopt;
    }
    return std::make_pair(row, col);
}

bool Board::InBounds(int row, int col) const {
    return row >= 0 && row < config_.lanes && col >= 0 && col < config_.cols;
}

float Board::LeftDangerX() const {
    return config_.boardX - 60.0f;
}

float Board::ZombieSpawnX() const {
    return config_.boardX + config_.cellWidth * config_.cols + 30.0f;
}

const GameConfig& Board::Config() const {
    return config_;
}

void Board::Draw() const {
    const Rectangle grid = GridBounds();
    DrawRectangleRounded(grid, 0.03f, 5, Color {98, 156, 82, 255});
    DrawRectangleLinesEx(grid, 2.0f, Color {42, 90, 42, 255});

    for (int row = 0; row < config_.lanes; ++row) {
        for (int col = 0; col < config_.cols; ++col) {
            const Rectangle cell = CellRect(row, col);
            const Color cellColor = ((row + col) % 2 == 0) ? Color {109, 178, 90, 255} : Color {98, 162, 82, 255};
            DrawRectangleRounded(cell, 0.08f, 4, cellColor);
            DrawRectangleLinesEx(cell, 1.0f, Color {70, 120, 68, 150});
        }
    }

    DrawRectangle(0, static_cast<int>(config_.boardY), static_cast<int>(config_.boardX - 25.0f), static_cast<int>(config_.cellHeight * config_.lanes), Color {204, 194, 174, 255});
    DrawText("HOME", 52, static_cast<int>(config_.boardY + config_.cellHeight * 2.0f), 32, DARKBROWN);
}
