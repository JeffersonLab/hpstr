#!/usr/bin/env python
"""
Compare smeared MC distributions to data for momentum and z0.
Plots inclusive, top, and bottom distributions for both variables.
Optionally include unsmeared MC for comparison.

Usage:
    python compareSmearing.py -d data.root -m smeared_mc.root -o output_dir
    python compareSmearing.py -d data.root -m smeared_mc.root -u unsmeared_mc.root -o output_dir
"""

import os
from optparse import OptionParser

import ROOT
ROOT.gROOT.SetBatch(ROOT.kTRUE)
ROOT.gStyle.SetOptStat(0)


def get_hist(tfile, histname):
    """Get histogram from file and detach from file directory."""
    hist = tfile.Get(histname)
    if not hist:
        print(f"WARNING: Histogram {histname} not found")
        return None
    hist.SetDirectory(0)
    return hist


def get_canvas(name, width=800, height=600):
    """Create a styled canvas."""
    can = ROOT.TCanvas(name, name, width, height)
    can.SetBorderSize(0)
    can.SetFillColor(ROOT.kWhite)
    can.SetTickx()
    can.SetTicky()
    can.SetBottomMargin(0.12)
    can.SetTopMargin(0.08)
    can.SetLeftMargin(0.12)
    can.SetRightMargin(0.05)
    return can


def style_hist(hist, color, marker=20, linewidth=2):
    """Apply styling to histogram."""
    hist.SetMarkerStyle(marker)
    hist.SetMarkerColor(color)
    hist.SetLineColor(color)
    hist.SetLineWidth(linewidth)
    hist.SetMarkerSize(1.0)
    hist.SetTitle("")
    hist.GetXaxis().SetTitleSize(0.05)
    hist.GetXaxis().SetLabelSize(0.04)
    hist.GetYaxis().SetTitleSize(0.05)
    hist.GetYaxis().SetLabelSize(0.04)
    hist.GetYaxis().SetTitleOffset(1.1)


def normalize_hist(hist):
    """Normalize histogram to unit area."""
    if hist and hist.Integral() > 0:
        hist.Scale(1.0 / hist.Integral())


def make_comparison_plot(data_hist, mc_hist, title, xtitle, outname, unsmeared_hist=None, normalize=True):
    """Create comparison plot between data, smeared MC, and optionally unsmeared MC."""
    if not data_hist or not mc_hist:
        print(f"WARNING: Cannot make plot {outname}, missing histogram")
        return

    can = get_canvas(outname)

    # Clone to avoid modifying originals
    h_data = data_hist.Clone(f"{data_hist.GetName()}_clone")
    h_mc = mc_hist.Clone(f"{mc_hist.GetName()}_clone")
    h_unsmeared = None
    if unsmeared_hist:
        h_unsmeared = unsmeared_hist.Clone(f"{unsmeared_hist.GetName()}_clone")

    if normalize:
        normalize_hist(h_data)
        normalize_hist(h_mc)
        if h_unsmeared:
            normalize_hist(h_unsmeared)

    # Style histograms
    style_hist(h_data, ROOT.kBlack, marker=20)
    style_hist(h_mc, ROOT.kRed, marker=21)
    if h_unsmeared:
        style_hist(h_unsmeared, ROOT.kBlue, marker=22)

    # Set axis titles
    h_data.GetXaxis().SetTitle(xtitle)
    ytitle = "Normalized" if normalize else "Entries"
    h_data.GetYaxis().SetTitle(ytitle)

    # Get max for y-axis range
    ymax = max(h_data.GetMaximum(), h_mc.GetMaximum())
    if h_unsmeared:
        ymax = max(ymax, h_unsmeared.GetMaximum())
    h_data.GetYaxis().SetRangeUser(0, ymax * 1.3)

    # Draw
    h_data.Draw("PE")
    h_mc.Draw("PE SAME")
    if h_unsmeared:
        h_unsmeared.Draw("PE SAME")

    # Legend
    leg_y2 = 0.88
    leg_y1 = 0.70 if h_unsmeared else 0.75
    leg = ROOT.TLegend(0.62, leg_y1, 0.92, leg_y2)
    leg.SetBorderSize(0)
    leg.SetFillColor(ROOT.kWhite)
    leg.SetTextSize(0.04)
    leg.AddEntry(h_data, "Data", "PE")
    leg.AddEntry(h_mc, "Smeared MC", "PE")
    if h_unsmeared:
        leg.AddEntry(h_unsmeared, "Unsmeared MC", "PE")
    leg.Draw("SAME")

    # Title
    latex = ROOT.TLatex()
    latex.SetNDC()
    latex.SetTextSize(0.045)
    latex.DrawLatex(0.12, 0.93, title)

    can.SaveAs(outname)
    print(f"Saved: {outname}")


def main():
    parser = OptionParser()
    parser.add_option("-d", "--data", type="string", dest="datafile",
                      help="Input data ROOT file", default="")
    parser.add_option("-m", "--mc", type="string", dest="mcfile",
                      help="Input smeared MC ROOT file", default="")
    parser.add_option("-u", "--unsmeared", type="string", dest="unsmearedfile",
                      help="Input unsmeared MC ROOT file (optional)", default="")
    parser.add_option("-o", "--outdir", type="string", dest="outdir",
                      help="Output directory for plots", default=".")
    parser.add_option("-t", "--track-coll", type="string", dest="trackcoll",
                      help="Track collection name", default="KalmanFullTracks")
    parser.add_option("-f", "--format", type="string", dest="fmt",
                      help="Output format (png, pdf)", default="png")

    (options, args) = parser.parse_args()

    if not options.datafile or not options.mcfile:
        parser.error("Both data (-d) and MC (-m) files are required")

    # Create output directory if needed
    if not os.path.exists(options.outdir):
        os.makedirs(options.outdir)

    # Open files
    dataFile = ROOT.TFile.Open(options.datafile, "READ")
    mcFile = ROOT.TFile.Open(options.mcfile, "READ")
    unsmearedFile = None
    if options.unsmearedfile:
        unsmearedFile = ROOT.TFile.Open(options.unsmearedfile, "READ")

    if not dataFile or dataFile.IsZombie():
        print(f"ERROR: Cannot open data file {options.datafile}")
        return
    if not mcFile or mcFile.IsZombie():
        print(f"ERROR: Cannot open MC file {options.mcfile}")
        return
    if options.unsmearedfile and (not unsmearedFile or unsmearedFile.IsZombie()):
        print(f"ERROR: Cannot open unsmeared MC file {options.unsmearedfile}")
        return

    coll = options.trackcoll
    fmt = options.fmt

    # Define histograms to compare
    # Format: (hist_name, title, xtitle, output_name)
    p_histos = [
        ("psmear_h", "Momentum (Inclusive)", "p [GeV]", "p_inclusive"),
        ("psmear_top_h", "Momentum (Top)", "p [GeV]", "p_top"),
        ("psmear_bot_h", "Momentum (Bottom)", "p [GeV]", "p_bottom"),
    ]

    z0_histos = [
        ("z0smear_h", "z_{0} (Inclusive)", "z_{0} [mm]", "z0_inclusive"),
        ("z0smear_top_h", "z_{0} (Top)", "z_{0} [mm]", "z0_top"),
        ("z0smear_bot_h", "z_{0} (Bottom)", "z_{0} [mm]", "z0_bottom"),
    ]

    # Make momentum plots
    print("\n=== Momentum Comparisons ===")
    for hname, title, xtitle, outname in p_histos:
        full_hname = f"{coll}/{hname}"
        h_data = get_hist(dataFile, full_hname)
        h_mc = get_hist(mcFile, full_hname)
        h_unsmeared = get_hist(unsmearedFile, full_hname) if unsmearedFile else None
        outpath = os.path.join(options.outdir, f"{outname}.{fmt}")
        make_comparison_plot(h_data, h_mc, title, xtitle, outpath, h_unsmeared)

    # Make z0 plots
    print("\n=== z0 Comparisons ===")
    for hname, title, xtitle, outname in z0_histos:
        full_hname = f"{coll}/{hname}"
        h_data = get_hist(dataFile, full_hname)
        h_mc = get_hist(mcFile, full_hname)
        h_unsmeared = get_hist(unsmearedFile, full_hname) if unsmearedFile else None
        outpath = os.path.join(options.outdir, f"{outname}.{fmt}")
        make_comparison_plot(h_data, h_mc, title, xtitle, outpath, h_unsmeared)

    # Close files
    dataFile.Close()
    mcFile.Close()
    if unsmearedFile:
        unsmearedFile.Close()

    print(f"\nAll plots saved to: {options.outdir}")


if __name__ == "__main__":
    main()
