#!/usr/bin/scl enable devtoolset-8 -- /bin/bash
#SBATCH --ntasks=1
#SBATCH --time=1:00:00
#SBATCH --mem=3000M
#SBATCH --array=1-10
#SBATCH --partition=hps
#SBATCH --job-name=tuple
#SBATCH --output=/dev/null

OPTIND=1

while getopts r:o:e:s:y: flag
do
    case "${flag}" in
        r) runnumber=${OPTARG};;
        o) output_dir=${OPTARG};;
        e) cfg_env=${OPTARG};;
        s) scratch_dir=${OPTARG};;
        y) year=${OPTARG};; 

    esac

done

cfg_env=$(readlink -f $cfg_env)
output_dir=$(readlink -f $output_dir)
scratch_dir=$(readlink -f $scratch_dir)

source ${cfg_env}

export JOB_ID=$(($SLURM_ARRAY_TASK_ID))
export SCRATCHDIR=${scratch_dir}/ntuples/${JOB_ID}
mkdir -p $SCRATCHDIR
mkdir $SCRATCHDIR/../logs
cd $SCRATCHDIR

#filename=$(basename -s .slcio ${SCRATCHDIR}/lcio/${JOB_ID})/data_events.slcio
inputfile=${SCRATCHDIR}/../../lcio/${JOB_ID}/data_events.slcio

echo 'Running HPSTR'
hpstr $HPSTR_BASE/processors/config/rawSvtHits_cfg.py -i ${inputfile} -o ${SCRATCHDIR}/hps_${runnumber}_evio_${JOB_ID}.root -t 1 -y $year

echo 'Copying files to output'
cp ${SCRATCHDIR}/hps_${runnumber}_evio_${JOB_ID}.root ${output_dir}/

