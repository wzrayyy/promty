#!/bin/bash -e
# shellcheck disable=SC1091

SCRIPT_DIR="$(dirname "$(dirname "$(readlink -e "$0")")")"
SSH_HOST="${1:?Missing SSH host}"

mkdir -p build/{"Program Files (x86)/Common Files",ProgramData}
scp "$SCRIPT_DIR/scripts/dump_registry.ps1" "$SSH_HOST:"
ssh "$SSH_HOST" "powershell .\dump_registry.ps1"
scp "$SSH_HOST:promt-registry.reg" build/registry.reg
ssh "$SSH_HOST" powershell -c 'rm promt-registry.reg'

scp -r "$SSH_HOST:/Program Files (x86)/Common Files/PROject MT" "$SCRIPT_DIR/build/Program Files (x86)/Common Files/PROject MT"
scp -r "$SSH_HOST:/Program Files (x86)/PRMT6" "$SCRIPT_DIR/build/Program Files (x86)/PRMT6"
scp -r "$SSH_HOST:/ProgramData/PROject MT" "$SCRIPT_DIR/build/ProgramData/PROject MT"

export WINETRICKS_LIB=1
. winetricks 2>/dev/null

winetricks_init
winetricks_vcrun6_helper
w_try_cabextract "${W_CACHE}"/vcrun6/vcredist.exe -d "$SCRIPT_DIR/build/windows/" -F "mfc42*.dll"
mkdir -p "$SCRIPT_DIR/build/windows/system32"

date +'%s' > "$SCRIPT_DIR/build/build-date"
