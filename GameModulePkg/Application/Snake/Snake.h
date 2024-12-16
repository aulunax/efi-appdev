#ifndef SNAKE_H
#define SNAKE_H

#include <Uefi.h>
#include <Library/RngLib.h>
#include <Protocol/Rng.h>

#define HORIZONTAL_CELS 50
#define VERTICAL_CELS 50
#define MAX_SNAKE_SIZE HORIZONTAL_CELS * VERTICAL_CELS

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

EFI_SIMPLE_TEXT_INPUT_PROTOCOL *cin = NULL;
UINT32 seedBase = 1;

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

void updateHead(Point *point, Direction direction)
{
    if (direction == UP)
    {
        if (point->y == 0)
        {
            point->y = HORIZONTAL_CELS - 1;
        }
        else
        {
            point->y -= 1;
        }
    }
    if (direction == DOWN)
    {
        if (point->y == VERTICAL_CELS - 1)
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
            point->x = HORIZONTAL_CELS - 1;
        }
        else
        {
            point->x -= 1;
        }
    }
    if (direction == RIGHT)
    {
        if (point->x == HORIZONTAL_CELS - 1)
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

void initSnake(Point *snakeParts, UINT32 snakeSize)
{
    for (UINT32 i = 0; i < snakeSize; i++)
    {
        snakeParts[i].x = 0;
        snakeParts[i].y = 0;
    }
}

EFI_STATUS drawSnake(GAME_GRAPHICS_LIB_GRID *grid, Point *snakeParts, UINT32 snakeSize, EFI_GRAPHICS_OUTPUT_BLT_PIXEL *color)
{
    EFI_STATUS status;
    for (UINT32 i = 0; i < snakeSize; i++)
    {
        status = FillCellInGrid(grid, snakeParts[i].x, snakeParts[i].y, color);
        if (EFI_ERROR(status))
        {
            return status;
        }
    }
    return EFI_SUCCESS;
}

void moveSnake(Point *snakeParts, UINT32 snakeSize, Direction direction, UINT32 screenWidth, UINT32 screenHeight)
{
    for (UINT32 i = snakeSize - 1; i > 0; i--)
    {
        snakeParts[i] = snakeParts[i - 1];
    }
    updateHead(&snakeParts[0], direction);
}

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

void generateRandomPoint(Point *point, Point *snakeParts, UINT32 snakeSize, GAME_GRAPHICS_LIB_GRID *grid, EFI_GRAPHICS_OUTPUT_BLT_PIXEL *color, UINT32 seed)
{
    point->x = simple_rng(seed, 0, HORIZONTAL_CELS - 1);
    point->y = simple_rng(seed, 0, VERTICAL_CELS - 1);
    while (checkIfPointIsInSnake(snakeParts, snakeSize, *point))
    {
        point->x = simple_rng(seed, 0, HORIZONTAL_CELS - 1);
        point->y = simple_rng(seed, 0, VERTICAL_CELS - 1);
    }
}

BOOLEAN checkIfSnakeAteFood(Point *snakeParts, UINT32 snakeSize, Point food)
{
    if (snakeParts[0].x == food.x && snakeParts[0].y == food.y)
    {
        return TRUE;
    }
    return FALSE;
}

void growSnake(Point *snakeParts, UINT32 *snakeSize, Point food)
{
    snakeParts[*snakeSize].x = food.x;
    snakeParts[*snakeSize].y = food.y;
    *snakeSize += 1;
}

void drawFood(GAME_GRAPHICS_LIB_GRID *grid, Point food, EFI_GRAPHICS_OUTPUT_BLT_PIXEL *color)
{
    FillCellInGrid(grid, food.x, food.y, color);
}

void printStartMessage(GAME_GRAPHICS_LIB_DATA *GraphicsLibData, EFI_GRAPHICS_OUTPUT_BLT_PIXEL White, EFI_GRAPHICS_OUTPUT_BLT_PIXEL Black, UINT32 screenWidth, UINT32 screenHeight)
{

    ClearScreen(GraphicsLibData);
    DrawText(GraphicsLibData, screenWidth/2 - 272,  screenHeight/2 - 32, "Welcome to Snake!", &White, &Black, 4);
    DrawText(GraphicsLibData, screenWidth/2 - 176, screenHeight/2 + 16, "Use arrow keys to move", &White, &Black, 2);
    DrawText(GraphicsLibData, screenWidth/2 - 200, screenHeight/2 + 48, "Press any key to start...", &White, &Black, 2);
    UpdateVideoBuffer(GraphicsLibData);
}

void printGameOverMessage(GAME_GRAPHICS_LIB_DATA *GraphicsLibData, EFI_GRAPHICS_OUTPUT_BLT_PIXEL White, EFI_GRAPHICS_OUTPUT_BLT_PIXEL Black, EFI_GRAPHICS_OUTPUT_BLT_PIXEL Red,  UINT32 screenWidth, UINT32 screenHeight, UINT32 score)
{
    CHAR8 textScore[16]; // Buffer to store the score as a string
    // Convert score (UINT32) to string
    AsciiSPrint(textScore, sizeof(textScore), "%u", score);
    ClearScreen(GraphicsLibData);
    DrawText(GraphicsLibData, screenWidth/2 - 160, screenHeight/2 - 32, "Game Over!", &Red, &Black, 4);
    DrawText(GraphicsLibData, screenWidth/2 - 184, screenHeight/2 + 16, "Score: ", &White, &Black, 2);
    DrawText(GraphicsLibData, screenWidth/2 - 72, screenHeight/2 + 16, textScore, &White, &Black, 2);
    DrawText(GraphicsLibData, screenWidth/2 - 200, screenHeight/2 + 48, "Press any key to restart...", &White, &Black, 2);
    UpdateVideoBuffer(GraphicsLibData);
}


#endif