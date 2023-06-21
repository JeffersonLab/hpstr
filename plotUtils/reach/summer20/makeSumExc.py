#!/bin/env python
import numpy as np
import ROOT as r
import copy
from optparse import OptionParser

parser = OptionParser()

parser.add_option("-n", "--nWeeks", type="int", dest="nWeeks",
                  help="The number of weeks worth of lumi.", metavar="nWeeks", default=16)
parser.add_option("-s", "--subWeeks", type="int", dest="subWeeks",
                  help="The number of weeks worth of lumi at 2.3 GeV.", metavar="subWeeks", default=6)

(options, args) = parser.parse_args()

nWeeks = options.nWeeks
subWeeks = options.subWeeks
#e2pt3Weeks = subWeeks
#e3pt7Weeks = nWeeks - subWeeks
e2pt3Weeks = 4
e3pt7Weeks = 3.5

run19File = r.TFile("expSigRate2019.root")
run19SigRate_hh = copy.deepcopy(run19File.Get("Nsig_hh"))
run19SigRate_hh.SetName("run19SigRate_hh")
run19File.Close()

e2pt3File = r.TFile("sigRateE2pt3Nw%i.root" % e2pt3Weeks)
e2pt3SigRate_hh = copy.deepcopy(e2pt3File.Get("Nsig_hh"))
e2pt3SigRate_hh.SetName("e2pt3SigRate_hh")
e2pt3File.Close()

#e3pt7File = r.TFile("sigRateE3pt7Nw%i.root"%e3pt7Weeks)
e3pt7File = r.TFile("sigRateE3pt7Nw3pt5.root")
e3pt7SigRate_hh = copy.deepcopy(e3pt7File.Get("Nsig_hh"))
e3pt7SigRate_hh.SetName("e3pt7SigRate_hh")
e3pt7File.Close()

#run19SigRate_hh = e2pt3SigRate_hh
#run19SigRate_hh.Add(e2pt3SigRate_hh)
run19SigRate_hh.Add(e3pt7SigRate_hh)

excArea = 0.0
excThr = 2.3
upExContourMass = []
upExContourEps2 = []
upExContourLogEps2 = []
downExContourMass = []
downExContourLogEps2 = []
downExContourEps2 = []
for mass in range(40, 211, 10):
    massF = float(mass)
    prevRate = 0.0
    for logEps2 in range(-1000, -690):
        logEps2 = logEps2/100.0
        eps2 = pow(10, logEps2)
        Nsig = run19SigRate_hh.GetBinContent(run19SigRate_hh.FindBin(massF, logEps2))
        if prevRate < excThr and Nsig > excThr:
            downExContourMass.append(massF)
            downExContourEps2.append(eps2)
            downExContourLogEps2.append(logEps2)
            pass
        if prevRate > excThr and Nsig < excThr:
            excArea += 10.0*(logEps2 - downExContourLogEps2[-1])
            upExContourMass.append(massF)
            upExContourEps2.append(eps2)
            upExContourLogEps2.append(logEps2)
            pass
        prevRate = Nsig
        pass
    pass
upExContourMass.reverse()
upExContourEps2.reverse()
upExContourLogEps2.reverse()
exContourMass = upExContourMass + downExContourMass
exContourEps2 = upExContourEps2 + downExContourEps2
exContourLogEps2 = upExContourLogEps2 + downExContourLogEps2
#contOutFile = open("sums/excContour%isw%iw.txt"%(subWeeks, nWeeks),"w")
contOutFile = open("excContour2019plus2021.txt", 'w')
for i in range(len(exContourMass)):
    contOutFile.write("%f\t%E\n" % (exContourMass[i], exContourLogEps2[i]))
    pass
contOutFile.close()

#outFile = r.TFile("sums/sum%isw%iw.root"%(subWeeks, nWeeks),"RECREATE")
outFile = r.TFile("expSigExc2019plus2021.root", "RECREATE")
outFile.cd()
run19SigRate_hh.Write()
if len(exContourEps2) > 0:
    excContour_g = r.TGraph(len(exContourMass), np.array(exContourMass), np.array(exContourEps2))
    excContour_g.SetName("excContour_g")
    excContour_g.Write()

    excContourLog_g = r.TGraph(len(exContourMass), np.array(exContourMass), np.array(exContourLogEps2))
    excContourLog_g.SetName("excContourLog_g")
    excContourLog_g.Write()
    pass
outFile.Close()
print("E2pt3: %i and E3pt7: %i" % (e2pt3Weeks, e3pt7Weeks))
print("Excluded Area: %f" % excArea)
