#include <raylib.h>
#include <string>
#include <filesystem>

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

// everything for animation
struct AnimData
{
    Rectangle rec;
    Vector2 pos;
    int frame;
    float updateTime;
    float runningTime;
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

    // sound stuff
    InitAudioDevice();
    Sound carNoise = LoadSound("resources/sounds/carNoise.wav");
    Sound speedUpCol = LoadSound("resources/sounds/speedcol.wav");
    Sound pickUp = LoadSound("resources/sounds/pickupCoin.wav");
    Sound roadBlockcol = LoadSound("resources/sounds/roadBlockHit.wav");
    Music bgMusic = LoadMusicStream("resources/sounds/marioKart.mp3");
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
    int healthLength = 100;
    int health = 100;

    // scoring
    int score = 0;
    GameState currentState = GAME;
    float scoreTimer = 0.0f;
    bool isPaused = false;

    // game backround
    Texture2D gameBackround = LoadTexture("resources/images/gameBackround2.png");
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

    // default car
    Texture2D regCar = LoadTexture("resources/images/carStraight.png");
    AnimData regCarData;
    regCarData.rec.width = regCar.width;
    regCarData.rec.height = regCar.height;
    regCarData.rec.x = 0;
    regCarData.rec.y = 0;
    regCarData.pos.x = GetScreenWidth() / 2;
    regCarData.pos.y = 576;
    regCarData.frame = 0;
    regCarData.runningTime = 0;
    regCarData.updateTime = 0.1;
    // car move left
    Texture2D leftCar = LoadTexture("resources/images/carLeft.png");
    AnimData leftCarData;
    leftCarData.rec.width = leftCar.width / 4;
    leftCarData.rec.height = regCar.height;
    leftCarData.rec.x = 0;
    leftCarData.rec.y = 0;
    leftCarData.pos.x = GetScreenWidth() / 2;
    leftCarData.pos.y = 576;
    leftCarData.frame = 0;
    leftCarData.runningTime = 0;
    leftCarData.updateTime = 0.1;
    // car move right
    Texture2D rightCar = LoadTexture("resources/images/carRight.png");
    AnimData rightCarData;
    rightCarData.rec.width = rightCar.width / 3;
    rightCarData.rec.height = regCar.height;
    rightCarData.rec.x = 0;
    rightCarData.rec.y = 0;
    rightCarData.pos.x = GetScreenWidth() / 2;
    rightCarData.pos.y = 576;
    rightCarData.frame = 0;
    rightCarData.runningTime = 0;
    rightCarData.updateTime = 0.1;

    // car movement
    float carSpeed = 200.0f;
    Vector2 carPosition = {(float)GetScreenWidth() / 2, 576}; // default in middle

    // walls
    float leftBound = 180.0f;
    float rightBound = 1034.0f;

    // speedup prop
    Texture2D speedUp = LoadTexture("resources/images/speedUp.png");
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
    Texture2D roadBlock = LoadTexture("resources/images/roadblock.png");
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
    Texture2D apple = LoadTexture("resources/images/apple.png");
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
            if (health <= 0)
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

            // animate vehicles
            leftCarData = updateAnimData(leftCarData, frameTime, 1);
            rightCarData = updateAnimData(rightCarData, frameTime, 1);
            regCarData = updateAnimData(regCarData, frameTime, 6);

            // movement based on keys
            if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
            {
                if (carPosition.x > leftBound)
                {
                    carPosition.x -= carSpeed * frameTime; // move left
                }
                DrawTextureRec(leftCar, leftCarData.rec, carPosition, WHITE);
            }
            else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
            {
                if (carPosition.x < rightBound)
                {
                    carPosition.x += carSpeed * frameTime; // move right
                }
                DrawTextureRec(rightCar, rightCarData.rec, carPosition, WHITE);
            }
            else
            {
                DrawTextureRec(regCar, regCarData.rec, carPosition, WHITE);
            }

            // rectangles for collision checking
            Rectangle speedUpRect = {
                speedUpData.pos.x,     // actual x position
                speedUpData.pos.y,     // actual y position
                speedUpData.rec.width, // width of the texture
                speedUpData.rec.height // height of the texture
            };
            Rectangle carRect = {
                carPosition.x,        // actual x position
                carPosition.y,        // actual y position
                regCarData.rec.width, // width of the car texture
                regCarData.rec.height // height of the car texture
            };
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
            updateAnimData(appleData, frameTime, 3);
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

                health -= 10;
                healthLength -= 10; // decrease health by 10 when collision happens
                if (health < 0)
                {
                    health = 0;
                    healthLength = health;
                }
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
                health += 10;
                healthLength += 10; // increase health by 10 when collision happens
                if (health > 100)
                {
                    health = 100;
                    healthLength = health;
                }
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
            DrawText(TextFormat("Health: %d", health), 10, 40, 20, RED);
            DrawRectangle(12, 60, healthLength, 25, RED);

            // fps display
            DrawText(TextFormat("FPS: %d", GetFPS()), 1115, 10, 40, WHITE);

            // toggle gamestate
            if (health <= 0)
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
    UnloadTexture(regCar);
    UnloadTexture(leftCar);
    UnloadTexture(rightCar);
    UnloadTexture(speedUp);
    UnloadTexture(roadBlock);
    UnloadTexture(apple);

    CloseAudioDevice();
    CloseWindow();

    return 0;
}