#!/usr/bin/env python3
"""Plot v0-finding efficiency vs. one or more truth quantities from a TruthVertexMatch2021 tree.

Reads the "truth_vertex_match" tree produced by TruthVertexMatch2021 (see
processors/src/TruthVertexMatch2021.cxx and run_v0eff_vs_z.sh in this directory) and, among
events where a truth signal (A'/SIMP vector) was found, produces up to three plots PER
--xvar requested (pass multiple --xvar values to get the full set for each in one command).

Available --xvar names (see VARIABLES below for exact expressions/ranges):
  z          true A' decay-vertex z [mm]                          (default)
  ap_p       true A'/v0 momentum |p| [GeV]
  ele_p      true e^{-} momentum |p| [GeV]
  pos_p      true e^{+} momentum |p| [GeV]
  ele_tanL   true e^{-} tanLambda proxy (p_y/p_z at the true vertex)
  pos_tanL   true e^{+} tanLambda proxy (p_y/p_z at the true vertex)
  ele_phi0   true e^{-} phi0 proxy (p_x/p_z at the true vertex, bend-plane slope)
  pos_phi0   true e^{+} phi0 proxy (p_x/p_z at the true vertex, bend-plane slope)
The tanL/phi0 proxies are truth-momentum-direction slopes at the true vertex, not the actual
fitted helix parameters (which are quoted at closest approach and include the effect of
propagating through the field) -- see the true_ele_slopeX/Y branches in TruthVertexMatch2021.cxx.

Main plot (v0eff_vs_<xvar>_<tag>.png), unconditioned efficiencies:
  - "has_v0"        : >=1 reconstructed v0 in the event (no selection applied)
  - "matched_trk"     : >=1 v0 whose two tracks truth-match the true e-/e+ by majority hits,
                        independent of the reconstructed vertex z (both legs required)
  - "matched_trk_ele" : >=1 v0 carrying the truth e- (no requirement on the e+ leg)
  - "matched_trk_pos" : >=1 v0 carrying the truth e+ (no requirement on the e- leg)
  - "matched_trk_clus_ele_only" : >=1 v0 with BOTH tracks matched AND an e- ECal cluster present
                        (e+ cluster not required) -- i.e. matched_trk plus an e- cluster on top

matched_trk is the "did we build the right vertex" flag; it does not depend on how well the
vertex fit's z happens to agree with truth (see TruthVertexMatch2021.cxx for the z-window-based
matched_z, kept in the tree but not plotted here). Comparing matched_trk_ele/matched_trk_pos to
matched_trk shows whether it is meaningfully harder to reconstruct both signal tracks into the
same v0 than to reconstruct just one of them.

Note on vertex multiplicity: every one of these flags is a per-EVENT OR over every v0 in
UnconstrainedV0Vertices_KF (see the vtx loop in TruthVertexMatch2021::process) -- an event
counts in the numerator if ANY reconstructed v0 satisfies the condition, regardless of how many
other (possibly fake/combinatoric) v0s are also in the event. In particular matched_trk_ele and
matched_trk_pos can both be true from two DIFFERENT v0s (e.g. one v0 has the right e- paired with
a wrong e+, another has the right e+ paired with a wrong e-); that does not imply matched_trk is
true, which requires both legs to be correct on the SAME v0. So matched_trk <=
min(matched_trk_ele, matched_trk_pos) always holds, but the reverse need not.

Per-leg truth-hit comparison plots (v0eff_vs_<xvar>_pos_truthhits_<tag>.png and
v0eff_vs_<xvar>_ele_truthhits_<tag>.png), each with exactly two curves so the effect of the
truth-hit requirement on that ONE leg is isolated:
  - the plain matched_trk_pos (resp. matched_trk_ele) baseline, identical to the main plot's curve
  - the same flag, but with the DENOMINATOR further restricted to events where that SAME leg's
    true track crossed at least --minTruthHits distinct SVT layers (has_truth_hits/
    n_truth_layers_ele/n_truth_layers_pos from TruthVertexMatch2021, populated only when
    recoTuple_cfg.py was run with -sh/--truthHits 1). Unique-layer count is used instead of the
    raw truth-hit count (n_truth_hits_ele/pos) so that split-cluster/duplicate hits on the same
    layer don't inflate the count without adding real tracking information. The OTHER leg's layer
    count is not constrained, so the difference between the two curves isolates this leg's own
    hit-sufficiency effect. These two plots are silently omitted if the input has no truth-hit
    info at all.

The mass shown in the top-left label is auto-parsed from the input filename (e.g.
"truth_vertex_match_ap100MeV.root" -> "100 MeV"); run_v0eff_vs_z.sh names its merged output file
after the input mass-point list for exactly this purpose. Pass --mass to override.

Pass --cut to AND an extra selection onto the denominator (and therefore the numerator too) for
every --xvar in the command -- e.g. to drop a low-statistics/degenerate region of the variable
being plotted:
  --xvar ele_tanL --cut "abs(true_ele_slopeY)>0.01"

Errors are binomial (Clopper-Pearson) via TEfficiency.
"""
import argparse
import os
import re
import sys

import ROOT

ROOT.gROOT.SetBatch(ROOT.kTRUE)
ROOT.gStyle.SetOptStat(0)
ROOT.gStyle.SetPadTickX(1)
ROOT.gStyle.SetPadTickY(1)
ROOT.gErrorIgnoreLevel = ROOT.kWarning

TREE_NAME = "truth_vertex_match"
DENOM_CUT = "truth_signal_found"

# name -> (TTree::Draw expression, default xmin, default xmax, default nbins, axis title).
# --xmin/--xmax/--nbins on the command line override these defaults for EVERY --xvar requested
# in one command; leave them unset to get each variable's own sensible default range.
VARIABLES = {
    "z": dict(expr="true_vtx_z", xmin=-10.0, xmax=200.0, nbins=40,
              xlabel="true A' decay-vertex z [mm]"),
    "ap_p": dict(expr="true_ap_p.Mag()", xmin=0.0, xmax=5.0, nbins=40,
                 xlabel="true A' momentum p [GeV]"),
    "ele_p": dict(expr="true_ele_p.Mag()", xmin=0.0, xmax=5.0, nbins=40,
                  xlabel="true e^{-} momentum p [GeV]"),
    "pos_p": dict(expr="true_pos_p.Mag()", xmin=0.0, xmax=5.0, nbins=40,
                  xlabel="true e^{+} momentum p [GeV]"),
    "ele_tanL": dict(expr="true_ele_slopeY", xmin=-0.1, xmax=0.1, nbins=40,
                      xlabel="true e^{-} tan#Lambda proxy (p_{y}/p_{z})"),
    "pos_tanL": dict(expr="true_pos_slopeY", xmin=-0.1, xmax=0.1, nbins=40,
                      xlabel="true e^{+} tan#Lambda proxy (p_{y}/p_{z})"),
    "ele_phi0": dict(expr="true_ele_slopeX", xmin=-0.2, xmax=0.2, nbins=40,
                      xlabel="true e^{-} #phi_{0} proxy (p_{x}/p_{z})"),
    "pos_phi0": dict(expr="true_pos_slopeX", xmin=-0.2, xmax=0.2, nbins=40,
                      xlabel="true e^{+} #phi_{0} proxy (p_{x}/p_{z})"),
}


def make_efficiency(tree, expr, num_cut, nbins, xmin, xmax, name, denom_cut=DENOM_CUT):
    h_den = ROOT.TH1D(f"h_den_{name}", "", nbins, xmin, xmax)
    h_num = ROOT.TH1D(f"h_num_{name}", "", nbins, xmin, xmax)
    tree.Draw(f"{expr} >> h_den_{name}", denom_cut, "goff")
    tree.Draw(f"{expr} >> h_num_{name}", f"({denom_cut}) && ({num_cut})", "goff")
    eff = ROOT.TEfficiency(h_num, h_den)
    eff.SetStatisticOption(ROOT.TEfficiency.kFCP)
    eff.SetName(f"eff_{name}")
    return eff, h_den, h_num


def parse_mass_label(path):
    """Auto-derive a '<m symbol> = <N> MeV' label from an input path, or None if not parseable.

    Looks for a "<number>MeV" pattern anywhere in the basename (e.g. "ap100MeV",
    "simp90MeV-merged"), which is how the mass-point MC lists/ntuples in mc_lists/pass5 and
    run_v0eff_vs_z.sh's output naming are already conventionally named.
    """
    base = os.path.basename(path)
    m = re.search(r"(\d+(?:\.\d+)?)\s*MeV", base, re.IGNORECASE)
    if not m:
        return None
    symbol = "m_{V}" if "simp" in base.lower() else "m_{A'}"
    return f"{symbol} = {m.group(1)} MeV"


def output_tag(input_path):
    """Filename-safe tag for this input, so plots for different mass points don't collide."""
    base = os.path.splitext(os.path.basename(input_path))[0]
    prefix = "truth_vertex_match_"
    if base.startswith(prefix):
        base = base[len(prefix):]
    return base


def print_table(h_den, effs, labels):
    header = f"{'x_lo':>8} {'x_hi':>8} {'n_truth':>8}"
    for lbl in labels:
        header += f" {lbl:>14}"
    print(header)
    nbins = h_den.GetNbinsX()
    for b in range(1, nbins + 1):
        x_lo = h_den.GetXaxis().GetBinLowEdge(b)
        x_hi = h_den.GetXaxis().GetBinUpEdge(b)
        n_den = h_den.GetBinContent(b)
        row = f"{x_lo:8.3f} {x_hi:8.3f} {n_den:8.0f}"
        for eff in effs:
            row += f" {eff.GetEfficiency(b):14.3f}"
        print(row)


def draw_plot(effs, colors, markers, legend_labels, xmin, xmax, outpath, mass_label, canvas_name, xtitle):
    """Render one canvas of TEfficiency curves (frame + legend + labels) and save it to outpath."""
    for eff, color, marker in zip(effs, colors, markers):
        eff.SetLineColor(color)
        eff.SetMarkerColor(color)
        eff.SetMarkerStyle(marker)

    c = ROOT.TCanvas(canvas_name, canvas_name, 800, 600)
    c.SetTickx(1)
    c.SetTicky(1)
    c.SetLeftMargin(0.13)
    c.SetBottomMargin(0.12)
    frame = ROOT.TH1D(f"frame_{canvas_name}", f";{xtitle};v0-finding efficiency", 1, xmin, xmax)
    frame.SetMinimum(0.0)
    # Headroom above 1.0 so the legend (parked near the top of the pad) doesn't sit on top of
    # points/error bars near the physical efficiency ceiling of 1.0.
    frame.SetMaximum(1.75)
    frame.GetXaxis().SetTitleSize(0.05)
    frame.GetXaxis().SetLabelSize(0.045)
    frame.GetYaxis().SetTitleSize(0.05)
    frame.GetYaxis().SetLabelSize(0.045)
    frame.GetYaxis().SetTitleOffset(1.2)
    frame.Draw()

    for eff in effs:
        eff.Draw("pe same")

    legend = ROOT.TLegend(0.35, 0.85 - 0.05 * len(effs), 0.88, 0.88)
    for eff, lbl in zip(effs, legend_labels):
        legend.AddEntry(eff, lbl, "pe")
    legend.Draw()

    latex = ROOT.TLatex()
    latex.SetNDC()
    latex.SetTextAlign(31)
    latex.SetTextFont(42)
    latex.SetTextSize(0.04)
    latex.DrawLatex(0.9, 0.91, "HPS Work in Progress")

    if mass_label:
        mass_latex = ROOT.TLatex()
        mass_latex.SetNDC()
        mass_latex.SetTextAlign(11)
        mass_latex.SetTextFont(42)
        mass_latex.SetTextSize(0.04)
        mass_latex.DrawLatex(0.13, 0.91, mass_label)

    c.SaveAs(outpath)
    print(f"Wrote {outpath}")


def plot_variable(tree, var_key, args, tag, mass_label, n_truth, base_cut=DENOM_CUT):
    cfg = VARIABLES[var_key]
    expr = cfg["expr"]
    nbins = args.nbins if args.nbins is not None else cfg["nbins"]
    xmin = args.xmin if args.xmin is not None else cfg["xmin"]
    xmax = args.xmax if args.xmax is not None else cfg["xmax"]
    xlabel = cfg["xlabel"]

    print(f"\n===== --xvar {var_key} ({expr}, range [{xmin}, {xmax}], {nbins} bins) =====")

    # ===================== main plot: unconditioned efficiencies =====================
    eff_hasv0, h_den, _ = make_efficiency(tree, expr, "has_v0", nbins, xmin, xmax, f"hasv0_{var_key}",
                                           denom_cut=base_cut)
    eff_matched_trk, _, _ = make_efficiency(tree, expr, "matched_trk", nbins, xmin, xmax, f"matchedtrk_{var_key}",
                                             denom_cut=base_cut)
    eff_trk_ele, _, _ = make_efficiency(tree, expr, "matched_trk_ele", nbins, xmin, xmax, f"trkele_{var_key}",
                                         denom_cut=base_cut)
    eff_trk_pos, _, _ = make_efficiency(tree, expr, "matched_trk_pos", nbins, xmin, xmax, f"trkpos_{var_key}",
                                         denom_cut=base_cut)
    eff_trk_eleclus, _, _ = make_efficiency(tree, expr, "matched_trk_clus_ele_only", nbins, xmin, xmax,
                                             f"trkeleclus_{var_key}", denom_cut=base_cut)

    effs = [eff_hasv0, eff_matched_trk, eff_trk_ele, eff_trk_pos, eff_trk_eleclus]
    labels = ["has_v0", "matched_trk", "matched_trk_ele", "matched_trk_pos", "matched_trk_clus_ele_only"]
    legend_labels = ["#geq1 reco v0 (no selection)",
                      "v0 track-matched (both e^{-} and e^{+})",
                      "v0 has truth e^{-} (any e^{+})",
                      "v0 has truth e^{+} (any e^{-})",
                      "e^{-}/e^{+} tracks + e^{-} ECal cluster"]
    colors = [ROOT.kBlue + 1, ROOT.kGreen + 2, ROOT.kOrange + 1, ROOT.kCyan + 2, ROOT.kMagenta + 1]
    markers = [20, 22, 26, 32, 23]

    outname = f"v0eff_vs_{var_key}_{tag}.png" if tag else f"v0eff_vs_{var_key}.png"
    outpath = os.path.join(args.output, outname)
    draw_plot(effs, colors, markers, legend_labels, xmin, xmax, outpath, mass_label,
              f"c_v0eff_vs_{var_key}", xlabel)
    print_table(h_den, effs, labels)

    # ===================== per-leg truth-hit comparison plots =====================
    # Each plot has exactly two curves so the effect of the hit requirement on ONE leg is isolated
    # (the other leg is left unconstrained in both curves).
    n_min = args.minTruthHits
    if not tree.GetBranch("has_truth_hits"):
        print(f"\nNote ({var_key}): input has no 'has_truth_hits' branch (produced before "
              "truth-hit support was added); skipping the per-leg truth-hit comparison plots.",
              file=sys.stderr)
        return
    n_with_hits = tree.GetEntries(f"({base_cut}) && has_truth_hits")
    if n_with_hits == 0:
        print(f"\nNote ({var_key}): input has 'has_truth_hits' but no events with truth-hit info "
              "populated (recoTuple_cfg.py was likely run without -sh/--truthHits 1); skipping "
              "the per-leg truth-hit comparison plots.", file=sys.stderr)
        return

    print(f"\ntruth-hit info available for {n_with_hits}/{n_truth} truth events "
          f"(>= {n_min} unique layers required below):")
    leg_config = [
        ("pos", "matched_trk_pos", "n_truth_layers_pos", eff_trk_pos,
         "v0 has truth e^{+} (any e^{-})", ROOT.kCyan + 2, 32),
        ("ele", "matched_trk_ele", "n_truth_layers_ele", eff_trk_ele,
         "v0 has truth e^{-} (any e^{+})", ROOT.kOrange + 1, 26),
    ]
    for leg, flag, hit_branch, baseline_eff, baseline_lbl, base_color, base_marker in leg_config:
        hits_cut = f"has_truth_hits && {hit_branch}>={n_min}"
        leg_denom_cut = f"({base_cut}) && ({hits_cut})"
        n_pass = tree.GetEntries(leg_denom_cut)
        print(f"  {flag}, {hit_branch}>={n_min}: {n_pass}/{n_with_hits} pass")

        eff_cond, h_den_leg, _ = make_efficiency(tree, expr, flag, nbins, xmin, xmax,
                                                  f"{flag}_{var_key}_min{n_min}", denom_cut=leg_denom_cut)

        leg_effs = [baseline_eff, eff_cond]
        leg_labels = [flag, f"{flag}_min{n_min}layers"]
        leg_legend = [baseline_lbl, f"same, + this leg #geq{n_min} truth layers"]
        leg_colors = [base_color, ROOT.kRed + 1]
        leg_markers = [base_marker, 34]

        leg_outname = (f"v0eff_vs_{var_key}_{leg}_truthhits_{tag}.png" if tag
                        else f"v0eff_vs_{var_key}_{leg}_truthhits.png")
        leg_outpath = os.path.join(args.output, leg_outname)
        draw_plot(leg_effs, leg_colors, leg_markers, leg_legend, xmin, xmax, leg_outpath,
                  mass_label, f"c_v0eff_vs_{var_key}_{leg}_truthhits", xlabel)
        print_table(h_den_leg, leg_effs, leg_labels)


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("-i", "--input", required=True, help="TruthVertexMatch2021 output ROOT file (merged)")
    parser.add_argument("-o", "--output", default=".", help="Output directory for the plots")
    parser.add_argument("--xvar", nargs="+", default=["z"], choices=sorted(VARIABLES.keys()),
                        help="One or more truth quantities to plot efficiency against (default: z). "
                             "Pass multiple names to produce the full set of plots for each in one "
                             "command, e.g. --xvar z ele_p pos_p ele_tanL.")
    parser.add_argument("--nbins", type=int, default=None,
                        help="Number of bins, applied to every --xvar requested. "
                             "Default: each variable's own default (see VARIABLES).")
    parser.add_argument("--xmin", type=float, default=None,
                        help="Lower edge of the x range, applied to every --xvar requested. "
                             "Default: each variable's own default (see VARIABLES).")
    parser.add_argument("--xmax", type=float, default=None,
                        help="Upper edge of the x range, applied to every --xvar requested. "
                             "Default: each variable's own default (see VARIABLES).")
    parser.add_argument("--mass", default=None,
                        help="Mass label text to draw top-left (e.g. \"m_{A'} = 100 MeV\"). "
                             "If omitted, auto-parsed from the input filename.")
    parser.add_argument("--minTruthHits", type=int, default=6,
                        help="Minimum distinct true SVT layers required on a leg (n_truth_layers_ele "
                             "or n_truth_layers_pos) for the per-leg truth-hit comparison plots "
                             "(default 6). Requires the input to have been produced from a "
                             "recoTuple_cfg.py run with -sh/--truthHits 1; otherwise those plots "
                             "are silently skipped.")
    parser.add_argument("--cut", default=None,
                        help="Extra TTree::Draw selection ANDed onto the denominator (and therefore "
                             "the numerator too, since numerator = denom && num_cut) for every --xvar "
                             "in this command. Useful to drop low-statistics/undefined bins for a "
                             "particular variable, e.g. --xvar ele_tanL --cut \"abs(true_ele_slopeY)>0.01\" "
                             "to exclude the near-zero-tanL region.")
    args = parser.parse_args()

    if not os.path.isfile(args.input):
        print(f"Error: input file not found: {args.input}", file=sys.stderr)
        return 1
    os.makedirs(args.output, exist_ok=True)

    f = ROOT.TFile.Open(args.input)
    tree = f.Get(TREE_NAME)
    if not tree:
        print(f"Error: tree '{TREE_NAME}' not found in {args.input}", file=sys.stderr)
        return 1

    base_cut = f"({DENOM_CUT}) && ({args.cut})" if args.cut else DENOM_CUT
    n_truth = tree.GetEntries(base_cut)
    print(f"truth-signal events: {n_truth}" + (f"  (after --cut '{args.cut}')" if args.cut else ""))
    if n_truth == 0:
        print("No truth-signal events found; nothing to plot.", file=sys.stderr)
        return 1

    mass_label = args.mass or parse_mass_label(args.input)
    if not mass_label:
        print(f"Warning: could not auto-parse a mass from '{args.input}'; "
              "pass --mass to label the plot.", file=sys.stderr)
    tag = output_tag(args.input)

    for var_key in args.xvar:
        plot_variable(tree, var_key, args, tag, mass_label, n_truth, base_cut)

    f.Close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
