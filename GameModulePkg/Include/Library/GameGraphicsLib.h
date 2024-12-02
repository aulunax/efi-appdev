#ifndef _GAME_GRAPHICS_LIBRARY_H_
#define _GAME_GRAPHICS_LIBRARY_H_


// GAME_GRAPHICS_LIB_DATA
typedef struct
{
  UINT32 HorizontalResolution;
  UINT32 VerticalResolution;
} GAME_GRAPHICS_LIB_SCREEN_DATA;

typedef struct
{
  EFI_GRAPHICS_OUTPUT_PROTOCOL *GraphicsOutput;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BackBuffer;
  UINTN SizeOfBackBuffer;
  GAME_GRAPHICS_LIB_SCREEN_DATA Screen;
} GAME_GRAPHICS_LIB_DATA;



VOID
EFIAPI
MyLibraryFunction (
  VOID
  );

/// @brief Prints the information of the specific mode of the Graphics Output Protocol
/// @param ModeInfo The mode information of the Graphics Output Protocol
/// @return EFI_SUCCESS if the function executed successfully, otherwise an error code.
EFI_STATUS
EFIAPI
PrintModeQueryInfo(
    IN EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *ModeInfo);


/// @brief Prints the information of the current mode of the Graphics Output Protocol
/// @param Mode The mode of the Graphics Output Protocol
/// @return EFI_SUCCESS if the function executed successfully, otherwise an error code.
EFI_STATUS
EFIAPI
PrintGraphicsOutputProtocolMode(
   IN EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE *Mode);


/// @brief Sets up the library variables to be used in the library. Also clears the screen (paints it black)
/// @param Data The data structure that will be used to store the library variables
/// @return EFI_SUCCESS if the function executed successfully, otherwise an error code.
/// @note This function must be called before any other function in the library. FinishGraphicMode must be called after the library is no longer needed.
EFI_STATUS
EFIAPI
InitializeGraphicMode(
    IN OUT GAME_GRAPHICS_LIB_DATA *Data);

/// @brief Mainly frees the memory allocated by the library
/// @param Data The data structure that is used to store the library variables
/// @return EFI_SUCCESS if the function executed successfully, otherwise an error code.
/// @note This function must be called after the library is no longer needed.
EFI_STATUS
EFIAPI
FinishGraphicMode(
    IN OUT GAME_GRAPHICS_LIB_DATA *Data);


/// @brief Not implemented yet
/// @param Data The data structure that is used to store the library variables
/// @param x 
/// @param y 
/// @param HorizontalSize 
/// @param VerticalSize 
/// @return EFI_SUCCESS if the function executed successfully, otherwise an error code.
EFI_STATUS
EFIAPI
DrawSquare(
    IN GAME_GRAPHICS_LIB_DATA *Data,
    IN INT32 x,
    IN INT32 y,
    IN INT32 HorizontalSize,
    IN INT32 VerticalSize);


/// @brief Clears the screen by painting it black
/// @param Data The data structure that is used to store the library variables
/// @return EFI_SUCCESS if the function executed successfully, otherwise an error code.
EFI_STATUS
EFIAPI
ClearScreen(
    IN GAME_GRAPHICS_LIB_DATA *Data);


/// @brief Updates the video buffer with the back buffer in the library data structure
/// @param Data The data structure that is used to store the library variables. It contains the back buffer that will be copied to the video buffer
/// @return EFI_SUCCESS if the function executed successfully, otherwise an error code.
EFI_STATUS
EFIAPI
UpdateVideoBuffer(
    IN GAME_GRAPHICS_LIB_DATA *Data);

#endif // _GAME_GRAPHICS_LIBRARY_H_