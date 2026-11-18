#!/usr/bin/env python3
"""
Compare truth and reconstructed vertex mass under the two truth-match definitions.

  *_truth_link_hit   hit-based link only (majority vote over SVTTrueHitRelations)
  *_has_truth_link   hit-based link OR the kinematic dR/dp fallback

Both tracks are required to be matched. Everything is split on isRadEle, and every
histogram is normalised to unit area so shapes can be compared directly regardless
of how many events each selection keeps.

Three figures per mass point:
  1. isRadEle==0 : reco and truth mass, under both flag definitions
  2. isRadEle==1 : same
  3. reco mass only, hit vs link, one panel per isRadEle value

Uses uproot + matplotlib rather than PyROOT: cppyy segfaults against this
environment's Python during interpreter teardown, even for a trivial TTree::Draw.

Usage:
  ./plot_truth_reco_mass.py                       # all masses, default input dir
  ./plot_truth_reco_mass.py -m ap100MeV ap180MeV  # selected masses
  ./plot_truth_reco_mass.py -o /path/to/outdir
"""

import argparse
import glob
import os
import re
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
import numpy as np
import uproot

DEFAULT_INPUT = "/sdf/data/hps/physics2021/preselection/v16/ap_signal_smeared"

RECO_BR = "vertex./vertex.invM_"
TRUE_BR = "true_vertex_invM"
BRANCHES = [RECO_BR, TRUE_BR, "isRadEle",
            "ele_truth_link_hit", "pos_truth_link_hit",
            "ele_has_truth_link", "pos_has_truth_link"]

HIT_COLOUR = "#1f77b4"
LINK_COLOUR = "#d62728"


def nominal_mass_gev(mass_dir):
    """ap100MeV -> 0.100"""
    m = re.search(r"ap(\d+)MeV", mass_dir)
    return int(m.group(1)) / 1000.0 if m else None


def mass_window(m_gev):
    """Wide enough for the reco tails without burying the peak."""
    half = max(0.030, 0.35 * m_gev)
    return max(0.0, m_gev - half), m_gev + half


def load(mass_dir, indir):
    files = sorted(glob.glob(os.path.join(indir, mass_dir, "*.root")))
    if not files:
        return None
    chunks = []
    for path in files:
        with uproot.open(path) as f:
            if "preselection" not in f:
                continue
            chunks.append(f["preselection"].arrays(BRANCHES, library="np"))
    if not chunks:
        return None
    return {k: np.concatenate([c[k] for c in chunks]) for k in BRANCHES}


def norm_hist(values, bins):
    """Unit-area histogram; returns (counts, n_entries)."""
    counts, _ = np.histogram(values, bins=bins)
    n = counts.sum()
    return (counts / n if n > 0 else counts.astype(float)), int(len(values))


def step(ax, bins, counts, label, colour, ls, lw=1.6, alpha=1.0):
    # The two flag definitions often agree almost exactly, so the hit-link curve is
    # drawn thick and semi-transparent and the has-link curve thin on top of it;
    # otherwise one hides the other completely.
    ax.step(bins[:-1], counts, where="post", label=label, color=colour,
            linestyle=ls, linewidth=lw, alpha=alpha)


def panel_truth_and_reco(ax, d, israd, bins, mass_dir):
    """Reco and truth mass, under both flag definitions, for one isRadEle value."""
    sel_rad = d["isRadEle"] == bool(israd)
    combos = [
        ("hit link", d["ele_truth_link_hit"] & d["pos_truth_link_hit"], HIT_COLOUR, 3.2, 0.55),
        ("has link", d["ele_has_truth_link"] & d["pos_has_truth_link"], LINK_COLOUR, 1.5, 1.0),
    ]
    reco_max = 0.0
    truth_max = 0.0
    for name, flag, colour, lw, alpha in combos:
        m = sel_rad & flag
        for br, style, tag in ((RECO_BR, "-", "reco"), (TRUE_BR, "--", "truth")):
            counts, n = norm_hist(d[br][m], bins)
            step(ax, bins, counts, f"{tag}, {name} (N={n})", colour, style, lw, alpha)
            if counts.size:
                if tag == "reco":
                    reco_max = max(reco_max, counts.max())
                else:
                    truth_max = max(truth_max, counts.max())

    # For the correctly-paired category the truth mass is a delta function at the
    # generated value, so a single bin holds essentially the whole distribution.
    # Scale to the reco histograms and let the truth spike clip, otherwise every
    # other curve is compressed into the axis.
    if reco_max > 0:
        ax.set_ylim(0, 1.25 * reco_max)
        if truth_max > 1.25 * reco_max:
            ax.annotate(f"truth peak {truth_max:.2f} (clipped)",
                        xy=(0.02, 0.95), xycoords="axes fraction",
                        fontsize=8, color="0.35")
    ax.set_title(f"{mass_dir}   isRadEle=={israd}")
    ax.set_xlabel(r"$m_{ee}$ [GeV]")
    ax.set_ylabel("normalised to 1")
    ax.legend(fontsize=8, frameon=False)
    ax.grid(alpha=0.3)


def panel_reco_only(ax, d, israd, bins, mass_dir):
    """Reco mass only: hit flag vs link flag."""
    sel_rad = d["isRadEle"] == bool(israd)
    for name, flag, colour, lw, alpha in [
            ("hit link", d["ele_truth_link_hit"] & d["pos_truth_link_hit"], HIT_COLOUR, 3.2, 0.55),
            ("has link", d["ele_has_truth_link"] & d["pos_has_truth_link"], LINK_COLOUR, 1.5, 1.0)]:
        counts, n = norm_hist(d[RECO_BR][sel_rad & flag], bins)
        step(ax, bins, counts, f"reco, {name} (N={n})", colour, "-", lw, alpha)
    ax.set_title(f"{mass_dir}   reco mass, hit vs link   (isRadEle=={israd})")
    ax.set_xlabel(r"$m_{ee}$ [GeV]")
    ax.set_ylabel("normalised to 1")
    ax.legend(fontsize=8, frameon=False)
    ax.grid(alpha=0.3)


def process_mass(mass_dir, indir, outdir, nbins, pdf):
    m_gev = nominal_mass_gev(mass_dir)
    if m_gev is None:
        print(f"  skipping {mass_dir}: cannot parse mass")
        return
    d = load(mass_dir, indir)
    if d is None:
        print(f"  skipping {mass_dir}: no usable files")
        return

    lo, hi = mass_window(m_gev)
    bins = np.linspace(lo, hi, nbins + 1)
    print(f"  {mass_dir}: {len(d[RECO_BR])} entries, window [{lo:.3f}, {hi:.3f}] GeV")

    # figures 1 and 2: truth + reco, both flags, one per isRadEle value
    for israd in (0, 1):
        fig, ax = plt.subplots(figsize=(7.5, 5.5))
        panel_truth_and_reco(ax, d, israd, bins, mass_dir)
        fig.tight_layout()
        fig.savefig(os.path.join(outdir, f"{mass_dir}_isRadEle{israd}.png"), dpi=130)
        pdf.savefig(fig)
        plt.close(fig)

    # figure 3: reco only, hit vs link, both isRadEle values side by side
    fig, axes = plt.subplots(1, 2, figsize=(13, 5))
    for ax, israd in zip(axes, (0, 1)):
        panel_reco_only(ax, d, israd, bins, mass_dir)
    fig.tight_layout()
    fig.savefig(os.path.join(outdir, f"{mass_dir}_reco_hit_vs_link.png"), dpi=130)
    pdf.savefig(fig)
    plt.close(fig)


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("-i", "--indir", default=DEFAULT_INPUT)
    ap.add_argument("-o", "--outdir", default="truth_reco_mass_plots")
    ap.add_argument("-m", "--masses", nargs="*", default=None)
    ap.add_argument("-n", "--nbins", type=int, default=80)
    args = ap.parse_args()

    masses = args.masses or [d for d in os.listdir(args.indir)
                             if re.match(r"ap\d+MeV$", d)
                             and os.path.isdir(os.path.join(args.indir, d))]
    masses.sort(key=lambda d: nominal_mass_gev(d) or 0)
    if not masses:
        sys.exit(f"no apXXXMeV directories under {args.indir}")

    os.makedirs(args.outdir, exist_ok=True)
    pdf_path = os.path.join(args.outdir, "truth_reco_mass_all.pdf")
    print(f"input : {args.indir}")
    print(f"output: {args.outdir}")
    print(f"masses: {', '.join(masses)}\n")

    with PdfPages(pdf_path) as pdf:
        for mass_dir in masses:
            process_mass(mass_dir, args.indir, args.outdir, args.nbins, pdf)

    print(f"\nwrote {args.outdir}/*.png and {pdf_path}")


if __name__ == "__main__":
    main()
