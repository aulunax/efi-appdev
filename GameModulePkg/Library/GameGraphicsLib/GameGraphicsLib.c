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

  // Debug information about the current mode
  Status = PrintGraphicsOutputProtocolMode(
      Data->GraphicsOutput->Mode);
  if (EFI_ERROR(Status))
  {
    return Status;
  }

  Data->Screen.HorizontalResolution = Data->GraphicsOutput->Mode->Info->HorizontalResolution;
  Data->Screen.VerticalResolution = Data->GraphicsOutput->Mode->Info->VerticalResolution;

  Data->SizeOfBackBuffer =
      Data->Screen.HorizontalResolution *
      Data->Screen.VerticalResolution *
      sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);

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

  for (INT32 i = 0; i < VerticalSize; i++)
  {
    for (INT32 j = 0; j < HorizontalSize; j++)
    {
      // Ignoring off screen pixels
      if ((x + j >= Data->Screen.HorizontalResolution) ||
          (y + i >= Data->Screen.VerticalResolution) ||
          (x + j < 0) ||
          (y + i < 0))
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
SmartUpdateVideoBuffer(
    IN GAME_GRAPHICS_LIB_DATA *Data,
    IN INT32 x,
    IN INT32 y,
    IN INT32 HorizontalSize,
    IN INT32 VerticalSize)
{
  EFI_STATUS Status;
  INT32 RealX = x < 0 ? 0 : x;
  INT32 RealY = y < 0 ? 0 : y;
  INT32 RealWidth = x + HorizontalSize < Data->Screen.HorizontalResolution ? HorizontalSize : Data->Screen.HorizontalResolution - x;
  INT32 RealHeight = y + VerticalSize < Data->Screen.VerticalResolution ? VerticalSize : Data->Screen.VerticalResolution - y;

  Status = Data->GraphicsOutput->Blt(
      Data->GraphicsOutput,
      Data->BackBuffer,
      EfiBltBufferToVideo,
      RealX,
      RealY,
      RealX,
      RealY,
      RealWidth,
      RealHeight,
      Data->Screen.HorizontalResolution * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
  if (EFI_ERROR(Status))
  {
    DEBUG((DEBUG_ERROR, "RealX: %d, RealY: %d, RealWidth: %d, RealHeight: %d\n", RealX, RealY, RealWidth, RealHeight));
    DEBUG((DEBUG_ERROR, "SmartUpdateVideoBuffer: Failed to update video buffer: %r\n", Status));
    return Status;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
UpdateCellInGrid(
    IN GAME_GRAPHICS_LIB_DATA *Data,
    IN GAME_GRAPHICS_LIB_GRID *Grid,
    IN INT32 xOffset,
    IN INT32 yOffset,
    IN INT32 x,
    IN INT32 y)
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

  // Calculating the size of each cell using division with remainder,
  // to not create unintended 1 pixel gaps
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
      // Ignoring cells in a row until we are in a correct row
      if (i != y)
      {
        continue;
      }

      CurrentCellHorizontalSize = Grid->HorizontalSize / Grid->HorizontalCellsCount;
      HorizontalRemainder += Grid->HorizontalSize % Grid->HorizontalCellsCount;
      if (HorizontalRemainder >= Grid->HorizontalCellsCount)
      {
        HorizontalRemainder -= Grid->HorizontalCellsCount;
        CurrentCellHorizontalSize++;
      }

      if (i == y && j == x)
      {
        SmartUpdateVideoBuffer(Data,
                               xOffset + HorizontalOffset - 1,
                               yOffset + VerticalOffset - 1,
                               CurrentCellHorizontalSize + 2,
                               CurrentCellVerticalSize + 2);
        return EFI_SUCCESS;
      }

      HorizontalOffset += CurrentCellHorizontalSize;
    }
    VerticalOffset += CurrentCellVerticalSize;
    HorizontalOffset = 0;
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

  // If a bitmap is provided, use it
  // else create a new bitmap with all cells colored black
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

  Grid->DirtyBitmap = AllocatePool(HorizontalCellsCount * VerticalCellsCount * sizeof(BOOLEAN));
  SetMem(Grid->DirtyBitmap, HorizontalCellsCount * VerticalCellsCount * sizeof(BOOLEAN), TRUE);

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

  // Calculating the size of each cell using division with remainder,
  // to not create unintended 1 pixel gaps
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

      if (Grid->DirtyBitmap[i * Grid->HorizontalCellsCount + j])
      {
        // Intentionally ignoring return status of DrawRectangle
        // This allows for the grid to be fully drawn even if some cells are off screen
        DrawRectangle(Data,
                      x + HorizontalOffset,
                      y + VerticalOffset,
                      CurrentCellHorizontalSize,
                      CurrentCellVerticalSize,
                      &Grid->ColorsBitmap[i * Grid->HorizontalCellsCount + j]);

        Grid->DirtyBitmap[i * Grid->HorizontalCellsCount + j] = FALSE;
      }

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
  if ((Grid == NULL) || (Color == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  Grid->ColorsBitmap[y * Grid->HorizontalCellsCount + x] = *Color;
  Grid->DirtyBitmap[y * Grid->HorizontalCellsCount + x] = TRUE;
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

  if (Grid->DirtyBitmap != NULL)
  {
    FreePool(Grid->DirtyBitmap);
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
  SetMem(Grid->DirtyBitmap, Grid->HorizontalCellsCount * Grid->VerticalCellsCount * sizeof(BOOLEAN), TRUE);

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
  UINTN FontHeight = FONT_VERTICAL_SIZE;
  UINTN FontWidth = FONT_HORIZONTAL_SIZE;
  UINT32 CurrentX = x;
  UINT32 CurrentY = y;
  UINTN CurrentCharacter = (UINTN)Character;

  if (CurrentCharacter >= FONT_CHARACTER_COUNT)
  {
    DEBUG((DEBUG_ERROR, "DrawCharacter: Invalid character. \n"));
    return EFI_INVALID_PARAMETER;
  }

  if (CurrentX < 0 || CurrentY < 0 ||
      CurrentX >= Data->Screen.HorizontalResolution ||
      CurrentY >= Data->Screen.VerticalResolution)
  {
    DEBUG((DEBUG_ERROR, "DrawCharacter: Coordinates out of screen bounds.\n"));
    return EFI_INVALID_PARAMETER;
  }

  for (UINTN BitmapRow = 0; BitmapRow < FontHeight; BitmapRow++)
  {
    for (UINTN BitmapColumn = 0; BitmapColumn < FontWidth; BitmapColumn++)
    {
      // If the pixel is out of screen bounds, skip it
      if (CurrentX + SizeMultipiler >= Data->Screen.HorizontalResolution ||
          CurrentY + SizeMultipiler >= Data->Screen.VerticalResolution)
      {
        continue;
      }

      // Using 8x8 font bitmap, we extract the bit of the current pixel
      // of the current character
      if ((gFont8x8_basic[CurrentCharacter][BitmapRow] >> BitmapColumn) & 0x1)
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
    PosX += FONT_HORIZONTAL_SIZE * SizeMultipiler;
    Text++;
  }

  return EFI_SUCCESS;
}