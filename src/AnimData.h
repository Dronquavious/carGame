#ifndef ANIMDATA_H
#define ANIMDATA_H

#include <raylib.h>

// Enum to identify the type of game object
enum ObjectType
{
    NONE,
    ROADBLOCK,
    APPLE,
    SPEEDUP
};

// everything for animation
struct AnimData
{
    Rectangle rec;
    Vector2 pos;
    int frame;
    float updateTime;
    float runningTime;
    ObjectType type; // Type of the object
    bool active;     // Is the object currently on screen?
};

#endif // ANIMDATA_H