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

GLOBAL_REMOVE_IF_UNREFERENCED EFI_STRING_ID mStringHelpTokenId = STRING_TOKEN(STR_TEST_HELP_INFORMATION);

EFI_STATUS
EFIAPI
SnakeMain(
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_EVENT FrameTimerEvent;
  UINT32 FrameCounter;
  UINT32 SubFramesCounter;
  EFI_STATUS Status;

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


    DEBUG((EFI_D_INFO, "Frame: %d, Subframes: %d\n",
           FrameCounter + 1,
           SubFramesCounter + 1));
    SubFramesCounter = 0;
    FrameCounter++;
  }

  return EFI_SUCCESS;
}