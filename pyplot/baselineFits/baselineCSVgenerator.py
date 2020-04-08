import ROOT as r
import numpy as np
import ModuleMapper as mmap
import csv
from optparse import OptionParser

parser = OptionParser()

parser.add_option("-i", type="string",dest="offlineFile", help = "ROOT file containin offline baselinefits in Flat-Tuple format",default="")

parser.add_option("-b", type="string",dest="onlineFile", help = "ROOT file containing online baseline fits in TGraph format",default="")

parser.add_option("-o", type="string",dest="outFile", help = "output csv file",default="baselines.csv")

(options,args) = parser.parse_args()

offlineFile = r.TFile(options.offlineFile,"READ")
onlineFile = r.TFile(options.onlineFile,"READ")


Feb = []
Hybrid = []
Channel = []
Mean = []
Sigma = []

onlineFile.cd("baseline")
onlineKeys = []
for key in r.gDirectory.GetListOfKeys():
    onlineFile.cd("baseline")
    kname = key.GetName()
    print kname
    classType = key.GetClassName()
    if classType.find("TGraphErrors") != -1:
        onlineKeys.append(kname)
        TGraph = onlineFile.Get("baseline/%s"%(kname))
        onCh = TGraph.GetX()
        onMean = TGraph.GetY()

        febhyb = kname.replace('baseline_','').replace('_ge','')
        string = mmap.hw_to_str(febhyb)

        offlineFile.cd()
        myTree = offlineFile.gaus_fit
        for fitData in myTree:
            if (fitData.SvtAna2DHisto_key).find(string):
                if ((fitData.lowdaq == 1.0 or fitData.minbinFail == 1.0 or string.find("L4") != -1 or string.find("L5") != -1 or string.find("L6") != -1) and fitData.channel < len(onCh)):

                    Feb.append(febhyb[0:2])
                    Hybrid.append(febhyb[2:])
                    cc = int(fitData.channel)
                    Channel.append(onCh[cc])
                    Mean.append(onMean[cc])
                    Sigma.append(TGraph.GetErrorY(cc))

                elif fitData.channel < len(onCh):

                    Feb.append(febhyb[0:2])
                    Hybrid.append(febhyb[2:])
                    cc = int(fitData.channel)
                    Channel.append(cc)
                    Mean.append(fitData.BlFitMean)
                    Sigma.append(fitData.BlFitSigma)


rows = zip(Feb,Hybrid,Channel,Mean,Sigma)
with open(options.outFile,'w') as file:
    writer = csv.writer(file)
    for row in rows:
        writer.writerow(row)
                

            


            



