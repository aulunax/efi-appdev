## @file
# EFI Package for a game application
#
#
##

[Defines]
  PLATFORM_NAME                  = GameModule
  PLATFORM_GUID                  = 5e1b4ff8-89f7-4ab0-ac95-6fd340f54ba4
  PLATFORM_VERSION               = 0.01
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/GameModule
  SUPPORTED_ARCHITECTURES        = IA32|X64|EBC|ARM|AARCH64|RISCV64|LOONGARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE|NOOPT
  SKUID_IDENTIFIER               = DEFAULT