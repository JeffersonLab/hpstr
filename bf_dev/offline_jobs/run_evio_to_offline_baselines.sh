#!/bin/bash

#evio to lcio
jid1=$(sbatch /sdf/group/hps/users/alspellm/projects/baselines/jlab/run_job/submit_slurm_evio_to_slcio.sh | sed 's/Submitted batch job //')
echo "${jid1} has been submitted"

jid2=$(sbatch --dependency=afterany:$jid1 /sdf/group/hps/users/alspellm/projects/baselines/jlab/run_job/submit_slurm_ntuples.sh | sed 's/Submitted batch job //')
echo "${jid2} has been submitted"

jid3=$(sbatch --dependency=afterany:$jid2 //sdf/group/hps/users/alspellm/projects/baselines/jlab/run_job/submit_slurm_2dhistos.sh | sed 's/Submitted batch job //')
echo "${jid3} has been submitted"

echo "hadding 2dhisto files to perform baseline fitting"
hadd ./2dhistos/hps_141143_bl2dhistos.root ./2dhistos/hps_14143_*_bl2dhisto.root

jid4=$(sbatch --dependency=afterany:$jid3 //sdf/group/hps/users/alspellm/projects/baselines/jlab/run_job/submit_slurm_blfit.sh | sed 's/Submitted batch job //')

echo "combining L0-L6 baselines into one root file"
hadd ./fits/hps_14143_offline_baseline_fits.root ./fits/hps_14143_*L*.root
rm ./fits/hps_14143_*L*.root

echo "Running baseline fit analysis python script"
python3 /sdf/home/a/alspellm/work/src/hpstr/bf_dev/generateBaselineCSV.py -i ./fits/hps_14143_offline_baseline_fits.root -o ./fits/hps_14143_offline_baseline_fits_analysis.root 

echo "Offline baseline fitting procedure complete"
