#/bin/env python
import math
import glob
import numpy as np
import ROOT as r
import utilities as utils
import copy
from optparse import OptionParser


def getZCut(mass, shift=0.0):
    #minimums = {30:-4.,40:-4,50:-4.,60:-4.,70:-4.5, 80:-5.0,90:-5.,100:-5.5,110:-5.5,120:-5.5,130:-6.0,140:-6.0,150:-6.5,160:-6.5,170:-7.0 }
    minimums = {30: -4., 40: -4, 50: -4., 60: -4., 70: -4.5, 80: -5.0, 90: -5., 100: -5.5, 110: -5.5, 120: -5.5, 130: -5.5, 140: -6.0, 150: -6.0, 160: -6.5, 170: -6.5, 180: -7.0, 190: -7., 200: -7.}
    zCut = shift + 9.71425 + -0.140865*mass + 0.000441817*math.pow(mass, 2) + -4.73974e-07*math.pow(mass, 3)  # PASS0 TRITRIG
    if mass in minimums.keys():
        if zCut < minimums[mass]:
            zCut = minimums[mass]
    else:
        if zCut < -7.0:
            zCut = -7.0
    return zCut


def getZCut2(mass, shift=0.0):
    zCut = shift + 9.71425 + -0.140865*mass + 0.000441817*math.pow(mass, 2) + -4.73974e-07*math.pow(mass, 3)  # PASS0 TRITRIG
    if zCut < -5.0:
        zCut = -5.0
    return zCut


Lumi = 110.

utils.SetStyle()

parser = OptionParser()

parser.add_option("-o", "--outputFile", type="string", dest="outputFile",
                  help="Specify the output filename.", metavar="outputFile", default="zcut_background_scan.root")

(options, args) = parser.parse_args()
outfile = r.TFile("%s" % (options.outputFile), "RECREATE")

mcScale = {}
mcScale['tritrig'] = 6.610e08*Lumi/(10000*1425)
mcScale['wab'] = 4.715e10*Lumi/(10000*9944)

#outfile = r.TFile("background_distr.root","RECREATE")

bkg_hh = r.TH2F("2019_background_rate_hh", "2019_background_rate;M_{vtx} [GeV]; Z_{vtx}", 450, 0.0, 450., 200, -50., 50.)

ttFile = r.TFile("/sdf/group/hps/users/alspellm/projects/THESIS/simp_reach_estimates/simps_2019/mc/tritrig_beam/ana_SR/hadd_tritrig_SR_ana.root", "READ")
ttFile.cd()
ttTree = ttFile.Get("vtxana_kf_Tight_2019_simpSR/vtxana_kf_Tight_2019_simpSR_tree")
ttTree.SetName("tritrig_Tight_tree")
#tt_mass_h = r.TH1F("tritrig_SR_mass","tritrig_SR_mass;mass [MeV];events",450,0.0,450.0)
#tt_mass_z_h = r.TH1F("tritrig_SR_mass_zcut","tritrig_SR_mass;mass [MeV];events",450,0.0,450.0)
for ev in ttTree:
    #tt_mass_h.Fill(1000.0*ev.unc_vtx_mass)
    mass = 1000*ev.unc_vtx_mass
    z = ev.unc_vtx_z
    bkg_hh.Fill(mass, z, mcScale['tritrig'])
ttFile.Close()


wabFile = r.TFile("/sdf/group/hps/users/alspellm/projects/THESIS/simp_reach_estimates/simps_2019/mc/wab_beam/ana_SR/hadd_wab_SR_ana.root", "READ")
wabFile.cd()
wabTree = wabFile.Get("vtxana_kf_Tight_2019_simpSR/vtxana_kf_Tight_2019_simpSR_tree")
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

mass = np.arange(40.0, 210.0, 5.)
mass_gev = np.array(mass/1000.)

outfile.cd()
#altzcuts = np.array([-7.5 + 12.7252 + 169.564*massFGeV - 5066.71*massFGeV*massFGeV + 39148*pow(massFGeV,3) - 101548*pow(massFGeV,4) for massFGeV in mass_gev])
#altzcuts = np.array([17.7702 + 138.166*massFGeV - 5363.29*massFGeV*massFGeV + 44532.4*pow(massFGeV,3) - 120578*pow(massFGeV,4) for massFGeV in mass_gev])
#altzcut_g = r.TGraph(len(mass),mass,altzcuts)
#altzcut_g.SetName("alt_zcut")
#altzcut_g.SetLineColor(2)


#myz = np.array([-2.5,-3.,-3.25,-3.5,-4.0,-4.25,-4.5,-4.75,-5.25,-5.5,-5.75,-5.75,-6.,-6.5,-6.5,-7.0])
#myz = np.array([x+1.8 for x in myz])
#myx = np.array([50.,60.,70.,80.,90.,100.,110.,120.,130.,140.,150.,160.,180.,200.,220.,380.])

myz = np.array([2.5, 2.0, 1.5, 0.0, -1.0, -2.0, -2.5, -2.75, -3.0, -3.5, -4.0, -4.25, -4.5, -5.5, -6.5, -7.0])
myz = np.array([x+1. for x in myz])
myx = np.array([50., 60., 70., 80., 90., 100., 110., 120., 130., 140., 150., 160., 180., 200., 220., 380.])

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

colorsMap = {15: r.kBlue, 14: r.kGreen, 12: r.kOrange, 13: r.kRed, 11: r.kYellow, 8: r.kMagenta, 9: r.kCyan, 10: r.kPink+1, 4: r.kSpring+10, 6: r.kViolet+2, 7: r.kTeal-1, 3: r.kOrange+7, 5: r.kMagenta-3, 2: r.kYellow-3, 1: r.kBlue+2, 0: r.kPink-9}

masses = list([30., 40., 50., 60., 70., 80., 90., 100., 110., 120., 130., 140., 150., 160., 170., 180., 190., 200., 210., 220., 230., 240., 250., 260., 270., 280., 290., 300., 310., 320., 330., 340., 350.])
#for i,shift in enumerate([-0.5,-1.0,-1.5,-2.0,-2.5,-3.0]):
plots = []
for i, shift in enumerate([4.0, 3.0, 2.0, 1.0, 0.0, -1.0, -2.0, -3.0, -4.0, -5., -6.]):
    shifts = [shift] * len(masses)
    cuts = np.array(list(map(getZCut, masses, shifts)))
    print(cuts)
    cuts = np.array(cuts)
    shift_g = r.TGraph(len(masses), np.array(masses), cuts)
    shift_g.SetName("Zcut_shifted_%f" % (shift))
    shift_g.SetTitle("Zcut_shifted_%f;M_{vtx} [GeV]; Vtx_{z} [mm]" % (shift))
    shift_g.SetLineWidth(2)
    shift_g.SetLineColor(colorsMap[i])
    plots.append(shift_g)
    #shift_g.Draw("same")
    #shift_g.Write()
canvas = r.TCanvas("zcut_scan", "zcut_scan", 1800, 1200)
canvas.cd()
bkg_hh.Draw("colz")
for plot in plots:
    plot.Draw("same")
legend = canvas.BuildLegend()
legend.Draw()
canvas.Write()

#results of zcut fit
#zCut = 9.71425 + -0.140865*mass + 0.000441817*math.pow(mass,2) + -4.73974e-07*math.pow(mass,3) #PASS0 TRITRIG
