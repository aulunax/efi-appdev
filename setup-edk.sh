#!/bin/sh

# Exit script if anything fails
set -e

# Ask for sudo access at the beginning
sudo -v || { echo "Error: sudo is not installed."; exit 1; }

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

# Download and build edk2
if [ ! -d "edk2" ]; then
    git clone https://github.com/tianocore/edk2 || { echo "Error: Failed to clone edk2."; exit 1; }
else
    echo "edk2 repository already cloned."
fi

cd edk2

# fatal: repository 'https://github.com/Zeex/subhook.git/' not found
# replacing original repo with a fork
sed -i 's|https://github.com/Zeex/subhook.git|https://github.com/zhagnyongfdsfsdfsdfsdf/subhook.git|' .gitmodules
git submodule sync
git submodule update --init || { echo "Error: Failed to update submodules."; exit 1; }
