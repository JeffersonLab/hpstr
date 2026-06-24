#!/usr/bin/env python3
import os
import re
import sys
import argparse
import ROOT
ROOT.gROOT.SetBatch(ROOT.kTRUE)
ROOT.gStyle.SetOptStat(0)
ROOT.gErrorIgnoreLevel = ROOT.kWarning  # Suppress info messages from fits

# Variable definitions: branch_name -> (nbins, xmin, xmax, axis_label)
variables = {
    "psum": (50, 0, 6, "|#vec{P}_{e^{-}} + #vec{P}_{e^{+}}| [GeV]"),
    "psum_vtx": (50, 0, 6, "P_{sum} (vtx) [GeV]"),
    "psum_scalar": (50, 0, 6, "P_{e^{-}} + P_{e^{+}} [GeV]"),
    "vertex.invM_": (30, 0, 0.300, "m_{e^{+}e^{-}} [MeV]"),
    "ele.track_.z0_": (50, -0.5, 0.5, "e^{-} z_{0} [mm]"),
    "pos.track_.z0_": (50, -0.5, 0.5, "e^{+} z_{0} [mm]"),
    "sqrt(ele.track_.px_*ele.track_.px_ + ele.track_.py_*ele.track_.py_ + ele.track_.pz_*ele.track_.pz_)": (50, 0, 5, "P_{e^{-}} [GeV]"),
    "sqrt(pos.track_.px_*pos.track_.px_ + pos.track_.py_*pos.track_.py_ + pos.track_.pz_*pos.track_.pz_)": (50, 0, 5, "P_{e^{+}} [GeV]"),
#    "ele_p_smear_ratio": (50, 0, 2, "e^{-} P smear ratio"),
#    "pos_p_smear_ratio": (50, 0, 2, "e^{+} P smear ratio"),
}

tree_name = "preselection"

# Selection cuts for truth link categories
TRUTH_LINK_CUT = "ele_has_truth_link==1 && pos_has_truth_link==1"
PULSER_CUT = "(ele_has_truth_link==0 || pos_has_truth_link==0)"


def sanitize_name(name):
    """Convert branch name to safe filename."""
    return re.sub(r'[^a-zA-Z0-9_]', '_', name).strip('_')


def compute_ratio_range(ratios, coverage=0.8, min_range=(0, 2), max_range=(0, 5)):
    """Compute y-axis range for ratio plot to show at least coverage fraction of points."""
    values = []
    for r in ratios:
        for i in range(1, r.GetNbinsX() + 1):
            val = r.GetBinContent(i)
            if val > 0:
                values.append(val)
    if not values:
        return max_range

    values.sort()
    n = len(values)
    n_keep = int(n * coverage)
    # Find the smallest window containing n_keep points
    best_lo, best_hi = values[0], values[-1]
    for i in range(n - n_keep + 1):
        lo, hi = values[i], values[i + n_keep - 1]
        if (hi - lo) < (best_hi - best_lo):
            best_lo, best_hi = lo, hi

    # Add some padding
    margin = 0.1 * (best_hi - best_lo) if best_hi > best_lo else 0.1
    ymin = max(max_range[0], best_lo - margin)
    ymax = min(max_range[1], best_hi + margin)
    # Ensure at least min_range, but allow extending up to max_range
    ymin = min(ymin, min_range[0])
    ymax = max(ymax, min_range[1])
    return (ymin, ymax)


def load_histogram(filepath, branch, nbins, xmin, xmax, label, hist_name, selection=""):
    """Load a histogram from a ROOT file by drawing from a TTree."""
    f = ROOT.TFile.Open(filepath)
    tree = f.Get(tree_name)

    safe_name = sanitize_name(hist_name)
    tree.Draw(f"{branch} >> {safe_name}({nbins},{xmin},{xmax})", selection, "goff")
    h = ROOT.gDirectory.Get(safe_name)
    h.SetTitle(f";{label};Events")
    h.SetDirectory(0)

    f.Close()
    return h


def main():
    parser = argparse.ArgumentParser(description="Compare smeared and unsmeared distributions")
    parser.add_argument("--unsmeared", "-u", required=True, help="Unsmeared input ROOT file(s), comma-separated for multiple (first=94%%, second=6%%)")
    parser.add_argument("--smeared", "-s", required=True, help="Smeared input ROOT file(s), comma-separated for multiple (first=94%%, second=6%%)")
    parser.add_argument("--data", "-d", required=False, help="Data input ROOT file")
    parser.add_argument("--output", "-o", default="plots", help="Output directory")
    parser.add_argument("--category", "-c", choices=["all", "truth_link", "pulser", "both"], default="all",
                        help="Event category: all (no cut), truth_link (ele&pos have truth link), pulser (either missing truth link), both (overlay truth_link and pulser)")
    args = parser.parse_args()

    # Set selection cut based on category
    if args.category == "truth_link":
        selection_cut = TRUTH_LINK_CUT
        category_label = "Truth Link"
    elif args.category == "pulser":
        selection_cut = PULSER_CUT
        category_label = "Pulser"
    elif args.category == "both":
        selection_cut = None  # Special handling for both categories
        category_label = ""
    else:
        selection_cut = ""
        category_label = ""

    # Parse unsmeared files (comma-separated)
    unsmeared_files = [f.strip() for f in args.unsmeared.split(",")]
    unsmeared_fractions = [0.94, 0.06]  # First file 94%, second file 6%
    unsmeared_labels = ["Tritrig", "WABs"]
    unsmeared_colors = [ROOT.kBlue, ROOT.kCyan + 2]

    # Parse smeared files (comma-separated)
    smeared_files = [f.strip() for f in args.smeared.split(",")]
    smeared_fractions = [0.94, 0.06]  # First file 94%, second file 6%
    smeared_labels = ["Tritrig", "WABs"]
    smeared_colors = [ROOT.kRed, ROOT.kGreen + 2]

    # Check that input files exist
    missing = []
    for path in unsmeared_files:
        if not os.path.isfile(path):
            missing.append(path)
    for path in smeared_files:
        if not os.path.isfile(path):
            missing.append(path)
    if args.data and not os.path.isfile(args.data):
        missing.append(args.data)
    if missing:
        print(f"Error: The following input files do not exist:")
        for path in missing:
            print(f"  {path}")
        return 1

    os.makedirs(args.output, exist_ok=True)

    # Loop over variables
    for branch, (nbins, xmin, xmax, label) in variables.items():
        # Special handling for "both" category - overlay truth_link and pulser for both smeared and unsmeared
        if args.category == "both":
            # Load unsmeared histograms for truth_link and pulser
            h_unsmeared_tl_components = []
            n_unsmeared_tl = 0
            for i, unsmeared_file in enumerate(unsmeared_files):
                h = load_histogram(unsmeared_file, branch, nbins, xmin, xmax, label, f"h_unsmeared_tl_{i}_{branch}", TRUTH_LINK_CUT)
                n_unsmeared_tl += int(h.GetEntries())
                h_unsmeared_tl_components.append(h)

            h_unsmeared_pulser_components = []
            n_unsmeared_pulser = 0
            for i, unsmeared_file in enumerate(unsmeared_files):
                h = load_histogram(unsmeared_file, branch, nbins, xmin, xmax, label, f"h_unsmeared_pulser_{i}_{branch}", PULSER_CUT)
                n_unsmeared_pulser += int(h.GetEntries())
                h_unsmeared_pulser_components.append(h)

            # Load smeared histograms for truth_link and pulser
            h_smeared_tl_components = []
            n_smeared_tl = 0
            for i, smeared_file in enumerate(smeared_files):
                h = load_histogram(smeared_file, branch, nbins, xmin, xmax, label, f"h_smeared_tl_{i}_{branch}", TRUTH_LINK_CUT)
                n_smeared_tl += int(h.GetEntries())
                h_smeared_tl_components.append(h)

            h_smeared_pulser_components = []
            n_smeared_pulser = 0
            for i, smeared_file in enumerate(smeared_files):
                h = load_histogram(smeared_file, branch, nbins, xmin, xmax, label, f"h_smeared_pulser_{i}_{branch}", PULSER_CUT)
                n_smeared_pulser += int(h.GetEntries())
                h_smeared_pulser_components.append(h)

            # Combine unsmeared truth_link components (weighted by fractions)
            if len(unsmeared_files) > 1:
                for i, h in enumerate(h_unsmeared_tl_components):
                    if h.Integral() > 0:
                        h.Scale(1.0 / h.Integral())
                    h.Scale(unsmeared_fractions[i] if i < len(unsmeared_fractions) else 0.0)
                h_unsmeared_tl = h_unsmeared_tl_components[0].Clone(sanitize_name(f"h_unsmeared_tl_combined_{branch}"))
                for h in h_unsmeared_tl_components[1:]:
                    h_unsmeared_tl.Add(h)
            else:
                h_unsmeared_tl = h_unsmeared_tl_components[0]

            # Combine unsmeared pulser components (weighted by fractions)
            if len(unsmeared_files) > 1:
                for i, h in enumerate(h_unsmeared_pulser_components):
                    if h.Integral() > 0:
                        h.Scale(1.0 / h.Integral())
                    h.Scale(unsmeared_fractions[i] if i < len(unsmeared_fractions) else 0.0)
                h_unsmeared_pulser = h_unsmeared_pulser_components[0].Clone(sanitize_name(f"h_unsmeared_pulser_combined_{branch}"))
                for h in h_unsmeared_pulser_components[1:]:
                    h_unsmeared_pulser.Add(h)
            else:
                h_unsmeared_pulser = h_unsmeared_pulser_components[0]

            # Combine smeared truth_link components (weighted by fractions)
            if len(smeared_files) > 1:
                for i, h in enumerate(h_smeared_tl_components):
                    if h.Integral() > 0:
                        h.Scale(1.0 / h.Integral())
                    h.Scale(smeared_fractions[i] if i < len(smeared_fractions) else 0.0)
                h_smeared_tl = h_smeared_tl_components[0].Clone(sanitize_name(f"h_smeared_tl_combined_{branch}"))
                for h in h_smeared_tl_components[1:]:
                    h_smeared_tl.Add(h)
            else:
                h_smeared_tl = h_smeared_tl_components[0]

            # Combine smeared pulser components (weighted by fractions)
            if len(smeared_files) > 1:
                for i, h in enumerate(h_smeared_pulser_components):
                    if h.Integral() > 0:
                        h.Scale(1.0 / h.Integral())
                    h.Scale(smeared_fractions[i] if i < len(smeared_fractions) else 0.0)
                h_smeared_pulser = h_smeared_pulser_components[0].Clone(sanitize_name(f"h_smeared_pulser_combined_{branch}"))
                for h in h_smeared_pulser_components[1:]:
                    h_smeared_pulser.Add(h)
            else:
                h_smeared_pulser = h_smeared_pulser_components[0]

            # Normalize unsmeared so that truth_link + pulser sum to 1, preserving relative fractions
            total_unsmeared = n_unsmeared_tl + n_unsmeared_pulser
            if total_unsmeared > 0:
                frac_unsmeared_tl = n_unsmeared_tl / total_unsmeared
                frac_unsmeared_pulser = n_unsmeared_pulser / total_unsmeared
            else:
                frac_unsmeared_tl = 0.5
                frac_unsmeared_pulser = 0.5

            if h_unsmeared_tl.Integral() > 0:
                h_unsmeared_tl.Scale(frac_unsmeared_tl / h_unsmeared_tl.Integral())
            if h_unsmeared_pulser.Integral() > 0:
                h_unsmeared_pulser.Scale(frac_unsmeared_pulser / h_unsmeared_pulser.Integral())

            # Normalize smeared so that truth_link + pulser sum to 1, preserving relative fractions
            total_smeared = n_smeared_tl + n_smeared_pulser
            if total_smeared > 0:
                frac_smeared_tl = n_smeared_tl / total_smeared
                frac_smeared_pulser = n_smeared_pulser / total_smeared
            else:
                frac_smeared_tl = 0.5
                frac_smeared_pulser = 0.5

            if h_smeared_tl.Integral() > 0:
                h_smeared_tl.Scale(frac_smeared_tl / h_smeared_tl.Integral())
            if h_smeared_pulser.Integral() > 0:
                h_smeared_pulser.Scale(frac_smeared_pulser / h_smeared_pulser.Integral())

            # Style: unsmeared = blue, smeared = red; truth_link = solid, pulser = dashed
            h_unsmeared_tl.SetLineColor(ROOT.kBlue)
            h_unsmeared_tl.SetLineWidth(2)
            h_unsmeared_tl.SetLineStyle(1)
            h_unsmeared_pulser.SetLineColor(ROOT.kBlue)
            h_unsmeared_pulser.SetLineWidth(2)
            h_unsmeared_pulser.SetLineStyle(2)
            h_smeared_tl.SetLineColor(ROOT.kRed)
            h_smeared_tl.SetLineWidth(2)
            h_smeared_tl.SetLineStyle(1)
            h_smeared_pulser.SetLineColor(ROOT.kRed)
            h_smeared_pulser.SetLineWidth(2)
            h_smeared_pulser.SetLineStyle(2)

            # Load data if provided
            h_data = None
            n_data = 0
            if args.data:
                h_data = load_histogram(args.data, branch, nbins, xmin, xmax, label, f"h_data_{branch}", "")
                h_data.SetMarkerColor(ROOT.kBlack)
                h_data.SetMarkerStyle(20)
                h_data.SetLineColor(ROOT.kBlack)
                n_data = int(h_data.GetEntries())
                if h_data.Integral() > 0:
                    h_data.Scale(1.0 / h_data.Integral())

            # Set Y-axis range
            ymax = max(h_unsmeared_tl.GetMaximum(), h_unsmeared_pulser.GetMaximum(),
                       h_smeared_tl.GetMaximum(), h_smeared_pulser.GetMaximum())
            if h_data:
                ymax = max(ymax, h_data.GetMaximum())
            ymax *= 1.2
            h_unsmeared_tl.SetMaximum(ymax)
            h_unsmeared_pulser.SetMaximum(ymax)
            h_smeared_tl.SetMaximum(ymax)
            h_smeared_pulser.SetMaximum(ymax)
            if h_data:
                h_data.SetMaximum(ymax)

            # Create canvas
            c = ROOT.TCanvas(sanitize_name(f"c_{branch}"), "Truth Link vs Pulser", 800, 800 if h_data else 600)

            if h_data:
                # Upper pad for distributions
                pad1 = ROOT.TPad("pad1", "pad1", 0, 0.3, 1, 1.0)
                pad1.SetBottomMargin(0.02)
                pad1.Draw()
                pad1.cd()

                h_data.GetXaxis().SetLabelSize(0)
                h_data.GetXaxis().SetTitleSize(0)
                h_data.Draw("pe")
                h_unsmeared_tl.Draw("hist same")
                h_unsmeared_pulser.Draw("hist same")
                h_smeared_tl.Draw("hist same")
                h_smeared_pulser.Draw("hist same")
                h_data.Draw("pe same")

                # Legend
                legend = ROOT.TLegend(0.55, 0.60, 0.9, 0.85)
                legend.AddEntry(h_data, f"Data ({n_data})", "pe")
                legend.AddEntry(h_unsmeared_tl, f"Unsmeared TL ({n_unsmeared_tl}, {frac_unsmeared_tl*100:.1f}%)", "l")
                legend.AddEntry(h_unsmeared_pulser, f"Unsmeared Pulser ({n_unsmeared_pulser}, {frac_unsmeared_pulser*100:.1f}%)", "l")
                legend.AddEntry(h_smeared_tl, f"Smeared TL ({n_smeared_tl}, {frac_smeared_tl*100:.1f}%)", "l")
                legend.AddEntry(h_smeared_pulser, f"Smeared Pulser ({n_smeared_pulser}, {frac_smeared_pulser*100:.1f}%)", "l")
                legend.Draw()

                # Label
                latex = ROOT.TLatex()
                latex.SetNDC()
                latex.SetTextAlign(31)
                latex.SetTextFont(42)
                latex.SetTextSize(0.04)
                latex.DrawLatex(0.9, 0.91, "HPS Work in Progress")

                # Lower pad for ratio
                c.cd()
                pad2 = ROOT.TPad("pad2", "pad2", 0, 0.0, 1, 0.3)
                pad2.SetTopMargin(0.02)
                pad2.SetBottomMargin(0.3)
                pad2.Draw()
                pad2.cd()

                # Compute ratios
                r_unsmeared_tl = h_unsmeared_tl.Clone(sanitize_name(f"r_unsmeared_tl_{branch}"))
                r_unsmeared_pulser = h_unsmeared_pulser.Clone(sanitize_name(f"r_unsmeared_pulser_{branch}"))
                r_smeared_tl = h_smeared_tl.Clone(sanitize_name(f"r_smeared_tl_{branch}"))
                r_smeared_pulser = h_smeared_pulser.Clone(sanitize_name(f"r_smeared_pulser_{branch}"))
                r_unsmeared_tl.Divide(h_data)
                r_unsmeared_pulser.Divide(h_data)
                r_smeared_tl.Divide(h_data)
                r_smeared_pulser.Divide(h_data)

                ymin_r, ymax_r = compute_ratio_range([r_unsmeared_tl, r_unsmeared_pulser, r_smeared_tl, r_smeared_pulser])

                r_unsmeared_tl.SetMarkerStyle(20)
                r_unsmeared_tl.SetMarkerColor(ROOT.kBlue)
                r_unsmeared_tl.SetLineColor(ROOT.kBlue)
                r_unsmeared_pulser.SetMarkerStyle(24)
                r_unsmeared_pulser.SetMarkerColor(ROOT.kBlue)
                r_unsmeared_pulser.SetLineColor(ROOT.kBlue)
                r_smeared_tl.SetMarkerStyle(21)
                r_smeared_tl.SetMarkerColor(ROOT.kRed)
                r_smeared_tl.SetLineColor(ROOT.kRed)
                r_smeared_pulser.SetMarkerStyle(25)
                r_smeared_pulser.SetMarkerColor(ROOT.kRed)
                r_smeared_pulser.SetLineColor(ROOT.kRed)

                r_unsmeared_tl.SetTitle(f";{label};MC / Data")
                r_unsmeared_tl.GetYaxis().SetRangeUser(ymin_r, ymax_r)
                r_unsmeared_tl.GetYaxis().SetNdivisions(505)
                r_unsmeared_tl.GetYaxis().SetTitleSize(0.1)
                r_unsmeared_tl.GetYaxis().SetTitleOffset(0.4)
                r_unsmeared_tl.GetYaxis().SetLabelSize(0.08)
                r_unsmeared_tl.GetXaxis().SetTitleSize(0.12)
                r_unsmeared_tl.GetXaxis().SetLabelSize(0.1)

                r_unsmeared_tl.Draw("pe")
                r_unsmeared_pulser.Draw("pe same")
                r_smeared_tl.Draw("pe same")
                r_smeared_pulser.Draw("pe same")

                line = ROOT.TLine(xmin, 1, xmax, 1)
                line.SetLineStyle(2)
                line.Draw()

            else:
                # No data - simple plot
                h_unsmeared_tl.Draw("hist")
                h_unsmeared_pulser.Draw("hist same")
                h_smeared_tl.Draw("hist same")
                h_smeared_pulser.Draw("hist same")

                legend = ROOT.TLegend(0.55, 0.65, 0.9, 0.85)
                legend.AddEntry(h_unsmeared_tl, f"Unsmeared TL ({n_unsmeared_tl}, {frac_unsmeared_tl*100:.1f}%)", "l")
                legend.AddEntry(h_unsmeared_pulser, f"Unsmeared Pulser ({n_unsmeared_pulser}, {frac_unsmeared_pulser*100:.1f}%)", "l")
                legend.AddEntry(h_smeared_tl, f"Smeared TL ({n_smeared_tl}, {frac_smeared_tl*100:.1f}%)", "l")
                legend.AddEntry(h_smeared_pulser, f"Smeared Pulser ({n_smeared_pulser}, {frac_smeared_pulser*100:.1f}%)", "l")
                legend.Draw()

                latex = ROOT.TLatex()
                latex.SetNDC()
                latex.SetTextAlign(31)
                latex.SetTextFont(42)
                latex.SetTextSize(0.04)
                latex.DrawLatex(0.9, 0.91, "HPS Work in Progress")

            c.SaveAs(f"{args.output}/{sanitize_name(branch)}_both.png")

            del h_unsmeared_tl, h_unsmeared_pulser, h_smeared_tl, h_smeared_pulser, legend, c
            for h in h_unsmeared_tl_components:
                del h
            for h in h_unsmeared_pulser_components:
                del h
            for h in h_smeared_tl_components:
                del h
            for h in h_smeared_pulser_components:
                del h
            if h_data:
                del h_data, r_unsmeared_tl, r_unsmeared_pulser, r_smeared_tl, r_smeared_pulser

            continue  # Skip the rest of the loop for this variable

        # Load unsmeared histograms (one or more)
        h_unsmeared_components = []
        n_unsmeared_components = []
        for i, unsmeared_file in enumerate(unsmeared_files):
            h = load_histogram(unsmeared_file, branch, nbins, xmin, xmax, label, f"h_unsmeared_{i}_{branch}", selection_cut)
            n_unsmeared_components.append(int(h.GetEntries()))
            h_unsmeared_components.append(h)

        # Load smeared histograms (one or more)
        h_smeared_components = []
        n_smeared_components = []
        for i, smeared_file in enumerate(smeared_files):
            h = load_histogram(smeared_file, branch, nbins, xmin, xmax, label, f"h_smeared_{i}_{branch}", selection_cut)
            n_smeared_components.append(int(h.GetEntries()))
            h_smeared_components.append(h)

        h_data = None
        if args.data:
            h_data = load_histogram(args.data, branch, nbins, xmin, xmax, label, f"h_data_{branch}", selection_cut)

        # Style data
        if h_data:
            h_data.SetMarkerColor(ROOT.kBlack)
            h_data.SetMarkerStyle(20)
            h_data.SetLineColor(ROOT.kBlack)

        # Get data yield before normalizing
        n_data = int(h_data.GetEntries()) if h_data else 0

        # Normalize data for shape comparison
        if h_data and h_data.Integral() > 0:
            h_data.Scale(1.0 / h_data.Integral())

        # Normalize and combine unsmeared components
        if len(unsmeared_files) > 1:
            # Multiple unsmeared files: normalize each, scale by fraction, then sum
            for i, h in enumerate(h_unsmeared_components):
                if h.Integral() > 0:
                    h.Scale(1.0 / h.Integral())
                h.Scale(unsmeared_fractions[i] if i < len(unsmeared_fractions) else 0.0)
                h.SetLineColor(unsmeared_colors[i] if i < len(unsmeared_colors) else ROOT.kGray)
                h.SetLineWidth(2)
                h.SetLineStyle(2)  # Dashed for components

            # Create combined histogram
            h_unsmeared = h_unsmeared_components[0].Clone(sanitize_name(f"h_unsmeared_combined_{branch}"))
            for h in h_unsmeared_components[1:]:
                h_unsmeared.Add(h)
            h_unsmeared.SetLineColor(ROOT.kBlue)
            h_unsmeared.SetLineWidth(2)
            h_unsmeared.SetLineStyle(1)  # Solid for combined
            n_unsmeared = sum(n_unsmeared_components)
        else:
            # Single unsmeared file: normalize directly
            h_unsmeared = h_unsmeared_components[0]
            if h_unsmeared.Integral() > 0:
                h_unsmeared.Scale(1.0 / h_unsmeared.Integral())
            h_unsmeared.SetLineColor(ROOT.kBlue)
            h_unsmeared.SetLineWidth(2)
            n_unsmeared = n_unsmeared_components[0]

        # Normalize and combine smeared components
        if len(smeared_files) > 1:
            # Multiple smeared files: normalize each, scale by fraction, then sum
            for i, h in enumerate(h_smeared_components):
                if h.Integral() > 0:
                    h.Scale(1.0 / h.Integral())
                h.Scale(smeared_fractions[i] if i < len(smeared_fractions) else 0.0)
                h.SetLineColor(smeared_colors[i] if i < len(smeared_colors) else ROOT.kGray)
                h.SetLineWidth(2)
                h.SetLineStyle(2)  # Dashed for components

            # Create combined histogram
            h_smeared = h_smeared_components[0].Clone(sanitize_name(f"h_smeared_combined_{branch}"))
            for h in h_smeared_components[1:]:
                h_smeared.Add(h)
            h_smeared.SetLineColor(ROOT.kRed)
            h_smeared.SetLineWidth(2)
            h_smeared.SetLineStyle(1)  # Solid for combined
            n_smeared = sum(n_smeared_components)
        else:
            # Single smeared file: normalize directly
            h_smeared = h_smeared_components[0]
            if h_smeared.Integral() > 0:
                h_smeared.Scale(1.0 / h_smeared.Integral())
            h_smeared.SetLineColor(ROOT.kRed)
            h_smeared.SetLineWidth(2)
            n_smeared = n_smeared_components[0]

        # Set Y-axis range to show all histograms with headroom
        ymax = max(h_unsmeared.GetMaximum(), h_smeared.GetMaximum())
        if h_data:
            ymax = max(ymax, h_data.GetMaximum())
        for h in h_unsmeared_components:
            ymax = max(ymax, h.GetMaximum())
        for h in h_smeared_components:
            ymax = max(ymax, h.GetMaximum())
        ymax *= 1.2
        h_unsmeared.SetMaximum(ymax)
        h_smeared.SetMaximum(ymax)
        for h in h_unsmeared_components:
            h.SetMaximum(ymax)
        for h in h_smeared_components:
            h.SetMaximum(ymax)
        if h_data:
            h_data.SetMaximum(ymax)

        # Create canvas with ratio pad if data exists
        c = ROOT.TCanvas(sanitize_name(f"c_{branch}"), "Smearing Comparison", 800, 800 if h_data else 600)

        if h_data:
            # Upper pad for distributions
            pad1 = ROOT.TPad("pad1", "pad1", 0, 0.3, 1, 1.0)
            pad1.SetBottomMargin(0.02)
            pad1.Draw()
            pad1.cd()

            h_data.GetXaxis().SetLabelSize(0)
            h_data.GetXaxis().SetTitleSize(0)
            h_data.Draw("pe")
            # Draw individual components if multiple unsmeared files
            if len(unsmeared_files) > 1:
                for h in h_unsmeared_components:
                    h.Draw("hist same")
            h_unsmeared.Draw("hist same")
            # Draw individual components if multiple smeared files
            if len(smeared_files) > 1:
                for h in h_smeared_components:
                    h.Draw("hist same")
            h_smeared.Draw("hist same")
            h_data.Draw("pe same")

            # Legend (data first)
            n_legend_entries = 3
            if len(unsmeared_files) > 1:
                n_legend_entries += len(unsmeared_files)
            if len(smeared_files) > 1:
                n_legend_entries += len(smeared_files)
            legend_height = 0.05 * n_legend_entries
            legend = ROOT.TLegend(0.60, 0.85 - legend_height, 0.9, 0.85)
            legend.AddEntry(h_data, f"Data ({n_data})", "pe")
            if len(unsmeared_files) > 1:
                legend.AddEntry(h_unsmeared, f"Unsmeared ({n_unsmeared})", "l")
                for i, h in enumerate(h_unsmeared_components):
                    lbl = unsmeared_labels[i] if i < len(unsmeared_labels) else f"Unsmeared {i+1}"
                    frac = unsmeared_fractions[i] if i < len(unsmeared_fractions) else 0.0
                    legend.AddEntry(h, f"{lbl} ({int(frac*100)}%)", "l")
            else:
                legend.AddEntry(h_unsmeared, f"Unsmeared ({n_unsmeared})", "l")
            if len(smeared_files) > 1:
                legend.AddEntry(h_smeared, f"Smeared ({n_smeared})", "l")
                for i, h in enumerate(h_smeared_components):
                    lbl = smeared_labels[i] if i < len(smeared_labels) else f"Smeared {i+1}"
                    frac = smeared_fractions[i] if i < len(smeared_fractions) else 0.0
                    legend.AddEntry(h, f"{lbl} ({int(frac*100)}%)", "l")
            else:
                legend.AddEntry(h_smeared, f"Smeared ({n_smeared})", "l")
            legend.Draw()

            # Label
            latex = ROOT.TLatex()
            latex.SetNDC()
            latex.SetTextAlign(31)  # Right-aligned
            latex.SetTextFont(42)
            latex.SetTextSize(0.04)
            latex.DrawLatex(0.9, 0.91, "HPS Work in Progress")
            if category_label:
                latex.SetTextSize(0.035)
                latex.DrawLatex(0.9, 0.86, category_label)

            # Lower pad for ratio
            c.cd()
            pad2 = ROOT.TPad("pad2", "pad2", 0, 0.0, 1, 0.3)
            pad2.SetTopMargin(0.02)
            pad2.SetBottomMargin(0.3)
            pad2.Draw()
            pad2.cd()

            # Compute ratios with respect to data
            r_unsmeared = h_unsmeared.Clone(sanitize_name(f"r_unsmeared_{branch}"))
            r_smeared = h_smeared.Clone(sanitize_name(f"r_smeared_{branch}"))
            r_unsmeared.Divide(h_data)
            r_smeared.Divide(h_data)

            # Auto-detect ratio range
            ymin, ymax = compute_ratio_range([r_unsmeared, r_smeared])

            # Style ratio plots as data points with colors matching the MC
            r_unsmeared.SetMarkerStyle(20)
            r_unsmeared.SetMarkerColor(ROOT.kBlue)
            r_unsmeared.SetLineColor(ROOT.kBlue)
            r_smeared.SetMarkerStyle(21)
            r_smeared.SetMarkerColor(ROOT.kRed)
            r_smeared.SetLineColor(ROOT.kRed)

            r_unsmeared.SetTitle(f";{label};MC / Data")
            r_unsmeared.GetYaxis().SetRangeUser(ymin, ymax)
            r_unsmeared.GetYaxis().SetNdivisions(505)
            r_unsmeared.GetYaxis().SetTitleSize(0.1)
            r_unsmeared.GetYaxis().SetTitleOffset(0.4)
            r_unsmeared.GetYaxis().SetLabelSize(0.08)
            r_unsmeared.GetXaxis().SetTitleSize(0.12)
            r_unsmeared.GetXaxis().SetLabelSize(0.1)

            r_unsmeared.Draw("pe")
            r_smeared.Draw("pe same")

            # Draw line at 1
            line = ROOT.TLine(xmin, 1, xmax, 1)
            line.SetLineStyle(2)
            line.Draw()

        else:
            # No data - simple plot without ratio
            # Draw individual components if multiple unsmeared files
            if len(unsmeared_files) > 1:
                for h in h_unsmeared_components:
                    h.Draw("hist same" if h != h_unsmeared_components[0] else "hist")
            h_unsmeared.Draw("hist" if len(unsmeared_files) == 1 else "hist same")
            # Draw individual components if multiple smeared files
            if len(smeared_files) > 1:
                for h in h_smeared_components:
                    h.Draw("hist same")
            h_smeared.Draw("hist same")

            n_legend_entries = 2
            if len(unsmeared_files) > 1:
                n_legend_entries += len(unsmeared_files)
            if len(smeared_files) > 1:
                n_legend_entries += len(smeared_files)
            legend_height = 0.05 * n_legend_entries
            legend = ROOT.TLegend(0.60, 0.85 - legend_height, 0.9, 0.85)
            if len(unsmeared_files) > 1:
                legend.AddEntry(h_unsmeared, f"Unsmeared ({n_unsmeared})", "l")
                for i, h in enumerate(h_unsmeared_components):
                    lbl = unsmeared_labels[i] if i < len(unsmeared_labels) else f"Unsmeared {i+1}"
                    frac = unsmeared_fractions[i] if i < len(unsmeared_fractions) else 0.0
                    legend.AddEntry(h, f"{lbl} ({int(frac*100)}%)", "l")
            else:
                legend.AddEntry(h_unsmeared, f"Unsmeared ({n_unsmeared})", "l")
            if len(smeared_files) > 1:
                legend.AddEntry(h_smeared, f"Smeared ({n_smeared})", "l")
                for i, h in enumerate(h_smeared_components):
                    lbl = smeared_labels[i] if i < len(smeared_labels) else f"Smeared {i+1}"
                    frac = smeared_fractions[i] if i < len(smeared_fractions) else 0.0
                    legend.AddEntry(h, f"{lbl} ({int(frac*100)}%)", "l")
            else:
                legend.AddEntry(h_smeared, f"Smeared ({n_smeared})", "l")
            legend.Draw()

            # Label
            latex = ROOT.TLatex()
            latex.SetNDC()
            latex.SetTextAlign(31)
            latex.SetTextFont(42)
            latex.SetTextSize(0.04)
            latex.DrawLatex(0.9, 0.91, "HPS Work in Progress")
            if category_label:
                latex.SetTextSize(0.035)
                latex.DrawLatex(0.9, 0.86, category_label)

        # Include category in output filename if specified
        output_suffix = f"_{args.category}" if args.category != "all" else ""
        c.SaveAs(f"{args.output}/{sanitize_name(branch)}{output_suffix}.png")

        del h_unsmeared, h_smeared, legend, c
        for h in h_unsmeared_components:
            del h
        for h in h_smeared_components:
            del h
        if h_data:
            del h_data, r_unsmeared, r_smeared

if __name__ == "__main__":
    sys.exit(main() or 0)
