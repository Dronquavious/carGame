#include "Player.h"

Player::Player(const std::string &carStraightTexPath, const std::string &carLeftTexPath, const std::string &carRightTexPath)
{
    texStraight = LoadTexture(carStraightTexPath.c_str());
    texLeft = LoadTexture(carLeftTexPath.c_str());
    texRight = LoadTexture(carRightTexPath.c_str());

    position = {(float)GetScreenWidth() / 2, 576};
    speed = 200.0f;
    health = 100;

    leftBound = 180.0f;
    rightBound = 1034.0f;

    // Initialize animation data
    animDataStraight.rec = {0, 0, (float)texStraight.width, (float)texStraight.height};
    animDataStraight.pos = position;
    animDataStraight.frame = 0;
    animDataStraight.updateTime = 1.0f / 12.0f;
    animDataStraight.runningTime = 0.0f;

    animDataLeft.rec = {0, 0, (float)texLeft.width / 4, (float)texLeft.height};
    animDataLeft.pos = position;
    animDataLeft.frame = 0;
    animDataLeft.updateTime = 1.0f / 12.0f;
    animDataLeft.runningTime = 0.0f;

    animDataRight.rec = {0, 0, (float)texRight.width / 3, (float)texRight.height};
    animDataRight.pos = position;
    animDataRight.frame = 0;
    animDataRight.updateTime = 1.0f / 12.0f;
    animDataRight.runningTime = 0.0f;
}

Player::~Player()
{
    UnloadTexture(texStraight);
    UnloadTexture(texLeft);
    UnloadTexture(texRight);
}

void Player::HandleInput(float frameTime)
{
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
    {
        if (position.x > leftBound)
        {
            position.x -= speed * frameTime;
        }
    }
    else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
    {
        if (position.x < rightBound)
        {
            position.x += speed * frameTime;
        }
    }
}

void Player::Update(float frameTime)
{
    animDataStraight = updateAnimData(animDataStraight, frameTime, 6);
    animDataLeft = updateAnimData(animDataLeft, frameTime, 1);
    animDataRight = updateAnimData(animDataRight, frameTime, 1);

    animDataStraight.pos = position;
    animDataLeft.pos = position;
    animDataRight.pos = position;
}

void Player::Draw()
{
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
    {
        DrawTextureRec(texLeft, animDataLeft.rec, position, WHITE);
    }
    else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
    {
        DrawTextureRec(texRight, animDataRight.rec, position, WHITE);
    }
    else
    {
        DrawTextureRec(texStraight, animDataStraight.rec, position, WHITE);
    }
}

Vector2 Player::GetPosition() const
{
    return position;
}

Rectangle Player::GetRect() const
{
    return {position.x, position.y, animDataStraight.rec.width, animDataStraight.rec.height};
}

void Player::TakeDamage(int amount)
{
    health -= amount;
    if (health < 0)
    {
        health = 0;
    }
}

void Player::Heal(int amount)
{
    health += amount;
    if (health > 100)
    {
        health = 100;
    }
}

int Player::GetHealth() const
{
    return health;
}

AnimData Player::updateAnimData(AnimData data, float frameTime, int maxFrame)
{
    data.runningTime += frameTime;
    if (data.runningTime >= data.updateTime)
    {
        data.runningTime = 0.0;
        data.rec.x = data.frame * data.rec.width;
        data.frame++;
        if (data.frame > maxFrame)
        {
            data.frame = 0;
        }
    }
    return data;
}
