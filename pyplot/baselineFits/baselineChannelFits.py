import ROOT as r
import time
from copy import deepcopy
import os.path
from os import path
import numpy as np
import ModuleMapper as mmap
import argparse
import csv


parser=argparse.ArgumentParser(description="Configuration options for baseline fit")

parser.add_argument("-i", type=str, dest="inFilename", help="Input SvtBlFitHistoProcessor output root file",default="")
parser.add_argument("-o", type=str, dest="outFilename", help="output root file",default="baselineFitTool.root")
parser.add_argument("-b", type=str, dest="onlineBaselines", help="online baseline fits root file",default="")
parser.add_argument("-csv", type=str, dest="csvOut", help="File name for CSV output file",default="offline_baseline_fits.csv")
parser.add_argument("-s", "--hybrids", type=str, dest="hybrids",nargs="+",
        help="L<#><T/B>_<axial/stereo>_<ele/pos>", default="L")
parser.add_argument("-r", type=str, dest="runNumber", help="Run Number",default="")
parser.add_argument("-g", type=str, dest="show_graphs", help="If == show, print all graphs associate with specified channels", default="")
options = parser.parse_args()

r.gROOT.SetBatch(r.kTRUE)
r.gStyle.SetOptStat(1111)
######################################################################################################

def getKeysFromFile(inFile,cType="", attr1=[""], attr2=""):
    inFile.cd()
    histo_keys=[]
    for hybrid in inFile.GetListOfKeys():
        kname = hybrid.GetName()
        classType = hybrid.GetClassName()
        for string in attr1:
            if classType.find(cType) != -1 and kname.find(string) != -1 and kname.find(attr2) != -1:
                histo_keys.append(kname)
    return histo_keys
              
def buildTGraph(name,title, n_points, x, y,color):
    g = r.TGraph(n_points,np.array(x, dtype = float), np.array(y, dtype = float))
    g.SetName("%s"%(name))
    g.SetTitle("%s"%(title))
    g.SetLineColor(color)
    return g

def getPlotFromTFile(inFile, plotname):
    inFile.cd()
    plot = inFile.Get(plotname)
    if plot:
        print(plotname,'Found in file file',inFile)
    else:
        print(plotname,'NOT FOUND in file',inFile)
    return plot


######################################################################################################
outFile = r.TFile(options.outFilename,"RECREATE")
csvOutFile = options.csvOut
if(path.exists(csvOutFile)):
    csvOutFile = os.path.splitext(csvOutFile)[0] + "_" + time.strftime("%H%M%S") + ".csv"

onlineFile = r.TFile(options.onlineBaselines, "READ")
inFile = r.TFile(options.inFilename, "READ")
hybrids = options.hybrids
run = options.runNumber


#Get SvtBl2D histogram hybrids from input file
hybridsFromFile = getKeysFromFile(inFile,"TH2", hybrids,"")
print(hybridsFromFile)
outFile = r.TFile(options.outFilename,"RECREATE")
loadOnlineBaselines = False

for hybrid in hybridsFromFile:
    #Get 2d histogram for hybrid
    inFile.cd()
    hybrid_hh = getPlotFromTFile(inFile, hybrid)

    #Get offline baseline fit values from SvtBlFitProcessor output
    #Fit values are stored in Flat Tuple

    #Offline baseline Fit Information
    channel, mean, sigma, norm, chi2, ndf, fitlow, fithigh, RMS = ([] for i in range(9))

    #Offline Channel Flags
    minStatsFailure, noisy, lowdaq, dead, TFRerror = ([] for i in range(5))
    #Online baseline fit information
    onlineChannel=[]
    onlineMean=[]
    onlineSigma=[]

    myTree = inFile.gaus_fit
    for fitData in myTree:
        #hybridFitKey = str(fitData.SvtAna2DHisto_key)
        hybridFitKey = str(fitData.halfmodule_hh)
        if hybrid == hybridFitKey:
            channel.append(fitData.channel)
            mean.append(round(fitData.BlFitMean,3))
            sigma.append(round(fitData.BlFitSigma,3))
            norm.append(round(fitData.BlFitNorm,3))
            fitlow.append(fitData.BlFitRangeLower)
            fithigh.append(fitData.BlFitRangeUpper)
            chi2.append(fitData.BlFitChi2)
            ndf.append(fitData.BlFitNdf)
            minStatsFailure.append(fitData.minStatsFailure)
            lowdaq.append(fitData.lowdaq)
            dead.append(fitData.dead)
            TFRerror.append(fitData.TFitResultError)

    #Get offline baseline RMS value, used to determine if channel is "dead" based on config param
    for cc in range(len(channel)): 
        yproj_h = hybrid_hh.ProjectionY('%s_ch%i_h'%(hybrid,channel[cc]),int(channel[cc]+1),int(channel[cc]+1),"e")
        rms = yproj_h.GetRMS()
        RMS.append(round(rms,3))

    #Remove extra phrases in input plots to isolate Hybrid name
    #hybrid = hybrid.replace('raw_hits_','').replace('_SvtHybrids0_hh','')
    hybrid = hybrid.replace('raw_hits_','').replace('baseline0_','').replace('_hh','')
    hwtag = mmap.str_to_hw(hybrid)
    feb = hwtag[0:2]
    hyb = hwtag[2:]
    print("Hybrid: ", hybrid)
    print("hwtag: ", hwtag)

    #Read online baseline fit values from root file
    if(options.onlineBaselines) != "":
        loadOnlineBaselines = True
        graphName = "baseline/baseline_"+hwtag + "_ge"
        print('retrieving online baseline fits from graph:',graphName)
        onlinePlot = getPlotFromTFile(onlineFile,graphName)
        x = (onlinePlot.GetX())
        onlineChannel = list(x)
        y =(onlinePlot.GetY())
        onlineMean = list(y)
        for c in onlineChannel:
            onlineSigma.append(onlinePlot.GetErrorY(c))

    #Plot baseline fit values overlaid on 2d histograms
    outFile.cd()
    canvas = r.TCanvas("%s_mean"%(hybrid), "c", 1800,800)
    canvas.cd()
    canvas.SetTicky()
    canvas.SetTickx()
    mean_gr_x = np.array(channel, dtype = float)
    mean_gr_y = np.array(mean, dtype=float)
    mean_gr = buildTGraph("%s_BlFitMean_%s"%(run,hybrid),"%s_BlFitMean_%s;Channel;ADC"%(run,hybrid),len(mean_gr_x),mean_gr_x,mean_gr_y,2)
    lowdaq_gr_x = np.array(channel, dtype = float)
    lowdaq_gr_y = np.array([3500.0 * x for x in lowdaq], dtype=float)
    lowdaq_gr = buildTGraph("lowdaqflag_%s"%(hybrid),"low-daq channels_%s;Channel;ADC"%(hybrid),len(lowdaq_gr_x),lowdaq_gr_x, lowdaq_gr_y,2)
    lowdaq_gr.SetMarkerStyle(8)
    lowdaq_gr.SetMarkerSize(1)
    lowdaq_gr.SetMarkerColor(3)

    hybrid_hh.GetYaxis().SetRangeUser(3000.0,7500.0)
    if hybrid_hh.GetName().find("L0") != -1 or hybrid_hh.GetName().find("L1") != -1:
        hybrid_hh.GetXaxis().SetRangeUser(0.0,512.0)
    hybrid_hh.Draw("colz")
    mean_gr.Draw("same")
    lowdaq_gr.Draw("psame")

    if loadOnlineBaselines == True:
        bl_gr_x = np.array(onlineChannel, dtype = float)
        bl_gr_y = np.array(onlineMean, dtype = float)
        bl_gr = buildTGraph("onlineBlFits_%s"%(hybrid),"Online Baseline Fits_%s;Channel;ADC"%(hybrid),len(bl_gr_x),bl_gr_x,bl_gr_y,1)
        bl_gr.Draw("same")


    legend = r.TLegend(0.1,0.75,0.28,0.9)
    legend.AddEntry(mean_gr,"offline baselines using hpstr processor","l")
    if loadOnlineBaselines == True:
        legend.AddEntry(bl_gr,"online baseline fits","l")
    legend.AddEntry(lowdaq_gr,"low-daq threshold","p")
    legend.Draw()
    canvas.Write()
    canvas.Close()
    
    if loadOnlineBaselines == True:
        #Plot Difference between online and offline baselines
        canvas = r.TCanvas("%s_diff"%(hybrid), "c", 1800,800)
        canvas.cd()
        canvas.SetTicky()
        canvas.SetTickx()
        diff_ch = []
        diff_mean = []
        diff_bl_mean = []
        
        for i in range(len(channel)):
            if mean[i] > 0:
                diff_ch.append(channel[i])
                diff_mean.append(mean[i])
                diff_bl_mean.append(onlineMean[i])

        diff_gr_x = np.array(diff_ch, dtype = float)
        diff_gr_y = np.array([x - y for x,y in zip(diff_mean,diff_bl_mean)], dtype=float)
        diff_gr = buildTGraph("FitDiff_%s"%(hybrid),"(Offline - Online) BlFit Mean_%s;Channel;ADC Difference"%(hybrid),len(diff_gr_x),diff_gr_x,diff_gr_y,1)

        diff_gr.GetYaxis().SetRangeUser(-400.0,400.0)
        if hybrid.find("L0") != -1 or hybrid.find("L1") != -1:
            diff_gr.GetXaxis().SetRangeUser(0.0,512.0)
        else:
            diff_gr.GetXaxis().SetRangeUser(0.0,640.0)

        lowdaq_gr_x = np.array(channel, dtype = float)
        lowdaq_gr_y = np.array([-1000.0 + 600.0* x for x in lowdaq], dtype=float)
        lowdaq_gr = buildTGraph("lowdaqflag_%s"%(hybrid),"low-daq channels_%s;Channel;ADC"%(hybrid),len(lowdaq_gr_x),lowdaq_gr_x, lowdaq_gr_y,2)

        diff_gr.Draw()

        lowdaq_gr.SetMarkerStyle(8)
        lowdaq_gr.SetMarkerSize(1)
        lowdaq_gr.SetMarkerColor(3)
        lowdaq_gr.Draw("psame")

        canvas.Write()
        canvas.Close()

    ############################################################################################################################
    #1D Histogram of channel fit sigma
    sigma_h = r.TH1F("Fit_Sigma_%s"%(hybrid),"Fit_Sigma_Distribution_%s;sigma;events"%(hybrid),len(sigma),0.,max((sigma)))
    for i in range(len(sigma)):
        sigma_h.Fill(sigma[i],1.)
    sigma_h.Write()

    #TGraph of channel fit sigma
    sigma_g = buildTGraph("Fit_Sigma_%s_per_Channel"%(hybrid),"Fit_Sigma_per_Channel_%s;sigma;channel"%(hybrid),len(channel),np.array(channel, dtype = float),np.array(sigma, dtype=float),1)
    sigma_g.Draw()
    sigma_g.Write()


    #Plot lowdaq
    ld_gr_y = np.array(lowdaq, dtype= float)
    ld_gr_x = np.array(channel, dtype=float)
    ld_gr = buildTGraph("lowdaq_flags_%s"%(hybrid),"Low_Daq_Threshold_%s;Channel;(1 = lowdaq)"%(hybrid),len(ld_gr_x),ld_gr_x,ld_gr_y,1)
    ld_gr.Draw()
    ld_gr.Write()

    #Plot Chi2/Ndf
    #Chi2Ndf = [i / j for i, j in zip(chi2,ndf)]
    #chi2_gr_y = np.array(Chi2Ndf, dtype= float)
    #chi2_gr_x = np.array(channel, dtype=float)
    #chi2_gr = buildTGraph("BlFitChi2_%s"%(hybrid),"BlFit_Chi2/Ndf_%s;Channel;Status"%(hybrid),len(chi2_gr_x),chi2_gr_x,chi2_gr_y,1)
    #chi2_gr.Draw()
    #chi2_gr.Write()

    #####################################################################################################
    #Channel Noise Plots
    RMS_gr = buildTGraph("%s_noise"%(hybrid),"%s_noise;Channel;Noise"%(hybrid),len(channel),np.array(channel,dtype=float),np.array(RMS,dtype=float),1)
    RMS_gr.Draw()
    RMS_gr.Write()
    
    ######################################################################################################
    #Show Channel Fits
    cfdir = outFile.mkdir("%s_channel_fits"%(hybrid))
    for cc in range(len(channel)): 
        print(hybrid," ", channel[cc])
        print("norm = ", norm[cc])
        print("mean = ", mean[cc])
        print("sigma = ", sigma[cc])
        canvas = r.TCanvas("%s_ch_%i_h"%(hybrid,channel[cc]), "c", 1800,800)
        canvas.cd()
        yproj_h = hybrid_hh.ProjectionY('%s_ch%i_h'%(hybrid,channel[cc]),int(channel[cc]+1),int(channel[cc]+1),"e")
        if yproj_h.GetEntries() == 0:
            continue
        func = r.TF1("m1","gaus",fitlow[cc],fithigh[cc])
        func.SetParameter(0,norm[cc])
        func.SetParameter(2,sigma[cc])
        func.SetParameter(1,mean[cc])

        cfdir.cd()
        yproj_h.SetTitle("%s_ch_%i_h"%(hybrid,channel[cc]))
        yproj_h.Draw()
        func.Draw("same")
        canvas.Write()
        canvas.Close()

#outFile.Write()


##########################################################################################################
    ###Show Channel Graphs
    showgraphs = True
    if(showgraphs == True):
        cgdir = outFile.mkdir("%s_channel_graphs"%(hybrid))
        cgdir.cd()
        bw=hybrid_hh.GetXaxis().GetBinWidth(1)
        #if options.show_graphs == "show":
        myTree = inFile.gaus_fit
        print("TREE",myTree)
        for fitData in myTree:
           SvtAna2DHisto_key = str(fitData.halfmodule_hh)
           #if str(fitData.halfmodule_hh).find(hybrid) != -1:
           if hybrid == SvtAna2DHisto_key.replace('raw_hits_','').replace('baseline0_','').replace('_hh',''):
               cc=(fitData.channel)

               mean_gr = buildTGraph("iterMean_%s_ch_%i"%(hybrid,cc),"iterMean_vs_Position_%s_ch_%i;FitRangeEnd;mean"%(hybrid,cc),len(fitData.iterFitRangeEnd),np.array(fitData.iterFitRangeEnd, dtype = float) ,np.array(fitData.iterMean, dtype = float),1)

               chi2_gr = buildTGraph("iterChi2_%s_ch_%i"%(hybrid,cc),"iterChi2/NDF_vs_Position_%s_ch_%i;FitRangeEnd;chi2"%(hybrid,cc),len(fitData.iterFitRangeEnd),np.array(fitData.iterFitRangeEnd, dtype = float) ,np.array(fitData.iterChi2NDF, dtype = float),1)

               chi2_2Der_gr = buildTGraph("iterFit_chi2/NDF_2Der_%s_ch_%i"%(hybrid,cc),"iterChi2_2Der_%s_ch_%i;FitRangeEnd;chi2_2ndDeriv"%(hybrid,cc),len(fitData.iterChi2NDF_derRange),np.array(fitData.iterChi2NDF_derRange, dtype = float) ,np.array(fitData.iterChi2NDF_2der, dtype = float),1)

               chi2_1Der_gr = buildTGraph("iterFit_chi2/NDF_1Der_%s_ch_%i"%(hybrid,cc),"iterChi2_1Der_%s_ch_%i;FitRangeEnd;chi2_1ndDeriv"%(hybrid,cc),len(fitData.iterChi2NDF_derRange),np.array(fitData.iterChi2NDF_derRange, dtype = float) ,np.array(fitData.iterChi2NDF_1der, dtype = float),1)

               #temp=fitData.iterChi2NDF
               #ratio = [i / j for i,j in zip(fitData.iterChi2NDF_2der,temp[3:])]

               #ratio_gr = buildTGraph("ratio_%s_ch_%i"%(hybrid,cc),"ratio_2Der/Chi2_%s_ch_%i;FitRangeEnd;chi2NDF_2der/chi2NDF"%(hybrid,cc),len(fitData.iterChi2NDF_derRange),np.array(fitData.iterChi2NDF_derRange, dtype = float) ,np.array(ratio, dtype = float),1)

               mean_gr.Draw()
               chi2_gr.Draw()
               chi2_1Der_gr.Draw()
               chi2_2Der_gr.Draw()
               #ratio_gr.Draw()

               mean_gr.Write()
               chi2_gr.Write()
               chi2_1Der_gr.Write()
               chi2_2Der_gr.Write()
               #ratio_gr.Write()
outFile.Write()
#####
