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
GLOBAL_REMOVE_IF_UNREFERENCED EFI_STRING_ID  mStringHelpTokenId = STRING_TOKEN (STR_TEST_HELP_INFORMATION);

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
UefiGraphicsLibTestMain(
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS Status;
    GAME_GRAPHICS_LIB_DATA GraphicsLibData;
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL Red = {0, 0, 255, 0};

    MyLibraryFunction();

    Status = InitializeGraphicMode(&GraphicsLibData);
    if (Status != EFI_SUCCESS)
    {
        DEBUG((EFI_D_ERROR, "Failed to enable graphic mode.\n"));
        return Status;
    }

    Status = DrawRectangle(
        &GraphicsLibData,
        100, 100,
        100, 50,
        &Red);
    if (Status != EFI_SUCCESS)
    {
      DEBUG((EFI_D_ERROR, "Failed to draw rectangle.\n"));
      return Status;
    }

    Status = DrawRectangle(
        &GraphicsLibData,
        100, 300,
        20, 2000,
        &Red);
    if (Status != EFI_SUCCESS)
    {
      DEBUG((EFI_D_ERROR, "Failed to draw rectangle.\n"));
      return Status;
    }

    Status = UpdateVideoBuffer(&GraphicsLibData);
    if (Status != EFI_SUCCESS)
    {
        DEBUG((EFI_D_ERROR, "Failed to update video buffer.\n"));
      return Status;
    }

    gBS->Stall(3000000);

    Status = ClearScreen(&GraphicsLibData);
    if (Status != EFI_SUCCESS)
    {
      DEBUG((EFI_D_ERROR, "Failed to clear screen.\n"));
      return Status;
    }

    Status = FinishGraphicMode(&GraphicsLibData);
    if (Status != EFI_SUCCESS) {
        DEBUG((EFI_D_ERROR, "Failed to disable graphic mode.\n"));
        return Status;
    }

    // Test Debug statement
    DEBUG((EFI_D_INFO, "My Entry point: 0x%08x\r\n", (CHAR16 *)UefiGraphicsLibTestMain));

    return EFI_SUCCESS;
}
