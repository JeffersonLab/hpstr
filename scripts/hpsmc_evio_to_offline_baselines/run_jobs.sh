#!/bin/bash

while getopts r:d:s:n: flag
do
    case "${flag}" in
        r) run=${OPTARG};;  
        d) jobdir=${OPTARG};;
        s) rundir=${OPTARG};;
        n) first_id=${OPTARG};; 
    esac

done

source ./mkrundirectories.sh -r ${run}

#evio to lcio
jid1=$(sbatch ./submit_slurm_evio_to_slcio.sh -r ${run} -d ${jobdir} -s ${rundir} | sed 's/Submitted batch job //') 
echo "${jid1} (evio to slcio) has been submitted"

jid2=$(sbatch --dependency=afterany:$jid1 ./submit_slurm_ntuples.sh -r ${run} -d ${jobdir} -s ${rundir} -n ${first_id} | sed 's/Submitted batch job //') 
echo "${jid2} (slcio to ntuple) has been submitted"

jid3=$(sbatch --dependency=afterany:$jid2 ./submit_slurm_2dhistos.sh -r ${run} -d ${jobdir} -s ${rundir} -n ${first_id}| sed 's/Submitted batch job //') 
echo "${jid3} (RawSvtHit 2dhistos) has been submitted"

echo "hadding 2dhisto files to perform baseline fitting"
jid4=$(sbatch --dependency=afterany:$jid3 ./hadd_2dhistos.sh -r ${run} -d ${jobdir} | sed 's/Submitted batch job //') 

jid5=$(sbatch --dependency=afterany:$jid4 ./submit_slurm_blfit.sh -r ${run} -d ${jobdir} -s ${rundir} | sed 's/Submitted batch job //') 

