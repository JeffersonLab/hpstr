12/9/21
Alic Spellman

This directory houses all of the scripts necessary to convert multiple evio data files into a single offline baseline fit file.
***********Requires installation of hps-mc software!***************


0. Edit 'setup.sh' and update path to HPSTR and HPSMC based on your installation

0a. Get a local copy of the DAQ thresholds file used for the run you're fitting.
    The fit window is defined by the apv channel threshold settings at the time the data is taken.

0b. Get a local copy of the ONLINE baselines database file corresponding to the run you're fitting. 
    Online pedestals are used in cases where offline fits fail.
    ONLINE NOISES should be used instead of offline noises, as they seem to perform better. 
        *This is by default inside 'hpstr/plotUtils/makeOfflineBaselineDatabaseFile.py'

1. Store all input run_data. evio file paths into a txt file. INCLUDE AT LEAST 10 FILES (stats requiement)!

2. Edit 'mkjobs.sh' 
    a. '-i <tag> <input_file_paths.txt>'  
    b. 'job.json.tmpl' specifies which template to use
    c. jobs_<name>.json specifies the output jobs file
    d. Edit 'job.json.tmpl' file to configure the template that will be used to create all jobs
        da. template requires unique (varaible) input and output names...uses Jinja2 to parse string variables
        db. Example: '{{input_files['hps'][0]}}' -> input_files are read from <input_file_paths.txt>, 
        and ['hps'] is the <tag> from 2a. 

3. Run './mkjobs.sh' to create jobs.json file
    a. Open the file and check that your jobs make sense

4. Run 'source run_job.sh'
    a. Provide flags defined at top: {run_number, jobdirectory, scratch run_directory, 
        first_id = first number of evio file}
    b. --thresh specifies the full path to the apv channel thresholds file used at the time of this run
    c. output of these jobs will be a collection of baseline_fit root files Layers 1 - 7
    d. Need to hadd and run analysis script on those files

5. 'hadd hps_<run>_offline_baselines.root hps_<run>_offline_baselines_L<n>.root'

**To generate database baseline output file, you must have the online_baseline.dat file taken closest to the 
    current run (svt_<run>_cond.dat).
    These online baseline files are avaialable on ifarm.

6. Run python analysis script to generate baseline database files and threshold files
    a. example:
    python3 hpstr/plotUtils/makeOfflineBaselineDatabaseFile.py -i output/hps_14710_offline_baseline_fits.root -o output/hps_14710_offline_baseline_fits_ana.root -b /data/hps/slac_svt/server/thresholds/run/svt_014622_cond.dat -dbo output/hps_14710_offline_baselines.dat -r 14710 -y 2019


