#!/usr/bin/scl enable devtoolset-8 -- /bin/bash
#SBATCH --ntasks=1
#SBATCH --time=1:00:00
#SBATCH --mem=3000M
#SBATCH --array=1-10
#SBATCH --partition=hps
#SBATCH --job-name=evio_slcio
#SBATCH --output=/dev/null
OPTIND=1

while getopts r:j:o:e:s: flag
do
    case "${flag}" in
        r) runnumber=${OPTARG};;
        j) jobjson=${OPTARG};;
        o) output_dir=${OPTARG};;
        e) cfg_env=${OPTARG};;
        s) scratch_dir=${OPTARG};;
    esac

done

cfg_env=$(readlink -f $cfg_env)
jobjson=$(readlink -f $jobjson)
output_dir=$(readlink -f $output_dir)
scratch_dir=$(readlink -f $scratch_dir)

export JOB_ID=$(($SLURM_ARRAY_TASK_ID))

export SCRATCHDIR=${scratch_dir}/lcio/${JOB_ID}
mkdir -p $SCRATCHDIR
mkdir $SCRATCHDIR/../logs
cd $SCRATCHDIR

#Configure HPSTR and hps-mc
source $cfg_env

/usr/bin/python3 ${HPSMC_DIR}/lib/python/hpsmc/job.py run -o $SCRATCHDIR/../logs/job.${JOB_ID}.out -e $SCRATCHDIR/../logs/job.${JOB_ID}.err -l $SCRATCHDIR/../logs/job.${JOB_ID}.log -d $SCRATCHDIR -i $JOB_ID ${HPSMC_DIR}/../python/jobs/data_cnv_job.py $jobjson


