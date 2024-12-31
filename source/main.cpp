#include "starter.h"
#include <vector>
#include <fstream>

const int TOP_SCREEN_WIDTH = 400;
const int BOTTOM_SCREEN_WIDTH = 320;
const int SCREEN_HEIGHT = 240;

C3D_RenderTarget *topScreen = nullptr;
C3D_RenderTarget *bottomScreen = nullptr;

bool isGameOver;
bool isGamePaused;
float startGameTimer;

bool shouldRotateUp = false;
float downRotationTimer = 0;
float upRotationTimer = 0;

float gravity = 0;

int score = 0;
float initialAngle = 0;
int highScore;

C2D_TextBuf textDynamicBuffer;

C2D_TextBuf textStaticBuffer;
C2D_Text staticTexts[1];

float textSize = 1.0f;

const int PLAYER_SPEED = 10;

Rectangle birdsBounds;
Sprite birdSprites;
Sprite playerSprite;
Sprite startGameSprite;
Sprite backgroundSprite;
Sprite groundSprite;

Sprite upPipeSprite;
Sprite downPipeSprite;

std::vector<Sprite> numbers;
std::vector<Sprite> numberTens;
std::vector<Sprite> highScoreNumbers;
std::vector<Sprite> highScoreNumberTens;

typedef struct
{
    float y;
    Sprite sprite;
    float impulse;
    float gravityIncrement;
} Player;

Player player;

float groundYPosition;

Rectangle groundCollisionBounds;

typedef struct
{
    float x;
    float y;
} Vector2;

std::vector<Vector2> groundPositions;

typedef struct
{
    float x;
    Sprite sprite;
    bool isBehind;
    bool isDestroyed;
} Pipe;

std::vector<Pipe> pipes;

float lastPipeSpawnTime;

void generatePipes()
{
    //pipe position range 240 - 290
    int upPipePosition = rand() % 290;

//minimum upPipePosition.
    if(upPipePosition < 240)
        upPipePosition = 240;
    
    upPipePosition *= -1;

    Rectangle upPipeBounds = {TOP_SCREEN_WIDTH, (float)upPipePosition, 0, upPipeSprite.bounds.w, upPipeSprite.bounds.h, WHITE};

    Sprite upSprite = {upPipeSprite.texture, upPipeBounds};

    Pipe upPipe = {TOP_SCREEN_WIDTH, upSprite, false, false};

    // gap size = 40.
    int downPipePosition = upPipePosition + upPipeSprite.bounds.h + 75;

    Rectangle downPipeBounds = {TOP_SCREEN_WIDTH, (float)downPipePosition, 0, downPipeSprite.bounds.w, downPipeSprite.bounds.h, WHITE};

    Sprite downSprite = {downPipeSprite.texture, downPipeBounds};

    Pipe downPipe = {TOP_SCREEN_WIDTH, downSprite, false, false};

    pipes.push_back(upPipe);
    pipes.push_back(downPipe);

    lastPipeSpawnTime = 0;
}

int loadHighScore()
{
    std::string highScoreText;

    std::ifstream highScores("high-score.txt");

    getline(highScores, highScoreText);

    highScores.close();

    int highScore = stoi(highScoreText);

    return highScore;
}

void saveScore()
{
    std::ofstream highScores("high-score.txt");

    std::string scoreString = std::to_string(score);
    highScores << scoreString;

    highScores.close();
}

void resetGame(Player &player)
{
    if (score > highScore)
    {
        saveScore();
    }

    highScore = loadHighScore();

    isGameOver = false;
    startGameTimer = 0;
    score = 0;
    startGameTimer = 0;
    initialAngle = 0;
    player.y = SCREEN_HEIGHT / 2;
    player.sprite.bounds.x = TOP_SCREEN_WIDTH / 2;
    player.sprite.bounds.y = TOP_SCREEN_WIDTH / 2;
    gravity = 0;
    pipes.clear();
}

// I'm replacing deltaTime for the value 10, just for now
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

	startGameTimer++;

    lastPipeSpawnTime++;

    if (lastPipeSpawnTime >= 120)
    {
        generatePipes();
    }

    if (player.y < -player.sprite.bounds.h)
    {
        isGameOver = true;
    }
    
    if (startGameTimer > 60)
    {
        player.y += gravity * 10;
        player.sprite.bounds.y = player.y;
        gravity += player.gravityIncrement * 10;
    }

    if (hasCollision(player.sprite.bounds, groundCollisionBounds))
    {
        isGameOver = true;
        // Mix_PlayChannel(-1, dieSound, 0);
    }

    // for (Vector2 &groundPosition : groundPositions)
    // {
    //     groundPosition.x -= 75 * 10;

    //     if (groundPosition.x < -groundSprite.bounds.w)
    //     {
    //         groundPosition.x = groundSprite.bounds.w * 2;
    //     }
    // }

    for (auto actualPipe = pipes.begin(); actualPipe != pipes.end();)
    {
        if (!actualPipe->isDestroyed)
        {
            actualPipe->x -= 2;
            actualPipe->sprite.bounds.x = actualPipe->x;
        }

        if (hasCollision(player.sprite.bounds, actualPipe->sprite.bounds))
        {
            isGameOver = true;
            // Mix_PlayChannel(-1, dieSound, 0);
        }

        if (!actualPipe->isBehind && player.sprite.bounds.x > actualPipe->sprite.bounds.x)
        {
            actualPipe->isBehind = true;

            if (actualPipe->sprite.bounds.y < player.sprite.bounds.y)
            {
                score++;
                // Mix_PlayChannel(-1, crossPipeSound, 0);
            }
        }

        if (actualPipe->sprite.bounds.x < -actualPipe->sprite.bounds.w)
        {
            actualPipe->isDestroyed = true;
            pipes.erase(actualPipe);
        }
        else
        {
            actualPipe++;
        }
    }
}

void renderTopScreen()
{
	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	C2D_TargetClear(topScreen, BLACK);
	C2D_SceneBegin(topScreen);

	backgroundSprite.bounds.x = 0;
    renderSprite(backgroundSprite);

    backgroundSprite.bounds.x = backgroundSprite.bounds.w;
    renderSprite(backgroundSprite);

    groundSprite.bounds.x = 0;
    renderSprite(groundSprite);

    groundSprite.bounds.x = groundSprite.bounds.w;
    renderSprite(groundSprite);
    
    for (Pipe &pipe : pipes)
    {
        if (!pipe.isDestroyed)
        {
            renderSprite(pipe.sprite);
        }
    }

    // if (highScore < 10)
    // {
    //     highScoreNumbers[highScore].bounds.x = 180;
    //     renderSprite(highScoreNumbers[highScore]);
    // }
    // else
    // {
    //     int tens = (int)(highScore / 10);
    //     int units = (int)(highScore % 10);

    //     highScoreNumberTens[tens].bounds.x = 170;
    //     highScoreNumbers[units].bounds.x = 180;

    //     renderSprite(highScoreNumberTens[tens]);
    //     renderSprite(highScoreNumbers[units]);
    // }

    if (score < 10)
    {
        renderSprite(numbers[score]);
    }
    else
    {
        int tens = (int)(score / 10);
        int units = (score % 10);

        numberTens[tens].bounds.x = TOP_SCREEN_WIDTH / 2 - 20;

        renderSprite(numberTens[tens]);
        renderSprite(numbers[units]);
    }

    for (Vector2 &groundPosition : groundPositions)
    {
        groundSprite.bounds.x = groundPosition.x;
        renderSprite(groundSprite);
    }

    if (isGameOver)
    {
        renderSprite(startGameSprite);
    }

	renderSprite(playerSprite);

	C3D_FrameEnd(0);
}

void renderBottomScreen()
{
	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	C2D_TargetClear(bottomScreen, BLACK);
	C2D_SceneBegin(bottomScreen);

	drawDynamicText("Total collisions: %d", 0, textDynamicBuffer, 150, 175, textSize);

    if (isGamePaused)
	{
		C2D_DrawText(&staticTexts[0], C2D_AtBaseline | C2D_WithColor, 80, 60, 0, textSize, textSize, WHITE);
	}

	C3D_FrameEnd(0);
}

void loadNumbersSprites()
{
    std::string fileExtension = ".t3x";
    
    numbers.reserve(10);
    numberTens.reserve(10);

    highScoreNumbers.reserve(10);
    highScoreNumberTens.reserve(10);

    for (int i = 0; i < 10; i++)
    {
        std::string completeString = std::to_string(i) + fileExtension;

		Sprite numberSprite = loadSprite(completeString.c_str(), TOP_SCREEN_WIDTH / 2, 15, 24, 36);

        numbers.push_back(numberSprite);
        numberTens.push_back(numberSprite);

        highScoreNumbers.push_back(numberSprite);
        highScoreNumberTens.push_back(numberSprite);
    }
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

	playerSprite = loadSprite("yellowbird-midflap.t3x", TOP_SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 34, 24);

	highScore = loadHighScore();

    upPipeSprite = loadSprite("pipe-green-180.t3x", TOP_SCREEN_WIDTH / 2, -220, 52, 320);
    downPipeSprite = loadSprite("pipe-green.t3x", TOP_SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 52, 320);

    startGameSprite = loadSprite("message.t3x", TOP_SCREEN_WIDTH / 2 - 75, 0, 184, 267);
    backgroundSprite = loadSprite("background-day.t3x", 0, 0, 288, 512);

    groundSprite = loadSprite("base.t3x", 0, 0, 336, 112);

    groundYPosition = SCREEN_HEIGHT - groundSprite.bounds.h + 50;

    groundSprite.bounds.y = groundYPosition;

    groundCollisionBounds = {0, groundYPosition, 0, SCREEN_HEIGHT, groundSprite.bounds.h, WHITE};

    groundPositions.push_back({0, groundYPosition});
    groundPositions.push_back({groundSprite.bounds.w, groundYPosition});

    player = Player{SCREEN_HEIGHT / 2, playerSprite, -10000, 500};

    loadNumbersSprites();

    birdSprites = loadSprite("yellow-bird.t3x", TOP_SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 105, 24);

    birdsBounds = {0, 0, birdSprites.bounds.w / 3, birdSprites.bounds.h};

    srand(time(NULL));

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

		if (!isGameOver && keyDown & KEY_A)
        {
			// no deltaTime
            gravity = player.impulse * 10;

            shouldRotateUp = true;
            upRotationTimer = 1;
            downRotationTimer = 0;
            initialAngle = -20;

            // Mix_PlayChannel(-1, flapSound, 0);
        }

        else if (isGameOver && keyDown & KEY_A)
        {
            resetGame(player);
        }

		if (!isGameOver && !isGamePaused)
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
