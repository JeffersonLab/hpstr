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
            help="Specify the output filename.", metavar="outputFile", default="testOut.root")

(options, args) = parser.parse_args()

mcScale = {}
mcScale['tritrig'] = 1.416e9*Lumi/(50000*9853) #pb2016
mcScale['wab'] = 0.1985e12*Lumi/(100000*9966) #pb2016

outfile = r.TFile("./background_distr.root","RECREATE")

bkg_hh = r.TH2F("2016_background_rate_hh","2016_background_rate;M_{vtx} [GeV]; Z_{vtx}",450,0.0,450.,200,-50.,50.)

ttFile = r.TFile("/sdf/group/hps/users/alspellm/projects/simps_2016/reach_estimate/mc/tritrig_beam/SR_ana/hadd_tritrigv2-beamv6_2500kBunches_HPS-PhysicsRun2016-Pass2_v4_5_0_pairs1_SR_ana.root","READ")
ttFile.cd()
ttTree = ttFile.Get("vtxana_Tight_simpSIG/vtxana_Tight_simpSIG_tree")
ttTree.SetName("tritrig_Tight_tree")
#tt_mass_h = r.TH1F("tritrig_SR_mass","tritrig_SR_mass;mass [MeV];events",450,0.0,450.0) 
#tt_mass_z_h = r.TH1F("tritrig_SR_mass_zcut","tritrig_SR_mass;mass [MeV];events",450,0.0,450.0) 
for ev in ttTree:
    #tt_mass_h.Fill(1000.0*ev.unc_vtx_mass)
    mass = 1000*ev.unc_vtx_mass
    z = ev.unc_vtx_z
    bkg_hh.Fill(mass,z,mcScale['tritrig'])
ttFile.Close()


wabFile = r.TFile("/sdf/group/hps/users/alspellm/projects/simps_2016/reach_estimate/mc/wab_beam/SR_ana/hadd_wabv3-beamv6_2500kBunches_HPS-PhysicsRun2016-Pass2_v4_5_0_pairs1_ana.root","READ")
wabFile.cd()
wabTree = wabFile.Get("vtxana_Tight_simpSIG/vtxana_Tight_simpSIG_tree")
wabTree.SetName("wab_Tight_tree")
#wab_mass_h = r.TH1F("wab_SR_mass","wab_SR_mass;mass [MeV];events",450,0.0,450.0) 
for ev in wabTree:
    #wab_mass_h.Fill(1000.0*ev.unc_vtx_mass)
    mass = 1000*ev.unc_vtx_mass
    z = ev.unc_vtx_z
    bkg_hh.Fill(mass,z,mcScale['wab'])
wabFile.Close()


outfile.cd()
#myz = np.array([2.5,2.0,1.5,0.0,-1.0,-2.0,-2.5, -2.75,-3.0,-3.5,-4.0,-4.25,-4.5,-5.5,-6.5,-7.0])
#myz = np.array([x+1. for x in myz])
#myx = np.array([50.,60.,70.,80.,90.,100.,110.,120.,130.,140.,150.,160.,180.,200.,220.,380.])

myz = np.array([39.3,36.5,33.6,30.7,28.1,25.8,23.7,22.1,20.4,19.2,18.3,17.2,16.3,15.8,15.2,14.7,14.1,13.4,12.6,11.7,11.1,9.7,9.,8.9,9.2,9.1,7.3,5.5,6.9,6.1,4.8,3.9])
myx = np.array([40.0,45.0,50.0,55.0,60.0,65.0,70.0,75.0,80.0,85.0,90.0,95.0,100.,105.,110.,115.0,120.,125.,130.,135.,140.,145.,150.,155.,160.,170.,175.,180.,185.,190.,195.,200.])

myzcut_g = r.TGraph(len(myx),myx,myz)
myzcut_g.SetName("my_zcut")
myfit = myzcut_g.Fit('pol3','ES')
myzcut_g.Draw()
myzcut_g.SetLineColor(3)
myzcut_g.Write()

bkg_hh.Write()
c = r.TCanvas("background",'background',1800,1000)
c.cd()
bkg_hh.Draw("colz")
#zcut_g.Draw("same")
#altzcut_g.Draw("same")
myzcut_g.Draw("same")
c.Write()

#results of zcut fit
#zCut = 9.71425 + -0.140865*mass + 0.000441817*math.pow(mass,2) + -4.73974e-07*math.pow(mass,3) #PASS0 TRITRIG

