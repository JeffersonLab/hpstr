#!/bin/bash
#SBATCH --job-name fee-ana
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=4g
#SBATCH --time=04:00:00

# sbatch submit script for the FEE analysis (FeeAnaProcessor).
#
#  ARGUMENTS
#   1  - submission directory (where config.sh / the local hpstr configs live)
#   2  - input ROOT ntuple to process
#   3+ - additional arguments forwarded to hpstr (e.g. --sample data)
#
#  ENVIRONMENT VARIABLES (passed via sbatch --export):
#   OUTPUT_DIR       - Base output directory for this job
#   HPSTR_CONFIG     - Local hpstr config to use (anaFeeSmearing_1p92_cfg.py / _3p74_cfg.py)
#   PRESELECT_SUBDIR - Optional subdirectory (e.g. for systematics studies)
#
#  NOTE: the FEE configs key off the -t (isData) flag, NOT --sample. We accept
#  --sample for bookkeeping/parallelism with scripts/2021_ana, translate it to
#  -t, then strip it before invoking hpstr (the FEE configs don't define it).
set -o errexit
set -o nounset

# Source central configuration + environment using the passed submission directory.
SUBMIT_DIR="${1}"
source "${SUBMIT_DIR}/config.sh"
source "${SUBMIT_DIR}/setup.sh"

# ============== DEBUG: Environment diagnostics ==============
echo "===== ENVIRONMENT DEBUG INFO ====="
echo "Hostname: $(hostname)"
echo "Date: $(date)"
echo "Working directory: $(pwd)"
echo "SLURM_JOB_ID: ${SLURM_JOB_ID:-not set}"
echo "which hpstr: $(which hpstr 2>&1 || echo 'NOT FOUND')"
echo "HPSTR_BASE: ${HPSTR_BASE:-not set}"
echo "Sourced config from: ${SUBMIT_DIR}/config.sh"
echo "===== END DEBUG INFO ====="
echo ""
# ============== END DEBUG ==============

# OUTPUT_DIR and HPSTR_CONFIG come from the environment (sbatch --export) with
# config.sh defaults as a fallback.
OUTPUT_DIR="${OUTPUT_DIR:-}"
HPSTR_CONFIG="${HPSTR_CONFIG:-anaFeeSmearing_3p74_cfg.py}"

if [[ -z "${OUTPUT_DIR}" ]]; then
  echo "ERROR: OUTPUT_DIR not set. Pass via sbatch --export or set in config.sh"
  exit 1
fi

input_file="${2}"
shift 2

# Build output directory (with optional systematics subdir).
out_dir="${OUTPUT_DIR}"
if [[ -n "${PRESELECT_SUBDIR:-}" ]]; then
  out_dir="${out_dir}/${PRESELECT_SUBDIR}"
fi
mkdir -p "${out_dir}"

# Disambiguate the output filename by run number when present (hps_0NNNNN),
# so files from different run subdirectories don't collide.
filename="$(basename "${input_file}")"
if [[ "${input_file}" =~ (hps_[0-9]+) && "${filename}" != *"${BASH_REMATCH[1]}"* ]]; then
  filename="${BASH_REMATCH[1]}_${filename}"
fi
output_file="${out_dir}/${filename}"

# Determine data/MC flag from the --sample argument, then strip --sample so it
# is not forwarded to the FEE config (which does not define it).
# -t 1 = data, -t 0 = MC
is_data=0  # default to MC
hpstr_args=()
prev_arg=""
for arg in "$@"; do
  if [[ "${prev_arg}" == "--sample" ]]; then
    [[ "${arg}" == "data" ]] && is_data=1
    prev_arg=""
    continue          # drop the --sample value
  fi
  if [[ "${arg}" == "--sample" ]]; then
    prev_arg="${arg}"
    continue          # drop the --sample flag
  fi
  hpstr_args+=("${arg}")
done
echo "Data/MC flag: -t ${is_data}  (1=data, 0=MC)"

# Generate a unique smearing seed from the SLURM job ID.
smearing_seed="${SLURM_JOB_ID:-$RANDOM}"
echo "Using smearing seed: ${smearing_seed}"

echo "Input:  ${input_file}"
echo "Output: ${output_file}"
echo "Config: ${HPSTR_CONFIG}"

if ! hpstr "${HPSTR_CONFIG}" -i "${input_file}" -o "${output_file}" \
       -t "${is_data}" --seed "${smearing_seed}" "${hpstr_args[@]+"${hpstr_args[@]}"}"; then
  echo "ERROR: hpstr FEE analysis exited with non-zero status."
  rm -f "${output_file}"
fi
