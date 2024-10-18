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

### Sources
- https://blog.3mdeb.com/2015/2015-11-21-uefi-application-development-in-ovmf/
- https://github.com/tianocore/tianocore.github.io/wiki/
