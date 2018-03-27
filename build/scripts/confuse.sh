#!/bin/sh

#argv1: HOST
#argv2: ARCH
#argv3: HOST_COMPILER

getConfigureCmd() {
    case "$1,$2" in
        "Darwin","x86_64" | "Linux","x86_64" | "Raspbian",*)
            _TOOL="./configure"
            ;;
        "Linux","armv7l")
            _TOOL="./configure --host=$3"
            ;;
        *,*)
            echo "Not supported platform:Arch ($1:$2)"
            exit 1;;
    esac

    echo ${_TOOL}
}

if [ x"$1" = x"command" ]; then
    getConfigureCmd $2 $3 $4
    exit 0
fi

exit 0

