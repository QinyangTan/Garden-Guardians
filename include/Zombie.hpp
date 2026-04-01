#pragma once

#include <memory>

#include "Types.hpp"
#include "raylib.h"

class Zombie {
  public:
    Zombie(ZombieType type, int row, float x, float y, float health, float armor, float speed, float dps, Color color, bool slowResistant = false);
    virtual ~Zombie() = default;

    virtual void Update(float dt);
    virtual void Draw() const;
    virtual void ReceiveHit(float damage, bool slow, float slowFactor, float slowDuration);

    [[nodiscard]] bool IsAlive() const;
    [[nodiscard]] float X() const;
    [[nodiscard]] int Row() const;
    [[nodiscard]] float Dps() const;
    [[nodiscard]] float Health() const;
    [[nodiscard]] float MaxHealth() const;
    [[nodiscard]] float Armor() const;
    [[nodiscard]] float MaxArmor() const;
    [[nodiscard]] bool HasArmor() const;
    [[nodiscard]] ZombieType Type() const;
    [[nodiscard]] Rectangle BodyRect() const;
    [[nodiscard]] bool IsStopped() const;
    [[nodiscard]] bool IsSlowed() const;
    [[nodiscard]] bool CanUseJump() const;
    [[nodiscard]] bool HasJumped() const;
    [[nodiscard]] float SpecialTimer() const;
    [[nodiscard]] bool IsCasting() const;

    void SetX(float newX);
    void SetY(float newY);
    void SetStopped(bool stopped);
    void Move(float dt);
    void Kill();
    void ApplyMultipliers(float healthScale, float speedScale, float damageScale);
    void RemoveArmor(float amount);
    void MarkJumpUsed();
    void SetBaseSpeed(float speed);
    void SetSpecialTimer(float time);
    void SetCasting(bool casting);
    void TickSpecial(float dt);

  protected:
    ZombieType type_;
    int row_;
    float x_;
    float y_;
    float health_;
    float maxHealth_;
    float armor_ = 0.0f;
    float maxArmor_ = 0.0f;
    float baseSpeed_;
    float dps_;
    float slowFactor_ = 1.0f;
    float slowTimer_ = 0.0f;
    float hitFlash_ = 0.0f;
    bool slowResistant_ = false;
    bool stopped_ = false;
    bool jumpUsed_ = false;
    float specialTimer_ = 0.0f;
    bool casting_ = false;
    Color color_ {200, 200, 200, 255};
};

std::unique_ptr<Zombie> CreateZombie(ZombieType type, int row, float spawnX, float laneY);
