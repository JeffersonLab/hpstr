#!/bin/bash
#SBATCH --job-name fee-ana
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=4g
#SBATCH --time=04:00:00

# sbatch submit script for the FEE analysis (FeeAnaProcessor).
#
#  ARGUMENTS
#   1  - submission directory (where config.sh / the local hpstr configs live)
#   2  - input file(s) to process, comma-separated. Each is either a ROOT ntuple
#        or an LCIO (.slcio) file; see "LCIO inputs" below.
#   3+ - additional arguments forwarded to the FEE hpstr config (e.g. --sample data)
#
#  ENVIRONMENT VARIABLES (passed via sbatch --export):
#   OUTPUT_DIR       - Base output directory for this job
#   HPSTR_CONFIG     - Local hpstr config to use (anaFeeSmearing_1p92_cfg.py / _3p74_cfg.py)
#   PRESELECT_SUBDIR - Optional subdirectory (e.g. for systematics studies)
#   NTUPLE_DIR       - Optional: where converted ntuples go (see below)
#   RECO_CONFIG      - Optional: recoTuple config for .slcio inputs (see below)
#
#  LCIO inputs: an .slcio input is first converted to a ROOT ntuple with
#  RECO_CONFIG (default processors/config/recoTuple_noHitColl_cfg.py), then analysed. The ntuple is kept so it
#  never has to be reproduced: by default it is written next to the .slcio with
#  the extension swapped; if NTUPLE_DIR is set it goes to NTUPLE_DIR/<hps_0NNNNN>/.
#  An existing non-empty ntuple is reused rather than regenerated. Conversion
#  writes to a temporary name and renames on success, so a job killed mid-way
#  (e.g. preempted) never leaves a partial ntuple behind.
#
#  NOTE: the FEE configs key off the -t (isData) flag, NOT --sample. We accept
#  --sample for bookkeeping/parallelism with scripts/2021_ana, translate it to
#  -t, then strip it before invoking hpstr (the FEE configs don't define it).
set -o errexit
set -o nounset

# Source central configuration + environment using the passed submission directory.
SUBMIT_DIR="${1}"
source "${SUBMIT_DIR}/config.sh"
source "${SUBMIT_DIR}/setup.sh"

# ============== DEBUG: Environment diagnostics ==============
echo "===== ENVIRONMENT DEBUG INFO ====="
echo "Hostname: $(hostname)"
echo "Date: $(date)"
echo "Working directory: $(pwd)"
echo "SLURM_JOB_ID: ${SLURM_JOB_ID:-not set}"
echo "which hpstr: $(which hpstr 2>&1 || echo 'NOT FOUND')"
echo "HPSTR_BASE: ${HPSTR_BASE:-not set}"
echo "Sourced config from: ${SUBMIT_DIR}/config.sh"
echo "===== END DEBUG INFO ====="
echo ""
# ============== END DEBUG ==============

# OUTPUT_DIR and HPSTR_CONFIG come from the environment (sbatch --export) with
# config.sh defaults as a fallback.
OUTPUT_DIR="${OUTPUT_DIR:-}"
HPSTR_CONFIG="${HPSTR_CONFIG:-anaFeeSmearing_3p74_cfg.py}"
# The FEE skims are slimmed (no SVT hit collections), so the no-hit-collection
# recoTuple variant is the default; override RECO_CONFIG for unslimmed inputs.
RECO_CONFIG="${RECO_CONFIG:-${HPSTR_BASE}/processors/config/recoTuple_noHitColl_cfg.py}"

if [[ -z "${OUTPUT_DIR}" ]]; then
  echo "ERROR: OUTPUT_DIR not set. Pass via sbatch --export or set in config.sh"
  exit 1
fi

IFS=',' read -r -a input_files <<< "${2}"
shift 2

# Build output directory (with optional systematics subdir).
out_dir="${OUTPUT_DIR}"
if [[ -n "${PRESELECT_SUBDIR:-}" ]]; then
  out_dir="${out_dir}/${PRESELECT_SUBDIR}"
fi
mkdir -p "${out_dir}"

# Determine data/MC flag from the --sample argument, then strip --sample so it
# is not forwarded to the FEE config (which does not define it).
# -t 1 = data, -t 0 = MC
is_data=0  # default to MC
hpstr_args=()
prev_arg=""
for arg in "$@"; do
  if [[ "${prev_arg}" == "--sample" ]]; then
    [[ "${arg}" == "data" ]] && is_data=1
    prev_arg=""
    continue          # drop the --sample value
  fi
  if [[ "${arg}" == "--sample" ]]; then
    prev_arg="${arg}"
    continue          # drop the --sample flag
  fi
  hpstr_args+=("${arg}")
done
echo "Data/MC flag: -t ${is_data}  (1=data, 0=MC)"

# Generate a unique smearing seed from the SLURM job ID.
smearing_seed="${SLURM_JOB_ID:-$RANDOM}"
echo "Using smearing seed: ${smearing_seed}"
echo "Config: ${HPSTR_CONFIG}"

# Convert an .slcio file to a persistent ROOT ntuple (reused if it already exists).
# Prints the ntuple path on stdout; returns non-zero on failure.
convert_lcio() {
  local slcio="$1"
  local ntuple
  if [[ -n "${NTUPLE_DIR:-}" ]]; then
    local run_dir=""
    [[ "${slcio}" =~ (hps_[0-9]+)/ ]] && run_dir="${BASH_REMATCH[1]}"
    ntuple="${NTUPLE_DIR}/${run_dir}/$(basename "${slcio%.slcio}").root"
  else
    ntuple="${slcio%.slcio}.root"
  fi

  if [[ -s "${ntuple}" ]]; then
    echo "Reusing existing ntuple: ${ntuple}" >&2
    echo "${ntuple}"
    return 0
  fi

  mkdir -p "$(dirname "${ntuple}")"
  local tmp="${ntuple%.root}.tmp_${SLURM_JOB_ID:-$$}.root"
  echo "Converting ${slcio} -> ${ntuple}" >&2
  if hpstr "${RECO_CONFIG}" -i "${slcio}" -o "${tmp}" -t "${is_data}" >&2 && [[ -s "${tmp}" ]]; then
    mv -f "${tmp}" "${ntuple}"
    echo "${ntuple}"
  else
    echo "ERROR: recoTuple conversion failed for ${slcio}" >&2
    rm -f "${tmp}"
    return 1
  fi
}

n_failed=0
for input_file in "${input_files[@]}"; do
  [[ -z "${input_file}" ]] && continue
  echo ""
  echo "===== Input: ${input_file} ====="

  ana_input="${input_file}"
  if [[ "${input_file}" == *.slcio ]]; then
    if ! ana_input="$(convert_lcio "${input_file}")"; then
      ((++n_failed))
      continue
    fi
  fi

  # Disambiguate the output filename by run number when present (hps_0NNNNN),
  # so files from different run subdirectories don't collide.
  filename="$(basename "${ana_input}")"
  if [[ "${ana_input}" =~ (hps_[0-9]+) && "${filename}" != *"${BASH_REMATCH[1]}"* ]]; then
    filename="${BASH_REMATCH[1]}_${filename}"
  fi
  output_file="${out_dir}/${filename}"

  echo "Input:  ${ana_input}"
  echo "Output: ${output_file}"

  if ! hpstr "${HPSTR_CONFIG}" -i "${ana_input}" -o "${output_file}" \
         -t "${is_data}" --seed "${smearing_seed}" "${hpstr_args[@]+"${hpstr_args[@]}"}"; then
    echo "ERROR: hpstr FEE analysis exited with non-zero status."
    rm -f "${output_file}"
    ((++n_failed))
  fi
done

if [[ "${n_failed}" -gt 0 ]]; then
  echo "ERROR: ${n_failed} of ${#input_files[@]} input(s) failed"
  exit 1
fi
