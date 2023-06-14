#/bin/env python
import math
import numpy as np
import ROOT as r
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
parser.add_option("-m", "--selectedModels", type="string", dest="modelFile",
                  help="Sets the file containing the selected models for each mass.", metavar="modelFile", default="dat/selectedModels.dat")
(options, args) = parser.parse_args()

# Get the command line variables.
prefix = options.prefix
plotDirectory = options.plot_dir

# Get the selected models. These should be in a file with
# one mass per line of the form "Mass(MeV),Order,Window".
# Thre should be no empty lines. These should ideally be
# in ascending order. Unexpected behavior may otherwise
# be observed.
masses = []
polOrder = {}
winSize = {}
tmfFile = open(options.modelFile, "r")
print("Reading selected models:")
for line in tmfFile:
    # Parse the line to get the selected model.
    vals = line.split(",")
    mass = int(vals[0])
    order = int(vals[1])
    window = int(vals[2])

    # Add the mass to the list of masses that are to
    # be processed.
    masses.append(mass)

    # Add the models to the appropriate maps.
    polOrder[mass] = order
    winSize[mass] = window
    print('    Mass: %i;   Order: %i;   Window: %i' % (mass, order, window))
    pass

# Track the plot values.
massFloats = []
pSearch = []
pSearchG = []
limitN = []
limitEps = []

toyEpsilon = []
toyEpsilonEU1 = []
toyEpsilonEU2 = []
toyEpsilonEL1 = []
toyEpsilonEL2 = []

# Make the output file.
outFile = r.TFile(options.outputFile, 'RECREATE')

# Plot pull for all models.
sigPull = r.TProfile("limits_toyPull", "Toy Pull;Mass (MeV);Signal Pull", 150, 29.5, 179.5)
sigPull.SetErrorOption('s')

for mass in masses:
    # Track the masses processed.
    massFloats.append(float(mass))

    # Load the input file.
    inputFile = "%s/bhToys_m%iw%ip%ir100s0.root" % (options.inputDir, mass, winSize[mass], polOrder[mass])
    inFile = r.TFile(inputFile)
    #print('Reading file %s...' % inputFile)

    # Calculate the radiative fraction.
    gmass = mass / 1000.0
    radFrac = (13603.8 * gmass * gmass * gmass * gmass * gmass) - (7779.47 * gmass * gmass * gmass * gmass) + (1669.07 * gmass * gmass * gmass) - (164.023 * gmass * gmass) + (7.07417 * gmass) - 0.0344651

    # Calculate the epsilon squared limits.
    for entry in inFile.fit_toys:
        # Store the p-values for data.
        pSearch.append(entry.p_value)
        globalPValue = entry.p_value * 32.8481
        if globalPValue > 1:
            globalPValue = 1
        pSearchG.append(globalPValue)

        # Get the signal yield and upper limit for data.
        nSigFit = entry.sig_yield
        if nSigFit < 0.0:
            nSigFit = 0.0
        upLim = nSigFit + 1.64 * entry.sig_yield_err

        # Store the upper limit and the epsilon squared upper limit
        # for data.
        limitN.append(upLim)
        limitEps.append(2 * upLim / (411 * 3.14159 * mass * radFrac * 20 * entry.bkg_rate_mass_hypo))

        # Get all values for signal yield, upper limit, and
        # background rate from toys.
        t_epsilon = []
        for toy in range(len(entry.toy_sig_yield)):
            # Skip entries with bad fit stati.
            if entry.toy_minuit_status[toy] > 0.0:
                continue

            # The signal yield is zero if negative.
            t_sigYieldVal = float(entry.toy_sig_yield[toy])
            if t_sigYieldVal < 0.0:
                t_sigYieldVal = 0.0

            # Plot the pull.
            sigPull.Fill(float(mass), float((entry.toy_sig_yield[toy] - entry.toy_sig_samples) / entry.toy_sig_yield_err[toy]))

            # Get the upper limit and background rate.
            t_upperLimit = float(t_sigYieldVal + 1.64 * entry.toy_sig_yield_err[toy])
            t_bkgRate = float(entry.toy_bkg_rate_mass_hypo[toy])

            # Calculate epsilon squared.
            t_epsilon.append(2 * t_upperLimit / (411 * 3.14159 * mass * radFrac * 20 * t_bkgRate))
            pass

        # Make plots to store the epsilon squared values.
        minBin = min(t_epsilon) * 0.75
        maxBin = max(t_epsilon) * 1.25
        epsPlot = r.TH1D('m%i_epsilon' % (mass), '%i MeV #epsilon^{2};#epsilon^{2};Bin Count' % (mass), 100, minBin, maxBin)

        for eps2 in t_epsilon:
            epsPlot.Fill(eps2)
            pass

        # Save the plot.
        outFile.cd()
        epsPlot.Write()

        # Sort the lists.
        t_epsilon.sort()

        # Get the indices for each value type.
        i_mean = int(math.floor(len(t_epsilon) / 2))
        i_1sigma_u = i_mean + int(math.floor(len(t_epsilon) * 0.341))
        i_2sigma_u = i_mean + int(math.floor(len(t_epsilon) * 0.477))
        i_1sigma_l = i_mean - int(math.floor(len(t_epsilon) * 0.341))
        i_2sigma_l = i_mean - int(math.floor(len(t_epsilon) * 0.477))

        # Calculate epsilon at this point and store the values.
        toyEpsilon.append(t_epsilon[i_mean])
        toyEpsilonEU1.append(t_epsilon[i_1sigma_u] - t_epsilon[i_mean])
        toyEpsilonEU2.append(t_epsilon[i_2sigma_u] - t_epsilon[i_mean])
        toyEpsilonEL1.append(t_epsilon[i_mean] - t_epsilon[i_1sigma_l])
        toyEpsilonEL2.append(t_epsilon[i_mean] - t_epsilon[i_2sigma_l])

        print('%i MeV :: %f;    %f <-- %f <-- %f --> %f --> %f' % (mass, limitEps[len(limitEps) - 1], t_epsilon[i_2sigma_l], t_epsilon[i_1sigma_l], t_epsilon[i_mean], t_epsilon[i_1sigma_u], t_epsilon[i_2sigma_u]))
        pass
    inFile.Close()
    pass

# Save and draw the pull plots.
outFile.cd()
utils.SetStyle()
sigPull.SetLineColor(utils.colors[0])
sigPull.SetLineWidth(3)
sigPull.GetXaxis().SetTitleOffset(1.2)
sigPull.GetYaxis().SetTitleOffset(1.0)
sigPull.Write()

utils.InsertText("", [], 0.15, 0.15)
massPullCanvas = r.TCanvas("massPull_canvas", "massPull_canvas", 2500, 1000)
sigPull.Draw()
massPullCanvas.SaveAs('%s/%smassPull.png' % (plotDirectory, prefix))

# Make the p-value plots.
pSearch_g = r.TGraph(len(massFloats), np.array(massFloats), np.array(pSearch))
pSearch_g.SetName("pSearch_g")
pSearch_g.SetTitle("pSearch_g;m_{A'} (MeV);Local p-Value")
pSearchG_g = r.TGraph(len(massFloats), np.array(massFloats), np.array(pSearchG))
pSearchG_g.SetName("pSearchG_g")
pSearchG_g.SetTitle("pSearch_g;m_{A'} (MeV);Global p-Value")

utils.InsertText("", [], 0.15, 0.15)
pValueCanvas = r.TCanvas("pValueCanvas", "pValueCanvas", 2500, 1000)
pValueCanvas.SetLogy(1)
pSearch_g.Draw()
pValueCanvas.SaveAs('%s/%spValue.png' % (plotDirectory, prefix))

# Make the upper limit of the signal yield plot.
limitN_g = r.TGraph(len(massFloats), np.array(massFloats), np.array(limitN))
limitN_g.SetName("limitN_g")
limitN_g.SetTitle("limitN_g;m_{A'} (MeV);N_{up}")

utils.InsertText("", [], 0.15, 0.15)
sigYieldCanvas = r.TCanvas("sigYieldCanvas", "sigYieldCanvas", 2500, 1000)
sigYieldCanvas.SetLogy(1)
limitN_g.Draw()
sigYieldCanvas.SaveAs('%s/%ssigYieldUpperLimit.png' % (plotDirectory, prefix))

# Make a canvas.
canvas = r.TCanvas("ep2_canvas", "ep2_canvas", 2500, 1000)
canvas.SetLogy(1)
canvas.cd()

# Make an array of zero for the "x error."
xErrors = []
for entry in massFloats:
    xErrors.append(0)
    pass

# Make the toy epsilon squared range plot.
limitEps_s2_g = r.TGraphAsymmErrors(len(massFloats), np.array(massFloats), np.array(toyEpsilon), np.array(xErrors), np.array(xErrors), np.array(toyEpsilonEL2), np.array(toyEpsilonEU2))
limitEps_s2_g.SetName("limitEps_s2_g")
limitEps_s2_g.SetTitle("limitEps_s1_g;m_{A'} (MeV);#epsilon^{2}_{up}")
limitEps_s2_g.SetFillColor(r.kOrange-2)
limitEps_s2_g.GetYaxis().SetTitleOffset(1.4)
limitEps_s2_g.Draw("a3")

limitEps_s1_g = r.TGraphAsymmErrors(len(massFloats), np.array(massFloats), np.array(toyEpsilon), np.array(xErrors), np.array(xErrors), np.array(toyEpsilonEL1), np.array(toyEpsilonEU1))
limitEps_s1_g.SetName("limitEps_s1_g")
limitEps_s1_g.SetFillColor(r.kGreen-3)
limitEps_s1_g.Draw("Same3l")

# Make the data epsilon squared plot.
limitEps_g = r.TGraph(len(massFloats), np.array(massFloats), np.array(limitEps))
limitEps_g.SetName("limitEps_g")
limitEps_g.SetLineWidth(3)
limitEps_g.Draw("Same")

# Make the toy median epsilon squared plot.
limitEps_toy_g = r.TGraph(len(massFloats), np.array(massFloats), np.array(toyEpsilon))
limitEps_toy_g.SetName("limitEps_toy_g")
limitEps_toy_g.SetLineWidth(2)
limitEps_toy_g.SetLineStyle(2)
limitEps_toy_g.Draw("Same")

# Save the epsilon limit plot.
utils.InsertText("", [], 0.15, 0.15)
canvas.SaveAs('%s/%sepsilonSquaredLimit.png' % (plotDirectory, prefix))
canvas.Write()

# Save the files.
outFile.cd()
pSearch_g.Write()
pSearchG_g.Write()
limitN_g.Write()
limitEps_g.Write()
limitEps_s1_g.Write()
limitEps_s2_g.Write()
limitEps_toy_g.Write()
outFile.Close()
