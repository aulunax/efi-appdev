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
  init_global_variables(ImageHandle, SystemTable);

  EFI_INPUT_KEY key;
  EFI_STATUS status;
  GAME_GRAPHICS_LIB_DATA GraphicsLibData;


  while (1)
  {
    status = cin->ReadKeyStroke(cin, &key);
    if (key.ScanCode == SCAN_ESC)
    {
      break;
    }
    if(status == EFI_SUCCESS)
    {
      Print(L"Key pressed: %c\n", key.UnicodeChar);
    }
  }


  return EFI_SUCCESS;

}