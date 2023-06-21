#/bin/env python
import os
import re
import ROOT as r
import numpy as np
import utilities as utils
from optparse import OptionParser

# Generates fit comparison plots for the resonance search. The script
# will generate two plots: one depicting the data histogram minus the
# background-only fit function and one depicting the background with
# signal fit function minus the background-only fit function.

# One plot is generated for each combination of mass, order, and window
# multiplier. These are output to the directory specified by the "-d"
# option. The plots are generated from all ROOT files in the input
# directory, which is specified by the "-i" option. It is required that
# the fit plots be a part of the invariant mass histogram stored in the
# input ROOT files and have the names "bkg" and "full". The directory
# containing the input files should only contain bump hunter output
# files. No error checking is performed to verify that the files are
# valid.

# Parse the command line options.
parser = OptionParser()
parser.add_option("-i", "--inputDir", type="string", dest="inputDir",
                  help="The directory containing the input ROOT files.", metavar="inputDir", default="toys/toys.root")
parser.add_option("-p", "--prefix", type="string", dest="prefix",
                  help="Sets a prefix to prepend to all file names.", metavar="prefix", default="")
parser.add_option("-d", "--plotdir", type="string", dest="plot_dir",
                  help="Sets the plot output directory.", metavar="plot_dir", default=".")
parser.add_option("-H", "--hist_name", type="string", dest="hist_name",
                  help="Sets the name of the invariant mass histogram where the fits are stored.", metavar="hist_name", default="h_Minv_General_Final_1")
(options, args) = parser.parse_args()

# Get the command line variables.
prefix = options.prefix
plotDirectory = options.plot_dir

# Iterate over the files in the input directory.
filenames = os.listdir(options.inputDir)
for filename in filenames:
    # Get the file name key.
    temp_splitExtension = re.split('.root', filename)[0]
    temp_splitUnderscore = temp_splitExtension.split("_")
    filenameKey = temp_splitUnderscore[len(temp_splitUnderscore) - 1]

    # Access the invariant mass file.
    inFile = r.TFile('%s/%s' % (options.inputDir, filename))

    # Get the invariant mass histogram.
    hist = inFile.Get(options.hist_name)
    bins = hist.GetXaxis().GetNbins()
    xMin = hist.GetXaxis().GetXmin()
    xMax = hist.GetXaxis().GetXmin()

    # Get the fit functions from the histogram.
    bkgFunction = hist.GetFunction("bkg")
    bkgsigFunction = hist.GetFunction("full")

    # Calculate the histogram value minus the background function value
    # as well as the background plus signal value minus the background
    # value at each bin center of the histogram.
    xVals = []
    funcDiff = []
    histDiff = []
    for i in range(1, bins + 1):
        # Calculate the values.
        histX = hist.GetBinCenter(i)
        histVal = hist.GetBinContent(i)
        bkgVal = bkgFunction.Eval(histX)
        bkgsigVal = bkgsigFunction.Eval(histX)

        # Calculate the differences.
        histDiffVal = histVal - bkgVal
        funcDiffVal = bkgsigVal - bkgVal

        # Store them in the arrays, but only if the function difference
        # is non-zero. A value of zero indicates that this x value is not
        # within the fit window.
        if funcDiffVal != 0:
            xVals.append(histX)
            histDiff.append(histVal - bkgVal)
            funcDiff.append(bkgsigVal - bkgVal)
        pass

    # DEBUG :: Print the results.
    #for i in range(0, len(funcDiff)):
    #	print 'x = %f;   funcDiff = %f;   histDiff = %f' % (xVals[i], funcDiff[i], histDiff[i])
    #	pass

    # Create a graph of each of the two plots.
    histDiffGraph = r.TGraph(len(xVals), np.array(xVals), np.array(histDiff))
    histDiffGraph.SetName('%s_histDiff_g' % filenameKey)
    histDiffGraph.SetTitle('%s Fits;Mass (GeV);Difference' % filenameKey)
    histDiffGraph.SetLineWidth(3)
    histDiffGraph.SetLineColor(r.kAzure+4)

    funcDiffGraph = r.TGraph(len(xVals), np.array(xVals), np.array(funcDiff))
    funcDiffGraph.SetName('%s_histDiff_g' % filenameKey)
    funcDiffGraph.SetTitle('%s Fits;Mass (GeV);Difference' % filenameKey)
    funcDiffGraph.SetLineWidth(3)
    funcDiffGraph.SetLineColor(r.kRed-3)

    # Rescale the graph axis to make room for a legend.
    graphXMin = xVals[0]
    graphXMax = xVals[len(xVals) - 1]
    scaledXMax = graphXMax + ((graphXMax - graphXMin) / 3.0)
    histDiffGraph.GetXaxis().SetLimits(graphXMin, scaledXMax)

    # Make a canvas for the plots.
    canvas = r.TCanvas('%s_fit_canvas' % (filenameKey), '%s_fit_canvas' % (filenameKey), 2500, 1000)
    canvas.cd()

    # Draw the plots.
    histDiffGraph.Draw()
    funcDiffGraph.Draw("same")

    # Add the "HPS Internal" text.
    utils.InsertText("", [], 0.15, 0.73)

    # Make the legend.
    legend = r.TLegend(0.72, 0.507, 0.85, 0.507 - (2 * 0.07))
    legend.SetName("compiledLegend")
    legend.SetBorderSize(0)
    legend.SetFillStyle(0)
    legend.SetFillColor(0)
    legend.SetTextSize(0.03)
    legend.AddEntry(histDiffGraph, "Data - Bkg. Function", "lpf")
    legend.AddEntry(funcDiffGraph, "Sigbkg. Func. - Bkg. Func.", "lpf")
    legend.Draw("same")

    # Save the plot.
    canvas.SaveAs('%s/%s%s_fitDiffPlots.png' % (plotDirectory, prefix, filenameKey))
    pass
