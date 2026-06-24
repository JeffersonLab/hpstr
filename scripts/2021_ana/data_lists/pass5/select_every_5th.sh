#!/bin/bash
# Select every 5th folder from the merged directory and list all files within them.

BASE_DIR="/sdf/data/hps/physics2021/data/recon/pass5_v9/merged/merge-batch-1/"

mapfile -t folders < <(ls -d "${BASE_DIR}"/*)

count=0
for folder in "${folders[@]}"; do
    count=$((count + 1))
    if (( count % 5 == 1 )); then
        find "$folder" -maxdepth 1 -type f
    fi
done
