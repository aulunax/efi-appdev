#ifndef SNAKE_H
#define SNAKE_H
#include <Uefi.h>

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

EFI_SIMPLE_TEXT_INPUT_PROTOCOL  *cin  = NULL; 
void initGlobalVariables(EFI_HANDLE handle, EFI_SYSTEM_TABLE *SystemTable)
{
    cin = SystemTable->ConIn;
}

void updateHead(Point* point, Direction direction)
{
    if(direction == UP)
    {
        if(point->y == 0)
        {
            point->y = HORIZONTAL_CELS - 1;
        }
        else
        {
            point->y -= 1;
        }
    }
    if(direction == DOWN)
    {
        if(point->y == VERTICAL_CELS - 1)
        {
            point->y = 0;
        }
        else
        {
            point->y += 1;
        }
    }
    if(direction == LEFT)
    {
        if(point->x == 0)
        {
            point->x = HORIZONTAL_CELS - 1;
        }
        else
        {
            point->x -= 1;
        }
    }
    if(direction == RIGHT)
    {
        if(point->x == HORIZONTAL_CELS - 1)
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

void initSnake(Point* snakeParts, UINT32 snakeSize)
{
    for(UINT32 i = 0; i < snakeSize; i++)
    {
        snakeParts[i].x = 0;
        snakeParts[i].y = 0;
    }
}


EFI_STATUS drawSnake(GAME_GRAPHICS_LIB_GRID* grid, Point* snakeParts, UINT32 snakeSize, EFI_GRAPHICS_OUTPUT_BLT_PIXEL* color)
{
    EFI_STATUS status;
    for(UINT32 i = 0; i < snakeSize; i++)
    {
        status = FillCellInGrid(grid, snakeParts[i].x, snakeParts[i].y, color);
        if(EFI_ERROR(status))
        {
            return status;
        }

    }
    return EFI_SUCCESS;
}

void moveSnake(Point* snakeParts, UINT32 snakeSize, Direction direction, UINT32 screenWidth, UINT32 screenHeight)
{
    for(UINT32 i = snakeSize - 1; i > 0; i--)
    {
        snakeParts[i] = snakeParts[i - 1];
    }
    updateHead(&snakeParts[0], direction);
}

void changeDirection(EFI_INPUT_KEY key, Direction* direction)
{
    if(key.ScanCode == SCAN_UP)
    {
        *direction = UP;
    }
    if(key.ScanCode == SCAN_DOWN)
    {
        *direction = DOWN;
    }
    if(key.ScanCode == SCAN_LEFT)
    {
        *direction = LEFT;
    }
    if(key.ScanCode == SCAN_RIGHT)
    {
        *direction = RIGHT;
    }
}


#endif