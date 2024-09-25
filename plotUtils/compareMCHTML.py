import ROOT as r
import os
import json
import utilities as utils 

import argparse 

# define a function to extract the name for the file
def extractName(filePath):
    return os.path.splitext(os.path.basename(filePath))[0]

# example command line: hpstr /Users/zhangyangwu/workspace/hpstr/plotUtils/compareHTML.py -i ~/Documents/compareHTML_config.json -o test_script
parser = argparse.ArgumentParser(description="The baseConfig options for compareKalGBL. ")

parser.add_argument('--debug', '-D', # increase debug level
                     action  = "count", 
                     dest    = "debug", 
                     help    = "Increase debug level.", 
                     default = 0)

parser.add_argument("-i", "--inFile", # either statement activate the action of inputting filename
                    dest    = "inFilename", 
                    help    = "Input filename.",
                    metavar = "inFilename",
                    default = None)

parser.add_argument("-o", "--outDir", # name of output directory 
                    dest    = "outDirectory",
                    help    = "Output directory.",
                    metavar = "outDirectory",
                    default = "output/")

options = parser.parse_args()
inFile = options.inFilename
outbase = options.outDirectory

# reading input file 
if inFile: # only if 'inFile' is provided
    f = open(inFile)
    data = json.load(f)
    scaling_factors = data['scaling_factors']
    inFileList = data['input_files']
    legends = data['legends']
    selection = data['selection']

# set the plotting style using the utility function 
utils.SetStyle()

# define a list of colors for plotting
colors = [r.kBlack, r.kRed, r.kBlue, r.kGreen+2, r.kOrange-2]

# creating output directory
if not os.path.exists(outbase):
    os.makedirs(outbase)

basedir = os.getcwd()

print("Making plots for ", selection)

# loop through each selection sel 
for sel in selection:
    print("plotting selection " + sel)
    outdir = os.path.join(outbase, sel)

    os.chdir(basedir)
    if not os.path.exists(outdir): # similar logic: create directory if not already existed 
        print("Making Directory " + outdir)
        os.makedirs(outdir)

    # set ROOT to batch mode
    TH1list = []
    TH2list = []
    # for i, histo in enumerate(TH1list):
    #     if i < len(scaling_factors):
    #         scale = scaling_factors[i]
    #         if isinstance(histo, r.TH1):
    #             histo.Scale(scale)
    #         else:
    #             print(f"Warning: Object at index {i} is not a histogram. Type: {type(histo)}")
    r.gROOT.SetBatch(1)
    
    # initialize a list to store input ROOT files 
    inputFiles = []
    # load root input files in inFileList
    for ifileVal in inFileList:
        # print("Loading file " + ifileVal)
        # open the ROOT file
        inf = r.TFile(ifileVal)
        # add the root file to the inputList created earlier 
        inputFiles.append(inf)

    # Plotting Hiostograms 
    # --------------------------------
    canvs = [] # initialize a list to store canvases 

    # get list of plots and directories in root file
    keynames = []
    for key in inputFiles[0].GetListOfKeys(): # go through the list of keys in the first ROOT file 
        keyname = key.GetName()
        
        if inputFiles[0].Get(keyname).InheritsFrom("TH1"): # check if the key is a 1D histogram 
            TH1list = [] 
            c = r.TCanvas() # create a new canvas
            # add histograms from each inputFiles to the TH1list
            for inf in inputFiles: 
                TH1list.append(inf.Get(keyname))        
            # generate and append the plot to canvs
            canvs.append(utils.MakeStackPlot(keyname, outdir, TH1list, legends, ".png",
                                             LogY = False, Normalise = True, scaling_factors = scaling_factors))

        # check the diagram and make sure it is a 2D histogram 
        elif inputFiles[0].Get(keyname).InheritsFrom("TH2"):
            TH2list = []
            xtitle = []
            ytitle = []
            name = []
            # create a new can
            c = r.TCanvas()
            # add histograms from each input file to the list 
            for i in range(0, len(inputFiles)):
                inf = inputFiles[i]
                histo = inf.Get(keyname)
                TH2list.append(histo)
                xtitle.append(histo.GetXaxis().GetTitle())
                ytitle.append(histo.GetYaxis().GetTitle())
                name.append(keyname + '_' + legends[i])
            
            canvs.append(utils.Make2DPlots(keyname, outdir, TH2list, xtitle, ytitle)) # generate and save the 2D plot             

        else:
            for subkey in inputFiles[0].Get(keyname).GetListOfKeys():
                subkeyname = subkey.GetName()
                if inputFiles[0].Get(keyname).Get(subkeyname).InheritsFrom("TH1"):
                    TH1list = []
                    c = r.TCanvas()
                    for inf in inputFiles:
                        TH1list.append(inf.Get(keyname).Get(subkeyname))
                    canvs.append(utils.MakeStackPlot(subkeyname, outdir, TH1list, legends, ".png",
                                                     LogY = False, Normalise = True, scaling_factors = scaling_factors))

                elif inputFiles[0].Get(keyname).Get(subkeyname).InheritsFrom("TH2"):
                    TH2list = []
                    xtitle = []
                    ytitle = []
                    c = r.TCanvas()
                    for i in range(0, len(inputFiles)):
                        inf = inputFiles[i]
                        histo = inf.Get(keyname).Get(subkeyname)
                        TH2list.append(histo)
                        xtitle.append(histo.GetXaxis().GetTitle())
                        ytitle.append(histo.GetYaxis().GetTitle())
                    canvs.append(utils.Make2DPlots(subkeyname, outdir, TH2list, xtitle, ytitle))


    utils.makeHTML(outdir,'test ('+sel+')', selection)
    outF = r.TFile("file.root","RECREATE")
    outF.cd()
    for canv in canvs:
        if canv != None: canv.Write()
    outF.Close()
