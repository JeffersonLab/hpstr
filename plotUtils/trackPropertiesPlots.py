import ROOT as r
import os
import utilities as utils

utils.SetStyle()

path = "/Users/pbutti/HPS_plots/Kalman/TrackingAndVertexing_2019_MCBeam_Data/VTX_FIX"

#START WITH GBL FILE OTHERWISE IT BREAKS
inFileList = [
    #    "hps_010031_anaKalVtxTuple_cfg_total_KF.root",
    #    "hps_010031_anaKalVtxTuple_cfg_total_GBL.root"
    "2019_tridents_from_LCIO_VtxFix_histos_KF.root",
    "2019_tridents_from_LCIO_VtxFix_histos_GBL.root"
    ]

colors = [r.kRed, r.kBlue, r.kBlack, r.kGreen+2, r.kOrange-2]
inputFiles = []
legends = ["hps 10031 KF", "hps 10031 GBL"]
outdir = "./TrackingPlots_2019_MC/"

if not os.path.exists(outdir):
    os.makedirs(outdir)

r.gROOT.SetBatch(1)

for ifile in inFileList:
    inf = r.TFile(path+"/"+ifile)
    inputFiles.append(inf)
    print inf
    pass

sel = ["vtxana_kf_vtxSelection", "vtxana_gbl_vtxSelection"]
#sel = ["vtxana_kf_L1CutPos","vtxana_gbl_L1CutPos"]
#sel = ["vtxana_kf_tightUncVChi2","vtxana_gbl_tightUncVChi2"]

particles = ["ele", "pos"]
variables = ["nHits_2d", "nShared", "p", "time", "chi2", "d0"]
globalVars = ["Psum", "n_tracks"]

for var in variables:
    for particle in particles:
        histos = []
        for i_f in xrange(len(inputFiles)):
            print inputFiles[i_f], sel[i_f]+"/"+sel[i_f]+"_"+particle+"_"+var+"_h"
            histo = inputFiles[i_f].Get(sel[i_f]+"/"+sel[i_f]+"_"+particle+"_"+var+"_h")
            histos.append(histo)
            pass

        xtitle = histos[0].GetXaxis().GetTitle()
        ytitle = histos[0].GetXaxis().GetTitle()
        xmax = histos[0].GetXaxis().GetXmax()
        xmin = histos[0].GetXaxis().GetXmin()
        y_max = 0.08
        Rmin = 0.5
        Rmax = 1.5
        Norm = True
        print histos, legends
        if "chi2" in histos[0].GetName():
            xmax = 30
        if "Shared" in histos[0].GetName():
            y_max = 1
        if ("nHits_2d" in histos[0].GetName()):
            y_max = histos[1].GetMaximum()*1.3
            Rmin = 0.5
            Rmax = 2
            xmin = 6
            Norm = False
        basename = sel[0].split("_")[-1]
        utils.MakePlot(basename+"_"+particle+"_"+var+"_h", outdir, histos, legends, ".pdf", ymin=0, ymax=y_max, RatioType="Sequential", RatioMin=Rmin, RatioMax=Rmax, drawOptions="pe", Normalise=Norm, Xmin=xmin, Xmax=xmax)
        pass
    pass
pass

#Cutflow
histos = []
for i_f in xrange(len(inputFiles)):
    print sel[i_f]+"/"+sel[i_f]+"cutflow"
    histos.append(inputFiles[i_f].Get(sel[i_f]+"/"+sel[i_f]+"_cutflow"))
    pass
basename = sel[0].split("_")[-1]
utils.MakePlot(basename+"_cutflow_h", outdir, histos, legends, ".pdf", ymin=0.01, ymax=histos[0].GetMaximum()*2, RatioType="Sequential", RatioMin=0.1, RatioMax=4, drawOptions="pe", LogY=False)

#Psum
histos = []
for i_f in xrange(len(inputFiles)):
    print sel[i_f]+"/"+sel[i_f]+"Psum_h"
    histos.append(inputFiles[i_f].Get(sel[i_f]+"/"+sel[i_f]+"_Psum_h"))
    pass
basename = sel[0].split("_")[-1]
utils.MakePlot(basename+"_Psum_h", outdir, histos, legends, ".pdf", ymin=0, ymax=0.02, RatioType="Sequential", RatioMin=0.1, RatioMax=4, drawOptions="pe", Normalise=True, Xmin=1, Xmax=5.5)

#n_tracks
histos = []
for i_f in xrange(len(inputFiles)):
    print sel[i_f]+"/"+sel[i_f]+"n_tracks_h"
    histos.append(inputFiles[i_f].Get(sel[i_f]+"/"+sel[i_f]+"_n_tracks_h"))
    pass
basename = sel[0].split("_")[-1]
utils.MakePlot(basename+"_n_tracks_h", outdir, histos, legends, ".pdf", ymin=0, ymax=histos[0].GetMaximum()*2, RatioType="Sequential", RatioMin=0.1, RatioMax=4, drawOptions="pe", Normalise=False)


variables = ["vtx_InvM", "vtx_X_svt", "vtx_Y", "vtx_Z_svt", "vtx_chi2",
             "vtx_p"]
for var in variables:
    histos = []
    for i_f in xrange(len(inputFiles)):
        print inputFiles[i_f], sel[i_f]+"/"+sel[i_f]+"_"+var+"_h"
        histo = inputFiles[i_f].Get(sel[i_f]+"/"+sel[i_f]+"_"+var+"_h")
        histos.append(histo)
        pass
    xtitle = histos[0].GetXaxis().GetTitle()
    ytitle = histos[0].GetXaxis().GetTitle()
    xmax = histos[0].GetXaxis().GetXmax()
    xmin = histos[0].GetXaxis().GetXmin()
    y_max = 0.08
    Rmin = 0.5
    Rmax = 1.5
    Norm = True
    if "vtx_Y" in histos[0].GetName():
        y_max = 0.12
    if "vtx_Z_svt" in histos[0].GetName():
        xmin = -30
        xmax = 10
        y_max = 0.15
    print histos, legends
    basename = sel[0].split("_")[-1]
    utils.MakePlot(basename+"_"+var+"_h", outdir, histos, legends, ".pdf", ymin=0, ymax=y_max, RatioType="Sequential", RatioMin=Rmin, RatioMax=Rmax, drawOptions="pe", Normalise=Norm, Xmin=xmin, Xmax=xmax)
    pass
