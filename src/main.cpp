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
    srand(time(NULL)); // random gen for obstacle spawns

    InitAudioDevice();
    Sound carNoise = LoadSound(getAssetPath("sounds/carNoise.wav").c_str());
    Sound speedUpCol = LoadSound(getAssetPath("sounds/speedcol.wav").c_str());
    Sound pickUp = LoadSound(getAssetPath("sounds/pickupCoin.wav").c_str());
    Sound roadBlockcol = LoadSound(getAssetPath("sounds/roadBlockHit.wav").c_str());
    Music bgMusic = LoadMusicStream(getAssetPath("sounds/marioKart.mp3").c_str());

    // game backround
    Texture2D gameBackround = LoadTexture(getAssetPath("images/gameBackround2.png").c_str());
    // speedup prop
    Texture2D speedUp = LoadTexture(getAssetPath("images/speedUp.png").c_str());
    // roadBlock
    Texture2D roadBlock = LoadTexture(getAssetPath("images/roadblock.png").c_str());
    // apple
    Texture2D apple = LoadTexture(getAssetPath("images/apple.png").c_str());

    Player player(getAssetPath("images/carStraight.png"), getAssetPath("images/carLeft.png"), getAssetPath("images/carRight.png"));

    bool hasPlayedSpeedSound = false;
    bool hasPlayedPickUpsound = false;
    bool hasPlayedRoadBlockSound = false;
    SetSoundVolume(carNoise, 0.10f);
    SetSoundVolume(speedUpCol, 0.5f);
    SetSoundVolume(roadBlockcol, 0.5f);
    SetSoundVolume(pickUp, 0.5f);
    SetMusicVolume(bgMusic, 0.35f);
    PlaySound(carNoise);

    // Play the background music only once
    if (!IsMusicStreamPlaying(bgMusic))
    {
        PlayMusicStream(bgMusic);
    }

    // health
    float healthTimer = 2.0f;

    // scoring
    int score = 0;
    GameState currentState = GAME;
    float scoreTimer = 0.0f;
    bool isPaused = false;

    // game backround
    float bgY1 = 0.0f;
    float bgY2 = -GetScreenHeight();
    float bgScrollSpeed = 500.0f;

    // x values of lane positions
    Vector2 laneSpawnPositions[] =
        {
            {239, -144},
            {322, -144},
            {397, -144},
            {475, -144},
            {557, -144},
            {640, -144},
            {725, -144},
            {810, -144},
            {885, -144},
            {964, -144},

        };

    // speedup prop
    AnimData speedUpData;
    speedUpData.rec.width = speedUp.width;
    speedUpData.rec.height = speedUp.height;
    speedUpData.rec.x = 0;
    speedUpData.rec.y = 0;
    speedUpData.pos.x = 300;
    speedUpData.pos.y = 500;
    speedUpData.frame = 0;
    speedUpData.runningTime = 0;
    speedUpData.updateTime = 0.1;
    // speedUp temp effect / spawning
    float speedUpDuration = 2.0f;
    float speedUpTimer = 0.0f;
    float originalBgScrollSpeed = bgScrollSpeed;
    bool isSpeedUpActive = false;
    speedUpData.pos = laneSpawnPositions[GetRandomValue(0, 9)]; // random spawning

    // roadBlock
    AnimData roadBlockData;
    roadBlockData.rec.width = roadBlock.width;
    roadBlockData.rec.height = roadBlock.height;
    roadBlockData.rec.x = 0;
    roadBlockData.rec.y = 0;
    roadBlockData.pos.x = 300;
    roadBlockData.pos.y = 500;
    roadBlockData.frame = 0;
    roadBlockData.runningTime = 0;
    roadBlockData.updateTime = 0.1;
    roadBlockData.pos = laneSpawnPositions[GetRandomValue(0, 9)]; // random spawning

    // apple
    AnimData appleData;
    appleData.rec.width = apple.width / 3;
    appleData.rec.height = apple.height;
    appleData.rec.x = 0;
    appleData.rec.y = 0;
    appleData.pos.x = 300;
    appleData.pos.y = 500;
    appleData.frame = 0;
    appleData.runningTime = 0;
    appleData.updateTime = 0.1;
    appleData.pos = laneSpawnPositions[GetRandomValue(0, 9)]; // random spawning

    // game loop
    while (!WindowShouldClose())
    {

        if (!IsSoundPlaying(carNoise))
        {
            PlaySound(carNoise);
        }

        // pausing
        if (IsKeyPressed(KEY_P))
        {
            isPaused = !isPaused; // toggle the paused state
        }

        BeginDrawing();

        if (isPaused)
        {
            if (player.GetHealth() <= 0)
            {
                StopMusicStream(bgMusic);
                StopSound(carNoise);
                ClearBackground(BLACK);
                DrawText("GAME OVER", windowWidth / 2 - 100, windowHeight / 2 - 50, 40, RED);
            }
            else
            {
                ClearBackground(BLACK);

                // get the width of the pause message
                const char *pauseMessage = "Game Paused. Press 'P' to Resume";
                int textWidth = MeasureText(pauseMessage, 40); // get the width of the text at font size 40
                int textHeight = 40;                           // the height of the text (same as font size in this case)

                // calculate the X and Y positions to center the text
                int posX = (GetScreenWidth() - textWidth) / 2;
                int posY = (GetScreenHeight() - textHeight) / 2;

                // draw the pause message centered on the screen
                DrawText(pauseMessage, posX, posY, 40, GREEN);
            }
        }
        else
        {
            // delta time
            float frameTime = GetFrameTime();

            UpdateMusicStream(bgMusic);

            player.HandleInput(frameTime);
            player.Update(frameTime);

            // scoring
            scoreTimer += frameTime;
            if (scoreTimer >= 1.0f)
            {
                score += 10;
                scoreTimer = 0.0f;
            }

            ClearBackground(WHITE);

            // backround scrolling
            scrollBackground(bgY1, bgY2, bgScrollSpeed, frameTime, GetScreenHeight());
            DrawTexture(gameBackround, 0, bgY1, WHITE);
            DrawTexture(gameBackround, 0, bgY2, WHITE);

            // score drawing
            DrawText(TextFormat("Score : %d", score), 10, 10, 20, WHITE);

            player.Draw();

            // rectangles for collision checking
            Rectangle speedUpRect = {
                speedUpData.pos.x,     // actual x position
                speedUpData.pos.y,     // actual y position
                speedUpData.rec.width, // width of the texture
                speedUpData.rec.height // height of the texture
            };
            Rectangle carRect = player.GetRect();
            Rectangle roadBlockRect = {
                roadBlockData.pos.x,     // actual x position
                roadBlockData.pos.y,     // actual y position
                roadBlockData.rec.width, // width of the obstacle
                roadBlockData.rec.height // height of the obstacle
            };
            Rectangle appleRect = {
                appleData.pos.x,
                appleData.pos.y,
                appleData.rec.width,
                appleData.rec.height};

            // speedup prop
            speedUpData.pos.y += bgScrollSpeed * frameTime;
            if (speedUpData.pos.y > GetScreenHeight())
            {
                speedUpData.pos = laneSpawnPositions[GetRandomValue(0, 9)];
            }
            DrawTextureRec(speedUp, speedUpData.rec, speedUpData.pos, WHITE);

            // roadblock prop
            roadBlockData.pos.y += bgScrollSpeed * frameTime;
            if (roadBlockData.pos.y > GetScreenHeight())
            {
                roadBlockData.pos.y = -50;
                roadBlockData.pos.x = laneSpawnPositions[GetRandomValue(0, 9)].x; // random spawn in one of the lanes
            }

            DrawTextureRec(roadBlock, roadBlockData.rec, roadBlockData.pos, WHITE);

            // apple prop
            appleData = updateAnimData(appleData, frameTime, 3);
            appleData.pos.y += bgScrollSpeed * frameTime;
            if (appleData.pos.y > GetScreenHeight())
            {
                appleData.pos.y = -50;
                appleData.pos.x = laneSpawnPositions[GetRandomValue(0, 9)].x; // random spawn in one of the lanes
            }

            DrawTextureRec(apple, appleData.rec, appleData.pos, WHITE);

            // check collision between car and speed-up
            if (CheckCollisionRecs(carRect, speedUpRect))
            {
                if (!hasPlayedSpeedSound)
                {
                    PlaySound(speedUpCol);
                    hasPlayedSpeedSound = true; // true after the sound is played
                }
                isSpeedUpActive = true;
                speedUpTimer = speedUpDuration;
                bgScrollSpeed += 25; // increase backround scroll speed
            }

            // speedup effect
            if (isSpeedUpActive)
            {
                speedUpTimer -= frameTime;
                if (speedUpTimer <= 0.0f)
                {
                    isSpeedUpActive = false;
                    bgScrollSpeed = originalBgScrollSpeed;
                    hasPlayedSpeedSound = false;
                }
            }

            // collision between car and roadbloack
            if (CheckCollisionRecs(carRect, roadBlockRect))
            {
                if (!hasPlayedRoadBlockSound)
                {
                    PlaySound(roadBlockcol);
                    hasPlayedRoadBlockSound = true; // set true after sound is played
                }

                player.TakeDamage(10);
                roadBlockData.pos.y = -50; // reset obstacle position after collision
            }

            if (roadBlockData.pos.y == -50)
            {
                hasPlayedRoadBlockSound = false; //  to play again on the next collision
            }

            if (CheckCollisionRecs(carRect, appleRect))
            {
                if (!hasPlayedPickUpsound)
                {
                    PlaySound(pickUp);
                    hasPlayedPickUpsound = true; // true after the sound is played
                }
                player.Heal(10);
                appleData.pos.y = -50; // reset obstacle position after collision
            }
            if (appleData.pos.y == -50)
            {
                hasPlayedPickUpsound = false; // allow sound to play again on the next collision
            }

            if (hasPlayedPickUpsound)
            {
                healthTimer -= frameTime;
                if (healthTimer <= 0.0f)
                {
                    healthTimer = 2.0f;
                    hasPlayedPickUpsound = false;
                }
            }

            // health display
            DrawText(TextFormat("Health: %d", player.GetHealth()), 10, 40, 20, RED);
            DrawRectangle(12, 60, player.GetHealth(), 25, RED);

            // fps display
            DrawText(TextFormat("FPS: %d", GetFPS()), 1115, 10, 40, WHITE);

            // toggle gamestate
            if (player.GetHealth() <= 0)
            {
                currentState = GAME_OVER;
            }

            if (currentState == GAME_OVER)
            {
                isPaused = true; // pauses and checks if over
            }
        }

        EndDrawing();
    }

    // cleanup resources
    UnloadSound(carNoise);
    UnloadSound(speedUpCol);
    UnloadSound(pickUp);
    UnloadSound(roadBlockcol);
    UnloadMusicStream(bgMusic);
    UnloadTexture(gameBackround);
    UnloadTexture(speedUp);
    UnloadTexture(roadBlock);
    UnloadTexture(apple);

    CloseAudioDevice();
    CloseWindow();

    return 0;
}