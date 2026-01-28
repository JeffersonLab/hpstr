#!/usr/bin/env python
"""
Compare smeared MC distributions to data for momentum and z0.
Plots inclusive, top, and bottom distributions for both variables.
Optionally include unsmeared MC for comparison.

Usage:
    python compareSmearing.py -d data.root -m smeared_mc.root -o output_dir
    python compareSmearing.py -d data.root -m smeared_mc.root -u unsmeared_mc.root -o output_dir
    python compareSmearing.py -d data.root -m smeared_mc.root -u unsmeared_mc.root --fit -o output_dir

Options:
    --fit           Perform Gaussian fits to histograms and display results
    --fit-falloff   Fraction of max bin for fit range boundary (default: 0.4)
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


def fit_gaussian(hist, color, falloff_fraction=0.4):
    """
    Fit a Gaussian to a histogram using auto-detected range.

    The fit range is determined by finding the maximum bin and then
    searching outward on each side until the bin content falls to
    a specified fraction of the maximum.

    Args:
        hist: ROOT histogram to fit
        color: Color for the fit function
        falloff_fraction: Fraction of max bin content to use as range boundary (default: 0.4)

    Returns:
        Tuple of (fit_function, mean, mean_err, sigma, sigma_err) or None if fit fails
    """
    if not hist or hist.GetEntries() < 10:
        return None

    # Find the maximum bin
    max_bin = hist.GetMaximumBin()
    max_val = hist.GetBinContent(max_bin)
    threshold = falloff_fraction * max_val

    # Search left from max bin until we fall below threshold
    left_bin = max_bin
    for i in range(max_bin - 1, 0, -1):
        if hist.GetBinContent(i) < threshold:
            left_bin = i
            break
    else:
        left_bin = 1  # Reached the first bin

    # Search right from max bin until we fall below threshold
    n_bins = hist.GetNbinsX()
    right_bin = max_bin
    for i in range(max_bin + 1, n_bins + 1):
        if hist.GetBinContent(i) < threshold:
            right_bin = i
            break
    else:
        right_bin = n_bins  # Reached the last bin

    # Convert bins to x values for fit range
    fit_min = hist.GetBinLowEdge(left_bin)
    fit_max = hist.GetBinLowEdge(right_bin) + hist.GetBinWidth(right_bin)

    # Create Gaussian fit function
    fit_name = f"gaus_{hist.GetName()}"
    gaus = ROOT.TF1(fit_name, "gaus", fit_min, fit_max)
    gaus.SetLineColor(color)
    gaus.SetLineWidth(2)
    gaus.SetLineStyle(1)

    # Set initial parameters from histogram statistics
    mean = hist.GetMean()
    rms = hist.GetRMS()
    gaus.SetParameter(0, hist.GetMaximum())  # Amplitude
    gaus.SetParameter(1, mean)                # Mean
    gaus.SetParameter(2, rms)                 # Sigma

    # Perform fit (Q = quiet, R = use function range, S = return result)
    fit_result = hist.Fit(gaus, "QRS")

    if fit_result.Status() != 0:
        print(f"WARNING: Fit failed for {hist.GetName()}")
        return None

    # Extract fit parameters
    fit_mean = gaus.GetParameter(1)
    fit_mean_err = gaus.GetParError(1)
    fit_sigma = gaus.GetParameter(2)
    fit_sigma_err = gaus.GetParError(2)

    return (gaus, fit_mean, fit_mean_err, fit_sigma, fit_sigma_err)


def make_comparison_plot(data_hist, mc_hist, title, xtitle, outname, unsmeared_hist=None, normalize=True, do_fit=False, fit_falloff=0.4):
    """
    Create comparison plot between data, smeared MC, and optionally unsmeared MC.

    Args:
        data_hist: Data histogram
        mc_hist: Smeared MC histogram
        title: Plot title
        xtitle: X-axis title
        outname: Output filename
        unsmeared_hist: Optional unsmeared MC histogram
        normalize: Whether to normalize histograms to unit area
        do_fit: Whether to perform Gaussian fits and display results
        fit_falloff: Fraction of max for fit range boundary (default: 0.4)
    """
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

    # Perform Gaussian fits if requested
    fit_results = {}
    if do_fit:
        fit_data = fit_gaussian(h_data, ROOT.kBlack, fit_falloff)
        fit_mc = fit_gaussian(h_mc, ROOT.kRed, fit_falloff)
        fit_unsmeared = None
        if h_unsmeared:
            fit_unsmeared = fit_gaussian(h_unsmeared, ROOT.kBlue, fit_falloff)

        if fit_data:
            fit_results['data'] = fit_data
            fit_data[0].Draw("SAME")
        if fit_mc:
            fit_results['mc'] = fit_mc
            fit_mc[0].Draw("SAME")
        if fit_unsmeared:
            fit_results['unsmeared'] = fit_unsmeared
            fit_unsmeared[0].Draw("SAME")

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

    # Draw fit results on canvas
    if do_fit and fit_results:
        latex.SetTextSize(0.035)
        y_pos = 0.85

        if 'data' in fit_results:
            _, mean, mean_err, sigma, sigma_err = fit_results['data']
            latex.SetTextColor(ROOT.kBlack)
            latex.DrawLatex(0.15, y_pos, f"Data: #mu = {mean:.4f} #pm {mean_err:.4f}")
            y_pos -= 0.05
            latex.DrawLatex(0.15, y_pos, f"         #sigma = {sigma:.4f} #pm {sigma_err:.4f}")
            y_pos -= 0.06

        if 'mc' in fit_results:
            _, mean, mean_err, sigma, sigma_err = fit_results['mc']
            latex.SetTextColor(ROOT.kRed)
            latex.DrawLatex(0.15, y_pos, f"Smeared: #mu = {mean:.4f} #pm {mean_err:.4f}")
            y_pos -= 0.05
            latex.DrawLatex(0.15, y_pos, f"              #sigma = {sigma:.4f} #pm {sigma_err:.4f}")
            y_pos -= 0.06

        if 'unsmeared' in fit_results:
            _, mean, mean_err, sigma, sigma_err = fit_results['unsmeared']
            latex.SetTextColor(ROOT.kBlue)
            latex.DrawLatex(0.15, y_pos, f"Unsmeared: #mu = {mean:.4f} #pm {mean_err:.4f}")
            y_pos -= 0.05
            latex.DrawLatex(0.15, y_pos, f"                  #sigma = {sigma:.4f} #pm {sigma_err:.4f}")

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
    parser.add_option("--fit", action="store_true", dest="do_fit",
                      help="Perform Gaussian fits and display results", default=False)
    parser.add_option("--fit-falloff", type="float", dest="fit_falloff",
                      help="Fraction of max bin for fit range boundary (default: 0.4)", default=0.4)

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
        make_comparison_plot(h_data, h_mc, title, xtitle, outpath, h_unsmeared,
                             do_fit=options.do_fit, fit_falloff=options.fit_falloff)

    # Make z0 plots
    print("\n=== z0 Comparisons ===")
    for hname, title, xtitle, outname in z0_histos:
        full_hname = f"{coll}/{hname}"
        h_data = get_hist(dataFile, full_hname)
        h_mc = get_hist(mcFile, full_hname)
        h_unsmeared = get_hist(unsmearedFile, full_hname) if unsmearedFile else None
        outpath = os.path.join(options.outdir, f"{outname}.{fmt}")
        make_comparison_plot(h_data, h_mc, title, xtitle, outpath, h_unsmeared,
                             do_fit=options.do_fit, fit_falloff=options.fit_falloff)

    # Close files
    dataFile.Close()
    mcFile.Close()
    if unsmearedFile:
        unsmearedFile.Close()

    print(f"\nAll plots saved to: {options.outdir}")


if __name__ == "__main__":
    main()
