#!/bin/bash
#SBATCH --ntasks=1
#SBATCH --time=1:00:00
#SBATCH --mem=6000M
#SBATCH --partition=shared
#SBATCH --job-name=haddhistos
#SBATCH --output=/dev/null

OPTIND=1

while getopts r:o:s:c: flag
do
    case "${flag}" in
        r) run=${OPTARG};;
        o) output_dir=${OPTARG};;
        s) scratch_dir=${OPTARG};;
        c) cfg_env=${OPTARG};;
    esac
done

source ${cfg_env}

#Configure HPSTR and hps-mc environment 
export cfg_env=$(readlink -f $cfg_env)
export scratch_dir=$(readlink -f $scratch_dir)
export output_dir=$(readlink -f $output_dir)

export JOB_ID=$(($SLURM_ARRAY_TASK_ID))

filename=$(basename -s .root ${JOBDIR}/${run}/2dhistos/hps*${JOB_ID}.root)

echo 'hadding histograms'
hadd ${scratch_dir}/2dhistos/hadd_hps_${runnumber}_evio_${JOB_ID}_bl2dhistos.root ${scratch_dir}/2dhistos/*/*bl2dhisto.root

echo 'Copying files to output'
cp ${scratch_dir}/2dhistos/hadd*.root ${output_dir}
