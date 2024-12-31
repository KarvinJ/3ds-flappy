#pragma once

#include <citro2d.h>

const u32 WHITE = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
const u32 BLACK = C2D_Color32(0x00, 0x00, 0x00, 0x00);
const u32 YELLOW = C2D_Color32(0xF3, 0xD8, 0x3F, 0xFF);
const u32 GREEN = C2D_Color32(0x00, 0xFF, 0x00, 0xFF);
const u32 RED = C2D_Color32(0xFF, 0x00, 0x00, 0xFF);
const u32 BLUE = C2D_Color32(0x00, 0x00, 0xFF, 0xFF);
const u32 BROWN = C2D_Color32(0x1D, 0x1D, 0x1B, 0xFF);

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

void renderSprite(Sprite &sprite);

void drawDynamicText(const char *textFormat, int value, C2D_TextBuf &dynamicBuffer, int positionX, int positionY, float textSize);