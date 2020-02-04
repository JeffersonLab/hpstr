import ROOT as r
from copy import deepcopy
import os.path
import utilities as utils
utils.SetStyle()
r.gStyle.SetOptStat(11111111)
from optparse import OptionParser


parser=OptionParser()
parser.add_option("-i", "--inFile", type="string", dest="inFilename", help="Input file name", 
        default="")
parser.add_option("-b", "--baseline", type="string", dest="baseline", help="baseline<0-5>",
        default="")
parser.add_option("-s", "--hybrid", type="string", dest="hybrid",
        help="L<#><T/B>_<axial/stereo>_<ele/pos>", default="")
(options, args) = parser.parse_args()
baseline= "baseline"+options.baseline
hybrid =options.hybrid

def gethistos2DKeys(inFile):
    inFile.cd()
    histos2D=[]
    for key in inFile.GetListOfKeys():
        kname = key.GetName()
        classType = key.GetClassName()
        if classType.find("TH2") != -1 and kname.find(options.baseline) != -1 and kname.find(options.hybrid) != -1:
            histos2D.append(kname)
    return histos2D
              
def readhistoFromFile(inFile, histo_key):
    histo_hh = deepcopy(getattr(inFile, histo_key))
    return histo_hh


######################################################################################################
SvtBl2D_file = options.inFilename
out_File = '%s/%s_hitsperChannel.root'%(os.path.dirname(SvtBl2D_file),SvtBl2D_file[:-5])

inFile = r.TFile(SvtBl2D_file, "READ")
outFile = r.TFile(out_File, "RECREATE")
hitsperChannel_h=r.TH1F("hitsperChannel","raw_hits_per_channel;nhits;Events",5000,0,60000) 
histokeys_hh = gethistos2DKeys(inFile)
#6 baseline measurements, "baseline0" -> "baseline5"
nhitsperCCtotal=[]
nhitsperCCtsM=[]
for ts in range(1):
    nhitsperCCts=[]
    for k in histokeys_hh:
        if k.find("baseline"+str(ts)) != -1:
            histo_hh = readhistoFromFile(inFile, k)
            nbins = histo_hh.GetXaxis().GetNbins()
            print k
            for i in range(nbins):
                projy=histo_hh.ProjectionY('%s_ch%i_h'%(k,i),i+1,i+1,"e")
                nEntries=projy.GetEntries()
                nhitsperCCts.append(nEntries)
    nhitsperCCtsM.append(nhitsperCCts)

nhitsperCCtotal=[]
for i in range(len(nhitsperCCtsM[0])):
    for ts in range(len(nhitsperCCtsM)):
        ccSum = 0
        ccSum = ccSum + nhitsperCCtsM[ts][i]
    hitsperChannel_h.Fill(float(ccSum),1.)
hitsperChannel_h.GetXaxis().SetRange(0,hitsperChannel_h.FindLastBinAbove(0,1))


outFile.cd()
#hitsperChannel_h.GetXaxis().SetRangeUser(0,hitsperChannel_h.FindLastBinAbove(0,1))
#hitsperChannel_h.GetYaxis().SetRangeUser(0,1000)
hitsperChannel_h.Write()

