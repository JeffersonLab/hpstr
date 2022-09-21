#!/usr/bin/scl enable devtoolset-8 -- /bin/bash
#SBATCH --ntasks=1
#SBATCH --time=4:00:00
#SBATCH --mem=6000M
#SBATCH --array=1-10
#SBATCH --partition=shared
#SBATCH --job-name=2dhistos
#SBATCH --output=/dev/null

OPTIND=1

#You must specify all flag options to run.
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

#Configure HPSTR and hps-mc environment 
export cfg_env=$(readlink -f $cfg_env)
export scratch_dir=$(readlink -f $scratch_dir)
export output_dir=$(readlink -f $output_dir)

source ${cfg_env}

export JOB_ID=$(($SLURM_ARRAY_TASK_ID))
export SCRATCHDIR=${scratch_dir}/2dhistos/${JOB_ID}
mkdir -p $SCRATCHDIR
mkdir $SCRATCHDIR/../logs
cd $SCRATCHDIR

#filename=$(basename -s .root ${JOBDIR}/${run}/ntuples/hps*${JOB_ID}.root)
inputfile=${SCRATCHDIR}/../../ntuples/${JOB_ID}/*.root
echo "2dhisto input file name is $filename"

echo 'Runing hpstr anaSvtBl2D_cfg'
hpstr ${HPSTR_BASE}/processors/config/anaSvtBl2D_cfg.py -i $inputfile -o ${SCRATCHDIR}/hps_${runnumber}_evio_${JOB_ID}_bl2dhisto.root -y $year

echo 'Copying files to output'
cp ${SCRATCHDIR}/hps_${runnumber}_evio_${JOB_ID}_bl2dhisto.root ${output_dir}/

