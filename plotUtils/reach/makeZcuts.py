#/bin/env python
import glob
import numpy as np
import ROOT as r
import utilities as utils
import copy
from optparse import OptionParser


def massRes(mass):
    #res = 6.86902 - 9.88222e-2*mass + 1.34223e-3*mass*mass - 3.56501e-6*mass*mass*mass
    #res = 3.40881 + 4.29919e-2*mass - 5.28684e-5*mass*mass
    res = 1.91442 + 1.87619e-2*mass - 5.60051e-6*mass*mass
    return res


#Lumi = 1100.861649
#Lumi = 11008.61649
Lumi = 125.0

#utils.SetStyle()

parser = OptionParser()

parser.add_option("-i", "--inputFile", type="string", dest="inputFile",
                  help="Name of file to run on.", metavar="inputFile", default="toys/toys.root")
parser.add_option("-o", "--outputFile", type="string", dest="outputFile",
                  help="Specify the output filename.", metavar="outputFile", default="testOut.root")

(options, args) = parser.parse_args()

r.gROOT.SetBatch(1)

#invMasses = [50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100, 105, 110, 115, 120, 125, 130, 135, 140, 145, 150, 175]
invMasses = [75, 100, 150, 200]

#Calculate the weights
mcScale = {}
#mcScale['tritrig'] = 1.416e6*Lumi/492650000.0
#mcScale['wab'] = 0.1985e9*Lumi/996600000.0
mcScale['tritrig'] = 4.566e8*Lumi/9040000
mcScale['wab'] = 4.715e10*Lumi/(4933*20000)

outFile = r.TFile("zcuts.root", "RECREATE")

#Get unbinnded MC after selection
ttFile = r.TFile("/nfs/hps_data2/users/bravo/mc/det19/tritrig/tritrigAnaVtx.root")
ttTree = ttFile.Get("vtxana_Tight_2019/vtxana_Tight_2019_tree")
ttTree.SetName("tritrig_Tight_2019_tree")

wabFile = r.TFile("/nfs/hps_data2/users/bravo/mc/det19/wab/wabAnaVtx.root")
wabTree = wabFile.Get("vtxana_Tight_2019/vtxana_Tight_2019_tree")
wabTree.SetName("wab_Tight_2019_tree")

rand = r.TRandom3()
rand.SetSeed(0)

zcutTxtFile = open("zcuts.dat", "w")
#zcutTxtFile.write("mass/D:zcut/D\n")
zcuts = []
masses = [float(x) for x in invMasses]
for mass in invMasses:
    massF = float(mass)
    print "Running %i MeV with res %f" % (mass, massRes(massF))
    lowMass = massF - 2.8*massRes(massF)/2.0
    highMass = massF + 2.8*massRes(massF)/2.0
    print "%f\t%f\t%f\t%f" % (lowMass, massF, highMass, massRes(massF))
    zVtx_h = r.TH1D("zVtx%i_h" % mass, "zVtx%i_h" % mass, 150, -50.0, 100.0)
    #Fill histogram from unbinned data
    for ev in ttTree:
        if 1000.0*ev.unc_vtx_mass < lowMass:
            continue
        if 1000.0*ev.unc_vtx_mass > highMass:
            continue
        zVtx_h.Fill(ev.unc_vtx_z, mcScale['tritrig'])
        pass
    #for ev in wabTree:
    #    if 1000.0*ev.unc_vtx_mass < lowMass: continue
    #    if 1000.0*ev.unc_vtx_mass > highMass: continue
    #    zVtx_h.Fill(ev.unc_vtx_z, mcScale['wab'])
    #    pass
    #fitFunc = r.TF1("fit%i_f"%mass,"[0]*TMath::Exp([1]*x)", -10.0, 90.0)
    #fitFunc = r.TF1("fit%i_f"%mass,"[0]*TMath::Exp([1]*x)+[2]*TMath::Exp([3]*x)", -10.0, 90.0)
    fitFunc = r.TF1("fitfunc", "[0]*exp( (((x-[1])/[2])<[3])*(-0.5*(x-[1])^2/[2]^2) + (((x-[1])/[2])>=[3])*(0.5*[3]^2-[3]*(x-[1])/[2]))", -100.0, 100.0)
    gausResult = zVtx_h.Fit("gaus", "QS")
    gausParams = gausResult.GetParams()
    gausResult = zVtx_h.Fit("gaus", "QS", "", gausParams[1]-3.0*gausParams[2], gausParams[1]+3.0*gausParams[2])
    gausParams = gausResult.GetParams()
    tailZ = gausParams[1] + 3.0*gausParams[2]
    bestChi2 = -99.9
    bestParams = [999.9, 999.9, 999.9, 999.9]
    bestFitInit = [999.9, 999.9, 999.9, 999.9]
    #for fitI in range(10):
    #    fitInit = [rand.Uniform(500.0, 1500.0),
    #               rand.Uniform(-2.0,-1.0),
    #               rand.Uniform(50, 150),
    #               rand.Uniform(-1.0, -0.5)]
    #    fitFunc.SetParameters(fitInit[0], fitInit[1], fitInit[2], fitInit[3])
    #    #fitResult = zVtx_h.Fit(fitFunc, "LSIM", "", gausParams[1]-2.0*gausParams[2], gausParams[1]+10.0*gausParams[2])
    #    fitResult = zVtx_h.Fit(fitFunc, "LES", "", tailZ, 90.0)
    #    if not fitResult.IsValid(): continue
    #    if fitResult.Chi2() < bestChi2 or bestChi2 < 0.0:
    #        bestChi2 = fitResult.Chi2()
    #        bestParams = fitResult.GetParams()
    #        bestFitInit = fitInit
    #    pass
    fitFunc.SetParameters(gausParams[0], gausParams[1], gausParams[2], 3.0)
    fitResult = zVtx_h.Fit(fitFunc, "LSIM", "", gausParams[1]-2.0*gausParams[2], gausParams[1]+10.0*gausParams[2])
    #fitFunc.SetParameters(bestFitInit[0], bestFitInit[1], bestFitInit[2], bestFitInit[3])
    #fitResult = zVtx_h.Fit(fitFunc, "LES", "", tailZ, 90.0)
    zcut = -6.0
    testIntegral = fitFunc.Integral(zcut, 90.0)
    while testIntegral > 0.5:
        zcut = zcut+0.1
        testIntegral = fitFunc.Integral(zcut, 90.0)
        pass
    print "Zcut: %f" % zcut
    zcuts.append(zcut)
    zcutTxtFile.write("%f\t%f\n" % (massF, zcut))
    outFile.cd()
    zVtx_h.Write()
    pass
zcutTxtFile.close()

utils.SetStyle()
zcuts_g = r.TGraph(len(masses), np.array(masses), np.array(zcuts))
zcuts_g.SetName("zcuts_g")
zcuts_g.SetTitle(";m_{A'} [MeV];z_{cut} [mm]")
zcuts_g.Write()

outFile.Close()
exit(0)

canv = r.TCanvas("canv", "canv", 1400, 1000)
canv.cd()
massResScaled_ge.SetMinimum(0.0)
massResScaled_ge.SetMaximum(15.0)
massResScaled_ge.Draw("ape")
massRes_ge.Draw("pesame")
fitFunc.Draw("same")
utils.InsertText()
canv.SaveAs("massRes.png")

massRes_ge.Write()
massResScaled_ge.Write()

outFile.Close()
