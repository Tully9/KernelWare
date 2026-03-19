#!/bin/bash

MODULE_NAME="kernelware"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MODULE_PATH="${SCRIPT_DIR}/../driver/kernelware.ko"

#check root
if [ "$(id -u)" -ne 0 ]; then
    echo "Error: Must be run as root"
    exit 1
fi

#check loaded
if ! lsmod | grep -q "^${MODULE_NAME}"; then
    echo "Module '${MODULE_NAME}' is not loaded"
    exit 0
fi

#remove
rmmod "${MODULE_NAME}"

#check .ko exists and make clean
if [ -f "${MODULE_PATH}" ]; then
    echo "Module file '${MODULE_PATH}' found, making clean"
    make -C "${SCRIPT_DIR}/../driver" clean
fi

if [ $? -eq 0 ]; then
    echo "Module '${MODULE_NAME}' unloaded successfully"
else
    echo "Error: Failed to unload '${MODULE_NAME}' (may be in use)"
    exit 1
fi