#include <raylib.h>
#include <string>
#include <filesystem>
#include "Player.h"
#include "AnimData.h"

// function to get the correct path for assets
std::string getAssetPath(const std::string& asset) {
    std::filesystem::path currentPath = std::filesystem::current_path();
    for (int i = 0; i < 3; ++i) { // check up to 3 levels up
        std::filesystem::path resourcesPath = currentPath / "resources";
        if (std::filesystem::exists(resourcesPath) && std::filesystem::is_directory(resourcesPath)) {
            return (resourcesPath / asset).string();
        }
        currentPath = currentPath.parent_path();
    }
    return asset; // fallback to original path
}

// game states
enum GameState
{
    GAME,
    GAME_OVER
};

// play sprite animation
AnimData updateAnimData(AnimData data, float frameTime, int maxFrame)
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

// make backround scroll down
void scrollBackground(float &bgY1, float &bgY2, float bgScrollSpeed, float frameTime, int screenHeight)
{
    bgY1 += bgScrollSpeed * frameTime;
    bgY2 += bgScrollSpeed * frameTime;

    if (bgY1 >= screenHeight)
    {
        bgY1 = bgY2 - screenHeight;
    }
    if (bgY2 >= screenHeight)
    {
        bgY2 = bgY1 - screenHeight;
    }
}

int main()
{
    int windowDimension[2] = {1280, 720};
    int windowWidth = windowDimension[0];
    int windowHeight = windowDimension[1];

    InitWindow(windowWidth, windowHeight, "CarGame");
    SetTargetFPS(60);
    srand(time(NULL)); // Seed random number generator

    InitAudioDevice();
    Sound carNoise = LoadSound(getAssetPath("sounds/carNoise.wav").c_str());
    Sound speedUpCol = LoadSound(getAssetPath("sounds/speedcol.wav").c_str());
    Sound pickUp = LoadSound(getAssetPath("sounds/pickupCoin.wav").c_str());
    Sound roadBlockcol = LoadSound(getAssetPath("sounds/roadBlockHit.wav").c_str());
    Music bgMusic = LoadMusicStream(getAssetPath("sounds/marioKart.mp3").c_str());

    // Load textures
    Texture2D gameBackround = LoadTexture(getAssetPath("images/gameBackround2.png").c_str());
    Texture2D speedUpTexture = LoadTexture(getAssetPath("images/speedUp.png").c_str());
    Texture2D roadBlockTexture = LoadTexture(getAssetPath("images/roadblock.png").c_str());
    Texture2D appleTexture = LoadTexture(getAssetPath("images/apple.png").c_str());

    Player player(getAssetPath("images/carStraight.png"), getAssetPath("images/carLeft.png"), getAssetPath("images/carRight.png"));

    // Sound settings
    SetSoundVolume(carNoise, 0.10f);
    SetSoundVolume(speedUpCol, 0.5f);
    SetSoundVolume(roadBlockcol, 0.5f);
    SetSoundVolume(pickUp, 0.5f);
    SetMusicVolume(bgMusic, 0.35f);
    PlaySound(carNoise);

    if (!IsMusicStreamPlaying(bgMusic))
    {
        PlayMusicStream(bgMusic);
    }

    // Game state variables
    int score = 0;
    GameState currentState = GAME;
    float scoreTimer = 0.0f;
    bool isPaused = false;

    // Background scrolling variables
    float bgY1 = 0.0f;
    float bgY2 = -GetScreenHeight();
    float bgScrollSpeed = 700.0f;
    float originalBgScrollSpeed = bgScrollSpeed;

    // Lane positions for spawning
    Vector2 laneSpawnPositions[] = {
        {239, -144}, {322, -144}, {397, -144}, {475, -144}, {557, -144},
        {640, -144}, {725, -144}, {810, -144}, {885, -144}, {964, -144}};

    // Object pool for dynamic spawning
    const int maxObjects = 15;
    AnimData objectPool[maxObjects] = {};

    // Spawning timer
    float spawnTimer = 0.0f;
    float spawnInterval = 0.5f; // Time between spawns

    // Speedup effect state
    float speedUpDuration = 5.0f;
    float speedUpTimer = 0.0f;
    bool isSpeedUpActive = false;

    // Game loop
    while (!WindowShouldClose())
    {
        if (!IsSoundPlaying(carNoise))
        {
            PlaySound(carNoise);
        }

        if (IsKeyPressed(KEY_P))
        {
            isPaused = !isPaused;
        }

        BeginDrawing();

        if (isPaused)
        {
            ClearBackground(BLACK);
            const char *message;
            if (player.GetHealth() <= 0)
            {
                StopMusicStream(bgMusic);
                StopSound(carNoise);
                message = "GAME OVER";
            }
            else
            {
                message = "Game Paused. Press 'P' to Resume";
            }
            int textWidth = MeasureText(message, 40);
            DrawText(message, (GetScreenWidth() - textWidth) / 2, (GetScreenHeight() - 40) / 2, 40, RED);
        }
        else
        {
            float frameTime = GetFrameTime();
            UpdateMusicStream(bgMusic);

            player.HandleInput(frameTime);
            player.Update(frameTime);

            // Scoring
            scoreTimer += frameTime;
            if (scoreTimer >= 1.0f)
            {
                score += 10;
                scoreTimer = 0.0f;
            }

            // Background scrolling
            scrollBackground(bgY1, bgY2, bgScrollSpeed, frameTime, GetScreenHeight());
            ClearBackground(WHITE);
            DrawTexture(gameBackround, 0, bgY1, WHITE);
            DrawTexture(gameBackround, 0, bgY2, WHITE);

            player.Draw();

            // --- Dynamic Object Spawning ---
            spawnTimer += frameTime;
            if (spawnTimer >= spawnInterval)
            {
                spawnTimer = 0.0f;
                spawnInterval = (float)GetRandomValue(5, 25) / 10.0f; // Randomize next spawn time

                for (int i = 0; i < maxObjects; ++i)
                {
                    if (!objectPool[i].active)
                    {
                        objectPool[i].active = true;
                        int lane = GetRandomValue(0, 9);
                        objectPool[i].pos = laneSpawnPositions[lane];
                        
                        int objectTypeRoll = GetRandomValue(1, 100);
                        if (objectTypeRoll <= 50) { // 50% chance for roadblock
                            objectPool[i].type = ROADBLOCK;
                            objectPool[i].rec = {0, 0, (float)roadBlockTexture.width, (float)roadBlockTexture.height};
                        } else if (objectTypeRoll <= 80) { // 30% chance for apple
                            objectPool[i].type = APPLE;
                            objectPool[i].rec = {0, 0, (float)appleTexture.width / 3, (float)appleTexture.height};
                            objectPool[i].updateTime = 0.1f;
                        } else { // 20% chance for speedup
                            objectPool[i].type = SPEEDUP;
                            objectPool[i].rec = {0, 0, (float)speedUpTexture.width, (float)speedUpTexture.height};
                        }
                        break; 
                    }
                }
            }

            // --- Update and Draw Objects ---
            Rectangle carRect = player.GetRect();
            for (int i = 0; i < maxObjects; ++i)
            {
                if (objectPool[i].active)
                {
                    objectPool[i].pos.y += bgScrollSpeed * frameTime;

                    // Deactivate if off-screen
                    if (objectPool[i].pos.y > windowHeight)
                    {
                        objectPool[i].active = false;
                        continue;
                    }

                    // Get object rectangle for collision
                    Rectangle objectRect = {objectPool[i].pos.x, objectPool[i].pos.y, objectPool[i].rec.width, objectPool[i].rec.height};

                    // Check for collision
                    if (CheckCollisionRecs(carRect, objectRect))
                    {
                        switch (objectPool[i].type)
                        {
                            case ROADBLOCK:
                                player.TakeDamage(10);
                                PlaySound(roadBlockcol);
                                break;
                            case APPLE:
                                player.Heal(10);
                                PlaySound(pickUp);
                                break;
                            case SPEEDUP:
                                if (!isSpeedUpActive) {
                                    bgScrollSpeed += 350;
                                }
                                isSpeedUpActive = true;
                                speedUpTimer = speedUpDuration;
                                PlaySound(speedUpCol);
                                break;
                            default: break;
                        }
                        objectPool[i].active = false; // Deactivate on collision
                    }

                    // Draw object
                    switch (objectPool[i].type)
                    {
                        case ROADBLOCK:
                            DrawTextureRec(roadBlockTexture, objectPool[i].rec, objectPool[i].pos, WHITE);
                            break;
                        case APPLE:
                            objectPool[i] = updateAnimData(objectPool[i], frameTime, 3);
                            DrawTextureRec(appleTexture, objectPool[i].rec, objectPool[i].pos, WHITE);
                            break;
                        case SPEEDUP:
                            DrawTextureRec(speedUpTexture, objectPool[i].rec, objectPool[i].pos, WHITE);
                            break;
                        default: break;
                    }
                }
            }

            // Handle speedup effect timer
            if (isSpeedUpActive)
            {
                speedUpTimer -= frameTime;
                if (speedUpTimer <= 0.0f)
                {
                    isSpeedUpActive = false;
                    bgScrollSpeed = originalBgScrollSpeed;
                }
            }

            // UI Drawing
            DrawText(TextFormat("Score : %d", score), 10, 10, 20, WHITE);
            DrawText(TextFormat("Health: %d", player.GetHealth()), 10, 40, 20, RED);
            DrawRectangle(12, 60, player.GetHealth(), 25, RED);
            DrawText(TextFormat("FPS: %d", GetFPS()), 1115, 10, 40, WHITE);

            // Game Over Check
            if (player.GetHealth() <= 0)
            {
                currentState = GAME_OVER;
                isPaused = true;
            }
        }

        EndDrawing();
    }

    // Cleanup
    UnloadSound(carNoise);
    UnloadSound(speedUpCol);
    UnloadSound(pickUp);
    UnloadSound(roadBlockcol);
    UnloadMusicStream(bgMusic);
    UnloadTexture(gameBackround);
    UnloadTexture(speedUpTexture);
    UnloadTexture(roadBlockTexture);
    UnloadTexture(appleTexture);

    CloseAudioDevice();
    CloseWindow();

    return 0;
}