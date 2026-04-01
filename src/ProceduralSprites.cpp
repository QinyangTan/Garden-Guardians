#include "ProceduralSprites.hpp"

#include <cmath>

namespace {
void DrawPlantFrame(Image* image, PlantType type, int frameOffsetX, int frameW, int frameH, int phase) {
    const int cx = frameOffsetX + frameW / 2;
    const int cy = frameH / 2;
    const int bob = (phase % 2 == 0) ? -2 : 2;

    switch (type) {
        case PlantType::Peashooter:
            ImageDrawCircle(image, cx, cy + bob, 22, Color {71, 181, 74, 255});
            ImageDrawCircle(image, cx + 14, cy - 6 + bob, 10, Color {55, 140, 64, 255});
            ImageDrawRectangle(image, cx - 4, cy + 18 + bob, 8, 14, DARKGREEN);
            break;
        case PlantType::SunBloom:
            ImageDrawCircle(image, cx, cy + bob, 15, Color {245, 185, 56, 255});
            for (int i = 0; i < 8; ++i) {
                const float a = (2.0f * PI * static_cast<float>(i)) / 8.0f + static_cast<float>(phase) * 0.2f;
                const int px = cx + static_cast<int>(std::cos(a) * 22.0f);
                const int py = cy + bob + static_cast<int>(std::sin(a) * 22.0f);
                ImageDrawCircle(image, px, py, 8, YELLOW);
            }
            ImageDrawRectangle(image, cx - 3, cy + 16 + bob, 6, 15, DARKGREEN);
            break;
        case PlantType::WallRoot:
            ImageDrawRectangle(image, cx - 18, cy - 24 + bob, 36, 48, Color {124, 90, 62, 255});
            ImageDrawRectangle(image, cx - 12, cy - 18 + bob, 8, 10, Color {158, 120, 88, 255});
            ImageDrawRectangle(image, cx + 4, cy - 10 + bob, 8, 10, Color {158, 120, 88, 255});
            break;
        case PlantType::Chomper:
            ImageDrawCircle(image, cx, cy + bob, 22, Color {177, 82, 202, 255});
            ImageDrawCircle(image, cx + 6, cy - 8 + bob, 10, Color {115, 46, 134, 255});
            ImageDrawCircle(image, cx + 12, cy - 8 + bob, 3 + (phase % 2), WHITE);
            break;
        case PlantType::FrostBurst:
            ImageDrawCircle(image, cx, cy + bob, 20, Color {89, 194, 250, 255});
            ImageDrawCircle(image, cx, cy + bob, 26 + (phase % 3), Color {130, 220, 255, 80});
            ImageDrawCircle(image, cx, cy + bob, 20, Color {89, 194, 250, 255});
            ImageDrawCircle(image, cx + 6, cy - 8 + bob, 3, WHITE);
            break;
        case PlantType::Repeater:
            ImageDrawCircle(image, cx - 6, cy + bob, 16, Color {44, 143, 58, 255});
            ImageDrawCircle(image, cx + 12, cy + bob, 16, Color {57, 166, 72, 255});
            break;
        case PlantType::IcePea:
            ImageDrawCircle(image, cx, cy + bob, 19, Color {116, 211, 248, 255});
            ImageDrawCircle(image, cx + 10, cy - 6 + bob, 7, WHITE);
            break;
        case PlantType::SpikeTrap:
            ImageDrawRectangle(image, cx - 22, cy + 10 + bob, 44, 10, Color {108, 86, 70, 255});
            for (int i = 0; i < 5; ++i) {
                const int sx = cx - 18 + i * 8;
                ImageDrawRectangle(image, sx, cy - 2 + bob, 6, 12, LIGHTGRAY);
            }
            break;
        case PlantType::BombBerry:
            ImageDrawCircle(image, cx, cy + bob, 17, Color {230, 102, 89, 255});
            ImageDrawCircle(image, cx + 5, cy - 7 + bob, 5 + (phase % 2), YELLOW);
            break;
        case PlantType::MagnetVine:
            ImageDrawCircle(image, cx, cy + bob, 18, Color {151, 104, 220, 255});
            ImageDrawRectangle(image, cx - 10, cy - 4 + bob, 20, 5, Color {210, 160, 255, 255});
            break;
        default: break;
    }
}

void DrawZombieFrame(Image* image, ZombieType type, int frameOffsetX, int frameW, int frameH, int phase) {
    const int cx = frameOffsetX + frameW / 2;
    const int cy = frameH / 2 + ((phase % 2 == 0) ? -1 : 1);
    Color body = Color {120, 145, 110, 255};
    int headR = 15;

    if (type == ZombieType::Runner || type == ZombieType::Imp) {
        body = Color {225, 132, 85, 255};
        headR = (type == ZombieType::Imp) ? 10 : 13;
    } else if (type == ZombieType::Tank) {
        body = Color {105, 87, 144, 255};
        headR = 16;
    } else if (type == ZombieType::Bucket || type == ZombieType::Conehead) {
        body = Color {136, 136, 156, 255};
        headR = 15;
    } else if (type == ZombieType::Spitter) {
        body = Color {84, 188, 122, 255};
    } else if (type == ZombieType::PoleJump) {
        body = Color {215, 165, 96, 255};
    } else if (type == ZombieType::Summoner) {
        body = Color {148, 96, 200, 255};
    }

    ImageDrawRectangle(image, cx - 16, cy - 18, 32, 40, body);
    ImageDrawCircle(image, cx, cy - 20, headR, Color {176, 198, 154, 255});
    ImageDrawRectangle(image, cx - 14, cy + 22, 10, 18, DARKBROWN);
    ImageDrawRectangle(image, cx + 4, cy + 22, 10, 18, DARKBROWN);

    if (type == ZombieType::Bucket || type == ZombieType::Conehead) {
        ImageDrawRectangle(image, cx - 16, cy - 38, 32, 11, GRAY);
    }
    if (type == ZombieType::PoleJump) {
        ImageDrawRectangle(image, cx + 18, cy - 18, 5, 36, BROWN);
    }
}
}  // namespace

void ProceduralSprites::Initialize() {
    if (ready_) {
        return;
    }

    plantAtlases_.assign(PlantTypeCount(), Texture2D {});
    zombieAtlases_.assign(ZombieTypeCount(), Texture2D {});
    const auto plants = AllPlantTypes();
    for (PlantType type : plants) {
        plantAtlases_[PlantIndex(type)] = BuildPlantAtlas(type);
    }
    const auto zombies = AllZombieTypes();
    for (ZombieType type : zombies) {
        zombieAtlases_[ZombieIndex(type)] = BuildZombieAtlas(type);
    }
    ready_ = true;
}

void ProceduralSprites::Shutdown() {
    if (!ready_) {
        return;
    }
    for (auto& texture : plantAtlases_) {
        if (texture.id > 0) {
            UnloadTexture(texture);
            texture = Texture2D {};
        }
    }
    for (auto& texture : zombieAtlases_) {
        if (texture.id > 0) {
            UnloadTexture(texture);
            texture = Texture2D {};
        }
    }
    ready_ = false;
}

bool ProceduralSprites::Ready() const {
    return ready_;
}

void ProceduralSprites::DrawPlant(PlantType type, Vector2 center, float timeSeconds) const {
    if (!ready_) {
        return;
    }
    const std::size_t index = PlantIndex(type);
    if (index >= plantAtlases_.size()) {
        return;
    }

    const Texture2D atlas = plantAtlases_[index];
    if (atlas.id == 0 || atlas.width <= 0 || atlas.height <= 0) {
        return;
    }
    const int frame = static_cast<int>(timeSeconds * 6.0f) % kFrames;
    const float frameW = static_cast<float>(atlas.width / kFrames);
    const Rectangle src {frameW * static_cast<float>(frame), 0.0f, frameW, static_cast<float>(atlas.height)};
    const Rectangle dst {center.x - 28.0f, center.y - 30.0f, 56.0f, 60.0f};
    DrawTexturePro(atlas, src, dst, Vector2 {0.0f, 0.0f}, 0.0f, WHITE);
}

void ProceduralSprites::DrawZombie(ZombieType type, Rectangle bodyRect, float timeSeconds) const {
    if (!ready_) {
        return;
    }
    const std::size_t index = ZombieIndex(type);
    if (index >= zombieAtlases_.size()) {
        return;
    }
    const Texture2D atlas = zombieAtlases_[index];
    if (atlas.id == 0 || atlas.width <= 0 || atlas.height <= 0) {
        return;
    }

    const int frame = static_cast<int>(timeSeconds * 8.0f) % kFrames;
    const float frameW = static_cast<float>(atlas.width / kFrames);
    const Rectangle src {frameW * static_cast<float>(frame), 0.0f, frameW, static_cast<float>(atlas.height)};
    const Rectangle dst {bodyRect.x, bodyRect.y - 8.0f, bodyRect.width, bodyRect.height + 12.0f};
    DrawTexturePro(atlas, src, dst, Vector2 {0.0f, 0.0f}, 0.0f, WHITE);
}

Texture2D ProceduralSprites::BuildPlantAtlas(PlantType type) const {
    constexpr int frameW = 64;
    constexpr int frameH = 64;
    Image image = GenImageColor(frameW * kFrames, frameH, Color {0, 0, 0, 0});
    for (int frame = 0; frame < kFrames; ++frame) {
        DrawPlantFrame(&image, type, frame * frameW, frameW, frameH, frame);
    }
    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);
    return texture;
}

Texture2D ProceduralSprites::BuildZombieAtlas(ZombieType type) const {
    constexpr int frameW = 64;
    constexpr int frameH = 88;
    Image image = GenImageColor(frameW * kFrames, frameH, Color {0, 0, 0, 0});
    for (int frame = 0; frame < kFrames; ++frame) {
        DrawZombieFrame(&image, type, frame * frameW, frameW, frameH, frame);
    }
    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);
    return texture;
}
