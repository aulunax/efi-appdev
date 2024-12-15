/** @file
 * Snake game
**/

#include <Uefi.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/PrintLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/GameGraphicsLib.h>
#include <Snake.h>

// Global constant for console input
extern EFI_SIMPLE_TEXT_INPUT_PROTOCOL *cin;

// Entry point for the application so i use UEFI convention
EFI_STATUS EFIAPI SnakeMain(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  
  //initialize global variables
  initGlobalVariables(ImageHandle, SystemTable);

  EFI_INPUT_KEY key;
  EFI_STATUS status;
  GAME_GRAPHICS_LIB_DATA GraphicsLibData;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL Red = {0, 0, 255, 0};
  GAME_GRAPHICS_LIB_GRID MainGrid;
  Point snakeParts[MAX_SNAKE_SIZE]; 
  UINT32 snakeSize = 20;
  Direction direction = NONE;
  

  status = InitializeGraphicMode(&GraphicsLibData);
  if (status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_ERROR, "Failed to enable graphic mode.\n"));
    return status;
  }

  UINT32 screenWidth = GraphicsLibData.Screen.HorizontalResolution;
  UINT32 screenHeight = GraphicsLibData.Screen.VerticalResolution;

  status =  CreateCustomGrid(&MainGrid, screenWidth, screenHeight, HORIZONTAL_CELS, VERTICAL_CELS, NULL);
  if (status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_ERROR, "Failed to create grid.\n"));
    return status;
  }



  initSnake(snakeParts, snakeSize);
  ClearScreen(&GraphicsLibData);
  UpdateVideoBuffer(&GraphicsLibData);

  while (1)
  {
    status = cin->ReadKeyStroke(cin, &key);
    if (key.ScanCode == SCAN_ESC)
    {
      break;
    }
    if(status == EFI_SUCCESS)
    {
      changeDirection(key, &direction);
    }
    
    moveSnake(snakeParts, snakeSize, direction, screenWidth, screenHeight);

    ClearGrid(&MainGrid);
    status = drawSnake(&MainGrid, snakeParts, snakeSize, &Red);
    DrawGrid(&GraphicsLibData, &MainGrid, 0, 0);
    UpdateVideoBuffer(&GraphicsLibData);
    // Stall for 100ms
    gBS->Stall(100000);
  }

  DeleteGrid(&MainGrid);
  FinishGraphicMode(&GraphicsLibData);
  return EFI_SUCCESS;

}