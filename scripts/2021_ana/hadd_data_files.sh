#!/bin/bash

FILEPATH="/sdf/data/hps/physics2021/data/recon/pass4_v8"

for dir in "$FILEPATH"/hps_*; do
  runnum=$(basename "$dir" | cut -d'_' -f2)
  echo $runnum
  hadd "$FILEPATH/hps_${runnum}/analysis/hps_${runnum}_sum1pct_ana.root" "$FILEPATH/hps_${runnum}/analysis/hps_${runnum}_"*_v0skim.root
done

