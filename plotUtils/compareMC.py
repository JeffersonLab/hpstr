import ROOT as r
import os
import utilities as utils


utils.SetStyle()

path = "/gpfs/slac/atlas/fs1/d/ssevova/hps/hpstr/histos"
inFileList = [
    "tritrig/tritrig_anaVtx.root",
    "wab/wab_anaVtx.root",
    "simp/simp_100_60_33p3_recon_ctau10mm_anaVtx.root",
    "simp/simp_130_78_43p3_recon_ctau10mm_anaVtx.root"]
 #   "simp/simp_100_40_30_recon_ctau10mm_anaVtx.root",
 #   "simp/simp_50_30_16p7_recon_ctau10mm_anaVtx.root"]


colors = [r.kBlack, r.kRed, r.kBlue, r.kGreen+2, r.kOrange-2]

inputFiles = []
legends     = ["tritrig",
               "wab",
               "simp_100_60_33p3",
               "simp_130_78_43p3"]
#               "simp_100_40_30",
#               "simp_50_30_16p7"]
outdir     = "tritrig_wab_simp_beam_recon_ctau10mm_anaVtxPresel_norm"

if not os.path.exists(outdir):
    os.makedirs(outdir)

r.gROOT.SetBatch(1)

for ifile in inFileList:
    inf = r.TFile(path+"/"+ifile)
    inputFiles.append(inf)
    pass

canvs = []

for key in inputFiles[0].Get('vtxana_vtxSelection').GetListOfKeys():
    histos = []
    histos2D = []
    xtitle = []
    ytitle = []
    name   = []

    print key.GetName()
    c = r.TCanvas()
    for i_f in range(0,len(inputFiles)):

        if '_hh' in key.GetName():
            histos2D.append(inputFiles[i_f].Get('vtxana_vtxSelection').Get(key.GetName()))
            xtitle.append(histos2D[i_f].GetXaxis().GetTitle())
            ytitle.append(histos2D[i_f].GetYaxis().GetTitle())
            name.append(key.GetName()+'_'+legends[i_f])
        
        histos.append(inputFiles[i_f].Get('vtxana_vtxSelection').Get(key.GetName()))
        #histos[i_f].SetMarkerColor(colors[i_f])
        #histos[i_f].SetLineColor(colors[i_f])
        pass

    canvs.append(utils.Make2DPlots(name,outdir,histos2D,xtitle,ytitle))
    canvs.append(utils.MakePlot(key.GetName(),outdir,histos,legends,".png",LogY=False,RatioType="Sequential",Normalise=True))
    canvs.append(utils.MakePlot(key.GetName()+"_log",outdir,histos,legends,".png",LogY=True,RatioType="Sequential",Normalise=True))
    pass

outF = r.TFile("tritrig_wab_simp_beam_recon_ctau10mm_anaVtxPresel_norm.root","RECREATE")
outF.cd()
for canv in canvs: canv.Write()
outF.Close()
