/** @file
  This test application is meant as a playing ground for me to learn stuff.

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

//
// String token ID of help message text.
// Shell supports to find help message in the resource section of an application image if
// .MAN file is not found. This global variable is added to make build tool recognizes
// that the help string is consumed by user and then build tool will add the string into
// the resource section. Thus the application can use '-?' option to show help message in
// Shell.
//
GLOBAL_REMOVE_IF_UNREFERENCED EFI_STRING_ID mStringHelpTokenId = STRING_TOKEN(STR_TEST_HELP_INFORMATION);

/**
  The user Entry Point for Application. The user code starts with this function
  as the real entry point for the application.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
UefiTestMain(
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_EVENT FrameTimerEvent;
  UINT32 FrameCounter;
  UINT32 SubFramesCounter;
  EFI_STATUS Status;
  GAME_GRAPHICS_LIB_DATA GraphicsLibData;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL Red = {0, 0, 255, 0};
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL Black = {0, 0, 0, 0};
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL LightGray = {192, 192, 192, 0};
  GAME_GRAPHICS_LIB_GRID MainGrid;

  // Create a timer event
  Status = gBS->CreateEvent(EVT_TIMER, TPL_NOTIFY, NULL, NULL, &FrameTimerEvent);
  if (EFI_ERROR(Status))
  {
    Print(L"Failed to create timer event: %r\n", Status);
    return Status;
  }

  // Set the timer to trigger in 1/30th of a second (about 33.33 milliseconds)
  Status = gBS->SetTimer(FrameTimerEvent, TimerPeriodic, 10000000 / PcdGet32(PcdTestFramerate));
  if (EFI_ERROR(Status))
  {
    Print(L"Failed to set timer: %r\n", Status);
    gBS->CloseEvent(FrameTimerEvent);
    return Status;
  }

  Status = InitializeGraphicMode(&GraphicsLibData);
  if (Status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_ERROR, "Failed to enable graphic mode.\n"));
    return Status;
  }

  ClearScreen(&GraphicsLibData);
  UpdateVideoBuffer(&GraphicsLibData);

  FrameCounter = 0;
  SubFramesCounter = 0;
  // Loop to perform the operation at constant intervals
  // This will be smth like a main game loop
  while (FrameCounter < PcdGet32(PcdTestTimes))
  {

    // This is timer for checking whether its time to finish the frame
    Status = gBS->CheckEvent(FrameTimerEvent);
    if ((RETURN_STATUS)Status == EFI_NOT_READY)
    {
      SubFramesCounter++;

      //
      // stuff here happens multiple times in a 'frame' so
      // do stuff like input handling here
      //
      continue;
    }
    else if (EFI_ERROR(Status))
    {
      DEBUG((EFI_D_ERROR, "Failed to check timer event: %r\n", Status));
      gBS->CloseEvent(FrameTimerEvent);
      return Status;
    }

    //
    // Here goes stuff that happens once during a frame,
    // so like rendering, game logic etc.
    //
    ClearScreen(&GraphicsLibData);

    CreateCustomGrid(&MainGrid,
                     600, 600,
                     21, 21,
                     NULL);

    for (INT32 i = 0; i < MainGrid.VerticalCellsCount; i++)
    {
      for (INT32 j = 0; j < MainGrid.HorizontalCellsCount; j++)
      {
        if ((i + j) % 2 == 0)
        {
          FillCellInGrid(&MainGrid, i, j, &Red);
        }
      }
    }

    DrawGrid(&GraphicsLibData,
             &MainGrid,
             100 + FrameCounter * 12,
             100 + FrameCounter * 6);

    DeleteGrid(&MainGrid);

    DrawText(&GraphicsLibData,
             8, 8,
             "This is a random string of text!",
             &LightGray,
             &Black,
             2);

    UpdateVideoBuffer(&GraphicsLibData);

    // DEBUG ((EFI_D_INFO,"Frame finished."));
    DEBUG((EFI_D_INFO, "Frame: %d, Subframes: %d\n",
           FrameCounter + 1,
           SubFramesCounter + 1));
    SubFramesCounter = 0;
    FrameCounter++;
  }

  ClearScreen(&GraphicsLibData);
  UpdateVideoBuffer(&GraphicsLibData);

  // Clean up
  gBS->CloseEvent(FrameTimerEvent);
  FinishGraphicMode(&GraphicsLibData);

  return EFI_SUCCESS;
}
