#!/usr/bin/scl enable devtoolset-8 -- /bin/bash
#SBATCH --ntasks=1
#SBATCH --time=4:00:00
#SBATCH --mem=6000M
#SBATCH --array=1-20
#SBATCH --partition=shared
#SBATCH --job-name=evio_slcio

while getopts r:d:s:c: flag
do
    case "${flag}" in
        r) run=${OPTARG};;
        d) jobdir=${OPTARG};;
        s) rundir=${OPTARG};;
        c) cfg_env=${OPTARG};;
    esac

done

export FIRST_ID=0
export JOB_ID=$(($SLURM_ARRAY_TASK_ID+$FIRST_ID))
source ${cfg_env}/setup.sh
export JOBDIR=$(readlink -f $jobdir)
runpath=$(readlink -f $rundir)
export RUNDIR=${runpath}/${run}/evio/${JOB_ID}
echo $RUNDIR
mkdir -p $RUNDIR
mkdir $RUNDIR/../logs
cd $RUNDIR
/usr/bin/python3 ${HPSMC_DIR}/lib/python/hpsmc/job.py run -o $RUNDIR/../logs/job.${JOB_ID}.out -e $RUNDIR/../logs/job.${JOB_ID}.err -l $RUNDIR/../logs/job.${JOB_ID}.log -d $RUNDIR -c $JOBDIR/.hpsmc -i $JOB_ID ${HPSMC_DIR}/../python/jobs/data_cnv_job.py $JOBDIR/jobs_${run}.json


