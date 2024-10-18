#!/bin/sh

WORKING_DIRECTORY=$(pwd)
CONF_TARGET_FILE="target.txt"

MDEMODULE_ACTIVE_PLATFORM="MdeModulePkg\/MdeModulePkg.dsc"
OVMF_ACTIVE_PLATFORM="OvmfPkg\/OvmfPkgX64.dsc"
GCC5_TOOL_CHAIN_TAG="GCC5"
X64_TARGET_ARCH="X64"

DISK_SIZEMB=32

# Ask for sudo access at the beginning
sudo -v

# Download and build edk2
git clone https://github.com/tianocore/edk2
cd edk2
git submodule update --init
make -C BaseTools
echo "Current Directory: $(pwd)"
. ./edksetup.sh

# Editing the config for MdeModule build
cd Conf


sed -i "s/^\(ACTIVE_PLATFORM\s*=\s*\).*/\1$MDEMODULE_ACTIVE_PLATFORM/" $CONF_TARGET_FILE

sed -i "s/^\(TARGET_ARCH\s*=\s*\).*/\1$X64_TARGET_ARCH/" $CONF_TARGET_FILE

sed -i "s/^\(TOOL_CHAIN_TAG\s*=\s*\).*/\1$GCC5_TOOL_CHAIN_TAG/" $CONF_TARGET_FILE

# Build the MdeModule
build

# Editing the config for Ovmf build
sed -i "s/^\(ACTIVE_PLATFORM\s*=\s*\).*/\1$OVMF_ACTIVE_PLATFORM/" $CONF_TARGET_FILE

# Build Ovmf module
build

# Preparing qemu env
cd ../..
mkdir efi-qemu
cd efi-qemu

# Create disk image for the app
dd if=/dev/zero of=app.disk bs=1 count=1 seek=$(( ($DISK_SIZEMB * 1024 * 1024) - 1))
sudo mkfs.vfat app.disk

# Add test app to the disk
mkdir mnt_app
sudo mount app.disk mnt_app
sudo cp "../edk2/Build/MdeModule/DEBUG_$GCC5_TOOL_CHAIN_TAG/$X64_TARGET_ARCH/HelloWorld.efi" mnt_app
sudo umount mnt_app

# Copy ovmf image to qemu directory
cp "../edk2/Build/Ovmf$X64_TARGET_ARCH/DEBUG_$GCC5_TOOL_CHAIN_TAG/FV/OVMF.fd" ovmf.flash

echo "Efi setup for qemu finished. Run omvf.sh to start the vm."

