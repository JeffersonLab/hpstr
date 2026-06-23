#!/usr/bin/env python3
import glob
import copy
import json
import math
import numpy as np
import ROOT as r
import utilities as utils
from optparse import OptionParser

# ---------------------------------------------------------------------------
# Default paths for 2021 analysis inputs
# ---------------------------------------------------------------------------
PATHS = {
    # Truth-level A' vtx_z histograms produced by makeTruthSimZ.py
    'simZ_file':    'apSimZ_2021.root',

    # Preselected background MC (flat tuples, tree name: "preselection")
    'tritrig_dir':  '/sdf/data/hps/physics2021/preselection/v11/tritrig_smeared',
    'wab_dir':      '/sdf/data/hps/physics2021/preselection/v11/wab_smeared',

    # Preselected signal MC, one subdirectory per mass named ap{mass}MeV
    'signal_dir':   '/sdf/data/hps/physics2021/preselection/v11/ap_signal_smeared',
}

# Background cross sections and generated event counts (from summary.json)
BKG_NORM = {
    'tritrig': {'xsec': 4.026e9, 'n_gen': 1000 * 80000},
    'wab':     {'xsec': 8.079e10, 'n_gen': 2000 * 200000},
}

# Target position in z (mm) — origin of the exponential decay probability
# and reference for the minimum displacement cut
TARGET_Z = -1.1

# Minimum displacement from the target to include in the efficiency integral (mm)
MIN_DISPLACEMENT = 2.0

# Beam energy in GeV — sets the average A' Lorentz boost (p_A' ∝ E_beam)
BEAM_ENERGY_GEV = 3.74
# ---------------------------------------------------------------------------


def radFrac(mass):
    mass = mass/1000.0
    radF = (-2.32204e-1 + 7.58935*mass - 7.81378e1*pow(mass, 2) + 3.80505e2*pow(mass, 3) - 8.93101e2*pow(mass, 4) + 8.07483e2*pow(mass, 5))
    return radF


def vtxRes(mass):
    res = (1.67213 - 9.81333*mass + 20.0887*pow(mass, 2))
    return res


def massRes(mass):
    res = 3.21662e+00 - 7.07072e-03*mass + 1.04779e-04*mass*mass
    return res


def calcLifetime(mass, eps2, beam_energy_gev=3.74):
    return 8.0*(beam_energy_gev/10.0)*(1e-8/eps2)*pow(100.0/mass, 2)


def make_chain(directory, tree_name='preselection'):
    """Chain all *.root files in a directory onto the given tree."""
    chain = r.TChain(tree_name)
    for f in sorted(glob.glob(directory + '/*.root')):
        chain.Add(f)
    return chain


utils.SetStyle()

parser = OptionParser()
parser.add_option("-o", "--outputFile", type="string", dest="outputFile",
                  help="Output ROOT file.", metavar="outputFile",
                  default="expSigRate_2021.root")
parser.add_option("-s", "--simZFile", type="string", dest="simZFile",
                  help="ROOT file with apSimZ{mass}_h histograms (from makeTruthSimZ.py).",
                  metavar="simZFile", default=PATHS['simZ_file'])
parser.add_option("--tritrig-dir", type="string", dest="tritrig_dir",
                  help="Directory containing preselected tritrig MC files.",
                  metavar="DIR", default=PATHS['tritrig_dir'])
parser.add_option("--wab-dir", type="string", dest="wab_dir",
                  help="Directory containing preselected WAB MC files.",
                  metavar="DIR", default=PATHS['wab_dir'])
parser.add_option("--signal-dir", type="string", dest="signal_dir",
                  help="Directory containing preselected signal MC (ap{mass}MeV subdirs).",
                  metavar="DIR", default=PATHS['signal_dir'])
parser.add_option("-n", "--nWeeks", type="float", dest="nWeeks",
                  help="Number of weeks of luminosity.", metavar="nWeeks", default=10.0)

(options, args) = parser.parse_args()

simZFile = r.TFile(options.simZFile)
if simZFile.IsZombie():
    raise RuntimeError("Cannot open simZ file: %s" % options.simZFile)

nWeeks = options.nWeeks
Lumi = nWeeks * 57.0 * 0.93
mcScale = {
    name: norm['xsec'] * Lumi / norm['n_gen']
    for name, norm in BKG_NORM.items()
}
print("Luminosity: %.1f nb^-1" % Lumi)
print("mcScale tritrig: %g" % mcScale['tritrig'])
print("mcScale wab:     %g" % mcScale['wab'])

invMasses = [60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180]

# Chain background MC once — reused for every mass window
print("Loading background MC chains...")
ttChain  = make_chain(options.tritrig_dir)
wabChain = make_chain(options.wab_dir)
print("  tritrig entries: %d" % ttChain.GetEntries())
print("  wab entries:     %d" % wabChain.GetEntries())

outFile = r.TFile(options.outputFile, "RECREATE")

nMasses = len(invMasses)
lowM  = float(invMasses[0]  - 5.0)
highM = float(invMasses[-1] + 5.0)
apProd_hh = r.TH2D("apProd_hh", ";m_{A'} [MeV];log_{10}(#epsilon^{2})", nMasses, lowM, highM, 310, -10.005, -6.905)
Nsig_hh   = r.TH2D("Nsig_hh",   ";m_{A'} [MeV];log_{10}(#epsilon^{2})", nMasses, lowM, highM, 310, -10.005, -6.905)
effVtx_hh = r.TH2D("effVtx_hh", ";m_{A'} [MeV];log_{10}(#epsilon^{2})", nMasses, lowM, highM, 310, -10.005, -6.905)
gcTau_hh  = r.TH2D("gcTau_hh",  ";m_{A'} [MeV];log_{10}(#epsilon^{2})", nMasses, lowM, highM, 310, -10.005, -6.905)

zCutVals   = []
zCutMasRes = []
zCutExpTail = []
dNdms = []

upExContourMass     = []
upExContourEps2     = []
upExContourLogEps2  = []
downExContourMass   = []
downExContourLogEps2 = []
downExContourEps2   = []

for mass in invMasses:
    massF    = float(mass)
    massFGeV = massF / 1000.0
    print("\nRunning mass = %i MeV" % mass)

    # --- truth sim-z denominator ---
    apSimZ_h = copy.deepcopy(simZFile.Get("apSimZ%i_h" % mass))
    if not apSimZ_h:
        print("WARNING: apSimZ%i_h not found in %s, skipping" % (mass, options.simZFile))
        continue
    apSimZ_h.SetDirectory(0)
    outFile.cd()
    apSimZ_h.Write()

    # --- background rate dN/dm in a 10 MeV window ---
    Mbin  = 10.0
    mLow  = massF - Mbin/2.0
    mHigh = massF + Mbin/2.0
    # vertex.invM_ is in GeV; convert window to GeV
    mLow_GeV  = mLow  / 1000.0
    mHigh_GeV = mHigh / 1000.0

    print("Counting background rate")
    dNdm = 0.0
    for ev in ttChain:
        if ev.vertex.invM_ < mLow_GeV or ev.vertex.invM_ > mHigh_GeV:
            continue
        dNdm += mcScale['tritrig']
    for ev in wabChain:
        if ev.vertex.invM_ < mLow_GeV or ev.vertex.invM_ > mHigh_GeV:
            continue
        dNdm += mcScale['wab']
    dNdm /= Mbin
    dNdms.append(dNdm)
    print("Background rate: %f" % dNdm)

    # --- signal efficiency numerator ---
    lowMass  = (massF - 2.8*massRes(massF)/2.0) / 1000.0   # GeV
    highMass = (massF + 2.8*massRes(massF)/2.0) / 1000.0   # GeV
    zCut = 0.5*math.log(Lumi) + 7.4*vtxRes(massFGeV)
    zCutVals.append(zCut)
    zCutMasRes.append(7.4*vtxRes(massFGeV))
    zCutExpTail.append(0.5*math.log(Lumi))

    apSelZ_h   = r.TH1F("apSelZ%i_h"   % mass, ";true z_{vtx} [mm];MC Events", 200, -50.0, 150.0)
    apSelNoZ_h = r.TH1F("apSelNoZ%i_h" % mass, ";true z_{vtx} [mm];MC Events", 200, -50.0, 150.0)

    sigDir = "%s/ap%iMeV" % (options.signal_dir, mass)
    apChain = make_chain(sigDir)
    print("Counting signal (%d entries)" % apChain.GetEntries())
    for ev in apChain:
        if ev.vertex.invM_ < lowMass or ev.vertex.invM_ > highMass:
            continue
        true_z = ev.true_ap.vtx_z_
        if true_z > 138.0:
            continue
        apSelNoZ_h.Fill(true_z)
        if ev.vertex.pos_.fZ < zCut:
            continue
        apSelZ_h.Fill(true_z)

    # --- efficiencies ---
    apEffVtxZ_gae   = r.TGraphAsymmErrors(apSelZ_h,   apSimZ_h, "shortest")
    apEffVtxZ_gae.SetName("apEffVtxZ%i_gae"   % mass)
    apEffVtxZ_e     = r.TEfficiency(apSelZ_h,   apSimZ_h)
    apEffVtxZ_e.SetName("apEffVtxZ%i_e"     % mass)
    apEffVtxNoZ_gae = r.TGraphAsymmErrors(apSelNoZ_h, apSimZ_h, "shortest")
    apEffVtxNoZ_gae.SetName("apEffVtxNoZ%i_gae" % mass)
    apEffVtxNoZ_e   = r.TEfficiency(apSelNoZ_h, apSimZ_h)
    apEffVtxNoZ_e.SetName("apEffVtxNoZ%i_e"   % mass)

    Seff = (apEffVtxNoZ_e.GetEfficiency(51) + apEffVtxNoZ_e.GetEfficiency(52) + apEffVtxNoZ_e.GetEfficiency(53)
            + apEffVtxNoZ_e.GetEfficiencyErrorUp(51) + apEffVtxNoZ_e.GetEfficiencyErrorUp(52) + apEffVtxNoZ_e.GetEfficiencyErrorUp(53)) / 3.0
    apEffVtxZ_gae.SetMaximum(Seff*2.0)
    apEffVtxNoZ_gae.SetMaximum(Seff*2.0)
    if Seff > 0:
        Seff = 1.0/Seff
    print("Seff: %f" % Seff)

    outFile.cd()
    apSelNoZ_h.Write()
    apSelZ_h.Write()
    apEffVtxNoZ_gae.Write()
    apEffVtxZ_gae.Write()
    apEffVtxNoZ_e.Write()
    apEffVtxZ_e.Write()

    effCalc_h = apEffVtxZ_e
    prevRate  = 0.0
    excThr    = 2.3
    for logEps2_i in range(-1000, -690):
        logEps2 = logEps2_i / 100.0
        eps2    = pow(10, logEps2)
        gcTau   = calcLifetime(massF, eps2, BEAM_ENERGY_GEV)
        effVtx  = 0.0
        for zbin in range(1, 201):
            zz = apSelZ_h.GetBinCenter(zbin)
            if zz < TARGET_Z + MIN_DISPLACEMENT:
                continue
            effVtx += (r.TMath.Exp(-(zz - TARGET_Z)/gcTau)/gcTau) * Seff * effCalc_h.GetEfficiency(zbin) * apSelZ_h.GetBinWidth(zbin)
        apProd = 205.5*3.1416*eps2*massF*radFrac(massF)*dNdm
        Nsig   = apProd * effVtx
        apProd_hh.Fill(massF, logEps2, apProd)
        Nsig_hh.Fill(  massF, logEps2, Nsig)
        effVtx_hh.Fill(massF, logEps2, effVtx)
        gcTau_hh.Fill( massF, logEps2, gcTau)
        if prevRate < excThr and Nsig > excThr:
            downExContourMass.append(massF)
            downExContourEps2.append(eps2)
            downExContourLogEps2.append(logEps2)
        if prevRate > excThr and Nsig < excThr:
            upExContourMass.append(massF)
            upExContourEps2.append(eps2)
            upExContourLogEps2.append(logEps2)
        prevRate = Nsig

print("\nSaving output...")
upExContourMass.reverse()
upExContourEps2.reverse()
upExContourLogEps2.reverse()
exContourMass   = upExContourMass   + downExContourMass
exContourEps2   = upExContourEps2   + downExContourEps2
exContourLogEps2 = upExContourLogEps2 + downExContourLogEps2

contOutFile = open("excContourE3pt7Nw%f.txt" % nWeeks, "w")
for i in range(len(exContourMass)):
    contOutFile.write("%f\t%E\n" % (exContourMass[i], exContourEps2[i]))
contOutFile.close()
print("Contours written.")

if len(exContourEps2) > 0:
    excContour_g = r.TGraph(len(exContourMass), np.array(exContourMass), np.array(exContourEps2))
    excContour_g.SetName("excContour_g")
    excContour_g.Write()
    excContourLog_g = r.TGraph(len(exContourMass), np.array(exContourMass), np.array(exContourLogEps2))
    excContourLog_g.SetName("excContourLog_g")
    excContourLog_g.Write()

zCuts_g = r.TGraph(len(zCutVals), np.array([float(x) for x in invMasses[:len(zCutVals)]]), np.array(zCutVals))
zCuts_g.SetName("zCuts_g")
zCuts_g.Write()
print("zCutVals:    ", zCutVals)
print("zCutMasRes:  ", zCutMasRes)
print("zCutExpTail: ", zCutExpTail)

dNdm_g = r.TGraph(len(dNdms), np.array([float(x) for x in invMasses[:len(dNdms)]]), np.array(dNdms))
dNdm_g.SetName("dNdm_g")
dNdm_g.Write()

apProd_hh.Write()
Nsig_hh.Write()
gcTau_hh.Write()
effVtx_hh.Write()
outFile.Close()
print("Done. Output: %s" % options.outputFile)
