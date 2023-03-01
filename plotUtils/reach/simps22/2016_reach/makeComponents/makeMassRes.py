#/bin/env python
import glob
import numpy as np
import ROOT as r
import utilities as utils
import copy
from optparse import OptionParser

utils.SetStyle()

parser = OptionParser()

parser.add_option("-i", "--inputFile", type="string", dest="inputFile",
                  help="Name of file to run on.", metavar="inputFile", default="toys/toys.root")
parser.add_option("-o", "--outputFile", type="string", dest="outputFile",
                  help="Specify the output filename.", metavar="outputFile", default="massResolution.root")

(options, args) = parser.parse_args()

invMasses = [25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100, 105, 110, 115, 120, 125, 130, 135, 140, 145, 150, 155, 160, 165, 170, 175, 180, 185, 190, 195, 200]

outFile = r.TFile(options.outputFile, "RECREATE")

zeros = [0.0 for mass in invMasses]
masses = [float(mass) for mass in invMasses]
massRezs = []
massRezErrs = []
massRezsScaled = []
massRezErrsScaled = []

for mass in invMasses:
    print(mass)
    apFilename = "/sdf/group/hps/users/alspellm/projects/THESIS/simp_reach_estimates/simps_2016_kf/mc/simps/reco_ana/hadd_mass_%i_simp_recon_KF_ana_SR.root" % (mass)
    apFile = r.TFile(apFilename)
    apRecoMass_h = copy.deepcopy(apFile.Get("vtxana_kf_vertexSelection_radMatchTight_SR/vtxana_kf_vertexSelection_radMatchTight_SR_vtx_InvM_h"))
    apRecoMass_h.SetName("vtxana_radMatchTight_SR_vtx_InvM%i_h" % (mass))
    print("Mass %i MeV: %i" % (mass, apRecoMass_h.GetEntries()))
    apFile.Close()
    frl = apRecoMass_h.GetXaxis().GetBinLowEdge(apRecoMass_h.FindFirstBinAbove(0))
    frh = apRecoMass_h.GetXaxis().GetBinLowEdge(apRecoMass_h.FindLastBinAbove(0))
    fitRes = apRecoMass_h.Fit("gaus", "ES+", "", frl, frh)
    outFile.cd()
    apRecoMass_h.Write()
    massRez = 1000.0*fitRes.GetParams()[2]
    massRezErr = 1000.0*fitRes.GetErrors()[2]
    massRezs.append(massRez)
    massRezErrs.append(massRezErr)
    massRezsScaled.append(1.43*massRez)
    massRezErrsScaled.append(1.43*massRezErr)
    print("Mass res: %f +- %f" % (massRez, massRezErr))
    pass

massRes_ge = r.TGraphErrors(len(masses), np.array(masses), np.array(massRezs), np.array(zeros), np.array(massRezErrs))
massRes_ge.SetName("massRes_ge")
massRes_ge.SetTitle(";m_{vtx} [MeV];#sigma_{m} [MeV]")
massRes_ge.SetLineColor(utils.colors[4])
massRes_ge.SetLineWidth(4)
massResScaled_ge = r.TGraphErrors(len(masses), np.array(masses), np.array(massRezsScaled), np.array(zeros), np.array(massRezErrsScaled))
massResScaled_ge.SetName("massResScaled_ge")
massResScaled_ge.SetTitle(";m_{vtx} [MeV];#sigma_{m} [MeV]")
massResScaled_ge.SetLineColor(utils.colors[5])
massResScaled_ge.SetLineWidth(4)

nPoints = len(masses)
chi2s = []
fstats = []
for polyO in range(5):
    fitResult = massResScaled_ge.Fit('pol%i' % polyO, "ES")
    chi2s.append(fitResult.Chi2())
    if polyO > 0:
        fstats.append((chi2s[polyO-1]-chi2s[polyO])*(nPoints-polyO-1)/(chi2s[polyO]))
    else:
        fitCon = fitResult.GetParams()[0]

print("nPoints: %i" % nPoints)
for polyO in range(1, 5):
    print("Order: %i    NDF: %f    chi2: %f    f-stat: %f" % (polyO, nPoints-polyO-1, chi2s[polyO], fstats[polyO-1]))
fitResult = massResScaled_ge.Fit('pol2', "ES")
fitFunc = massResScaled_ge.GetListOfFunctions().FindObject("pol2")
fitFunc.SetLineColor(utils.colors[3])

canv = r.TCanvas("canv", "canv", 1400, 1000)
canv.cd()
massResScaled_ge.SetMinimum(0.0)
massResScaled_ge.SetMaximum(20.0)
massResScaled_ge.Draw("ape")
massRes_ge.Draw("pesame")
fitFunc.Draw("same")
utils.InsertText()
canv.SaveAs("massRes.png")

massRes_ge.Write()
massResScaled_ge.Write()

outFile.Close()

#2016 Simps KF 11/15/22
'''
EXT PARAMETER                                   STEP         FIRST
  NO.   NAME      VALUE            ERROR          SIZE      DERIVATIVE
   1  p0           1.06314e+00   5.08804e-02   1.96911e-06   8.32806e-09
   2  p1           3.45955e-02   1.20154e-03  -2.30725e-08  -5.34448e-07
   3  p2          -6.62113e-05   6.48625e-06   6.48625e-06  -1.73755e-05
'''
