#!/bin/sh
hps-mc-batch slurm -q hps -m 7500 -n 4 -E /sdf/home/a/alspellm/.bashrc -o -r 1:1000 -d /scratch/alspellm/mc/2019/simps/gen  -l /scratch/alspellm/mc/2019/simps/gen/logs simp /sdf/group/hps/users/alspellm/run/mc/2019/simps/gen/jobs.json -c /sdf/group/hps/users/alspellm/run/mc/2019/simps/gen/.hpsmc
