#!/bin/bash

MODULE_NAME="kernelware"

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

if [ $? -eq 0 ]; then
    echo "Module '${MODULE_NAME}' unloaded successfully"
else
    echo "Error: Failed to unload '${MODULE_NAME}' (may be in use)"
    exit 1
fi