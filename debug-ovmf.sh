#!/bin/bash

# WIP: This script is a work in progress and is not yet functional

WORKSPACE=$(pwd)
DEBUG_FILE="$WORKSPACE/efi-qemu/debug.log"
APP_FILE="Test.efi"

ATTEMPTS=0
MAX_ATTEMPTS=30
LOAD_ADDRESS=

#./ovmf.sh &
echo "Waiting for QEMU..."
sleep 1

# Loop to find address of app you are launching
while [ $ATTEMPTS -lt $MAX_ATTEMPTS ]; do
    echo "Attempting to read load address of $APP_FILE (Attempt $((ATTEMPTS + 1)))"
    LOAD_ADDRESS=$(tac "$DEBUG_FILE" | grep -oP -m 1 "Loading driver at 0x[0-9A-Fa-f]+ EntryPoint=0x[0-9A-Fa-f]+ $APP_FILE" | sed -n 's/Loading driver at 0x\([0-9A-Fa-f]\+\).*/\1/p')

    if [ -n "$LOAD_ADDRESS" ]; then
        echo "Found app address. LOAD_ADDRESS=0x$LOAD_ADDRESS"
        break
    fi

    # Increment attempts and wait 1 second before retrying
    ATTEMPTS=$((ATTEMPTS+1))
    sleep 2
done

cd $WORKSPACE/Build/GameModule/DEBUG_GCC5/X64

TEXT_OFFSET=$(gdb -batch -ex "file $APP_FILE" -ex "info files" \
    | grep -oP "0x[0-9A-Fa-f]+ - 0x[0-9A-Fa-f]+ is \.text" \
    | sed -n 's/0x\([0-9A-Fa-f]\+\).*/\1/p')
TEXT_OFFSET=$(echo "$TEXT_OFFSET" | tr 'a-f' 'A-F')

echo "TEXT_OFFSET=0x$TEXT_OFFSET"

DATA_OFFSET=$(gdb -batch -ex "file $APP_FILE" -ex "info files" \
    | grep -oP "0x[0-9A-Fa-f]+ - 0x[0-9A-Fa-f]+ is \.data" \
    | sed -n 's/0x\([0-9A-Fa-f]\+\).*/\1/p')
DATA_OFFSET=$(echo "$DATA_OFFSET" | tr 'a-f' 'A-F')

echo "DATA_OFFSET=0x$DATA_OFFSET"

TEXT_SEGMENT=$(printf "0x%X" $(echo "ibase=16; $LOAD_ADDRESS + $TEXT_OFFSET" | bc))
DATA_SEGMENT=$(printf "0x%X" $(echo "ibase=16; $LOAD_ADDRESS + $DATA_OFFSET" | bc))

echo "TEXT_SEGMENT=$TEXT_SEGMENT"
echo "DATA_SEGMENT=$DATA_SEGMENT"

cd $WORKSPACE/edk2/BaseTools/Scripts

gdb -ex "set pagination off" \
    -ex "set confirm off" \
    -ex "set architecture i386:x86-64" \
    -ex "file" \
    -ex "target remote localhost:1234" \
    -ex "source efi_gdb.py" \
    -ex "set confirm on" \
    -ex "set pagination on"

#    -ex "add-symbol-file $WORKSPACE/Build/GameModule/DEBUG_GCC5/X64/Test.debug $TEXT_SEGMENT -s .data $DATA_SEGMENT" \
#    -ex "break UefiTestMain" \
#    -ex "break Test.c:98" \

#-ex "source $WORKSPACE/DebugPkg/Scripts/gdb_uefi.py" \
#    -ex "reload-uefi -o $WORKSPACE/Build/GameModule/DEBUG_GCC5/X64/DebugPkg/GdbSyms/GdbSyms/DEBUG/GdbSyms.dll" \




