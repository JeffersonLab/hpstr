#!/bin/bash
# Submit individual SLURM jobs for each file in the list(s)
#
# Usage: ./submit-jobs.sh --job <job_name> [-- extra_hpstr_args...]
#        ./submit-jobs.sh --all [--dry-run]
#        ./submit-jobs.sh --sample <sample_type> [file.list ...] [-- extra_hpstr_args...]
#        ./submit-jobs.sh --list
#
# New interface (recommended):
#   --job <name>     - Use predefined job from job_params.sh
#   --all            - Submit all predefined jobs
#   --list           - List all available job definitions
#   --dry-run        - Show what would be submitted without actually submitting
#
# Legacy interface (still supported):
#   --sample <type>  - Sample type: data, sim_bkgd, ap_signal, simp_signal
#   file.list        - Path(s) to file list(s) (supports wildcards)
#
# Examples:
#   ./submit-jobs.sh --job wab                    # Submit WAB background jobs
#   ./submit-jobs.sh --job ap_signal_smeared      # Submit all A' mass points with smearing
#   ./submit-jobs.sh --job ap80                   # Submit single mass point
#   ./submit-jobs.sh --all                        # Submit all predefined jobs
#   ./submit-jobs.sh --all --dry-run              # Preview all jobs without submitting
#   ./submit-jobs.sh --list                       # Show available jobs
#   ./submit-jobs.sh --job wab --dry-run          # Preview without submitting
#
#   # Legacy mode (still works):
#   ./submit-jobs.sh --sample data my_files.list
#
# Environment variables:
#   PRESELECT_SUBDIR - if set, output goes to OUTPUT_DIR/PRESELECT_SUBDIR

set -o errexit
set -o nounset

# Valid sample types (must match config_2021.py)
VALID_SAMPLES=("data" "sim_bkgd" "ap_signal" "simp_signal")

usage() {
  echo "Usage: $0 --job <job_name> [-- extra_hpstr_args...]"
  echo "       $0 --all [--dry-run]"
  echo "       $0 --sample <sample_type> [file.list ...] [-- extra_hpstr_args...]"
  echo "       $0 --list"
  echo ""
  echo "Options:"
  echo "  --job <name>      Use predefined job from job_params.sh (recommended)"
  echo "  --all             Submit all predefined jobs"
  echo "  --list            List all available job definitions"
  echo "  --dry-run         Show what would be submitted without actually submitting"
  echo "  --sample <type>   Legacy: one of: ${VALID_SAMPLES[*]}"
  echo "  file.list         Legacy: path(s) to file list(s)"
  echo "  -- extra_args     Additional arguments to pass to hpstr"
  exit 1
}

# Get submission directory (where this script and config.sh live)
SUBMIT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SUBMIT_DIR}/config.sh"
source "${SUBMIT_DIR}/job_params.sh"

# Parse arguments
file_lists=()
extra_args=()
sample=""
job_name=""
dry_run=false
list_mode=false
all_mode=false

while [[ $# -gt 0 ]]; do
  case "$1" in
    --job)
      if [[ $# -lt 2 ]]; then
        echo "ERROR: --job requires a job name"
        usage
      fi
      job_name="$2"
      shift 2
      ;;
    --sample)
      if [[ $# -lt 2 ]]; then
        echo "ERROR: --sample requires a value"
        usage
      fi
      sample="$2"
      shift 2
      ;;
    --list)
      list_mode=true
      shift
      ;;
    --all)
      all_mode=true
      shift
      ;;
    --dry-run)
      dry_run=true
      shift
      ;;
    --)
      shift
      extra_args=("$@")
      break
      ;;
    -*)
      echo "ERROR: Unknown option: $1"
      usage
      ;;
    *)
      file_lists+=("$1")
      shift
      ;;
  esac
done

# Handle --list mode
if [[ "${list_mode}" == "true" ]]; then
  list_jobs
  exit 0
fi

# Function to submit jobs for a single job definition
# Returns the number of jobs submitted
submit_job_definition() {
  local job="$1"
  local is_dry_run="$2"
  local job_count=0

  if ! load_job_params "${job}"; then
    echo "ERROR: Failed to load job '${job}'"
    return 1
  fi

  # Expand glob patterns in FILE_LISTS
  local expanded_lists=()
  for pattern in ${FILE_LISTS}; do
    for f in ${pattern}; do
      if [[ -f "${f}" ]]; then
        expanded_lists+=("${f}")
      fi
    done
  done

  if [[ ${#expanded_lists[@]} -eq 0 ]]; then
    echo "WARNING: No file lists found for job '${job}', skipping"
    return 0
  fi

  local job_extra_args=()
  if [[ -n "${EXTRA_ARGS}" ]]; then
    # shellcheck disable=SC2206
    job_extra_args=(${EXTRA_ARGS})
  fi

  # Count total files
  local total_files=0
  for file_list in "${expanded_lists[@]}"; do
    local count
    count=$(wc -l < "${file_list}")
    total_files=$((total_files + count))
  done

  echo ""
  echo "=== Job: ${job} ==="
  echo "Output directory: ${OUTPUT_DIR}"
  echo "hpstr config: ${HPSTR_CONFIG}"
  echo "File lists (${#expanded_lists[@]}): ${expanded_lists[*]}"
  echo "Total files: ${total_files}"
  echo "Sample type: ${SAMPLE}"
  if [[ ${#job_extra_args[@]} -gt 0 ]]; then
    echo "Extra args: ${job_extra_args[*]}"
  fi

  if [[ "${is_dry_run}" == "true" ]]; then
    echo "DRY RUN - would submit ${total_files} jobs"
    echo "${total_files}"
    return 0
  fi

  # Submit jobs
  for file_list in "${expanded_lists[@]}"; do
    while IFS= read -r input_file; do
      [[ -z "${input_file}" ]] && continue

      sbatch \
        --partition="${SLURM_PARTITION}" \
        --account="${SLURM_ACCOUNT}" \
        --output="${LOG_DIR}/job_%j.log" \
        --error="${LOG_DIR}/job_%j.log" \
        --export="OUTPUT_DIR=${OUTPUT_DIR},HPSTR_CONFIG=${HPSTR_CONFIG}" \
        "${SUBMIT_DIR}/batch-script.sh" \
        "${SUBMIT_DIR}" \
        "${input_file}" \
        --sample "${SAMPLE}" \
        "${job_extra_args[@]+"${job_extra_args[@]}"}"

      ((++job_count))
    done < "${file_list}"
  done

  echo "Submitted ${job_count} jobs for ${job}"
  echo "${job_count}"
}

# Handle --all mode
if [[ "${all_mode}" == "true" ]]; then
  mkdir -p "${LOG_DIR}"

  total_submitted=0
  # Get sorted list of all job names
  mapfile -t all_jobs < <(printf '%s\n' "${!JOB_SAMPLE[@]}" | sort)

  echo "Submitting all ${#all_jobs[@]} job definitions..."

  for job in "${all_jobs[@]}"; do
    result=$(submit_job_definition "${job}" "${dry_run}")
    # Last line of result is the count
    count=$(echo "${result}" | tail -1)
    if [[ "${count}" =~ ^[0-9]+$ ]]; then
      total_submitted=$((total_submitted + count))
    fi
  done

  echo ""
  if [[ "${dry_run}" == "true" ]]; then
    echo "DRY RUN TOTAL: would submit ${total_submitted} jobs across ${#all_jobs[@]} job definitions"
  else
    echo "TOTAL: Submitted ${total_submitted} jobs across ${#all_jobs[@]} job definitions"
  fi
  exit 0
fi

# Determine mode: --job (new) vs --sample (legacy)
if [[ -n "${job_name}" ]]; then
  # New mode: load parameters from job definition
  if ! load_job_params "${job_name}"; then
    echo ""
    list_jobs
    exit 1
  fi

  # Expand glob patterns in FILE_LISTS
  expanded_lists=()
  for pattern in ${FILE_LISTS}; do
    # shellcheck disable=SC2086
    for f in ${pattern}; do
      if [[ -f "${f}" ]]; then
        expanded_lists+=("${f}")
      fi
    done
  done

  if [[ ${#expanded_lists[@]} -eq 0 ]]; then
    echo "ERROR: No file lists found matching: ${FILE_LISTS}"
    exit 1
  fi

  file_lists=("${expanded_lists[@]}")
  sample="${SAMPLE}"

  # Merge extra args from job definition with command-line extra args
  if [[ -n "${EXTRA_ARGS}" ]]; then
    # Split EXTRA_ARGS on spaces and prepend to extra_args
    # shellcheck disable=SC2206
    extra_args=(${EXTRA_ARGS} "${extra_args[@]+"${extra_args[@]}"}")
  fi

  echo "Job: ${job_name}"
  echo "Output directory: ${OUTPUT_DIR}"
  echo "hpstr config: ${HPSTR_CONFIG}"

elif [[ -n "${sample}" ]]; then
  # Legacy mode: use command-line arguments
  # Use default file list if none provided
  if [[ ${#file_lists[@]} -eq 0 ]]; then
    if [[ -z "${FILE_LIST:-}" ]]; then
      echo "ERROR: No file lists provided and FILE_LIST not set"
      usage
    fi
    file_lists=("${FILE_LIST}")
  fi

  # In legacy mode, OUTPUT_DIR comes from config.sh (which may not be set now)
  if [[ -z "${OUTPUT_DIR:-}" ]]; then
    echo "ERROR: OUTPUT_DIR not set. Use --job mode or set OUTPUT_DIR in config.sh"
    exit 1
  fi

else
  echo "ERROR: Either --job or --sample is required"
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

echo "File lists (${#file_lists[@]}): ${file_lists[*]}"
echo "Total files: ${total_files}"
echo "Sample type: ${sample}"
if [[ ${#extra_args[@]} -gt 0 ]]; then
  echo "Extra args: ${extra_args[*]}"
fi

if [[ "${dry_run}" == "true" ]]; then
  echo ""
  echo "DRY RUN - would submit ${total_files} jobs"
  exit 0
fi

# Ensure log directory exists
mkdir -p "${LOG_DIR}"

# Submit one job per file from each file list
job_count=0
for file_list in "${file_lists[@]}"; do
  echo "Processing: ${file_list}"
  while IFS= read -r input_file; do
    # Skip empty lines
    [[ -z "${input_file}" ]] && continue

    # Submit individual job, passing OUTPUT_DIR and HPSTR_CONFIG
    sbatch \
      --partition="${SLURM_PARTITION}" \
      --account="${SLURM_ACCOUNT}" \
      --output="${LOG_DIR}/job_%j.log" \
      --error="${LOG_DIR}/job_%j.log" \
      --export="OUTPUT_DIR=${OUTPUT_DIR},HPSTR_CONFIG=${HPSTR_CONFIG}" \
      "${SUBMIT_DIR}/batch-script.sh" \
      "${SUBMIT_DIR}" \
      "${input_file}" \
      --sample "${sample}" \
      "${extra_args[@]+"${extra_args[@]}"}"

    ((++job_count))
  done < "${file_list}"
done

echo "Submitted ${job_count} jobs"
