from tabulate import tabulate
import utilities as utils
import os,sys
import ROOT as r



#Compute the efficiencies - TODO improve this.

def getEff(thisCut,previousCut):
    effs=[]
    #The first is the cut name
    for ival in range(1,len(thisCut)):
        if (abs(previousCut[ival]) > 1e-8):
            print(thisCut[ival],previousCut[ival])
            eff=thisCut[ival] / previousCut[ival]
        else:
            eff=-1.0
        effs.append(thisCut[ival])
        if (eff==-1.0):
            effs.append("--")
        else:
            effs.append(round(eff,3))
    
    return effs


#Parse the command line
parser=utils.parser
(config,sys.argv[1:]) = parser.parse_args(sys.argv[1:])

debug=config.debug


#Get the inputFileList 
fList = []
if (config.inputFiles):
    fList=(config.inputFiles).split(",")
    print("inputFiles:",fList)

#Selection:
sel="vtxSelection"
if (config.selection):
    sel = config.selection

#Get the cutFlowHistograms and fill the yield entries
hList = []
nHists=0

for inF in fList:
    cf_h=utils.getPlot(inF,sel+"/"+sel+"_cutflow")
    hList.append(cf_h)
    nHists+=1

#Get the cuts: assume all cut flows are the same for that particular selection
nCuts = hList[0].GetXaxis().GetNbins()
cuts=[]
for icut in range(1,nCuts+1):
    cut=hList[0].GetXaxis().GetBinLabel(icut)
    cuts.append(cut)

#Fill the yield tables - Get one entry for each read in histogram
table = []

for icut in range(1,nCuts+1):
    cutEntry=[]
    cutEntry.append(cuts[icut-1])
    for cf_h in hList:
        v_yield=cf_h.GetBinContent(icut)
        cutEntry.append(cf_h.GetBinContent(icut))
        #place holder for eff
        #cutEntry.append(-1)
    if (debug):
        print(cutEntry)
    table.append(cutEntry)

table_wEffs = []    
#Get Efficiencies - TODO FIX
for ientry in range(len(table)):
    print("table entry",table[ientry])
    yields=table[ientry][1:]
    print(yields)
    effs=[table[ientry][0]]
    if (ientry<1):
        for yi in yields:
            effs.append(yi)
            effs.append("--")
        table_wEffs.append(effs)
    else:
        if (debug):
            print("Eff: "+table[ientry][0] +"/"+table[ientry-1][0])
        effs += getEff(table[ientry],table[ientry-1])
        table_wEffs.append(effs)
        
    



#effs out
headers = ["cut", "sample1","eff","sample2","eff"]
outfile=open("test.txt","w")
outfile.write(tabulate(table, headers, tablefmt="latex"))
outfile.close()

print tabulate(table, headers, tablefmt="latex")
print tabulate(table_wEffs, headers, tablefmt="latex")
