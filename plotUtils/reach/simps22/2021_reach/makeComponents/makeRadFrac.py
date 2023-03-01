#/bin/env python
import numpy as np
import ROOT as r
import utilities as utils
import copy
from optparse import OptionParser

print("running")
Lumi = 168.1  # 1/pb

utils.SetStyle()

parser = OptionParser()

parser.add_option("-i", "--inputFile", type="string", dest="inputFile",
                  help="Name of file to run on.", metavar="inputFile", default="toys/toys.root")
parser.add_option("-o", "--outputFile", type="string", dest="outputFile",
                  help="Specify the output filename.", metavar="outputFile", default="testOut.root")

(options, args) = parser.parse_args()
outfile = r.TFile("%s" % (options.outputFile), "RECREATE")

invMassHistos = {}
#Rad+Beam MC
inFile = r.TFile("/sdf/group/hps/users/alspellm/projects/simps_3pt7/rad/rad_beam/tuple_ana/hadd_rad_beam_ana.root")  # Use Control Region analysis
invMassHistos['rad'] = copy.deepcopy(inFile.Get("vtxana_kf_radMatchTight_2021/vtxana_kf_radMatchTight_2021_mcMass622_h"))  # Use Control Region radMatchTight_mcMass622_h
inFile.Close()

#Tritrig+Beam MC
inFile = r.TFile("/sdf/group/hps/users/alspellm/projects/simps_3pt7/tritrig_beam/tuple_ana/CR/hadd_tritrig_beam_ana.root")  # Use Control Region analysis
invMassHistos['tritrig'] = copy.deepcopy(inFile.Get("vtxana_kf_Tight_2021/vtxana_kf_Tight_2021_vtx_InvM_h"))  # Use Control Region vtx_InvM_h
inFile.Close()
inFile = r.TFile("/sdf/group/hps/users/alspellm/projects/simps_3pt7/wab_beam/super_hadd_wab_beam_CR_ana.root")  # Use Control Region analysis
invMassHistos['wab'] = copy.deepcopy(inFile.Get("vtxana_kf_Tight_2021/vtxana_kf_Tight_2021_vtx_InvM_h"))  # Use Control Region vtx_InvM_h
inFile.Close()

outfile.cd()
#Convert x axis scaling from GeV to MeV (If Needed)
for histo in invMassHistos.keys():
    name = invMassHistos[histo].GetName()
    invMassHistos[histo].SetName("need_to_rescale_%s" % (name))
    rescaled_h = r.TH1F("rescaled_%s" % (name), "rescaled_%s" % (name), 400, 0., 400.)
    nbins = invMassHistos[histo].GetXaxis().GetNbins()
    for b in range(nbins):
        val = invMassHistos[histo].GetBinContent(b+1)
        rescaled_h.SetBinContent(b+1, val)
    rescaled_h.SetName(name)
    invMassHistos[histo] = rescaled_h
    print("%s N bins = %i" % (histo, invMassHistos[histo].GetXaxis().GetNbins()))

#Scale the histograms
rebin = 1
units = 1
invMassHistos['rad'].Rebin(rebin)
invMassHistos['tritrig'].Rebin(rebin)
invMassHistos['wab'].Rebin(rebin)
invMassHistos['rad'].Scale(units*3.480e07*Lumi/(rebin*10000*1000))
invMassHistos['tritrig'].Scale(units*6.853e08*Lumi/(rebin*10000*993))
invMassHistos['wab'].Scale(units*7.557e10*Lumi/(rebin*(30000*3333 + 1405*40000 + 907*40000)))

invMassHistos['rad'].SetName("rad_beam")
invMassHistos['tritrig'].SetName("tritrig_beam")
invMassHistos['wab'].SetName("wab_beam")

invMassHistos['rad'].Write()
invMassHistos['tritrig'].Write()
invMassHistos['wab'].Write()

canv = utils.MakeRadFrac("radFrac", ".", [invMassHistos['rad'], invMassHistos['wab'], invMassHistos['tritrig']], ['rad', 'wab', 'tritrig+wab'], '.png', RatioMin=0.00, RatioMax=0.3, LogY=True)

#RESULT FIT TO 7th ORDER POLYNOMIAL
# radFrac = -8.95377e-01 + 4.15313e-02*mass + -6.92406e-04*math.pow(mass,2) + 6.01862e-06*math.pow(mass,3) + -2.94263e-08*math.pow(mass,4) + 8.10924e-11*math.pow(mass,5) + -1.17466e-13*math.pow(mass,6) + 6.96371e-17*math.pow(mass,7)
