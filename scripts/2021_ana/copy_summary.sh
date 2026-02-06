#!/bin/bash
# Link (or copy) summary.json from MC input directories to output directories
#
# Usage: ./copy_summary.sh --job <job_name> [--copy] [--dry-run]
#        ./copy_summary.sh --all [--copy] [--dry-run]
#        ./copy_summary.sh --list
#
# By default, creates symlinks. Use --copy to make copies instead.
#
# For each file list in a job:
#   1. Reads the first entry to determine the MC input directory
#   2. Looks for summary.json in that directory
#   3. Symlinks (or copies) it to the corresponding output directory
#   4. For signal samples with mass points, mirrors the mass-point subdirectory

set -o errexit
set -o nounset

SUBMIT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SUBMIT_DIR}/config.sh"
source "${SUBMIT_DIR}/job_params.sh"

# Parse arguments
job_name=""
all_mode=false
dry_run=false
use_copy=false

while [[ $# -gt 0 ]]; do
  case "$1" in
    --job)
      job_name="$2"
      shift 2
      ;;
    --all)
      all_mode=true
      shift
      ;;
    --dry-run)
      dry_run=true
      shift
      ;;
    --copy)
      use_copy=true
      shift
      ;;
    --list)
      list_jobs
      exit 0
      ;;
    *)
      echo "ERROR: Unknown option: $1"
      echo "Usage: $0 --job <job_name> | --all [--copy] [--dry-run]"
      exit 1
      ;;
  esac
done

# Link summary.json for a single job definition
link_summary_for_job() {
  local job="$1"

  if ! load_job_params "${job}"; then
    echo "ERROR: Failed to load job '${job}'"
    return 1
  fi

  # Skip data jobs (no summary.json)
  if [[ "${SAMPLE}" == "data" ]]; then
    return 0
  fi

  # Expand file list glob patterns
  local expanded_lists=()
  for pattern in ${FILE_LISTS}; do
    for f in ${pattern}; do
      [[ -f "${f}" ]] && expanded_lists+=("${f}")
    done
  done

  if [[ ${#expanded_lists[@]} -eq 0 ]]; then
    echo "  WARNING: No file lists found for job '${job}'"
    return 0
  fi

  echo "=== Job: ${job} ==="

  for file_list in "${expanded_lists[@]}"; do
    # Read the first non-empty line to determine the source directory
    local first_file=""
    while IFS= read -r line; do
      if [[ -n "${line}" ]]; then
        first_file="${line}"
        break
      fi
    done < "${file_list}"

    if [[ -z "${first_file}" ]]; then
      echo "  WARNING: Empty file list: ${file_list}"
      continue
    fi

    local src_dir
    src_dir="$(dirname "${first_file}")"
    local src_summary="${src_dir}/summary.json"

    if [[ ! -f "${src_summary}" ]]; then
      echo "  WARNING: No summary.json in ${src_dir}"
      continue
    fi

    # Determine output directory (mirrors batch-script.sh mass-point logic)
    local out_dir="${OUTPUT_DIR}"
    if [[ "${first_file}" =~ (ap[0-9]+MeV) ]]; then
      out_dir="${out_dir}/${BASH_REMATCH[1]}"
    fi
    if [[ "${first_file}" =~ (simp[0-9]+MeV) ]]; then
      out_dir="${out_dir}/${BASH_REMATCH[1]}"
    fi

    local dest="${out_dir}/summary.json"

    if [[ "${dry_run}" == "true" ]]; then
      echo "  Would link: ${src_summary} -> ${dest}"
      continue
    fi

    mkdir -p "${out_dir}"

    if [[ -e "${dest}" || -L "${dest}" ]]; then
      echo "  Already exists: ${dest}"
      continue
    fi

    if [[ "${use_copy}" == "true" ]]; then
      cp "${src_summary}" "${dest}"
      echo "  Copied: ${dest}"
    else
      ln -s "${src_summary}" "${dest}"
      echo "  Linked: ${dest}"
    fi
  done
}

# Main
if [[ "${all_mode}" == "true" ]]; then
  mapfile -t all_jobs < <(printf '%s\n' "${!JOB_SAMPLE[@]}" | sort)
  for job in "${all_jobs[@]}"; do
    link_summary_for_job "${job}"
  done
elif [[ -n "${job_name}" ]]; then
  link_summary_for_job "${job_name}"
else
  echo "Usage: $0 --job <job_name> | --all [--copy] [--dry-run]"
  exit 1
fi
