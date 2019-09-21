#!/usr/bin/env bash

if [ $# -lt 1 ]; then
    echo "Missing required parameter: name of directory to zip.";
    exit 1
fi

tar --preserve-permissions -czf "${1%/}.tar.gz" "$1"
