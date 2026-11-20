#!/usr/bin/env python3
"""
Compare preselection cutflow efficiencies across N sample folders.

Recursively finds all .root files under each given folder, sums the
'event_cutflow_h' and 'vertex_cutflow_h' histograms across those files,
normalizes each so the first bin is 1 (i.e. converts raw counts into a
cut efficiency relative to the first cut), then overlays all folders on
the same plot for comparison.

Usage:
    python compare_cutflows.py <folder1> <folder2> ... [--labels NAME1 NAME2 ...] [--output plot.png]

Example:
    python compare_cutflows.py \\
        /sdf/data/hps/physics2021/preselection/v13/ap_signal_smeared/ap180MeV \\
        /sdf/data/hps/physics2021/preselection/v13/ap_signal_smeared_improvedTrking/ap180MeV \\
        --labels nominal improved_tracking
"""
import argparse
import glob
import os
import sys

import matplotlib.pyplot as plt
import numpy as np
import uproot

CUTFLOW_HISTS = ["event_cutflow_h", "vertex_cutflow_h"]

# Fixed-order categorical palette (colorblind-validated)
COLORS = [
    "#2a78d6",  # blue
    "#008300",  # green
    "#e87ba4",  # magenta
    "#eda100",  # yellow
    "#1baf7a",  # aqua
    "#eb6834",  # orange
    "#4a3aa7",  # violet
    "#e34948",  # red
]
MARKERS = ["o", "s", "^", "D", "v", "P", "X", "*"]


def sum_cutflow(folder, hist_name):
    """Sum a named cutflow histogram across every ROOT file found under folder."""
    root_files = sorted(glob.glob(os.path.join(folder, "**", "*.root"), recursive=True))
    if not root_files:
        raise FileNotFoundError(f"No ROOT files found under {folder}")

    total = None
    bin_labels = None
    for fname in root_files:
        with uproot.open(fname) as f:
            if hist_name not in f:
                continue
            h = f[hist_name]
            vals, _ = h.to_numpy()
            if total is None:
                total = vals.copy()
                bin_labels = h.axis().labels()
            else:
                total += vals
    if total is None:
        raise KeyError(f"'{hist_name}' not found in any ROOT file under {folder}")
    return total, bin_labels


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument(
        "folders", nargs="+", help="Sample folders to compare (searched recursively for .root files)"
    )
    parser.add_argument(
        "--labels", nargs="+", help="Legend label for each folder (default: folder basenames)"
    )
    parser.add_argument(
        "--output", default="cutflow_comparison.png", help="Output plot filename"
    )
    args = parser.parse_args()

    if args.labels and len(args.labels) != len(args.folders):
        sys.exit("--labels must have the same length as the number of folders")
    labels = args.labels or [os.path.basename(os.path.normpath(f)) for f in args.folders]

    if len(args.folders) > len(COLORS):
        sys.exit(f"Too many folders ({len(args.folders)}); supports up to {len(COLORS)}")

    fig, axes = plt.subplots(len(CUTFLOW_HISTS), 1, figsize=(9, 5 * len(CUTFLOW_HISTS)))

    for ax, hist_name in zip(axes, CUTFLOW_HISTS):
        for folder, label, color, marker in zip(args.folders, labels, COLORS, MARKERS):
            vals, bin_labels = sum_cutflow(folder, hist_name)
            efficiency = vals / vals[0]
            x = np.arange(len(efficiency))
            ax.plot(
                x, efficiency,
                color=color, marker=marker, markersize=6,
                linewidth=1.6, label=label,
            )

        ax.set_xticks(x)
        ax.set_xticklabels(bin_labels, rotation=45, ha="right", fontsize=8)
        ax.set_ylabel("Cut efficiency (relative to bin 1)")
        ax.set_title(hist_name)
        ax.grid(True, color="#e1e0d9", linewidth=0.8)
        ax.set_axisbelow(True)
        ax.legend(frameon=False)

    fig.tight_layout()
    fig.savefig(args.output, dpi=150)
    print(f"Saved {args.output}")


if __name__ == "__main__":
    main()
