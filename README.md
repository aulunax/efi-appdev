Script to setup efi and qemu for efi app development

### Prerequisites
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

### Installation
Run the setup-qemu.sh with sudo

If installation was successful:
``` sh
cd efi-qemu
../ovmf.sh
```

Inside qemu EFI shell type:
```
fs0:
HelloWorld.efi
```

If the app printed "UEFI Hello World!", everything is working correctly.

### Sources
- https://blog.3mdeb.com/2015/2015-11-21-uefi-application-development-in-ovmf/
- https://github.com/tianocore/tianocore.github.io/wiki/
- https://raw.githubusercontent.com/pietrushnic/edk2/ovmf-helloworld/ovmf.sh
