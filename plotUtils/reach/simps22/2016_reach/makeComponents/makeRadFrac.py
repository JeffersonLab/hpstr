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
#Control Region Rad+Beam MC
inFile = r.TFile("/sdf/group/hps/users/alspellm/projects/THESIS/mc/2016/rad_beam/pass4_2016_mc/rerecon_kf_v5_1/simps_2016_kf/final_hadd_RADv3-beamv6_2500kBunches_HPS-PhysicsRun2016-Pass2_v4_5_0_pairs1_KF_ana_CR.root")
invMassHistos['rad'] = copy.deepcopy(inFile.Get("vtxana_kf_vertexSelection_radMatchTight_CR/vtxana_kf_vertexSelection_radMatchTight_CR_mcMass622_h"))
inFile.Close()
#Control Region Tritrig+Beam MC
inFile = r.TFile("/sdf/group/hps/users/alspellm/projects/THESIS/mc/2016/tritrig_beam/pass4_2016_mc/rerecon_kf_v5_1/simps_2016_kf/final_hadd_tritrigv2-beamv6_2500kBunches_HPS-PhysicsRun2016-Pass2_v4_5_0_pairs1_976_KF_CR.root")
invMassHistos['tritrig'] = copy.deepcopy(inFile.Get("vtxana_kf_vertexSelection_Tight_CR/vtxana_kf_vertexSelection_Tight_CR_vtx_InvM_h"))
inFile.Close()
#Control Region WAB+Beam MC
inFile = r.TFile("/sdf/group/hps/users/alspellm/projects/THESIS/mc/2016/wab_beam/pass4_2016_mc/rerecon_kf_v5_1/simps_2016_kf/final_hadd_wabv3-beamv6_2500kBunches_HPS-PhysicsRun2016-Pass2_v4_5_0_pairs1_KF_ana_CR.root")
invMassHistos['wab'] = copy.deepcopy(inFile.Get("vtxana_kf_vertexSelection_Tight_CR/vtxana_kf_vertexSelection_Tight_CR_vtx_InvM_h"))
inFile.Close()

outfile.cd()
#Convert x axis scaling from GeV to MeV (may or may not be necessary)
for histo in invMassHistos.keys():
    name = invMassHistos[histo].GetName()
    invMassHistos[histo].SetName("need_to_rescale_%s" % (name))
    rescaled_h = r.TH1F("rescaled_%s" % (name), "rescaled_%s" % (name), 200, 0., 200.)
    nbins = invMassHistos[histo].GetXaxis().GetNbins()
    for b in range(nbins):
        val = invMassHistos[histo].GetBinContent(b+1)
        rescaled_h.SetBinContent(b+1, val)
    rescaled_h.SetName(name)
    invMassHistos[histo] = rescaled_h

#Scale the histograms
rebin = 1
units = 1  # 1 MeV
invMassHistos['rad'].Rebin(rebin)
invMassHistos['tritrig'].Rebin(rebin)
invMassHistos['wab'].Rebin(rebin)
invMassHistos['rad'].Scale(units*66.36e6*Lumi/(rebin*10000*9959))
invMassHistos['tritrig'].Scale(units*1.416e9*Lumi/(rebin*50000*9853))
invMassHistos['wab'].Scale(units*0.1985e12*Lumi/(rebin*100000*9966))

outfile.mkdir('input_histos')
outfile.cd('input_histos')
invMassHistos['rad'].Write()
invMassHistos['wab'].Write()
invMassHistos['tritrig'].Write()
outfile.cd()

canv = utils.MakeRadFrac("radFrac", ".", [invMassHistos['rad'], invMassHistos['wab'], invMassHistos['tritrig']], ['rad', 'wab', 'tritrig+wab'], '.png', RatioMin=0.00, RatioMax=0.3, LogY=True)

#2016 KF results 11/15
'''
EXT PARAMETER                APPROXIMATE        STEP         FIRST
  NO.   NAME      VALUE            ERROR          SIZE      DERIVATIVE
   1  p0          -1.04206e-01   1.01064e-03   4.42889e-04  -1.12429e-06
   2  p1           9.92547e-03   2.40791e-05  -2.67020e-05   1.42600e-04
   3  p2          -1.99437e-04   2.40882e-07   5.98873e-07   4.16987e-02
   4  p3           1.83534e-06   1.99432e-09  -6.26690e-09   4.33594e+00
   5  p4          -7.93138e-09   1.40660e-11   3.07356e-11   1.11698e+03
   6  p5           1.30456e-11   5.68258e-14   5.68258e-14   1.62780e+09
'''
