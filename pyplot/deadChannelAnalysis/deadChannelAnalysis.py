import ROOT as r
import numpy as np
import argparse
from copy import deepcopy
import utilities as utils
import svtCondAnaTools as svtc
from collections import defaultdict


parser = argparse.ArgumentParser(description = "Config arguments")
parser.add_argument('--infile','-i', type=str, dest="inFilename", metavar='infile',
                            help="Input file")
parser.add_argument("-o", "--outFile", type=str, dest="outFilename", action='store',
                          help="Output filename.", metavar="outFilename", default="outDeadChAnalysis.root")

options = parser.parse_args()

inFile = r.TFile(options.inFilename)
outFile = r.TFile(options.outFilename,"RECREATE")


histo_names=svtc.BuildHybridKeys("raw_hits_baseline0_","hh")
baselineHistos=svtc.DeepCopy(inFile, histo_names)
outFile.cd()
for hybrid,hh in baselineHistos.items():
    histos1d = svtc.getYprojections(hh)
    hybridRMS = {}
    channels = []
    rmsvals = []
    for cc, yproj in histos1d.items()  :
        hybridRMS[cc] = svtc.getRMS(yproj)
        channels.append(cc)
        rmsvals.append(hybridRMS[cc])
    svtc.DrawGraph(outFile, hybrid, hybrid +"StripNumber;RMS [ADC]",640,rmsvals)

outFile.Close()
