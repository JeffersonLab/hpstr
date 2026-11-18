#!/bin/bash
# Central configuration for FEE (Full Energy Electron) analysis job submission.
# Source this file in other scripts: source config.sh
#
# This file contains SYSTEM-LEVEL settings only.
# Job-specific parameters are defined in job_params.sh
#
# This framework runs FeeAnaProcessor (via anaFeeSmearing_*_cfg.py) on hpstr
# ntuples for the two 2021 beam energies:
#   1.92 GeV  -> anaFeeSmearing_1p92_cfg.py
#   3.74 GeV  -> anaFeeSmearing_3p74_cfg.py
# It is intentionally completely separate from scripts/2021_ana.

# User configuration
USER_NAME="${USER:-$(whoami)}"

# SLURM configuration
SLURM_PARTITION="milano"
SLURM_ACCOUNT="hps"

# Script directory (where this config lives)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Log directory
LOG_DIR="${SCRIPT_DIR}/log"

# Base output directory for FEE analysis products (can be overridden before
# sourcing job_params.sh). Per-energy / per-sample subdirs are set in job_params.sh.
BASE_OUTPUT_DIR="/sdf/data/hps/physics2021/fee_ana/v5"

# Per-energy file-list locations (populate these with make_file_list.sh).
LISTS_1P92_DIR="${SCRIPT_DIR}/data_lists/1p92"
LISTS_3P74_DIR="${SCRIPT_DIR}/data_lists/3p74"
LISTS_4P55_DIR="${SCRIPT_DIR}/data_lists/4p55"   # 2019 data, 4.55 GeV (per-run lists)
MC_LISTS_1P92_DIR="${SCRIPT_DIR}/mc_lists/1p92"
MC_LISTS_3P74_DIR="${SCRIPT_DIR}/mc_lists/3p74"

# ============================================================================
# LEGACY SUPPORT - used by make_file_list.sh for manual single-list generation.
# Set INPUT_DATA_DIR / FILE_LIST before running make_file_list.sh.
# (Inputs are provided by the user; left blank here on purpose.)
# ============================================================================
INPUT_DATA_DIR="${INPUT_DATA_DIR:-}"
FILE_LIST="${FILE_LIST:-${SCRIPT_DIR}/data_lists/files.list}"

# Default hpstr config (used by batch-script.sh if not overridden via --export).
# Most jobs set this explicitly in job_params.sh.
HPSTR_CONFIG="${HPSTR_CONFIG:-anaFeeSmearing_3p74_cfg.py}"
