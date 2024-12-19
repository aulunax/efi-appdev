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
EFI_STATUS
EFIAPI SnakeMain(
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable)
{
  // Uefi variables
  EFI_STATUS status;
  EFI_EVENT FrameTimerEvent;
  EFI_EVENT FpsDisplayEvent;
  EFI_STATUS keyStatus;
  EFI_INPUT_KEY key;

  // Colors definitions
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL Red = {0, 0, 255, 0};
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL Green = {0, 255, 0, 0};
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL Black = {0, 0, 0, 0};
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL White = {255, 255, 255, 0};

  // Game graphics library structures
  GAME_GRAPHICS_LIB_DATA GraphicsLibData;
  GAME_GRAPHICS_LIB_GRID MainGrid;
  UINT32 screenWidth = GraphicsLibData.Screen.HorizontalResolution;
  UINT32 screenHeight = GraphicsLibData.Screen.VerticalResolution;

  // Game variables
  Point snakeParts[MAX_SNAKE_SIZE];
  UINT32 snakeSize = 1;
  Direction direction = NONE;
  Direction nextDirection = NONE;
  BOOLEAN firstMove = TRUE;
  Point food;
  UINT32 score = 0;

  // Frame counting related variables
  UINT32 subFrames = 0;
  UINT32 frames = 0;
  FPS_CONTEXT FpsContext = {&frames, &GraphicsLibData};

  // initialize global variables
  initGlobalVariables(ImageHandle, SystemTable);

  // Initialize the graphics library
  status = InitializeGraphicMode(&GraphicsLibData);
  if (status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_ERROR, "Failed to enable graphic mode.\n"));
    return status;
  }

  // Create a timer event for the game loop
  status = gBS->CreateEvent(EVT_TIMER, TPL_NOTIFY, NULL, NULL, &FrameTimerEvent);
  if (status != EFI_SUCCESS)
  {
    Print(L"Failed to create timer event: %r\n", status);
    return status;
  }

  // Set the timer event to fire every 1/FPS seconds
  status = gBS->SetTimer(FrameTimerEvent, TimerPeriodic, 10000000 / PcdGet32(PcdTestFramerate));
  if (status != EFI_SUCCESS)
  {
    Print(L"Failed to set timer: %r\n", status);
    gBS->CloseEvent(FrameTimerEvent);
    return status;
  }

  // Get the screen resolution variables
  screenWidth = GraphicsLibData.Screen.HorizontalResolution;
  screenHeight = GraphicsLibData.Screen.VerticalResolution;

  // Create the grid used for the game board
  status = CreateCustomGrid(&MainGrid, screenWidth, screenHeight - 32, HORIZONTAL_CELLS, VERTICAL_CELLS, NULL); // subtract 32 for score display
  if (status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_ERROR, "Failed to create grid.\n"));
    return status;
  }

  // Start screen handling
  printStartMessage(&GraphicsLibData, White, Black, screenWidth, screenHeight);
  while (1)
  {
    keyStatus = cin->ReadKeyStroke(cin, &key);
    if (keyStatus == EFI_SUCCESS)
    {
      break;
    }
  }

  // Initialize the game parameters
  initSnake(snakeParts, snakeSize);
  generateRandomPoint(&food, snakeParts, snakeSize, subFrames);

  // Draw the initial screen state
  ClearScreen(&GraphicsLibData);
  drawFood(&MainGrid, food, &Green);
  drawScore(&GraphicsLibData, White, Black, score, screenWidth, screenHeight);
  DrawRectangle(&GraphicsLibData, 0, 31, screenWidth, 1, &White);
  displayFpsCounter(&GraphicsLibData, &frames);
  DrawGrid(&GraphicsLibData, &MainGrid, 0, 32);
  UpdateVideoBuffer(&GraphicsLibData);

  // Create a timer event for the FPS display
  status = gBS->CreateEvent(EVT_TIMER | EVT_NOTIFY_SIGNAL, TPL_CALLBACK, FpsDisplayCallback, &FpsContext, &FpsDisplayEvent);
  if (status != EFI_SUCCESS)
  {
    Print(L"Failed to create timer event: %r\n", status);
    return status;
  }

  // Set the timer event to fire every FPS_DISPLAY_RATE seconds
  status = gBS->SetTimer(FpsDisplayEvent, TimerPeriodic, FPS_DISPLAY_RATE);
  if (status != EFI_SUCCESS)
  {
    Print(L"Failed to set timer: %r\n", status);
    gBS->CloseEvent(FpsDisplayEvent);
    return status;
  }

  // Main game loop
  while (1)
  {
    // Input handling
    keyStatus = cin->ReadKeyStroke(cin, &key);
    if (key.ScanCode == SCAN_ESC)
    {
      break;
    }
    if (keyStatus == EFI_SUCCESS)
    {
      firstMove = FALSE;
      changeDirection(key, &nextDirection);
    }
    subFrames++;
    status = gBS->CheckEvent(FrameTimerEvent);
    if ((RETURN_STATUS)status == EFI_NOT_READY)
    {
      // If event hasn't been signaled, frame is not done yet. Jumping to start of the loop.
      continue;
    }
    frames++;

    //
    // Game logic
    //

    setNewDirection(&direction, nextDirection);

    moveSnake(snakeParts, snakeSize, direction, screenWidth, screenHeight);
    if (checkCollision(snakeParts, snakeSize) && !firstMove)
    {
      break;
    }

    drawSnake(&MainGrid, snakeParts, snakeSize, &Red);
    snakeAteFood = FALSE;

    if (checkIfSnakeAteFood(snakeParts, snakeSize, food))
    {
      snakeSize++;
      score += 10;
      snakeAteFood = TRUE;
      generateRandomPoint(&food, snakeParts, snakeSize, subFrames);

      // update score rectangle
      drawScore(&GraphicsLibData, White, Black, score, screenWidth, screenHeight);
      SmartUpdateVideoBuffer(&GraphicsLibData, 112, 8, 96, 16);
    }

    drawFood(&MainGrid, food, &Green);
    DrawGrid(&GraphicsLibData, &MainGrid, 0, 32);

    if (snakeAteFood)
    {
      UpdateCellInGrid(&GraphicsLibData, &MainGrid, 0, 32, food.x, food.y);
    }

    UpdateCellInGrid(&GraphicsLibData, &MainGrid, 0, 32, snakeParts[0].x, snakeParts[0].y);
    UpdateCellInGrid(&GraphicsLibData, &MainGrid, 0, 32, SnakeBeforeBack.x, SnakeBeforeBack.y);
  }

  // Cleanup
  gBS->CloseEvent(FrameTimerEvent);
  gBS->CloseEvent(FpsDisplayEvent);
  DeleteGrid(&MainGrid);

  // Game over screen handling
  printGameOverMessage(&GraphicsLibData, White, Black, Red, screenWidth, screenHeight, score);
  while (1)
  {
    keyStatus = cin->ReadKeyStroke(cin, &key);
    if (keyStatus == EFI_SUCCESS)
    {
      break;
    }
  }

  ClearScreen(&GraphicsLibData);
  UpdateVideoBuffer(&GraphicsLibData);
  FinishGraphicMode(&GraphicsLibData);

  return EFI_SUCCESS;
}