#!/bin/bash

echo 'initializing wine...'
wineboot -i
echo 'copying registy values...'
wine regedit $WINEPREFIX/drive_c/registry.reg
echo 'starting...'
exec wine /app/promt-puppy.exe
