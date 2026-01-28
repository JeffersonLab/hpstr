#!/bin/bash
# Central configuration for 2021 analysis job submission
# Source this file in other scripts: source config.sh

# User configuration
USER_NAME="${USER:-$(whoami)}"

# SLURM configuration
SLURM_PARTITION="roma"
SLURM_ACCOUNT="hps"

# Year for processing
YEAR="2021"

# Input data paths
INPUT_DATA_DIR="/sdf/data/hps/physics2021/mc/hpstr/ap_pulser/pass5_v9-merged/"

#INPUT_DATA_DIR="/sdf/data/hps/physics2021/data/recon/pass5_v9/merged"
#INPUT_DATA_DIR="/sdf/data/hps/physics2021/mc/hpstr/tritrig_pulser/pass5_v9/HPS_Run2021Pass1_v9_14272_hitSmearKill-merged/"
#INPUT_DATA_DIR="/sdf/data/hps/physics2021/mc/hpstr/wab_pulser/pass5_v9/HPS_Run2021Pass1_v9_14272_hitSmearKill-merged/"

# Output configuration
#OUTPUT_DIR="/sdf/data/hps/physics2021/preselection/tritrig-no-smeared/"
#OUTPUT_DIR="/sdf/data/hps/physics2021/preselection/wab-no-smeared/"
OUTPUT_DIR="/sdf/data/hps/physics2021/preselection/Ap-no-time-no-smeared/"

# File list configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
#DATA_LISTS_DIR="${SCRIPT_DIR}/data_lists/pass5"
DATA_LISTS_DIR="${SCRIPT_DIR}/mc_lists/pass5"
FILE_LIST="${DATA_LISTS_DIR}/wab.list"

# Log directory
LOG_DIR="${SCRIPT_DIR}/log"

# hpstr configuration file
HPSTR_CONFIG="config_displaced_2021.py"
#HPSTR_CONFIG="config_prompt_2021.py"
