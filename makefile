SHELL=/bin/sh
.DEFAULT_GOAL := help

export WORKSPACE := $(PWD)
export EDK_TOOLS_PATH := $(WORKSPACE)/edk2/BaseTools
export PACKAGES_PATH := $(WORKSPACE)/edk2:$(WORKSPACE)/GameModulePkg


CONF_TARGET_FILE := target.txt
GAMEMODULE_ACTIVE_PLATFORM := GameModulePkg/GameModulePkg.dsc
MDEMODULE_ACTIVE_PLATFORM := MdeModulePkg/MdeModulePkg.dsc
OVMF_ACTIVE_PLATFORM := OvmfPkg/OvmfPkgX64.dsc
TOOL_CHAIN_TAG := GCC5
TARGET_ARCH := X64
BUILD_TARGET := RELEASE

DISK_SIZEMB := 32

APP_NAME ?= Test


.PHONY: _check-dependencies help all _create_conf_dir _create_qemu_dir _create_disk_image _add-app _copy_ovmf build-basetools build-app build-ovmf release

_check-dependencies:
	@echo "Checking system dependencies..."
	@command -v git >/dev/null 2>&1 || { echo "Error: git is not installed."; exit 1; }
	@command -v make >/dev/null 2>&1 || { echo "Error: make is not installed."; exit 1; }
	@command -v qemu-system-x86_64 >/dev/null 2>&1 || { echo "Error: qemu-system-x86_64 is not installed."; exit 1; }
	@command -v dd >/dev/null 2>&1 || { echo "Error: dd is not installed."; exit 1; }
	@command -v mkfs.vfat >/dev/null 2>&1 || { echo "Error: mkfs.vfat is not installed."; exit 1; }
	@command -v nasm >/dev/null 2>&1 || { echo "Error: nasm is not installed."; exit 1; }
	@command -v uuidgen >/dev/null 2>&1 || { echo "Error: uuid-dev is not installed."; exit 1; }
	@command -v iasl >/dev/null 2>&1 || { echo "Error: iasl is not installed."; exit 1; }
	@command -v python >/dev/null 2>&1 || { echo "Error: python is not installed."; exit 1; }
	@sudo -v || { echo "Error: Sudo access is required."; exit 1; }
	@echo "All dependencies satisfied. Continuing.."

help:
	@echo "Makefile Help"
	@echo "================="
	@echo "Use 'EFI_APP=' variable to choose which app to copy to QEMU disk from GameModulePkg. 'Test' is default."
	@echo "================="
	@echo "Available targets:"
	@echo "  all                - Build and do everything"
	@echo "  rebuild            - Build the EFI GameModulePkg and copy EFI_APP to QEMU disk"
	@echo "  clean              - Clean up build artifacts"
	@echo "  run                - Run QEMU with GUI"
	@echo "  run-text           - Run QEMU without GUI"
	@echo "================="
	@echo "  help               - Display this help message"

all: _check-dependencies build-basetools build-app build-ovmf _add-app _copy_ovmf
rebuild: _check-dependencies build-app _add-app _copy_ovmf

_create_conf_dir:
	@if [ -d "Conf" ]; then \
		echo "Skipping making Conf directory. It already exists."; \
	else \
		mkdir -p Conf; \
		echo "Created Conf directory."; \
	fi

build_basetools: _check-dependencies
	@echo "Starting BaseTools build..."
	@if [ -f "$(WORKSPACE)/edk2/BaseTools/Source/C/bin/BrotliCompress" ]; then \
		echo "BaseTools build already completed. Ignoring..."; \
	else \
		echo "Build not done yet. Running make..."; \
		. $(WORKSPACE)/edk2/edksetup.sh && cd edk2 && $(MAKE) -C BaseTools; \
	fi 

build-app: build_basetools _create_conf_dir _check-dependencies
	@. $(WORKSPACE)/edk2/edksetup.sh && cd Conf && build -a $(TARGET_ARCH) -t $(TOOL_CHAIN_TAG) -p $(GAMEMODULE_ACTIVE_PLATFORM) -Y COMPILE_INFO -y BuildReport.log -b $(BUILD_TARGET)

build-ovmf: build_basetools _create_conf_dir _check-dependencies
	@if [ -f "$(WORKSPACE)/Build/Ovmf$(TARGET_ARCH)/$(BUILD_TARGET)_$(TOOL_CHAIN_TAG)/FV/OVMF.fd" ]; then \
		echo "Skipping build-ovmf: $(WORKSPACE)/Build/Ovmf$(TARGET_ARCH)/$(BUILD_TARGET)_$(TOOL_CHAIN_TAG)/FV/OVMF.fd already exists."; \
	else \
		echo "Building OVMF..."; \
		. $(WORKSPACE)/edk2/edksetup.sh && cd Conf && build -a $(TARGET_ARCH) -t $(TOOL_CHAIN_TAG) -p $(OVMF_ACTIVE_PLATFORM) -b $(BUILD_TARGET); \
	fi

_create_qemu_dir:
	@if [ -d "efi-qemu" ]; then \
		echo "Removing efi-qemu directory..."; \
		rm -r efi-qemu; \
	fi
	@mkdir -p efi-qemu
	@echo "Created efi-qemu directory."

# Create disk image for the app
_create_disk_image: _create_qemu_dir _check-dependencies
	@echo "Creating disk image for the app..."
	@cd efi-qemu && \
		dd if=/dev/zero of=app.disk bs=1 count=1 seek=$$(( $(DISK_SIZEMB) * 1024 * 1024 - 1 )) && \
		sudo mkfs.vfat app.disk
	@echo "Disk image created and formatted as FAT."

# Add test app to the disk image
_add-app: _create_disk_image build-app 
	@mkdir -p efi-qemu/mnt_app
	@if [ ! -f "$(WORKSPACE)/Build/GameModule/$(BUILD_TARGET)_$(TOOL_CHAIN_TAG)/$(TARGET_ARCH)/$(APP_NAME).efi" ]; then \
		echo "$(WORKSPACE)/Build/GameModule/$(BUILD_TARGET)_$(TOOL_CHAIN_TAG)/$(TARGET_ARCH)/$(APP_NAME).efi not found"; \
		exit 1; \
	fi
	@sudo mount efi-qemu/app.disk efi-qemu/mnt_app
	@sudo cp -r $(WORKSPACE)/Build/GameModule/$(BUILD_TARGET)_$(TOOL_CHAIN_TAG)/$(TARGET_ARCH)/* efi-qemu/mnt_app || { sudo umount efi-qemu/mnt_app; exit 1; }
	@sudo umount efi-qemu/mnt_app
	@echo "Added EFI app to the disk image."

# Copy OVMF firmware image to QEMU directory
_copy_ovmf: _create_qemu_dir build-ovmf
	@echo "Copying OVMF firmware image to QEMU directory..."
	@cp "$(WORKSPACE)/Build/Ovmf$(TARGET_ARCH)/$(BUILD_TARGET)_$(TOOL_CHAIN_TAG)/FV/OVMF.fd" efi-qemu/ovmf.flash
	@echo "OVMF firmware image copied."

clean:
	rm -r $(WORKSPACE)/Build
	rm -r $(WORKSPACE)/Conf
	rm -r $(WORKSPACE)/efi-qemu

release:
	@echo "Starting release build of the app..."
	@. $(WORKSPACE)/edk2/edksetup.sh && cd Conf && build -a $(TARGET_ARCH) -t $(TOOL_CHAIN_TAG) -p $(GAMEMODULE_ACTIVE_PLATFORM) -Y COMPILE_INFO -y BuildReport.log -b RELEASE
	@echo "Release build done."

run:
	@./ovmf.sh

run-text:
	@./ovmf.sh --nographic
