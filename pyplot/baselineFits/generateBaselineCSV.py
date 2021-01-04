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

def savePNG(canvas,directory,name):
        #canvas.Draw()

        canvas.SaveAs("%s/%s.png"%(directory,name))
        canvas.Close()

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
    print("Checking Hybrid",hybrid)
    #Get 2d histogram for hybrid
    inFile.cd()
    hybrid_hh = getPlotFromTFile(inFile, hybrid)

    #Get offline baseline fit values from SvtBlFitProcessor output
    #Fit values are stored in Flat Tuple

    #Offline baseline Fit Information
    channel, svt_id, mean, sigma, norm, chi2, ndf, fitlow, fithigh, RMS = ([] for i in range(10))

    #Offline Channel Flags
    minStatsFailure, noisy, lowdaq, dead, TFRerror = ([] for i in range(5))
    #Online baseline fit information
    onlineChannel=[]
    onlineMean=[]
    onlineSigma=[]

    print("Grabbing Ntuples from TTree")
    myTree = inFile.gaus_fit
    for fitData in myTree:
        hybridFitKey = str(fitData.SvtAna2DHisto_key)
        if hybrid == hybridFitKey+"_hh":
            channel.append(fitData.channel)
            mean.append(round(fitData.BlFitMean,3))
            sigma.append(round(fitData.BlFitSigma,3))
            norm.append(round(fitData.BlFitNorm,3))
            fitlow.append(fitData.BlFitRangeLower)
            fithigh.append(fitData.BlFitRangeUpper)
            chi2.append(fitData.BlFitChi2)
            ndf.append(fitData.BlFitNdf)
            minStatsFailure.append(fitData.minStatsFailure)
            noisy.append(fitData.noisy)
            lowdaq.append(fitData.lowdaq)
            dead.append(fitData.dead)
            TFRerror.append(fitData.TFitResultError)
            svt_id.append(fitData.svt_id)

    #Get offline baseline RMS value, used to determine if channel is "dead" based on config param
    for cc in range(len(channel)): 
        yproj_h = hybrid_hh.ProjectionY('%s_ch%i_h'%(hybrid,channel[cc]),int(channel[cc]+1),int(channel[cc]+1),"e")
        rms = yproj_h.GetRMS()
        RMS.append(round(rms,3))

    #Remove extra phrases in input plots to isolate Hybrid name
    hybrid = hybrid.replace('raw_hits_','').replace('_SvtHybrids0_hh','')
    hwtag = mmap.str_to_hw(hybrid)
    feb = hwtag[0:2]
    hyb = hwtag[2:]

    #Read online baseline fit values from root file
    if(options.onlineBaselines) != "":
        loadOnlineBaselines = True
        graphName = "baseline/baseline_"+hwtag + "_ge"
        print('retrieving online baseline fits from graph:',graphName)
        onlinePlot = getPlotFromTFile(onlineFile,graphName)
        print("successfully loaded online baseline")
        x = (onlinePlot.GetX())
        onlineChannel = list(x)
        y =(onlinePlot.GetY())
        onlineMean = list(y)
        for c in onlineChannel:
            onlineSigma.append(onlinePlot.GetErrorY(int(c)))

    
    #Write baselines to csv file
    csvMean, csvSigma, csvType, csvHybrid, csvFeb = ([] for i in range(5)) 
    for c in range(len(channel)):
        csvFeb.append(feb)
        csvHybrid.append(hyb)
        if(lowdaq[c] == 1.0 or minStatsFailure[c] == 1.0 or RMS[c] < 10 or dead[c] == 1.0):
            if(loadOnlineBaselines):
                csvMean.append(round(onlineMean[c],3))
                csvSigma.append(round(onlineSigma[c],3))
                csvType.append("online_baseline")
            else:
                csvMean.append(99999)
                csvSigma.append(99999)
                csvType.append("offline_fit_failure")
        else:
            csvMean.append(mean[c])
            csvSigma.append(sigma[c])
            csvType.append("offline_baseline")
    csvRows = zip(csvFeb, csvHybrid, channel, svt_id, csvMean, csvSigma, csvType)
    with open(csvOutFile,'a') as f:
        writer = csv.writer(f)
        for row in csvRows:
            writer.writerow(row)

            

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
    #savePNG(canvas,".","%s_%s_gausFit"%(run,hybrid))
    canvas.Close()
    
    if(hybrid_hh.GetEntries() == 0):
        continue
    if loadOnlineBaselines == True:
        if(len(mean) == 0 and len(onlineMean) == 0):
            continue;
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
        lowdaq_gr = buildTGraph("lowdaqflag_%s"%(hybrid),"low-daq channels_%s;Channel;ADC"%(hybrid),len(lowdaq_gr_x),lowdaq_gr_x, lowdaq_gr_y,3)

        diff_gr.Draw()

        lowdaq_gr.SetMarkerStyle(8)
        lowdaq_gr.SetMarkerSize(1)
        lowdaq_gr.SetMarkerColor(2)
        lowdaq_gr.Draw("psame")

        canvas.Write()
        canvas.Close()

outFile.Write()

