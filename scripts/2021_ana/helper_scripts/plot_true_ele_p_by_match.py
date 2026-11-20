#!/usr/bin/env python3
"""Compare truth e-/e+ momentum between "one leg only" and "both legs" v0-matched events.

Reads the "truth_vertex_match" tree produced by TruthVertexMatch2021 (see
processors/src/TruthVertexMatch2021.cxx, and run_v0eff_vs_z.sh in this directory for how to
produce a merged ntuple) and, for BOTH the electron and the positron, plots the truth momentum
for two disjoint event categories:

  - "e+ only"   : matched_trk_pos && !matched_trk_ele
                  the true e+ is in some v0, but the true e- is in NO v0 anywhere in the event
                  (a true tracking/vertexing failure for the electron leg, not just a mismatch
                  onto the wrong v0 -- see the vertex-multiplicity note in plot_v0eff_vs_z.py).
  - "e+ and e-" : matched_trk
                  some v0 correctly carries BOTH the true e+ and the true e-.

The categories are the same regardless of which particle's momentum is being plotted -- e.g. the
positron-momentum plots ask "given the electron leg failed everywhere (or didn't), what was the
*positron's* truth momentum", which probes whether the electron-matching failure correlates with
overall event kinematics (e.g. how the beam energy split between the two tracks) rather than just
the electron's own momentum.

For each particle this makes:
  1. A 1D truth-momentum shape comparison (normalized to unit area by default; --no-normalize
     for raw yields).
  2. A 2D truth momentum-vs-tanLambda plot (tanLambda approximated by p_y/p_z, see the truth
     block in TruthVertexMatch2021.cxx) as a SEPARATE plot per category -- the acceptance-shape
     check: is the "leg missing" population concentrated at low p and/or large |tanLambda|
     (edge-of-acceptance), as opposed to the well-matched population?

Also makes a 2D true e- momentum vs true e+ momentum plot, again as a SEPARATE plot per category
-- this tests whether matching success tracks how the beam energy splits between the two tracks
(e.g. does the electron leg fail specifically when the positron carries most of the momentum),
rather than either particle's momentum in isolation.

Also makes a truth electron momentum plot for the "e+ and e-" (bothMatched) category alone,
overlaying the as-is curve with increasingly tight minimum-tracker-hit-count cuts on BOTH legs (up
to 14, the max for a full 7-layer Kalman track) -- shows whether track-quality cuts preferentially
remove low- or high-momentum truth electrons from the matched sample. (posOnly is not applicable
here: it has no track pair, so the hit-count branches are never filled for it.)

Also makes reco track-momentum vs ECal-cluster-energy (E/p-style) plots for all 4 track x cluster
leg combinations, under the STRICT (both legs' clusters required), RELAXED (only the e+ cluster
required, matching the singles-trigger premise), and !STRICT (RELAXED but NOT STRICT -- e+ cluster
present, e- cluster absent; exactly the "gain" population) categories from TruthVertexMatch2021 --
see its finalize() for the headline acceptance-gain number. STRICT and !STRICT partition RELAXED.

Also makes a reco e- track momentum vs reco e+ track momentum plot under those same three
categories -- the reco-level analogue of the truth p_ele-vs-p_pos plot above.

For every one of those category 2D plots, also makes STRICT/RELAXED and !STRICT/RELAXED ratio
plots (via TEfficiency, since both numerators are proper subsets of RELAXED -- genuine binomial
numerator/denominator pairs) showing, bin by bin, what fraction of the RELAXED population does (or
does not) also pass the stricter e- cluster requirement. Low-ratio regions in the STRICT/RELAXED
map (equivalently, high-ratio regions in the !STRICT/RELAXED map) are where relaxing the e- cluster
requirement buys the most acceptance.
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

# particle key -> (momentum expr, tanLambda expr, truth-E branch for the sentinel guard, label)
PARTICLES = {
    "ele": ("true_ele_p.Mag()", "true_ele_slopeY", "true_ele_E", "e^{-}"),
    "pos": ("true_pos_p.Mag()", "true_pos_slopeY", "true_pos_E", "e^{+}"),
}
# category tag -> (cut, legend/title template with {n} for the event count)
CATEGORIES = [
    ("posOnly", "matched_trk_pos && !matched_trk_ele", "e^{{+}} only, no e^{{-}} in any v0 ({n})"),
    ("bothMatched", "matched_trk", "e^{{+}} and e^{{-}} matched, same v0 ({n})"),
]


def parse_mass_label(path):
    """Auto-derive a '<m symbol> = <N> MeV' label from an input path, or None if not parseable."""
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


def make_hist(tree, var, cut, nbins, pmax, name):
    h = ROOT.TH1D(name, "", nbins, 0.0, pmax)
    tree.Draw(f"{var} >> {name}", cut, "goff")
    return h


def make_th2(tree, xvar, yvar, cut, nbinsx, xmin, xmax, nbinsy, ymin, ymax, name):
    h = ROOT.TH2D(name, "", nbinsx, xmin, xmax, nbinsy, ymin, ymax)
    # TTree::Draw("Y:X") fills X on the histogram's x-axis, Y on its y-axis.
    tree.Draw(f"{yvar}:{xvar} >> {name}", cut, "goff")
    return h


def make_th2_p_vs_tanl(tree, var, var_tanl, cut, nbins_p, pmax, nbins_tanl, tanlmin, tanlmax, name):
    return make_th2(tree, var, var_tanl, cut, nbins_p, 0.0, pmax, nbins_tanl, tanlmin, tanlmax, name)


def draw_corner_labels(mass_label, input_path):
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
    else:
        print(f"Warning: could not auto-parse a mass from '{input_path}'; "
              "pass --mass to label the plot.", file=sys.stderr)


def plot_cluster_ratio(h_num, h_den, num_label, den_label, x_label, y_label, canvas_tag, mass_label, args, outpath):
    """<num_label>/<den_label> ratio map from two same-binning TH2s, via TEfficiency. Only valid
    when num_label's selection is a proper subset of den_label's (a binomial numerator/denominator
    pair) -- true for both STRICT/RELAXED and !STRICT/RELAXED here, since STRICT and !STRICT
    (matched_trk_clus_pos_only && !matched_trk_clus_both) partition RELAXED exactly.
    """
    n_num = int(h_num.GetEntries())
    n_den = int(h_den.GetEntries())
    if n_den == 0:
        print(f"[ratio] skipping '{canvas_tag}': {den_label} sample is empty.", file=sys.stderr)
        return

    eff = ROOT.TEfficiency(h_num, h_den)
    eff.SetStatisticOption(ROOT.TEfficiency.kFCP)
    eff.SetTitle(f";{x_label};{y_label}")

    c = ROOT.TCanvas(f"c_ratio_{canvas_tag}", f"{num_label} / {den_label}", 800, 600)
    c.SetTickx(1)
    c.SetTicky(1)
    c.SetLeftMargin(0.13)
    c.SetRightMargin(0.15)
    c.SetBottomMargin(0.12)
    eff.Draw("colz")
    ROOT.gPad.Update()  # TEfficiency only builds its painted histogram after the first Draw

    h_paint = eff.GetPaintedHistogram()
    if h_paint:
        h_paint.SetMinimum(0.0)
        h_paint.SetMaximum(1.0)
        h_paint.GetXaxis().SetTitleSize(0.05)
        h_paint.GetXaxis().SetLabelSize(0.045)
        h_paint.GetYaxis().SetTitleSize(0.05)
        h_paint.GetYaxis().SetLabelSize(0.045)
        h_paint.GetYaxis().SetTitleOffset(1.2)

    draw_corner_labels(mass_label, args.input)

    title_latex = ROOT.TLatex()
    title_latex.SetNDC()
    title_latex.SetTextAlign(13)
    title_latex.SetTextFont(42)
    title_latex.SetTextSize(0.035)
    title_latex.DrawLatex(0.16, 0.86, f"{num_label} / {den_label}  (N_{{num}}={n_num}, N_{{den}}={n_den})")

    c.SaveAs(outpath)
    print(f"Wrote {outpath}")


def plot_1d(tree, particle_key, var, e_branch, plabel, args, mass_label, tag):
    base_cut = f"truth_signal_found && {e_branch} > -9000"
    hists, counts = [], []
    for cat_tag, cut, _ in CATEGORIES:
        h = make_hist(tree, var, f"({base_cut}) && ({cut})", args.nbins, args.pmax, f"h_{particle_key}_{cat_tag}")
        hists.append(h)
        counts.append(int(h.GetEntries()))

    print(f"[{particle_key}] {CATEGORIES[0][2].format(n=counts[0])}")
    print(f"[{particle_key}] {CATEGORIES[1][2].format(n=counts[1])}")
    if sum(counts) == 0:
        print(f"[{particle_key}] both categories empty; skipping 1D plot.", file=sys.stderr)
        return

    colors = [ROOT.kRed + 1, ROOT.kBlue + 1]
    for h, color in zip(hists, colors):
        h.SetLineColor(color)
        h.SetLineWidth(2)

    if args.normalize:
        for h in hists:
            if h.Integral() > 0:
                h.Scale(1.0 / h.Integral())
        y_title = "Fraction of events / bin"
    else:
        y_title = "Events / bin"

    c = ROOT.TCanvas(f"c_true_{particle_key}_p_by_match", "truth momentum by match category", 800, 600)
    c.SetTickx(1)
    c.SetTicky(1)
    c.SetLeftMargin(0.13)
    c.SetBottomMargin(0.12)

    ymax = 1.3 * max(h.GetMaximum() for h in hists)
    frame = ROOT.TH1D("frame", f";true {plabel} momentum [GeV];{y_title}", 1, 0.0, args.pmax)
    frame.SetMinimum(0.0)
    frame.SetMaximum(ymax if ymax > 0 else 1.0)
    frame.GetXaxis().SetTitleSize(0.05)
    frame.GetXaxis().SetLabelSize(0.045)
    frame.GetYaxis().SetTitleSize(0.05)
    frame.GetYaxis().SetLabelSize(0.045)
    frame.GetYaxis().SetTitleOffset(1.2)
    frame.Draw()

    for h in hists:
        h.Draw("hist same")

    legend = ROOT.TLegend(0.45, 0.72, 0.88, 0.88)
    for h, (_, _, title_tmpl), n in zip(hists, CATEGORIES, counts):
        legend.AddEntry(h, title_tmpl.format(n=n), "l")
    legend.Draw()

    draw_corner_labels(mass_label, args.input)

    outname = f"true_{particle_key}_p_by_match_{tag}.png" if tag else f"true_{particle_key}_p_by_match.png"
    outpath = os.path.join(args.output, outname)
    c.SaveAs(outpath)
    print(f"Wrote {outpath}")


# Minimum tracker-hit-count thresholds (None = no cut, i.e. the plain "bothMatched" baseline) to
# overlay on the truth electron momentum, requiring BOTH legs' Track::getTrackerHitCount() >= N.
# 14 is the max for a full 7-layer Kalman track (see TruthVertexMatch2021.cxx's ele_trk_nhits).
NHITS_THRESHOLDS = [None, 6, 8, 10, 12, 14]
NHITS_COLORS = [ROOT.kBlack, ROOT.kBlue + 1, ROOT.kAzure + 1, ROOT.kGreen + 2, ROOT.kOrange + 1, ROOT.kRed + 1]


def plot_1d_ele_p_by_nhits(tree, args, mass_label, tag):
    """Truth electron momentum for the "e+ and e-" (bothMatched) category, as-is and with
    increasingly tight minimum-hit-count requirements on BOTH legs' tracks, up to 14 hits.

    Only bothMatched is meaningful here: posOnly events have no track pair (matched_trk is false
    by construction there), so ele_trk_nhits/pos_trk_nhits are always the sentinel/0 for them.
    Shows whether track-quality (hit-completeness) cuts preferentially remove low- or high-momentum
    truth electrons from the matched sample.
    """
    base_cut = "truth_signal_found && true_ele_E > -9000 && matched_trk"
    hists, labels, counts = [], [], []
    for nhits_min in NHITS_THRESHOLDS:
        if nhits_min is None:
            cut = base_cut
            label = "no hit cut"
            tag_n = "noCut"
        else:
            cut = f"({base_cut}) && (ele_trk_nhits >= {nhits_min}) && (pos_trk_nhits >= {nhits_min})"
            label = f"#geq{nhits_min} hits (both legs)"
            tag_n = f"nhits{nhits_min}"
        h = make_hist(tree, "true_ele_p.Mag()", cut, args.nbins, args.pmax, f"h_ele_p_nhits_{tag_n}")
        n = int(h.GetEntries())
        hists.append(h)
        labels.append(f"{label} ({n})")
        counts.append(n)
        print(f"[nhits] {label}: {n} events")

    if counts[0] == 0:
        print("[nhits] baseline (no hit cut) sample is empty; skipping plot.", file=sys.stderr)
        return

    for h, color in zip(hists, NHITS_COLORS):
        h.SetLineColor(color)
        h.SetLineWidth(2)

    if args.normalize:
        for h in hists:
            if h.Integral() > 0:
                h.Scale(1.0 / h.Integral())
        y_title = "Fraction of events / bin"
    else:
        y_title = "Events / bin"

    c = ROOT.TCanvas("c_true_ele_p_by_nhits", "truth electron momentum by hit-count cut", 800, 600)
    c.SetTickx(1)
    c.SetTicky(1)
    c.SetLeftMargin(0.13)
    c.SetBottomMargin(0.12)

    ymax = 1.3 * max(h.GetMaximum() for h in hists)
    frame = ROOT.TH1D("frame", f";true e^{{-}} momentum [GeV];{y_title}", 1, 0.0, args.pmax)
    frame.SetMinimum(0.0)
    frame.SetMaximum(ymax if ymax > 0 else 1.0)
    frame.GetXaxis().SetTitleSize(0.05)
    frame.GetXaxis().SetLabelSize(0.045)
    frame.GetYaxis().SetTitleSize(0.05)
    frame.GetYaxis().SetLabelSize(0.045)
    frame.GetYaxis().SetTitleOffset(1.2)
    frame.Draw()

    for h in hists:
        h.Draw("hist same")

    legend = ROOT.TLegend(0.45, 0.85 - 0.045 * len(hists), 0.88, 0.88)
    for h, label in zip(hists, labels):
        legend.AddEntry(h, label, "l")
    legend.Draw()

    draw_corner_labels(mass_label, args.input)

    outname = f"true_ele_p_by_nhits_{tag}.png" if tag else "true_ele_p_by_nhits.png"
    outpath = os.path.join(args.output, outname)
    c.SaveAs(outpath)
    print(f"Wrote {outpath}")


def plot_2d(tree, particle_key, var, var_tanl, e_branch, plabel, args, mass_label, tag):
    base_cut = f"truth_signal_found && {e_branch} > -9000"
    for cat_tag, cut, title_tmpl in CATEGORIES:
        full_cut = f"({base_cut}) && ({cut})"
        n_events = int(tree.GetEntries(full_cut))
        if n_events == 0:
            print(f"[{particle_key}] skipping 2D plot for '{cat_tag}': no events pass the selection.",
                  file=sys.stderr)
            continue

        h2 = make_th2_p_vs_tanl(tree, var, var_tanl, full_cut, args.nbins, args.pmax, args.nbins2d,
                                 args.tanlmin, args.tanlmax, f"h2_{particle_key}_{cat_tag}")

        title = title_tmpl.format(n=n_events)
        c2 = ROOT.TCanvas(f"c_true_{particle_key}_ptanL_{cat_tag}", title, 800, 600)
        c2.SetTickx(1)
        c2.SetTicky(1)
        c2.SetLeftMargin(0.13)
        c2.SetRightMargin(0.15)
        c2.SetBottomMargin(0.12)

        h2.SetTitle(f";true {plabel} momentum [GeV];true {plabel} tan#Lambda (p_{{y}}/p_{{z}})")
        h2.GetXaxis().SetTitleSize(0.05)
        h2.GetXaxis().SetLabelSize(0.045)
        h2.GetYaxis().SetTitleSize(0.05)
        h2.GetYaxis().SetLabelSize(0.045)
        h2.GetYaxis().SetTitleOffset(1.2)
        h2.Draw("colz")

        draw_corner_labels(mass_label, args.input)

        cat_latex = ROOT.TLatex()
        cat_latex.SetNDC()
        cat_latex.SetTextAlign(13)
        cat_latex.SetTextFont(42)
        cat_latex.SetTextSize(0.035)
        cat_latex.DrawLatex(0.16, 0.86, title)

        outname2d = f"true_{particle_key}_ptanL_{cat_tag}_{tag}.png" if tag \
            else f"true_{particle_key}_ptanL_{cat_tag}.png"
        outpath2d = os.path.join(args.output, outname2d)
        c2.SaveAs(outpath2d)
        print(f"Wrote {outpath2d}")


def plot_2d_ele_vs_pos(tree, args, mass_label, tag):
    """True e- momentum vs true e+ momentum, one separate plot per category.

    Tests whether matching success tracks how the beam energy splits between the two tracks,
    rather than either particle's momentum on its own.
    """
    base_cut = "truth_signal_found && true_ele_E > -9000 && true_pos_E > -9000"
    for cat_tag, cut, title_tmpl in CATEGORIES:
        full_cut = f"({base_cut}) && ({cut})"
        n_events = int(tree.GetEntries(full_cut))
        if n_events == 0:
            print(f"[ele_vs_pos] skipping 2D plot for '{cat_tag}': no events pass the selection.",
                  file=sys.stderr)
            continue

        h2 = make_th2(tree, "true_ele_p.Mag()", "true_pos_p.Mag()", full_cut,
                       args.nbins, 0.0, args.pmax, args.nbins, 0.0, args.pmax, f"h2_elevspos_{cat_tag}")

        title = title_tmpl.format(n=n_events)
        c2 = ROOT.TCanvas(f"c_true_ele_vs_pos_p_{cat_tag}", title, 800, 600)
        c2.SetTickx(1)
        c2.SetTicky(1)
        c2.SetLeftMargin(0.13)
        c2.SetRightMargin(0.15)
        c2.SetBottomMargin(0.12)

        h2.SetTitle(";true e^{-} momentum [GeV];true e^{+} momentum [GeV]")
        h2.GetXaxis().SetTitleSize(0.05)
        h2.GetXaxis().SetLabelSize(0.045)
        h2.GetYaxis().SetTitleSize(0.05)
        h2.GetYaxis().SetLabelSize(0.045)
        h2.GetYaxis().SetTitleOffset(1.2)
        h2.Draw("colz")

        draw_corner_labels(mass_label, args.input)

        cat_latex = ROOT.TLatex()
        cat_latex.SetNDC()
        cat_latex.SetTextAlign(13)
        cat_latex.SetTextFont(42)
        cat_latex.SetTextSize(0.035)
        cat_latex.DrawLatex(0.16, 0.86, title)

        outname2d = f"true_ele_vs_pos_p_{cat_tag}_{tag}.png" if tag else f"true_ele_vs_pos_p_{cat_tag}.png"
        outpath2d = os.path.join(args.output, outname2d)
        c2.SaveAs(outpath2d)
        print(f"Wrote {outpath2d}")


# cluster-matching category tag -> (cut, legend/title template with {n} for the event count).
# All are subsets of matched_trk (both tracks truth-matched, see TruthVertexMatch2021.cxx):
# STRICT additionally requires an e- cluster AND an e+ cluster; RELAXED drops the e- cluster
# requirement (the singles-trigger premise -- see finalize()'s acceptance-gain number). !STRICT is
# RELAXED's complement within itself (e+ cluster present, e- cluster absent) -- i.e. exactly the
# "gain" population that RELAXED picks up and STRICT misses. STRICT and !STRICT partition RELAXED.
CLUSTER_CATEGORIES = [
    ("STRICT", "matched_trk_clus_both", "STRICT: both tracks + both clusters ({n})"),
    ("RELAXED", "matched_trk_clus_pos_only", "RELAXED: both tracks + e^{{+}} cluster only ({n})"),
    ("NOTSTRICT", "matched_trk_clus_pos_only && !matched_trk_clus_both",
     "!STRICT: e^{{+}} cluster only, NO e^{{-}} cluster ({n})"),
]
# (numerator tag, denominator tag) pairs to build ratio-of-a-subset-of-RELAXED plots for.
RATIO_PAIRS = [("STRICT", "RELAXED"), ("NOTSTRICT", "RELAXED")]
RATIO_DISPLAY_LABEL = {"STRICT": "STRICT", "RELAXED": "RELAXED", "NOTSTRICT": "!STRICT"}
# (x branch, x label, y branch, y label, guard branch for the y-axis cluster-energy sentinel).
# All 4 track-p x cluster-E leg combinations, including the cross terms (e- track vs e+ cluster
# and vice versa) -- the guard is needed because RELAXED does not require an e- cluster, so
# ele_clus_E can be the -9999 sentinel there unless has_ele_clus is also required.
TRKP_VS_CLUSE_CORRELATIONS = [
    ("ele_trk_p", "reco e^{-} track p [GeV]", "ele_clus_E", "e^{-} cluster E [GeV]", "has_ele_clus"),
    ("ele_trk_p", "reco e^{-} track p [GeV]", "pos_clus_E", "e^{+} cluster E [GeV]", "has_pos_clus"),
    ("pos_trk_p", "reco e^{+} track p [GeV]", "ele_clus_E", "e^{-} cluster E [GeV]", "has_ele_clus"),
    ("pos_trk_p", "reco e^{+} track p [GeV]", "pos_clus_E", "e^{+} cluster E [GeV]", "has_pos_clus"),
]


def plot_2d_trkp_vs_clusE(tree, args, mass_label, tag):
    """Reco track momentum vs ECal cluster energy (E/p-style), all 4 track-x-cluster leg
    combinations, under BOTH the STRICT and RELAXED cluster-matching categories, plus a
    STRICT/RELAXED ratio plot for each combination.
    """
    base_cut = "truth_signal_found"
    for x_branch, x_label, y_branch, y_label, y_guard in TRKP_VS_CLUSE_CORRELATIONS:
        hists_by_cat = {}
        for cat_tag, cat_cut, cat_title_tmpl in CLUSTER_CATEGORIES:
            full_cut = f"({base_cut}) && ({cat_cut}) && ({y_guard})"
            n_events = int(tree.GetEntries(full_cut))
            if n_events == 0:
                print(f"[trkp_vs_clusE] skipping '{x_branch}_vs_{y_branch}' ({cat_tag}): "
                      "no events pass the selection.", file=sys.stderr)
                continue

            name = f"h2_{x_branch}_{y_branch}_{cat_tag}"
            h2 = make_th2(tree, x_branch, y_branch, full_cut,
                           args.nbins2d, 0.0, args.pmax, args.nbins2d, 0.0, args.pmax, name)
            hists_by_cat[cat_tag] = h2

            title = cat_title_tmpl.format(n=n_events)
            c2 = ROOT.TCanvas(f"c_{name}", title, 800, 600)
            c2.SetTickx(1)
            c2.SetTicky(1)
            c2.SetLeftMargin(0.13)
            c2.SetRightMargin(0.15)
            c2.SetBottomMargin(0.12)

            h2.SetTitle(f";{x_label};{y_label}")
            h2.GetXaxis().SetTitleSize(0.05)
            h2.GetXaxis().SetLabelSize(0.045)
            h2.GetYaxis().SetTitleSize(0.05)
            h2.GetYaxis().SetLabelSize(0.045)
            h2.GetYaxis().SetTitleOffset(1.2)
            h2.Draw("colz")

            # E/p == 1 reference line
            line = ROOT.TLine(0.0, 0.0, args.pmax, args.pmax)
            line.SetLineStyle(2)
            line.SetLineColor(ROOT.kGray + 2)
            line.Draw()

            draw_corner_labels(mass_label, args.input)

            cat_latex = ROOT.TLatex()
            cat_latex.SetNDC()
            cat_latex.SetTextAlign(13)
            cat_latex.SetTextFont(42)
            cat_latex.SetTextSize(0.035)
            cat_latex.DrawLatex(0.16, 0.86, title)

            outname2d = f"trkp_vs_clusE_{x_branch}_{y_branch}_{cat_tag}_{tag}.png" if tag \
                else f"trkp_vs_clusE_{x_branch}_{y_branch}_{cat_tag}.png"
            outpath2d = os.path.join(args.output, outname2d)
            c2.SaveAs(outpath2d)
            print(f"Wrote {outpath2d}")

        for num_tag, den_tag in RATIO_PAIRS:
            if num_tag not in hists_by_cat or den_tag not in hists_by_cat:
                continue
            outname_ratio = f"trkp_vs_clusE_{x_branch}_{y_branch}_ratio_{num_tag}_{tag}.png" if tag \
                else f"trkp_vs_clusE_{x_branch}_{y_branch}_ratio_{num_tag}.png"
            plot_cluster_ratio(hists_by_cat[num_tag], hists_by_cat[den_tag],
                                RATIO_DISPLAY_LABEL[num_tag], RATIO_DISPLAY_LABEL[den_tag], x_label, y_label,
                                f"trkp_vs_clusE_{x_branch}_{y_branch}_{num_tag}", mass_label, args,
                                os.path.join(args.output, outname_ratio))


def plot_2d_trkp_ele_vs_pos(tree, args, mass_label, tag):
    """Reco e- track momentum vs reco e+ track momentum, one plot per STRICT/RELAXED category.

    The reco-level analogue of plot_2d_ele_vs_pos's truth p_ele-vs-p_pos plot: same question (does
    matching/cluster success track how the beam energy splits between the two tracks?) but for the
    actual reconstructed tracks of the "best" v0 (see the tiered selection in
    TruthVertexMatch2021::process) under the cluster-matching categories instead of the plain
    track-matching ones.
    """
    base_cut = "truth_signal_found"
    x_label, y_label = "reco e^{-} track p [GeV]", "reco e^{+} track p [GeV]"
    hists_by_cat = {}
    for cat_tag, cat_cut, title_tmpl in CLUSTER_CATEGORIES:
        full_cut = f"({base_cut}) && ({cat_cut})"
        n_events = int(tree.GetEntries(full_cut))
        if n_events == 0:
            print(f"[trkp_ele_vs_pos] skipping 2D plot for '{cat_tag}': no events pass the selection.",
                  file=sys.stderr)
            continue

        h2 = make_th2(tree, "ele_trk_p", "pos_trk_p", full_cut,
                       args.nbins, 0.0, args.pmax, args.nbins, 0.0, args.pmax, f"h2_trkp_elevspos_{cat_tag}")
        hists_by_cat[cat_tag] = h2

        title = title_tmpl.format(n=n_events)
        c2 = ROOT.TCanvas(f"c_trkp_ele_vs_pos_{cat_tag}", title, 800, 600)
        c2.SetTickx(1)
        c2.SetTicky(1)
        c2.SetLeftMargin(0.13)
        c2.SetRightMargin(0.15)
        c2.SetBottomMargin(0.12)

        h2.SetTitle(f";{x_label};{y_label}")
        h2.GetXaxis().SetTitleSize(0.05)
        h2.GetXaxis().SetLabelSize(0.045)
        h2.GetYaxis().SetTitleSize(0.05)
        h2.GetYaxis().SetLabelSize(0.045)
        h2.GetYaxis().SetTitleOffset(1.2)
        h2.Draw("colz")

        draw_corner_labels(mass_label, args.input)

        cat_latex = ROOT.TLatex()
        cat_latex.SetNDC()
        cat_latex.SetTextAlign(13)
        cat_latex.SetTextFont(42)
        cat_latex.SetTextSize(0.035)
        cat_latex.DrawLatex(0.16, 0.86, title)

        outname2d = f"trkp_ele_vs_pos_{cat_tag}_{tag}.png" if tag else f"trkp_ele_vs_pos_{cat_tag}.png"
        outpath2d = os.path.join(args.output, outname2d)
        c2.SaveAs(outpath2d)
        print(f"Wrote {outpath2d}")

    for num_tag, den_tag in RATIO_PAIRS:
        if num_tag not in hists_by_cat or den_tag not in hists_by_cat:
            continue
        outname_ratio = f"trkp_ele_vs_pos_ratio_{num_tag}_{tag}.png" if tag \
            else f"trkp_ele_vs_pos_ratio_{num_tag}.png"
        plot_cluster_ratio(hists_by_cat[num_tag], hists_by_cat[den_tag],
                            RATIO_DISPLAY_LABEL[num_tag], RATIO_DISPLAY_LABEL[den_tag], x_label, y_label,
                            f"trkp_ele_vs_pos_{num_tag}", mass_label, args,
                            os.path.join(args.output, outname_ratio))


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("-i", "--input", required=True, help="TruthVertexMatch2021 output ROOT file (merged)")
    parser.add_argument("-o", "--output", default=".", help="Output directory for the plots")
    parser.add_argument("--nbins", type=int, default=40, help="Number of momentum bins")
    parser.add_argument("--pmax", type=float, default=4.0, help="Upper edge of the momentum range [GeV]")
    parser.add_argument("--no-normalize", dest="normalize", action="store_false",
                        help="Plot raw event counts instead of unit-area-normalized shapes (1D only)")
    parser.add_argument("--nbins2d", type=int, default=50, help="Number of bins per axis for the 2D plots")
    parser.add_argument("--tanlmin", type=float, default=-0.15, help="Lower edge of the tanLambda range")
    parser.add_argument("--tanlmax", type=float, default=0.15, help="Upper edge of the tanLambda range")
    parser.add_argument("--particle", choices=["ele", "pos", "both"], default="both",
                        help="Which truth particle's momentum to plot (default: both)")
    parser.add_argument("--mass", default=None,
                        help="Mass label text to draw top-left (e.g. \"m_{A'} = 100 MeV\"). "
                             "If omitted, auto-parsed from the input filename.")
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

    mass_label = args.mass or parse_mass_label(args.input)
    tag = output_tag(args.input)

    ROOT.gStyle.SetPalette(ROOT.kBird)

    particle_keys = list(PARTICLES.keys()) if args.particle == "both" else [args.particle]
    for particle_key in particle_keys:
        var, var_tanl, e_branch, plabel = PARTICLES[particle_key]
        plot_1d(tree, particle_key, var, e_branch, plabel, args, mass_label, tag)
        plot_2d(tree, particle_key, var, var_tanl, e_branch, plabel, args, mass_label, tag)

    plot_2d_ele_vs_pos(tree, args, mass_label, tag)
    plot_1d_ele_p_by_nhits(tree, args, mass_label, tag)
    plot_2d_trkp_vs_clusE(tree, args, mass_label, tag)
    plot_2d_trkp_ele_vs_pos(tree, args, mass_label, tag)

    f.Close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
