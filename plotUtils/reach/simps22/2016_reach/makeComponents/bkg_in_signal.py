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
                  help="Specify the output filename.", metavar="outputFile", default="scaled_background_vtxana_kf_SR.root")

(options, args) = parser.parse_args()

mcScale = {}
mcScale['tritrig'] = 1.416e9*Lumi/(50000*9853)  # pb2016
mcScale['wab'] = 0.1985e12*Lumi/(100000*9966)  # pb2016

outfile = r.TFile("%s" % (options.outputFile), "RECREATE")

bkg_hh = r.TH2F("2016_MC_gbl_background_rate_SR_hh", "2016_MC_gbl_background_rate_SR;M_{vtx} [GeV]; Z_{vtx}", 180, 20.0, 200., 160, -20., 60.)

#old GBL reach
#ttFile = r.TFile("/sdf/group/hps/users/alspellm/projects/simps_2016/reach_estimate/mc/tritrig_beam/SR_ana/hadd_tritrigv2-beamv6_2500kBunches_HPS-PhysicsRun2016-Pass2_v4_5_0_pairs1_SR_ana.root","READ")

#2016 KF ana 11 2022
#ttFile = r.TFile("/sdf/group/hps/users/alspellm/projects/THESIS/simp_reach_estimates/simps_2016_kf/mc/tritrig_beam/ana/final_hadd_tritrigv2-beamv6_2500kBunches_HPS-PhysicsRun2016-Pass2_v4_5_0_pairs1_976_KF_SR.root","READ")

#2016 GBL ana 11 2022
ttFile = r.TFile("/sdf/group/hps/users/alspellm/projects/THESIS/simp_reach_estimates/simps_2016_kf/mc/seedtracker/final_hadd_tritrigv2-beamv6_pairs1_seedtracker_ana_SR.root", "READ")

ttFile.cd()
ttTree = ttFile.Get("vtxana_gbl_vertexSelection_Tight_SR/vtxana_gbl_vertexSelection_Tight_SR_tree")
ttTree.SetName("tritrig_Tight_tree")
#tt_mass_h = r.TH1F("tritrig_SR_mass","tritrig_SR_mass;mass [MeV];events",450,0.0,450.0)
#tt_mass_z_h = r.TH1F("tritrig_SR_mass_zcut","tritrig_SR_mass;mass [MeV];events",450,0.0,450.0)
for ev in ttTree:
    #tt_mass_h.Fill(1000.0*ev.unc_vtx_mass)
    mass = 1000*ev.unc_vtx_mass
    z = ev.unc_vtx_z
    bkg_hh.Fill(mass, z, mcScale['tritrig'])
ttFile.Close()

#old GBL reach
#wabFile = r.TFile("/sdf/group/hps/users/alspellm/projects/THESIS/simp_reach_estimates/simps_2016_kf/mc/wab_beam/ana/final_hadd_wabv3-beamv6_2500kBunches_HPS-PhysicsRun2016-Pass2_v4_5_0_pairs1_gbl_ana_SR.root","READ")

#2016 KF ana 11 2022
#wabFile = r.TFile("/sdf/group/hps/users/alspellm/projects/THESIS/simp_reach_estimates/simps_2016_kf/mc/wab_beam/ana/final_hadd_wabv3-beamv6_2500kBunches_HPS-PhysicsRun2016-Pass2_v4_5_0_pairs1_KF_ana_SR.root","READ")

#2016 seedtracker ana 11 2022
wabFile = r.TFile("/sdf/group/hps/users/alspellm/projects/THESIS/simp_reach_estimates/simps_2016_kf/mc/seedtracker/final_hadd_wabv3-beamv6_seedtracker_ana_SR.root", "READ")
wabFile.cd()
wabTree = wabFile.Get("vtxana_gbl_vertexSelection_Tight_SR/vtxana_gbl_vertexSelection_Tight_SR_tree")
wabTree.SetName("wab_Tight_tree")
#wab_mass_h = r.TH1F("wab_SR_mass","wab_SR_mass;mass [MeV];events",450,0.0,450.0)
for ev in wabTree:
    #wab_mass_h.Fill(1000.0*ev.unc_vtx_mass)
    mass = 1000*ev.unc_vtx_mass
    z = ev.unc_vtx_z
    bkg_hh.Fill(mass, z, mcScale['wab'])
wabFile.Close()


outfile.cd()
#myz = np.array([2.5,2.0,1.5,0.0,-1.0,-2.0,-2.5, -2.75,-3.0,-3.5,-4.0,-4.25,-4.5,-5.5,-6.5,-7.0])
#myz = np.array([x+1. for x in myz])
#myx = np.array([50.,60.,70.,80.,90.,100.,110.,120.,130.,140.,150.,160.,180.,200.,220.,380.])

#2016 GBL old
#myz = np.array([39.3,36.5,33.6,30.7,28.1,25.8,23.7,22.1,20.4,19.2,18.3,17.2,16.3,15.8,15.2,14.7,14.1,13.4,12.6,11.7,11.1,9.7,9.,8.9,9.2,9.1,7.3,5.5,6.9,6.1,4.8,3.9])
#myx = np.array([40.0,45.0,50.0,55.0,60.0,65.0,70.0,75.0,80.0,85.0,90.0,95.0,100.,105.,110.,115.0,120.,125.,130.,135.,140.,145.,150.,155.,160.,170.,175.,180.,185.,190.,195.,200.])


#2016 KF 11/15
myz = np.array([45.3, 46.9, 44.8, 41.4, 38.7, 35.5, 32.5, 30.1, 28.7, 25.4, 23.1, 22.4, 21.4, 20.0, 18.3, 17.2, 17.1, 15.8, 14.2, 13.7, 13.8, 13.1, 11.8, 9.5, 9.2, 8.1, 9.4, 10.2, 10.3, 9.5, 7.1, 6.7, 6.7, 6.1, 3.2, 3.1])
myx = np.array([25.0, 30.0, 35.0, 40.0, 45.0, 50.0, 55., 60., 65., 70., 75., 80., 85., 90., 95., 100., 105., 110., 115., 120., 125., 130., 135., 140., 145., 150., 155., 160., 165., 170., 175., 180., 185., 190., 195., 200.])
myzcut_g = r.TGraph(len(myx), myx, myz)
myzcut_g.SetName("my_zcut")
myfit = myzcut_g.Fit('pol3', 'ES')
myzcut_g.Draw()
myzcut_g.SetLineColor(3)
myzcut_g.Write()

bkg_hh.Write()
c = r.TCanvas("background", 'background', 1800, 1000)
c.cd()
bkg_hh.Draw("colz")
#zcut_g.Draw("same")
#altzcut_g.Draw("same")
myzcut_g.Draw("same")
c.Write()

#2016 simps kf zcut 11/15/22
'''
****************************************
Minimizer is Minuit / Migrad
Chi2                      =      50.5331
NDf                       =           32
Edm                       =  4.06771e-09
NCalls                    =          252
p0                        =      69.2555   +/-   1.95263       -1.55394     +1.55381      (Minos)
p1                        =    -0.916318   +/-   0.0663479     -0.0527965   +0.0528012    (Minos)
p2                        =   0.00504772   +/-   0.000650649   -0.000517802 +0.000517755  (Minos)
p3                        = -1.04946e-05   +/-   1.91058e-06   -1.52035e-06 +1.52048e-06  (Minos)
'''
#Data...

#KF 11 2022
#dataFile = r.TFile("/sdf/group/hps/users/alspellm/projects/THESIS/simp_reach_estimates/simps_2016_kf/data/hadd_sample0_KF_ana_SR.root","READ")#data_bkg_hh = dataFile.Get("vtxana_kf_vertexSelection_Tight_SR/vtxana_kf_vertexSelection_Tight_SR_vtx_InvM_vtx_z_hh")
#data_bkg_hh = dataFile.Get("vtxana_kf_vertexSelection_Tight_SR/vtxana_kf_vertexSelection_Tight_SR_vtx_InvM_vtx_z_hh")

#Seedtracker 11 2022
dataFile = r.TFile("/sdf/group/hps/users/alspellm/projects/THESIS/simp_reach_estimates/simps_2016_kf/data/hadd_sample0_GBL_ana_SR.root", "READ")
data_bkg_hh = dataFile.Get("vtxana_gbl_vertexSelection_Tight_SR/vtxana_gbl_vertexSelection_Tight_SR_vtx_InvM_vtx_z_hh")

dataFile.cd()
rescaled_data_bkg_hh = r.TH2F("2016_gbl_data_background_rate_SR_hh", "2016_gbl_data_background_rate_SR;M_{vtx} [GeV]; Z_{vtx}", 180, 20.0, 200., 160, -20., 60.)

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
