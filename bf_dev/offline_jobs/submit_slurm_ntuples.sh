#!/usr/bin/scl enable devtoolset-8 -- /bin/bash
#SBATCH --ntasks=1
#SBATCH --time=4:00:00
#SBATCH --mem=6000M
#SBATCH --array=1-20
#SBATCH --partition=shared
#SBATCH --job-name=rootuple

while getopts r:d:s:n: flag
do
    case "${flag}" in
        r) run=${OPTARG};;
        d) jobdir=${OPTARG};;
        s) rundir=${OPTARG};;
        n) first_id=${OPTARG};;
    esac

done

export FIRST_ID=${first_id}
export JOB_ID=$(($SLURM_ARRAY_TASK_ID+$FIRST_ID))
source /sdf/home/a/alspellm/.bashrc
export JOBDIR=$(readlink -f $jobdir)
runpath=$(readlink -f $rundir)
export RUNDIR=${runpath}/${run}/ntuples/${JOB_ID}
mkdir -p $RUNDIR
mkdir $RUNDIR/../logs
cd $RUNDIR

filename=$(basename -s .slcio ${JOBDIR}/${run}/lcio/hps*${JOB_ID}.slcio)
echo "FILENAME IS $filename"

hpstr /sdf/group/hps/users/alspellm/src/hpstr/processors/config/rawSvtHits_cfg.py -i ${JOBDIR}/${run}/lcio/*${JOB_ID}.slcio -o ${JOBDIR}/${run}/ntuples/${filename}.root -t 1 -y 2019 

