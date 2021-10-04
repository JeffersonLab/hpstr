#!/usr/bin/scl enable devtoolset-8 -- /bin/bash
#SBATCH --ntasks=1
#SBATCH --time=4:00:00
#SBATCH --mem=6000M
#SBATCH --array=1-20
#SBATCH --partition=shared
#SBATCH --job-name=2dhistos

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

#for file in ${JOBDIR}/${run}/ntuples/
#do 
#    if [[ "$file" == *"$JOB_ID"* ]]
#    then
#        export filename=$(basename -s .root $file)
#    else
#        echo "NO NTUPLE FILE WITH ID ${JOB_ID} FOUND. EXITING"
#        exit 1
#    fi

filename=$(basename -s .root ${JOBDIR}/${run}/ntuples/hps*${JOB_ID}.root)
echo "ntuple input file name is $filename"

hpstr ${HPSTR_BASE}/processors/config/anaSvtBl2D_cfg.py -i ${JOBDIR}/${run}/ntuples/*${JOB_ID}.root -o $JOBDIR/${run}/2dhistos/${filename}_bl2dhisto.root

