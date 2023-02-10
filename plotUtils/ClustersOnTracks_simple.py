from ROOT import *
import os
from utilities import *


SetStyle()

#inFileList = ["hists_10494_bin5.root",
#              "hists_10491_bin5.root",
#              "hists_10492_bin5.root"]


#path = "/Users/"+os.environ["USER"]+"/Dropbox/HPS/macros/hists/"
path = "/data/run/hists/"
inFileList = [
    "projections_10710.root",
    "projections_10711.root",
    "projections_10712.root",
    "projections_10713.root",
    "projections_10714.root"]


graphs = []
colors = [kBlack, kRed, kBlue, kGreen+2, kOrange-2]

inputFiles = []
#legends    = ["10494","10491: L0-1=70V, L2-3=270V, L4-6=180V","10492 L0-1=40V L2-3=100V L4-6=180V"]
legends = ["10710", "10711", "10712", "10713", "10714"]
outdir = "./Plots_BiasScan"

if not os.path.exists(outdir):
    os.makedirs(outdir)

gROOT.SetBatch(1)

for ifile in inFileList:
    inf = TFile(path+"/"+ifile)
    inputFiles.append(inf)

for key in inputFiles[0].GetListOfKeys():
    print key.GetName()

    #TODO do it better
    if "mean" not in key.GetName():
        continue
    if "charge" not in key.GetName():
        continue
    c = TCanvas()
    leg = TLegend(0.3, 0.4, 0.7, 0.15)
    leg.SetBorderSize(0)
    for i_f in range(0, len(inputFiles)):
        graphs.append(inputFiles[i_f].Get(key.GetName()))
        graphs[i_f].SetMarkerColor(colors[i_f])
        graphs[i_f].SetLineColor(colors[i_f])

        if i_f == 0:
            graphs[i_f].Draw("AP")
            tYoff = graphs[i_f].GetYaxis().GetTitleOffset()
            graphs[i_f].GetYaxis().SetTitleOffset(tYoff*0.75)
            graphs[i_f].GetYaxis().SetRangeUser(400, 1700)
            labelSize = graphs[i_f].GetYaxis().GetLabelSize()
            graphs[i_f].GetYaxis().SetLabelSize(labelSize*0.7)
            graphs[i_f].GetXaxis().SetLabelSize(labelSize*0.7)
            graphs[i_f].GetYaxis().SetTitleSize(graphs[i_f].GetYaxis().GetTitleSize() * 0.6)
            graphs[i_f].GetYaxis().SetTitleOffset(graphs[i_f].GetYaxis().GetTitleOffset() * 2.2)
            graphs[i_f].GetXaxis().SetTitleSize(graphs[i_f].GetXaxis().GetTitleSize() * 0.6)
            graphs[i_f].GetXaxis().SetTitleOffset(graphs[i_f].GetXaxis().GetTitleOffset() * 1)
            graphs[i_f].GetXaxis().SetTitle("strip Position")

        else:
            graphs[i_f].Draw("Psame")
        entry = leg.AddEntry(legends[i_f], legends[i_f], "l")
        entry.SetLineColor(colors[i_f])
    leg.Draw()
    InsertText("Data 2019")

    c.SaveAs(outdir+"/"+key.GetName()+".pdf")

    graphs = []
