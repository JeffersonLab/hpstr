#!/bin/bash
# Generate file lists from input data directory
#
# Usage: ./make_file_list.sh [--mass <mass_point>] [--all-masses] [max_files]
#   --mass <mass_point> - generate list for specific mass point (e.g., ap140MeV)
#   --all-masses - generate separate lists for each mass point found
#   max_files - optional, limit the number of files in the list (for testing)
#
# When processing a directory with mass point subdirs (ap*MeV), use --mass or --all-masses.
# Without these flags, behaves as before (single flat list).

set -o errexit
set -o nounset

# Source central configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/config.sh"

# Parse arguments
mass_point=""
all_masses=false
max_files=0

while [[ $# -gt 0 ]]; do
  case "$1" in
    --mass)
      mass_point="$2"
      shift 2
      ;;
    --all-masses)
      all_masses=true
      shift
      ;;
    *)
      max_files="$1"
      shift
      ;;
  esac
done

# Ensure output directory exists
mkdir -p "${DATA_LISTS_DIR}"

shopt -s nullglob

# Function to generate file list for a given directory
generate_list_for_dir() {
  local input_dir="$1"
  local output_list="$2"
  local limit="$3"
  local count=0

  > "${output_list}"

  # Check if run-number subdirectories exist (hps_* or HPS_* pattern)
  local run_dirs=("${input_dir}"/hps_* "${input_dir}"/HPS_*)

  # Filter to only existing directories
  local valid_run_dirs=()
  for d in "${run_dirs[@]}"; do
    [[ -d "$d" ]] && valid_run_dirs+=("$d")
  done

  if [ ${#valid_run_dirs[@]} -gt 0 ]; then
    # Structured directory with subdirectories
    for dir in "${valid_run_dirs[@]}"; do
      if [ "${limit}" -gt 0 ] && [ "${count}" -ge "${limit}" ]; then
        break
      fi

      # Find all ROOT files in this subdirectory
      local files=("${dir}"/*.root)

      for f in "${files[@]}"; do
        if [ "${limit}" -gt 0 ] && [ "${count}" -ge "${limit}" ]; then
          break
        fi
        printf '%s\n' "$f" >> "${output_list}"
        ((++count))
      done
    done
  else
    # Flat directory: ROOT files directly in input_dir
    local files=("${input_dir}"/*.root)

    for f in "${files[@]}"; do
      if [ "${limit}" -gt 0 ] && [ "${count}" -ge "${limit}" ]; then
        break
      fi
      printf '%s\n' "$f" >> "${output_list}"
      ((++count))
    done
  fi

  echo "Generated ${output_list} with ${count} files"
}

# Check for mass point directories (ap*MeV pattern)
mass_dirs=("${INPUT_DATA_DIR}"/ap*MeV)

if [[ "${all_masses}" == "true" ]]; then
  # Generate separate list for each mass point
  if [ ${#mass_dirs[@]} -eq 0 ] || [ ! -d "${mass_dirs[0]}" ]; then
    echo "ERROR: No mass point directories (ap*MeV) found in ${INPUT_DATA_DIR}"
    exit 1
  fi

  for mass_dir in "${mass_dirs[@]}"; do
    [[ -d "${mass_dir}" ]] || continue
    mp=$(basename "${mass_dir}")
    output_list="${DATA_LISTS_DIR}/${mp}.list"
    generate_list_for_dir "${mass_dir}" "${output_list}" "${max_files}"
  done

elif [[ -n "${mass_point}" ]]; then
  # Generate list for specific mass point
  mass_dir="${INPUT_DATA_DIR}/${mass_point}"
  if [[ ! -d "${mass_dir}" ]]; then
    echo "ERROR: Mass point directory not found: ${mass_dir}"
    exit 1
  fi
  output_list="${DATA_LISTS_DIR}/${mass_point}.list"
  generate_list_for_dir "${mass_dir}" "${output_list}" "${max_files}"

else
  # Default behavior: single flat list (original behavior)
  generate_list_for_dir "${INPUT_DATA_DIR}" "${FILE_LIST}" "${max_files}"
fi

