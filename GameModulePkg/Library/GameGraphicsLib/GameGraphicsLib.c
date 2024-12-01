#include <Library/DebugLib.h>
#include <Uefi.h>
#include <Protocol/GraphicsOutput.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/GameGraphicsLib.h>

VOID
EFIAPI
MyLibraryFunction (
  VOID
  )
{
  DEBUG((DEBUG_INFO, "MyLibraryFunction called\n"));
}

EFI_STATUS
EFIAPI
PrintModeQueryInfo(
    IN EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *ModeInfo) 
{
  if (ModeInfo == NULL)
  {
    DEBUG((EFI_D_ERROR, "Invalid ModeInfo pointer.\n"));
    return EFI_ABORTED;
  }

  // Print general mode information
  DEBUG((EFI_D_INFO, "EFI_GRAPHICS_OUTPUT_MODE_INFORMATION:\n"));
  DEBUG((EFI_D_INFO, "  Version: %u\n", ModeInfo->Version));
  DEBUG((EFI_D_INFO, "  Horizontal Resolution: %u pixels\n", ModeInfo->HorizontalResolution));
  DEBUG((EFI_D_INFO, "  Vertical Resolution: %u pixels\n", ModeInfo->VerticalResolution));

  // Print pixel format
  switch (ModeInfo->PixelFormat)
  {
  case PixelRedGreenBlueReserved8BitPerColor:
    DEBUG((EFI_D_INFO, "  Pixel Format: PixelRedGreenBlueReserved8BitPerColor\n"));
    break;
  case PixelBlueGreenRedReserved8BitPerColor:
    DEBUG((EFI_D_INFO, "  Pixel Format: PixelBlueGreenRedReserved8BitPerColor\n"));
    break;
  case PixelBitMask:
    DEBUG((EFI_D_INFO, "  Pixel Format: PixelBitMask\n"));
    break;
  case PixelBltOnly:
    DEBUG((EFI_D_INFO, "  Pixel Format: PixelBltOnly\n"));
    break;
  default:
    DEBUG((EFI_D_INFO, "  Pixel Format: Unknown (%d)\n", ModeInfo->PixelFormat));
    break;
  }

  // Print pixel information (only if PixelFormat is PixelBitMask)
  if (ModeInfo->PixelFormat == PixelBitMask)
  {
    DEBUG((EFI_D_INFO, "  Pixel Bitmask:\n"));
    DEBUG((EFI_D_INFO, "    Red Mask: 0x%08X\n", ModeInfo->PixelInformation.RedMask));
    DEBUG((EFI_D_INFO, "    Green Mask: 0x%08X\n", ModeInfo->PixelInformation.GreenMask));
    DEBUG((EFI_D_INFO, "    Blue Mask: 0x%08X\n", ModeInfo->PixelInformation.BlueMask));
    DEBUG((EFI_D_INFO, "    Reserved Mask: 0x%08X\n", ModeInfo->PixelInformation.ReservedMask));
  }

  // Print pixels per scan line
  DEBUG((EFI_D_INFO, "  Pixels Per Scan Line: %u\n", ModeInfo->PixelsPerScanLine));

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
EnableGraphicMode(
    IN OUT GAME_GRAPHICS_LIB_DEV* Devices)
{
  EFI_STATUS Status;
  UINT32 ModeNumber = 0;
  UINTN SizeOfInfo = 0;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* Info;

  Status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID **)&Devices->GraphicsOutput);
  if (EFI_ERROR(Status))
  {
    return Status;
  }

  Status = Devices->GraphicsOutput->QueryMode(
      Devices->GraphicsOutput,
      ModeNumber,
      &SizeOfInfo,
      &Info);
  if (EFI_ERROR(Status))
  {
    return Status;
  }

  Status = PrintModeQueryInfo(Info);
  if (EFI_ERROR(Status))
  {
    return Status;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
DisableGraphicMode(
    IN GAME_GRAPHICS_LIB_DEV *Devices)
{
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
DrawSquare(
    IN GAME_GRAPHICS_LIB_DEV *Devices,
    IN INT32 x,
    IN INT32 y)
{
  return EFI_SUCCESS;
}