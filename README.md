# EFI Application Game (TBD)
Game application in EFI/UEFI. Includes setup for EDK2 and QEMU for EFI/UEFI app development

## Prerequisites
Install all required packages:
- build-essential
- uuid-dev 
- iasl 
- git
- nasm
- python-is-python3
- gcc 5+ (Check version with 'gcc --version')

And qemu:
- qemu-system

## Installation
Run the setup-edk.sh to download edk2 repository from Github.
```sh
./setup-edk.sh
```
After that is finished, build everything by running:
```sh
make all
```

If build was successful, to launch QEMU:
``` sh
./ovmf.sh
```

To launch the EFI application inside QEMU, type in EFI shell:
```
fs0:
ApplicationName.efi
```

'Test' is the default app to be copied to QEMU disk. You can change that by passing <b>EFI_APP=AppName</b> to make. 
<br/>
Example:
```sh
make rebuild EFI_APP=Tetris
```

## Sources
- https://blog.3mdeb.com/2015/2015-11-21-uefi-application-development-in-ovmf/
- https://github.com/tianocore/tianocore.github.io/wiki/
- https://raw.githubusercontent.com/pietrushnic/edk2/ovmf-helloworld/ovmf.sh
