#ifndef ANIMDATA_H
#define ANIMDATA_H

#include <raylib.h>

// everything for animation
struct AnimData
{
    Rectangle rec;
    Vector2 pos;
    int frame;
    float updateTime;
    float runningTime;
};

#endif // ANIMDATA_H
