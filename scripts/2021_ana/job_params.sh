#!/bin/bash
# Job parameter definitions for 2021 analysis workflow
# Each job type defines: file lists, sample type, output dir, and extra args
#
# Usage: source job_params.sh; load_job_params <job_name>
#
# To add a new job type, add entries to each associative array below.

# Declare associative arrays for job parameters
declare -A JOB_FILE_LISTS      # Space-separated list of input file lists
declare -A JOB_SAMPLE          # Sample type: data, sim_bkgd, ap_signal, simp_signal
declare -A JOB_OUTPUT_DIR      # Output directory
declare -A JOB_HPSTR_CONFIG    # hpstr configuration file
declare -A JOB_EXTRA_ARGS      # Extra arguments to pass to hpstr

# Base paths (can be overridden before sourcing)
: "${BASE_OUTPUT_DIR:=/sdf/data/hps/physics2021/preselection}"
: "${MC_LISTS_DIR:=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/mc_lists/pass5}"
: "${DATA_LISTS_DIR:=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/data_lists/pass5}"

# ============================================================================
# DATA JOBS
# ============================================================================

JOB_FILE_LISTS["data_small"]="${DATA_LISTS_DIR}/small.txt"
JOB_SAMPLE["data_small"]="data"
JOB_OUTPUT_DIR["data_small"]="${BASE_OUTPUT_DIR}//data_1pc_pScaleCorr/"
JOB_HPSTR_CONFIG["data_small"]="config_displaced_2021.py"
JOB_EXTRA_ARGS["data_small"]=""

JOB_FILE_LISTS["data_14185_nom"]="${DATA_LISTS_DIR}/data_14185_nom.txt"
JOB_SAMPLE["data_14185_nom"]="data"
JOB_OUTPUT_DIR["data_14185_nom"]="${BASE_OUTPUT_DIR}/data_14185_nom/"
JOB_HPSTR_CONFIG["data_14185_nom"]="config_displaced_2021.py"
JOB_EXTRA_ARGS["data_14185_nom"]=""

JOB_FILE_LISTS["data_14185_lowp"]="${DATA_LISTS_DIR}/data_14185_lowp.txt"
JOB_SAMPLE["data_14185_lowp"]="data"
JOB_OUTPUT_DIR["data_14185_lowp"]="${BASE_OUTPUT_DIR}/data_14185_lowp/"
JOB_HPSTR_CONFIG["data_14185_lowp"]="config_displaced_2021.py"
JOB_EXTRA_ARGS["data_14185_lowp"]=""

JOB_FILE_LISTS["data"]="${DATA_LISTS_DIR}/data_1pc.txt"
JOB_SAMPLE["data"]="data"
JOB_OUTPUT_DIR["data"]="${BASE_OUTPUT_DIR}/data_1pc/"
JOB_HPSTR_CONFIG["data"]="config_displaced_2021.py"
JOB_EXTRA_ARGS["data"]=""

JOB_FILE_LISTS["data_5pc"]="${DATA_LISTS_DIR}/data_5pc.txt"
JOB_SAMPLE["data_5pc"]="data"
JOB_OUTPUT_DIR["data_5pc"]="${BASE_OUTPUT_DIR}/data_5pc"
JOB_HPSTR_CONFIG["data_5pc"]="config_displaced_2021.py"
JOB_EXTRA_ARGS["data_5pc"]=""

JOB_FILE_LISTS["data_10pc"]="${DATA_LISTS_DIR}/single.txt"
JOB_SAMPLE["data_10pc"]="data"
JOB_OUTPUT_DIR["data_10pc"]="${BASE_OUTPUT_DIR}/data_10pc_psum2p8"
JOB_HPSTR_CONFIG["data_10pc"]="config_displaced_2021.py"
JOB_EXTRA_ARGS["data_10pc"]=""

JOB_FILE_LISTS["data_prompt"]="${DATA_LISTS_DIR}/data_1pc.txt"
JOB_SAMPLE["data_prompt"]="data"
JOB_OUTPUT_DIR["data_prompt"]="${BASE_OUTPUT_DIR}/data_1pc_tc"
JOB_HPSTR_CONFIG["data_prompt"]="config_prompt_2021.py"
JOB_EXTRA_ARGS["data_prompt"]=""

JOB_FILE_LISTS["data_10pc_prompt"]="${DATA_LISTS_DIR}/data_10pc.txt"
JOB_SAMPLE["data_10pc_prompt"]="data"
JOB_OUTPUT_DIR["data_10pc_prompt"]="${BASE_OUTPUT_DIR}/data_10pc_prompt_TC_psum2p8"
JOB_HPSTR_CONFIG["data_10pc_prompt"]="config_displaced_2021.py"
JOB_EXTRA_ARGS["data_10pc_prompt"]=""

# ============================================================================
# BACKGROUND MC JOBS
# ============================================================================

JOB_FILE_LISTS["wab"]="${MC_LISTS_DIR}/wab.list"
JOB_SAMPLE["wab"]="sim_bkgd"
JOB_OUTPUT_DIR["wab"]="${BASE_OUTPUT_DIR}/wab"
JOB_HPSTR_CONFIG["wab"]="config_displaced_2021.py" #/sdf/data/hps/users/mgignac/software/2021-ana/hpstr/processors/config/anaTrkEff_2021_cfg.py"
JOB_EXTRA_ARGS["wab"]=""

JOB_FILE_LISTS["wab_14596"]="${MC_LISTS_DIR}/wab_hit_kill.list"
JOB_SAMPLE["wab_14596"]="sim_bkgd"
JOB_OUTPUT_DIR["wab_14596"]="${BASE_OUTPUT_DIR}/wab_smeared_hitkill_14596"
JOB_HPSTR_CONFIG["wab_14596"]="config_displaced_2021.py" #/sdf/data/hps/users/mgignac/software/2021-ana/hpstr/processors/config/anaTrkEff_2021_cfg.py"
JOB_EXTRA_ARGS["wab_14596"]="--smearing"

JOB_FILE_LISTS["wab_smeared"]="${MC_LISTS_DIR}/wab.list"
JOB_SAMPLE["wab_smeared"]="sim_bkgd"
JOB_OUTPUT_DIR["wab_smeared"]="${BASE_OUTPUT_DIR}/wab_smeared"
JOB_HPSTR_CONFIG["wab_smeared"]="config_displaced_2021.py"
JOB_EXTRA_ARGS["wab_smeared"]="--smearing"

JOB_FILE_LISTS["tritrig"]="${MC_LISTS_DIR}/tritrig.list"
JOB_SAMPLE["tritrig"]="sim_bkgd"
JOB_OUTPUT_DIR["tritrig"]="${BASE_OUTPUT_DIR}/tritrig"
JOB_HPSTR_CONFIG["tritrig"]="config_displaced_2021.py" #/sdf/data/hps/users/mgignac/software/2021-ana/hpstr/processors/config/anaTrkEff_2021_cfg.py" 
JOB_EXTRA_ARGS["tritrig"]=""

JOB_FILE_LISTS["tritrig_smeared"]="${MC_LISTS_DIR}/tritrig.list"
JOB_SAMPLE["tritrig_smeared"]="sim_bkgd"
JOB_OUTPUT_DIR["tritrig_smeared"]="${BASE_OUTPUT_DIR}/tritrig_smeared"
JOB_HPSTR_CONFIG["tritrig_smeared"]="config_displaced_2021.py"
JOB_EXTRA_ARGS["tritrig_smeared"]="--smearing"

JOB_FILE_LISTS["tritrig_14596"]="${MC_LISTS_DIR}/tritrig_hit_kill.list" #tritrig_14596.list"
JOB_SAMPLE["tritrig_14596"]="sim_bkgd"
JOB_OUTPUT_DIR["tritrig_14596"]="${BASE_OUTPUT_DIR}/tritrig_smeared_hitkill_14596"
JOB_HPSTR_CONFIG["tritrig_14596"]="config_displaced_2021.py" 
JOB_EXTRA_ARGS["tritrig_14596"]="--smearing"

JOB_FILE_LISTS["tritrig_noHit"]="${MC_LISTS_DIR}/tritrig_14272_noHit.list"
JOB_SAMPLE["tritrig_noHit"]="sim_bkgd"
JOB_OUTPUT_DIR["tritrig_noHit"]="${BASE_OUTPUT_DIR}/tritrig_noHitKillingSmearing"
JOB_HPSTR_CONFIG["tritrig_noHit"]="config_displaced_2021.py"
JOB_EXTRA_ARGS["tritrig_noHit"]=""

JOB_FILE_LISTS["rad"]="${MC_LISTS_DIR}/rad.list"
JOB_SAMPLE["rad"]="sim_bkgd"
JOB_OUTPUT_DIR["rad"]="${BASE_OUTPUT_DIR}/rad"
JOB_HPSTR_CONFIG["rad"]="config_displaced_2021.py"
JOB_EXTRA_ARGS["rad"]=""

JOB_FILE_LISTS["rad_smeared"]="${MC_LISTS_DIR}/rad.list"
JOB_SAMPLE["rad_smeared"]="sim_bkgd"
JOB_OUTPUT_DIR["rad_smeared"]="${BASE_OUTPUT_DIR}/rad_smeared/"
JOB_HPSTR_CONFIG["rad_smeared"]="config_displaced_2021.py"
JOB_EXTRA_ARGS["rad_smeared"]="--smearing"

# ============================================================================
# A' SIGNAL MC JOBS
# ============================================================================

JOB_FILE_LISTS["ap_signal_prompt_smeared"]="${MC_LISTS_DIR}/ap_prompt.list"
JOB_SAMPLE["ap_signal_prompt_smeared"]="ap_signal"
JOB_OUTPUT_DIR["ap_signal_prompt_smeared"]="${BASE_OUTPUT_DIR}/ap_signal_prompt_smeared"
JOB_HPSTR_CONFIG["ap_signal_prompt_smeared"]="config_prompt_2021.py" #config_displaced_2021.py"
JOB_EXTRA_ARGS["ap_signal_prompt_smeared"]="--smearing"


# All A' mass points combined
JOB_FILE_LISTS["ap_signal"]="${MC_LISTS_DIR}/ap*.list"
JOB_SAMPLE["ap_signal"]="ap_signal"
JOB_OUTPUT_DIR["ap_signal"]="${BASE_OUTPUT_DIR}/ap_signal"
JOB_HPSTR_CONFIG["ap_signal"]="config_displaced_2021.py"
JOB_EXTRA_ARGS["ap_signal"]=""

JOB_FILE_LISTS["ap_signal_smeared"]="${MC_LISTS_DIR}/ap*.list"
JOB_SAMPLE["ap_signal_smeared"]="ap_signal"
JOB_OUTPUT_DIR["ap_signal_smeared"]="${BASE_OUTPUT_DIR}/ap_signal_smeared"
JOB_HPSTR_CONFIG["ap_signal_smeared"]="config_displaced_2021.py"
JOB_EXTRA_ARGS["ap_signal_smeared"]="--smearing"

JOB_FILE_LISTS["ap_signal_improved"]="${MC_LISTS_DIR}/improved_180MeV.list"
JOB_SAMPLE["ap_signal_improved"]="ap_signal"
JOB_OUTPUT_DIR["ap_signal_improved"]="${BASE_OUTPUT_DIR}/ap_signal_smeared_improvedTrking/ap180MeV/"
JOB_HPSTR_CONFIG["ap_signal_improved"]="config_displaced_2021.py"
JOB_EXTRA_ARGS["ap_signal_improved"]="--smearing"


JOB_FILE_LISTS["ap_tgt_sys"]="${MC_LISTS_DIR}/sys_ap80MeV_tgt_m0pt1mm.list"
JOB_SAMPLE["ap_tgt_sys"]="ap_signal"
JOB_OUTPUT_DIR["ap_tgt_sys"]="${BASE_OUTPUT_DIR}/sys/ap_tgt_m0pt1mm/"
JOB_HPSTR_CONFIG["ap_tgt_sys"]="config_displaced_2021.py"
JOB_EXTRA_ARGS["ap_tgt_sys"]="--smearing"

# All A' mass points combined
JOB_FILE_LISTS["ap_signal_tc"]="${MC_LISTS_DIR}/ap*.list"
JOB_SAMPLE["ap_signal_tc"]="ap_signal"
JOB_OUTPUT_DIR["ap_signal_tc"]="${BASE_OUTPUT_DIR}/ap_signal_tc"
JOB_HPSTR_CONFIG["ap_signal_tc"]="config_prompt_2021.py"
JOB_EXTRA_ARGS["ap_signal_tc"]=""

JOB_FILE_LISTS["ap_signal_tc_smeared"]="${MC_LISTS_DIR}/ap*.list"
JOB_SAMPLE["ap_signal_tc_smeared"]="ap_signal"
JOB_OUTPUT_DIR["ap_signal_tc_smeared"]="${BASE_OUTPUT_DIR}/ap_signal_tc_smeared"
JOB_HPSTR_CONFIG["ap_signal_tc_smeared"]="config_prompt_2021.py"
JOB_EXTRA_ARGS["ap_signal_tc_smeared"]="--smearing"

# ============================================================================
# SIMP SIGNAL MC JOBS
# ============================================================================

JOB_FILE_LISTS["simp_signal"]="${MC_LISTS_DIR}/simp*.list"
JOB_SAMPLE["simp_signal"]="simp_signal"
JOB_OUTPUT_DIR["simp_signal"]="${BASE_OUTPUT_DIR}/simp_signal"
JOB_HPSTR_CONFIG["simp_signal"]="config_displaced_2021.py"
JOB_EXTRA_ARGS["simp_signal"]=""

JOB_FILE_LISTS["simp_signal_smeared"]="${MC_LISTS_DIR}/simp*.list"
JOB_SAMPLE["simp_signal_smeared"]="simp_signal"
JOB_OUTPUT_DIR["simp_signal_smeared"]="${BASE_OUTPUT_DIR}/simp_signal_smeared/"
JOB_HPSTR_CONFIG["simp_signal_smeared"]="config_displaced_2021.py"
JOB_EXTRA_ARGS["simp_signal_smeared"]="--smearing"

JOB_FILE_LISTS["simp90_0p18"]="${MC_LISTS_DIR}/sys_simp90_0p18.txt"
JOB_SAMPLE["simp90_0p18"]="simp_signal"
JOB_OUTPUT_DIR["simp90_0p18"]="${BASE_OUTPUT_DIR}/sys/simp90_n0p18/"
JOB_HPSTR_CONFIG["simp90_0p18"]="config_displaced_2021.py"
JOB_EXTRA_ARGS["simp90_0p18"]="--smearing"

JOB_FILE_LISTS["simp90_1p8"]="${MC_LISTS_DIR}/sys_simp90_n1p8.txt"
JOB_SAMPLE["simp90_1p8"]="simp_signal"
JOB_OUTPUT_DIR["simp90_1p8"]="${BASE_OUTPUT_DIR}/sys/simp90_n1p8/"
JOB_HPSTR_CONFIG["simp90_1p8"]="config_displaced_2021.py"
JOB_EXTRA_ARGS["simp90_1p8"]="--smearing"

# ============================================================================
# HELPER FUNCTIONS
# ============================================================================

# List all available job names
list_jobs() {
  echo "Available jobs:"
  for job in "${!JOB_SAMPLE[@]}"; do
    printf "  %-20s  sample=%-12s  output=%s\n" \
      "${job}" "${JOB_SAMPLE[$job]}" "${JOB_OUTPUT_DIR[$job]}"
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

  # Export parameters
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
