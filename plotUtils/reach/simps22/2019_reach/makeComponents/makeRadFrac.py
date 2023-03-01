#/bin/env python
import numpy as np
import ROOT as r
import utilities as utils
import copy
from optparse import OptionParser

#Lumi = 11008.61649e-3 #1/pb
Lumi = 110.

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
inFile = r.TFile("/sdf/group/hps/users/alspellm/projects/simps_2019/mc/rad_beam/hadd_rad_beam_recon_ana.root")  # Use Control Region analysis
invMassHistos['rad'] = copy.deepcopy(inFile.Get("vtxana_kf_radMatchTight_2019_simpCR/vtxana_kf_radMatchTight_2019_simpCR_mcMass622_h"))  # Use Control Region radMatchTight mcMass622_h
inFile.Close()

#Tritrig+Beam MC
inFile = r.TFile("/sdf/group/hps/users/alspellm/projects/simps_2019/mc/tritrig_beam/hadd_tritrig_beam_ana_CR.root")  # Use Control Region analysis
invMassHistos['tritrig'] = copy.deepcopy(inFile.Get("vtxana_kf_Tight_2019_simpCR/vtxana_kf_Tight_2019_simpCR_vtx_InvM_h"))  # Control Region Tight vtx_InvM_h

#Wab+Beam MC
inFile.Close()
inFile = r.TFile("/sdf/group/hps/users/alspellm/projects/simps_2019/mc/wab_beam/hadd_wab_beam_ana_CR.root")  # Use Control Region analysis
invMassHistos['wab'] = copy.deepcopy(inFile.Get("vtxana_kf_Tight_2019_simpCR/vtxana_kf_Tight_2019_simpCR_vtx_InvM_h"))  # Control Region Tight vtx_invM_h
inFile.Close()

outfile.cd()
#Convert x axis scaling from GeV to MeV (if necessary)
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
invMassHistos['rad'].Scale(units*2.539e07*Lumi/(rebin*10000*976))
#invMassHistos['tritrig'].Scale(units*4.566e08*Lumi/(rebin*10000*1425))  #pass0, NOT pass0d
invMassHistos['tritrig'].Scale(units*6.610e08*Lumi/(rebin*10000*1425))
invMassHistos['wab'].Scale(units*4.715e10*Lumi/(rebin*10000*9944))

canv = utils.MakeRadFrac("radFrac", ".", [invMassHistos['rad'], invMassHistos['wab'], invMassHistos['tritrig']], ['rad', 'wab', 'tritrig+wab'], '.png', RatioMin=0.00, RatioMax=0.3, LogY=True)

#Result fit to 5th order polynomial
# radFrac = -4.02992e-01 + 1.25999e-02*mass + -1.28959e-04*math.pow(mass,2) + 6.30470e-07*math.pow(mass,3) + -1.46441e-9*math.pow(mass,4) + 1.28540e-12*math.pow(mass,5)  THIS IS IF USING PASS0 TRITRIG XSECTION

# radFrac = -3.03024e-01 + 9.48279e-03*mass + -9.74867e-05*math.pow(mass,2) + 4.78994e-07*math.pow(mass,3) + -1.12040e-09*math.pow(mass,4) + 9.92700e-13*math.pow(mass,5)  PASS0D TRITRIG
