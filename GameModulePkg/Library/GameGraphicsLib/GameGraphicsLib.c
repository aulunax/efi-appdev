#include <Base.h>
#include <Library/DebugLib.h>


VOID
EFIAPI
MyLibraryFunction (
  VOID
  )
{
  DEBUG((DEBUG_INFO, "MyLibraryFunction called\n"));
}