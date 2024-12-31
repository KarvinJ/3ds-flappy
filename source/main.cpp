#include "starter.h"

// the 3ds has different screen width, but the same screen height.
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

Rectangle ball = {TOP_SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2, 0, 20, 20, WHITE};
Rectangle bottomBounds = {BOTTOM_SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0, 32, 32, BLUE};
Rectangle touchBounds = {0, 0, 0, 8, 8, WHITE};

const int PLAYER_SPEED = 10;

int ballVelocityX = 5;
int ballVelocityY = 5;

void update()
{
	// Respond to user input
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

	if (ball.x < 0 || ball.x > TOP_SCREEN_WIDTH - ball.w)
	{
		ballVelocityX *= -1;
		ball.color = GREEN;
	}

	else if (ball.y < 0 || ball.y > SCREEN_HEIGHT - ball.h)
	{
		ballVelocityY *= -1;
		ball.color = RED;
	}

	else if (hasCollision(playerSprite.bounds, ball))
	{
		ballVelocityX *= -1;
		ballVelocityY *= -1;

		ball.color = BLUE;

		collisionCounter++;
	}

	ball.x += ballVelocityX;
	ball.y += ballVelocityY;
}

void renderTopScreen()
{
	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	C2D_TargetClear(topScreen, BLACK);
	C2D_SceneBegin(topScreen);

	// (float x, float y, float z, float  w, float h, u32 clr)
	drawRectangle(ball);

	// ( 	C2D_Image img, float x, float y, float depth, const C2D_ImageTint *tint C2D_OPTIONALnullptr, float scaleX C2D_OPTIONAL1.0f, float scaleY C2D_OPTIONAL1.0f)
	renderSprite(playerSprite);

	if (isGamePaused)
	{
		// Draws text using the GPU.

		// Parameters
		//     [in]	text	Pointer to text object.
		//     [in]	flags	Text drawing flags.
		//     [in]	x	Horizontal position to draw the text on.
		//     [in]	y	Vertical position to draw the text on. If C2D_AtBaseline is not specified (default), this is the top left corner of the block of text; otherwise this is the position of the baseline of the first line of text.
		//     [in]	z	Depth value of the text. If unsure, pass 0.0f.
		//     [in]	scaleX	Horizontal textSize of the font. 1.0f corresponds to the native textSize of the font.
		//     [in]	scaleY	Vertical textSize of the font. 1.0f corresponds to the native textSize of the font.
		// Draw static text strings
		C2D_DrawText(&staticTexts[0], C2D_AtBaseline | C2D_WithColor, 110, 60, 0, textSize, textSize, WHITE);
	}

	C3D_FrameEnd(0);
}

void renderBottomScreen()
{
	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	C2D_TargetClear(bottomScreen, BLACK);
	C2D_SceneBegin(bottomScreen);

	drawRectangle(bottomBounds);

	drawDynamicText("Total collisions: %d", collisionCounter, textDynamicBuffer, 150, 175, textSize);

	C3D_FrameEnd(0);
}

int main(int argc, char *argv[])
{
	// Init libs
	romfsInit();
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();

	// Create top and bottom screens
	topScreen = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	bottomScreen = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

	// Create two text buffers: one for static text, and another one for
	// dynamic text - the latter will be cleared at each frame.
	textStaticBuffer = C2D_TextBufNew(1024); // support up to 4096 glyphs in the buffer
	textDynamicBuffer = C2D_TextBufNew(4096);

	// Parse the static text strings
	C2D_TextParse(&staticTexts[0], textStaticBuffer, "Game Paused");

	// Optimize the static text strings
	C2D_TextOptimize(&staticTexts[0]);

	playerSprite = loadSprite("romfs:/gfx/alien_1.t3x", TOP_SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 32, 32);

	touchPosition touch;

	// Main loop
	while (aptMainLoop())
	{
		hidScanInput();

		// Read the touch screen coordinates
		hidTouchRead(&touch);

		if (touch.px > 0 && touch.py > 0 && touch.px < BOTTOM_SCREEN_WIDTH - bottomBounds.w && touch.py < SCREEN_HEIGHT - bottomBounds.h)
		{
			bottomBounds.x = touch.px;
			bottomBounds.y = touch.py;
		}

		touchBounds.x = touch.px;
		touchBounds.y = touch.py;

		if (hasCollision(touchBounds, bottomBounds))
		{
			bottomBounds.color = RED;
		}
		else
		{
			bottomBounds.color = BLUE;
		}

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

	// Delete the text buffers
	C2D_TextBufDelete(textDynamicBuffer);
	C2D_TextBufDelete(textStaticBuffer);

	// Deinit libs
	C2D_Fini();
	C3D_Fini();
	gfxExit();
}
