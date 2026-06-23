#!/bin/bash
#SBATCH --job-name pre-select
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=4g
#SBATCH --time=04:00:00

# sbatch submit script
#  ARGUMENTS
#   1 - submission directory (where config.sh lives)
#   2 - input ROOT file to process
#   3+ - additional arguments to be given to hpstr for each job (e.g. --sample data)
#
#  ENVIRONMENT VARIABLES (passed via sbatch --export):
#   OUTPUT_DIR   - Base output directory for this job
#   HPSTR_CONFIG - Python config file to use (e.g., config_displaced_2021.py)
#   PRESELECT_SUBDIR - Optional subdirectory for systematics studies
#
#  USAGE
#   sbatch --export="OUTPUT_DIR=/path/to/output,HPSTR_CONFIG=config.py" \
#          --output=log/job_%j.log batch-script.sh /path/to/submit/dir /path/to/input.root
set -o errexit
set -o nounset

# Source central configuration using the passed submission directory
SUBMIT_DIR="${1}"
source "${SUBMIT_DIR}/config.sh"
source "${SUBMIT_DIR}/setup.sh"

# ============== DEBUG: Environment diagnostics ==============
echo "===== ENVIRONMENT DEBUG INFO ====="
echo "Hostname: $(hostname)"
echo "Date: $(date)"
echo "Working directory: $(pwd)"
echo "SLURM_JOB_ID: ${SLURM_JOB_ID:-not set}"
echo "SLURM_JOB_NODELIST: ${SLURM_JOB_NODELIST:-not set}"
echo ""
echo "--- PATH ---"
echo "${PATH}" | tr ':' '\n'
echo ""
echo "--- hpstr lookup ---"
echo "which hpstr: $(which hpstr 2>&1 || echo 'NOT FOUND')"
echo "type hpstr: $(type hpstr 2>&1 || echo 'NOT FOUND')"
echo ""
echo "--- Relevant environment variables ---"
echo "HPSTR_BASE: ${HPSTR_BASE:-not set}"
echo "HPSTR_HOME: ${HPSTR_HOME:-not set}"
echo "LD_LIBRARY_PATH: ${LD_LIBRARY_PATH:-not set}"
echo "PYTHONPATH: ${PYTHONPATH:-not set}"
echo "ROOTSYS: ${ROOTSYS:-not set}"
echo ""
echo "--- config.sh location ---"
echo "Sourced config from: ${SUBMIT_DIR}/config.sh"
ls -la "${SUBMIT_DIR}/config.sh" 2>&1 || echo "config.sh not found"
echo "===== END DEBUG INFO ====="
echo ""
# ============== END DEBUG ==============

# OUTPUT_DIR and HPSTR_CONFIG can come from:
# 1. Environment variables (passed via sbatch --export) - preferred
# 2. config.sh defaults - fallback for legacy usage
# The :- syntax means "use env var if set, otherwise use config.sh value"
OUTPUT_DIR="${OUTPUT_DIR:-}"
HPSTR_CONFIG="${HPSTR_CONFIG:-config_displaced_2021.py}"

if [[ -z "${OUTPUT_DIR}" ]]; then
  echo "ERROR: OUTPUT_DIR not set. Pass via sbatch --export or set in config.sh"
  exit 1
fi

input_file="${2}"

# Extract mass point from input path (e.g., ap140MeV from /path/ap140MeV/...)
# This pattern matches ap followed by digits followed by MeV
mass_point=""
if [[ "${input_file}" =~ (ap[0-9]+MeV) ]]; then
  mass_point="${BASH_REMATCH[1]}"
  echo "Detected mass point: ${mass_point}"
fi

# Also check for SIMP mass points (e.g., simp100MeV)
if [[ "${input_file}" =~ (simp[0-9]+MeV) ]]; then
  mass_point="${BASH_REMATCH[1]}"
  echo "Detected SIMP mass point: ${mass_point}"
fi

# Use OUTPUT_DIR with optional PRESELECT_SUBDIR and mass point
out_dir="${OUTPUT_DIR}"
if [[ -n "${PRESELECT_SUBDIR:-}" ]]; then
  out_dir="${out_dir}/${PRESELECT_SUBDIR}"
fi
if [[ -n "${mass_point}" ]]; then
  out_dir="${out_dir}/${mass_point}"
fi
mkdir -p "${out_dir}"

filename="$(basename "${input_file}")"
if [[ "${input_file}" =~ (merge-batch-[0-9]+) ]]; then
  filename="${filename%.root}_${BASH_REMATCH[1]}.root"
fi
output_file="${out_dir}/${filename}"

shift 2

# Determine data/MC flag from --sample argument
# -t 1 = data, -t 0 = MC
is_mc=0  # default to MC
for arg in "$@"; do
  if [[ "${prev_arg:-}" == "--sample" ]]; then
    if [[ "${arg}" == "data" ]]; then
      is_mc=1
    fi
    break
  fi
  prev_arg="${arg}"
done
echo "Data/MC flag: -t ${is_mc}"

# Generate unique smearing seed from SLURM job ID
smearing_seed="${SLURM_JOB_ID:-$RANDOM}"
echo "Using smearing seed: ${smearing_seed}"

echo "Input:  ${input_file}"
echo "Output: ${output_file}"
echo "Config: ${HPSTR_CONFIG}"

if ! hpstr "${HPSTR_CONFIG}" -i "${input_file}" -o "${output_file}" -t "${is_mc}" --seed "${smearing_seed}" ${@}; then
  echo "ERROR: hpstr preselect exited with non-zero error status."
  rm -f "${output_file}"
fi
