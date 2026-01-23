#!/bin/bash
# Submit individual SLURM jobs for each file in the list(s)
#
# Usage: ./submit-jobs.sh --sample <sample_type> [file.list ...] [-- extra_hpstr_args...]
#   --sample - required, one of: data, sim_bkgd, ap_signal, simp_signal
#   file.list - optional, path(s) to file list(s) (supports wildcards, defaults to FILE_LIST from config.sh)
#   -- extra_hpstr_args - optional, additional arguments to pass to hpstr
#
# Examples:
#   ./submit-jobs.sh --sample data my_files_*.list
#   ./submit-jobs.sh --sample sim_bkgd list1.list list2.list list3.list
#
# Environment variables:
#   PRESELECT_SUBDIR - if set, output goes to OUTPUT_DIR/PRESELECT_SUBDIR

set -o errexit
set -o nounset

# Valid sample types (must match config_2021.py)
VALID_SAMPLES=("data" "sim_bkgd" "ap_signal" "simp_signal")

usage() {
  echo "Usage: $0 --sample <sample_type> [file.list ...] [-- extra_hpstr_args...]"
  echo "  --sample: required, one of: ${VALID_SAMPLES[*]}"
  echo "  file.list: optional, path(s) to file list(s) - supports wildcards (defaults to FILE_LIST from config.sh)"
  echo "  -- extra_hpstr_args: optional, additional arguments to pass to hpstr"
  exit 1
}

# Get submission directory (where this script and config.sh live)
SUBMIT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SUBMIT_DIR}/config.sh"

# Parse arguments
file_lists=()
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
      file_lists+=("$1")
      shift
      ;;
  esac
done

# Use default if no file lists provided
if [[ ${#file_lists[@]} -eq 0 ]]; then
  file_lists=("${FILE_LIST}")
fi

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

# Verify all file lists exist before submitting any jobs
for file_list in "${file_lists[@]}"; do
  if [[ ! -f "${file_list}" ]]; then
    echo "ERROR: File list not found: ${file_list}"
    exit 1
  fi
done

# Count total files across all lists
total_files=0
for file_list in "${file_lists[@]}"; do
  count=$(wc -l < "${file_list}")
  total_files=$((total_files + count))
done

echo "Submitting jobs from ${#file_lists[@]} file list(s): ${file_lists[*]}"
echo "Total files: ${total_files}"
echo "Sample type: ${sample}"

# Submit one job per file from each file list
job_count=0
for file_list in "${file_lists[@]}"; do
  echo "Processing: ${file_list}"
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
done

echo "Submitted ${job_count} jobs"
