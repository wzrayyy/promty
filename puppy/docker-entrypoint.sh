#!/bin/bash
# shellcheck disable=SC2155

echo 'initializing wine...'
wineboot -i

echo 'copying registy values...'
wine regedit "$WINEPREFIX"/drive_c/registry.reg

export FAKETIME="$(cat "$WINEPREFIX"/drive_c/build-date)"
echo "starting with date $(date -d @"$FAKETIME")..."
exec wine /app/promt-puppy.exe
