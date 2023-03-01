import ROOT as r
import plotting
import utilities as utils
import os
import sys

import json

from plotVars import plot_vars

utils.SetStyle()

path = "/nfs/slac/g/hps3/users/pbutti/hpstr_histos/"

#nb-1
Lumi = 1100.861649
ScaleFactor = 0.6
Lumi = Lumi*ScaleFactor
Selections = ["vtxSelection"]

#Samples dictionary: key is the name of the sample, value is the location of the file
#They *must* be the same of the normalization file at the moment. Histos are not weighted yet.

samplesDict = {
    "RADBeam": path+"rad/radTuple.root",
    "wabBeam": path+"wab/wabTuple.root",
    "tritrigBeam": path+"tritrig/tritrigTuple.root"
}

samplesToStack = ["wabBeam", "tritrigBeam"]
samplesNotToStack = ["RADBeam"]
dataSample = {"data": path+"data_10percent/data.root"}
MCOnly = False

#Make the histograms Dictionary
hists = {}

#Get the normalizations of the samples
print "Loading normalizations..."
norm_json = open(os.environ["HPSTR_BASE"] + "/analysis/data/samples_xsections.json")
norms = json.load(norm_json)
norm_dict = {}
#Get the normalization factors
for entry in norms:
    normFactor = (norms[entry]["xsec"] * Lumi) / norms[entry]["nGen"]
    normFactorE = (norms[entry]["xsecerr"] * Lumi) / norms[entry]["nGen"]
    norm_dict[entry] = normFactor
print norm_dict


#Get the plot json file  - This could be the loop on the file keys instead??
#Well, no if I want to change the hist title/min-max

print "Loading plots..."
plots_json = open(os.environ["HPSTR_BASE"] + "/analysis/plotconfigs/tracking/vtxAnalysis.json")
plots = json.load(plots_json)

outF = r.TFile("allHistos.root", "RECREATE")

for sel in Selections:

    print "Making plots for selection:", sel
    for var in plot_vars.keys():
        rebinFactor = plot_vars[var]

        stackHists = []
        hists = []
        norms = []

        data_histo = None

        if (not MCOnly):
            data_histo = utils.getPlot(dataSample["data"], sel+"/"+sel+"_"+var)

        for sample in samplesDict:
            histo = utils.getPlot(samplesDict[sample], sel+"/"+sel+"_"+var)
            histo.Rebin(rebinFactor)
            histo.SetName(sample)

            if sample in samplesToStack:
                stackHists.append(histo)
                norms.append(norm_dict[sample])
            else:
                #Scale the histogram that is not in the list for stacking
                histo.Scale(1./norm_dict[sample])
                hists.append(histo)

        stack_tot = plotting.prepare1DStack(stackHists, norms)

        stack = stack_tot[0]
        h_tot = stack_tot[1]

        outF.cd()
        c = r.TCanvas(var, var, 600, 600)

        top = r.TPad("top", "top", 0, 0.42, 1, 1)
        bot = r.TPad("bot", "bot", 0, 0, 1, 0.38)

        if (not MCOnly):
            top.Draw()
            top.SetBottomMargin(0)
            top.SetTopMargin(r.gStyle.GetPadTopMargin()*utils.topScale)
            bot.Draw()
            bot.SetTopMargin(0)
            bot.SetBottomMargin(0.4)
            top.cd()

        stack.Draw("histo")
        binWidth = stack.GetXaxis().GetBinWidth(1)
        if (var != "cutflow"):
            stack.GetXaxis().SetTitle(plots[var]["xtitle"])
            stack.GetYaxis().SetTitle(plots[var]["ytitle"] + "/"+str(binWidth))
        else:
            stack.GetXaxis().SetTitle("cutflow")
            stack.GetYaxis().SetTitle("vertices")
        #Plot histos not to be stacked

        for i_hist in range(len(hists)):
            hists[i_hist].SetMarkerColor(utils.blueColors[i_hist])
            hists[i_hist].SetLineColor(utils.blueColors[i_hist])
            hists[i_hist].Draw("same")

        #Plot total histo

        h_tot.SetMarkerColor(r.kRed)
        h_tot.SetLineColor(r.kRed)
        h_tot.SetLineWidth(2)
        h_tot.Draw("histosame")

        if (not MCOnly):
            data_histo.SetMarkerColor(r.kBlack)
            data_histo.SetLineWidth(2)
            data_histo.SetMarkerSize(20)
            data_histo.Draw("E0 SAME")

        #Make legend - TODO make it configurable from json

        leg = r.TLegend(0.65, 0.9, 0.9, 0.65)
        leg.SetBorderSize(0)

        #Add Data
        if (not MCOnly):
            leg.AddEntry(data_histo, "Data 2016 [0.1L]", "lp")

        #Add total MC

        leg.AddEntry(h_tot, "MC Simulation", "l")

        for h in stackHists:
            leg.AddEntry(h, h.GetName(), "lpf")

        #Add legends for non-stacked hists
        for h in hists:
            leg.AddEntry(h, h.GetName(), "l")

        leg.Draw()

        #HPS Internal
        utils.InsertText()

        #Make Ratio
        if (not MCOnly):
            bot.cd()
            bot.SetGridy()
            reference = h_tot.Clone("reference")
            reference.GetYaxis().SetTitle("Ratio")
            #reference.GetYaxis().SetTitleSize(0.06)
            #reference.GetXaxis().SetTitleSize(0.1)
            #reference.GetXaxis().SetLabelSize(0.12)

            reference.Draw("axis")

            data_over_mc = data_histo.Clone()
            data_over_mc.SetTitle("")
            data_over_mc.Divide(reference)
            data_over_mc.Draw("pe")
            data_over_mc.GetYaxis().SetTitleSize(0.075)
            y_ratio_offset = data_over_mc.GetYaxis().GetTitleOffset()
            data_over_mc.GetYaxis().SetTitleOffset(y_ratio_offset*0.5)
            data_over_mc.GetXaxis().SetTitleSize(0.1)
            data_over_mc.GetXaxis().SetLabelSize(0.12)
            data_over_mc.GetYaxis().SetRangeUser(0.1, 2)
            data_over_mc.GetYaxis().SetNdivisions(508)
            data_over_mc.GetYaxis().SetTitle("Data / MC")
            data_over_mc.GetYaxis().SetDecimals(True)

            #bot.RedrawAxis()
            #reference.Draw("axis same")

        c.Write()
        c.SaveAs(sel+"_"+var+".pdf")


outF.Close()
