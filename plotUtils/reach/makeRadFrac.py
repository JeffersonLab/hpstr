#/bin/env python
import numpy as np
import ROOT as r
import utilities as utils
import copy
from optparse import OptionParser

#Lumi = 11008.61649
Lumi = 125.0

utils.SetStyle()

parser = OptionParser()

parser.add_option("-i", "--inputFile", type="string", dest="inputFile",
                  help="Name of file to run on.", metavar="inputFile", default="toys/toys.root")
parser.add_option("-o", "--outputFile", type="string", dest="outputFile",
                  help="Specify the output filename.", metavar="outputFile", default="testOut.root")

(options, args) = parser.parse_args()

invMassHistos = {}
inFile = r.TFile("/nfs/hps_data2/users/bravo/mc/det19/rad/radAnaVtx.root")
invMassHistos['rad'] = copy.deepcopy(inFile.Get("vtxana_radMatchTight_2019/vtxana_radMatchTight_2019_mcMass622_h"))
#invMassHistos['rad'] = copy.deepcopy(inFile.Get("vtxana_Tight/vtxana_Tight_mcMass622_h"))
#invMassHistos['rad'] = copy.deepcopy(inFile.Get("bhTight/bhTight_vtx_InvM_h"))
inFile.Close()
inFile = r.TFile("/nfs/hps_data2/users/bravo/mc/det19/tritrig/tritrigAnaVtx.root")
invMassHistos['tritrig'] = copy.deepcopy(inFile.Get("vtxana_Tight_2019/vtxana_Tight_2019_vtx_InvM_h"))
inFile.Close()
inFile = r.TFile("/nfs/hps_data2/users/bravo/mc/det19/wab/wabAnaVtx.root")
invMassHistos['wab'] = copy.deepcopy(inFile.Get("vtxana_Tight_2019/vtxana_Tight_2019_vtx_InvM_h"))
inFile.Close()

#Scale the histograms
#invMassHistos['rad'].Scale(81.61e3*Lumi/99590000)
#invMassHistos['tritrig'].Scale(1.416e6*Lumi/492650000)
#invMassHistos['wab'].Scale(0.1985e9*Lumi/996600000)
invMassHistos['rad'].Rebin(4)
invMassHistos['tritrig'].Rebin(4)
invMassHistos['wab'].Rebin(4)
invMassHistos['rad'].Scale(3.123e7*Lumi/8740000)
invMassHistos['tritrig'].Scale(4.566e8*Lumi/9040000)
invMassHistos['wab'].Scale(4.715e10*Lumi/(4933*20000))

triWab_sh = r.THStack("triWab_sh", ";m_{e^{+}e^{-}} [GeV];#frac{dN}{dm} [MeV^{-1}]")

triWab_sh.Add(invMassHistos['wab'])
triWab_sh.Add(invMassHistos['tritrig'])

#canv = r.TCanvas("canv","canv",1600,1200)
#canv.cd()

#invMassHistos['rad'].GetXaxis().SetRangeUser(0.05,0.2)
#invMassHistos['rad'].GetYaxis().SetTitle("#frac{dN}{dm} [1 / 2 MeV]")
#invMassHistos['rad'].Draw()
#canv.SetLogy(1)

canv = utils.MakeRadFrac("radFrac", ".", [invMassHistos['rad'], invMassHistos['wab'], invMassHistos['tritrig']], ['rad', 'wab', 'tritrig+wab'], '.png', RatioMin=0.00, RatioMax=0.3, LogY=True)
#canv = utils.MakeStackPlot("radFrac", ".", [invMassHistos['rad'],invMassHistos['tritrig']], ['rad', 'tritrig'],'.png', RatioMin=0.02, RatioMax=0.1, LogY=True)
