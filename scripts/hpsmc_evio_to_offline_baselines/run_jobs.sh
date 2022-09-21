#!/bin/bash
OPTIND=1

#### Congifugre slurm environment by pointing to HPSTR and HPS-MC installation  ####
cfg_env=/sdf/group/hps/users/alspellm/run/dev_offlinebaselines/config_env.sh         # <-MAKE SURE TO MODIFY PATHS HERE

#You must specify all flag options to run.
while getopts r:j:t:o:s:y: flag
do
    case "${flag}" in
        r) runnumber=${OPTARG};;  
        j) jobjson=${OPTARG};;
        t) thresholds_file=${OPTARG};;
        o) output_dir=${OPTARG};;
        s) scratch_dir=${OPTARG};;
        y) year=${OPTARG};; 
    esac

done

#Get full paths if not provided by command flags
jobjson=$(readlink -f $jobjson)
scratch_dir=$(readlink -f $scratch_dir)
output_dir=$(readlink -f $output_dir)
thresholds_file=$(readlink -f $thresholds_file)

echo "Running on hps run $runnumber"
echo "Run batch jobs in scratch directory $scratch_dir"
echo "Set output folder: $output_dir"
echo "jobs.json file: ${jobjson}"
echo "SVT Thresholds File For Run ${runnumber}: $thresholds_file"

####   CONFIGURE SLURM  #### 
export time='1:00:00'
export mem='6000M'
export array='1-10'
export partition=shared

#evio to lcio
export job_name='evio_lcio'
jid1=$(sbatch --time $time --mem $mem --array $array --partition $partition --job-name $job_name ./submit_slurm_evio_to_slcio.sh -r ${runnumber} -j $jobjson -e $cfg_env -s $scratch_dir -o $output_dir | sed 's/Submitted batch job //') 
echo "${jid1} (evio to slcio) has been submitted"

export job_name='tuple'
jid2=$(sbatch --dependency=afterany:$jid1 --time $time --mem $mem --array $array --partition $partition --job-name $job_name ./submit_slurm_ntuples.sh -r ${runnumber} -e $cfg_env -s $scratch_dir -o $output_dir -y $year | sed 's/Submitted batch job //') 
echo "${jid2} (slcio to ntuple) has been submitted"

export job_name='2dhist'
jid3=$(sbatch --dependency=afterany:$jid2 --time $time --mem $mem --array $array --partition $partition --job-name $job_name ./submit_slurm_2dhistos.sh -r ${runnumber} -o ${output_dir} -c ${cfg_env} -s $scratch_dir -o $output_dir -y $year | sed 's/Submitted batch job //') 
echo "${jid3} (RawSvtHit 2dhistos) has been submitted"

echo "hadding 2dhisto files to perform baseline fitting"
export job_name='hadd'
jid4=$(sbatch --dependency=afterany:$jid3 --time $time --mem $mem --partition $partition --job-name $job_name ./hadd_2dhistos.sh -r ${runnumber} -c $cfg_env -s $scratch_dir -o $output_dir | sed 's/Submitted batch job //') 

export job_name='Blfits'
jid5=$(sbatch --dependency=afterany:$jid4 --time $time --mem $mem --partition $partition --job-name $job_name ./submit_slurm_blfit.sh -r ${runnumber} -c $cfg_env -o ${output_dir} -t ${thresholds_file} -s $scratch_dir -y $year -t $thresholds_file | sed 's/Submitted batch job //') 

