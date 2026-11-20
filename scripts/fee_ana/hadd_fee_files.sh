#!/bin/bash
# Merge FEE analysis output ROOT files for a given job into a single file.
#
# Usage: ./hadd_fee_files.sh --job <job_name> [output.root]
#        ./hadd_fee_files.sh <output_dir> [output.root]
#
# With --job, the output directory is looked up from job_params.sh.
# The merged file defaults to <output_dir>/fee_merged.root.

set -o errexit
set -o nounset

SUBMIT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SUBMIT_DIR}/config.sh"
source "${SUBMIT_DIR}/job_params.sh"

out_dir=""
merged=""

if [[ "${1:-}" == "--job" ]]; then
  if ! load_job_params "${2:-}"; then
    list_jobs
    exit 1
  fi
  out_dir="${OUTPUT_DIR}"
  merged="${3:-${out_dir}/fee_merged.root}"
else
  out_dir="${1:-}"
  merged="${2:-${out_dir%/}/fee_merged.root}"
fi

if [[ -z "${out_dir}" || ! -d "${out_dir}" ]]; then
  echo "ERROR: output directory not found: ${out_dir}"
  echo "Usage: $0 --job <job_name> [output.root]  |  $0 <output_dir> [output.root]"
  exit 1
fi

echo "Merging ${out_dir}/*.root -> ${merged}"
hadd -f "${merged}" "${out_dir}"/*.root
echo "Done: ${merged}"
