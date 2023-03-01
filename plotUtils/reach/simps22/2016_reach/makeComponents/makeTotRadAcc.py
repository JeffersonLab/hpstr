#/bin/env python
import numpy as np
import ROOT as r
import utilities as utils
import copy
from optparse import OptionParser

Lumi = 10.7  # 1/pb

utils.SetStyle()

parser = OptionParser()

parser.add_option("-i", "--inputFile", type="string", dest="inputFile",
                  help="Name of file to run on.", metavar="inputFile", default="toys/toys.root")
parser.add_option("-o", "--outputFile", type="string", dest="outputFile",
                  help="Specify the output filename.", metavar="outputFile", default="radFrac.root")

(options, args) = parser.parse_args()
outfile = r.TFile("%s" % (options.outputFile), "RECREATE")

invMassHistos = {}
#RAD SLIC (NO BEAM)
inFile = r.TFile("/sdf/group/hps/users/alspellm/projects/THESIS/mc/2016/rad_beam/pass4_2016_mc/rerecon_kf_v5_1/simps_2016_kf/slic_ana/hadd_RADv3_MG5_noXchange_HPS-PhysicsRun2016-Pass2_ana.root")
invMassHistos['rad_slic'] = copy.deepcopy(inFile.Get("mcAna/mcAna_mc622Mass_h"))
inFile.Close()
#RAD+Beam in Control Region (truth matched using radMatch)
inFile = r.TFile("/sdf/group/hps/users/alspellm/projects/THESIS/mc/2016/rad_beam/pass4_2016_mc/rerecon_kf_v5_1/simps_2016_kf/ana/final_hadd_RADv3-beamv6_2500kBunches_HPS-PhysicsRun2016-Pass2_v4_5_0_pairs1_KF_ana_CR.root")
invMassHistos['rad'] = copy.deepcopy(inFile.Get("vtxana_kf_vertexSelection_radMatchTight_CR/vtxana_kf_vertexSelection_radMatchTight_CR_mcMass622_h"))
inFile.Close()

outfile.cd()

#Rad+Beam histogram x-axis here needed to be converted from GeV to MeV
name = invMassHistos['rad'].GetName()
invMassHistos['rad'].SetName("needs_rescaling")
rad_mev_h = r.TH1F("rescale_rad", "rescale_rad", 200, 0., 200.)
nbins = invMassHistos['rad'].GetXaxis().GetNbins()
for b in range(nbins):
    val = invMassHistos['rad'].GetBinContent(b+1)
    rad_mev_h.SetBinContent(b+1, val)
rad_mev_h.SetName(name)
invMassHistos['rad'] = rad_mev_h

#Scale the histograms
rebin = 1
units = 1  # 1/(2 MeV)
invMassHistos['rad_slic'].Rebin(rebin)
#MC Scale
invMassHistos['rad_slic'].Scale(units*66.36e6*Lumi/(rebin*10000*9959))
invMassHistos['rad'].Scale(units*66.36e6*Lumi/(rebin*10000*9959))

#Make N bins the same
invMassHistos['rad_slic'].GetXaxis().SetRange(1, 200)
invMassHistos['rad'].GetXaxis().SetRange(1, 200)
invMassHistos['rad_slic'].SetBins(200, 0.0, 200.0)
invMassHistos['rad'].SetBins(200, 0.0, 200.0)

#Write histos
invMassHistos['rad_slic'].Write()
invMassHistos['rad'].Write()

canv = utils.MakeTotalRadAcc("totRadAcceptance", ".", [invMassHistos['rad'], invMassHistos['rad_slic']], ['rad', 'rad_slic'], '.png', RatioMin=0.00, RatioMax=0.3, LogY=True)
#canv.SaveAs("./totRadAccept.png")
outfile.Write()

#2016 Simps KF  11/15/22
'''
 FCN=186.124 FROM MINOS     STATUS=FAILURE      1093 CALLS       12322 TOTAL
                     EDM=5.73317e-15    STRATEGY= 1      ERR MATRIX NOT POS-DEF
  EXT PARAMETER                APPROXIMATE        STEP         FIRST
  NO.   NAME      VALUE            ERROR          SIZE      DERIVATIVE
   1  p0          -7.35934e-01   2.50819e-05  -6.01459e-06   5.98050e+01
   2  p1           9.75402e-02   9.63321e-07   3.34510e-07   4.15551e+03
   3  p2          -5.22599e-03   2.86350e-08  -1.41372e-09   4.80059e+04
   4  p3           1.47226e-04   5.29319e-10  -1.78016e-10   4.07836e+06
   5  p4          -2.41435e-06   4.65925e-12   2.59369e-12   4.13920e+08
   6  p5           2.45015e-08   2.87095e-14   7.62367e-16  -8.77231e+09
   7  p6          -1.56938e-10   1.65152e-16  -1.19289e-16   4.47449e+12
   8  p7           6.19494e-13   9.04505e-19  -3.72581e-19   2.75638e+15
   9  p8          -1.37780e-15   4.61557e-21   7.82382e-21   3.20438e+16
  10  p9           1.32155e-18   2.07422e-23   2.07422e-23   6.66020e+14
'''
