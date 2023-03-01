import ROOT as r
from copy import deepcopy
import numpy as np
from optparse import OptionParser

parser = OptionParser()

parser.add_option("-i", type="string", dest="inFile", default="")
(options, args) = parser.parse_args()

inFilename = options.inFile
inFile = r.TFile(inFilename, "READ")
invm_sigma_h = inFile.Get("vtxana_kf_vertexSelection_Tight_SR_vtx_InvM_vtx_svt_z_hh_sigma")

frl = invm_sigma_h.GetXaxis().GetBinLowEdge(invm_sigma_h.FindFirstBinAbove(0))
fru = invm_sigma_h.GetXaxis().GetBinLowEdge(invm_sigma_h.FindLastBinAbove(0))

outfile = r.TFile("vtxResPlot_SR.root", "RECREATE")
outfile.cd()

canv = r.TCanvas("canv", "canv", 1800, 1000)
canv.cd()

invm_sigma_h.Fit("pol3", "ES", "", 0.030, 0.21)
invm_sigma_h.Draw()
invm_sigma_h.Write()
canv.SaveAs("vtxResPlot_SR.png")


#2016 KF using SR Tritrig 11/15/22
'''
 FCN=504.205 FROM MINOS     STATUS=SUCCESSFUL     28 CALLS         233 TOTAL
                     EDM=2.47199e-17    STRATEGY= 1      ERROR MATRIX ACCURATE
  EXT PARAMETER                                   STEP         FIRST
  NO.   NAME      VALUE            ERROR          SIZE      DERIVATIVE
   1  p0           8.09149e+00   3.72169e-02  -1.29232e-06  -1.41572e-08
   2  p1          -1.54072e+02   1.54838e+00   3.84205e-05  -1.20168e-09
   3  p2           1.25624e+03   1.94377e+01  -2.45593e-04  -4.84652e-11
   4  p3          -3.43499e+03   7.30905e+01   7.30905e+01   2.12696e-12
'''
#using CR Tritrig
'''
FCN=195.204 FROM MINOS     STATUS=SUCCESSFUL     28 CALLS         230 TOTAL
                     EDM=4.55793e-16    STRATEGY= 1      ERROR MATRIX ACCURATE
  EXT PARAMETER                                   STEP         FIRST
  NO.   NAME      VALUE            ERROR          SIZE      DERIVATIVE
   1  p0           6.02623e+00   6.38298e-02   4.11895e-06   5.66344e-09
   2  p1          -8.28310e+01   1.96155e+00  -8.90618e-05   1.84059e-09
   3  p2           4.96606e+02   1.86762e+01   4.33421e-04  -1.98880e-10
   4  p3          -1.00976e+03   5.49186e+01   5.49186e+01  -1.12150e-11
'''
