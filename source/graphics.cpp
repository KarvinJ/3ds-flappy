#include "graphics.h"
#include <iostream>

void drawRectangle(Rectangle &rectangle)
{
	C2D_DrawRectSolid(rectangle.x, rectangle.y, rectangle.z, rectangle.w, rectangle.h, rectangle.color);
}

bool hasCollision(Rectangle &rectangle, Rectangle &rectangle2)
{
	return rectangle.x < rectangle2.x + rectangle2.w && rectangle.x + rectangle.w > rectangle2.x &&
		   rectangle.y < rectangle2.y + rectangle2.h && rectangle.y + rectangle.h > rectangle2.y;
}

Sprite loadSprite(const char *filePath, float positionX, float positionY, float width, float height)
{
	std::string basePath = "romfs:/gfx/";
	std::string fullPath = basePath + filePath;

	C2D_SpriteSheet sheet = C2D_SpriteSheetLoad(fullPath.c_str());
	C2D_Image image = C2D_SpriteSheetGetImage(sheet, 0);

	Rectangle bounds = {positionX, positionY, 0, width, height, WHITE};

	Sprite sprite = {image, bounds, sheet};

	return sprite;
}

void drawSprite(Sprite &sprite)
{
	C2D_DrawImageAt(sprite.texture, sprite.bounds.x, sprite.bounds.y, sprite.bounds.z, NULL, 1, 1);
}

void drawDynamicText(const char *textFormat, int value, C2D_TextBuf &dynamicBuffer, int positionX, int positionY, float textSize)
{
	char buf[160];
	C2D_Text dynamicText;
	snprintf(buf, sizeof(buf), textFormat, value);

	C2D_TextParse(&dynamicText, dynamicBuffer, buf);
	C2D_TextOptimize(&dynamicText);
	C2D_DrawText(&dynamicText, C2D_AlignCenter | C2D_WithColor, positionX, positionY, 0, textSize, textSize, WHITE);

	C2D_TextBufClear(dynamicBuffer);
}

void drawAndRotateImage(Sprite &sprite, float rotation)
{
    // need to make this adjustment, because the x and y coordinates place the image in the center, instead of the top-left corner.
    float positionX = sprite.bounds.x + sprite.bounds.w / 2;
    float positionY = sprite.bounds.y + sprite.bounds.h / 2;

    C2D_DrawImageAtRotated(sprite.texture, positionX, positionY, sprite.bounds.z, rotation, NULL, 1, 1);
}

void handleSpriteRotation(C2D_Sprite *sprites, Rectangle &spriteBounds, size_t spritesQuantity, float rotationAngle)
{
    for (size_t index = 0; index < spritesQuantity; index++)
    {
        C2D_SpriteSetPos(&sprites[index], spriteBounds.x, spriteBounds.y);

        // [in]	sprite	Pointer to sprite [in]	radians	Amount to rotate in radians, if I want to use degrees I can use C2D_SpriteRotateDegrees for the angle
        C2D_SpriteSetRotation(&sprites[index], rotationAngle);
    }
}

void drawSpriteAnimation(C2D_Sprite *sprites, SpriteRefreshInfo &refreshInfo, SpriteFrameInfo &frameInfo)
{
    // Get an elapsed time
    refreshInfo.stop = osGetTime();
    refreshInfo.elapsed = refreshInfo.stop - refreshInfo.start;

    // Check the elapsed time which is greater than or equal to the refresh time
    if (refreshInfo.elapsed >= refreshInfo.refreshTime)
    {
        // Update next sprite
        if (!frameInfo.shouldLoopOnce)
        {
            frameInfo.currentFrameIndex = (frameInfo.currentFrameIndex + 1) % frameInfo.SpritesQuantity;
        }
        else
        {
            if (frameInfo.currentFrameIndex < frameInfo.SpritesQuantity - 1)
            {
                frameInfo.currentFrameIndex++;
            }
        }

        refreshInfo.start = osGetTime();
        C2D_DrawSprite(&sprites[frameInfo.currentFrameIndex]);
    }
    else
    {
        C2D_DrawSprite(&sprites[frameInfo.currentFrameIndex]);
    }
}