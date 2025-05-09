#include "graphics.h"
#include <vector>
#include <fstream>

const int TOP_SCREEN_WIDTH = 400;
const int BOTTOM_SCREEN_WIDTH = 320;
const int SCREEN_HEIGHT = 240;

C3D_RenderTarget *topScreen = nullptr;
C3D_RenderTarget *bottomScreen = nullptr;

// when working with animations I need to use sprites[].
C2D_Sprite birdSprites[3];

bool isGameOver;
bool isGamePaused;
float startGameTimer;

bool shouldRotateUp;
float downRotationTimer;
float upRotationTimer;

float gravity;

float rotationAngle;
int score;
int highScore;

SpriteRefreshInfo refreshInfo;
SpriteFrameInfo frameInfo;

Rectangle birdsBounds;
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

void saveScore()
{
    std::string path = "high-score.txt";

    std::ofstream highScores(path);

    std::string scoreString = std::to_string(score);
    highScores << scoreString;

    highScores.close();
}

int loadHighScore()
{
    std::string highScoreText;

    std::string path = "high-score.txt";

    std::ifstream highScores(path);

    if (!highScores.is_open())
    {
        saveScore();

        std::ifstream auxHighScores(path);

        getline(auxHighScores, highScoreText);

        highScores.close();

        int highScore = stoi(highScoreText);

        return highScore;
    }

    getline(highScores, highScoreText);

    highScores.close();

    int highScore = stoi(highScoreText);

    return highScore;
}

void resetGame()
{
    if (score > highScore)
    {
        highScore = score;
        saveScore();
    }

    isGameOver = false;
    isGamePaused = false;
    startGameTimer = 0;
    score = 0;
    rotationAngle = 0;
    player.sprite.bounds.x = TOP_SCREEN_WIDTH / 2;
    player.sprite.bounds.y = SCREEN_HEIGHT / 2;
    gravity = 0;
    pipes.clear();
    bottomScreenPipes.clear();
}

void handleBirdRotation()
{
    downRotationTimer++;

    if (downRotationTimer < 10)
    {
        handleSpriteRotation(birdSprites, player.sprite.bounds, frameInfo.SpritesQuantity, rotationAngle);
    }

    if (shouldRotateUp)
    {
        if (upRotationTimer > 0)
        {
            upRotationTimer--;
        }

        if (upRotationTimer <= 0)
        {
            shouldRotateUp = false;
        }

        handleSpriteRotation(birdSprites, player.sprite.bounds, frameInfo.SpritesQuantity, rotationAngle);
    }

    if (downRotationTimer > 10)
    {
        if (rotationAngle <= 1.570796 && !isGameOver && !isGamePaused)
        {
            rotationAngle += 0.1;
        }

        handleSpriteRotation(birdSprites, player.sprite.bounds, frameInfo.SpritesQuantity, rotationAngle);
    }
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

    handleSpriteRotation(birdSprites, player.sprite.bounds, frameInfo.SpritesQuantity, rotationAngle);

    if (startGameTimer > 40)
    {
        handleBirdRotation();
    }
    else
    {
        handleSpriteRotation(birdSprites, player.sprite.bounds, frameInfo.SpritesQuantity, rotationAngle);
    }
}

void renderTopScreen()
{
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C2D_TargetClear(topScreen, BLACK);
    C2D_SceneBegin(topScreen);

    topBackgroundSprite.bounds.x = 0;
    drawSprite(topBackgroundSprite);

    topBackgroundSprite.bounds.x = topBackgroundSprite.bounds.w;
    drawSprite(topBackgroundSprite);

    groundSprite.bounds.x = 0;
    drawSprite(groundSprite);

    groundSprite.bounds.x = groundSprite.bounds.w;
    drawSprite(groundSprite);

    for (Pipe &pipe : pipes)
    {
        if (!pipe.isDestroyed)
        {
            drawSprite(pipe.sprite);
        }
    }

    if (highScore < 10)
    {
        highScoreNumbers[highScore].bounds.x = 50;
        drawSprite(highScoreNumbers[highScore]);
    }
    else
    {
        int tens = (int)(highScore / 10);
        int units = (int)(highScore % 10);

        highScoreNumberTens[tens].bounds.x = 25;
        highScoreNumbers[units].bounds.x = 50;

        drawSprite(highScoreNumberTens[tens]);
        drawSprite(highScoreNumbers[units]);
    }

    if (score < 10)
    {
        drawSprite(numbers[score]);
    }
    else
    {
        int tens = (int)(score / 10);
        int units = (score % 10);

        numberTens[tens].bounds.x = TOP_SCREEN_WIDTH / 2 - 25;

        drawSprite(numberTens[tens]);
        drawSprite(numbers[units]);
    }

    for (Vector2 &groundPosition : groundPositions)
    {
        groundSprite.bounds.x = groundPosition.x;
        drawSprite(groundSprite);
    }

    if (!isGameOver)
    {
        drawSpriteAnimation(birdSprites, refreshInfo, frameInfo);
    }
    else
    {
        drawAndRotateImage(player.sprite, rotationAngle);
    }

    C3D_FrameEnd(0);
}

void renderBottomScreen()
{
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C2D_TargetClear(bottomScreen, BLACK);
    C2D_SceneBegin(bottomScreen);

    bottomBackgroundSprite.bounds.x = 0;
    bottomBackgroundSprite.bounds.y = 0;
    drawSprite(bottomBackgroundSprite);

    bottomBackgroundSprite.bounds.x = bottomBackgroundSprite.bounds.w;
    bottomBackgroundSprite.bounds.y = 0;
    drawSprite(bottomBackgroundSprite);

    for (Pipe &pipe : bottomScreenPipes)
    {
        if (!pipe.isDestroyed)
        {
            drawSprite(pipe.sprite);
        }
    }

    if (isGamePaused || isGameOver)
    {
        drawSprite(startGameSprite);
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
        std::string completePath = std::to_string(i) + fileExtension;

        Sprite numberSprite = loadSprite(completePath.c_str(), TOP_SCREEN_WIDTH / 2, 15, 24, 36);

        numbers.push_back(numberSprite);
        numberTens.push_back(numberSprite);

        highScoreNumbers.push_back(numberSprite);
        highScoreNumberTens.push_back(numberSprite);
    }
}

void initializeSpriteAnimations(C2D_SpriteSheet &spriteSheet, C2D_Sprite *sprites, int positionX, int positionY)
{
    frameInfo.SpritesQuantity = C2D_SpriteSheetCount(spriteSheet);

    // Set the first sprite to the beginning of the spriteSheet
    frameInfo.currentFrameIndex = 0;
    frameInfo.shouldLoopOnce = false;

    // init sprites
    for (size_t index = 0; index < frameInfo.SpritesQuantity; index++)
    {
        // Load the spriteheet to each sprites (or frames)
        C2D_SpriteFromSheet(&sprites[index], spriteSheet, index);
        // Set the position, and rotation of the object
        C2D_SpriteSetPos(&sprites[index], positionX, positionY);
        C2D_SpriteSetRotationDegrees(&sprites[index], 0);
    }

    // Set initial value
    refreshInfo.start = osGetTime();
    refreshInfo.elapsed = 0;

    int refreshTime = 140;

    // Set a desired sprite refresh time (ms)
    if (refreshTime < ANIMATION_REFRESH_TIME_MIN)
    {
        refreshInfo.refreshTime = ANIMATION_REFRESH_TIME_MIN;
    }
    else
    {
        refreshInfo.refreshTime = refreshTime;
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

    C2D_SpriteSheet birdsSpriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/bird-sprites.t3x");

    initializeSpriteAnimations(birdsSpriteSheet, birdSprites, TOP_SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

    loadNumbersSprites();

    Sprite playerSprite = loadSprite("yellowbird-midflap.t3x", TOP_SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 34, 24);
    player = Player{playerSprite, -6, 1};

    highScore = loadHighScore();

    upPipeSprite = loadSprite("pipe-green-180.t3x", TOP_SCREEN_WIDTH / 2, -220, 52, 320);
    downPipeSprite = loadSprite("pipe-green.t3x", TOP_SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 52, 320);

    startGameSprite = loadSprite("message.t3x", BOTTOM_SCREEN_WIDTH / 2 - 90, -80, 184, 267);

    topBackgroundSprite = loadSprite("background-day.t3x", 0, -250, 288, 512);
    bottomBackgroundSprite = topBackgroundSprite;

    groundSprite = loadSprite("base.t3x", 0, 0, 336, 112);

    float groundYPosition = SCREEN_HEIGHT - groundSprite.bounds.h + 50;

    groundSprite.bounds.y = groundYPosition;

    groundCollisionBounds = {0, groundYPosition, 0, SCREEN_HEIGHT, groundSprite.bounds.h, WHITE};

    groundPositions.push_back({0, groundYPosition});
    groundPositions.push_back({groundSprite.bounds.w, groundYPosition});
    groundPositions.push_back({groundSprite.bounds.w * 2, groundYPosition});

    srand(time(NULL));

    while (aptMainLoop())
    {
        hidScanInput();

        int keyDown = hidKeysDown();

        // just use keyDown & KEY_TOUCH to detect touch in the screen
        if (keyDown & KEY_START || (isGamePaused && keyDown & KEY_TOUCH))
        {
            isGamePaused = !isGamePaused;
        }

        if (!isGameOver && (keyDown & KEY_A || keyDown & KEY_TOUCH))
        {
            gravity = player.impulse;

            shouldRotateUp = true;
            upRotationTimer = 1;
            downRotationTimer = 0;
            rotationAngle = -0.3490659;
            // Mix_PlayChannel(-1, flapSound, 0);
        }

        else if (isGameOver && (keyDown & KEY_A || keyDown & KEY_TOUCH))
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

    C2D_SpriteSheetFree(birdsSpriteSheet);
    C2D_SpriteSheetFree(startGameSprite.sheet);
    C2D_SpriteSheetFree(topBackgroundSprite.sheet);
    C2D_SpriteSheetFree(bottomBackgroundSprite.sheet);
    C2D_SpriteSheetFree(upPipeSprite.sheet);
    C2D_SpriteSheetFree(downPipeSprite.sheet);
    C2D_SpriteSheetFree(groundSprite.sheet);
    C2D_SpriteSheetFree(playerSprite.sheet);
    C2D_SpriteSheetFree(player.sprite.sheet);

    for (Sprite &number : numbers)
    {
        C2D_SpriteSheetFree(number.sheet);
    }

    for (Sprite &number : numberTens)
    {
        C2D_SpriteSheetFree(number.sheet);
    }

    for (Sprite &number : highScoreNumbers)
    {
        C2D_SpriteSheetFree(number.sheet);
    }

    for (Sprite &number : highScoreNumberTens)
    {
        C2D_SpriteSheetFree(number.sheet);
    }

    C2D_Fini();
    C3D_Fini();
    gfxExit();
    romfsExit();
}
