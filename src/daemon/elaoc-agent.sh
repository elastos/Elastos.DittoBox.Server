#!/bin/sh

HOST="$(uname -s)"
ARCH="$(uname -m)"

BUILD=debug

case "${HOST}" in
    "Darwin")
        DSO_ENV=DYLD_LIBRARY_PATH
        ;;
    "Linux")
        DSO_ENV=LD_LIBRARY_PATH
        ;;
    *)
        echo "Error: Unsupported platform ${HOST}"
        exit 1;;
esac

export ${DSO_ENV}=${CARRIER_DIST_PATH}/${HOST}-${ARCH}/${BUILD}/lib

if [ ! -e ${PWD}/elaoc-agentd ]; then
    echo "Error: elaoc-agentd not available."
    exit 1
fi

./elaoc-agentd -c ${PWD}/elaoc-agent.conf --foreground $*

exit 0

