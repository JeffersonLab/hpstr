import ROOT as r
import os
import utilities as utils


utils.SetStyle()

path = "/Users/tongtongcao/research/hps/analysis/2019/experiment/ecalTimingStudies/hps_010688"
inFileList = [
    "ecalTimingStuides.root"]



colors = [r.kRed]

inputFiles = []
legends     = []
outdir     = "./"

if not os.path.exists(outdir):
    os.makedirs(outdir)

r.gROOT.SetBatch(1)

for ifile in inFileList:
    inf = r.TFile(path+"/"+ifile)
    inputFiles.append(inf)
    pass

c = r.TCanvas()
canvs = []
for key in inputFiles[0].GetListOfKeys():
    histos = []
    print key.GetName()
    if(key.GetName() == "ecalTimingAna"): 
        dirFile = inputFiles[0].Get(key.GetName())
        for subKey in dirFile.GetListOfKeys():
            print subKey.GetName()
            histo = dirFile.Get(subKey.GetName())
            histos = []
            if ("TH2" in histo.ClassName()):
                print subKey.GetName()
                histos.append(histo)
            canvs.append(utils.Make2DPlots(subKey.GetName(),outdir,histos, zmin = 1.2, zmax = 2.5))

outF = r.TFile("histEcalTimingStuides.root","RECREATE")
outF.cd()
#for canv in canvs: canv.Write()
#outF.Close()
