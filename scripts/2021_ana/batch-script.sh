#!/bin/bash
#SBATCH --job-name pre-select
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=4g
#SBATCH --time=04:00:00

# sbatch submit script
#  ARGUMENTS
#   1 - submission directory (where config.sh lives)
#   2 - input ROOT file to process
#   3+ - additional arguments to be given to hpstr for each job (e.g. --sample data)
#
#  The environment variable PRESELECT_SUBDIR can be defined to direct the
#  output analysis tuples into a subdirectory of the root output directory.
#  This is mainly used when processing the systematics.
#
#  USAGE
#   sbatch --output=log/job_%j.log --error=log/job_%j.log batch-script.sh /path/to/submit/dir /path/to/input.root
set -o errexit
set -o nounset

# Source central configuration using the passed submission directory
SUBMIT_DIR="${1}"
source "${SUBMIT_DIR}/config.sh"

input_file="${2}"

# Use OUTPUT_DIR from config, with optional PRESELECT_SUBDIR
out_dir="${OUTPUT_DIR}${PRESELECT_SUBDIR:+/${PRESELECT_SUBDIR}}"
mkdir -p "${out_dir}"

# trusting the filenames from the different batches to not clash
filename="$(basename "${input_file}")"
output_file="${out_dir}/${filename}"

shift 2
if ! hpstr "${HPSTR_CONFIG}" -i "${input_file}" -o "${output_file}" ${@}; then
  echo "ERROR: hpstr preselect exited with non-zero error status."
  rm -f "${output_file}"
fi
