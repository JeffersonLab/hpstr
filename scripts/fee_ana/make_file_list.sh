#!/bin/bash
# Generate a file list of input ntuples for the FEE analysis.
#
# Usage:
#   ./make_file_list.sh --input <dir> --output <list.txt> [--ext root] [max_files]
#
#   --input  <dir>   Directory to search for input files (searched recursively,
#                    so run-number subdirectories like hps_0NNNNN are picked up).
#   --output <list>  Output file-list path (e.g. data_lists/3p74/data.list).
#   --ext    <ext>   File extension to match (default: root).
#   max_files        Optional positional limit on the number of files (for testing).
#
# Examples:
#   ./make_file_list.sh --input /path/to/3p74/ntuples --output data_lists/3p74/data.list
#   ./make_file_list.sh --input /path/to/1p92/ntuples --output data_lists/1p92/data.list 50
#
# NOTE: inputs are provided/staged by the user. This just enumerates them.

set -o errexit
set -o nounset

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

input_dir=""
output_list=""
ext="root"
max_files=0

while [[ $# -gt 0 ]]; do
  case "$1" in
    --input)  input_dir="$2";   shift 2 ;;
    --output) output_list="$2";  shift 2 ;;
    --ext)    ext="$2";          shift 2 ;;
    -*)
      echo "ERROR: Unknown option: $1"
      exit 1
      ;;
    *)        max_files="$1";    shift ;;
  esac
done

if [[ -z "${input_dir}" || -z "${output_list}" ]]; then
  echo "ERROR: --input <dir> and --output <list> are required"
  echo "Usage: $0 --input <dir> --output <list.txt> [--ext root] [max_files]"
  exit 1
fi

if [[ ! -d "${input_dir}" ]]; then
  echo "ERROR: input directory not found: ${input_dir}"
  exit 1
fi

# Resolve output path relative to the script dir when not absolute.
if [[ "${output_list}" != /* ]]; then
  output_list="${SCRIPT_DIR}/${output_list}"
fi
mkdir -p "$(dirname "${output_list}")"

# Enumerate matching files (recursive), sorted, with optional limit.
if [[ "${max_files}" -gt 0 ]]; then
  find "${input_dir}" -type f -name "*.${ext}" | sort | head -n "${max_files}" > "${output_list}"
else
  find "${input_dir}" -type f -name "*.${ext}" | sort > "${output_list}"
fi

count=$(wc -l < "${output_list}")
echo "Generated ${output_list} with ${count} files (ext=${ext}, from ${input_dir})"
