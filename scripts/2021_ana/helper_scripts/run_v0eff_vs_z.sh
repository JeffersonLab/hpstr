#!/bin/bash
# Run TruthVertexMatch2021 over a displaced A' (or SIMP) signal sample and plot the
# v0-finding efficiency as a function of the true A' decay-vertex z.
#
# This is the local/interactive counterpart to run.sh: instead of a single hpstr call
# producing a preselection tree, it loops hpstr over every file in a mass-point list
# using processors/config/truthVertexMatch_cfg.py, hadds the per-file "truth_vertex_match"
# trees, and hands the merged file to plot_v0eff_vs_z.py.
#
# The merged ntuple is named after the input list's basename (e.g. ap100MeV.list ->
# truth_vertex_match_ap100MeV.root) so that running this for several mass points writes
# distinct, non-clobbering files, and so plot_v0eff_vs_z.py can auto-parse the mass straight
# out of the filename to label the plot -- no need to pass it by hand.
#
# Usage:
#   ./run_v0eff_vs_z.sh
#   ./run_v0eff_vs_z.sh --list ../mc_lists/pass5/ap150MeV.list --apPDG 622 -n 50000
#   ./run_v0eff_vs_z.sh --sample simp_signal --list ../mc_lists/pass5/simp90MeV-merged.list
#   # several mass points into the same outdir (each gets its own merged file + plot):
#   for m in ap60MeV ap80MeV ap100MeV ap150MeV; do
#     ./run_v0eff_vs_z.sh --list ../mc_lists/pass5/${m}.list
#   done

set -o errexit
set -o nounset

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CFG="${SCRIPT_DIR}/../../../processors/config/truthVertexMatch_cfg.py"

LIST="${SCRIPT_DIR}/../mc_lists/pass6/matt_graham.txt"
OUTDIR="${SCRIPT_DIR}/v0eff_vs_z"
SAMPLE="ap_signal"   # ap_signal, simp_signal, rad
APPDG=622            # 622 = displaced A', 623 = prompt A' (ignored for simp_signal/rad)
NEVENTS=10000            # -1 = all events in each file

while [[ $# -gt 0 ]]; do
  case "$1" in
    --list) LIST="$2"; shift 2 ;;
    --outdir) OUTDIR="$2"; shift 2 ;;
    --sample) SAMPLE="$2"; shift 2 ;;
    --apPDG) APPDG="$2"; shift 2 ;;
    -n|--nevents) NEVENTS="$2"; shift 2 ;;
    -h|--help) grep '^#' "$0" | sed 's/^#//'; exit 0 ;;
    *) echo "Unknown option: $1" >&2; exit 1 ;;
  esac
done

mkdir -p "${OUTDIR}"

list_tag="$(basename "${LIST}")"
list_tag="${list_tag%.*}"  # e.g. ap100MeV.list -> ap100MeV, simp90MeV-merged.list -> simp90MeV-merged
merged="${OUTDIR}/truth_vertex_match_${list_tag}.root"
rm -f "${merged}"

per_file_outputs=()
i=0
while read -r infile; do
  [[ -z "${infile}" ]] && continue
  i=$((i + 1))
  outfile="${OUTDIR}/tvm_${list_tag}_$(printf '%03d' "${i}").root"
  echo "[${i}] ${infile}"
  hpstr "${CFG}" -i "${infile}" -o "${outfile}" -t 0 --seed 100 \
      --sample "${SAMPLE}" --apPDG "${APPDG}" -n "${NEVENTS}"
  per_file_outputs+=("${outfile}")
done < "${LIST}"

if [[ ${#per_file_outputs[@]} -eq 0 ]]; then
  echo "No input files found in ${LIST}" >&2
  exit 1
fi

hadd -f "${merged}" "${per_file_outputs[@]}"

python3 "${SCRIPT_DIR}/plot_v0eff_vs_z.py" -i "${merged}" -o "${OUTDIR}"
