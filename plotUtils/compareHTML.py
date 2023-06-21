import ROOT as r
import os
import json
import utilities as utils

import argparse

parser = argparse.ArgumentParser(description="The baseConfig options for compareKalGBL. ")

parser.add_argument('--debug', '-D', action="count", dest="debug", help="Increase debug level.", default=0)

parser.add_argument("-i", "--inFile", dest="inFilename",
                    help="Input filename.", metavar="inFilename", default=None)
parser.add_argument("-o", "--outDir", dest="outDirectory",
                    help="Output directory.", metavar="outDirectory", default="output/")

print("test")
options = parser.parse_args()
inFile = options.inFilename
outbase = options.outDirectory

print(inFile)

if inFile:
    f = open(inFile)
    data = json.load(f)
    inFileList = data['input_files']
    legends = data['legends']
    selection = data['selection']

utils.SetStyle()

colors = [r.kBlack, r.kRed, r.kBlue, r.kGreen+2, r.kOrange-2]

if not os.path.exists(outbase):
    os.makedirs(outbase)

basedir=os.getcwd()

print("Making plots for ", selection)

for sel in selection:
    print("plotting selection " + sel)
    outdir = os.path.join(outbase, sel)

    os.chdir(basedir)
    if not os.path.exists(outdir):
        print("Making Directory " + outdir)
        os.makedirs(outdir)

    r.gROOT.SetBatch(1)
    inputFiles = []

    # load root input files
    for ifile in inFileList:
        print("Loading file " + ifile)
        inf = r.TFile(ifile)
        inputFiles.append(inf)

    canvs = []

    # get list of plots and directories in root file
    keynames = []
    for key in inputFiles[0].GetListOfKeys():
        keyname = key.GetName()
        if inputFiles[0].Get(keyname).InheritsFrom("TH1"):
            TH1list = []
            c = r.TCanvas()
            for inf in inputFiles:
                TH1list.append(inf.Get(keyname))
            canvs.append(utils.MakePlot(keyname,outdir,TH1list,legends,".png",LogY=False,RatioType="Sequential",Normalise=False))
        elif inputFiles[0].Get(keyname).InheritsFrom("TH2"):
            TH2list = []
            xtitle = []
            ytitle = []
            name = []
            c = r.TCanvas()
            for i in range(0, len(inputFiles)):
                inf = inputFiles[i]
                histo = inf.Get(keyname)
                TH2list.append(histo)
                xtitle.append(histo.GetXaxis().GetTitle())
                ytitle.append(histo.GetYaxis().GetTitle())
                name.append(keyname + '_' + legends[i])
                canvs.append(utils.Make2DPlots(name,outdir,TH2list,xtitle,ytitle))
        else:
            for subkey in inputFiles[0].Get(keyname).GetListOfKeys():
                subkeyname = subkey.GetName()
                if inputFiles[0].Get(keyname).Get(subkeyname).InheritsFrom("TH1"):
                    TH1list = []
                    c = r.TCanvas()
                    for inf in inputFiles:
                        TH1list.append(inf.Get(keyname).Get(subkeyname))
                    canvs.append(utils.MakePlot(subkeyname,outdir,TH1list,legends,".png",LogY=False,RatioType="Sequential",Normalise=False))
                elif inputFiles[0].Get(keyname).Get(subkeyname).InheritsFrom("TH2"):
                    TH2list = []
                    xtitle = []
                    ytitle = []
                    name = []
                    c = r.TCanvas()
                    for i in range(0, len(inputFiles)):
                        inf = inputFiles[i]
                        histo = inf.Get(keyname).Get(subkeyname)
                        TH2list.append(histo)
                        xtitle.append(histo.GetXaxis().GetTitle())
                        ytitle.append(histo.GetYaxis().GetTitle())
                        name.append(subkeyname + '_' + legends[i])
                        canvs.append(utils.Make2DPlots(name,outdir,TH2list,xtitle,ytitle))

    utils.makeHTML(outdir,'test ('+sel+')', selection)
    outF = r.TFile("file.root","RECREATE")
    outF.cd()
    for canv in canvs:
        if canv != None: canv.Write()
    outF.Close()
