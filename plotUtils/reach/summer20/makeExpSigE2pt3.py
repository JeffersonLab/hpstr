#/bin/env python
import copy
import math
import glob
import numpy as np
import ROOT as r
import utilities as utils
from optparse import OptionParser


def radFrac(mass):
    mass = mass/1000.0
    radF = (-5.17784e-2 + 1.31019*mass + 3.37933e1*pow(mass, 2) - 7.48190e2*pow(mass, 3) + 4.78523e3*pow(mass, 4) - 1.00723e4*pow(mass, 5))
    return radF


def vtxRes(mass):
    res = (2.97795 - 46.4387*mass + 319.229*pow(mass, 2) - 783.897*pow(mass, 3))
    return res


def massRes(mass):
    res = (4.29173 - 4.14727e-02*mass + 3.91206e-4*mass*mass)
    return res


def calcLifetime(mass, eps2):
    return 8.0*(2.3/10.0)*(1e-8/eps2)*pow(100.0/mass, 2)
    #gamma = 0.95
    #hbar_c = 1.973e-13
    #ct = hbar_c*3.0/(mass*(1/137.036)*10**logEps2)
    #gammact = hbar_c*3.0*2.3*gamma/(massFGeV*massFGeV*(1/137.036)*10**logEps2)
    #print "logEps2: %f    gcTau: %f    gammact: %f    percent diff: %f"%(logEps2, gcTau, gammact, 1 - gammact/gcTau)
    #gcTau = gammact


utils.SetStyle()

parser = OptionParser()

parser.add_option("-i", "--inputFile", type="string", dest="inputFile",
                  help="Name of file to run on.", metavar="inputFile", default="toys/toys.root")
parser.add_option("-o", "--outputFile", type="string", dest="outputFile",
                  help="Specify the output filename.", metavar="outputFile", default="expSigRate.root")
parser.add_option("-z", "--zcutFile", type="string", dest="zcutFile",
                  help="Name of file containing zcut values.", metavar="zcutFile",
                  default="/data/src/hpstr/run/reach/zcut/zcuts.dat")
parser.add_option("-n", "--nWeeks", type="int", dest="nWeeks",
                  help="The number of weeks worth of lumi.", metavar="nWeeks", default=6)

(options, args) = parser.parse_args()

#Calculate the weights
nWeeks = options.nWeeks
iLumi = 19.0
Lumi = nWeeks*iLumi*0.93
mcScale = {}
mcScale['tritrig'] = (1.416E+09*Lumi/(978*10000))
mcScale['wab'] = (1.985E+11*Lumi/(9966*10000))

zCutVals = []
dNdms = []
#invMasses = [30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180, 190, 200, 210]
invMasses = range(64, 70, 2)

zcuts = {}
zcutFile = open(options.zcutFile, "r")
for line in zcutFile:
    lineList = line.split()
    zcuts[float(lineList[0])] = float(lineList[1])
    pass

outFile = r.TFile(options.outputFile, "RECREATE")

nMasses = len(invMasses)
#lowM = float(invMasses[0] - 5.0)
#highM = float(invMasses[-1] + 5.0)
lowM = float(invMasses[0] - 1.0)
highM = float(invMasses[-1] + 1.0)
apProd_hh = r.TH2D("apProd_hh", ";m_{A'} [MeV];log_{10}(#epsilon^{2})", nMasses, lowM, highM, 310, -10.005, -6.905)
Nsig_hh = r.TH2D("Nsig_hh", ";m_{A'} [MeV];log_{10}(#epsilon^{2})", nMasses, lowM, highM, 310, -10.005, -6.905)
effVtx_hh = r.TH2D("effVtx_hh", ";m_{A'} [MeV];log_{10}(#epsilon^{2})", nMasses, lowM, highM, 310, -10.005, -6.905)
gcTau_hh = r.TH2D("gcTau_hh", ";m_{A'} [MeV];log_{10}(#epsilon^{2})", nMasses, lowM, highM, 310, -10.005, -6.905)

upExContourMass = []
upExContourEps2 = []
upExContourLogEps2 = []
downExContourMass = []
downExContourLogEps2 = []
downExContourEps2 = []
for mass in invMasses:
    massF = float(mass)
    massFGeV = massF/1000.0
    print "Running mass = %i MeV" % mass
    #First grab the pretrigger vtx z distribution
    apSimFilename = "/nfs/slac/g/hps_data2/users/bravo/mc/det19/2pt3/anaMCAp%ie2pt3.root" % (mass)
    apSimFile = r.TFile(apSimFilename)
    apSimZ_hcp = copy.deepcopy(apSimFile.Get("mcAna/mcAna_mc622Z_h"))
    apSimFile.Close()
    apSimZ_hcp.SetName("apSimZ%i_hcp" % mass)
    apSimZ_h = r.TH1F("apSimZ%i_h" % mass, ";true z_{vtx} [mm];MC Events", 200, -50.0, 150.0)
    for i in range(201):
        if i >= 195:
            continue
        apSimZ_h.SetBinContent(i, apSimZ_hcp.GetBinContent(i))
        pass
    outFile.cd()
    apSimZ_h.Write()
    #Next count the differential background rate in 1 MeV bin
    dNdm = 0.0
    ttFile = r.TFile("/nfs/slac/g/hps_data2/users/bravo/mc/det19/2pt3/tritrigAnaVtxE2pt3Sel3.root")
    ttTree = ttFile.Get("vtxana_Tight_e2pt3/vtxana_Tight_e2pt3_tree")
    ttTree.SetName("tritrig_Tight_e2pt3_tree")
    print "Counting background rate"
    Mbin = 4.0
    for ev in ttTree:
        if 1000.0*ev.unc_vtx_mass > massF + (Mbin/2):
            continue
        if 1000.0*ev.unc_vtx_mass < massF - (Mbin/2):
            continue
        dNdm += mcScale['tritrig']
        pass
    ttFile.Close()

    wabFile = r.TFile("/nfs/slac/g/hps_data2/users/bravo/mc/det19/2pt3/wabAnaVtxE2pt3Sel3.root")
    wabTree = wabFile.Get("vtxana_Tight_e2pt3/vtxana_Tight_e2pt3_tree")
    wabTree.SetName("wab_Tight_e2pt3_tree")
    for ev in wabTree:
        if 1000.0*ev.unc_vtx_mass > massF + (Mbin/2):
            continue
        if 1000.0*ev.unc_vtx_mass < massF - (Mbin/2):
            continue
        dNdm += mcScale['wab']
        pass
    dNdm = dNdm/Mbin
    dNdms.append(dNdm)
    wabFile.Close()
    print "Background Rate: %f" % dNdm

    #Next get flat tuple from anaVtx and fill eff_vtx numerator
    lowMass = massF - 2.8*massRes(massF)/2.0
    highMass = massF + 2.8*massRes(massF)/2.0
    #zCut = zcuts[massF] - 2.0
    zCut = 0.5*math.log(Lumi) + 7.0*vtxRes(massFGeV)
    zCutVals.append(zCut)
    apSelZ_h = r.TH1F("apSelZ%i_h" % mass, ";true z_{vtx} [mm];MC Events", 200, -50.0, 150.0)
    apSelNoZ_h = r.TH1F("apSelNoZ%i_h" % mass, ";true z_{vtx} [mm];MC Events", 200, -50.0, 150.0)
    apFilename = "/nfs/slac/g/hps_data2/users/bravo/mc/det19/2pt3/anaVtxAp%iE2pt3Sel3.root" % (mass)
    apFile = r.TFile(apFilename)
    apTree = apFile.Get("vtxana_Tight_e2pt3/vtxana_Tight_e2pt3_tree")
    print "Counting Signal"
    for ev in apTree:
        if 1000.0*ev.unc_vtx_mass > highMass:
            continue
        if 1000.0*ev.unc_vtx_mass < lowMass:
            continue
        if ev.true_vtx_z > 138.0:
            continue
        apSelNoZ_h.Fill(ev.true_vtx_z)
        if ev.unc_vtx_z < zCut:
            continue
        apSelZ_h.Fill(ev.true_vtx_z)
        pass
    apFile.Close()

    #Make the efficiencies
    apEffVtxZ_gae = r.TGraphAsymmErrors(apSelZ_h, apSimZ_h, "shortest")
    apEffVtxZ_gae.SetName("apEffVtxZ%i_gae" % mass)
    apEffVtxZ_e = r.TEfficiency(apSelZ_h, apSimZ_h)
    apEffVtxZ_e.SetName("apEffVtxZ%i_e" % mass)

    apEffVtxNoZ_gae = r.TGraphAsymmErrors(apSelNoZ_h, apSimZ_h, "shortest")
    apEffVtxNoZ_gae.SetName("apEffVtxNoZ%i_gae" % mass)
    apEffVtxNoZ_e = r.TEfficiency(apSelNoZ_h, apSimZ_h)
    apEffVtxNoZ_e.SetName("apEffVtxNoZ%i_e" % mass)

    Seff = (apEffVtxNoZ_e.GetEfficiency(51) + apEffVtxNoZ_e.GetEfficiency(52) + apEffVtxNoZ_e.GetEfficiency(53) + apEffVtxNoZ_e.GetEfficiencyErrorUp(51) + apEffVtxNoZ_e.GetEfficiencyErrorUp(52) + apEffVtxNoZ_e.GetEfficiencyErrorUp(53))/3.0
    #Seff = (apEffVtxNoZ_e.GetEfficiency(51) + apEffVtxNoZ_e.GetEfficiency(52) + apEffVtxNoZ_e.GetEfficiency(53) )/3.0
    apEffVtxZ_gae.SetMaximum(Seff*2.0)
    apEffVtxNoZ_gae.SetMaximum(Seff*2.0)
    if Seff > 0:
        Seff = 1.0/Seff
    print "Seff: %f" % Seff
    outFile.cd()
    apSelNoZ_h.Write()
    apSelZ_h.Write()
    apEffVtxNoZ_gae.Write()
    apEffVtxZ_gae.Write()
    apEffVtxNoZ_e.Write()
    apEffVtxZ_e.Write()
    effCalc_h = apEffVtxZ_e
    prevRate = 0.0
    excThr = 2.3
    for logEps2 in range(-1000, -690):
        logEps2 = logEps2/100.0
        eps2 = pow(10, logEps2)
        gcTau = calcLifetime(massF, eps2)
        effVtx = 0.0
        for zbin in range(1, 201):
            zz = apSelZ_h.GetBinCenter(zbin)
            zzLow = apSelZ_h.GetBinLowEdge(zbin)
            zzHigh = zzLow + apSelZ_h.GetBinWidth(zbin)
            if zz < 2.0:
                continue
            #if zzHigh < zCut: continue
            #if zzHigh > zCut and zzLow < zCut: effVtx += (r.TMath.Exp((-4.3-zz)/gcTau)/gcTau)*effCalc_h.GetBinContent(zbin)*(zzHigh - zCut)
            effVtx += (r.TMath.Exp((0.0-zz)/gcTau)/gcTau)*Seff*effCalc_h.GetEfficiency(zbin)*apSelZ_h.GetBinWidth(zbin)
            pass
        apProd = 205.5*3.1416*eps2*massF*radFrac(massF)*dNdm
        Nsig = 205.5*3.1416*eps2*massF*radFrac(massF)*dNdm*effVtx
        apProd_hh.Fill(massF, logEps2, apProd)
        Nsig_hh.Fill(massF, logEps2, Nsig)
        effVtx_hh.Fill(massF, logEps2, effVtx)
        gcTau_hh.Fill(massF, logEps2, gcTau)
        if prevRate < excThr and Nsig > excThr:
            downExContourMass.append(massF)
            downExContourEps2.append(eps2)
            downExContourLogEps2.append(logEps2)
            pass
        if prevRate > excThr and Nsig < excThr:
            upExContourMass.append(massF)
            upExContourEps2.append(eps2)
            upExContourLogEps2.append(logEps2)
            pass
        prevRate = Nsig
        pass
    pass
print("Now Save it all")
upExContourMass.reverse()
upExContourEps2.reverse()
upExContourLogEps2.reverse()
exContourMass = upExContourMass + downExContourMass
exContourEps2 = upExContourEps2 + downExContourEps2
exContourLogEps2 = upExContourLogEps2 + downExContourLogEps2
contOutFile = open("excContourE2pt3Nw%iLowMass.txt" % nWeeks, "w")
for i in range(len(exContourMass)):
    contOutFile.write("%f\t%E\n" % (exContourMass[i], exContourEps2[i]))
    pass
contOutFile.close()
print("Contours done")

if len(exContourEps2) > 0:
    excContour_g = r.TGraph(len(exContourMass), np.array(exContourMass), np.array(exContourEps2))
    excContour_g.SetName("excContour_g")
    excContour_g.Write()

    excContourLog_g = r.TGraph(len(exContourMass), np.array(exContourMass), np.array(exContourLogEps2))
    excContourLog_g.SetName("excContourLog_g")
    excContourLog_g.Write()
    pass

zCuts_g = r.TGraph(len(invMasses), np.array([float(x) for x in invMasses]), np.array(zCutVals))
zCuts_g.SetName("zCuts_g")
zCuts_g.Write()

dNdm_g = r.TGraph(len(invMasses), np.array([float(x) for x in invMasses]), np.array(dNdms))
dNdm_g.SetName("dNdm_g")
dNdm_g.Write()

apProd_hh.Write()
Nsig_hh.Write()
gcTau_hh.Write()
effVtx_hh.Write()
outFile.Close()
