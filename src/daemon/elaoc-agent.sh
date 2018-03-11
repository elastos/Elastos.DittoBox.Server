#!/bin/sh

HOST="$(uname -s)"
ARCH="$(uname -m)"

BUILD=debug

MODE=$1

case ${MODE} in
    "client")
        CONF=client.conf
        ;;
    "server")
        CONF=elaoc-agent.conf
        ;;
    *)
        echo "Error: Invalid command syntax."
        echo "USAGE: ./elaoc-agentd.sh client | server"
        echo ""
        ;;
esac

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

./elaoc-agentd -c ${PWD}/${CONF} --foreground $*

exit 0

