#include <Library/DebugLib.h>
#include <Uefi.h>
#include <Protocol/GraphicsOutput.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/GameGraphicsLib.h>
#include <Library/Font8x8.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>

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
PrintGraphicsOutputProtocolMode(
    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE *Mode)
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
      (VOID **)&Data->BackBuffer);
  if (EFI_ERROR(Status))
  {
    DEBUG((DEBUG_ERROR, "Failed to allocate BackBuffer: %r\n", Status));
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
  if ((Data == NULL) || (Color == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  if ((x < 0) || (y < 0) || (x > Data->Screen.HorizontalResolution) || (y > Data->Screen.VerticalResolution))
  {
    // DEBUG((DEBUG_ERROR, "DrawRectangle: Invalid coordinates. Coordinates out of screen. \n"));
    return EFI_INVALID_PARAMETER;
  }

  for (INT32 i = 0; i < VerticalSize; i++)
  {
    for (INT32 j = 0; j < HorizontalSize; j++)
    {
      if ((x + j >= Data->Screen.HorizontalResolution) || (y + i >= Data->Screen.VerticalResolution))
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
  // Filling the buffer with zeros (black)
  SetMem(Data->BackBuffer, Data->SizeOfBackBuffer, 0);

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
      0,
      0,
      0,
      0,
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

EFI_STATUS
EFIAPI
CreateCustomGrid(
    IN OUT GAME_GRAPHICS_LIB_GRID *Grid,
    IN UINT32 GridHorizontalSize,
    IN UINT32 GridVerticalSize,
    IN UINT32 HorizontalCellsCount,
    IN UINT32 VerticalCellsCount,
    IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Bitmap OPTIONAL)
{
  if (Grid == NULL)
  {
    return EFI_INVALID_PARAMETER;
  }

  Grid->HorizontalSize = GridHorizontalSize;
  Grid->VerticalSize = GridVerticalSize;
  Grid->HorizontalCellsCount = HorizontalCellsCount;
  Grid->VerticalCellsCount = VerticalCellsCount;

  if (Bitmap != NULL)
  {
    Grid->ColorsBitmap = Bitmap;
  }
  else
  {
    Grid->ColorsBitmap = AllocatePool(HorizontalCellsCount * VerticalCellsCount * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
    if (Grid->ColorsBitmap == NULL)
    {
      DEBUG((DEBUG_ERROR, "Failed to allocate ColorsBitmap memory pool.\n"));
      return EFI_OUT_OF_RESOURCES;
    }

    // Filling the bitmap with zeros (black)
    SetMem(Grid->ColorsBitmap, HorizontalCellsCount * VerticalCellsCount * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL), 0);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
DrawGrid(
    IN GAME_GRAPHICS_LIB_DATA *Data,
    IN GAME_GRAPHICS_LIB_GRID *Grid,
    IN UINT32 x,
    IN UINT32 y)
{
  UINT32 VerticalOffset = 0;
  UINT32 HorizontalOffset = 0;
  UINT32 CurrentCellHorizontalSize = 0;
  UINT32 CurrentCellVerticalSize = 0;
  UINT32 HorizontalRemainder = 0;
  UINT32 VerticalRemainder = 0;

  if ((Data == NULL) || (Grid == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  for (INT32 i = 0; i < Grid->VerticalCellsCount; i++)
  {
    CurrentCellVerticalSize = Grid->VerticalSize / Grid->VerticalCellsCount;
    VerticalRemainder += Grid->VerticalSize % Grid->VerticalCellsCount;
    if (VerticalRemainder >= Grid->VerticalCellsCount)
    {
      VerticalRemainder -= Grid->VerticalCellsCount;
      CurrentCellVerticalSize++;
    }
    for (INT32 j = 0; j < Grid->HorizontalCellsCount; j++)
    {
      CurrentCellHorizontalSize = Grid->HorizontalSize / Grid->HorizontalCellsCount;
      HorizontalRemainder += Grid->HorizontalSize % Grid->HorizontalCellsCount;
      if (HorizontalRemainder >= Grid->HorizontalCellsCount)
      {
        HorizontalRemainder -= Grid->HorizontalCellsCount;
        CurrentCellHorizontalSize++;
      }

      // Intentionally ignoring return status of DrawRectangle
      // This allows for the grid to be fully drawn even if some cells are off screen
      DrawRectangle(Data,
                    x + HorizontalOffset,
                    y + VerticalOffset,
                    CurrentCellHorizontalSize,
                    CurrentCellVerticalSize,
                    &Grid->ColorsBitmap[i * Grid->HorizontalCellsCount + j]);

      HorizontalOffset += CurrentCellHorizontalSize;
    }
    VerticalOffset += CurrentCellVerticalSize;
    HorizontalOffset = 0;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
FillCellInGrid(
    IN GAME_GRAPHICS_LIB_GRID *Grid,
    IN UINT32 x,
    IN UINT32 y,
    IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Color)
{
  Grid->ColorsBitmap[y * Grid->HorizontalCellsCount + x] = *Color;
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
DeleteGrid(
    IN GAME_GRAPHICS_LIB_GRID *Grid)
{
  if (Grid == NULL)
  {
    return EFI_INVALID_PARAMETER;
  }

  if (Grid->ColorsBitmap != NULL)
  {
    FreePool(Grid->ColorsBitmap);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ClearGrid(
    IN GAME_GRAPHICS_LIB_GRID *Grid)
{
  if (Grid == NULL)
  {
    return EFI_INVALID_PARAMETER;
  }

  SetMem(Grid->ColorsBitmap, Grid->HorizontalCellsCount * Grid->VerticalCellsCount * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL), 0);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
DrawCharacter(
    IN GAME_GRAPHICS_LIB_DATA *Data,
    IN UINT32 x,
    IN UINT32 y,
    IN CHAR8 Character,
    IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *ForegroundColor,
    IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BackgroundColor,
    IN UINT32 SizeMultipiler)
{
  UINTN FontHeight = 8;
  UINTN FontWidth = 8;
  UINT32 CurrentX = x;
  UINT32 CurrentY = y;
  UINTN CurrentCharacter = (UINTN)Character;

  for (UINTN row = 0; row < FontHeight; row++)
  {
    for (UINTN col = 0; col < FontWidth; col++)
    {
      if (CurrentY * Data->Screen.HorizontalResolution + CurrentX >= Data->SizeOfBackBuffer)
      {
        DEBUG((DEBUG_ERROR, "DrawCharacter: Coordinates out of screen. \n"));
        return EFI_INVALID_PARAMETER;
      }

      if ((font8x8_basic[CurrentCharacter][row] >> col) & 0x1)
      {
        for (INTN i = 0; i < SizeMultipiler; i++)
        {
          for (INTN j = 0; j < SizeMultipiler; j++)
          {
            Data->BackBuffer[(CurrentY + i) * Data->Screen.HorizontalResolution + (CurrentX + j)] = *ForegroundColor;
          }
        }
      }
      else
      {
        for (INTN i = 0; i < SizeMultipiler; i++)
        {
          for (INTN j = 0; j < SizeMultipiler; j++)
          {
            Data->BackBuffer[(CurrentY + i) * Data->Screen.HorizontalResolution + (CurrentX + j)] = *BackgroundColor;
          }
        }
      }
      CurrentX += SizeMultipiler;
    }
    CurrentY += SizeMultipiler;
    CurrentX = x;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
DrawText(
    IN GAME_GRAPHICS_LIB_DATA *Data,
    IN UINT32 x,
    IN UINT32 y,
    IN CHAR8 *Text,
    IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *ForegroundColor,
    IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BackgroundColor,
    IN UINT32 SizeMultipiler)
{
  EFI_STATUS Status;
  UINTN PosX = x;
  UINTN PosY = y;

  while (*Text != '\0')
  {
    Status = DrawCharacter(Data,
                           PosX, PosY,
                           *Text,
                           ForegroundColor,
                           BackgroundColor,
                           SizeMultipiler);
    if (EFI_ERROR(Status))
    {
      DEBUG((DEBUG_ERROR, "DrawText: DrawCharacter failed: %r\n", Status));
      return Status;
    }
    PosX += 8 * SizeMultipiler;
    Text++;
  }

  return EFI_SUCCESS;
}