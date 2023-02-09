import ROOT as r
import os
import utilities as utils


utils.SetStyle()

path = "/home/bravo/hps3/users/bravo/sim/det16/singleMuon/"
inFileList = [
    "slic/slicSingleMu4deg_anaMC.root",
    "hps-sim/muonHpsSim_anaMC.root"]


colors = [r.kBlack, r.kRed, r.kBlue, r.kGreen+2, r.kOrange-2]

inputFiles = []
legends = ["slic", "hps-sim"]
outdir = "./"

if not os.path.exists(outdir):
    os.makedirs(outdir)

r.gROOT.SetBatch(1)

for ifile in inFileList:
    inf = r.TFile(path+"/"+ifile)
    inputFiles.append(inf)
    pass

canvs = []
for key in inputFiles[0].GetListOfKeys():
    histos = []
    print key.GetName()
    c = r.TCanvas()
    for i_f in range(0, len(inputFiles)):
        histos.append(inputFiles[i_f].Get(key.GetName()))
        histos[i_f].SetMarkerColor(colors[i_f])
        histos[i_f].SetLineColor(colors[i_f])
        pass
    canvs.append(utils.MakePlot(key.GetName(), outdir, histos, legends, ".png", LogY=True, RatioType="Sequential"))
    pass

outF = r.TFile("slicSingleMu_compMC.root", "RECREATE")
outF.cd()
for canv in canvs:
    canv.Write()
outF.Close()
