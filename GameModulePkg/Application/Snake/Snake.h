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

}


#endif