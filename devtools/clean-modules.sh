#!/usr/bin/env bash

# devtools/clean-modules.sh -- clean kernel modules against the QEMU kernel.
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
[ -n "$EXAMPLES_OUTPUT_DIR" ] || die "EXAMPLES_OUTPUT_DIR is not set."
[ -d "$EXAMPLES_OUTPUT_DIR" ] || die "Directory indicated by EXAMPLES_OUTPUT_DIR is not exist"
[ "$EXAMPLES_OUTPUT_DIR" != "/" ] || die "EXAMPLES_OUTPUT_DIR cannot be root directory!!!"

echo "Cleaning modules against kernel $KERNEL_VERSION ..."
# make -C "$EXAMPLES_DIR" KDIR="$KERNEL_BUILD" "$@" clean
make -C "$EXAMPLES_OUTPUT_DIR" KDIR="$KERNEL_BUILD" "$@" clean
rm -rf "$EXAMPLES_OUTPUT_DIR"
