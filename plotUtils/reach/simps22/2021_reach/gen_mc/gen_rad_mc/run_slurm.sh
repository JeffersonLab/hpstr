#!/bin/sh
hps-mc-batch slurm -q shared -m 4500 -n 1 -E /sdf/home/a/alspellm/.bashrc -o -r 736:1000 -d /scratch/alspellm/mc/3pt7/rad_beam/merge_to_recon/ -l /scratch/alspellm/mc/3pt7/rad_beam/merge_to_recon/logs signal_beam_merge_to_recon /sdf/group/hps/users/alspellm/run/mc/3pt7/rad_beam/rad_beam_merge_to_recon/jobs.json -c /sdf/group/hps/users/alspellm/run/mc/3pt7/rad_beam/rad_beam_merge_to_recon/.hpsmc