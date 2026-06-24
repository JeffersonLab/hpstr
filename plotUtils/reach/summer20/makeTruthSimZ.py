#!/usr/bin/env python3
"""
Read 2021 A' truth MC trees and produce the generated vtx_z histograms
used as efficiency denominators in the reach calculation.

Output: one TH1F per mass named "apSimZ{mass}_h" in a single ROOT file.
"""
import os
import glob
import ROOT as r
from optparse import OptionParser

parser = OptionParser()
parser.add_option("-i", "--inputDir", type="string", dest="inputDir",
                  help="Directory containing ap{mass}/ subdirectories.",
                  metavar="inputDir",
                  default="/sdf/data/hps/users/mgignac/output/ap_cuts/root_files/pass5_v9")
parser.add_option("-o", "--outputFile", type="string", dest="outputFile",
                  help="Output ROOT file.", metavar="outputFile",
                  default="apSimZ_2021.root")
(options, args) = parser.parse_args()

masses = [60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180]

outFile = r.TFile(options.outputFile, "RECREATE")

for mass in masses:
    massDir = os.path.join(options.inputDir, "ap%i" % mass)
    files = glob.glob(os.path.join(massDir, "*.root"))
    if not files:
        print("WARNING: no files found for mass %i in %s, skipping" % (mass, massDir))
        continue

    chain = r.TChain("tree")
    for f in sorted(files):
        chain.Add(f)

    nEntries = chain.GetEntries()
    print("mass=%i MeV: %i entries from %i files" % (mass, nEntries, len(files)))

    h = r.TH1F("apSimZ%i_h" % mass, ";true z_{vtx} [mm];MC Events", 200, -50.0, 150.0)
    h.Sumw2()

    # zero out bins 195-200 to match the original script's truncation at ~138 mm
    chain.Draw("vtx_z>>apSimZ%i_h" % mass, "vtx_z < 138.0", "goff")

    outFile.cd()
    h.Write()
    print("  -> wrote apSimZ%i_h (%.0f entries in histogram)" % (mass, h.GetEntries()))

outFile.Close()
print("Done. Output: %s" % options.outputFile)
