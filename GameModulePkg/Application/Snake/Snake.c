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

  status = InitializeGraphicMode(&GraphicsLibData);
  if (status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_ERROR, "Failed to enable graphic mode.\n"));
    return status;
  }

  status =  CreateCustomGrid(&MainGrid, 800, 600, 50, 50, &Red);
  if (status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_ERROR, "Failed to create grid.\n"));
    return status;
  }



  Point snakePosition = {0, 0};
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
      updateSnakePosition(&snakePosition, key);
    }
    
    ClearGrid(&MainGrid);
    status = FillCellInGrid(&MainGrid, snakePosition.x, snakePosition.y, &Red);
    DrawGrid(&GraphicsLibData, &MainGrid, 0, 0);
    UpdateVideoBuffer(&GraphicsLibData);
  }

  DeleteGrid(&MainGrid);
  FinishGraphicMode(&GraphicsLibData);
  return EFI_SUCCESS;

}