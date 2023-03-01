#/bin/env python
import math
import glob
import numpy as np
import ROOT as r
import utilities as utils
import copy
from optparse import OptionParser

Lumi = 530. + 168.1  # 1/pb

utils.SetStyle()

parser = OptionParser()

parser.add_option("-o", "--outputFile", type="string", dest="outputFile",
                  help="Specify the output filename.", metavar="outputFile", default="testOut.root")

(options, args) = parser.parse_args()
outfile = r.TFile("%s" % (options.outputFile), "RECREATE")

mcScale = {}
mcScale['tritrig'] = 6.853e08*Lumi/(10000*993)
mcScale['wab'] = 7.557e10*Lumi/((30000*3333 + 1405*40000 + 907*40000))

outfile = r.TFile("background_distr.root", "RECREATE")

bkg_hh = r.TH2F("3pt7_background_rate_hh", "3pt7_background_rate;M_{vtx} [GeV]; Z_{vtx}", 450, 0.0, 450., 200, -40., 40.)

ttFile = r.TFile("/sdf/group/hps/users/alspellm/projects/simps_3pt7/tritrig_beam/tuple_ana/SR/hadd_tritrig_beam_SR_ana.root", "READ")
ttFile.cd()
ttTree = ttFile.Get("vtxana_kf_Tight_2021_simpSR/vtxana_kf_Tight_2021_simpSR_tree")
ttTree.SetName("tritrig_Tight_tree")
#tt_mass_h = r.TH1F("tritrig_SR_mass","tritrig_SR_mass;mass [MeV];events",450,0.0,450.0)
#tt_mass_z_h = r.TH1F("tritrig_SR_mass_zcut","tritrig_SR_mass;mass [MeV];events",450,0.0,450.0)
for ev in ttTree:
    #tt_mass_h.Fill(1000.0*ev.unc_vtx_mass)
    mass = 1000*ev.unc_vtx_mass
    z = ev.unc_vtx_z
    bkg_hh.Fill(mass, z, mcScale['tritrig'])
ttFile.Close()


wabFile = r.TFile("/sdf/group/hps/users/alspellm/projects/simps_3pt7/wab_beam/super_hadd_wab_beam_SR_ana.root", "READ")
wabFile.cd()
wabTree = wabFile.Get("vtxana_kf_Tight_2021_simpSR/vtxana_kf_Tight_2021_simpSR_tree")
wabTree.SetName("wab_Tight_tree")
#wab_mass_h = r.TH1F("wab_SR_mass","wab_SR_mass;mass [MeV];events",450,0.0,450.0)
for ev in wabTree:
    #wab_mass_h.Fill(1000.0*ev.unc_vtx_mass)
    mass = 1000*ev.unc_vtx_mass
    z = ev.unc_vtx_z
    bkg_hh.Fill(mass, z, mcScale['wab'])
wabFile.Close()

outfile.cd()
#zcut_f = r.TFile("/sdf/group/hps/users/alspellm/projects/simps_2019/reach_estimate/reach_estimates/makeComponents/zcuts.root","READ")
#zcut_f.cd()
#zcut_g = zcut_f.Get('zcuts_g')

#mass = np.arange(40.0,210.0,5.)
#mass_gev = np.array(mass/1000.)

outfile.cd()
#altzcuts = np.array([-7.5 + 12.7252 + 169.564*massFGeV - 5066.71*massFGeV*massFGeV + 39148*pow(massFGeV,3) - 101548*pow(massFGeV,4) for massFGeV in mass_gev])
#altzcuts = np.array([17.7702 + 138.166*massFGeV - 5363.29*massFGeV*massFGeV + 44532.4*pow(massFGeV,3) - 120578*pow(massFGeV,4) for massFGeV in mass_gev])
#altzcut_g = r.TGraph(len(mass),mass,altzcuts)
#altzcut_g.SetName("alt_zcut")
#altzcut_g.SetLineColor(2)


#myz = np.array([8.0,7.5,7.0,6.5,6.0,5.5,5.0,4.5,4.0,3.5,3.0,3.0,2.5,2.5,2.5,2.5,2.0,2.0,1.75,1.0,0.75,0.75,0.75,0.75])
#myx = np.array([50.0,60.0,70.0,80.0,90.0,100.0,110.,120.,130.,140.,150.,160.,170.,180.,190.,200.,210.,220.,230.,240.,250.,260.,270.,360.])
myz = np.array([14., 13.5, 13., 12., 11.5, 10., 9., 8., 4.5, 4.0, 3.5, 3.0, 3.0, 2.5, 2.5, 2.5, 2.5, 2.0, 2.0, 1.75, 1.0, 0.75, 0.75, 0.75, 0.75])
myx = np.array([40.0, 50.0, 60.0, 70.0, 80.0, 90.0, 100.0, 110., 120., 130., 140., 150., 160., 170., 180., 190., 200., 210., 220., 230., 240., 250., 260., 270., 360.])
myz = np.array([x+1.0 for x in myz])
#myx = np.array([50.,60.,70.,80.,90.,100.,110.,120.,130.,140.,150.,160.,180.,200.,220.,380.])
myzcut_g = r.TGraph(len(myx), myx, myz)
myzcut_g.SetName("my_zcut")
myfit = myzcut_g.Fit('pol4', 'ES')
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
