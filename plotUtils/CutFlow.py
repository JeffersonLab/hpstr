#!/bin/env python

from tabulate import tabulate
import utilities as utils
import os
import sys
import ROOT as r


#Compute the efficiencies - TODO improve this.

def getEff(thisCut, previousCut=[], initialCut=[]):
    effs = []
    getFullEff = bool(len(initialCut))
    getRelEff = bool(len(previousCut))
    #The first is the cut name
    for ival in range(1, len(thisCut)):
        eff = -1.0
        eff2 = -1.0

        if (getRelEff):
            if (abs(previousCut[ival]) > 1e-8):
                print(thisCut[ival], previousCut[ival])
                eff = thisCut[ival] / previousCut[ival]
            else:
                eff = -1.0

        if (getFullEff):
            if (abs(initialCut[ival]) > 1e-8):
                eff2 = thisCut[ival] / initialCut[ival]
            else:
                eff2 = -1

        effs.append(thisCut[ival])

        if (getRelEff):
            if (eff == -1.0):
                effs.append("--")
            else:
                effs.append(round(eff, 3))

        if (getFullEff):
            if (eff2 == -1.0):
                effs.append("--")
            else:
                effs.append(round(eff2, 3))

    return effs


if __name__ == '__main__':

    #Get the normalisation file
    norm_json = open(os.environ["HPSTR_BASE"] + "/analysis/data/samples_xsections.json")
    norms = json.load(norm_json)
    norm_dict = {}
    #Get the normalization factors
    for entry in norms:
        normFactor = (norms[entry]["xsec"] * Lumi) / norms[entry]["nGen"]
        normFactorE = (norms[entry]["xsecerr"] * Lumi) / norms[entry]["nGen"]
        norm_dict[entry] = normFactor
    print norm_dict

    #Parse the command line
    parser = utils.parser

    (config, sys.argv[1:]) = parser.parse_args(sys.argv[1:])
    doRelEff = False
    doTotEff = True

    debug = config.debug

    #Get the inputFileList
    fList = []
    if (config.inputFiles):
        fList = (config.inputFiles).split(",")
        print("inputFiles:", fList)

    #Selection:
    sel = "vtxSelection"
    if (config.selection):
        sel = config.selection

    #Get the cutFlowHistograms and fill the yield entries
    hList = []
    nHists = 0

    for inF in fList:
        cf_h = utils.getPlot(inF, sel+"/"+sel+"_cutflow")
        hList.append(cf_h)
        nHists += 1

    #Get the cuts: assume all cut flows are the same for that particular selection
    nCuts = hList[0].GetXaxis().GetNbins()
    cuts = []
    for icut in range(1, nCuts+1):
        cut = hList[0].GetXaxis().GetBinLabel(icut)
        cuts.append(cut)

    #Fill the yield tables - Get one entry for each read in histogram
    table = []

    for icut in range(1, nCuts+1):
        cutEntry = []
        cutEntry.append(cuts[icut-1])
        for cf_h in hList:
            v_yield = cf_h.GetBinContent(icut)
            cutEntry.append(cf_h.GetBinContent(icut))
            #place holder for eff
            #cutEntry.append(-1)
        if (debug):
            print(cutEntry)
        table.append(cutEntry)

    table_wEffs = []
    #Get Efficiencies - TODO FIX
    for ientry in range(len(table)):
        print("table entry", table[ientry])
        yields = table[ientry][1:]
        print(yields)
        effs = [table[ientry][0]]
        if (ientry < 1):
            for yi in yields:
                effs.append(yi)
                if (doRelEff):
                    effs.append("--")
                if (doTotEff):
                    effs.append("--")
            table_wEffs.append(effs)
        else:
            if (debug):
                print("Eff: "+table[ientry][0] + "/"+table[ientry-1][0])

            prevCut = []
            initialCut = []
            if (doRelEff):
                prevCut = table[ientry-1]
            if (doTotEff):
                initialCut = table[0]

            effs += getEff(table[ientry], prevCut, initialCut)
            table_wEffs.append(effs)

    #effs out
    legends = (config.Legends).split(",")

    headers = []
    for leg in legends:
        headers.append(leg)
        if (doRelEff):
            headers.append("$\\epsilon_{rel}$")
        if (doTotEff):
            headers.append("$\\epsilon_{tot}$")

    #outfile=open("test.txt","w")
    #outfile.write(tabulate(table, headers, tablefmt="latex"))
    #outfile.close()

    print tabulate(table, headers, tablefmt="latex")
    print tabulate(table_wEffs, headers, tablefmt="latex")
