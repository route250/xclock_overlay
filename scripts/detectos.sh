#!/bin/bash

if [ -f /etc/redhat-release ]; then
    echo "el9"
elif grep -q "openSUSE" /etc/os-release 2>/dev/null; then
    echo "suse"
else
    echo "unknown"
fi
