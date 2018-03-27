#!/bin/sh

TARGET_OS="Darwin"
TARGET_ARCH="$(uname -m)"
TARGET_MAKE="install"
TARGET_BUILD="debug"
TARGET_MODULE="elaoc-agent"

set -eo pipefail

if [ x"$(uname -s)" != x"Darwin" ]; then
    echo "Error: $0 should run on Darwin OS"
    exit 1
fi

while [ x"$1" != x ]; do
case "$1" in
    "x86_64")
        TARGET_ARCH=$1
        shift;;

    "x86")
        TARGET_ARCH=$1
        echo "Error: Not supported for arch $1"
        exit 1;;

    "confuse" | "elaoc-agent")
        TARGET_MODULE=$1
        shift;;

    "source" | "config" | "make" | "install" | \
    "source-clean" | "config-clean" | "clean" | "uninstall" | "dist")
        TARGET_MAKE=$1
        shift;;

    "debug" | "release")
        TARGET_BUILD="$1"
        shift;;

    *)
        echo "Usage: $0 [ module ] [ arch ] [ target ] [ build ]"
        echo "module options(default elaoc-agent):"
        echo "    confuse      | elaoc-agent"
        echo ""
        echo "arch options(default $(uname -m)):"
        echo "    x86_64       | x86(i386)"
        echo ""
        echo "target options(default install):"
        echo "    source       | config       | make     | install"
        echo "    source-clean | config-clean | clean    | uninstall"
        echo "    dist"
        echo ""
        echo "build options(default:debug):"
        echo "    debug        | release"
        echo ""
        exit 0
	;;
esac
done

if [ -z "${CARRIER_DIST_PATH}" ]; then
    echo "Error: CARRIER_DIST_PATH environment not set"
    exit
fi

MODULE=${TARGET_MODULE} \
    TARGET=${TARGET_MAKE} \
    HOST=${TARGET_OS} \
    ARCH=${TARGET_ARCH} \
    BUILD=${TARGET_BUILD} make ${TARGET_MODULE}

