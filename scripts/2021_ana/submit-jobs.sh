#!/bin/bash
# Submit individual SLURM jobs for each file in the list
#
# Usage: ./submit-jobs.sh --sample <sample_type> [file.list] [-- extra_hpstr_args...]
#   --sample - required, one of: data, sim_bkgd, ap_signal, simp_signal
#   file.list - optional, path to file list (defaults to FILE_LIST from config.sh)
#   -- extra_hpstr_args - optional, additional arguments to pass to hpstr
#
# Environment variables:
#   PRESELECT_SUBDIR - if set, output goes to OUTPUT_DIR/PRESELECT_SUBDIR

set -o errexit
set -o nounset

# Valid sample types (must match config_2021.py)
VALID_SAMPLES=("data" "sim_bkgd" "ap_signal" "simp_signal")

usage() {
  echo "Usage: $0 --sample <sample_type> [file.list] [-- extra_hpstr_args...]"
  echo "  --sample: required, one of: ${VALID_SAMPLES[*]}"
  echo "  file.list: optional, path to file list (defaults to FILE_LIST from config.sh)"
  echo "  -- extra_hpstr_args: optional, additional arguments to pass to hpstr"
  exit 1
}

# Get submission directory (where this script and config.sh live)
SUBMIT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SUBMIT_DIR}/config.sh"

# Parse arguments
file_list="${FILE_LIST}"
extra_args=()
sample=""

while [[ $# -gt 0 ]]; do
  case "$1" in
    --sample)
      if [[ $# -lt 2 ]]; then
        echo "ERROR: --sample requires a value"
        usage
      fi
      sample="$2"
      shift 2
      ;;
    --)
      shift
      extra_args=("$@")
      break
      ;;
    *)
      file_list="$1"
      shift
      ;;
  esac
done

# Validate --sample is provided
if [[ -z "${sample}" ]]; then
  echo "ERROR: --sample is required"
  usage
fi

# Validate sample is one of the allowed values
valid=false
for s in "${VALID_SAMPLES[@]}"; do
  if [[ "${sample}" == "${s}" ]]; then
    valid=true
    break
  fi
done

if [[ "${valid}" == "false" ]]; then
  echo "ERROR: Invalid sample '${sample}'. Must be one of: ${VALID_SAMPLES[*]}"
  exit 1
fi

# Ensure log directory exists
mkdir -p "${LOG_DIR}"

# Check file list exists
if [[ ! -f "${file_list}" ]]; then
  echo "ERROR: File list not found: ${file_list}"
  exit 1
fi

# Count files
num_files=$(wc -l < "${file_list}")
echo "Submitting ${num_files} individual jobs from ${file_list}"
echo "Sample type: ${sample}"

# Submit one job per file
job_count=0
while IFS= read -r input_file; do
  # Skip empty lines
  [[ -z "${input_file}" ]] && continue

  # Submit individual job
  sbatch \
    --partition="${SLURM_PARTITION}" \
    --account="${SLURM_ACCOUNT}" \
    --output="${LOG_DIR}/job_%j.log" \
    --error="${LOG_DIR}/job_%j.log" \
    "${SUBMIT_DIR}/batch-script.sh" \
    "${SUBMIT_DIR}" \
    "${input_file}" \
    --sample "${sample}" \
    "${extra_args[@]+"${extra_args[@]}"}"

  ((++job_count))
done < "${file_list}"

echo "Submitted ${job_count} jobs"
