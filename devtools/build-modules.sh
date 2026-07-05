#!/usr/bin/env bash

# devtools/build-modules.sh -- build kernel modules against the QEMU kernel.
# Wraps the existing examples/Makefile with the correct KDIR.
#
# Must run on Linux (the kernel build tree contains Linux-only generated files).

set -euo pipefail

DEVTOOLS_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$DEVTOOLS_DIR/.." && pwd)"
. "$DEVTOOLS_DIR/config.defaults"
[ -f "$DEVTOOLS_DIR/config.local" ] && . "$DEVTOOLS_DIR/config.local"

die() { echo "ERROR: $*" >&2; exit 1; }

[ -d "$KERNEL_BUILD" ] || die "Kernel not built. Run devtools/setup.sh first."
[ -f "$KERNEL_BUILD/Module.symvers" ] || die "Kernel modules not prepared. Run devtools/setup.sh first."

if [ "$(uname)" = "Darwin" ]; then
    die "Modules must be cross-compiled on Linux.
  lima devtools/build-modules.sh"
fi

echo "Building modules against kernel $KERNEL_VERSION ..."
make -C "$EXAMPLES_DIR" KDIR="$KERNEL_BUILD" "$@"

echo "Install modules with dependencies ..."
CP_TARGET_DIR="$EXAMPLES_MOUNT_DIR/lib/modules/$KERNEL_VERSION"
if [ -d "$EXAMPLES_MOUNT_DIR" ]; then
  rm -rf "$EXAMPLES_MOUNT_DIR"
fi
mkdir -p "$EXAMPLES_MOUNT_DIR/lib/modules/$KERNEL_VERSION"

cp $EXAMPLES_DIR/modules.order $CP_TARGET_DIR
cp $KERNEL_BUILD/modules.builtin $CP_TARGET_DIR
cp $KERNEL_BUILD/modules.builtin.modinfo $CP_TARGET_DIR

make -C "$KERNEL_BUILD" M="$EXAMPLES_DIR" INSTALL_MOD_PATH="$EXAMPLES_MOUNT_DIR" modules_install
