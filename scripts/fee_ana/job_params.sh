#!/bin/bash
# Job parameter definitions for the FEE analysis workflow.
# Each job type defines: file lists, sample type, output dir, hpstr config, and extra args.
#
# Usage: source job_params.sh; load_job_params <job_name>
#
# Jobs are organized by beam energy:
#   *_1p92  -> 1.92 GeV  (anaFeeSmearing_1p92_cfg.py)
#   *_3p74  -> 3.74 GeV  (anaFeeSmearing_3p74_cfg.py)
#
# To add a new job type, add one entry to each associative array below.

# Declare associative arrays for job parameters
declare -A JOB_FILE_LISTS      # Space-separated list of input file lists
declare -A JOB_SAMPLE          # Sample type: data, sim_bkgd
declare -A JOB_OUTPUT_DIR      # Output directory
declare -A JOB_HPSTR_CONFIG    # hpstr configuration file (local to this dir)
declare -A JOB_EXTRA_ARGS      # Extra arguments to pass to hpstr

# Base paths (can be overridden before sourcing)
: "${BASE_OUTPUT_DIR:=/sdf/data/hps/physics2021/fee_ana/v1}"
_THIS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
: "${LISTS_1P92_DIR:=${_THIS_DIR}/data_lists/1p92}"
: "${LISTS_3P74_DIR:=${_THIS_DIR}/data_lists/3p74}"
: "${LISTS_4P55_DIR:=${_THIS_DIR}/data_lists/4p55}"
: "${MC_LISTS_1P92_DIR:=${_THIS_DIR}/mc_lists/1p92}"
: "${MC_LISTS_3P74_DIR:=${_THIS_DIR}/mc_lists/3p74}"

# ============================================================================
# DATA JOBS
# ============================================================================

# 3.74 GeV data
JOB_FILE_LISTS["data_3p74"]="${LISTS_3P74_DIR}/feeskims_v10dev.txt"
JOB_SAMPLE["data_3p74"]="data"
JOB_OUTPUT_DIR["data_3p74"]="${BASE_OUTPUT_DIR}/3p74/data_v10dev"
JOB_HPSTR_CONFIG["data_3p74"]="anaFeeSmearing_3p74_cfg.py"
JOB_EXTRA_ARGS["data_3p74"]=""

# 1.92 GeV data
# pass6_v9 FEE skims (3.74 GeV). Inputs are .slcio: batch-script.sh converts each
# with recoTuple_noHitColl_cfg.py, keeps the ntuple next to the .slcio, then runs the FEE
# analysis on it. Use --files-per-job to group files, e.g.
#   ./submit-jobs.sh --job data_3p74_pass6 --files-per-job 20
JOB_FILE_LISTS["data_3p74_pass6"]="${LISTS_3P74_DIR}/feeskims_pass6_v9.list"
JOB_SAMPLE["data_3p74_pass6"]="data"
JOB_OUTPUT_DIR["data_3p74_pass6"]="${BASE_OUTPUT_DIR}/3p74/data_pass6_v9"
JOB_HPSTR_CONFIG["data_3p74_pass6"]="anaFeeSmearing_3p74_cfg.py"
JOB_EXTRA_ARGS["data_3p74_pass6"]=""

# Same inputs, analysed with the run-by-run momentum scale in track_smeared (closure test).
# The ntuples from data_3p74_pass6 are reused.
JOB_FILE_LISTS["data_3p74_pass6_runscale"]="${LISTS_3P74_DIR}/feeskims_pass6_v9.list"
JOB_SAMPLE["data_3p74_pass6_runscale"]="data"
JOB_OUTPUT_DIR["data_3p74_pass6_runscale"]="${BASE_OUTPUT_DIR}/3p74/data_pass6_v9_runscale"
JOB_HPSTR_CONFIG["data_3p74_pass6_runscale"]="anaFeeSmearing_3p74_cfg.py"
JOB_EXTRA_ARGS["data_3p74_pass6_runscale"]=""

# Single run (14185) of the above, for testing
JOB_FILE_LISTS["data_3p74_pass6_14185"]="${LISTS_3P74_DIR}/feeskims_pass6_v9_14185.list"
JOB_SAMPLE["data_3p74_pass6_14185"]="data"
JOB_OUTPUT_DIR["data_3p74_pass6_14185"]="${BASE_OUTPUT_DIR}/3p74/data_pass6_v9_14185"
JOB_HPSTR_CONFIG["data_3p74_pass6_14185"]="anaFeeSmearing_3p74_cfg.py"
JOB_EXTRA_ARGS["data_3p74_pass6_14185"]=""

JOB_FILE_LISTS["data_1p92"]="${LISTS_1P92_DIR}/data.list"
JOB_SAMPLE["data_1p92"]="data"
JOB_OUTPUT_DIR["data_1p92"]="${BASE_OUTPUT_DIR}/1p92/data"
JOB_HPSTR_CONFIG["data_1p92"]="anaFeeSmearing_1p92_cfg.py"
JOB_EXTRA_ARGS["data_1p92"]=""

# 4.55 GeV data (2019, pass2 unskimmed). One job per run so outputs are separable by
# run number (each run -> its own list and output subdir). The output filenames also
# retain the hps_0NNNNN run tag, so per-run differences are easy to track downstream.
FEE_4P55_RUNS=(10029 10068 10070 10131 10190 10375 10417)
for _run in "${FEE_4P55_RUNS[@]}"; do
  _job="data_4p55_${_run}"
  JOB_FILE_LISTS["${_job}"]="${LISTS_4P55_DIR}/run_${_run}.list"
  JOB_SAMPLE["${_job}"]="data"
  JOB_OUTPUT_DIR["${_job}"]="${BASE_OUTPUT_DIR}/4p55/data/run_${_run}"
  JOB_HPSTR_CONFIG["${_job}"]="anaFeeSmearing_4p55_cfg.py"
  JOB_EXTRA_ARGS["${_job}"]=""
done
unset _run _job

# ============================================================================
# BACKGROUND MC JOBS (placeholders -- MC inputs provided later)
# Populate the *.list files under mc_lists/<energy>/ and uncomment.
# ============================================================================

# 3.74 GeV background MC
# 3.74 GeV beam-background MC
JOB_FILE_LISTS["beam_3p74"]="${MC_LISTS_3P74_DIR}/beam.list"
JOB_SAMPLE["beam_3p74"]="sim_bkgd"
JOB_OUTPUT_DIR["beam_3p74"]="${BASE_OUTPUT_DIR}/3p74/beam"
JOB_HPSTR_CONFIG["beam_3p74"]="anaFeeSmearing_3p74_cfg.py"
JOB_EXTRA_ARGS["beam_3p74"]=""

JOB_FILE_LISTS["beam_fix_3p74"]="${MC_LISTS_3P74_DIR}/beam_fix.list"
JOB_SAMPLE["beam_fix_3p74"]="sim_bkgd"
JOB_OUTPUT_DIR["beam_fix_3p74"]="${BASE_OUTPUT_DIR}/3p74/beam_fix"
JOB_HPSTR_CONFIG["beam_fix_3p74"]="anaFeeSmearing_3p74_cfg.py"
JOB_EXTRA_ARGS["beam_fix_3p74"]=""

# 3.74 GeV tritrig MC (pass5_v9, 14272 hitSmearKill)
JOB_FILE_LISTS["tritrig_3p74"]="${MC_LISTS_3P74_DIR}/tritrig.list"
JOB_SAMPLE["tritrig_3p74"]="sim_bkgd"
JOB_OUTPUT_DIR["tritrig_3p74"]="${BASE_OUTPUT_DIR}/3p74/tritrig"
JOB_HPSTR_CONFIG["tritrig_3p74"]="anaFeeSmearing_3p74_cfg.py"
JOB_EXTRA_ARGS["tritrig_3p74"]=""

# 3.74 GeV WAB MC (pass5_v9, 14272 hitSmearKill)
JOB_FILE_LISTS["wab_3p74"]="${MC_LISTS_3P74_DIR}/wab.list"
JOB_SAMPLE["wab_3p74"]="sim_bkgd"
JOB_OUTPUT_DIR["wab_3p74"]="${BASE_OUTPUT_DIR}/3p74/wab"
JOB_HPSTR_CONFIG["wab_3p74"]="anaFeeSmearing_3p74_cfg.py"
JOB_EXTRA_ARGS["wab_3p74"]=""

# 1.92 GeV background MC (add lists to mc_lists/1p92/ then uncomment)
JOB_FILE_LISTS["beam_1p92"]="${MC_LISTS_1P92_DIR}/beam.list"
JOB_SAMPLE["beam_1p92"]="sim_bkgd"
JOB_OUTPUT_DIR["beam_1p92"]="${BASE_OUTPUT_DIR}/1p92/beam"
JOB_HPSTR_CONFIG["beam_1p92"]="anaFeeSmearing_1p92_cfg.py"
JOB_EXTRA_ARGS["beam_1p92"]=""

# ============================================================================
# HELPER FUNCTIONS
# ============================================================================

# List all available job names
list_jobs() {
  echo "Available jobs:"
  for job in "${!JOB_SAMPLE[@]}"; do
    printf "  %-16s  sample=%-10s  config=%-26s  output=%s\n" \
      "${job}" "${JOB_SAMPLE[$job]}" "${JOB_HPSTR_CONFIG[$job]}" "${JOB_OUTPUT_DIR[$job]}"
  done | sort
}

# Load job parameters into current shell variables
# Usage: load_job_params <job_name>
# Sets: FILE_LISTS, SAMPLE, OUTPUT_DIR, HPSTR_CONFIG, EXTRA_ARGS
load_job_params() {
  local job_name="${1:-}"

  if [[ -z "${job_name}" ]]; then
    echo "ERROR: Job name required" >&2
    return 1
  fi

  if [[ -z "${JOB_OUTPUT_DIR[$job_name]:-}" ]]; then
    echo "ERROR: Unknown job '${job_name}'" >&2
    echo "Run with --list to see available jobs" >&2
    return 1
  fi

  FILE_LISTS="${JOB_FILE_LISTS[$job_name]}"
  SAMPLE="${JOB_SAMPLE[$job_name]}"
  OUTPUT_DIR="${JOB_OUTPUT_DIR[$job_name]}"
  HPSTR_CONFIG="${JOB_HPSTR_CONFIG[$job_name]}"
  EXTRA_ARGS="${JOB_EXTRA_ARGS[$job_name]}"

  return 0
}

# Validate that a job exists
job_exists() {
  [[ -n "${JOB_OUTPUT_DIR[${1:-}]:-}" ]]
}
