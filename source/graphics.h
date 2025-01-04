#pragma once

#include <3ds/svc.h>
#include <3ds/types.h>
#include <citro2d.h>

#define ANIMATION_REFRESH_TIME_MIN 34 //  A minimum of the animation refresh time is 34ms if 60 fps then set to 17ms

const u32 WHITE = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
const u32 BLACK = C2D_Color32(0x00, 0x00, 0x00, 0x00);
const u32 YELLOW = C2D_Color32(0xF3, 0xD8, 0x3F, 0xFF);
const u32 GREEN = C2D_Color32(0x00, 0xFF, 0x00, 0xFF);
const u32 RED = C2D_Color32(0xFF, 0x00, 0x00, 0xFF);
const u32 BLUE = C2D_Color32(0x00, 0x00, 0xFF, 0xFF);
const u32 BROWN = C2D_Color32(0x1D, 0x1D, 0x1B, 0xFF);

typedef struct SpriteRefreshInfo
{
    uint64_t start;       ///< Start time
    uint64_t stop;        ///< Lap time
    uint64_t elapsed;     ///< Elapsed time (`start` - `stop`)
    uint64_t refreshTime; ///< Next sprite update time [Unit: ms]
} SpriteRefreshInfo;

typedef struct SpriteFrameInfo
{
    unsigned int currentFrameIndex; ///< Current sprite ID number
    size_t SpritesQuantity;  ///< Number of sprites
    bool shouldLoopOnce;     ///< Sprite animation loop information
} SpriteFrameInfo;

typedef struct
{
	float x;
	float y;
	float z;
	float w;
	float h;
	unsigned int color;
} Rectangle;

typedef struct
{
	C2D_Image texture;
	Rectangle bounds;
	C2D_SpriteSheet sheet;
} Sprite;

void drawRectangle(Rectangle &rectangle);

bool hasCollision(Rectangle &rectangle, Rectangle &rectangle2);

Sprite loadSprite(const char *filePath, float positionX, float positionY, float width, float height);

void drawSprite(Sprite &sprite);

void drawDynamicText(const char *textFormat, int value, C2D_TextBuf &dynamicBuffer, int positionX, int positionY, float textSize);

void drawAndRotateImage(Sprite &sprite, float rotation);

void handleSpriteRotation(C2D_Sprite *sprites, Rectangle &spriteBounds, size_t spritesQuantity, float rotationAngle);

void drawSpriteAnimation(C2D_Sprite *sprites, SpriteRefreshInfo &refreshInfo, SpriteFrameInfo &frameInfo);