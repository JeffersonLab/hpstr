#/bin/env python
import os
import re
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
(options, args) = parser.parse_args()

# Get the command line variables.
prefix = options.prefix
plotDirectory = options.plot_dir

# Track the x-value range.
smallestWindow = 9999
largestWindow = 0

# Determine the window size range and instantiate the plot files.
filenames = os.listdir(options.inputDir)
for filename in filenames:
    # Break the filename apart into its parameters.
    params = re.split('[mwprs.]', filename)[2:7]
    windowSize = int(params[1])

    # Track the window size range.
    if (smallestWindow > windowSize):
        smallestWindow = windowSize
    if (largestWindow < windowSize):
        largestWindow = windowSize
    pass

print('Window Minimum: %i;   Window Maximum: %i' % (smallestWindow, largestWindow))

# Instantiate the plot files.
sigYield = {}
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

    # Generate the unique key set name for the plot.
    keyName = 'm%ip%ir%is%i' % (mass, order, resScale, sigInj)

    # If the plot dictionaries lack this key, create a new plot.
    if keyName not in sigYield:
        # Determine the plot range.
        winMin = smallestWindow - 0.5
        winMax = largestWindow + 0.5
        winBin = winMax - winMin

        # Create a TProfile for this key set.
        sigYield[keyName] = r.TProfile('%s_toySig' % (keyName), '%s Toy Signal Yield;Window Size (Mass Resolution);Signal Yield' % (keyName), int(winBin), float(winMin), float(winMax))
        sigError[keyName] = r.TProfile('%s_toyErr' % (keyName), '%s Toy Signal Yield Error;Window Size (Mass Resolution);Signal Yield Error' % (keyName), int(winBin), float(winMin), float(winMax))
        sigPull[keyName] = r.TProfile('%s_toyPull' % (keyName), '%s Toy Pull;Window Size (Mass Resolution);Signal Pull' % (keyName), int(winBin), float(winMin), float(winMax))
        sigYield[keyName].SetErrorOption('s')
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
        sigInject = model.toy_sig_samples
        resScale = int(model.resolution_scale * 100)

        # Get the key set unique name.
        keyName = 'm%ip%ir%is%i' % (mass, order, resScale, sigInject)

        # Get the tracked values.
        for toy in range(len(model.toy_sig_yield)):
            if model.toy_minuit_status[toy] > 0.0:
                continue
            sigYield[keyName].Fill(float(windowSize), float(model.toy_sig_yield[toy]))
            sigError[keyName].Fill(float(windowSize), float(model.toy_sig_yield_err[toy]))
            sigPull[keyName].Fill(float(windowSize),  float((model.toy_sig_yield[toy] - model.toy_sig_samples) / model.toy_sig_yield_err[toy]))
            pass
        pass

# Create the output file and output the plots.
outFile = r.TFile(options.outputFile, 'RECREATE')
outFile.cd()

# Set the plot style.
utils.SetStyle()

# Print the plots and save them to the output file.
for keyName in sigYield.keys():
    # Get the key set plots.
    yieldPlot = sigYield[keyName]
    errorPlot = sigError[keyName]
    pullPlot = sigPull[keyName]

    # Set focus on the output file.
    outFile.cd()

    # Add the plots to a list for easy formatting.
    plots = [yieldPlot, errorPlot, pullPlot]

    # Write the plots and apply plot formatting.
    for plot in plots:
        plot.SetLineColor(utils.colors[0])
        plot.Write()
        plot.SetLineWidth(3)
        plot.GetXaxis().SetTitleOffset(1.2)
        plot.GetYaxis().SetTitleOffset(1.0)

    # The pull plot should have a special range.
    pullPlot.GetYaxis().SetRangeUser(-5, 5)

    # Each plot is saved individually.
    yieldCanvas = r.TCanvas('%s_yield_canvas' % (keyName), '%s_yield_canvas' % (keyName), 2500, 1000)
    errorCanvas = r.TCanvas('%s_error_canvas' % (keyName), '%s_error_canvas' % (keyName), 2500, 1000)
    pullCanvas = r.TCanvas('%s_pull_canvas' % (keyName), '%s_pull_canvas' % (keyName), 2500, 1000)

    # Draw the plots onto the canvases.
    yieldCanvas.cd()
    yieldPlot.Draw()
    utils.InsertText("", [], 0.15, 0.15)
    yieldCanvas.SaveAs('%s/%s%s_yield.png' % (plotDirectory, prefix, keyName))

    errorCanvas.cd()
    errorPlot.Draw()
    utils.InsertText("", [], 0.15, 0.15)
    errorCanvas.SaveAs('%s/%s%s_error.png' % (plotDirectory, prefix, keyName))

    pullCanvas.cd()
    pullPlot.Draw()
    utils.InsertText("", [], 0.15, 0.15)
    pullCanvas.SaveAs('%s/%s%s_pull.png' % (plotDirectory, prefix, keyName))
    pass

# Close the output file.
outFile.Close()
