#!/bin/sh
hps-mc-batch slurm -q shared -m 8500 -n 4 -E /sdf/home/a/alspellm/.bashrc -o -r 1:1000 -d /scratch/alspellm/mc/3pt7/rad_beam/gen/  -l /scratch/alspellm/mc/3pt7/rad_beam/gen/logs rad_gen_to_slic /sdf/group/hps/users/alspellm/run/mc/3pt7/rad_beam/rad_gen_to_slic/jobs.json -c /sdf/group/hps/users/alspellm/run/mc/3pt7/rad_beam/rad_gen_to_slic/.hpsmc
