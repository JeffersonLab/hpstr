#!/bin/sh
hps-mc-batch slurm -q shared -W 5 -m 2000 -E /sdf/home/a/alspellm/.bashrc -o -r 26:50 -d /lscratch/alspellm/mc/3pt7/wab_beam/test/  -l /lscratch/alspellm/mc/3pt7/wab_beam/test/logs wab_gen_to_slic /sdf/group/hps/users/alspellm/run/mc/3pt7/wab_beam/wab_gen_to_slic/jobs.json -c /sdf/group/hps/users/alspellm/run/mc/3pt7/wab_beam/wab_gen_to_slic/.hpsmc
