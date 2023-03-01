import ROOT as r
import os
import utilities as utils

import argparse

parser = argparse.ArgumentParser(description="The baseConfig options for compareKalGBL. ")

parser.add_argument('--debug', '-D', action="count", dest="debug", help="Increase debug level.", default=0)

#parser.add_option("-i", "--inFile", type="string", dest="inFilename",
#                  help="Input filename.", metavar="inFilename", default="")
parser.add_argument("-i", "--inFileBase", type=str, dest="inFileBase", action='store',
                    help="input file base, use when files are in the same directory and only differ by kf/gbl.root",  default=None)
parser.add_argument("-k", "--kalFile", type=str, dest="kalFile", action='store',
                    help="kal file",  default=None)
parser.add_argument("-g", "--gblFile", type=str, dest="gblFile", action='store',
                    help="gbl file",  default=None)
parser.add_argument("--doMC", dest="doMC", action='store_true',
                    help="Make MC folder plots", default=False)
parser.add_argument("--doSimp", dest="doSimp", action='store_true',
                    help="Make Simp vtxana_kf_vtxSelection plots", default=False)
parser.add_argument("--dir", dest="dir_suffix", action='store', type=str,
                    help="Add something to top level directory name", default=None)
parser.add_argument("--samples", dest="samples", action='store', type=str,
                    help="samples for legend", default="simp_100_60_33p3")


options = parser.parse_args()
ifb = options.inFileBase
fkal = options.kalFile
fgbl = options.gblFile
doMC = options.doMC
doSimp = options.doSimp
dir_suffix = options.dir_suffix
print(ifb)

inFileList = []

if ifb:
    inFileList.append(ifb+"_GBL.root")
    inFileList.append(ifb+"_KF.root")
elif kalFile and gblFile:
    inFileList.append(fgbl)
    inFileList.append(fkal)
else:
    path = "/gpfs/slac/atlas/fs1/d/ssevova/hps/hpstr/histos"
    inFileList.append(path+"/simp/gbl/simp_100_60_33p3_recon_ctau10mm_anaGBLVtx.root")
    inFileList.append(path+"/simp/kftrackstight/latest/simp_100_60_33p3_recon_ctau10mm_anaKalVtx.root")

utils.SetStyle()

#"tritrig/tritrig_anaVtx.root",
#"wab/wab_anaVtx.root",
#"simp/simp_100_60_33p3_recon_ctau10mm_anaVtx.root",
#"simp/simp_130_78_43p3_recon_ctau10mm_anaVtx.root",
#"simp/kftracks/simp_130_78_43p3_recon_ctau10mm_anaVtx.root"
#"simp/simp_100_40_30_recon_ctau10mm_anaVtx.root",
#"simp/simp_50_30_16p7_recon_ctau10mm_anaVtx.root"


colors = [r.kBlack, r.kRed, r.kBlue, r.kGreen+2, r.kOrange-2]

legends = ["gbl_tritrig", "kf_tritrig"]
#"wab",
#"gbl_simp_100_60_33p3",
#"kf_simp_100_60_33p3"]
#               "simp_100_40_30",
#               "simp_50_30_16p7"]

selection = ["vtxSelection"]

if doSimp:
    selection.append("simpTight")
    selection.append("simpTightL1L1")

if doMC:
    mcselection = []
    print(selection)
    for sel in selection:
        mcselection.append("mc_"+sel)
    selection.extend(mcselection)

outbase = "plots"
if dir_suffix:
    outbase = outbase+"_" + dir_suffix
if not os.path.exists(outbase):
    os.makedirs(outbase)

basedir = os.getcwd()

print("Making plots for ", selection)

algs = ["gbl", "kf"]

for sel in selection:
    print("plotting selection " + sel)
    outdir = outbase+"/KF_vs_GBL_simp_100_60_33p3_" + sel

#selection  = ["vtxana_gbl_vtxSelection","vtxana_kf_vtxSelection"]
    os.chdir(basedir)
    if not os.path.exists(outdir):
        print("Making Directory " + outdir)
        os.makedirs(outdir)

    r.gROOT.SetBatch(1)
    inputFiles = []

    for ifile in inFileList:
        print("Loading file " + ifile)
        inf = r.TFile(ifile)
        inputFiles.append(inf)
        pass

    canvs = []
    print("vtxana_gbl_"+sel)
    for key in inputFiles[0].Get("vtxana_gbl_"+sel).GetListOfKeys():
        #hack because its picking up a tree
        print(key)
        if "tree" in key.GetName():
            continue
        print("My key " + key.GetName())
        histos = []
        histos2D = []
        xtitle = []
        ytitle = []
        name = []

        c = r.TCanvas()
        for i_f in range(0, len(inputFiles)):
            #print("i_f " + str(i_f))
            #print("Looking at file " + inputFiles[i_f].GetName())
            keyname = ''
            if 'KF' in inputFiles[i_f].GetName():
                keyname = (key.GetName()).replace("vtxana_gbl", "vtxana_kf")
            else:
                keyname = key.GetName()
            #print("My KeyName " + keyname)
            if '_hh' in key.GetName():
                histos2D.append(inputFiles[i_f].Get("vtxana_"+algs[i_f]+"_"+sel).Get(keyname))
                xtitle.append(histos2D[i_f].GetXaxis().GetTitle())
                if 'ele_z0' in key.GetName():
                    ytitle.append('trk z0 [mm]')
                else:
                    ytitle.append(histos2D[i_f].GetYaxis().GetTitle())

                name.append(key.GetName()+'_'+legends[i_f])

            histos.append(inputFiles[i_f].Get("vtxana_"+algs[i_f]+"_"+sel).Get(keyname))

            #print(inputFiles[i_f].GetName())
            #print("vtxana_"+algs[i_f]+"_"+sel)
            #print(keyname)
            #print(inputFiles[i_f].Get("vtxana_"+algs[i_f]+"_"+sel).ls())

        #histos[i_f].SetMarkerColor(colors[i_f])
        #histos[i_f].SetLineColor(colors[i_f])
            pass

        canvs += utils.Make2DPlots(name, outdir, histos2D, xtitle, ytitle)
        canvs.append(utils.MakePlot(key.GetName(), outdir, histos, legends, ".png", LogY=False, RatioType="Sequential", Normalise=False))
        canvs.append(utils.MakePlot(key.GetName()+"_log", outdir, histos, legends, ".png", LogY=True, RatioType="Sequential", Normalise=False))
        pass
    print(outdir)
    utils.makeHTML(outdir, 'SIMPs KF vs GBL ('+sel+')', selection)
    outF = r.TFile("KF_vs_GBL_simp_100_60_33p3_vtxPresel"+sel+".root", "RECREATE")
    outF.cd()
    for canv in canvs:
        if canv is not None:
            canv.Write()
    outF.Close()
    #inf.Close()
