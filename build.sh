#!/usr/bin/env bash
set -e

TARGET=${1:-All}
CONFIG=${2:-Debug}

if [ "$TARGET" = "clean" ]; then
    echo "[Bun] Cleaning build outputs..."
    rm -rf Intermediate Binaries
    echo "[Bun] Clean done."
    exit 0
fi

if [ "$TARGET" = "cleanall" ]; then
    echo "[Bun] Cleaning build outputs and tool binaries..."
    rm -rf Intermediate Binaries Build/bin
    echo "[Bun] Full clean done."
    exit 0
fi

export DOTNET_SYSTEM_GLOBALIZATION_INVARIANT=1

OS="$(uname -s)"
ARCH="$(uname -m)"

if [ "$OS" = "Darwin" ]; then
    RID=$( [ "$ARCH" = "arm64" ] && echo "osx-arm64" || echo "osx-x64" )
else
    RID=$( [ "$ARCH" = "aarch64" ] && echo "linux-arm64" || echo "linux-x64" )
fi

TOOL="./Build/bin/${RID}/BunBuildTool"

if [ ! -f "$TOOL" ]; then
    echo "[Bun] Build tool not found. Run ./setup.sh first."
    exit 1
fi

"$TOOL" "$TARGET" "$CONFIG"