#ifndef SNAKE_H
#define SNAKE_H
#include <Uefi.h>

typedef struct Point
{
    UINT32 x;
    UINT32 y;
} Point;

EFI_SIMPLE_TEXT_INPUT_PROTOCOL  *cin  = NULL; 
void initGlobalVariables(EFI_HANDLE handle, EFI_SYSTEM_TABLE *SystemTable)
{
    cin = SystemTable->ConIn;
}

void updateSnakePosition(Point* point, EFI_INPUT_KEY key)
{
    if(key.ScanCode == SCAN_RIGHT)
    {
        point->x += 10;
    }
    if(key.ScanCode == SCAN_LEFT)
    {
        point->x -= 10;
    }
    if(key.ScanCode == SCAN_UP)
    {
        point->y -= 10;
    }
    if(key.ScanCode == SCAN_DOWN)
    {
        point->y += 10;
    }
    else
    {
        return;
    }

}


#endif