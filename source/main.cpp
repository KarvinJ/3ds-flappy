#include "starter.h"

const int TOP_SCREEN_WIDTH = 400;
const int BOTTOM_SCREEN_WIDTH = 320;
const int SCREEN_HEIGHT = 240;

C3D_RenderTarget *topScreen = nullptr;
C3D_RenderTarget *bottomScreen = nullptr;

bool isGamePaused;

int collisionCounter;

C2D_TextBuf textDynamicBuffer;

C2D_TextBuf textStaticBuffer;
C2D_Text staticTexts[1];

float textSize = 1.0f;

Sprite playerSprite;

const int PLAYER_SPEED = 10;

void update()
{
	int keyHeld = hidKeysHeld();

	if (keyHeld & KEY_LEFT && playerSprite.bounds.x > 0)
	{
		playerSprite.bounds.x -= PLAYER_SPEED;
	}

	else if (keyHeld & KEY_RIGHT && playerSprite.bounds.x < TOP_SCREEN_WIDTH - playerSprite.bounds.w)
	{
		playerSprite.bounds.x += PLAYER_SPEED;
	}

	else if (keyHeld & KEY_UP && playerSprite.bounds.y > 0)
	{
		playerSprite.bounds.y -= PLAYER_SPEED;
	}

	else if (keyHeld & KEY_DOWN && playerSprite.bounds.y < SCREEN_HEIGHT - playerSprite.bounds.h)
	{
		playerSprite.bounds.y += PLAYER_SPEED;
	}
}

void renderTopScreen()
{
	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	C2D_TargetClear(topScreen, BLACK);
	C2D_SceneBegin(topScreen);

	renderSprite(playerSprite);

	if (isGamePaused)
	{
		C2D_DrawText(&staticTexts[0], C2D_AtBaseline | C2D_WithColor, 110, 60, 0, textSize, textSize, WHITE);
	}

	C3D_FrameEnd(0);
}

void renderBottomScreen()
{
	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	C2D_TargetClear(bottomScreen, BLACK);
	C2D_SceneBegin(bottomScreen);

	drawDynamicText("Total collisions: %d", collisionCounter, textDynamicBuffer, 150, 175, textSize);

	C3D_FrameEnd(0);
}

int main(int argc, char *argv[])
{
	romfsInit();
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();

	topScreen = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	bottomScreen = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

	textStaticBuffer = C2D_TextBufNew(1024);
	textDynamicBuffer = C2D_TextBufNew(4096);

	C2D_TextParse(&staticTexts[0], textStaticBuffer, "Game Paused");
	C2D_TextOptimize(&staticTexts[0]);

	playerSprite = loadSprite("yellowbird-midflap.t3x", TOP_SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 32, 32);

	// touchPosition touch;

	while (aptMainLoop())
	{
		hidScanInput();

		// hidTouchRead(&touch);

		// touch.px;
		// touch.py;

		int keyDown = hidKeysDown();

		if (keyDown & KEY_START)
		{
			isGamePaused = !isGamePaused;
		}

		if (!isGamePaused)
		{
			update();
		}

		renderTopScreen();

		renderBottomScreen();
	}

	C2D_TextBufDelete(textDynamicBuffer);
	C2D_TextBufDelete(textStaticBuffer);

	C2D_Fini();
	C3D_Fini();
	gfxExit();
}
