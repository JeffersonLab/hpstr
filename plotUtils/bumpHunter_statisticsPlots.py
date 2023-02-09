#/bin/env python
import os
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
parser.add_option("-l", "--legend", type="int", dest="legend",
                  help="Enables/disables the legend. 0 = inactive; 1 = active.", metavar="legend", default=0)
parser.add_option("-p", "--prefix", type="string", dest="prefix",
                  help="Sets a prefix to prepend to all file names.", metavar="prefix", default="")
parser.add_option("-d", "--plotdir", type="string", dest="plot_dir",
                  help="Sets the plot output directory.", metavar="plot_dir", default=".")
(options, args) = parser.parse_args()

# Get the command line variables.
prefix = options.prefix
useLegend = (options.legend == 1)
plotDirectory = options.plot_dir

# Track the x-value range.
smallestWindow = 9999
largestWindow = 0

# Values must be mapped by mass, polynomial order, and window size. The
# dictionary should follow the structure of dict[mass][order][windowSize].
# At this point, just establish the mass dictionary for each tracked
# variable.
massPVal = {}
massBkgChi2 = {}
massBkgSigChi2 = {}
massToyChi2 = {}
massE2 = {}

# Iterate over all files in the input directory. Skip those that are not
# ROOT files.
filenames = os.listdir(options.inputDir)
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

        # Track the window size range.
        if (smallestWindow > windowSize):
            smallestWindow = windowSize
        if (largestWindow < windowSize):
            largestWindow = windowSize

        # Ensure that the mass key is defined.
        if mass not in massPVal:
            massPVal[mass] = {}
            massBkgChi2[mass] = {}
            massBkgSigChi2[mass] = {}
            massToyChi2[mass] = {}
            massE2[mass] = {}

        # Ensure that the order key is defined.
        if order not in massPVal[mass]:
            massPVal[mass][order] = {}
            massBkgChi2[mass][order] = {}
            massBkgSigChi2[mass][order] = {}
            massToyChi2[mass][order] = {}
            massE2[mass][order] = {}

        # Ensure that the signal injection key is defined.
        if sigInject not in massPVal[mass][order]:
            massPVal[mass][order][sigInject] = {}
            massBkgChi2[mass][order][sigInject] = {}
            massBkgSigChi2[mass][order][sigInject] = {}
            massToyChi2[mass][order][sigInject] = {}
            massE2[mass][order][sigInject] = {}

        # Ensure that the resolution scale key is defined.
        if resScale not in massPVal[mass][order][sigInject]:
            massPVal[mass][order][sigInject][resScale] = {}
            massBkgChi2[mass][order][sigInject][resScale] = {}
            massBkgSigChi2[mass][order][sigInject][resScale] = {}
            massToyChi2[mass][order][sigInject][resScale] = {}
            massE2[mass][order][sigInject][resScale] = {}

        # Get the tracked values.
        p_value = model.p_value
        bkg_chi2 = model.bkg_chi2_prob
        bkgsig_chi2 = model.bkgsig_chi2_prob
        toy_chi2 = model.toyfit_chi2_prob

        # Calculate the value of epsilon squared.
        gmass = mass / 1000.0
        if (model.sig_yield < 0):
            sigYield = 0.0
        else:
            sigYield = model.sig_yield
        radiativeFraction = (13603.8 * gmass * gmass * gmass * gmass * gmass) - (7779.47 * gmass * gmass * gmass * gmass) + (1669.07 * gmass * gmass * gmass) - (164.023 * gmass * gmass) + (7.07417 * gmass) - 0.0344651
        if (model.bkg_rate_mass_hypo == 0):
            e2 = 2
        else:
            e2 = 2 * (sigYield + 1.64 * model.sig_yield_err) / (411 * 3.14159 * mass * radiativeFraction * 20 * model.bkg_rate_mass_hypo)

        # Map each value to this window key for this combination of mass
        # order keys.
        massPVal[mass][order][sigInject][resScale][windowSize] = p_value
        massBkgChi2[mass][order][sigInject][resScale][windowSize] = bkg_chi2
        massBkgSigChi2[mass][order][sigInject][resScale][windowSize] = bkgsig_chi2
        massToyChi2[mass][order][sigInject][resScale][windowSize] = toy_chi2
        massE2[mass][order][sigInject][resScale][windowSize] = e2
        pass

# Print the extracted data.
for k_mass in massPVal:
    mass = int(k_mass)
    print('Mass %i:' % (mass))
    for k_order in massPVal[k_mass]:
        order = int(k_order)
        print('  Order %i:' % (order))
        for k_sigInj in massPVal[k_mass][k_order]:
            sigInj = int(k_sigInj)
            print('    Signal Injection %i:' % (sigInj))
            for k_resScale in massPVal[k_mass][k_order][k_sigInj]:
                resScale = int(k_resScale)
                f_resScale = resScale / 100.0
                print('      Resolution Scale %f:' % (resScale))
                for k_windowSize in massPVal[k_mass][k_order][k_sigInj][k_resScale]:
                    winSize = int(k_windowSize)
                    pValue = float(massPVal[k_mass][k_order][k_sigInj][k_resScale][k_windowSize])
                    bkg_chi2 = float(massBkgChi2[k_mass][k_order][k_sigInj][k_resScale][k_windowSize])
                    bkgsig_chi2 = float(massBkgSigChi2[k_mass][k_order][k_sigInj][k_resScale][k_windowSize])
                    toy_chi2 = float(massToyChi2[k_mass][k_order][k_sigInj][k_resScale][k_windowSize])
                    e2 = float(massE2[k_mass][k_order][k_sigInj][k_resScale][k_windowSize])
                    print('        %i :: p-Value: %f;  Background Chi2: %f;  SigBackground Chi2: %f;  Toy Chi2: %f;  e2: %f' % (winSize, pValue, bkg_chi2, bkgsig_chi2, toy_chi2, e2))

# Create the output file and output the plots.
outFile = r.TFile(options.outputFile, 'RECREATE')
outFile.cd()

# Set the plot style.
utils.SetStyle()

for k_mass in massPVal:
    mass = int(k_mass)
    for k_order in massPVal[k_mass]:
        order = int(k_order)
        for k_sigInj in massPVal[k_mass][k_order]:
            sigInj = int(k_sigInj)
            for k_resScale in massPVal[k_mass][k_order][k_sigInj]:
                resScale = int(k_resScale)

                # Get the window sizes and sort them in ascending order.
                windowSizes = sorted(massPVal[k_mass][k_order][k_sigInj][k_resScale].keys())

                # Place each tracked values into lists to serve as input
                # into the TGraphs.
                pValues = []
                bkgChi2s = []
                sigbkgChi2s = []
                toyChi2s = []
                e2s = []
                for k_windowSize in windowSizes:
                    pValues.append(float(massPVal[k_mass][k_order][k_sigInj][k_resScale][k_windowSize]))
                    bkgChi2s.append(float(massBkgChi2[k_mass][k_order][k_sigInj][k_resScale][k_windowSize]))
                    sigbkgChi2s.append(float(massBkgSigChi2[k_mass][k_order][k_sigInj][k_resScale][k_windowSize]))
                    toyChi2s.append(float(massToyChi2[k_mass][k_order][k_sigInj][k_resScale][k_windowSize]))
                    e2s.append(float(massE2[k_mass][k_order][k_sigInj][k_resScale][k_windowSize]))

                # Set focus on the output file.
                outFile.cd()

                # Create each graph.
                pValGraph = r.TGraph(len(pValues), np.array(windowSizes), np.array(pValues))
                pValGraph.SetName('m%ip%i_pValue_g' % (mass, order))
                pValGraph.SetTitle('Mass %s O(%i) p-Value vs. Window Size;Window Size (Mass Resolution);p-Value' % (mass, order))

                bkgChi2Graph = r.TGraph(len(bkgChi2s), np.array(windowSizes), np.array(bkgChi2s))
                bkgChi2Graph.SetName('m%ip%i_bkgChi2_g' % (mass, order))
                bkgChi2Graph.SetTitle('Mass %s O(%i) Background #chi^{2} vs. Window Size;Window Size (Mass Resolution);#chi^{2}' % (mass, order))

                bkgSigChi2Graph = r.TGraph(len(sigbkgChi2s), np.array(windowSizes), np.array(sigbkgChi2s))
                bkgSigChi2Graph.SetName('m%ip%i_sigbkgChi2_g' % (mass, order))
                bkgSigChi2Graph.SetTitle('Mass %s O(%i) Signal+Background #chi^{2} vs. Window Size;Window Size (Mass Resolution);#chi^{2}' % (mass, order))

                toyChi2Graph = r.TGraph(len(toyChi2s), np.array(windowSizes), np.array(toyChi2s))
                toyChi2Graph.SetName('m%ip%i_toyChi2_g' % (mass, order))
                toyChi2Graph.SetTitle('Mass %s O(%i) Toy Generator #chi^{2} vs. Window Size;Window Size (Mass Resolution);#chi^{2}' % (mass, order))

                e2Graph = r.TGraph(len(e2s), np.array(windowSizes), np.array(e2s))
                e2Graph.SetName('m%ip%i_e2_g' % (mass, order))
                e2Graph.SetTitle('Mass %s O(%i) #epsilon^{2} vs. Window Size;Window Size (Mass Resolution);#epsilon^{2}' % (mass, order))

                # Add the plots to a list for easy formatting.
                plots = [pValGraph, bkgChi2Graph, bkgSigChi2Graph, toyChi2Graph, e2Graph]

                # Set the graph styling.
                nColor = 0
                for plot in plots:
                    plot.Write()
                    plot.SetLineWidth(3)
                    plot.SetLineColor(utils.colors[nColor])
                    nColor += 1

                # Plot the plots all on the same canvas.
                combinedCanvas = r.TCanvas('m%ip%i_canvas' % (mass, order), 'm%ip%i_canvas' % (mass, order), 2500, 1000)
                combinedCanvas.SetLogy()

                # All plot windows should be locked the same range. If the
                # legend is to be included, this should 33% bigger than the
                # actual max so that there is room for the legend.
                if (useLegend):
                    windowMax = int(largestWindow + ((largestWindow - smallestWindow) / 3.0))
                    pValGraph.GetXaxis().SetLimits(float(smallestWindow), float(windowMax))
                else:
                    pValGraph.GetXaxis().SetLimits(float(smallestWindow), float(largestWindow))

                # Set the graph y-axis so that it covers only the higher
                # regions, which are of more interest.
                pValGraph.SetMinimum(1e-8)
                pValGraph.SetMaximum(2.0)

                # Set the title offset so the y-axis title does not
                # overlap anything. Also, change the y-axis title to
                # something that makes sense for all plots.
                pValGraph.SetTitle('Mass %s O(%i) Statistics Plots vs. Window Size;Window Size (Mass Resolution);Probability' % (mass, order))
                pValGraph.GetYaxis().SetTitleOffset(0.60)

                # Draw the plots onto the canvas.
                combinedCanvas.cd()
                pValGraph.Draw()
                bkgChi2Graph.Draw("same")
                bkgSigChi2Graph.Draw("same")
                toyChi2Graph.Draw("same")
                e2Graph.Draw("same")

                # Draw the "HPS Internal" text. This should move in
                # case of a legend to the empty area.
                if (useLegend):
                    utils.InsertText("", [], 0.15, 0.80)
                else:
                    utils.InsertText("", [], 0.15, 0.15)

                # If a legend should be used, draw it.
                if (useLegend):
                    legend = r.TLegend(0.82, 0.675, 0.95, 0.675 - (5 * 0.07))
                    legend.SetName("compiledLegend")
                    legend.SetBorderSize(0)
                    legend.SetFillStyle(0)
                    legend.SetFillColor(0)
                    legend.SetTextSize(0.03)
                    legend.AddEntry(pValGraph, "p-Value", "lpf")
                    legend.AddEntry(bkgChi2Graph, "Bkg #chi^{2}", "lpf")
                    legend.AddEntry(bkgSigChi2Graph, "Sig+Bkg #chi^{2}", "lpf")
                    legend.AddEntry(toyChi2Graph, "Generator #chi^{2}", "lpf")
                    legend.AddEntry(e2Graph, "#epsilon^{2}", "lpf")
                    legend.Draw("same")

                # Save the plot.
                combinedCanvas.SaveAs('%s/%sm%ip%ir%is%i_statPlots.png' % (plotDirectory, prefix, mass, order, resScale, sigInj))
                pass
            pass
        pass
    pass

# Close the output file.
outFile.Close()
