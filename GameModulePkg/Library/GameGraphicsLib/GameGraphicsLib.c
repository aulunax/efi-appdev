#include <Library/DebugLib.h>
#include <Uefi.h>
#include <Protocol/GraphicsOutput.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/GameGraphicsLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>

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
PrintGraphicsOutputProtocolMode(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE *Mode)
{
  if (Mode == NULL)
  {
    DEBUG((DEBUG_ERROR, "Invalid EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE pointer (NULL).\n"));
    return EFI_ABORTED;
  }

  DEBUG((DEBUG_INFO, "EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE:\n"));
  DEBUG((DEBUG_INFO, "  MaxMode: %u\n", Mode->MaxMode));
  DEBUG((DEBUG_INFO, "  Current Mode: %u\n", Mode->Mode));
  DEBUG((DEBUG_INFO, "  SizeOfInfo: %lu bytes\n", Mode->SizeOfInfo));
  DEBUG((DEBUG_INFO, "  FrameBufferBase: 0x%016lx\n", Mode->FrameBufferBase));
  DEBUG((DEBUG_INFO, "  FrameBufferSize: %lu bytes\n", Mode->FrameBufferSize));

  for (INT32 i = 0; i < Mode->MaxMode; i++) 
  {
    DEBUG((EFI_D_INFO, "Mode number: (%d)\n", i));
    PrintModeQueryInfo(Mode->Info);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
InitializeGraphicMode(
    IN OUT GAME_GRAPHICS_LIB_DATA *Data)
{
  EFI_STATUS Status;

  Status = gBS->LocateProtocol(
      &gEfiGraphicsOutputProtocolGuid,
      NULL,
      (VOID **)&Data->GraphicsOutput);
  if (EFI_ERROR(Status))
  {
    return Status;
  }

  Status = PrintGraphicsOutputProtocolMode(
      Data->GraphicsOutput->Mode);
  if (EFI_ERROR(Status))
  {
    return Status;
  }

  Data->Screen.HorizontalResolution = Data->GraphicsOutput->Mode->Info->HorizontalResolution;
  Data->Screen.VerticalResolution = Data->GraphicsOutput->Mode->Info->VerticalResolution;

  Data->SizeOfBackBuffer = 
      Data->Screen.HorizontalResolution * Data->Screen.VerticalResolution * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);

  // Allocating memory for the buffer
  Status = gBS->AllocatePool(
      EfiBootServicesData, 
      Data->SizeOfBackBuffer, 
      (VOID **)&Data->BackBuffer
      );
  if (EFI_ERROR(Status))
  {
    DEBUG((DEBUG_ERROR, "Failed to allocate BackBuffer: %r\n", Status));
    return Status;
  }

  Status = ClearScreen(Data);
  if (EFI_ERROR(Status))
  {
    return Status;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
FinishGraphicMode(
    IN OUT GAME_GRAPHICS_LIB_DATA *Data)
{
  EFI_STATUS Status;
  Status = gBS->FreePool(Data->BackBuffer);
  if (EFI_ERROR(Status))
  {
    DEBUG((DEBUG_ERROR, "Failed to free BackBuffer memory pool: %r\n", Status));
    return Status;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
DrawRectangle(
    IN GAME_GRAPHICS_LIB_DATA *Data,
    IN INT32 x,
    IN INT32 y,
    IN INT32 HorizontalSize,
    IN INT32 VerticalSize,
    IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Color)
{
  if (Data == NULL || Color == NULL)
  {
    return EFI_INVALID_PARAMETER;
  }

  if (x < 0 || y < 0 || x > Data->Screen.HorizontalResolution || y > Data->Screen.VerticalResolution)
  {
    DEBUG((DEBUG_ERROR, "DrawRectangle: Invalid coordinates. Coordinates out of screen. \n"));
    return EFI_INVALID_PARAMETER;
  }

  for (INT32 i = 0; i < VerticalSize; i++)
  {
    for (INT32 j = 0; j < HorizontalSize; j++)
    {
      if (x + j >= Data->Screen.HorizontalResolution || y + i >= Data->Screen.VerticalResolution)
      {
        continue;
      }
      Data->BackBuffer[(y + i) * Data->Screen.HorizontalResolution + (x + j)] = *Color;
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ClearScreen(
    IN GAME_GRAPHICS_LIB_DATA *Data) 
{
  EFI_STATUS Status;

  // Filling the buffer with zeros (black)
  SetMem(Data->BackBuffer, Data->SizeOfBackBuffer, 0);

  // Updating the video buffer immediately
  Status = UpdateVideoBuffer(Data);
  if (EFI_ERROR(Status))
  {
    return Status;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
UpdateVideoBuffer(
    IN GAME_GRAPHICS_LIB_DATA *Data) 
{
  EFI_STATUS Status;

  Status = Data->GraphicsOutput->Blt(
      Data->GraphicsOutput,
      Data->BackBuffer,
      EfiBltBufferToVideo,
      0, 0,
      0, 0,
      Data->Screen.HorizontalResolution,
      Data->Screen.VerticalResolution,
      0);
  if (EFI_ERROR(Status))
  {
    DEBUG((DEBUG_ERROR, "UpdateVideoBuffer: Failed to update video buffer: %r\n", Status));
    return Status;
  }

  return EFI_SUCCESS;
}