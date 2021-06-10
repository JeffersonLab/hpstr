import ROOT as r
import argparse
import numpy as np




parser=argparse.ArgumentParser(description="Configuration options for baseline fit")
parser.add_argument("-i", type=str, dest="inFilename", help="Input SvtBlFitHistoProcessor output root file",default="")
parser.add_argument("-o", type=str, dest="outFilename", help="output root file",default="baselineFitTool.root")

options = parser.parse_args()

#############################################################################################################################
def getKeysFromFile(inFile,cType="", attr1=[""], attr2=""):
    histo_keys=[]
    for hybrid in inFile.GetListOfKeys():
        kname = hybrid.GetName()
        classType = hybrid.GetClassName()
        for string in attr1:
            if classType.find(cType) != -1 and kname.find(string) != -1 and kname.find(attr2) != -1:
                histo_keys.append(kname)
    return histo_keys


################################################################################################################################

outFile = r.TFile(options.outFilename,"RECREATE")
inFile = r.TFile(options.inFilename, "READ")


keys_hh = getKeysFromFile(inFile, "TH2", [""], "")

rms_h = r.TH1F("RMS Distribution","RMS_Distribution;sigma;events",500,0.0,500.)
rms_h.SetLineWidth(2)
canvas1 = r.TCanvas("RMS_L0-L3","RMS_L0-L3",1800,800)
canvas2 = r.TCanvas("RMS_L4-L6","RMS_L4-L6",1800,800)
canvas1.cd()

multigr1 = r.TMultiGraph()
multigr1.SetTitle("Channel_RMS_Hybrids_L0-L3")
multigr2 = r.TMultiGraph()
multigr2.SetTitle("Channel_RMS_Hybrids_L4-L6")
markerStyle = 1
for key in keys_hh:
    print(key)    
    inFile.cd()
    hh = inFile.Get(key)

    maxchannel = 640
    if(key.find("L0") != -1 or key.find("L1") != -1):
        maxchannel = 512
    RMS = []
    channel = []
    for cc in range(maxchannel):
        channel.append(cc)
        #print(hh.GetBinContent(cc+1))
        #if(hh.GetBinContent(cc+1) == 0):
        #    RMS.append(-100)
        #    continue;
        yproj_h = hh.ProjectionY('%s_ch%i_h'%(key,cc),int(cc+1),int(cc+1),"e")
        rms = yproj_h.GetRMS()
        rms_h.Fill(rms,1.)
        RMS.append(rms);
        
    outFile.cd()
    gr = r.TGraph(len(channel),np.array(channel, dtype = float), np.array(RMS, dtype = float))
    gr.SetName("%s_RMS"%(key))
    gr.SetTitle("%s_RMS;Channel;RMS"%(key))
    gr.SetLineWidth(2)
    gr.SetMarkerStyle(markerStyle)
    markerStyle = markerStyle + 1
    if(markerStyle > 4):
        markerStyle = 1
    gr.Draw("P")
    gr.Write()
    #multigr1.Add(gr, "LP")
    if(key.find("L0") != -1 or key.find("L1") != -1 or key.find("L2") != -1  or key.find("L3") != -1):
        multigr1.Add(gr,"LP")
    else:
        multigr2.Add(gr,"LP")

outFile.cd()
#multigr1.GetHistogram().GetYaxis().SetRangeUser(-15.,500.)
multigr1.Draw("A pmc plc")
legend = canvas1.BuildLegend()
canvas1.Write()
canvas2.cd()
multigr2.Draw("A pmc plc")
canvas2.BuildLegend()
canvas2.Write()
rms_h.Draw()
rms_h.Write()
outFile.Write()



