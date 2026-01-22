#!/bin/bash
# Generate a single file list from input data directory
#
# Usage: ./make_file_list.sh [max_files]
#   max_files - optional, limit the number of files in the list (for testing)

set -o errexit
set -o nounset

# Source central configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/config.sh"

# Optional max files argument (0 or unset means no limit)
max_files="${1:-0}"

# Ensure output directory exists
mkdir -p "${DATA_LISTS_DIR}"

# Clear existing file list
> "${FILE_LIST}"

shopt -s nullglob

file_count=0

# Check if run-number subdirectories exist (hps_* pattern)
run_dirs=("${INPUT_DATA_DIR}"/hps_*)

if [ ${#run_dirs[@]} -gt 0 ] && [ -d "${run_dirs[0]}" ]; then
  # Structured directory: hps_* subdirectories with run-specific files
  for dir in "${run_dirs[@]}"; do
    # Check if we've reached the limit
    if [ "${max_files}" -gt 0 ] && [ "${file_count}" -ge "${max_files}" ]; then
      break
    fi

    # Use parameter expansion instead of spawning cut
    base=${dir##*/}
    runnum=${base#hps_}

    # Collect files into array (avoids double glob evaluation)
    files=("$dir/merged_hps_${runnum}_"*.root)

    # Check if any files matched (nullglob makes array empty if no match)
    if [ ${#files[@]} -gt 0 ]; then
      for f in "${files[@]}"; do
        # Check limit before adding each file
        if [ "${max_files}" -gt 0 ] && [ "${file_count}" -ge "${max_files}" ]; then
          break
        fi
        printf '%s\n' "$f" >> "${FILE_LIST}"
        ((++file_count))
      done
    fi
  done
else
  # Flat directory: ROOT files directly in INPUT_DATA_DIR
  files=("${INPUT_DATA_DIR}"/*.root)

  for f in "${files[@]}"; do
    if [ "${max_files}" -gt 0 ] && [ "${file_count}" -ge "${max_files}" ]; then
      break
    fi
    printf '%s\n' "$f" >> "${FILE_LIST}"
    ((++file_count))
  done
fi

echo "Generated ${FILE_LIST} with ${file_count} files"

