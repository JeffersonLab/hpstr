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
parser.add_option("-r", "--windowRange", type="int", dest="windowRange",
                  help="The minimum size of the consecutive windows needed to select a model.", metavar="windowRange", default=5)
(options, args) = parser.parse_args()

# Track the x-value ranges.
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

# The signal error and the pull are stored in TProfile objects. These
# must be instantiated before any other processing is done. Do this
# here.
sigError = {}
sigPull = {}
goodModels = {}
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

        # Model selection is only performed on data with no injected
        # signal. Skip all others.
        if (sigInj != 0):
            continue
        print('        Processing Mass: %i;   Order: %i;   Window Size: %i;   Signal: %i' % (mass, order, windowSize, sigInj))

        # Get the key set unique name.
        keyName = 'm%ip%ir%is%i' % (mass, order, resScale, sigInj)

        # The first model selection test is the background χ². This
        # value must exceed 0.01. Get the χ² probability, check if it
        # passes the threshold, and also store it so that it may be
        # persisted into the output file. This is only performed for
        # cases of no signal.

        # Get the χ² probability.
        bkg_chi2 = float(model.bkg_chi2_prob)

        # Test whether the χ² passes the threshold test.
        if (bkg_chi2 < 0.01):
            print('            Background Chi2: %f;   Passes: False' % (float(bkg_chi2)))
            continue
        print('            Background Chi2: %f;   Passes: True' % (float(bkg_chi2)))

        # The second model selection test is the pull. The pull is
        # determined by the pulls of each toy model that is generated
        # and stored in the input data set. The average pull and its
        # error are obtained by filling one of the TProfile objects
        # defined previously, both so that they may be persisted, and
        # also to make ROOT do all the math.

        # Populate the signal yield error and the pull plots. These
        # are not persisted. They are used to force ROOT to calculate
        # the relevant cut values.
        for toy in range(len(model.toy_sig_yield)):
            if model.toy_minuit_status[toy] > 0.0:
                continue
            sigError[keyName].Fill(float(windowSize), float(model.toy_sig_yield_err[toy]))
            sigPull[keyName].Fill(float(windowSize),  float(model.toy_sig_yield[toy] / model.toy_sig_yield_err[toy]))
            pass

        # Get the pull and and pull uncertainty from the TProfile.
        binN = sigPull[keyName].GetXaxis().FindBin(float(windowSize))
        pullAvg = float(sigPull[keyName].GetBinContent(binN))
        pullErr = float(sigPull[keyName].GetBinError(binN))
        #pullEntries = float(sigPull[keyName].GetBinEntries(binN))
        #sigma = math.sqrt(pullEntries) * pullErr

        # A model must be within 2σ of zero to be valid. Perform this
        # test and track the result.
        passesPull = False
        if ((pullAvg + (2 * pullErr) < 0) or (pullAvg - (2 * pullErr) > 0)):
            print('            Pull: %f;   Err: %f;   Passes: False' % (float(pullAvg), float(pullErr)))
            continue
        print('            Pull: %f;   Err: %f;   Passes: True' % (float(pullAvg), float(pullErr)))

        # If this point is reached, the model is good.
        if mass not in goodModels:
            goodModels[mass] = {}
        if order not in goodModels[mass]:
            goodModels[mass][order] = []
        goodModels[mass][order].append(windowSize)
        print("            Is Good Model: True")
        pass
    pass


# Extract the preferred model. This is done independently for each
# mass. For each model order, a range of consecutive, good windows
# is searched. The window range must exceed a certain, user-defined
# size to be valid. If found, this range is recorded for that order.
# Once each order has been searched, the lowest order with a range is
# considered and the window in the center of that range is selected.

# Perform the procedure for each mass hypothesis.
finalModelOrder = {}
finalModelWindow = {}
for mass in goodModels.keys():
    print('Mass: %i MeV:' % (int(mass)))

    # Track the start and end of each range by model order.
    startWindow = {}
    endWindow = {}

    # Search for a range in each model order.
    for order in goodModels[mass].keys():
        print('    Order: %i MeV:' % (int(order)))

        # The window sizes must be sorted in order  to determine if
        # they are consecutive.
        windows = sorted(goodModels[mass][order])

        # Search for a consecutive range.
        startRange = -1
        lastWindow = -1
        nWindows = 0
        for window in windows:
            print('        Window %i' % (int(window)))

            # Handle the case where the current window is consecutive
            # to the previous window.
            if (window == lastWindow + 1):
                nWindows += 1
                lastWindow = window

            # Handle the case where the current window is not
            # consecutive to the previous window, but a range of
            # acceptable size has been found. In this case, the range
            # is retained and no further ranges are considered.
            elif (window != lastWindow + 1 and nWindows >= options.windowRange):
                continue

            # Handle the case where either no windows have been yet
            # processed, or the current window is not consecutive to
            # the previous window and no acceptable range has been
            # found yet.
            else:
                startRange = window
                lastWindow = window
                nWindows = 1
            pass
        pass

        print('        Start: %i;   End: %i' % (startRange, startRange + nWindows - 1))

        # Store the range start and end.
        startWindow[order] = startRange
        endWindow[order] = startRange + nWindows - 1
    pass

    # Iterate over the ranges. An order is skipped if it is undefined
    # and the next order is considered instead. The lowest order with
    # a defined range is selected.
    modelOrder = 100
    modelWindow = -1
    for order in startWindow.keys():
        # Skip orders where the window range is too small.
        if (endWindow[order] - startWindow[order] < options.windowRange):
            continue

        # Otherwise, if the model order is lower than the current
        # selected range, replace it with this one.
        if (order < modelOrder):
            modelOrder = order
            modelWindow = math.floor(startWindow[order] + ((endWindow[order] - startWindow[order]) / 2.0))
        pass

    # If no proper range was found, just take whichever is largest
    # and pick the middle value. If two ranges are equal in size,
    # select the one with the smallest model order.
    if (modelOrder == 100):
        print("    Failed range test!! -- Using fallback method.")
        largestRange = 0
        for order in startWindow.keys():
            if (((endWindow[order] - startWindow[order]) > largestRange) or ((endWindow[order] - startWindow[order]) == largestRange and order < modelOrder)):
                modelOrder = order
                largestRange = endWindow[order] - startWindow[order]
                modelWindow = math.floor(startWindow[order] + ((endWindow[order] - startWindow[order]) / 2.0))
            pass

    print('    Model Order: %i;   Model Window: %i' % (modelOrder, modelWindow))
    finalModelOrder[mass] = modelOrder
    finalModelWindow[mass] = modelWindow

# Print the dictionary definition for the selected models.
for mass in finalModelOrder.keys():
    print('%i   %i   %i' % (mass, finalModelOrder[mass], finalModelWindow[mass]))
