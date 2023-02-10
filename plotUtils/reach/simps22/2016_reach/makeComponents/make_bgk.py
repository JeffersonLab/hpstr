#/bin/env python
import math
import glob
import numpy as np
import ROOT as r
import utilities as utils
import copy
from optparse import OptionParser

Lumi = 10.7

utils.SetStyle()

parser = OptionParser()

parser.add_option("-o", "--outputFile", type="string", dest="outputFile",
                  help="Specify the output filename.", metavar="outputFile", default="bkg_in_CR_GBL.root")

(options, args) = parser.parse_args()

mcScale = {}
mcScale['tritrig'] = 1.416e9*Lumi/(50000*9853)  # pb2016
mcScale['wab'] = 0.1985e12*Lumi/(100000*9966)  # pb2016

outfile = r.TFile("%s" % (options.outputFile), "RECREATE")

bkg_hh = r.TH2F("2016_background_rate_CR_hh", "2016_background_rate_CR;M_{vtx} [GeV]; Z_{vtx}", 200, 0.0, 200., 400, -100., 100.)

#ttFile = r.TFile("/sdf/group/hps/users/alspellm/projects/simps_2016/reach_estimate/mc/tritrig_beam/SR_ana/hadd_tritrigv2-beamv6_2500kBunches_HPS-PhysicsRun2016-Pass2_v4_5_0_pairs1_CR_ana.root","READ")
ttFile = r.TFile("/sdf/group/hps/users/alspellm/projects/THESIS/simp_reach_estimates/simps_2016_kf/mc/tritrig_beam/ana/final_hadd_tritrigv2-beamv6_2500kBunches_HPS-PhysicsRun2016-Pass2_v4_5_0_pairs1_976_KF_CR.root", "READ")
ttFile.cd()
ttTree = ttFile.Get("vtxana_kf_vertexSelection_Tight_CR/vtxana_kf_vertexSelection_Tight_CR_tree")
ttTree.SetName("tritrig_Tight_tree")
#tt_mass_h = r.TH1F("tritrig_CR_mass","tritrig_CR_mass;mass [MeV];events",450,0.0,450.0)
#tt_mass_z_h = r.TH1F("tritrig_CR_mass_zcut","tritrig_CR_mass;mass [MeV];events",450,0.0,450.0)
for ev in ttTree:
    #tt_mass_h.Fill(1000.0*ev.unc_vtx_mass)
    mass = 1000*ev.unc_vtx_mass
    z = ev.unc_vtx_z
    bkg_hh.Fill(mass, z, mcScale['tritrig'])
ttFile.Close()


wabFile = r.TFile("/sdf/group/hps/users/alspellm/projects/THESIS/simp_reach_estimates/simps_2016_kf/mc/wab_beam/ana/final_hadd_wabv3-beamv6_2500kBunches_HPS-PhysicsRun2016-Pass2_v4_5_0_pairs1_KF_ana_CR.root", "READ")
wabFile.cd()
wabTree = wabFile.Get("vtxana_kf_vertexSelection_Tight_CR/vtxana_kf_vertexSelection_Tight_CR_tree")
wabTree.SetName("wab_Tight_tree")
#wab_mass_h = r.TH1F("wab_CR_mass","wab_CR_mass;mass [MeV];events",450,0.0,450.0)
for ev in wabTree:
    #wab_mass_h.Fill(1000.0*ev.unc_vtx_mass)
    mass = 1000*ev.unc_vtx_mass
    z = ev.unc_vtx_z
    bkg_hh.Fill(mass, z, mcScale['wab'])
wabFile.Close()

bkg_hh.GetZaxis().SetRangeUser(0.0, 180000.0)

outfile.cd()
bkg_hh.Write()
c = r.TCanvas("background", 'background', 1800, 1000)
c.cd()
bkg_hh.Draw("colz")
c.Write()

#results of zcut fit
#zCut = 9.71425 + -0.140865*mass + 0.000441817*math.pow(mass,2) + -4.73974e-07*math.pow(mass,3) #PASS0 TRITRIG

#Data...
#dataFile = r.TFile("/sdf/group/hps/users/alspellm/projects/THESIS/simp_reach_estimates/simps_2016_kf/data/hadd_sample0_KF_ana_CR.root","READ")
#data_bkg_hh = dataFile.Get("vtxana_kf_vertexSelection_Tight_CR/vtxana_kf_vertexSelection_Tight_CR_vtx_InvM_vtx_z_hh")
dataFile = r.TFile("/sdf/group/hps/users/alspellm/projects/THESIS/simp_reach_estimates/simps_2016_kf/data/hadd_sample0_GBL_ana_CR.root", "READ")
data_bkg_hh = dataFile.Get("vtxana_gbl_vertexSelection_Tight_CR/vtxana_gbl_vertexSelection_Tight_CR_vtx_InvM_vtx_z_hh")
dataFile.cd()
rescaled_data_bkg_hh = r.TH2F("2016_GBL_data_background_rate_CR_hh", "2016_GBL_data_background_rate_CR;M_{vtx} [GeV]; Z_{vtx}", 200, 0.0, 200., 400, -100., 100.)

for y in range(data_bkg_hh.GetNbinsX()):
    mass_GeV = data_bkg_hh.GetXaxis().GetBinLowEdge(y+1)
    mass = mass_GeV*1000.
    projy_h = data_bkg_hh.ProjectionY("projy_%i" % (y), y+1, y+1, "e")
    for b in range(projy_h.GetXaxis().GetNbins()):
        z = projy_h.GetXaxis().GetBinLowEdge(b+1)
        entries = int(projy_h.GetBinContent(b+1))
        for n in range(entries):
            rescaled_data_bkg_hh.Fill(mass, z)
rescaled_data_bkg_hh.Scale(10.7/0.0913)
rescaled_data_bkg_hh.GetZaxis().SetRangeUser(0.0, 180000.0)

outfile.cd()
rescaled_data_bkg_hh.Write()
