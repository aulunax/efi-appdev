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

EFI_SIMPLE_TEXT_INPUT_PROTOCOL  *cin  = NULL; 
void initGlobalVariables(EFI_HANDLE handle, EFI_SYSTEM_TABLE *SystemTable)
{
    cin = SystemTable->ConIn;
}

UINT32 RandomRange(UINT32 Min, UINT32 Max)
{
    EFI_RNG_PROTOCOL *RngProtocol = NULL;
    EFI_STATUS Status;
    UINT32 RandomValue = 0;

    // Locate the RNG Protocol
    Status = gBS->LocateProtocol(&gEfiRngProtocolGuid, NULL, (VOID **)&RngProtocol);
    if (EFI_ERROR(Status))
    {
        return Min; // Fallback to Min if RNG is not available
    }

    // Generate a random number
    Status = RngProtocol->GetRNG(RngProtocol, NULL, sizeof(RandomValue), (UINT8 *)&RandomValue);
    if (EFI_ERROR(Status))
    {
        return Min; // Fallback to Min if RNG generation fails
    }

    // Scale the random number to the desired range
    if (Max > Min)
    {
        RandomValue = Min + (RandomValue % (Max - Min + 1));
    }

    return RandomValue;
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
        if(*direction == DOWN)
        {
            return;
        }
        *direction = UP;
    }
    if(key.ScanCode == SCAN_DOWN)
    {
        if(*direction == UP)
        {
            return;
        }

        *direction = DOWN;
    }
    if(key.ScanCode == SCAN_LEFT)
    {
        if(*direction == RIGHT)
        {
            return;
        }

        *direction = LEFT;
    }
    if(key.ScanCode == SCAN_RIGHT)
    {
        if(*direction == LEFT)
        {
            return;
        }

        *direction = RIGHT;
    }
}

BOOLEAN checkCollision(Point* snakeParts, UINT32 snakeSize)
{
    for(UINT32 i = 1; i < snakeSize; i++)
    {
        if(snakeParts[0].x == snakeParts[i].x && snakeParts[0].y == snakeParts[i].y)
        {
            return TRUE;
        }
    }
    return FALSE;
}

BOOLEAN checkIfPointIsInSnake(Point* snakeParts, UINT32 snakeSize, Point point)
{
    for(UINT32 i = 0; i < snakeSize; i++)
    {
        if(snakeParts[i].x == point.x && snakeParts[i].y == point.y)
        {
            return TRUE;
        }
    }
    return FALSE;
}

void generateRandomPoint(Point* point, Point* snakeParts, UINT32 snakeSize, GAME_GRAPHICS_LIB_GRID* grid, EFI_GRAPHICS_OUTPUT_BLT_PIXEL* color)
{
    point->x = (UINT32)RandomRange(0, HORIZONTAL_CELS - 1);
    point->y = (UINT32)RandomRange(0, VERTICAL_CELS - 1);
    while(checkIfPointIsInSnake(snakeParts, snakeSize, *point))
    {
        point->x = (UINT32)RandomRange(0, HORIZONTAL_CELS - 1);
        point->y = (UINT32)RandomRange(0, VERTICAL_CELS - 1);
    }
    FillCellInGrid(grid, point->x, point->y, color);
}


#endif