#!/bin/env python
import numpy as np
import ROOT as r
import copy
from optparse import OptionParser
import math

parser = OptionParser()

infile2016 = r.TFile("../2016_reach/reach_results/expSigRate2016_kf_fpi_4Pi.root", "READ")
nsig2016_hh = copy.deepcopy(infile2016.Get("Nsig_low_Lumi_10.7_hh"))
nsig2016_hh.SetName("hps_2016_SigRate_hh")
infile2016.Close()

infile2019 = r.TFile("../2019_reach/reach_results/expSigRate2019_fPi_4pi.root", "READ")
nsig2019_hh = copy.deepcopy(infile2019.Get("Nsig_low_Lumi_110.0_hh"))
nsig2019_hh.SetName("hps_2019_SigRate_hh")
infile2019.Close()

infile2021 = r.TFile("../2021_reach/reach_results/expSigRate2021_fPi_4pi.root", "READ")
nsig2021_hh = copy.deepcopy(infile2021.Get("Nsig_low_Lumi_168.1_hh"))
nsig2021_hh.SetName("hps_2021_SigRate_hh")
infile2021.Close()

low = 25.0*(3./1.8) - 5.0
high = 200.*(3./1.8) + 5.0
nmasses = nsig2021_hh.GetXaxis().GetNbins()
Nsig_hh = r.TH2D("Nsig_2016_2019_2021_hh", "Nsig_2016_2019_2021;m_{A'} [MeV];log_{10}(#epsilon^{2})", nmasses, low, high, 620, -10.005, -3.905)

#2021
for i in range(nsig2021_hh.GetXaxis().GetNbins()):
    mass = nsig2021_hh.GetXaxis().GetBinCenter(i+1)
    #print("mass: ", mass)
    projy = nsig2021_hh.ProjectionY("%s" % (mass), i+1, i+1)
    for ii in range(projy.GetXaxis().GetNbins()):
        logeps2 = projy.GetBinCenter(ii+1)
        nsig = projy.GetBinContent(ii+1)
        Nsig_hh.Fill(mass, logeps2, nsig)
#2019
for i in range(nsig2019_hh.GetXaxis().GetNbins()):
    mass = nsig2019_hh.GetXaxis().GetBinCenter(i+1)
    #print("mass: ", mass)
    projy = nsig2019_hh.ProjectionY("%s" % (mass), i+1, i+1)
    for ii in range(projy.GetXaxis().GetNbins()):
        logeps2 = projy.GetBinCenter(ii+1)
        nsig = projy.GetBinContent(ii+1)
        Nsig_hh.Fill(mass, logeps2, nsig)

#2016
for i in range(nsig2016_hh.GetXaxis().GetNbins()):
    mass = nsig2016_hh.GetXaxis().GetBinCenter(i+1)
    #print("mass: ", mass)
    projy = nsig2016_hh.ProjectionY("%s" % (mass), i+1, i+1)
    for ii in range(projy.GetXaxis().GetNbins()):
        logeps2 = projy.GetBinCenter(ii+1)
        nsig = projy.GetBinContent(ii+1)
        Nsig_hh.Fill(mass, logeps2, nsig)

excArea = 0.0
excThr = 2.3
upExContourMass = []
upExContourEps2 = []
upExContourLogEps2 = []
downExContourMass = []
downExContourLogEps2 = []
downExContourEps2 = []

outfile = r.TFile("reach_2016_2019_2021_fPi_4pi.root", 'RECREATE')
outfile.cd()

for i in range(Nsig_hh.GetXaxis().GetNbins()):
    mass = float(Nsig_hh.GetXaxis().GetBinCenter(i+1))
    prevRate = 0.0
    projy = Nsig_hh.ProjectionY("projy_%s" % (mass), i+1, i+1)
    projy.Write()
    for ii in range(projy.GetXaxis().GetNbins()):
        logEps2 = projy.GetBinCenter(ii+1)
        Nsig = projy.GetBinContent(ii+1)
        eps2 = pow(10, logEps2)

        #Filter out epsilons with no entries
        if projy.GetBinError(ii+1) <= 0.0:
            continue

        if prevRate < excThr and Nsig > excThr:
            downExContourMass.append(mass)
            downExContourEps2.append(logEps2)
            downExContourLogEps2.append(logEps2)
            pass
        if prevRate > excThr and Nsig < excThr:
            excArea += 10.0*(logEps2 - downExContourLogEps2[-1])
            upExContourMass.append(mass)
            upExContourEps2.append(logEps2)
            upExContourLogEps2.append(logEps2)
            pass
        prevRate = Nsig

nsig2021_hh.Write()
nsig2016_hh.Write()
nsig2019_hh.Write()
Nsig_hh.Write()

upExContourMass.reverse()
upExContourEps2.reverse()
upExContourLogEps2.reverse()
exContourMass = upExContourMass + downExContourMass
exContourEps2 = upExContourEps2 + downExContourEps2
exContourLogEps2 = upExContourLogEps2 + downExContourLogEps2
exContourEps = [math.sqrt(pow(10, x)) for x in exContourEps2]
exContourEps.append(exContourEps[0])
exContourMass.append(exContourMass[0])

#contOutFile = open("sums/excContour%isw%iw.txt"%(subWeeks, nWeeks),"w")
#contOutFile = open("excContour2019plus2021.txt",'w')
#for i in range(len(exContourMass)):
#    contOutFile.write("%f\t%E\n"%(exContourMass[i], exContourLogEps2[i]))
#    pass
#contOutFile.close()

print(len(exContourMass))
print(len(exContourLogEps2))
if len(exContourEps) > 0:
    contOutFile = open("simps_reach_estimate_16_19_21_fpi_4pi.txt", "w")
    for i in range(len(exContourMass)):
        contOutFile.write("%f\t%E\n" % (exContourMass[i], exContourEps[i]))
        pass
    contOutFile.close()
    excContour_g = r.TGraph(len(exContourMass), np.array(exContourMass), np.array(exContourEps))
    excContour_g.SetName("excContour_g")
    excContour_g.Write()

    #excContourLog_g = r.TGraph(len(exContourMass), np.array(exContourMass), np.array(exContourLogEps2))
    #excContourLog_g.SetName("excContourLog_g")
    #excContourLog_g.Write()
    pass
outfile.Close()
print("Excluded Area: %f" % excArea)
