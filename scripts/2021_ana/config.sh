#!/bin/bash
# Central configuration for 2021 analysis job submission
# Source this file in other scripts: source config.sh
#
# This file contains SYSTEM-LEVEL settings only.
# Job-specific parameters are defined in job_params.sh

# User configuration
USER_NAME="${USER:-$(whoami)}"

# SLURM configuration
SLURM_PARTITION="roma"
SLURM_ACCOUNT="hps"

# Year for processing
YEAR="2021"

# Script directory (where this config lives)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Log directory
LOG_DIR="${SCRIPT_DIR}/log"

# Base paths for job_params.sh (can be overridden before sourcing job_params.sh)
BASE_OUTPUT_DIR="/sdf/data/hps/physics2021/preselection/v13/"
MC_LISTS_DIR="${SCRIPT_DIR}/mc_lists/pass5"
DATA_LISTS_DIR="${SCRIPT_DIR}/data_lists/pass5"

# ============================================================================
# LEGACY SUPPORT - These are kept for make_file_list.sh and manual usage
# ============================================================================

# Input data paths (uncomment one for make_file_list.sh)
#INPUT_DATA_DIR="/sdf/data/hps/physics2021/data/recon/pass5_v9/merged"
#INPUT_DATA_DIR="/sdf/data/hps/physics2021/mc/hpstr/wab_pulser/pass5_v9/HPS_Run2021Pass1_v9_14272_hitSmearKill-reprocessed-merged/"
#INPUT_DATA_DIR="/sdf/data/hps/physics2021/mc/hpstr/tritrig_pulser/pass5_v9/HPS_Run2021Pass1_v9_14272_hitSmearKill-reprocessed-merged/"
#INPUT_DATA_DIR="/sdf/data/hps/physics2021/mc/hpstr/ap_pulser/pass5_v9/ap*MeV/HPS_Run2021Pass1_v9_14272_hitSmearKill-reprocess-merged/"
#INPUT_DATA_DIR="/sdf/data/hps/physics2021/mc/hpstr/simp_pulser_new/pass5_v9/HPS_Run2021Pass1_v9_14272/simp*MeV-merged"
INPUT_DATA_DIR="/sdf/data/hps/physics2021/mc/hpstr/rad_pulser/pass5_v9/HPS_Run2021Pass1_v9_14272_hitSmearKill-reprocessed-merged/"

# Default hpstr config (used by batch-script.sh if not overridden)
HPSTR_CONFIG="${HPSTR_CONFIG:-config_displaced_2021.py}"
