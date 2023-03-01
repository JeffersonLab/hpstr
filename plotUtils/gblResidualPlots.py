import ROOT as r
import os
import utilities as utils


utils.SetStyle()

path = "/Users/pbutti/HPS_plots/Kalman/UnbiasedResiduals_2019/GBL/"

#START WITH GBL FILE OTHERWISE IT BREAKS
inFileList = [

              #"AlignTracks_MPII_V0Align_iter0/AlignMonitoring_10031_V0Align_iter0.root",   #NOMINAL GEO
              #"AlignTracks_TriTrig_MPII_TRAL1_iter0/AlignMonitoring_triTrigs_nobeam.root",    #MC TRANSLATION 100um
              #"AlignTracks_MPII_V0Align_iter3/AlignMonitoring_10031.root"      # ALIGNED GEO

              # OPENING ANGLE CHECKS - DATA
              #"OpeningAngle_1m5_bottom_closing/AlignTracks_iter0/AlignMonitoring_10031_m1_5OA.root",
              #"OpeningAngle_1m5_bottom_closing/AlignTracks_iter0_check/AlignMonitoring_OPAngleBOTm15.root"

              # OPENING ANGLE CHECKS - MC
              "OpeningAngle_1m5_bottom_closing/triTrig_2019_MC/AlignTracks_iter0_1m5OpeningBOT/AlignMonitoring_10031.root"
              ]


colors = [r.kBlack, r.kRed, r.kBlue, r.kGreen+2, r.kOrange-2, r.kTeal-5]

inputFiles = []
legends = ["Nominal Geometry", "Opening Angle bottom -1.5mrad", "Opening Angle bottom Check"]
#legends     = ["+100um L1At tu"]
outdir = "./UnbiasedResiduals_2019_OpeningAngleBottom_m1_5/"

if not os.path.exists(outdir):
    os.makedirs(outdir)

r.gROOT.SetBatch(1)

for ifile in inFileList:
    inf = r.TFile(path+ifile)
    inputFiles.append(inf)
    print inf
    pass

tracks = ""

lyrs = range(1, 8)
sides = ["axial", "stereo"]
poss = ["hole", "slot"]
vols = ["t", "b"]

modules = []

for ily in lyrs:
    for vol in vols:
        for side in sides:
            for pos in poss:
                module = "L"+str(ily)+vol+"_halfmodule_"+side
                if ily > 4:
                    module += "_"+pos
                module += "_sensor0"
                modules.append(module)
                pass
            pass
        pass
    pass

print modules

'''
for module in modules:
    histos  = []
    histos.append(inputFiles[0].Get("residual_after_GBL_module_"+module))
    print "residual_after_GBL_module_"+module
    print "bresidual_GBL_module_"+module
    histos.append(inputFiles[0].Get("bresidual_GBL_module_"+module))
    #print histos
    #utils.MakePlot("bres_"+module,outdir,histos,legends,".pdf",xtitle="biased residual [mm]",ymax=0.2,Normalise=True)
    pass
'''


#Unbias residuals

for module in modules:
    histos = []
    fits = []
    histos_b = []
    for i_f in xrange(len(inputFiles)):
        histo_u = inputFiles[i_f].Get("uresidual_GBL_module_"+module)
        histos.append(histo_u)

        #(hNewMean,hNewRMS) = utils.findMeanRMSUsingFWHM(histo_u)

        fitFunc = utils.singleGausRestricted(histo_u)
        mu = fitFunc.GetParameter(1)
        sigma = fitFunc.GetParameter(2)
        fits.append(str(mu)+" "+str(sigma))
        #print "MEAN/RMS (FWHM)", hNewMean, hNewRMS
        histos_b.append(inputFiles[i_f].Get("bresidual_GBL_module_"+module))
        pass

    #print "uresidual_GBL_module_"+module
    can = utils.MakePlot("ures_"+module, outdir, histos, legends, ".pdf", xtitle="unbiased residual "+module+" [mm]", ytitle="hits-on-track", ymax=0.06, Normalise=True, additionalText=fits)

    utils.MakePlot("bres_"+module, outdir, histos_b, legends, ".pdf", xtitle="biased residual "+module+" [mm]", ytitle="hits-on-track", ymax=0.15, Normalise=True)
