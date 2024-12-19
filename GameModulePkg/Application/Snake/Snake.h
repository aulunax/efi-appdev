#ifndef SNAKE_H
#define SNAKE_H

#include <Uefi.h>
#include <Library/RngLib.h>
#include <Protocol/Rng.h>

#define HORIZONTAL_CELLS 60
#define VERTICAL_CELLS 50
#define MAX_SNAKE_SIZE HORIZONTAL_CELLS *VERTICAL_CELLS

#define FPS_DISPLAY_RATE_SECONDS 3
#define FPS_DISPLAY_RATE (FPS_DISPLAY_RATE_SECONDS * 10000000)

typedef struct Point
{
    UINT32 x;
    UINT32 y;
} Point;

typedef enum Direction
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
    NONE
} Direction;

typedef struct FPS_CONTEXT
{
    UINT32 *FrameCount;
    GAME_GRAPHICS_LIB_DATA *Data;
} FPS_CONTEXT;

EFI_SIMPLE_TEXT_INPUT_PROTOCOL *cin = NULL;
UINT32 seedBase = 1;
Point SnakeBeforeBack = {0, 0};
BOOLEAN snakeAteFood = FALSE;

EFI_GRAPHICS_OUTPUT_BLT_PIXEL gBlack = {0, 0, 0, 0};
EFI_GRAPHICS_OUTPUT_BLT_PIXEL gWhite = {255, 255, 255, 0};

void initGlobalVariables(EFI_HANDLE handle, EFI_SYSTEM_TABLE *SystemTable)
{
    cin = SystemTable->ConIn;
}

UINT32 simple_rng(UINT32 seed, UINT32 min, UINT32 max)
{
    // Linear Congruential Generator (LCG) formula
    seedBase += seed;
    seedBase = (seedBase * 1664525 + 1013904223); // LCG constants for better randomness

    return min + (seedBase % (max - min + 1)); // Ensure the result is between min and max
}

/// @brief Moves the head of the snake to the new position based on the current direction
/// @param point The point that will be updated. Preferably the head of the snake
/// @param direction The current direction of the snake
void updateHead(Point *point, Direction direction)
{
    if (direction == UP)
    {
        if (point->y == 0)
        {
            point->y = VERTICAL_CELLS - 1;
        }
        else
        {
            point->y -= 1;
        }
    }
    if (direction == DOWN)
    {
        if (point->y == VERTICAL_CELLS - 1)
        {
            point->y = 0;
        }
        else
        {
            point->y += 1;
        }
    }
    if (direction == LEFT)
    {
        if (point->x == 0)
        {
            point->x = HORIZONTAL_CELLS - 1;
        }
        else
        {
            point->x -= 1;
        }
    }
    if (direction == RIGHT)
    {
        if (point->x == HORIZONTAL_CELLS - 1)
        {
            point->x = 0;
        }
        else
        {
            point->x += 1;
        }
    }
    else
    {
        return;
    }
}

/// @brief Initializes the snake parts to the default position
/// @param snakeParts Array of points that represent the snake
/// @param snakeSize Size of the snake
void initSnake(Point *snakeParts, UINT32 snakeSize)
{
    for (UINT32 i = 0; i < snakeSize; i++)
    {
        snakeParts[i].x = 0;
        snakeParts[i].y = 0;
    }
}

/// @brief Draws the snake on the grid
/// @param grid The grid that will be drawn on
/// @param snakeParts Array of points that represent the snake
/// @param snakeSize Size of the snake
/// @param color The color of the snake
/// @return EFI_SUCCESS if the function executed successfully, otherwise an error code.
EFI_STATUS drawSnake(GAME_GRAPHICS_LIB_GRID *grid, Point *snakeParts, UINT32 snakeSize, EFI_GRAPHICS_OUTPUT_BLT_PIXEL *color)
{
    EFI_STATUS status;
    if (!snakeAteFood)
    {
        FillCellInGrid(grid, SnakeBeforeBack.x, SnakeBeforeBack.y, &gBlack);
    }

    // filling in last and first cell of the snake
    status = FillCellInGrid(grid, snakeParts[0].x, snakeParts[0].y, color);
    if (EFI_ERROR(status))
    {
        return status;
    }
    status = FillCellInGrid(grid, snakeParts[snakeSize - 1].x, snakeParts[snakeSize - 1].y, color);
    if (EFI_ERROR(status))
    {
        return status;
    }

    return EFI_SUCCESS;
}

/// @brief Moves every snake part to the position of the part in front of it
/// @param snakeParts Array of points that represent the snake
/// @param snakeSize Size of the snake
/// @param direction Current direction of the snake
/// @param screenWidth Width of the screen
/// @param screenHeight Height of the screen
/// @note Also updates the position of the part of the snake that was at the back of the snake during the last move
void moveSnake(Point *snakeParts, UINT32 snakeSize, Direction direction, UINT32 screenWidth, UINT32 screenHeight)
{
    if (!snakeAteFood)
    {
        SnakeBeforeBack = snakeParts[snakeSize - 1];
    }
    for (UINT32 i = snakeSize - 1; i > 0; i--)
    {
        snakeParts[i] = snakeParts[i - 1];
    }

    updateHead(&snakeParts[0], direction);
}

/// @brief Sets the new direction of the snake based on the current direction and the next direction
/// @param direction Pointer to the current direction of the snake. Can be updated by the function
/// @param nextDirection The next direction that the snake will take
void setNewDirection(Direction *direction, Direction nextDirection)
{
    if (*direction == NONE)
    {
        *direction = nextDirection;
    }
    if (*direction == UP && nextDirection == DOWN)
    {
        return;
    }
    if (*direction == DOWN && nextDirection == UP)
    {
        return;
    }
    if (*direction == LEFT && nextDirection == RIGHT)
    {
        return;
    }
    if (*direction == RIGHT && nextDirection == LEFT)
    {
        return;
    }
    *direction = nextDirection;
}

/// @brief Changes the direction of the snake based on the key pressed
/// @param key The key that was pressed
/// @param direction Pointer to a direction variable
/// @note The direction variable will passed to this should be the next direction of the snake,
///       not the current direction
void changeDirection(EFI_INPUT_KEY key, Direction *direction)
{
    if (key.ScanCode == SCAN_UP)
    {
        if (*direction == DOWN)
        {
            return;
        }
        *direction = UP;
    }
    if (key.ScanCode == SCAN_DOWN)
    {
        if (*direction == UP)
        {
            return;
        }

        *direction = DOWN;
    }
    if (key.ScanCode == SCAN_LEFT)
    {
        if (*direction == RIGHT)
        {
            return;
        }

        *direction = LEFT;
    }
    if (key.ScanCode == SCAN_RIGHT)
    {
        if (*direction == LEFT)
        {
            return;
        }

        *direction = RIGHT;
    }
}

/// @brief Checks if the snake collided with itself
/// @param snakeParts Array of points that represent the snake
/// @param snakeSize Size of the snake
/// @return TRUE if the snake collided with itself, otherwise FALSE
BOOLEAN checkCollision(Point *snakeParts, UINT32 snakeSize)
{
    for (UINT32 i = 1; i < snakeSize; i++)
    {
        if (snakeParts[0].x == snakeParts[i].x && snakeParts[0].y == snakeParts[i].y)
        {
            return TRUE;
        }
    }
    return FALSE;
}

/// @brief Checks if a point is in the snake
/// @param snakeParts Array of points that represent the snake
/// @param snakeSize Size of the snake
/// @param point Point to check
/// @return TRUE if the point is in the snake, otherwise FALSE
/// @note This function is used to check if the food is in the snake. Used for point generation
BOOLEAN checkIfPointIsInSnake(Point *snakeParts, UINT32 snakeSize, Point point)
{
    for (UINT32 i = 0; i < snakeSize; i++)
    {
        if (snakeParts[i].x == point.x && snakeParts[i].y == point.y)
        {
            return TRUE;
        }
    }
    return FALSE;
}

/// @brief Generates a random point that is not in the snake
/// @param point Pointer to the point that will be generated. The point will be updated by the function
/// @param snakeParts Array of points that represent the snake
/// @param snakeSize Size of the snake
/// @param seed Seed for the random number generator.
void generateRandomPoint(Point *point, Point *snakeParts, UINT32 snakeSize, UINT32 seed)
{
    point->x = simple_rng(seed, 0, HORIZONTAL_CELLS - 1);
    point->y = simple_rng(seed, 0, VERTICAL_CELLS - 1);
    while (checkIfPointIsInSnake(snakeParts, snakeSize, *point))
    {
        point->x = simple_rng(seed, 0, HORIZONTAL_CELLS - 1);
        point->y = simple_rng(seed, 0, VERTICAL_CELLS - 1);
    }
}

/// @brief Checks if the snake ate the food in the current game state
/// @param snakeParts Array of points that represent the snake
/// @param snakeSize Size of the snake
/// @param food Point that represents the food
/// @return TRUE if the snake head is inside the food, otherwise FALSE
BOOLEAN checkIfSnakeAteFood(Point *snakeParts, UINT32 snakeSize, Point food)
{
    if (snakeParts[0].x == food.x && snakeParts[0].y == food.y)
    {
        return TRUE;
    }
    return FALSE;
}

/// @brief Draws the food on the grid
/// @param grid The grid that will be drawn on
/// @param food The point that represents the food
/// @param color The color of the food
void drawFood(GAME_GRAPHICS_LIB_GRID *grid, Point food, EFI_GRAPHICS_OUTPUT_BLT_PIXEL *color)
{
    FillCellInGrid(grid, food.x, food.y, color);
}

/// @brief Prints the start message of the game
/// @param GraphicsLibData The data structure that is used to store the library variables
/// @param White The color white
/// @param Black The color black
/// @param screenWidth Width of the screen
/// @param screenHeight Height of the screen
void printStartMessage(GAME_GRAPHICS_LIB_DATA *GraphicsLibData, EFI_GRAPHICS_OUTPUT_BLT_PIXEL White, EFI_GRAPHICS_OUTPUT_BLT_PIXEL Black, UINT32 screenWidth, UINT32 screenHeight)
{
    ClearScreen(GraphicsLibData);
    DrawText(GraphicsLibData, screenWidth / 2 - 272, screenHeight / 2 - 32, "Welcome to Snake!", &White, &Black, 4);
    DrawText(GraphicsLibData, screenWidth / 2 - 176, screenHeight / 2 + 16, "Use arrow keys to move", &White, &Black, 2);
    DrawText(GraphicsLibData, screenWidth / 2 - 200, screenHeight / 2 + 48, "Press any key to start...", &White, &Black, 2);
    UpdateVideoBuffer(GraphicsLibData);
}

/// @brief Prints the game over message
/// @param GraphicsLibData The data structure that is used to store the library variables
/// @param White The color white
/// @param Black The color black
/// @param Red The color red
/// @param screenWidth Width of the screen
/// @param screenHeight Height of the screen
/// @param score The current score
void printGameOverMessage(GAME_GRAPHICS_LIB_DATA *GraphicsLibData, EFI_GRAPHICS_OUTPUT_BLT_PIXEL White, EFI_GRAPHICS_OUTPUT_BLT_PIXEL Black, EFI_GRAPHICS_OUTPUT_BLT_PIXEL Red, UINT32 screenWidth, UINT32 screenHeight, UINT32 score)
{
    CHAR8 textScore[16];

    AsciiSPrint(textScore, sizeof(textScore), "%u", score);
    ClearScreen(GraphicsLibData);
    DrawText(GraphicsLibData, screenWidth / 2 - 160, screenHeight / 2 - 32, "Game Over!", &Red, &Black, 4);
    DrawText(GraphicsLibData, screenWidth / 2 - 200, screenHeight / 2 + 16, "Score: ", &White, &Black, 2);
    DrawText(GraphicsLibData, screenWidth / 2 - 72, screenHeight / 2 + 16, textScore, &White, &Black, 2);
    DrawText(GraphicsLibData, screenWidth / 2 - 200, screenHeight / 2 + 48, "Press any key to continue...", &White, &Black, 2);
    UpdateVideoBuffer(GraphicsLibData);
}

/// @brief Draws the score on the screen at constant location
/// @param GraphicsLibData The data structure that is used to store the library variables
/// @param White The color white
/// @param Black The color black
/// @param score The current score
/// @param screenWidth Width of the screen
/// @param screenHeight Height of the screen
void drawScore(GAME_GRAPHICS_LIB_DATA *GraphicsLibData, EFI_GRAPHICS_OUTPUT_BLT_PIXEL White, EFI_GRAPHICS_OUTPUT_BLT_PIXEL Black, UINT32 score, UINT32 screenWidth, UINT32 screenHeight)
{
    CHAR8 textScore[16];

    AsciiSPrint(textScore, sizeof(textScore), "%u", score);
    DrawText(GraphicsLibData, 0, 8, "Score: ", &White, &Black, 2);
    DrawText(GraphicsLibData, 112, 8, textScore, &White, &Black, 2);
}

/// @brief Displays the frames per second counter on the screen
/// @param GraphicsLibData The data structure that is used to store the library variables
/// @param frames Pointer to the frame count
/// @note This function is called inside the FPS display callback
void displayFpsCounter(GAME_GRAPHICS_LIB_DATA *GraphicsLibData, UINT32 *frames)
{
    UINT32 FPS = *frames / FPS_DISPLAY_RATE_SECONDS;
    CHAR8 textFPS[16];

    AsciiSPrint(textFPS, sizeof(textFPS), "%u", FPS);
    DrawText(GraphicsLibData, GraphicsLibData->Screen.HorizontalResolution - 120, 8, "FPS: ", &gWhite, &gBlack, 2);
    DrawText(GraphicsLibData, GraphicsLibData->Screen.HorizontalResolution - 56, 8, textFPS, &gWhite, &gBlack, 2);
    SmartUpdateVideoBuffer(GraphicsLibData, GraphicsLibData->Screen.HorizontalResolution - 120, 8, 120, 16);
}

/// @brief Callback function for the FPS display event
/// @param Event The event that was signaled
/// @param Context The context that was passed to the event. Contains the frame count and the graphics library data. @see FPS_CONTEXT
/// @note Also sets frames counter back to 0
VOID EFIAPI FpsDisplayCallback(IN EFI_EVENT Event, IN VOID *Context)
{
    UINT32 *Frames = ((FPS_CONTEXT *)Context)->FrameCount;
    GAME_GRAPHICS_LIB_DATA *data = ((FPS_CONTEXT *)Context)->Data;

    displayFpsCounter(data, Frames);
    *Frames = 0;
}

#endif