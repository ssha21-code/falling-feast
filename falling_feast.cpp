#include <array>
#include <chrono>
#include <ctime>
#include "ExtraHeader.h"
#include <iostream>
#include <memory>
#include <raylib.h>
#include <raymath.h>
#include <vector>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

constexpr int WINDOW_WIDTH = 1000;
constexpr int WINDOW_HEIGHT = 800;
constexpr int DEFAULT_FPS = 60;
constexpr int FPS = 60;

Texture2D texturePlayer;
Texture2D texturePlayerStanding;
Texture2D textureTerrainSpriteSheet;
Texture2D textureGroundSpriteSheet;
Texture2D textureGoodFoodSpriteSheet;
Texture2D textureBadFoodSpriteSheet;
Texture2D texturePausePlayButtonSpriteSheet;
Texture2D textureProjectileSpriteSheet;
Texture2D textureTitleScreen;
Texture2D textureBow;
Texture2D textureEnemy;
Texture2D textureCoin;
Texture2D textureBroccoliBuddy;

Sound soundFail;
Sound soundLevelUp;
Sound soundKaching;
Sound soundBite;
Sound soundClick;
Sound soundShoot;
Sound soundHit;
Sound soundCollect;
Music musicCollectingBackground;
Music musicFightingBackground;

Font font;

void loadMedia() {
    texturePlayer = LoadTexture("images/player.png");
    texturePlayerStanding = LoadTexture("images/player_standing.png");
    textureTerrainSpriteSheet = LoadTexture("images/terrain_sprite_sheet.png");
    textureGroundSpriteSheet = LoadTexture("images/ground_sprite_sheet.png");
    textureGoodFoodSpriteSheet = LoadTexture("images/good_food_sprite_sheet.png");
    textureBadFoodSpriteSheet = LoadTexture("images/bad_food_sprite_sheet.png");
    texturePausePlayButtonSpriteSheet = LoadTexture("images/pause_play_button_sprite_sheet.png");
    textureProjectileSpriteSheet = LoadTexture("images/projectile_sprite_sheet.png");
    textureBow = LoadTexture("images/bow.png");
    textureTitleScreen = LoadTexture("images/title_screen.png");
    textureEnemy = LoadTexture("images/enemy.png");
    textureCoin = LoadTexture("images/coin.png");
    textureBroccoliBuddy = LoadTexture("images/broccoli_buddy.png");

    soundFail = LoadSound("sounds/fail.wav");
    soundLevelUp = LoadSound("sounds/level_up.wav");
    soundKaching = LoadSound("sounds/kaching.wav");
    soundBite = LoadSound("sounds/bite.wav");
    soundClick = LoadSound("sounds/click.wav");
    soundShoot = LoadSound("sounds/shoot.wav");
    soundHit = LoadSound("sounds/hit.wav");
    soundCollect = LoadSound("sounds/collect.wav");

    musicCollectingBackground = LoadMusicStream("sounds/collecting_background.wav");
    musicFightingBackground = LoadMusicStream("sounds/fighting_background.wav");

    font = LoadFontEx("fonts/font.ttf", 64, nullptr, 0);
}

class Player {
public:
    Vector2 position;
    Vector2 size;
    Vector2 center;
    float velocity;
    static constexpr float groundY = 700.0f;
    float nutrition;
    int prevGameStateIndex = 0;
    int coins = 0;
    Vector2 savedPosition = {0};
    float lookingAngle = 0;
    double level = 1;
    float health;
    float maxHealth;

    bool isDead = false;

    bool isAttracting = false;
    bool isExtraFast = false;
    bool isImmune = false;
    
    float powerUpDuration = 25.0f;
    float attractionTimer = 0;
    float speedTimer = 0;
    float immunityTimer = 0;

    Player() {
        this->size = {(float)texturePlayer.width, (float)texturePlayer.height};
        this->position = {400.0f, groundY - size.y};
        this->velocity = 8.0f * DEFAULT_FPS;
        this->nutrition = 100.0f;
        this->maxHealth = 100.0f;
        this->health = maxHealth;
    }

    void draw(int gameModeIndex) {
        if (gameModeIndex == 0) {
            DrawTexture(texturePlayer, position.x, position.y, WHITE);
        } else {
            DrawTexture(texturePlayerStanding, position.x, position.y, WHITE);
        }
    }
    void update(double dt, double timeElapsed, int gameStateIndex) {
        if (gameStateIndex != prevGameStateIndex) {
            Vector2 toBeSavedPosition = position;
            position = savedPosition;
            savedPosition = toBeSavedPosition;
        }
        if (isExtraFast) {
            velocity = 15.0f * DEFAULT_FPS;
        }
        if (gameStateIndex == 0) velocity = 8.0f * DEFAULT_FPS;
        if (IsKeyDown(KEY_A) && position.x > 0) {
            position.x -= velocity * dt;
        }
        if (IsKeyDown(KEY_D) && position.x + size.x < WINDOW_WIDTH) {
            position.x += velocity * dt;
        }
        if (gameStateIndex == 0) {
            position.y = groundY - size.y;
            size = {(float)texturePlayer.width, (float)texturePlayer.height};
        }
        if (gameStateIndex == 1) {
            if (IsKeyDown(KEY_W) && position.y > 0) {
                position.y -= velocity * dt;
            }
            if (IsKeyDown(KEY_S) && position.y + size.y < WINDOW_HEIGHT) {
                position.y += velocity * dt;
            }
            size = {(float)texturePlayerStanding.width, (float)texturePlayerStanding.height};
        }
        Vector2 mousePos = GetMousePosition();
        Vector2 delta = Vector2Subtract({position.x + size.x / 2, position.y + size.y / 2}, {mousePos});
        lookingAngle = atan2(delta.y, delta.x) * RAD2DEG;

        if (nutrition <= 0) {
            nutrition = 0;
        }
        if (health <= 0) {
            health = 0;
            isDead = true;
        }
        center = {position.x + size.x / 2, position.y + size.y / 2};
        prevGameStateIndex = gameStateIndex;
    }
    void drawDebugLines() {
        DrawRectangleLinesEx({position.x, position.y, size.x, size.y}, 2, RED);
        DrawCircleV(position, 3, BLUE);
    }
};

class Projectile {
public: 
    Vector2 position;
    Vector2 size;
    Vector2 velocity;
    Vector2 origin;
    int spriteSheetIndex;
    float angleDeg;
    std::array<Vector2, 4> rectCorners;

    bool isPlayerProjectile;
    bool shouldBeDestroyed = false;

    Projectile(Vector2 position, bool isPlayerProjectile, float angleDeg) {
        this->position = position;
        this->isPlayerProjectile = isPlayerProjectile;
        Vector2 maximumVelocity = {10.0f * DEFAULT_FPS, 10.0f * DEFAULT_FPS};
        velocity = {
            (float)cos(angleDeg * DEG2RAD) * maximumVelocity.x,
            (float)sin(angleDeg * DEG2RAD) * maximumVelocity.y,
        };
        this->spriteSheetIndex = static_cast<int>(!isPlayerProjectile);
        this->angleDeg = angleDeg;
        this->size = {100.0f, 13.0f};
        this->origin = {size.x / 2.0f, size.y / 2.0f};
        std::array<Vector2, 4> localCorners = {
            Vector2{ -origin.x, -origin.y },
            Vector2{  origin.x, -origin.y },
            Vector2{  origin.x,  origin.y },
            Vector2{ -origin.x,  origin.y }
        };
        for (int i = 0; i < 4; i++) {
            Vector2 rotated = Vector2Rotate(localCorners[i], angleDeg * DEG2RAD);
            rectCorners[i] = Vector2Add({position.x, position.y}, rotated);
        }
    }

    void draw() {
        Rectangle src = {spriteSheetIndex * size.x, 0, size.x, size.y};
        Rectangle dest = {position.x, position.y, size.x, size.y};
        
        DrawTexturePro(textureProjectileSpriteSheet, src, dest, origin, angleDeg, WHITE);
        // for (int i = 0; i < rectCorners.size(); i++) {
        //     DrawLineEx(rectCorners.at(i), rectCorners.at(i != 3 ? i+1 : 0), 2, RED);
        // }
        // DrawCircleV(position, 5, BLUE);
    }
    void update(float dt) {
        position = Vector2Add(position, {velocity.x * dt, velocity.y * dt});
        std::array<Vector2, 4> localCorners = {
            Vector2{ -origin.x, -origin.y },
            Vector2{  origin.x, -origin.y },
            Vector2{  origin.x,  origin.y },
            Vector2{ -origin.x,  origin.y }
        };
        for (int i = 0; i < 4; i++) {
            Vector2 rotated = Vector2Rotate(localCorners[i], angleDeg * DEG2RAD);
            rectCorners[i] = Vector2Add({position.x, position.y}, rotated);
        }
    }
    void drawDebugLines() {
        for (int i = 0; i < rectCorners.size(); i++) {
            DrawLineEx(rectCorners.at(i), i == 3 ? rectCorners.at(0) : rectCorners.at(i + 1), 2, RED);
        }
        DrawCircleV(position, 3, BLUE);
    }
};

class Bow {
public: 
    Vector2 position;
    Vector2 size;
    Vector2 origin;
    Vector2 *followPosition;
    Vector2 *pointingPosition;
    float angleDeg;
    float damage;
    float baseDamage;
    float extraDamagePerlevel;
    std::array<Vector2, 4> rectCorners;
    double *playerLevel;

    bool isPlayerBow;
    bool isBroccoliBow;
    bool shouldShoot = false;
    
    Bow(Vector2 *followPosition, Vector2 *pointingPosition, bool isPlayerBow, bool isBroccoliBow, double *playerLevel) {
        this->followPosition = followPosition;
        this->position = {followPosition->x + 50.0f, followPosition->y + 125.0f};
        this->size = {(float)textureBow.width, (float)textureBow.height};
        this->origin = {size.x / 2.0f, size.y / 2.0f};
        this->isPlayerBow = isPlayerBow;
        this->baseDamage = 10.0f;
        this->isBroccoliBow = isBroccoliBow;
        if (isPlayerBow || isBroccoliBow) {
            this->playerLevel = playerLevel;
            this->extraDamagePerlevel = 2.0f;
        } else {
            this->pointingPosition = pointingPosition;
        }
    }

    void draw() {
        Rectangle src = {0, 0, size.x, size.y};
        Rectangle dest = {position.x, position.y, size.x, size.y};
        DrawTexturePro(textureBow, src, dest, origin, angleDeg, WHITE);
        // for (int i = 0; i < rectCorners.size(); i++) {
        //     DrawLineEx(rectCorners.at(i), rectCorners.at(i != 3 ? i+1 : 0), 2, RED);
        // }
        // DrawCircleV(position, 5, BLUE);
    }
    void update() {
        position = {followPosition->x + 50.0f, followPosition->y + 125.0f};
        if (isPlayerBow) {
            Vector2 mousePos = GetMousePosition();
            Vector2 delta = {
                mousePos.x - position.x, 
                mousePos.y - position.y
            };
            angleDeg = atan2(delta.y, delta.x) * RAD2DEG;

            if (IsKeyPressed(KEY_SPACE)) {
                shouldShoot = true;
            }
            damage = baseDamage + ((static_cast<int>(*playerLevel) - 1) * extraDamagePerlevel);
        } else if (!isPlayerBow && !isBroccoliBow) {
            Vector2 delta = {
                pointingPosition->x - position.x, 
                pointingPosition->y - position.y
            };
            angleDeg = atan2(delta.y, delta.x) * RAD2DEG;
        }

        std::array<Vector2, 4> localCorners = {
            Vector2{ -origin.x, -origin.y },
            Vector2{  origin.x, -origin.y },
            Vector2{  origin.x,  origin.y },
            Vector2{ -origin.x,  origin.y }
        };
        for (int i = 0; i < 4; i++) {
            Vector2 rotated = Vector2Rotate(localCorners[i], angleDeg * DEG2RAD);
            rectCorners[i] = Vector2Add({position.x, position.y}, rotated);
        }
    }
    void drawDebugLines() {
        for (int i = 0; i < rectCorners.size(); i++) {
            DrawLineEx(rectCorners.at(i), i == 3 ? rectCorners.at(0) : rectCorners.at(i + 1), 2, RED);
        }
        DrawCircleV(position, 3, BLUE);
    }
};

class Enemy {
public: 
    Vector2 position;
    Vector2 size;
    Vector2 *playerPosition;
    Vector2 velocity;
    float playerAngleDeg;
    float distanceToMove;
    float distanceMoved = 0;
    float shootingCooldown;
    float lastShootingTime = 0;
    float health;
    std::unique_ptr<Bow> associatedBow;

    bool hasReachedPosition = false;
    bool shouldShoot = false;
    bool isDead = false;

    Enemy(Vector2 position, Vector2 *playerPosition) {
        this->position = position;
        this->playerPosition = playerPosition;
        this->distanceToMove = 400.0f;
        this->shootingCooldown = GetRandomValue(20, 25) / 10.0f;
        this->health = 50.0f;
        this->size = {(float)textureEnemy.width, (float)textureEnemy.height};
        this->velocity = {8.0f * DEFAULT_FPS, 8.0f * DEFAULT_FPS};
    }

    void draw() {
        DrawTexture(textureEnemy, position.x, position.y, WHITE);
        DrawTextEx(font, TextFormat("%i", (int)health), {position.x + 40.0f, position.y - 40.0f}, 35.0f, 1.0f, BLACK);
    }
    void update(float dt, float timeElapsed) {
        Vector2 delta = Vector2Subtract(*playerPosition, position);
        playerAngleDeg = atan2(delta.y, delta.x) * RAD2DEG;

        if (hasReachedPosition) {
            if (timeElapsed - lastShootingTime >= shootingCooldown) {
                shouldShoot = true;
                lastShootingTime = timeElapsed;
            }
        } else {
            Vector2 velocityToMove = {
                cos(playerAngleDeg * DEG2RAD) * velocity.x * dt,
                sin(playerAngleDeg * DEG2RAD) * velocity.y * dt,
            };
            position = Vector2Add(position, velocityToMove);
            distanceMoved += Vector2Length(velocityToMove);

            if (distanceMoved >= distanceToMove) {
                hasReachedPosition = true;
            }
        }
        if (health <= 0) {
            health = 0;
            isDead = true;
        }
    }
    void makeAssociatedBow(std::unique_ptr<Bow> associatedBow) {
        this->associatedBow = std::move(associatedBow);
    }
    void drawDebugLines() {
        DrawRectangleLinesEx({position.x, position.y, size.x, size.y}, 2, RED);
        DrawCircleV(position, 3, BLUE);
    }
};

class Coin {
public: 
    Vector2 position;
    Vector2 size;

    bool shouldBeDestroyed = false;

    Coin(Vector2 position) {
        this->position = position;
        this->size = {(float) textureCoin.width, (float) textureCoin.height};
    }

    void draw() {
        DrawTexture(textureCoin, position.x, position.y, WHITE);
    }
    void drawDebugLines() {
        DrawRectangleLinesEx({position.x, position.y, size.x, size.y}, 2, RED);
        DrawCircleV(position, 3, BLUE);
    }
};

class BroccoliBuddy {
public: 
    Vector2 position;
    Vector2 size;
    float velocity;
    std::unique_ptr<Bow> associatedBow;
    std::vector<std::unique_ptr<Enemy>> *enemies;
    float shootTimer = 0;
    float shootCooldown;
    float distanceMoved = 0;
    float aimingAngle = 0;
    float existenceTime = 20.0f;
    float existenceTimer = 0;

    bool shouldBeDestroyed = false;
    bool hasReachedPosition = false;

    BroccoliBuddy(Vector2 position, std::vector<std::unique_ptr<Enemy>> *enemies) {
        this->position = position;
        this->size = {(float)textureBroccoliBuddy.width, (float)textureBroccoliBuddy.height};
        this->velocity = 8.0f * DEFAULT_FPS;
        this->shootCooldown = GetRandomValue(2, 5) / 10.0f;
        this->enemies = enemies;
    }

    void draw(double timeElapsed) {
        DrawTexture(textureBroccoliBuddy, position.x, position.y, WHITE);
        if (hasReachedPosition) DrawTextEx(font, TextFormat("%i", (int)(existenceTime - (timeElapsed - existenceTimer))), {position.x + 30.0f, position.y - 40.0f}, 35.0f, 1.0f, BLACK);
    }

    void update(double dt, double timeElapsed) {
        if (hasReachedPosition) {
            if (!enemies->empty()) {
                Vector2 enemyPosition = enemies->at(0)->position;
                Vector2 enemySize = enemies->at(0)->size;
                Vector2 delta = {enemyPosition.x + enemySize.x / 2 - (position.x + size.x / 2), 
                    enemyPosition.y + enemySize.y / 2 - (position.y + size.y / 2)
                };
                aimingAngle = atan2(delta.y, delta.x) * RAD2DEG;
                associatedBow->angleDeg = aimingAngle;
            }

            if (timeElapsed - shootTimer >= shootCooldown) {
                associatedBow->shouldShoot = true;
                shootTimer = timeElapsed;
            }

            if (timeElapsed - existenceTimer >= existenceTime) {
                shouldBeDestroyed = true;
            }
        } else {
            position.y += velocity * dt;
            distanceMoved += velocity * dt;

            if (distanceMoved >= 400.0f) {
                hasReachedPosition = true;
                existenceTimer = timeElapsed;
            }
        }
    }
    void drawDebugLines() {
        DrawRectangleLinesEx({position.x, position.y, size.x, size.y}, 2, RED);
        DrawCircleV(position, 3, BLUE);
    }
    void makeAssociatedBow(std::unique_ptr<Bow> associatedBow) {
        this->associatedBow = std::move(associatedBow);
    }
};

class GoodFood {
public: 
    Vector2 position;
    Vector2 size;
    float velocityY = 6.0f * DEFAULT_FPS;
    float nutritionalValue;
    int spriteSheetIndex;

    bool shouldBeDestroyed = false;
    bool isLeftOfPlayer;

    GoodFood() {};

    virtual void draw() = 0;
    void update(double dt, double timeElapsed) {
        position.y += velocityY * dt;
        if (position.y > WINDOW_HEIGHT) shouldBeDestroyed = true;
    }
    void drawDebugLines() {
        DrawRectangleLinesEx({position.x, position.y, size.x, size.y}, 2, RED);
        DrawCircleV(position, 3, BLUE);
    }
};

class Cheese : public GoodFood{
public: 
    Cheese(Vector2 position, float speed) {
        this->position = position;
        this->size = {80.0f, 80.0f};
        this->nutritionalValue = 30.0f;
        this->spriteSheetIndex = 0;
        this->velocityY = speed * DEFAULT_FPS;
    }

    void draw() override {
        Rectangle src = {spriteSheetIndex * size.x, 0, size.x, size.y};
        Rectangle dest = {position.x, position.y, size.x, size.y};
        DrawTexturePro(textureGoodFoodSpriteSheet, src, dest, {0, 0}, 0, WHITE);
    }
};

class Apple : public GoodFood{
public: 
    Apple(Vector2 position, float speed) {
        this->position = position;
        this->size = {80.0f, 80.0f};
        this->nutritionalValue = 40.0f;
        this->spriteSheetIndex = 1;
        this->velocityY = speed * DEFAULT_FPS;
    }

    void draw() override {
        Rectangle src = {spriteSheetIndex * size.x, 0, size.x, size.y};
        Rectangle dest = {position.x, position.y, size.x, size.y};
        DrawTexturePro(textureGoodFoodSpriteSheet, src, dest, {0, 0}, 0, WHITE);
    }
};

class Banana : public GoodFood{
public: 
    Banana(Vector2 position, float speed) {
        this->position = position;
        this->size = {80.0f, 80.0f};
        this->nutritionalValue = 45.0f;
        this->spriteSheetIndex = 2;
        this->velocityY = speed * DEFAULT_FPS;
    }

    void draw() override {
        Rectangle src = {spriteSheetIndex * size.x, 0, size.x, size.y};
        Rectangle dest = {position.x, position.y, size.x, size.y};
        DrawTexturePro(textureGoodFoodSpriteSheet, src, dest, {0, 0}, 0, WHITE);
    }
};

class Pizza : public GoodFood{
public: 
    Pizza(Vector2 position, float speed) {
        this->position = position;
        this->size = {80.0f, 80.0f};
        this->nutritionalValue = 50.0f;
        this->spriteSheetIndex = 3;
        this->velocityY = speed * DEFAULT_FPS;
    }

    void draw() override {
        Rectangle src = {spriteSheetIndex * size.x, 0, size.x, size.y};
        Rectangle dest = {position.x, position.y, size.x, size.y};
        DrawTexturePro(textureGoodFoodSpriteSheet, src, dest, {0, 0}, 0, WHITE);
    }
};

class Yoghurt : public GoodFood{
public: 
    Yoghurt(Vector2 position, float speed) {
        this->position = position;
        this->size = {80.0f, 80.0f};
        this->nutritionalValue = 55.0f;
        this->spriteSheetIndex = 4;
        this->velocityY = speed * DEFAULT_FPS;
    }

    void draw() override {
        Rectangle src = {spriteSheetIndex * size.x, 0, size.x, size.y};
        Rectangle dest = {position.x, position.y, size.x, size.y};
        DrawTexturePro(textureGoodFoodSpriteSheet, src, dest, {0, 0}, 0, WHITE);
    }
};

class Potion : public GoodFood{
public: 
    Potion(Vector2 position, float speed) {
        this->position = position;
        this->size = {80.0f, 80.0f};
        this->nutritionalValue = 100.0f;
        this->spriteSheetIndex = 5;
        this->velocityY = speed * DEFAULT_FPS;
    }

    void draw() override {
        Rectangle src = {spriteSheetIndex * size.x, 0, size.x, size.y};
        Rectangle dest = {position.x, position.y, size.x, size.y};
        DrawTexturePro(textureGoodFoodSpriteSheet, src, dest, {0, 0}, 0, WHITE);
    }
};

class BadFood {
public: 
    Vector2 position;
    Vector2 size;
    float velocityY = 6.0f * DEFAULT_FPS;
    float harmValue;
    int spriteSheetIndex;

    bool shouldBeDestroyed = false;
    bool isLeftOfPlayer;

    BadFood() {};

    virtual void draw() = 0;
    void update(double dt, double timeElapsed) {
        position.y += velocityY * dt;
        if (position.y > WINDOW_HEIGHT) shouldBeDestroyed = true;
    }
    void drawDebugLines() {
        DrawRectangleLinesEx({position.x, position.y, size.x, size.y}, 2, RED);
        DrawCircleV(position, 3, BLUE);
    }
};

class SpoiltCheese : public BadFood{
public: 
    SpoiltCheese(Vector2 position, float speed) {
        this->position = position;
        this->size = {80.0f, 80.0f};
        this->harmValue = 30.0f;
        this->spriteSheetIndex = 0;
        this->velocityY = speed * DEFAULT_FPS;
    }

    void draw() override {
        Rectangle src = {spriteSheetIndex * size.x, 0, size.x, size.y};
        Rectangle dest = {position.x, position.y, size.x, size.y};
        DrawTexturePro(textureBadFoodSpriteSheet, src, dest, {0, 0}, 0, WHITE);
    }
};

class SpoiltApple : public BadFood{
public: 
    SpoiltApple(Vector2 position, float speed) {
        this->position = position;
        this->size = {80.0f, 80.0f};
        this->harmValue = 40.0f;
        this->spriteSheetIndex = 1;
        this->velocityY = speed * DEFAULT_FPS;
    }

    void draw() override {
        Rectangle src = {spriteSheetIndex * size.x, 0, size.x, size.y};
        Rectangle dest = {position.x, position.y, size.x, size.y};
        DrawTexturePro(textureBadFoodSpriteSheet, src, dest, {0, 0}, 0, WHITE);
    }
};

class SpoiltBanana : public BadFood{
public: 
    SpoiltBanana(Vector2 position, float speed) {
        this->position = position;
        this->size = {80.0f, 80.0f};
        this->harmValue = 45.0f;
        this->spriteSheetIndex = 2;
        this->velocityY = speed * DEFAULT_FPS;
    }

    void draw() override {
        Rectangle src = {spriteSheetIndex * size.x, 0, size.x, size.y};
        Rectangle dest = {position.x, position.y, size.x, size.y};
        DrawTexturePro(textureBadFoodSpriteSheet, src, dest, {0, 0}, 0, WHITE);
    }
};

class SpoiltPizza : public BadFood{
public: 
    SpoiltPizza(Vector2 position, float speed) {
        this->position = position;
        this->size = {80.0f, 80.0f};
        this->harmValue = 50.0f;
        this->spriteSheetIndex = 3;
        this->velocityY = speed * DEFAULT_FPS;
    }

    void draw() override {
        Rectangle src = {spriteSheetIndex * size.x, 0, size.x, size.y};
        Rectangle dest = {position.x, position.y, size.x, size.y};
        DrawTexturePro(textureBadFoodSpriteSheet, src, dest, {0, 0}, 0, WHITE);
    }
};

class SpoiltYoghurt : public BadFood{
public: 
    SpoiltYoghurt(Vector2 position, float speed) {
        this->position = position;
        this->size = {80.0f, 80.0f};
        this->harmValue = 55.0f;
        this->spriteSheetIndex = 4;
        this->velocityY = speed * DEFAULT_FPS;
    }

    void draw() override {
        Rectangle src = {spriteSheetIndex * size.x, 0, size.x, size.y};
        Rectangle dest = {position.x, position.y, size.x, size.y};
        DrawTexturePro(textureBadFoodSpriteSheet, src, dest, {0, 0}, 0, WHITE);
    }
};

class SpoiltPotion : public BadFood{
public: 
    SpoiltPotion(Vector2 position, float speed) {
        this->position = position;
        this->size = {80.0f, 80.0f};
        this->harmValue = 100.0f;
        this->spriteSheetIndex = 5;
        this->velocityY = speed * DEFAULT_FPS;
    }

    void draw() override {
        Rectangle src = {spriteSheetIndex * size.x, 0, size.x, size.y};
        Rectangle dest = {position.x, position.y, size.x, size.y};
        DrawTexturePro(textureBadFoodSpriteSheet, src, dest, {0, 0}, 0, WHITE);
    }
};

class Game {
public:
    Player player;
    std::unique_ptr<Bow> playerBow;
    std::vector<std::unique_ptr<GoodFood>> goodFoods;
    std::vector<std::unique_ptr<BadFood>> badFoods;
    std::vector<Projectile> projectiles;
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<std::unique_ptr<BroccoliBuddy>> broccoliBuddies;
    std::vector<Coin> coins;
    double dt = 0;
    double timeElapsed = 0;
    double collectingTimeElapsed = 0;
    double fightingTimeElapsed = 0;
    int spawnNumber = 1;
    int numFoods = 5;
    int terrainSpriteSheetIndex = 0;
    int maxTerrainSprites;
    int groundSpriteSheetIndex = 0;
    int maxGroundSprites;

    double spawnTimer = 0.0f;
    double spawnInterval = 2.0f;

    bool shouldSpawnFood = false;
    bool isPaused = false;
    bool isDebugging = false;

    enum class GameState {
        TITLE_SCREEN,
        COLLECTING_FOOD,
        FIGHTING,
    };

    GameState gameState = GameState::TITLE_SCREEN;
    int gameStateIndex = 0;
    int prevGameStateIndex = 0;
    int numEnemiesToSpawn = 5;

    Rectangle startButtonBounds = {380, 555, 240, 100};
    Rectangle pausePlayButtonBounds = {30, WINDOW_HEIGHT - 80, 50, 50};
    Rectangle gameModeMenuBounds = {700, 650, 200, 50};
    Rectangle changeBackgroundButtonBounds = {WINDOW_WIDTH - 250.0f, WINDOW_HEIGHT - 150.0f, 200.0f, 40.0f};
    Rectangle titleScreenButtonBounds = {WINDOW_WIDTH - 250.0f, WINDOW_HEIGHT - 90.0f, 200.0f, 40.0f};

    Vector2 enemySpawnPositions[5] = {
        {-100.0f, 200.0f}, {-100.0f, WINDOW_HEIGHT - 200.0f},
        {WINDOW_WIDTH + 100.0f, 200.0f}, {WINDOW_WIDTH + 100.0f, WINDOW_HEIGHT - 200.0f},
        {500.0f, WINDOW_HEIGHT + 100.0f}
    };

    Game() {
        SetRandomSeed(static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
        InitAudioDevice();
        loadMedia();
        PlayMusicStream(musicCollectingBackground);
        PlayMusicStream(musicFightingBackground);
        player = Player();
        playerBow = std::make_unique<Bow>(&player.position, nullptr, true, false, &player.level);

        maxTerrainSprites = textureTerrainSpriteSheet.width / 1000;
        maxGroundSprites = textureGroundSpriteSheet.width / 1000;
        setGuiStyles();
    }

    void draw() {
        if (gameState == GameState::TITLE_SCREEN) {
            DrawTexture(textureTitleScreen, 0, 0, WHITE);
            GuiComboBox(gameModeMenuBounds, "Collect;Fighting", &gameStateIndex);
            if (prevGameStateIndex != gameStateIndex) {
                PlaySound(soundClick);
                prevGameStateIndex = gameStateIndex;
            }
        } else if (gameState == GameState::COLLECTING_FOOD) {
            Rectangle src = {terrainSpriteSheetIndex * 1000.0f, 0, 1000, 800};
            Rectangle dest = {0, 0, 1000, 800};
            DrawTexturePro(textureTerrainSpriteSheet, src, dest, {0, 0}, 0, WHITE);
            for (auto &goodFood: goodFoods) {
                goodFood->draw();
                if (isDebugging) goodFood->drawDebugLines();
            }
            for (auto &badFood: badFoods) {
                badFood->draw();
                if (isDebugging) badFood->drawDebugLines();
            }
            player.draw(gameStateIndex);
            if (isDebugging) player.drawDebugLines();
            DrawTextEx(font, TextFormat("Nutrition: %.0f", player.nutrition), {40, 40}, 35, 2, terrainSpriteSheetIndex == 4 ? WHITE : BLACK);
            
            float levelMeter = (collectingTimeElapsed - spawnTimer) / spawnInterval * 250.0f;

            DrawRectangleV({WINDOW_WIDTH - 300.0f, 40.0f}, {250.0f, 50.0f}, GRAY);
            DrawRectangleGradientV(WINDOW_WIDTH - 300.0f, 40.0f, levelMeter, 50.0f, BLUE, {0, 255, 255, 255});
            DrawRectangleLinesEx({WINDOW_WIDTH - 300.0f, 40.0f, 250.0f, 50.0f}, 3, BLACK);
            DrawTextEx(font, "Next Spawn", {WINDOW_WIDTH - 280.0f, 55.0f}, 30.0f, 1.5f, BLACK);

            if (player.nutrition < 1000) GuiDisable();
            if (GuiButton({WINDOW_WIDTH - 300.0f, 120.0f, 150.0f, 50.0f}, "Purchase")) {
                PlaySound(soundKaching);
                player.nutrition -= 1000.0f;
                player.attractionTimer = collectingTimeElapsed;
                player.isAttracting = true;
            }
            GuiEnable();

            if (player.isAttracting) {
                float remaining = player.powerUpDuration - (collectingTimeElapsed - player.attractionTimer);
                levelMeter = (remaining / player.powerUpDuration) * 250.0f;
                levelMeter = Clamp(levelMeter, 0.0f, 250.0f);
                DrawRectangleV({40.0f, 300.0f}, {250.0f, 25.0f}, GRAY);
                DrawRectangleGradientV(40.0f, 300.0f, levelMeter, 25.0f, GREEN, DARKGREEN);
                DrawRectangleLinesEx({40.0f, 300.0f, 250.0f, 25.0f}, 2.0f, BLACK);
                DrawTextEx(font, "Attraction Timer", {50.0f, 302.0f}, 25.0f, 0.0f, BLACK);
            }
        } else if (gameState == GameState::FIGHTING) {
            Rectangle src = {groundSpriteSheetIndex * 1000.0f, 0, 1000, 800};
            Rectangle dest = {0, 0, 1000, 800};
            DrawTexturePro(textureGroundSpriteSheet, src, dest, {0, 0}, 0, WHITE);
            
            int integerLevel = static_cast<int>(player.level);
            float levelMeter = (player.level - integerLevel) * 250.0f;
            
            player.draw(gameStateIndex);
            if (isDebugging) player.drawDebugLines();
            playerBow->draw();
            if (isDebugging) playerBow->drawDebugLines();
            for (auto &enemy: enemies) {
                enemy->draw();
                if (isDebugging) enemy->drawDebugLines();
                enemy->associatedBow->draw();
                if (isDebugging) enemy->associatedBow->drawDebugLines();
            }
            for (auto &broccoliBuddy: broccoliBuddies) {
                broccoliBuddy->draw(fightingTimeElapsed);
                if (isDebugging) broccoliBuddy->drawDebugLines();
                broccoliBuddy->associatedBow->draw();
                if (isDebugging) broccoliBuddy->associatedBow->drawDebugLines();
            }
            for (auto &projectile: projectiles) {
                projectile.draw();
                if (isDebugging) projectile.drawDebugLines();
            }
            for (auto &coin: coins) {
                coin.draw();
                if (isDebugging) coin.drawDebugLines();
            }
            DrawTextEx(font, TextFormat("Nutrition: %.0f", player.nutrition), {50, 110}, 35, 2, BLACK);
            DrawTextEx(font, TextFormat("Coins: %i", player.coins), {WINDOW_WIDTH - 500.0f, 120.0f}, 35.0f, 2, BLACK);

            DrawRectangleV({WINDOW_WIDTH - 300.0f, 40.0f}, {250.0f, 50.0f}, GRAY);
            DrawRectangleGradientV(WINDOW_WIDTH - 300.0f, 40.0f, levelMeter, 50.0f, ORANGE, YELLOW);
            DrawRectangleLinesEx({WINDOW_WIDTH - 300.0f, 40.0f, 250.0f, 50.0f}, 3, BLACK);
            DrawTextEx(font, TextFormat("Level: %d", integerLevel), {WINDOW_WIDTH - 280.0f, 55.0f}, 30.0f, 1.5f, BLACK);

            levelMeter = player.health / player.maxHealth * 300.0f;
            DrawRectangleV({50.0f, 40.0f}, {300.0f, 50.0f}, GRAY);
            DrawRectangleGradientV(50.0f, 40.0f, levelMeter, 50.0f, RED, MAROON);
            DrawRectangleLinesEx({50.0f, 40.0f, 300.0f, 50.0f}, 3, BLACK);
            DrawTextEx(font, TextFormat("Health: %d", (int)player.health), {70.0f, 55.0f}, 30.0f, 1.5f, BLACK);

            if (player.nutrition <= 0) {
                GuiDisable();
            }
            if (GuiButton({WINDOW_WIDTH - 300.0f, 120.0f, 150.0f, 50.0f}, "Level Up")) {
                PlaySound(soundLevelUp);
                player.level += player.nutrition / 500.0f;
                player.nutrition = 0;
            }
            GuiEnable();

            if (player.coins < 15) GuiDisable();
            if (GuiButton({WINDOW_WIDTH - 500.0f, 40.0f, 150.0f, 50.0f}, "Purchase")) {
                PlaySound(soundKaching);
                player.coins -= 15;
                if (GetRandomValue(0, 1)) {
                    player.isExtraFast = true;
                    player.speedTimer = fightingTimeElapsed;
                } else {
                    player.isImmune = true;
                    player.immunityTimer = fightingTimeElapsed;
                }
            }
            GuiEnable();

            if (player.coins < 20) GuiDisable();
            if (GuiButton({50.0f, 180.0f, 300.0f, 35.0f}, "Buy Broccoli Buddy")) {
                player.coins -= 20;
                PlaySound(soundKaching);
                spawnBroccoliBuddy();
            }
            GuiEnable();

            if (player.isExtraFast) {
                float remaining = player.powerUpDuration - (fightingTimeElapsed - player.speedTimer);
                levelMeter = (remaining / player.powerUpDuration) * 250.0f;
                levelMeter = Clamp(levelMeter, 0.0f, 250.0f);
                DrawRectangleV({40.0f, 300.0f}, {250.0f, 25.0f}, GRAY);
                DrawRectangleGradientV(40.0f, 300.0f, levelMeter, 25.0f, GREEN, DARKGREEN);
                DrawRectangleLinesEx({40.0f, 300.0f, 250.0f, 25.0f}, 2.0f, BLACK);
                DrawTextEx(font, "Speed Timer", {50.0f, 302.0f}, 25.0f, 0.0f, BLACK);
            }
            if (player.isImmune) {
                float remaining = player.powerUpDuration - (fightingTimeElapsed - player.immunityTimer);
                levelMeter = (remaining / player.powerUpDuration) * 250.0f;
                levelMeter = Clamp(levelMeter, 0.0f, 250.0f);
                DrawRectangleV({40.0f, 350.0f}, {250.0f, 25.0f}, GRAY);
                DrawRectangleGradientV(40.0f, 350.0f, levelMeter, 25.0f, GREEN, DARKGREEN);
                DrawRectangleLinesEx({40.0f, 350.0f, 250.0f, 25.0f}, 2.0f, BLACK);
                DrawTextEx(font, "Immunity Timer", {50.0f, 352.0f}, 25.0f, 0.0f, BLACK);
            }
        }
        if (gameState != GameState::TITLE_SCREEN) {
            int pausePlayButtonIndex = isPaused ? 1 : 0;
            Rectangle src = {pausePlayButtonIndex * pausePlayButtonBounds.width, 0, pausePlayButtonBounds.width, pausePlayButtonBounds.height};
            Rectangle dest = {pausePlayButtonBounds.x, pausePlayButtonBounds.y, pausePlayButtonBounds.width, pausePlayButtonBounds.height};
            DrawTexturePro(texturePausePlayButtonSpriteSheet, src, dest, {0, 0}, 0, WHITE);

            if (GuiButton(titleScreenButtonBounds, "Title Screen")) {
                PlaySound(soundClick);
                gameState = GameState::TITLE_SCREEN;
            }
            if (GuiButton(changeBackgroundButtonBounds, "Change BG")) {
                PlaySound(soundClick);
                if (gameState == GameState::COLLECTING_FOOD) {
                    terrainSpriteSheetIndex = (terrainSpriteSheetIndex + 1) % maxTerrainSprites;
                } else if (gameState == GameState::FIGHTING) {
                    groundSpriteSheetIndex = (groundSpriteSheetIndex + 1) % maxGroundSprites;
                }
            }
        }
    }
    void update() {
        Vector2 mousePos = GetMousePosition();

        if (!isPaused) {
            dt = GetFrameTime();
            timeElapsed += dt;
            if (IsKeyPressed(KEY_TAB)) {
                isDebugging = !isDebugging;
            }
        }

        if (gameState == GameState::TITLE_SCREEN) {
            if (CheckCollisionPointRec(mousePos, startButtonBounds) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                PlaySound(soundClick);
                gameState = static_cast<GameState>(gameStateIndex + 1);
            }
        } else if (gameState == GameState::COLLECTING_FOOD && !isPaused) {
            collectingTimeElapsed += dt;
            UpdateMusicStream(musicCollectingBackground);

            if (collectingTimeElapsed - spawnTimer >= spawnInterval) {
                shouldSpawnFood = true;
                spawnTimer = collectingTimeElapsed;
            }
            if (IsKeyPressed(KEY_G)) {
                terrainSpriteSheetIndex = (terrainSpriteSheetIndex + 1) % maxTerrainSprites;
            }
            
            if (shouldSpawnFood) {
                spawnFood();
                shouldSpawnFood = false;
            }
            if (player.nutrition > 5000) {
                spawnNumber = 5;
            } else if (player.nutrition > 2000) {
                spawnNumber = 4;
            } else if (player.nutrition > 800) {
                spawnNumber = 3;
            } else if (player.nutrition > 400) {
                spawnNumber = 2;
            } else if (player.nutrition >= 0) {
                spawnNumber = 1;
            }

            if (player.isAttracting) {
                for (auto &food: goodFoods) {
                    if (food->isLeftOfPlayer) {
                        food->position.x += 1.0f * (DEFAULT_FPS * dt);
                        if (food->position.x + food->size.x / 2 > player.position.x + player.size.x / 2) {
                            food->position.x--;
                        }
                    } else {
                        food->position.x -= 1.0f * (DEFAULT_FPS * dt);
                        if (food->position.x + food->size.x / 2 < player.position.x + player.size.x / 2) {
                            food->position.x++;
                        }
                    }
                }
                for (auto &food: badFoods) {
                    if (food->isLeftOfPlayer) {
                        food->position.x -= 1.0f * (DEFAULT_FPS * dt);
                    } else {
                        food->position.x += 1.0f * (DEFAULT_FPS * dt);
                    }
                }

                if (collectingTimeElapsed - player.attractionTimer >= player.powerUpDuration) {
                    player.isAttracting = false;
                }
            }
    
            spawnInterval = std::max(2.0f - player.nutrition / 3500.0f, 1.2f);
    
            for (auto &goodFood: goodFoods) {
                goodFood->update(dt, timeElapsed);
                bool isLeftOfPlayer = goodFood->position.x + goodFood->size.x / 2 <= player.position.x + player.size.x / 2;
                goodFood->isLeftOfPlayer = isLeftOfPlayer;
            }
            for (auto &badFood: badFoods) {
                badFood->update(dt, timeElapsed);
                bool isLeftOfPlayer = badFood->position.x + badFood->size.x / 2 <= player.position.x + player.size.x / 2;
                badFood->isLeftOfPlayer = isLeftOfPlayer;
            }
        } else if (gameState == GameState::FIGHTING && !isPaused) {
            fightingTimeElapsed += dt;
            UpdateMusicStream(musicFightingBackground);

            playerBow->update();
            if (enemies.empty()) {
                spawnEnemies();
                player.health = player.maxHealth;
                projectiles.clear();
            }
            if (IsKeyPressed(KEY_G)) {
                groundSpriteSheetIndex = (groundSpriteSheetIndex + 1) % maxGroundSprites;
            }
            if (playerBow->shouldShoot) {
                projectiles.push_back(Projectile(playerBow->position, true, playerBow->angleDeg));
                PlaySound(soundShoot);
                playerBow->shouldShoot = false;
            }
            for (auto &projectile: projectiles) {
                projectile.update(dt);
            }
            for (auto &enemy: enemies) {
                enemy->update(dt, fightingTimeElapsed);
                enemy->associatedBow->update();
                if (enemy->shouldShoot) {
                    projectiles.push_back(Projectile(enemy->associatedBow->position, false, enemy->associatedBow->angleDeg));
                    PlaySound(soundShoot);
                    enemy->shouldShoot = false;
                }
            }
            for (auto &broccoliBuddy: broccoliBuddies) {
                broccoliBuddy->update(dt, fightingTimeElapsed);
                broccoliBuddy->associatedBow->update();
                if (broccoliBuddy->associatedBow->shouldShoot) {
                    projectiles.push_back(Projectile(broccoliBuddy->associatedBow->position, true, broccoliBuddy->associatedBow->angleDeg));
                    PlaySound(soundShoot);
                    broccoliBuddy->associatedBow->shouldShoot = false;
                }
            }

            if (player.isExtraFast) {
                if (fightingTimeElapsed - player.speedTimer >= player.powerUpDuration) {
                    player.isExtraFast = false;
                }
            }
            if (player.isImmune) {
                if (fightingTimeElapsed - player.immunityTimer >= player.powerUpDuration) {
                    player.isImmune = false;
                }
            }
        }
        if (gameState != GameState::TITLE_SCREEN) {
            if (CheckCollisionPointRec(mousePos, pausePlayButtonBounds) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                PlaySound(soundClick);
                isPaused = !isPaused;
            }
            if (IsKeyPressed(KEY_T)) {
                gameState = GameState::TITLE_SCREEN;
                isPaused = false;
            }
            if (!isPaused) player.update(dt, timeElapsed, gameStateIndex);
            if (player.isDead) {
                reset();
                PlaySound(soundFail);
            }

            checkForRemoval();
            garbageCollect();
            checkForCollisions();
        }
    }
    void checkForRemoval() {
        if (gameState == GameState::COLLECTING_FOOD) {
            for (auto &goodFood: goodFoods) {
                if (goodFood->position.y > WINDOW_HEIGHT) {
                    goodFood->shouldBeDestroyed = true;
                }
            }
            for (auto &badFood: badFoods) {
                if (badFood->position.y > WINDOW_HEIGHT) {
                    badFood->shouldBeDestroyed = true;
                }
            }
        } else if (gameState == GameState::FIGHTING) {
            for (auto &projectile: projectiles) {
                if (projectile.position.x - 100 > WINDOW_WIDTH || projectile.position.x + 100 < 0 || projectile.position.y - 100 > WINDOW_HEIGHT || projectile.position.y + 100 < 0) {
                    projectile.shouldBeDestroyed = true;
                }
            }
        }
    }
    void garbageCollect() {
        if (gameState == GameState::COLLECTING_FOOD) {
            for (int i = 0; i < goodFoods.size(); i++) {
                if (goodFoods.at(i)->shouldBeDestroyed) {
                    goodFoods.erase(goodFoods.begin() + i);
                    i--;
                }
            }
            for (int i = 0; i < badFoods.size(); i++) {
                if (badFoods.at(i)->shouldBeDestroyed) {
                    badFoods.erase(badFoods.begin() + i);
                    i--;
                }
            }
        } else if (gameState == GameState::FIGHTING) {
            for (int i = 0; i < projectiles.size(); i++) {
                if (projectiles.at(i).shouldBeDestroyed) {
                    projectiles.erase(projectiles.begin() + i);
                    i--;
                }
            }
            for (int i = 0; i < enemies.size(); i++) {
                if (enemies.at(i)->isDead) {
                    coins.push_back(Coin({enemies.at(i)->position.x + 60.0f, enemies.at(i)->position.y + 140.0f}));
                    enemies.erase(enemies.begin() + i);
                    i--;
                }
            }
            for (int i = 0; i < coins.size(); i++) {
                if (coins.at(i).shouldBeDestroyed) {
                    coins.erase(coins.begin() + i);
                    i--;
                }
            }
            for (int i = 0; i < broccoliBuddies.size(); i++) {
                if (broccoliBuddies.at(i)->shouldBeDestroyed) {
                    broccoliBuddies.erase(broccoliBuddies.begin() + i);
                    i--;
                }
            }
        }
    }
    void checkForCollisions() {
        if (gameState == GameState::COLLECTING_FOOD) {
            for (auto &goodFood: goodFoods) {
                Vector2 goodFoodPosition = goodFood->position;
                Vector2 goodFoodSize = goodFood->size;
                if (CheckCollisionRecs({goodFoodPosition.x, goodFoodPosition.y, goodFoodSize.x, goodFoodSize.y}, {player.position.x, player.position.y, player.size.x, player.size.y})) {
                    player.nutrition += goodFood->nutritionalValue;
                    PlaySound(soundBite);
                    goodFood->shouldBeDestroyed = true;
                }
            }
            for (auto &badFood: badFoods) {
                Vector2 badFoodPosition = badFood->position;
                Vector2 badFoodSize = badFood->size;
                if (CheckCollisionRecs({badFoodPosition.x, badFoodPosition.y, badFoodSize.x, badFoodSize.y}, {player.position.x, player.position.y, player.size.x, player.size.y})) {
                    player.nutrition -= badFood->harmValue;
                    PlaySound(soundBite);
                    badFood->shouldBeDestroyed = true;
                }
            }
        } else if (gameState == GameState::FIGHTING) {
            for (auto &projectile: projectiles) {
                if (Collision::CheckCollisionRectCornersRec({player.position.x, player.position.y, player.size.x, player.size.y}, projectile.rectCorners) && !projectile.isPlayerProjectile) {
                    int damage = GetRandomValue(5, 10);
                    if (!player.isImmune) player.health -= damage;
                    PlaySound(soundHit);
                    projectile.shouldBeDestroyed = true;
                }
            }
            for (auto &enemy: enemies) {
                for (auto &projectile: projectiles) {
                    if (Collision::CheckCollisionRectCornersRec({enemy->position.x, enemy->position.y, enemy->size.x, enemy->size.y}, projectile.rectCorners) && projectile.isPlayerProjectile) {
                        enemy->health -= playerBow->damage;
                        PlaySound(soundHit);
                        projectile.shouldBeDestroyed = true;
                    }
                }
            }
            for (auto &coin: coins) {
                if (CheckCollisionRecs({player.position.x, player.position.y, player.size.x, player.size.y}, {coin.position.x, coin.position.y, coin.size.x, coin.size.y})) {
                    PlaySound(soundCollect);
                    player.coins++;
                    coin.shouldBeDestroyed = true;
                }
            }
        }
    }
    void spawnFood() {
        for (int i = 0; i < spawnNumber; i++) {
            Vector2 spawnPos = {(float)GetRandomValue(100, WINDOW_WIDTH - 100), -200.0f};
            if (GetRandomValue(1, 2) == 1) {
                int goodFoodIndex = GetRandomValue(1, 6);
                switch (goodFoodIndex) {
                    case 1:
                        goodFoods.push_back(std::make_unique<Cheese>(spawnPos, 6.0f));
                        break;
                    case 2:
                        goodFoods.push_back(std::make_unique<Apple>(spawnPos, 6.0f));
                        break;
                    case 3:
                        goodFoods.push_back(std::make_unique<Banana>(spawnPos, 6.0f));
                        break;
                    case 4:
                        goodFoods.push_back(std::make_unique<Pizza>(spawnPos, 6.0f));
                        break;
                    case 5:
                        goodFoods.push_back(std::make_unique<Yoghurt>(spawnPos, 6.0f));
                        break;
                    case 6:
                        goodFoods.push_back(std::make_unique<Potion>(spawnPos, 6.0f));
                        break;
                }
            } else {
                int badFoodIndex = GetRandomValue(1, 6);
                switch (badFoodIndex) {
                    case 1:
                        badFoods.push_back(std::make_unique<SpoiltCheese>(spawnPos, 6.0f));
                        break;
                    case 2:
                        badFoods.push_back(std::make_unique<SpoiltApple>(spawnPos, 6.0f));
                        break;
                    case 3:
                        badFoods.push_back(std::make_unique<SpoiltBanana>(spawnPos, 6.0f));
                        break;
                    case 4:
                        badFoods.push_back(std::make_unique<SpoiltPizza>(spawnPos, 6.0f));
                        break;
                    case 5:
                        badFoods.push_back(std::make_unique<SpoiltYoghurt>(spawnPos, 6.0f));
                        break;
                    case 6:
                        badFoods.push_back(std::make_unique<SpoiltPotion>(spawnPos, 6.0f));
                        break;
                }
            }
        }
    }
    void spawnEnemies() {
        for (int i = 0; i < numEnemiesToSpawn; i++) {
            std::unique_ptr<Enemy> enemy = std::make_unique<Enemy>(enemySpawnPositions[i], &player.position);
            std::unique_ptr<Bow> enemyBow = std::make_unique<Bow>(&enemy->position, &player.center, false, false, nullptr);
            enemy->makeAssociatedBow(std::move(enemyBow));
            enemies.push_back(std::move(enemy));
        }
    }
    void spawnBroccoliBuddy() {
        Vector2 spawnPosition = {(float)GetRandomValue(100, WINDOW_WIDTH - 200), -200.0f};
        std::unique_ptr<BroccoliBuddy> broccoliBuddy = std::make_unique<BroccoliBuddy>(spawnPosition, &enemies);
        std::unique_ptr<Bow> broccoliBow = std::make_unique<Bow>(&broccoliBuddy->position, nullptr, false, true, &player.level);
        broccoliBuddy->makeAssociatedBow(std::move(broccoliBow));
        broccoliBuddies.push_back(std::move(broccoliBuddy));
    }
    void setGuiStyles() {
        GuiSetFont(font);
        GuiSetStyle(DEFAULT, TEXT_SIZE, 25);
        GuiSetStyle(COMBOBOX, COMBO_BUTTON_WIDTH, 50);
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt({189, 109, 30, 255}));
        GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, ColorToInt({189, 109, 30, 255}));
        GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, ColorToInt(RED));
        RayGuiTools::SetAllComboBoxTextStyles(ColorToInt(WHITE));
        RayGuiTools::SetAllComboBoxBorderStyles(ColorToInt(BLACK));
        GuiSetStyle(COMBOBOX, BASE_COLOR_NORMAL, ColorToInt({189, 109, 30, 255}));
        GuiSetStyle(COMBOBOX, BASE_COLOR_FOCUSED, ColorToInt({189, 109, 30, 255}));
        GuiSetStyle(COMBOBOX, BASE_COLOR_PRESSED, ColorToInt(RED));
        RayGuiTools::SetAllButtonTextStyles(ColorToInt(WHITE));
        RayGuiTools::SetAllButtonBorderStyles(ColorToInt(BLACK));
    }
    void reset() {
        player.health = player.maxHealth;
        player.isDead = false;
        player.coins = 0;
        player.nutrition = 100;
        player.isAttracting = false;
        player.isExtraFast = false;
        player.isImmune = false;
        playerBow->damage = playerBow->baseDamage;
        player.level = 1;
        projectiles.clear();
        enemies.clear();
        goodFoods.clear();
        badFoods.clear();
        coins.clear();
        broccoliBuddies.clear();
        gameState = GameState::TITLE_SCREEN;
        gameStateIndex = 0;
    }
};

int main() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Falling Feast");
    SetTargetFPS(FPS);
    Game game = Game();

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        game.update();
        game.draw();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}