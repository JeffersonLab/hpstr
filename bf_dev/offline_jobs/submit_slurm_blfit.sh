#!/usr/bin/scl enable devtoolset-8 -- /bin/bash
#SBATCH --ntasks=1
#SBATCH --time=4:00:00
#SBATCH --mem=6000M
#SBATCH --array=1-7
#SBATCH --partition=shared
#SBATCH --job-name=blfits

while getopts r:d:s: flag
do
    case "${flag}" in
        r) run=${OPTARG};;
        d) jobdir=${OPTARG};;
        s) rundir=${OPTARG};;
    esac

done

export FIRST_ID=-1
export JOB_ID=$(($SLURM_ARRAY_TASK_ID+$FIRST_ID))
source /sdf/home/a/alspellm/.bashrc
export JOBDIR=$(readlink -f $jobdir)
runpath=$(readlink -f $rundir)
export RUNDIR=${runpath}/${run}/fits/${JOB_ID}

mkdir -p $RUNDIR
cd $RUNDIR

hpstr ${HPSTR_BASE}/processors/config/fitBL_cfg.py -i ${JOBDIR}/${run}/2dhistos/hps_${run}_bl2dhistos.root -o $JOBDIR/${run}/fits/hps_${run}_offline_baselines_L${JOB_ID}.root -l L${JOB_ID}

