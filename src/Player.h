#ifndef PLAYER_H
#define PLAYER_H

#include <raylib.h>
#include <string>
#include "AnimData.h"

class Player
{
public:
    Player(const std::string &carStraightTexPath, const std::string &carLeftTexPath, const std::string &carRightTexPath);
    ~Player();

    void HandleInput(float frameTime);
    void Update(float frameTime);
    void Draw();

    Vector2 GetPosition() const;
    Rectangle GetRect() const;
    void TakeDamage(int amount);
    void Heal(int amount);
    int GetHealth() const;

private:
    Texture2D texStraight;
    Texture2D texLeft;
    Texture2D texRight;

    Vector2 position;
    float speed;
    int health;

    // Animation data for each car state
    AnimData animDataStraight;
    AnimData animDataLeft;
    AnimData animDataRight;

    // Bounds for movement
    float leftBound;
    float rightBound;

    AnimData updateAnimData(AnimData data, float frameTime, int maxFrame);
};

#endif // PLAYER_H
