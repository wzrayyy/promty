#!/bin/bash

wineboot -i
wine regedit $WINEPREFIX/drive_c/registry.reg
exec wine /app/promt-puppy.exe
