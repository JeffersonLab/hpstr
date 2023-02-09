#/bin/env python
import numpy as np
import ROOT as r
import utilities as utils
import copy
from optparse import OptionParser

#Lumi = 11008.61649e-3 #1/pb
Lumi = 530.0

utils.SetStyle()

parser = OptionParser()

parser.add_option("-i", "--inputFile", type="string", dest="inputFile",
                  help="Name of file to run on.", metavar="inputFile", default="toys/toys.root")
parser.add_option("-o", "--outputFile", type="string", dest="outputFile",
                  help="Specify the output filename.", metavar="outputFile", default="testOut.root")

(options, args) = parser.parse_args()
outfile = r.TFile("%s" % (options.outputFile), "RECREATE")

#Rad SLIC
invMassHistos = {}
inFile = r.TFile("/sdf/group/hps/users/alspellm/projects/simps_3pt7/rad/slic/tuple_ana/hadd_rad_slic_ana.root")  # Use rad slic analysis
invMassHistos['rad_slic'] = copy.deepcopy(inFile.Get("mcAna/mcAna_mc622Mass_h"))
inFile.Close()

#Rad+Beam
inFile = r.TFile("/sdf/group/hps/users/alspellm/projects/simps_3pt7/rad/rad_beam/tuple_ana/hadd_rad_beam_ana.root")  # Use Rad+Beam Control Region analysis
invMassHistos['rad'] = copy.deepcopy(inFile.Get("vtxana_kf_radMatchTight_2021/vtxana_kf_radMatchTight_2021_mcMass622_h"))  # Use Control Region radMatchTight mcMass622
inFile.Close()

outfile.cd()

#Fix the x axis scaling of rad hist (if needed)
name = invMassHistos['rad'].GetName()
invMassHistos['rad'].SetName("needs_rescaling")
rad_mev_h = r.TH1F("rescale_rad", "rescale_rad", 450, 0., 450.)
nbins = invMassHistos['rad'].GetXaxis().GetNbins()
for b in range(nbins):
    val = invMassHistos['rad'].GetBinContent(b+1)
    rad_mev_h.SetBinContent(b+1, val)
rad_mev_h.SetName(name)
#rad_mev_h.SetTitle(invMassHistos['rad'].GetTitle())
invMassHistos['rad'] = rad_mev_h

#Scale the histograms
rebin = 1
units = 1
#invMassHistos['rad'].Rebin(rebin)
invMassHistos['rad_slic'].Rebin(rebin)
#MC Scale
invMassHistos['rad_slic'].Scale(units*3.480e07*Lumi/(rebin*10000*1000))
invMassHistos['rad'].Scale(units*3.480e07*Lumi/(rebin*10000*1000))

#Make N bins the same
invMassHistos['rad_slic'].GetXaxis().SetRange(1, 450)
invMassHistos['rad'].GetXaxis().SetRange(1, 450)
invMassHistos['rad_slic'].SetBins(450, 0.0, 450.0)
invMassHistos['rad'].SetBins(450, 0.0, 450.0)

print(invMassHistos['rad_slic'].GetNbinsX())
print(invMassHistos['rad'].GetNbinsX())

#Write histos
invMassHistos['rad_slic'].Write()
invMassHistos['rad'].Write()

canv = utils.MakeTotalRadAcc("zeta", ".", [invMassHistos['rad'], invMassHistos['rad_slic']], ['rad', 'rad_slic'], '.png', RatioMin=0.00, RatioMax=0.3, LogY=True)
canv.SaveAs("./totRadAccept.png")
outfile.Write()

#RATIO FIT TO 8th ORDER POLYNOMIAL
# totRadAcc = -7.40426e-01 + 5.72523e-02*mass + -1.74473e-03*math.pow(mass,2) + 2.69127e-05*math.pow(mass,3) + -2.28890e-07*math.pow(mass,4) + 1.12918e-09*math.pow(mass,5) + -3.23213e-12*math.pow(mass,6) + 4.99070e-15*math.pow(mass,7) + -3.21865e-18*math.pow(mass,8)
