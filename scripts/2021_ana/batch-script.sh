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

# Extract mass point from input path (e.g., ap140MeV from /path/ap140MeV/...)
# This pattern matches ap followed by digits followed by MeV
mass_point=""
if [[ "${input_file}" =~ (ap[0-9]+MeV) ]]; then
  mass_point="${BASH_REMATCH[1]}"
  echo "Detected mass point: ${mass_point}"
fi

# Use OUTPUT_DIR from config, with optional PRESELECT_SUBDIR and mass point
out_dir="${OUTPUT_DIR}"
if [[ -n "${PRESELECT_SUBDIR:-}" ]]; then
  out_dir="${out_dir}/${PRESELECT_SUBDIR}"
fi
if [[ -n "${mass_point}" ]]; then
  out_dir="${out_dir}/${mass_point}"
fi
mkdir -p "${out_dir}"

# trusting the filenames from the different batches to not clash
filename="$(basename "${input_file}")"
output_file="${out_dir}/${filename}"

shift 2
if ! hpstr "${HPSTR_CONFIG}" -i "${input_file}" -o "${output_file}" ${@}; then
  echo "ERROR: hpstr preselect exited with non-zero error status."
  rm -f "${output_file}"
fi
