#!/bin/bash -e

cd "$(basename "$(basename "$(readlink "$0")")")"

SSH_HOST="${1:?Missing SSH host}"

mkdir -p build/{"Program Files (x86)/Common Files",ProgramData}
# scp ./scripts/dump_registry.ps1 "$SSH_HOST:"
# ssh "$SSH_HOST" "powershell .\dump_registry.ps1"
scp "$SSH_HOST:promt-registry.reg" build/registry.reg
ssh "$SSH_HOST" powershell -c 'rm promt-registry.reg'

scp -r "$SSH_HOST:/Program Files (x86)/Common Files/PROject MT" "./build/Program Files (x86)/Common Files/PROject MT"
scp -r "$SSH_HOST:/Program Files (x86)/PRMT6" "./build/Program Files (x86)/PRMT6"
scp -r "$SSH_HOST:/ProgramData/PROject MT" "./build/ProgramData/PROject MT"
