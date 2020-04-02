import ROOT as r
import os
import utilities as utils


utils.SetStyle()

path = "/gpfs/slac/atlas/fs1/d/ssevova/hps/hpstr/histos"
inFileList = [
    "tritrig/tritrig_anaVtx.root",
    "simp/simp_100_60_33p3_recon_ctau10mm_anaVtx.root",
    "simp/simp_130_78_43p3_recon_ctau10mm_anaVtx.root",
    "simp/simp_100_40_30_recon_ctau10mm_anaVtx.root",
    "simp/simp_50_30_16p7_recon_ctau10mm_anaVtx.root"]


colors = [r.kBlack, r.kRed, r.kBlue, r.kGreen+2, r.kOrange-2]

inputFiles = []
legends     = ["tritrig",
               "simp 100,60,33.3",
               "simp 130,78,43.3",
               "simp 100,40,30",
               "simp 50,30,16.7"]
outdir     = "tritrig_simp_recon_ctau10mm_anaVtxPresel_simpTightL1L1NoSharedL0_norm"#_TruthRadMatch_norm"

if not os.path.exists(outdir):
    os.makedirs(outdir)

r.gROOT.SetBatch(1)

for ifile in inFileList:
    inf = r.TFile(path+"/"+ifile)
    inputFiles.append(inf)
    pass

canvs = []
for key in inputFiles[0].Get('vtxana_simpTightL1L1NoSharedL0').GetListOfKeys():
    histos = []
    print key.GetName()
    c = r.TCanvas()
    for i_f in range(0,len(inputFiles)):
        histos.append(inputFiles[i_f].Get('vtxana_simpTightL1L1NoSharedL0').Get(key.GetName()))
        histos[i_f].SetMarkerColor(colors[i_f])
        histos[i_f].SetLineColor(colors[i_f])
        pass
    canvs.append(utils.MakePlot(key.GetName(),outdir,histos,legends,".png",LogY=False,RatioType="Sequential",Normalise=True))
    canvs.append(utils.MakePlot(key.GetName()+"_log",outdir,histos,legends,".png",LogY=True,RatioType="Sequential",Normalise=True))
    pass

outF = r.TFile("tritrig_simp_recon_ctau10mm_anaVtxPresel_simpTightL1L1NoSharedL0_norm.root","RECREATE")
outF.cd()
for canv in canvs: canv.Write()
outF.Close()
