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

for dir in "${INPUT_DATA_DIR}"/hps_*; do
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

echo "Generated ${FILE_LIST} with ${file_count} files"

