#!/bin/sh

# Exit script if anything fails
set -e

WORKING_DIRECTORY=$(pwd)
CONF_TARGET_FILE="target.txt"

MDEMODULE_ACTIVE_PLATFORM="MdeModulePkg/MdeModulePkg.dsc"
OVMF_ACTIVE_PLATFORM="OvmfPkg/OvmfPkgX64.dsc"
GCC5_TOOL_CHAIN_TAG="GCC5"
X64_TARGET_ARCH="X64"

DISK_SIZEMB=32


BUILD_APP_ONLY=false

# Parse options
while [ $# -gt 0 ]; do
  case "$1" in
    --build-app-only)
      BUILD_APP_ONLY=true
      ;;
    *)
      echo "Usage: $0 [--build-app-only]"
      exit 1
      ;;
  esac
  shift
done


# Check if required commands are installed
command -v git >/dev/null 2>&1 || { echo "Error: git is not installed."; exit 1; }
command -v make >/dev/null 2>&1 || { echo "Error: make is not installed."; exit 1; }
command -v qemu-system-x86_64 >/dev/null 2>&1 || { echo "Error: qemu-system-x86_64 is not installed."; exit 1; }
command -v dd >/dev/null 2>&1 || { echo "Error: dd is not installed."; exit 1; }
command -v mkfs.vfat >/dev/null 2>&1 || { echo "Error: mkfs.vfat is not installed."; exit 1; }
command -v nasm >/dev/null 2>&1 || { echo "Error: nasm is not installed."; exit 1; }
command -v uuidgen >/dev/null 2>&1 || { echo "Error: uuid-dev is not installed."; exit 1; }
command -v iasl >/dev/null 2>&1 || { echo "Error: iasl is not installed."; exit 1; }
command -v python >/dev/null 2>&1 || { echo "Error: python is not installed."; exit 1; }

# Ask for sudo access at the beginning
sudo -v


if [ "$BUILD_APP_ONLY" = false ]; then

    # Download and build edk2
    if [ ! -d "edk2" ]; then
        git clone https://github.com/tianocore/edk2 || { echo "Error: Failed to clone edk2."; exit 1; }
    else
        echo "edk2 repository already cloned."
    fi

    cd edk2

    git submodule update --init || { echo "Error: Failed to update submodules."; exit 1; }

    make -C BaseTools || { echo "Error: Failed to build BaseTools."; exit 1; }
    
    cd ..
else
  echo "Skipping edk2 setup and tool building as per --skip-build flag."
fi

cd edk2

. ./edksetup.sh || { echo "Error: Failed to set up edk environment."; exit 1; }

cd Conf

# Build MdeModule package
build -a $X64_TARGET_ARCH -t $GCC5_TOOL_CHAIN_TAG -p $MDEMODULE_ACTIVE_PLATFORM || { echo "Error: Failed to build MdeModule."; exit 1; }
            
# Build Ovmf package
build -a $X64_TARGET_ARCH -t $GCC5_TOOL_CHAIN_TAG -p $OVMF_ACTIVE_PLATFORM || { echo "Error: Failed to build OVMF."; exit 1; }


cd ../..

if [ "$BUILD_APP_ONLY" = true ]; then
  echo "Removing efi-qemu directory..."
  rm -r efi-qemu
fi

# Creating qemu directory
mkdir efi-qemu
cd efi-qemu

# Create disk image for the app
echo "Creating disk image for the app..."
dd if=/dev/zero of=app.disk bs=1 count=1 seek=$(( ($DISK_SIZEMB * 1024 * 1024) - 1)) || { echo "Error: Failed to create disk image."; exit 1; }
sudo mkfs.vfat app.disk || { echo "Error: Failed to format disk image as FAT."; exit 1; }

# Add test app to the disk
mkdir -p mnt_app
sudo mount app.disk mnt_app || { echo "Error: Failed to mount app disk."; exit 1; }
sudo cp "../edk2/Build/MdeModule/DEBUG_$GCC5_TOOL_CHAIN_TAG/$X64_TARGET_ARCH/HelloWorld.efi" mnt_app || { echo "Error: Failed to copy HelloWorld.efi to disk."; exit 1; }
sudo umount mnt_app || { echo "Error: Failed to unmount app disk."; exit 1; }

# Copy ovmf image to qemu directory
echo "Copying OVMF firmware image to QEMU directory..."
cp "../edk2/Build/Ovmf$X64_TARGET_ARCH/DEBUG_$GCC5_TOOL_CHAIN_TAG/FV/OVMF.fd" ovmf.flash || { echo "Error: Failed to copy OVMF firmware."; exit 1; }

echo "EFI setup for QEMU finished. Run omvf.sh to start the VM."

