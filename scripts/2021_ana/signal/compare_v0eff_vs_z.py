#!/usr/bin/env python3
"""Overlay v0-finding efficiency vs. one or more truth quantities from TWO TruthVertexMatch2021 files.

Companion to plot_v0eff_vs_z.py: instead of showing all curves for one file, this overlays a
chosen subset of categories from two different "truth_vertex_match" ntuples on the same axes so
they can be compared directly (e.g. two mass points, two samples, or before/after a processor
change). Each (file, category) combination gets its own TEfficiency curve: color encodes the
category (so the same category is the same color in both files), marker shape encodes the
category too, and the two files are told apart by filled (file 1) vs. hollow (file 2) markers of
that same shape.

One plot is written per --xvar, so a single command produces the whole comparison set. The
default is the decay-vertex z plus per-leg momentum and tanLambda, because a difference between
two samples/reco variants is usually either a displacement effect (z), a momentum effect, or an
angular-acceptance effect (tanLambda), and reading all three at once tells them apart:

  --xvar z ele_p pos_p ele_tanL pos_tanL

Any name from plot_v0eff_vs_z.VARIABLES works (z, ap_p, ele_p, pos_p, ele_tanL, pos_tanL,
ele_phi0, pos_phi0); that registry is imported rather than duplicated here, so the expressions,
default ranges and axis titles are guaranteed identical to the single-file plots.

Default categories are the two the study usually cares about:
  - "matched_trk_pos" : >=1 v0 carrying the truth e+ (no requirement on the e-)
  - "matched_trk"     : >=1 v0 track-matched on BOTH legs (truth e- and e+ on the same v0)
Pass --categories to compare a different set (any of: has_v0, matched_trk, matched_trk_ele,
matched_trk_pos, matched_trk_clus_ele_only -- see TruthVertexMatch2021.cxx).

Usage:
  # default set of plots (z, ele_p, pos_p, ele_tanL, pos_tanL):
  ./compare_v0eff_vs_z.py -i1 v0eff_vs_z/truth_vertex_match_ap60MeV.root \\
                           -i2 v0eff_vs_z/truth_vertex_match_ap100MeV.root \\
                           -o v0eff_vs_z
  # just one variable, custom labels and categories:
  ./compare_v0eff_vs_z.py -i1 fileA.root -i2 fileB.root --label1 "old cuts" --label2 "new cuts" \\
                           --xvar ele_tanL --categories matched_trk,matched_trk_ele,matched_trk_pos
  # restrict the range (applies to every --xvar in the command):
  ./compare_v0eff_vs_z.py -i1 A.root -i2 B.root --xvar ele_p --xmin 0 --xmax 3.5 --nbins 35

Each curve's label defaults to the mass auto-parsed from its filename (same convention as
plot_v0eff_vs_z.py); pass --label1/--label2 to override with something else (e.g. a cut variant
name) when the two files are not simply different mass points.

Errors are binomial (Clopper-Pearson) via TEfficiency, independent per file.
"""
import argparse
import os
import sys

import ROOT

# Single source of truth for the x-axis variables, the efficiency construction and the
# filename/label conventions: they are defined in the single-file plotting script and imported
# here so the two cannot drift apart. plot_v0eff_vs_z.py guards its own main() behind
# __name__ == "__main__", so importing it is side-effect free apart from ROOT's batch setup.
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from plot_v0eff_vs_z import (  # noqa: E402
    DENOM_CUT,
    TREE_NAME,
    VARIABLES,
    make_efficiency,
    output_tag,
    parse_mass_label,
)

ROOT.gROOT.SetBatch(ROOT.kTRUE)
ROOT.gStyle.SetOptStat(0)
ROOT.gStyle.SetPadTickX(1)
ROOT.gStyle.SetPadTickY(1)
ROOT.gErrorIgnoreLevel = ROOT.kWarning

# Plotted by default: the displacement variable plus both legs' momentum and tanLambda, so a
# difference between the two files can be attributed to displacement, momentum or angle without
# a second invocation.
DEFAULT_XVARS = ["z", "ele_p", "pos_p", "ele_tanL", "pos_tanL"]

CATEGORY_LABELS = {
    "has_v0": "#geq1 reco v0 (no selection)",
    "matched_trk": "both e^{-} and e^{+} tracks",
    "matched_trk_ele": "has e^{-} (any e^{+})",
    "matched_trk_pos": "has e^{+} (any e^{-})",
    "matched_trk_clus_ele_only": "both tracks + e^{-} ECal cluster",
}
# filled-marker family used for the first file; FILE2_MARKER remaps to the matching hollow shape
# so the two files stay visually distinct even when a category's color alone is hard to judge.
CATEGORY_MARKERS = {
    "has_v0": 21,
    "matched_trk": 20,
    "matched_trk_ele": 23,
    "matched_trk_pos": 22,
    "matched_trk_clus_ele_only": 33,
}
# color now encodes the category (so the same category is the same color in both files); the two
# files are told apart by filled (file 1) vs. hollow (file 2) markers of the SAME shape instead.
CATEGORY_COLORS = {
    "has_v0": ROOT.kBlue + 1,
    "matched_trk": ROOT.kGreen + 2,
    "matched_trk_ele": ROOT.kOrange + 1,
    "matched_trk_pos": ROOT.kMagenta + 1,
    "matched_trk_clus_ele_only": ROOT.kCyan + 2,
}
FILE2_MARKER = {21: 25, 20: 24, 23: 32, 22: 26, 33: 27}


def load_tree(path):
    if not os.path.isfile(path):
        print(f"Error: input file not found: {path}", file=sys.stderr)
        return None, None
    f = ROOT.TFile.Open(path)
    tree = f.Get(TREE_NAME)
    if not tree:
        print(f"Error: tree '{TREE_NAME}' not found in {path}", file=sys.stderr)
        return None, None
    return f, tree


def compare_variable(tree1, tree2, var_key, args, label1, label2, tag1, tag2, categories,
                     denom_cut=DENOM_CUT):
    """Build, draw and tabulate the two-file overlay for one x variable. Returns the output path."""
    cfg = VARIABLES[var_key]
    expr = cfg["expr"]
    nbins = args.nbins if args.nbins is not None else cfg["nbins"]
    xmin = args.xmin if args.xmin is not None else cfg["xmin"]
    xmax = args.xmax if args.xmax is not None else cfg["xmax"]

    print(f"\n===== --xvar {var_key} ({expr}, range [{xmin}, {xmax}], {nbins} bins) =====")

    # (eff, color, marker, legend_text); the list also keeps the TEfficiency objects alive, which
    # ROOT needs since the canvas does not own them.
    curves = []
    h_dens = []
    for cat in categories:
        # ROOT object names must be unique across variables, hence var_key in the name.
        eff1, h_den1, _ = make_efficiency(tree1, expr, cat, nbins, xmin, xmax,
                                          f"f1_{var_key}_{cat}", denom_cut=denom_cut)
        eff2, h_den2, _ = make_efficiency(tree2, expr, cat, nbins, xmin, xmax,
                                          f"f2_{var_key}_{cat}", denom_cut=denom_cut)
        marker = CATEGORY_MARKERS[cat]
        color = CATEGORY_COLORS[cat]
        curves.append((eff1, color, marker, f"{label1}: {CATEGORY_LABELS[cat]}"))
        curves.append((eff2, color, FILE2_MARKER[marker], f"{label2}: {CATEGORY_LABELS[cat]}"))
        h_dens = [h_den1, h_den2]  # same binning for every category; last one wins

    for eff, color, marker, _ in curves:
        eff.SetLineColor(color)
        eff.SetMarkerColor(color)
        eff.SetMarkerStyle(marker)

    c = ROOT.TCanvas(f"c_v0eff_compare_{var_key}", "v0 efficiency comparison", 800, 600)
    c.SetTickx(1)
    c.SetTicky(1)
    c.SetLeftMargin(0.13)
    c.SetBottomMargin(0.12)
    frame = ROOT.TH1D(f"frame_{var_key}", f";{cfg['xlabel']};v0-finding efficiency",
                      1, xmin, xmax)
    frame.SetMinimum(0.0)
    frame.SetMaximum(1.75)  # headroom above 1.0 so the legend doesn't sit on top of the curves
    frame.GetXaxis().SetTitleSize(0.05)
    frame.GetXaxis().SetLabelSize(0.045)
    frame.GetYaxis().SetTitleSize(0.05)
    frame.GetYaxis().SetLabelSize(0.045)
    frame.GetYaxis().SetTitleOffset(1.2)
    frame.Draw()

    for eff, _, _, _ in curves:
        eff.Draw("pe same")

    legend = ROOT.TLegend(0.35, 0.85 - 0.045 * len(curves), 0.88, 0.88)
    legend.SetTextSize(0.032)
    for eff, _, _, legend_text in curves:
        legend.AddEntry(eff, legend_text, "pe")
    legend.Draw()

    latex = ROOT.TLatex()
    latex.SetNDC()
    latex.SetTextAlign(31)
    latex.SetTextFont(42)
    latex.SetTextSize(0.04)
    latex.DrawLatex(0.9, 0.91, "HPS Work in Progress")

    outpath = os.path.join(args.output, f"v0eff_compare_{var_key}_{tag1}_vs_{tag2}.png")
    c.SaveAs(outpath)
    print(f"Wrote {outpath}")

    # per-bin table: bin edges, each file's denominator, then every curve's efficiency. The two
    # denominators are printed separately because the samples are independent -- a difference in
    # efficiency is only interesting where both files actually have events.
    h_den1, h_den2 = h_dens
    header = f"{'x_lo':>9} {'x_hi':>9} {'n1':>8} {'n2':>8}"
    for _, _, _, legend_text in curves:
        header += f" {legend_text:>34}"
    print(header)
    for b in range(1, nbins + 1):
        x_lo = h_den1.GetXaxis().GetBinLowEdge(b)
        x_hi = h_den1.GetXaxis().GetBinUpEdge(b)
        row = (f"{x_lo:9.3f} {x_hi:9.3f} {h_den1.GetBinContent(b):8.0f} "
               f"{h_den2.GetBinContent(b):8.0f}")
        for eff, _, _, _ in curves:
            row += f" {eff.GetEfficiency(b):34.3f}"
        print(row)

    return outpath


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("-i1", "--input1", required=True, help="First TruthVertexMatch2021 output ROOT file")
    parser.add_argument("-i2", "--input2", required=True, help="Second TruthVertexMatch2021 output ROOT file")
    parser.add_argument("--label1", default=None, help="Legend label for file 1 (default: auto mass label)")
    parser.add_argument("--label2", default=None, help="Legend label for file 2 (default: auto mass label)")
    parser.add_argument("-o", "--output", default=".", help="Output directory for the plots")
    parser.add_argument("--xvar", nargs="+", default=DEFAULT_XVARS, choices=sorted(VARIABLES.keys()),
                        help="One or more truth quantities to compare the efficiency against. "
                             "One plot is written per name. Default: "
                             f"{' '.join(DEFAULT_XVARS)}.")
    parser.add_argument("--categories", default="matched_trk_pos,matched_trk",
                         help="Comma-separated list of branches to compare "
                              "(default: matched_trk_pos,matched_trk)")
    parser.add_argument("--nbins", type=int, default=None,
                        help="Number of bins, applied to every --xvar requested. "
                             "Default: each variable's own default (see plot_v0eff_vs_z.VARIABLES).")
    parser.add_argument("--xmin", "--zmin", dest="xmin", type=float, default=None,
                        help="Lower edge of the x range, applied to every --xvar requested. "
                             "Default: each variable's own default.")
    parser.add_argument("--xmax", "--zmax", dest="xmax", type=float, default=None,
                        help="Upper edge of the x range, applied to every --xvar requested. "
                             "Default: each variable's own default.")
    parser.add_argument("--cut", default=None,
                        help="Extra TTree::Draw selection ANDed onto the denominator of BOTH files "
                             "(and therefore the numerator too, since numerator = denom && category) "
                             "for every --xvar in this command, e.g. --cut \"abs(true_ele_slopeY)>0.01\".")
    args = parser.parse_args()

    categories = [c.strip() for c in args.categories.split(",") if c.strip()]
    for cat in categories:
        if cat not in CATEGORY_LABELS:
            print(f"Error: unknown category '{cat}'; choose from {sorted(CATEGORY_LABELS)}", file=sys.stderr)
            return 1

    os.makedirs(args.output, exist_ok=True)

    f1, tree1 = load_tree(args.input1)
    f2, tree2 = load_tree(args.input2)
    if tree1 is None or tree2 is None:
        return 1

    denom_cut = DENOM_CUT if not args.cut else f"({DENOM_CUT}) && ({args.cut})"

    n1 = tree1.GetEntries(denom_cut)
    n2 = tree2.GetEntries(denom_cut)
    print(f"file 1 truth-signal events: {n1}")
    print(f"file 2 truth-signal events: {n2}")
    if n1 == 0 or n2 == 0:
        print("No truth-signal events found in one of the files; nothing to plot.", file=sys.stderr)
        return 1

    label1 = args.label1 or parse_mass_label(args.input1) or output_tag(args.input1)
    label2 = args.label2 or parse_mass_label(args.input2) or output_tag(args.input2)
    tag1 = output_tag(args.input1)
    tag2 = output_tag(args.input2)

    for var_key in args.xvar:
        compare_variable(tree1, tree2, var_key, args, label1, label2, tag1, tag2, categories,
                         denom_cut=denom_cut)

    f1.Close()
    f2.Close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
