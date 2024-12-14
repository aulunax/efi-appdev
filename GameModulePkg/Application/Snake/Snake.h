#ifndef SNAKE_H
#define SNAKE_H
#include <Uefi.h>

#define MAX_SNAKE_SIZE 20

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
        point->y -= 1;
    }
    if(direction == DOWN)
    {
        point->y += 1;
    }
    if(direction == LEFT)
    {
        point->x -= 1;
    }
    if(direction == RIGHT)
    {
        point->x += 1;
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

void moveSnake(Point* snakeParts, UINT32 snakeSize, Direction direction)
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