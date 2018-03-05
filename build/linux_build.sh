#!/bin/sh

TARGET_OS="Linux"
TARGET_ARCH="$(uname -m)"
TARGET_MAKE="install"
TARGET_BUILD="debug"
TARGET_MODULE="elaoc-agent"

set -e

if [ x"$(uname -s)" != x"Linux" ]; then
    echo "Error: $0 should run on Linux"
    exit 1
fi

while [ x"$1" != x ]; do
case "$1" in
    "x86")
        echo "Error: Architecture ($1) not supported yet"
        exit 1
        ;;

    "x86_64" | "armv7l")
        TARGET_ARCH="$1"
        shift;;

    "confuse" | "elaoc-agent")
        TARGET_MODULE="$1"
        shift;;

    "source" | "config" | "make" | "install" | \
    "source-clean" | "config-clean" | "clean" | "uninstall" | "dist")
        TARGET_MAKE="$1"
        shift;;

    "debug" | "release")
        TARGET_BUILD="$1"
        shift;;

    *)
        echo "Usage: $0 [ module ] [ arch ] [ target ] [ build ]"
        echo "module options(default: elaoc-agent):"
        echo "    confuse      | elaoc-gent"
        echo ""
        echo "arch options(default $(uname -m)):"
        echo "    x86_64       | armv7l(raspberry)"
        echo ""
        echo "target options(default:install):"
        echo "    source       | config       | make    | install"
        echo "    source-clean | config-clean | clean   | uninstall"
        echo "    dist"
        echo ""
        echo "build options(default:debug):"
        echo "    debug        | release"
        echo ""
        exit 0
        ;;
esac
done

if [ x"$(uname -m)" != x"${TARGET_ARCH}" ]; then
    echo "Using cross-compilation for on $(uname -s)-$(uname -m) ..."

    if [ -z "${RASPBERRY_TOOLCHAIN_HOME}" ]; then
        echo "Error: RASPBERRY_TOOLCHAIN_HOME environment not set"
        exit 1
    fi
fi

if [ -z "${CARRIER_DIST_PATH} ]; then
    echo "Error: CARRIER_DIST_PATH environment not set"
    exit
fi

MODULE=${TARGET_MODULE} \
    TARGET=${TARGET_MAKE} \
    HOST=${TARGET_OS} \
    ARCH=${TARGET_ARCH} \
    BUILD=${TARGET_BUILD} make ${TARGET_MODULE}

