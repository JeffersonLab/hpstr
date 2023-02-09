import ROOT as r
import os
import utilities as utils


utils.SetStyle()

#7 April
#path = "/Users/pbutti/HPS_plots/Kalman/TrackingAndVertexing_2019_MCBeam_Data/VTX_FIX/projections/Projections_ForAnalysisWorkshop/"
#17 April
path = "/Users/pbutti/HPS_plots/Kalman/TrackingAndVertexing_2019_MCBeam_Data/VTX_FIX/Projections_ForAnalysisWorkshop/"

#START WITH GBL FILE OTHERWISE IT BREAKS
inFileList = [
    "2019_tridents_from_LCIO_VtxFix_histos_GBL_projections.root",  # mc no v0 skim
    #"2019_tridents_from_LCIO_VtxFix_histos_KF_projections.root", #mc no v0 skim KF tracks
    #"hps_010031_anaKalVtxTuple_V0Align_iter0_projections.root", #v0 skim
    #"hps_010031_anaKalVtxTuple_cfg_total_KF_projections.root",
    #"hps_010031_anaKalVtxTuple_V0Align_iter3_projections.root"
    #"hps_010031_anaKalVtxTuple_cfg_total_GBL_projections.root", #nominal original processing all 10031
    "hps_010031_anaKalVtxTuple_noV0Skims_V0Align_iter0_projections.root",  # iter0 no v0 skims
    "hps_010031_anaKalVtxTuple_V0skimsLoose_V0Align_iter3_projections.root",  # iter3 v0 skims loose
    #"hps_010031_anaKalVtxTuple_V0Align_iter3_projections.root" #iter3 v0 skims (cluster on track)
    ]  # v0 skim

colors = [r.kBlack, r.kRed, r.kBlue, r.kGreen+2, r.kOrange-2]

inputFiles = []
#legends     = ["triTrig+beam GBL","triTrig+beam KF","hps 10031 GBL","hps 10031 KF"]
#legends     = ["triTrig+beam 2019","10031 nominal geo","10031 L1L2Align iter3"]
legends = ["tri+beam 2019 MC", "nominal Processing", "10031 L1L2it3"]
outdir = "./ProjectionPlots_2019_align_Data_MC_noV0Skims/"

if not os.path.exists(outdir):
    os.makedirs(outdir)

r.gROOT.SetBatch(1)

for ifile in inFileList:
    inf = r.TFile(path+"/"+ifile)
    inputFiles.append(inf)
    print inf
    pass

for key in inputFiles[0].GetListOfKeys():
    histos = []
    #print key.GetName()
    basekey = (key.GetName()).split("gbl")[0]
    lastkey = (key.GetName()).split("gbl")[-1]

    c = r.TCanvas()
    canvs = []
    for i_f in range(0, len(inFileList)):
        if "KF" in inputFiles[i_f].GetName():
            keyname = basekey+"kf"+lastkey
        elif "GBL" in inputFiles[i_f].GetName():
            keyname = key.GetName()
        else:
            keyname = key.GetName()
            #print keyname
        histo = inputFiles[i_f].Get(keyname)
        #print histo, histo.ClassName()
        if ("TH2" in histo.ClassName()):
            continue
        histos.append(histo)
        histos[i_f].SetMarkerColor(colors[i_f])
        histos[i_f].SetLineColor(colors[i_f])
    if (len(histos) == len(legends)):
        xAxisT = ""
        yAxisT = ""
        yaxmin = 0
        yaxmax = 6

        if "vtx_InvM_vtx_svt_z_hh_sigma" in keyname:
            xAxisT = "Vtx M [GeV]"
            yAxisT = "#sigma_z Vtx [mm]"
            yaxmin = 0
            yaxmax = 6
        if "vtx_InvM_vtx_svt_z_hh_mu" in keyname:
            xAxisT = "Vtx M [GeV]"
            yAxisT = "#mu_z Vtx [mm]"
            yaxmin = -20
            yaxmax = -5
        if "vtx_p_svt_z_hh_mu" in keyname:
            xAxisT = "Vtx P [GeV]"
            yAxisT = "#mu_z Vtx [mm]"
            yaxmin = -20
            yaxmax = -5
        if "vtx_p_svt_z_hh_sigma" in keyname:
            xAxisT = "Vtx P [GeV]"
            yAxisT = "#sigma_z Vtx [mm]"
            yaxmin = 0
            yaxmax = 8
        print "xAxisT", xAxisT
        canvs.append(utils.MakePlot(key.GetName(), outdir, histos, legends, ".pdf", xtitle=xAxisT, ytitle=yAxisT, ymin=yaxmin, ymax=yaxmax, LogY=False, RatioType="Sequential", RatioMin=0.25, RatioMax=3.5, drawOptions="pe"))

    pass
