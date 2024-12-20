#ifndef _GAME_GRAPHICS_LIBRARY_H_
#define _GAME_GRAPHICS_LIBRARY_H_

/// @file
/// Game Graphics Library
/// UEFI Graphics Output Protocol based library that provides functions to draw on the screen.
///
/// @section Initialization
/// To use the library, the InitializeGraphicMode function must be called first, and the FinishGraphicMode function must be called after the library is no longer needed.
///
/// @section Drawing
/// All drawing functions change the back buffer, which is then copied to the video buffer with an update function.
/// Therefore to see the changes on the screen, an update function must be called, for example UpdateVideoBuffer.
/// It is possilble to update only a specific area of the screen, which can be done with the SmartUpdateVideoBuffer function.
///
/// @section Grid
/// The library provides a grid data structure that allows for easy drawing of a colored grid on the screen.
/// The grid is divided into cells, each cell can be colored with a specific color, described by the ColorsBitmap field.
/// To create a grid, use CreateCustomGrid, to draw the grid on the screen, use DrawGrid, to fill a cell in the grid, use FillCellInGrid, and to delete the grid, use DeleteGrid.
/// Important to note is the fact, that the grid has to be deleted after it is no longer needed, to free the memory allocated by the grid.
/// The grid can be cleared with the ClearGrid function, which paints all cells black.
/// The UpdateCellInGrid function is used to update the video buffer with the corresponding area of the back buffer in the data structure at grid coordinates of the specified cell in the grid structure.
///
///
/// @section Text
/// The library provides a function to draw a string of text on the screen. It uses a 8x8 font to draw each character.
/// The bitmap of the font is located in the Font8x8.h file in the same directory as this file.

/// @brief Data structure that stores the screen resolution
typedef struct
{
    UINT32 HorizontalResolution;
    UINT32 VerticalResolution;
} GAME_GRAPHICS_LIB_SCREEN_DATA;

/// @brief Data structure that stores the library variables
typedef struct
{
    EFI_GRAPHICS_OUTPUT_PROTOCOL *GraphicsOutput; // Graphics Output Protocol instance pointer
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BackBuffer;    // Back buffer that will be used to draw on the screen
    UINTN SizeOfBackBuffer;                       // Size of the back buffer in bytes
    GAME_GRAPHICS_LIB_SCREEN_DATA Screen;         // Screen data structure
} GAME_GRAPHICS_LIB_DATA;

/// @brief Grid data structure that allows for easy drawing of a colored grid on the screen
/// @details
/// The grid is divided into cells, each cell can be colored with a specific color,
/// described by the ColorsBitmap field.
///
/// Related functions: CreateCustomGrid, DrawGrid, FillCellInGrid, DeleteGrid, ClearGrid, UpdateCellInGrid
typedef struct
{
    UINT32 HorizontalSize;                       // Total horizontal size of the grid
    UINT32 VerticalSize;                         // Total vertical size of the grid
    UINT32 HorizontalCellsCount;                 // Number of horizontal cells in the grid
    UINT32 VerticalCellsCount;                   // Number of vertical cells in the grid
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *ColorsBitmap; // Bitmap that stores the color of each cell in the grid
    BOOLEAN *DirtyBitmap;                        // Bitmap that stores the information about which cells have been changed
} GAME_GRAPHICS_LIB_GRID;

/// @brief Grid cell pattern data structure
/// This is used to store the pattern of the cell in the grid
/// Currently unused
typedef struct
{
    UINT32 SizeX;
    UINT32 SizeY;
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Pattern;
} GAME_GRAPHICS_LIB_GRID_CELL_PATTERN;

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

/// @brief Sets up the library variables to be used in the library.
/// @param Data The data structure that will be used to store the library variables
/// @return EFI_SUCCESS if the function executed successfully, otherwise an error code.
/// @note This function must be called before any other function in the library.
/// @note FinishGraphicMode must be called after the library is no longer needed.
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

/// @brief Draws a color filled rectangle on the screen
/// @param Data The data structure that is used to store the library variables
/// @param x x coordinate of the top left corner of the rectangle
/// @param y y coordinate of the top left corner of the rectangle
/// @param HorizontalSize Horizontal size of the rectangle
/// @param VerticalSize Vertical size of the rectangle
/// @return EFI_SUCCESS if the function executed successfully, otherwise an error code.
/// @note Requires using a function that updates the video buffer to see the changes on the screen
EFI_STATUS
EFIAPI
DrawRectangle(
    IN GAME_GRAPHICS_LIB_DATA *Data,
    IN INT32 x,
    IN INT32 y,
    IN INT32 HorizontalSize,
    IN INT32 VerticalSize,
    IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Color);

/// @brief Clears the screen by painting it black
/// @param Data The data structure that is used to store the library variables
/// @return EFI_SUCCESS if the function executed successfully, otherwise an error code.
/// @note Requires using a function that updates the video buffer to see the changes on the screen
EFI_STATUS
EFIAPI
ClearScreen(
    IN GAME_GRAPHICS_LIB_DATA *Data);

/// @brief Updates the video buffer with the back buffer in the data structure
/// @param Data The data structure that is used to store the library variables. It contains the back buffer that will be copied to the video buffer
/// @return EFI_SUCCESS if the function executed successfully, otherwise an error code.
/// @note This copies the back buffer in data structure to the video buffer using Blt BufferToVideo
EFI_STATUS
EFIAPI
UpdateVideoBuffer(
    IN GAME_GRAPHICS_LIB_DATA *Data);

/// @brief Updates the specified rectangle area of the video buffer with the corresponding area of the back buffer in the data structure
/// @param Data The data structure that is used to store the library variables.
/// @param x X coordinate of the top left corner of the area that will be updated
/// @param y Y coordinate of the top left corner of the area that will be updated
/// @param HorizontalSize Horizontal size of the area that will be updated
/// @param VerticalSize Vertical size of the area that will be updated
/// @return EFI_SUCCESS if the function executed successfully, otherwise an error code.
/// @note This copies the specified area of the back buffer in data structure to the video buffer using Blt BufferToVideo
/// @note Using this function is highly preferable to using UpdateVideoBuffer, since it can drastically reduce the amount of bytes that need to be copied to video buffer
EFI_STATUS
EFIAPI
SmartUpdateVideoBuffer(
    IN GAME_GRAPHICS_LIB_DATA *Data,
    IN INT32 x,
    IN INT32 y,
    IN INT32 HorizontalSize,
    IN INT32 VerticalSize);

/// @brief Creates a grid with the specified number of cells and size of the cells
/// @param Grid The grid data structure that will be created
/// @param GridHorizontalSize Horizontal size of the grid
/// @param GridVerticalSize Vertical size of the grid
/// @param HorizontalCellsCount Number of horizontal cells in the grid
/// @param VerticalCellsCount Number of vertical cells in the grid
/// @param Bitmap Optional bitmap that will be used to color fill the cells in the grid. If NULL, a new bitmap is created with all cells colored black
/// @return EFI_SUCCESS if the function executed successfully, otherwise an error code.
/// @note The grid abstracts from using DrawRectangle, allowing the user to color fill cells in the grid with use of FillCellInGrid
EFI_STATUS
EFIAPI
CreateCustomGrid(
    OUT GAME_GRAPHICS_LIB_GRID *Grid,
    IN UINT32 GridHorizontalSize,
    IN UINT32 GridVerticalSize,
    IN UINT32 HorizontalCellsCount,
    IN UINT32 VerticalCellsCount,
    IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Bitmap OPTIONAL);

/// @brief Updates the color of a cell in the grid at the specified grid coordinates
/// @param Grid The grid data structure that will be used to fill the cell
/// @param x X coordinate of the cell in the grid
/// @param y Y coordinate of the cell in the grid
/// @param Color The color that will be used to fill the cell
/// @return EFI_SUCCESS if the function executed successfully, otherwise an error code.
/// @note The grid has to be drawn again on the screen after the cell is filled to see the changes
EFI_STATUS
EFIAPI
FillCellInGrid(
    IN GAME_GRAPHICS_LIB_GRID *Grid,
    IN UINT32 x,
    IN UINT32 y,
    IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Color);

/// @brief Clears the grid by painting all cells black
/// @param Grid The grid data structure that will be cleared
/// @return EFI_SUCCESS if the function executed successfully, otherwise an error code.
/// @note Requires using a function that updates the video buffer to see the changes on the screen
EFI_STATUS
EFIAPI
ClearGrid(
    IN GAME_GRAPHICS_LIB_GRID *Grid);

/// @brief Draws the grid on the screen at specified coordinates
/// @param Data The data structure that is used to store the library variables
/// @param Grid The grid data structure that will be drawn
/// @param x X coordinate of the top left corner of the grid
/// @param y Y coordinate of the top left corner of the grid
/// @return EFI_SUCCESS if the function executed successfully, otherwise an error code.
/// @note Requires using a function that updates the video buffer to see the changes on the screen
EFI_STATUS
EFIAPI
DrawGrid(
    IN GAME_GRAPHICS_LIB_DATA *Data,
    IN GAME_GRAPHICS_LIB_GRID *Grid,
    IN UINT32 x,
    IN UINT32 y);

/// @brief Updates the area of the video buffer with the corresponding area of the back buffer in the data structure at grid coordinates of the specified cell in the grid structure
/// @param Data The data structure that is used to store the library variables
/// @param Grid The grid data structure that will be used to update the video buffer
/// @param xOffset X coordinate (pixel position) of the top left corner of the grid
/// @param yOffset Y coordinate (pixel position) of the top left corner of the grid
/// @param x X coordinate (index) of the cell in the grid
/// @param y Y coordinate (index )of the cell in the grid
/// @return EFI_SUCCESS if the function executed successfully, otherwise an error code.
/// @note This function is used to update the video buffer with the corresponding area of the back buffer in the data structure at grid coordinates of the specified cell in the grid structure
EFI_STATUS
EFIAPI
UpdateCellInGrid(
    IN GAME_GRAPHICS_LIB_DATA *Data,
    IN GAME_GRAPHICS_LIB_GRID *Grid,
    IN INT32 xOffset,
    IN INT32 yOffset,
    IN INT32 x,
    IN INT32 y);

/// @brief Deletes the grid and frees the memory allocated for the grid
/// @param Grid The grid data structure that will be deleted
/// @return EFI_SUCCESS if the function executed successfully, otherwise an error code.
EFI_STATUS
EFIAPI
DeleteGrid(
    IN GAME_GRAPHICS_LIB_GRID *Grid);

/// @brief Draws an 8x8 character on the screen at specified coordinates
/// @param Data The data structure that is used to store the library variables
/// @param x X coordinate of the character's top left corner
/// @param y Y coordinate of the character's top left corner
/// @param Character The character that will be drawn on the screen
/// @param ForegroundColor The color of the character
/// @param BackgroundColor The color of the background
/// @param SizeMultipiler The size multipiler of the character. 1 is the original size of the font
/// @return EFI_SUCCESS if the function executed successfully, otherwise an error code.
/// @note Requires using a function that updates the video buffer to see the changes on the screen
EFI_STATUS
EFIAPI
DrawCharacter(
    IN GAME_GRAPHICS_LIB_DATA *Data,
    IN UINT32 x,
    IN UINT32 y,
    IN CHAR8 Character,
    IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *ForegroundColor,
    IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BackgroundColor,
    IN UINT32 SizeMultipiler);

/// @brief Draws text in 8x8 font on the screen at specific coordinates
/// @param Data The data structure that is used to store the library variables
/// @param x X coordinate of the top left corner of the text
/// @param y Y coordinate of the top left corner of the text
/// @param Text The text that will be drawn on the screen
/// @param ForegroundColor The color of the text
/// @param BackgroundColor The color of the background
/// @param SizeMultipiler The size multipiler of the text. 1 is the original size of the font
/// @return EFI_SUCCESS if the function executed successfully, otherwise an error code.
/// @note Requires using a function that updates the video buffer to see the changes on the screen
EFI_STATUS
EFIAPI
DrawText(
    IN GAME_GRAPHICS_LIB_DATA *Data,
    IN UINT32 x,
    IN UINT32 y,
    IN CHAR8 *Text,
    IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *ForegroundColor,
    IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BackgroundColor,
    IN UINT32 SizeMultipiler);

#endif // _GAME_GRAPHICS_LIBRARY_H_
