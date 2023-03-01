import ROOT as r
from copy import deepcopy
import numpy as np
from optparse import OptionParser

parser = OptionParser()

parser.add_option("-i", type="string", dest="inFile", default="")
(options, args) = parser.parse_args()

inFilename = options.inFile
inFile = r.TFile(inFilename, "READ")
invm_sigma_h = inFile.Get("vtxana_kf_Tight_2019_simpCR_vtx_InvM_vtx_svt_z_hh_sigma")

frl = invm_sigma_h.GetXaxis().GetBinLowEdge(invm_sigma_h.FindFirstBinAbove(0))
fru = invm_sigma_h.GetXaxis().GetBinLowEdge(invm_sigma_h.FindLastBinAbove(0))

canv = r.TCanvas("canv", "canv", 1800, 1000)
canv.cd()

invm_sigma_h.Fit("pol3", "ES", "", 0.04, 0.45)
invm_sigma_h.Draw()
canv.SaveAs("vtxRes.png")
