#!/usr/bin/scl enable devtoolset-8 -- /bin/bash
#SBATCH --ntasks=1
#SBATCH --time=4:00:00
#SBATCH --mem=6000M
#SBATCH --array=1-7
#SBATCH --partition=shared
#SBATCH --job-name=blfits
#SBATCH --output=/dev/null

OPTIND=1
#You must specify all flag options to run.
while getopts r:t:o:e:s:y: flag
do
    case "${flag}" in
        r) export runnumber=${OPTARG};;
        t) export thresholds_file=${OPTARG};;
        o) export output_dir=${OPTARG};;
        e) export cfg_env=${OPTARG};;
        s) export scratch_dir=${OPTARG};;
        y) export year=${OPTARG};;
    esac

done

source $cfg_env

export cfg_env=$(readlink -f $cfg_env)
export scratch_dir=$(readlink -f $scratch_dir)
export output_dir=$(readlink -f $output_dir)
export thresholds_file=$(readlink -f $thresholds_file)

export FIRST_ID=-1
export JOB_ID=$(($SLURM_ARRAY_TASK_ID+$FIRST_ID))
export SCRATCHDIR=${scratch_dir}/blfits/${JOB_ID}
mkdir -p $SCRATCHDIR
mkdir $SCRATCHDIR/../logs
cd $SCRATCHDIR

echo "Fitting baselines for layer ${JOB_ID}"
hpstr ${HPSTR_BASE}/processors/config/fitBL_cfg.py -i ${SCRATCHDIR}/../../2dhistos/hadd_hps_${runnumber}*bl2dhistos.root -o ${SCRATCHDIR}/hps_${runnumber}_offline_baselines_L${JOB_ID}.root -l L${JOB_ID} --thresh ${thresholds_file} -y $year

echo "Copying files to output"
cp ${SCRATCHDIR}/hps_${runnumber}_offline_baselines_L${JOB_ID}.root $output_dir
