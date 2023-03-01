#/bin/env python
import numpy as np
import ROOT as r
import utilities as utils
import copy
from optparse import OptionParser

Lumi = 110  # 1/pb

utils.SetStyle()

parser = OptionParser()

parser.add_option("-i", "--inputFile", type="string", dest="inputFile",
                  help="Name of file to run on.", metavar="inputFile", default="toys/toys.root")
parser.add_option("-o", "--outputFile", type="string", dest="outputFile",
                  help="Specify the output filename.", metavar="outputFile", default="testOut.root")

(options, args) = parser.parse_args()
outfile = r.TFile("%s" % (options.outputFile), "RECREATE")

invMassHistos = {}
#Rad SLIC MC
inFile = r.TFile("/sdf/group/hps/users/alspellm/projects/simps_2019/mc/rad_beam/hadd_rad_beam_slic_ana.root")  # Use Rad Slic analysis
invMassHistos['rad_slic'] = copy.deepcopy(inFile.Get("mcAna/mcAna_mc622Mass_h"))
inFile.Close()

#Rad+Beam MC
inFile = r.TFile("/sdf/group/hps/users/alspellm/projects/simps_2019/mc/rad_beam/hadd_rad_beam_recon_ana.root")  # Use Rad+Beam analysis
invMassHistos['rad'] = copy.deepcopy(inFile.Get("vtxana_kf_radMatchTight_2019_simpCR/vtxana_kf_radMatchTight_2019_simpCR_mcMass622_h"))  # Use Control Region radMatchTight mcMass622_h
inFile.Close()

outfile.cd()

#Fix the x axis scaling of rad hist (if needed)
name = invMassHistos['rad'].GetName()
invMassHistos['rad'].SetName("needs_rescaling")
rad_mev_h = r.TH1F("rescale_rad", "rescale_rad", 500, 0., 500.)
nbins = invMassHistos['rad'].GetXaxis().GetNbins()
for b in range(nbins):
    val = invMassHistos['rad'].GetBinContent(b+1)
    rad_mev_h.SetBinContent(b+1, val)
rad_mev_h.SetName(name)
invMassHistos['rad'] = rad_mev_h

#Scale the histograms
rebin = 1
units = 1
invMassHistos['rad_slic'].Rebin(rebin)
#MC Scale
invMassHistos['rad_slic'].Scale(units*2.539e07*Lumi/(rebin*10000*976))
invMassHistos['rad'].Scale(units*2.539e07*Lumi/(rebin*10000*976))

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

#Result fit to 5th order polynomial
# radF = -7.77721e-1 + 5.67200e-2*mass + -1.67381e-3*math.pow(mass,2) + 2.60149e-05*math.pow(mass,3) + -2.35378e-7*math.pow(mass,4) + 1.31046e-9*math.pow(mass,5) + -4.56049e-12*math.pow(mass,6) + 9.67101e-15*math.pow(mass,7) + -1.14284e-17*math.pow(mass,8) + 5.76861e-21*math.pow(mass,9)
