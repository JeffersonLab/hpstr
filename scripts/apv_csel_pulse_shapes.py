#!/bin/usr/python2
import numpy as np
import ROOT as r
import argparse
import glob
import os


def getHitTime(sample_number, csel_delay):
    print(sample_number)
    print(csel_delay)
    time = 3.125*(8-csel_delay) + 25.0*sample_number
    print(time)
    return time


parser = argparse.ArgumentParser(description="baseConfig options ")
parser.add_argument("-o", "--outFile", type=str, dest="outFilename", action='store',
                    help="Output filename.", metavar="outFilename", default="out.root")
parser.add_argument('--inDir', '-i', type=str, dest="inDir", action='store',
                    help="Specify the input directory.", metavar="outDir", default=".")
parser.add_argument('--outDir', '-d', type=str, dest="outDir", action='store',
                    help="Specify the output directory.", metavar="outDir", default=".")
options = parser.parse_args()

indir = options.inDir
outdir = options.outDir
outfilename = options.outFilename

tsamples = ["smData0", "smData1", "smData2", "smData3", "smData4", "smData5"]

channel_tuples = []
for infilename in sorted(glob.iglob("%s/*del*_anaHD.root" % (indir))):
    name = os.path.splitext(infilename)[0]
    print(name)
    split = name.split("_")
    delay = int(split[2][-1])
    if delay == 0:
        continue
    group = int(split[1].replace("cg", ''))
    print("delay: ", delay)
    print("group: ", group)
    infile = r.TFile("%s" % (infilename), "READ")

    for key in infile.GetListOfKeys():
        keyname = key.GetName()
        for sample in tsamples:
            if sample in keyname:
                sampleN = int(sample[-1])
                time = getHitTime(sampleN, delay)
                h = infile.Get("%s" % (keyname))
                print(h.GetName())
                nbins = h.GetNbinsX()
                for b in range(1, nbins+1):
                    channel = b - 1
                    if channel % 8 != group:
                        continue
                    adcvals = []
                    adcentries = []
                    projy = h.ProjectionY("ch%s_projy" % (channel), b, b)
                    nbinsy = projy.GetNbinsX()
                    fb = projy.FindFirstBinAbove(0.0)
                    lb = projy.FindLastBinAbove(0.0)
                    for adcb in range(fb, lb):
                        val = projy.GetBinCenter(adcb)
                        entries = projy.GetBinContent(adcb)
                        adcvals.append(val)
                        adcentries.append(entries)
                    tup = (channel, time, adcvals, adcentries)
                    channel_tuples.append(tup)
    infile.Close()

#Loop over channel tuples to make time histograms for each channel
outfile = r.TFile("%s" % (outfilename), "RECREATE")
outfile.cd()
nchannels = 512

#for tup in channel_tuples:
timeSamplesN = 48
for cc in range(nchannels):
    print(cc)
    hh = r.TH2F("channel_%s_pulse_response" % (cc), "%s" % (cc), timeSamplesN, 0.0, 150.0, 10000, 0.0, 10000.0)
    for tup in channel_tuples:
        if tup[0] != cc:
            continue
        time = tup[1]
        adcs = tup[2]
        for i, adc in enumerate(adcs):
            entries = int(tup[3][i])
            for entry in range(entries):
                hh.Fill(time, adc, 1.0)
    hh.Write()

outfile.Write()


#
