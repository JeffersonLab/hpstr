from tabulate import tabulate
import utilities as utils
import os,sys
import ROOT as r



#Compute the efficiencies - TODO improve this.

def getEff(thisCut,previousCut):
    effs=[]
    #The first is the cut name
    for ival in range(1,len(thisCut)):
        if (previousCut[ival] > -0.5 and thisCut[ival] > -0.5):
            if (abs(previousCut[ival]) > 1e-8):
                print(thisCut[ival],previousCut[ival])
                eff=thisCut[ival] / previousCut[ival]
            else:
                eff=-1.
            effs.append(eff)
    
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
    for cf_h in hList:
        v_yield=cf_h.GetBinContent(icut)
        cutEntry.append(cf_h.GetBinContent(icut))
        #place holder for eff
        cutEntry.append(-1)
    if (debug):
        print(cutEntry)
    table.append(cutEntry)

    
#Get Efficiencies - TODO FIX
for ientry in range(len(table)):
    print("table entry",table[ientry])
    if (ientry<2):
        table[ientry][1]=-1
        table[ientry][3]=-1
    else:
        effs = getEff(table[ientry-1],table[ientry-2])
        print effs
        table[ientry][1]=effs[0]
        table[ientry][3]=effs[1]
        print(effs)



#Print out
headers = ["cut", "sample1","eff","sample2","eff"]
outfile=open("test.txt","w")
outfile.write(tabulate(table, headers, tablefmt="latex"))
outfile.close()

print tabulate(table, headers, tablefmt="latex")
