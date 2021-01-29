import ROOT as r
import os
import utilities as utils


utils.SetStyle()

path = "/gpfs/slac/atlas/fs1/d/ssevova/hps/hpstr/histos"
inFileList = ["simp/gbl/simp_100_60_33p3_recon_ctau10mm_anaGBLVtx.root",
              "simp/kftrackstight/latest/simp_100_60_33p3_recon_ctau10mm_anaKalVtx.root"]
    #"tritrig/tritrig_anaVtx.root",
    #"wab/wab_anaVtx.root",
    #"simp/simp_100_60_33p3_recon_ctau10mm_anaVtx.root",
    #"simp/simp_130_78_43p3_recon_ctau10mm_anaVtx.root",
    #"simp/kftracks/simp_130_78_43p3_recon_ctau10mm_anaVtx.root"
    #"simp/simp_100_40_30_recon_ctau10mm_anaVtx.root",
    #"simp/simp_50_30_16p7_recon_ctau10mm_anaVtx.root"


colors = [r.kBlack, r.kRed, r.kBlue, r.kGreen+2, r.kOrange-2]

inputFiles = []
legends     = [#"tritrig",
               #"wab",
               "gbl_simp_100_60_33p3",
               "kf_simp_100_60_33p3"]
#               "simp_100_40_30",
#               "simp_50_30_16p7"]
outdir     = "KF_vs_GBL_simp_100_60_33p3_vtxPresel"
selection  = ["vtxana_gbl_vtxSelection","vtxana_kf_vtxSelection"]

if not os.path.exists(outdir):
    os.makedirs(outdir)

r.gROOT.SetBatch(1)

for ifile in inFileList:
    inf = r.TFile(path+"/"+ifile)
    inputFiles.append(inf)
    pass

canvs = []

for key in inputFiles[0].Get(selection[0]).GetListOfKeys():
    histos = []
    histos2D = []
    xtitle = []
    ytitle = []
    name   = []

    print(key.GetName())
    c = r.TCanvas()
    for i_f in range(0,len(inputFiles)):
        keyname = ''
        if '_kf_' in selection[i_f]:
            keyname = (key.GetName()).replace("vtxana_gbl_vtx","vtxana_kf_vtx")
        else:
            keyname = key.GetName()

        if '_hh' in key.GetName():
            histos2D.append(inputFiles[i_f].Get(selection[i_f]).Get(keyname))
            xtitle.append(histos2D[i_f].GetXaxis().GetTitle())
            if 'ele_z0' in key.GetName():
                ytitle.append('trk z0 [mm]')
            else:
                ytitle.append(histos2D[i_f].GetYaxis().GetTitle())

            name.append(key.GetName()+'_'+legends[i_f])
        
        histos.append(inputFiles[i_f].Get(selection[i_f]).Get(keyname))
        #histos[i_f].SetMarkerColor(colors[i_f])
        #histos[i_f].SetLineColor(colors[i_f])
        pass
    canvs+=utils.Make2DPlots(name,outdir,histos2D,xtitle,ytitle)
    canvs.append(utils.MakePlot(key.GetName(),outdir,histos,legends,".png",LogY=False,RatioType="Sequential",Normalise=False))
    canvs.append(utils.MakePlot(key.GetName()+"_log",outdir,histos,legends,".png",LogY=True,RatioType="Sequential",Normalise=False))
    pass

utils.makeHTML(outdir,'SIMPs KF vs GBL (vtxPresel)', selection)
outF = r.TFile("KF_vs_GBL_simp_100_60_33p3_vtxPresel.root","RECREATE")
outF.cd()
for canv in canvs: 
    if canv != None: canv.Write()
outF.Close()
