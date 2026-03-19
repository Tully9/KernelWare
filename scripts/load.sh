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
if lsmod | grep -q "^${MODULE_NAME}"; then
    echo "Module '${MODULE_NAME}' is already loaded"
    exit 0
fi

#check .ko exists and make
if [ ! -f "${MODULE_PATH}" ]; then
    echo "Module file '${MODULE_PATH}' not found, making file"
    make -C "${SCRIPT_DIR}/../driver"
fi

#check .ko exits after make
if [ ! -f "${MODULE_PATH}" ]; then
    echo "Error: Module file '${MODULE_PATH}' not found"
    exit 1
fi

#insert
insmod "${MODULE_PATH}"

if [ $? -eq 0 ]; then
    echo "Module '${MODULE_NAME}' loaded successfully"
else
    echo "Error: Failed to load module '${MODULE_NAME}'"
fi