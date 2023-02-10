#/bin/env python
# coding: utf-8
import os
import re
import math
import ROOT as r
import numpy as np
import utilities as utils

from optparse import OptionParser

# Parse the command line options.
parser = OptionParser()
parser.add_option("-i", "--inputDir", type="string", dest="inputDir",
                  help="The directory containing the input ROOT files.", metavar="inputDir", default="toys/toys.root")
parser.add_option("-o", "--outputFile", type="string", dest="outputFile",
                  help="Specify the output filename.", metavar="outputFile", default="testOut.root")
parser.add_option("-p", "--prefix", type="string", dest="prefix",
                  help="Sets a prefix to prepend to all file names.", metavar="prefix", default="")
parser.add_option("-d", "--plotdir", type="string", dest="plot_dir",
                  help="Sets the plot output directory.", metavar="plot_dir", default=".")
parser.add_option("-f", "--truthFraction", type="string", dest="truthFracFile",
                  help="Sets the file containing the signal truth-matching fractions.", metavar="truthFracFile", default="dat/truthMatchFrac.dat")
(options, args) = parser.parse_args()

# Map the corrections for signal removal. This takes as an input
# file where each line is of the form "Mass(MeV),Fraction". There
# should be no spaces and should be one mass per line. No empty lines.
truthMatchFrac = {}
print("Defining signal truth-matching fraction:")
tmfFile = open(options.truthFracFile, "r")
for line in tmfFile:
    vals = line.split(",")
    mass = int(vals[0])
    frac = float(vals[1])
    truthMatchFrac[mass] = frac
    print('    Mass: %i;   Fraction: %f' % (mass, frac))
    pass

# Get the command line variables.
prefix = options.prefix
plotDirectory = options.plot_dir

# Track the x-value ranges.
smallestWindow = 9999
smallestSigInj = 99999999999
largestWindow = 0
largestSigInj = 0

# Determine the window size range and instantiate the plot files.
filenames = os.listdir(options.inputDir)
for filename in filenames:
    # Break the filename apart into its parameters.
    params = re.split('[mwprs.]', filename)[2:7]
    windowSize = int(params[1])
    sigInj = int(params[4])

    # Track the window size range.
    if (smallestWindow > windowSize):
        smallestWindow = windowSize
    if (smallestSigInj > sigInj):
        smallestSigInj = windowSize
    if (largestWindow < windowSize):
        largestWindow = windowSize
    if (largestSigInj < sigInj):
        largestSigInj = sigInj
    pass

print('Window Minimum: %i;   Window Maximum: %i' % (smallestWindow, largestWindow))
print('Signal Minimum: %i;   Signal Maximum: %i' % (smallestSigInj, largestSigInj))

# The signal error and the pull are stored in TProfile objects. These
# must be instantiated before any other processing is done. Do this
# here.
chi2Data = {}
pullSigG = {}
pullSigErrG = {}

sigError = {}
sigPull = {}
filenames = os.listdir(options.inputDir)
for filename in filenames:
    # Break the filename apart into its parameters.
    params = re.split('[mwprs.]', filename)[2:7]
    mass = int(params[0])
    windowSize = int(params[1])
    order = int(params[2])
    resScale = int(params[3])
    sigInj = int(params[4])

    # Generate the unique key set name.
    keyName = 'm%ip%ir%is%i' % (mass, order, resScale, sigInj)

    # If the key set name is not present in the pull and signal yield
    # error dictionary, instantiate the plots.
    if keyName not in sigPull:
        # Determine the plot range.
        winMin = smallestWindow - 0.5
        winMax = largestWindow + 0.5
        winBin = winMax - winMin

        # Create a TProfile for this key set.
        sigError[keyName] = r.TProfile('%s_toyErr' % (keyName), '%s Toy Signal Yield Error;Window Size (Mass Resolution);Signal Yield Error' % (keyName), int(winBin), float(winMin), float(winMax))
        sigPull[keyName] = r.TProfile('%s_toyPull' % (keyName), '%s Toy Pull;Window Size (Mass Resolution);Signal Pull' % (keyName), int(winBin), float(winMin), float(winMax))
        sigError[keyName].SetErrorOption('s')
        sigPull[keyName].SetErrorOption('s')
    pass


# Iterate over all files in the input directory. Skip those that are not
# ROOT files.
totalFiles = len(filenames)
processedFiles = 0
print("Processing data...")
for filename in filenames:
    processedFiles += 1
    print('    %f%%' % (100.0 * processedFiles / totalFiles))

    # Exclude non-ROOT files.
    if not filename.endswith(".root"):
        continue

    # Get the current input file.
    inFile = r.TFile(options.inputDir + filename)
    tree = inFile.fit_toys

    # Iterate over the "entries" in the file. There's really only ever
    # one, but this forces Python to set everything up automatically.
    for model in inFile.fit_toys:
        # Get the dictionary keys.
        order = model.poly_order
        mass = int(model.mass_hypo * 1000)
        windowSize = model.win_factor
        sigInj = model.toy_sig_samples
        resScale = int(model.resolution_scale * 100)
        print('        Processing Mass: %i;   Order: %i;   Window Size: %i;   Signal: %i' % (mass, order, windowSize, sigInj))

        # Get the key set unique name.
        keyName = 'm%ip%ir%is%i' % (mass, order, resScale, sigInj)

        # Collect the χ² data.
        if (sigInj == 0):
            # Get the χ² probability.
            bkg_chi2 = float(model.bkg_chi2_prob)

            # Map it to the current window size for this key set. If the
            # key set is not defined yet, instantiate it.
            if keyName not in chi2Data:
                chi2Data[keyName] = {}
            chi2Data[keyName][windowSize] = bkg_chi2

        # Collect the signal yield and pull data.
        nTotal = 0
        nFailed = 0
        for toy in range(len(model.toy_sig_yield)):
            nTotal += 1
            if model.toy_minuit_status[toy] > 0.0:
                continue
            if model.toy_sig_yield_err[toy] == 0:
                nFailed += 1
                print('            Yield: %f;   RFrac: %f;   Signal: %i;   Error: %f' % (model.toy_sig_yield[toy], float(truthMatchFrac[mass]), model.toy_sig_samples, model.toy_sig_yield_err[toy]))
                continue
            sigError[keyName].Fill(float(windowSize), float(model.toy_sig_yield_err[toy]))
            sigPull[keyName].Fill(float(windowSize),  float((model.toy_sig_yield[toy] - (float(truthMatchFrac[mass]) * model.toy_sig_samples)) / model.toy_sig_yield_err[toy]))
            pass
        print('            Failed: %i;   Total: %i;   Percent Failed: %f%%' % (nFailed, nTotal, 100.0 * nFailed / nTotal))

        # Get the pull and and pull uncertainty from the TProfile.
        binN = sigPull[keyName].GetXaxis().FindBin(float(windowSize))
        pullAvg = float(sigPull[keyName].GetBinContent(binN))
        pullErr = float(sigPull[keyName].GetBinError(binN))

        # Store the pull as a function of signal injection.
        pullSigGKey = 'm%ip%iw%i' % (mass, order, windowSize)
        if pullSigGKey not in pullSigG:
            pullSigG[pullSigGKey] = {}
        if pullSigGKey not in pullSigErrG:
            pullSigErrG[pullSigGKey] = {}
        pullSigG[pullSigGKey][sigInj] = pullAvg
        pullSigErrG[pullSigGKey][sigInj] = pullErr
        pass
    pass


# Create the output file and output the plots.
outFile = r.TFile(options.outputFile, 'RECREATE')
outFile.cd()

# Set the plot style.
utils.SetStyle()

# Save the error and pull versus window size plots.
for key in sigError.keys():
    sigError[key].Write()
    sigPull[key].Write()

# Save the pull as a function of signal strength plots.
fitSlope = {}
fitSlopeErr = {}
fitOffset = {}
fitOffsetErr = {}
for key in pullSigG:
    print('\n\n%s' % key)

    # Get the x-axis values. These are just the signal injection
    # strength keys sorted in ascending order.
    sigInjs = sorted(pullSigG[key].keys())

    # Get the signal injection order.
    logSigInj = []
    for sigInj in sigInjs:
        if (sigInj == 0):
            logSigInj.append(0)
        else:
            logSigInj.append(math.log(sigInj, 10))
        pass

    # Get the y-axis values. These are the pulls corresponding to the
    # signal injection strength values.
    pulls = []
    xErrs = []
    yErrs = []
    for sigInj in sigInjs:
        pulls.append(pullSigG[key][sigInj])
        xErrs.append(0)
        yErrs.append(pullSigErrG[key][sigInj])
        print('    Inj: %i;    Pull: %f' % (int(sigInj), float(pullSigG[key][sigInj])))
        pass

    # Create a TGraph to store the values.
    pullSigGraph = r.TGraphErrors(len(pulls), np.array(sigInjs), np.array(pulls), np.array(xErrs), np.array(yErrs))
    pullSigGraph.SetName('%s_injVSpull_g' % (key))
    pullSigGraph.SetTitle('%s Pull vs. Signal Injection Strength;Signal Injection (Events);Pull' % (key))

    pullLogSigGraph = r.TGraphErrors(len(pulls), np.array(logSigInj), np.array(pulls), np.array(xErrs), np.array(yErrs))
    pullLogSigGraph.SetName('%s_loginjVSpull_g' % (key))
    pullLogSigGraph.SetTitle('%s Pull vs. Log_{10}(Signal Injection Strength);Signal Injection (Events);Pull' % (key))

    # Break the key apart into its components.
    params = re.split('[mwp]', key)[1:]
    mass = int(params[0])
    order = int(params[1])
    windowSize = int(params[2])

    # Each TGraph should be fit with a linear fit function and then
    # the slope stored as a function of window size for a given mass
    # and order. Create a 1D fit function.
    fitResult = pullSigGraph.Fit("pol1", "S+")
    fitParams = fitResult.GetParams()
    fitErrors = fitResult.GetErrors()
    offset = fitParams[0]
    offsetErr = fitErrors[0]
    slope = fitParams[1]
    slopeErr = fitErrors[1]

    # Store the values.
    valKey = 'm%io%i' % (mass, order)
    if valKey not in fitSlope:
        fitSlope[valKey] = {}
        fitSlopeErr[valKey] = {}
        fitOffset[valKey] = {}
        fitOffsetErr[valKey] = {}
    fitSlope[valKey][windowSize] = slope
    fitSlopeErr[valKey][windowSize] = slopeErr
    fitOffset[valKey][windowSize] = offset
    fitOffsetErr[valKey][windowSize] = offsetErr

    # Set the plot style.
    plots = [pullSigGraph, pullLogSigGraph]
    plotNames = ["pullSig", "logPullSig"]
    for i in range(len(plots)):
        plots[i].SetLineWidth(3)
        plots[i].SetMarkerSize(3)
        plots[i].SetMarkerStyle(20)
        plots[i].GetYaxis().SetTitleOffset(0.60)

        # Draw the plot and save it to a file.
        canvas = r.TCanvas('%s_canvas' % (key), '%s_canvas' % (key), 2500, 1000)
        plots[i].Draw("ap")
        utils.InsertText("", [], 0.85, 0.15)
        canvas.SaveAs('%s/%sm%io%iw%i_%s.png' % (plotDirectory, prefix, mass, order, windowSize, plotNames[i]))

        # Save the graph.
        plots[i].Write()
        pass

# Create the slope and offset graphs.
for key in fitSlope:
    # Get the x-axis values. These are just the window size keys
    # sorted in ascending order.
    windowSizes = []
    for windowSize in fitSlope[key].keys():
        windowSizes.append(float(windowSize))
    windowSizes.sort()

    # Get the y-axis values. These are the fit parameters.
    slopes = []
    slopeXErrs = []
    slopeYErrs = []
    offsets = []
    offsetYErrs = []
    for windowSize in windowSizes:
        slopeXErrs.append(0.0)
        slopes.append(float(fitSlope[key][windowSize]))
        slopeYErrs.append(float(fitSlopeErr[key][windowSize]))
        offsets.append(float(fitOffset[key][windowSize]))
        offsetYErrs.append(float(fitOffsetErr[key][windowSize]))
        pass

    # DEBUG PRINT
    print(key)
    for i in range(len(windowSizes) - 1):
        print('(%i, %f, %f)' % (windowSizes[i], slopes[i], offsets[i]))

    # Create TGraph objects to store the values.
    slopeGraph = r.TGraphErrors(len(slopes), np.array(windowSizes), np.array(slopes), np.array(slopeXErrs), np.array(slopeYErrs))
    slopeGraph.SetName('%s_slope_g' % (key))
    slopeGraph.SetTitle('%s Fit Slope vs. Window Size;Window Size (Mass Resolution);Slope' % (key))

    offsetGraph = r.TGraphErrors(len(offsets), np.array(windowSizes), np.array(offsets), np.array(slopeXErrs), np.array(offsetYErrs))
    offsetGraph.SetName('%s_offset_g' % (key))
    offsetGraph.SetTitle('%s Fit Offset vs. Window Size;Window Size (Mass Resolution);Offset' % (key))

    # Set the plot style.
    slopeGraph.SetLineWidth(3)
    slopeGraph.SetMarkerSize(2)
    slopeGraph.SetMarkerStyle(20)
    slopeGraph.GetYaxis().SetTitleOffset(0.60)

    offsetGraph.SetLineWidth(3)
    offsetGraph.SetMarkerSize(2)
    offsetGraph.SetMarkerStyle(20)
    offsetGraph.GetYaxis().SetTitleOffset(0.60)

    # Draw the plot and save it to a file.
    canvas = r.TCanvas('%s_canvas' % (key), '%s_canvas' % (key), 2500, 1000)
    slopeGraph.Draw("ap")
    utils.InsertText("", [], 0.15, 0.15)
    canvas.SaveAs('%s/%s%s_slopeVSwindowSize.png' % (plotDirectory, prefix, key))

    # Save the graphs.
    outFile.cd()
    slopeGraph.Write()
    offsetGraph.Write()

# Save the χ² probabilities.
for key in chi2Data.keys():
    # Create two lists - one containing the window sizes and one
    # containing the χ² probabilities.
    windowSizes = sorted(chi2Data[key].keys())
    chi2s = []
    for windowSize in windowSizes:
        chi2s.append(chi2Data[key][windowSize])

    # Create a TGraph to store the χ² data.
    bkgChi2Graph = r.TGraph(len(chi2s), np.array(windowSizes), np.array(chi2s))
    bkgChi2Graph.SetName('%s_bkgChi2_g' % (key))
    bkgChi2Graph.SetTitle('%s Background #chi^{2} vs. Window Size;Window Size (Mass Resolution);#chi^{2}' % (key))
    bkgChi2Graph.Write()

# Close the output file.
outFile.Close()
