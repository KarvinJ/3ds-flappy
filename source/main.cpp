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

bool shouldRotateUp;
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

Rectangle touchBounds = {0, 0, 0, 8, 8, WHITE};
Rectangle bottomScreenBounds = {10, 10, 0, BOTTOM_SCREEN_WIDTH, SCREEN_HEIGHT, BLACK};

Rectangle birdsBounds;
Sprite birdSprites;
Sprite playerSprite;
Sprite startGameSprite;
Sprite topBackgroundSprite;
Sprite bottomBackgroundSprite;
Sprite groundSprite;

Sprite upPipeSprite;
Sprite downPipeSprite;

std::vector<Sprite> numbers;
std::vector<Sprite> numberTens;
std::vector<Sprite> highScoreNumbers;
std::vector<Sprite> highScoreNumberTens;

typedef struct
{
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
    Sprite sprite;
    bool isBehind;
    bool isDestroyed;
} Pipe;

std::vector<Pipe> pipes;
std::vector<Pipe> bottomScreenPipes;

float lastPipeSpawnTime;

int getRandomNumberBetweenRange(int min, int max)
{
    return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

void generatePipes()
{
    // the value needs to be negative.
    int upPipePosition = getRandomNumberBetweenRange(-290, -240);

    Rectangle upPipeBounds = {TOP_SCREEN_WIDTH, (float)upPipePosition, 0, upPipeSprite.bounds.w, upPipeSprite.bounds.h, WHITE};

    Sprite upSprite = {upPipeSprite.texture, upPipeBounds};

    Pipe upPipe = {upSprite, false, false};

    int downPipePosition = upPipePosition + upPipeSprite.bounds.h + 75;

    Rectangle downPipeBounds = {TOP_SCREEN_WIDTH, (float)downPipePosition, 0, downPipeSprite.bounds.w, downPipeSprite.bounds.h, WHITE};

    Sprite downSprite = {downPipeSprite.texture, downPipeBounds};

    Pipe downPipe = {downSprite, false, false};

    pipes.push_back(upPipe);
    pipes.push_back(downPipe);

    downPipe.sprite.bounds.x = BOTTOM_SCREEN_WIDTH + 40;
    downPipe.sprite.bounds.y = -25;
    bottomScreenPipes.push_back(downPipe);

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

void resetGame()
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
    player.sprite.bounds.x = TOP_SCREEN_WIDTH / 2;
    player.sprite.bounds.y = SCREEN_HEIGHT / 2;
    gravity = 0;
    pipes.clear();
    bottomScreenPipes.clear();
}

// I'm replacing deltaTime for the value 10, just for now
void update()
{
    startGameTimer++;

    lastPipeSpawnTime++;

    if (lastPipeSpawnTime >= 60)
    {
        generatePipes();
    }

    if (player.sprite.bounds.y < -player.sprite.bounds.h)
    {
        isGameOver = true;
    }

    if (startGameTimer > 40)
    {
        player.sprite.bounds.y += gravity;
        gravity += player.gravityIncrement;
    }

    if (hasCollision(player.sprite.bounds, groundCollisionBounds))
    {
        isGameOver = true;
        // Mix_PlayChannel(-1, dieSound, 0);
    }

    for (Vector2 &groundPosition : groundPositions)
    {
        groundPosition.x -= 3;

        if (groundPosition.x < -groundSprite.bounds.w)
        {
            groundPosition.x = groundSprite.bounds.w * 2;
        }
    }

    for (auto actualPipe = pipes.begin(); actualPipe != pipes.end();)
    {
        actualPipe->sprite.bounds.x -= 3;

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

    for (auto actualPipe = bottomScreenPipes.begin(); actualPipe != bottomScreenPipes.end();)
    {
        actualPipe->sprite.bounds.x -= 3;

        if (actualPipe->sprite.bounds.x < -actualPipe->sprite.bounds.w)
        {
            actualPipe->isDestroyed = true;
            bottomScreenPipes.erase(actualPipe);
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

    topBackgroundSprite.bounds.x = 0;
    renderSprite(topBackgroundSprite);

    topBackgroundSprite.bounds.x = topBackgroundSprite.bounds.w;
    renderSprite(topBackgroundSprite);

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

    if (highScore < 10)
    {
        highScoreNumbers[highScore].bounds.x = 50;
        renderSprite(highScoreNumbers[highScore]);
    }
    else
    {
        int tens = (int)(highScore / 10);
        int units = (int)(highScore % 10);

        highScoreNumberTens[tens].bounds.x = 25;
        highScoreNumbers[units].bounds.x = 50;

        renderSprite(highScoreNumberTens[tens]);
        renderSprite(highScoreNumbers[units]);
    }

    if (score < 10)
    {
        renderSprite(numbers[score]);
    }
    else
    {
        int tens = (int)(score / 10);
        int units = (score % 10);

        numberTens[tens].bounds.x = TOP_SCREEN_WIDTH / 2 - 25;

        renderSprite(numberTens[tens]);
        renderSprite(numbers[units]);
    }

    for (Vector2 &groundPosition : groundPositions)
    {
        groundSprite.bounds.x = groundPosition.x;
        renderSprite(groundSprite);
    }

    renderSprite(player.sprite);

    C3D_FrameEnd(0);
}

void renderBottomScreen()
{
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C2D_TargetClear(bottomScreen, BLACK);
    C2D_SceneBegin(bottomScreen);

    bottomBackgroundSprite.bounds.x = 0;
    bottomBackgroundSprite.bounds.y = 0;
    renderSprite(bottomBackgroundSprite);

    bottomBackgroundSprite.bounds.x = bottomBackgroundSprite.bounds.w;
    bottomBackgroundSprite.bounds.y = 0;
    renderSprite(bottomBackgroundSprite);

    for (Pipe &pipe : bottomScreenPipes)
    {
        if (!pipe.isDestroyed)
        {
            renderSprite(pipe.sprite);
        }
    }

    if (isGameOver)
    {
        renderSprite(startGameSprite);
    }

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

    loadNumbersSprites();

    playerSprite = loadSprite("yellowbird-midflap.t3x", TOP_SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 34, 24);

    player = Player{playerSprite, -6, 1};

    highScore = loadHighScore();

    upPipeSprite = loadSprite("pipe-green-180.t3x", TOP_SCREEN_WIDTH / 2, -220, 52, 320);
    downPipeSprite = loadSprite("pipe-green.t3x", TOP_SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 52, 320);

    startGameSprite = loadSprite("message.t3x", BOTTOM_SCREEN_WIDTH / 2 - 90, -80, 184, 267);

    topBackgroundSprite = loadSprite("background-day.t3x", 0, -250, 288, 512);
    bottomBackgroundSprite = topBackgroundSprite;

    groundSprite = loadSprite("base.t3x", 0, 0, 336, 112);

    groundYPosition = SCREEN_HEIGHT - groundSprite.bounds.h + 50;

    groundSprite.bounds.y = groundYPosition;

    groundCollisionBounds = {0, groundYPosition, 0, SCREEN_HEIGHT, groundSprite.bounds.h, WHITE};

    groundPositions.push_back({0, groundYPosition});
    groundPositions.push_back({groundSprite.bounds.w, groundYPosition});
    groundPositions.push_back({groundSprite.bounds.w * 2, groundYPosition});

    birdSprites = loadSprite("yellow-bird.t3x", TOP_SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 105, 24);

    birdsBounds = {0, 0, birdSprites.bounds.w / 3, birdSprites.bounds.h};

    srand(time(NULL));

    touchPosition touch;

    while (aptMainLoop())
    {
        hidScanInput();

        hidTouchRead(&touch);

        touchBounds.x = touch.px;
        touchBounds.y = touch.py;

        int keyDown = hidKeysDown();

        if (keyDown & KEY_START)
        {
            isGamePaused = !isGamePaused;
        }

        if (!isGameOver && (keyDown & KEY_A || hasCollision(bottomScreenBounds, touchBounds)))
        {
            gravity = player.impulse;

            shouldRotateUp = true;
            upRotationTimer = 1;
            downRotationTimer = 0;
            initialAngle = -20;
            // Mix_PlayChannel(-1, flapSound, 0);
        }

        else if (isGameOver && (keyDown & KEY_A || hasCollision(bottomScreenBounds, touchBounds)))
        {
            resetGame();
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
