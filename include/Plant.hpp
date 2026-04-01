#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Board.hpp"
#include "Types.hpp"

class Zombie;

struct PlantCard {
    PlantType type;
    std::string name;
    std::string role;
    std::string effectSummary;
    int cost;
    float cooldown;
    int health;
    Color color;
};

class Plant {
  public:
    Plant(PlantType type, int row, int col, Vector2 center, float health, Color color);
    virtual ~Plant() = default;

    virtual void Update(float dt,
                        const Board& board,
                        std::vector<std::vector<class Zombie*>>& zombiesByLane,
                        std::vector<Projectile>& projectiles,
                        std::vector<SunToken>& suns) = 0;
    virtual void Draw() const;
    virtual std::string Description() const = 0;
    [[nodiscard]] virtual bool BlocksMovement() const;

    void Damage(float amount);
    void TickVisual(float dt);
    [[nodiscard]] bool IsAlive() const;
    [[nodiscard]] int Row() const;
    [[nodiscard]] int Col() const;
    [[nodiscard]] PlantType Type() const;
    [[nodiscard]] Vector2 Center() const;
    [[nodiscard]] Rectangle Rect() const;

  protected:
    PlantType type_;
    int row_;
    int col_;
    Vector2 center_ {};
    float health_ = 100.0f;
    float maxHealth_ = 100.0f;
    float hitFlash_ = 0.0f;
    Color color_ {255, 255, 255, 255};
};

class PeashooterPlant final : public Plant {
  public:
    PeashooterPlant(int row, int col, Vector2 center);
    void Update(float dt, const Board& board, std::vector<std::vector<Zombie*>>& zombiesByLane, std::vector<Projectile>& projectiles, std::vector<SunToken>& suns) override;
    std::string Description() const override;

  private:
    float fireTimer_ = 0.6f;
};

class SunBloomPlant final : public Plant {
  public:
    SunBloomPlant(int row, int col, Vector2 center);
    void Update(float dt, const Board& board, std::vector<std::vector<Zombie*>>& zombiesByLane, std::vector<Projectile>& projectiles, std::vector<SunToken>& suns) override;
    void Draw() const override;
    std::string Description() const override;

  private:
    float sunTimer_ = 4.0f;
    float bloomPulse_ = 0.0f;
};

class WallRootPlant final : public Plant {
  public:
    WallRootPlant(int row, int col, Vector2 center);
    void Update(float dt, const Board& board, std::vector<std::vector<Zombie*>>& zombiesByLane, std::vector<Projectile>& projectiles, std::vector<SunToken>& suns) override;
    std::string Description() const override;
};

class ChomperPlant final : public Plant {
  public:
    ChomperPlant(int row, int col, Vector2 center);
    void Update(float dt, const Board& board, std::vector<std::vector<Zombie*>>& zombiesByLane, std::vector<Projectile>& projectiles, std::vector<SunToken>& suns) override;
    void Draw() const override;
    std::string Description() const override;

  private:
    float digestTimer_ = 0.0f;
    float digestDuration_ = 7.0f;
};

class FrostBurstPlant final : public Plant {
  public:
    FrostBurstPlant(int row, int col, Vector2 center);
    void Update(float dt, const Board& board, std::vector<std::vector<Zombie*>>& zombiesByLane, std::vector<Projectile>& projectiles, std::vector<SunToken>& suns) override;
    void Draw() const override;
    std::string Description() const override;

  private:
    float armTimer_ = 1.3f;
    bool exploded_ = false;
    float burstTimer_ = 0.0f;
    bool damageApplied_ = false;
};

class RepeaterPlant final : public Plant {
  public:
    RepeaterPlant(int row, int col, Vector2 center);
    void Update(float dt, const Board& board, std::vector<std::vector<Zombie*>>& zombiesByLane, std::vector<Projectile>& projectiles, std::vector<SunToken>& suns) override;
    void Draw() const override;
    std::string Description() const override;

  private:
    float fireTimer_ = 0.7f;
    float recoil_ = 0.0f;
};

class IcePeaPlant final : public Plant {
  public:
    IcePeaPlant(int row, int col, Vector2 center);
    void Update(float dt, const Board& board, std::vector<std::vector<Zombie*>>& zombiesByLane, std::vector<Projectile>& projectiles, std::vector<SunToken>& suns) override;
    void Draw() const override;
    std::string Description() const override;

  private:
    float fireTimer_ = 0.8f;
    float chillPulse_ = 0.0f;
};

class SpikeTrapPlant final : public Plant {
  public:
    SpikeTrapPlant(int row, int col, Vector2 center);
    void Update(float dt, const Board& board, std::vector<std::vector<Zombie*>>& zombiesByLane, std::vector<Projectile>& projectiles, std::vector<SunToken>& suns) override;
    void Draw() const override;
    std::string Description() const override;
    [[nodiscard]] bool BlocksMovement() const override;

  private:
    float tickTimer_ = 0.3f;
    float flash_ = 0.0f;
};

class BombBerryPlant final : public Plant {
  public:
    BombBerryPlant(int row, int col, Vector2 center);
    void Update(float dt, const Board& board, std::vector<std::vector<Zombie*>>& zombiesByLane, std::vector<Projectile>& projectiles, std::vector<SunToken>& suns) override;
    void Draw() const override;
    std::string Description() const override;

  private:
    float armTimer_ = 1.0f;
    bool exploded_ = false;
    float warningPulse_ = 0.0f;
    float burstTimer_ = 0.0f;
    bool damageApplied_ = false;
};

class MagnetVinePlant final : public Plant {
  public:
    MagnetVinePlant(int row, int col, Vector2 center);
    void Update(float dt, const Board& board, std::vector<std::vector<Zombie*>>& zombiesByLane, std::vector<Projectile>& projectiles, std::vector<SunToken>& suns) override;
    void Draw() const override;
    std::string Description() const override;

  private:
    float pulseTimer_ = 2.0f;
    float pulseFx_ = 0.0f;
};

PlantCard GetPlantCard(PlantType type);
std::unique_ptr<Plant> CreatePlant(PlantType type, int row, int col, Vector2 center);
