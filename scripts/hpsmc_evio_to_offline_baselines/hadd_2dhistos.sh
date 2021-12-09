#!/bin/bash
#SBATCH --ntasks=1
#SBATCH --time=1:00:00
#SBATCH --mem=6000M
#SBATCH --partition=shared
#SBATCH --job-name=haddhistos
#SBATCH --output=/dev/null

while getopts r:d: flag
do
    case "${flag}" in
        r) run=${OPTARG};;
        d) jobdir=${OPTARG};;
    esac

done
JOBDIR=$(readlink -f $jobdir)
filename=$(basename -s .root ${JOBDIR}/${run}/2dhistos/hps*${JOB_ID}.root)

hadd ${JOBDIR}/${run}/2dhistos/hps_${run}_bl2dhistos.root ${JOBDIR}/${run}/2dhistos/*bl2dhisto.root
