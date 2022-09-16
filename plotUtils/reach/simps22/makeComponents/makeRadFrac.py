#/bin/env python
import numpy as np
import ROOT as r
import utilities as utils
import copy
from optparse import OptionParser

Lumi = 10.7 #1/pb

utils.SetStyle()

parser = OptionParser()

parser.add_option("-i", "--inputFile", type="string", dest="inputFile",
    help="Name of file to run on.", metavar="inputFile", default="toys/toys.root")
parser.add_option("-o", "--outputFile", type="string", dest="outputFile",
    help="Specify the output filename.", metavar="outputFile", default="testOut.root")

(options, args) = parser.parse_args()
outfile = r.TFile("%s"%(options.outputFile),"RECREATE")

invMassHistos = {}
inFile = r.TFile("/sdf/group/hps/users/alspellm/projects/THESIS/simp_reach_estimates/simps_2016/mc/rad/rad_beam/ana/hadd_rad_beam_ana.root")
invMassHistos['rad'] = copy.deepcopy(inFile.Get("vtxana_radMatchTight_simps/vtxana_radMatchTight_simps_mcMass622_h"))
inFile.Close()
inFile = r.TFile("/sdf/group/hps/users/alspellm/projects/THESIS/simp_reach_estimates/simps_2016/mc/tritrig_beam/ana/hadd_tritrigv2-beamv6_2500kBunches_HPS-PhysicsRun2016-Pass2_v4_5_0_pairs1_ana.root")
invMassHistos['tritrig'] = copy.deepcopy(inFile.Get("vtxana_Tight_simpCR/vtxana_Tight_simpCR_vtx_InvM_h"))
inFile.Close()
inFile = r.TFile("/sdf/group/hps/users/alspellm/projects/THESIS/simp_reach_estimates/simps_2016/mc/wab_beam/ana/hadd_wabv3-beamv6_2500kBunches_HPS-PhysicsRun2016-Pass2_v4_5_0_pairs1_ana.root")
invMassHistos['wab'] = copy.deepcopy(inFile.Get("vtxana_Tight_simpCR/vtxana_Tight_simpCR_vtx_InvM_h"))
inFile.Close()

outfile.cd()
#Convert x axis scaling from GeV to MeV
for histo in invMassHistos.keys():
    name = invMassHistos[histo].GetName()
    invMassHistos[histo].SetName("need_to_rescale_%s"%(name))
    rescaled_h = r.TH1F("rescaled_%s"%(name),"rescaled_%s"%(name),200,0.,200.)
    nbins = invMassHistos[histo].GetXaxis().GetNbins()
    for b in range(nbins):
        val = invMassHistos[histo].GetBinContent(b+1)
        rescaled_h.SetBinContent(b+1,val)
    rescaled_h.SetName(name)
    invMassHistos[histo] = rescaled_h

#Scale the histograms
rebin = 1
units = 1 #1 MeV
invMassHistos['rad'].Rebin(rebin)
invMassHistos['tritrig'].Rebin(rebin)
invMassHistos['wab'].Rebin(rebin)
invMassHistos['rad'].Scale(units*66.36e6*Lumi/(rebin*10000*9967))
invMassHistos['tritrig'].Scale(units*1.416e9*Lumi/(rebin*50000*9853))
invMassHistos['wab'].Scale(units*0.1985e12*Lumi/(rebin*100000*9966))

canv = utils.MakeRadFrac("radFrac", ".", [invMassHistos['rad'],invMassHistos['wab'],invMassHistos['tritrig']], ['rad', 'wab', 'tritrig+wab'],'.png', RatioMin=0.00, RatioMax=0.3, LogY=True)

