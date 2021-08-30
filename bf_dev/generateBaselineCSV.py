import ROOT as r
import time
from copy import deepcopy
import os.path
from os import path
import numpy as np
#import ModuleMapper as mmap
import ModuleMapper
import argparse
import csv


parser=argparse.ArgumentParser(description="Configuration options for baseline fit")


parser.add_argument("-i", type=str, dest="inFilename", help="Input SvtBlFitHistoProcessor output root file",default="")
parser.add_argument("-o", type=str, dest="outFilename", help="output root file",default="baselineFitTool.root")

parser.add_argument("-b", type=str, dest="onlineBaselines", help="online baseline fits root file",default="")
parser.add_argument("-csv", type=str, dest="csvOut", help="File name for CSV output file",default="offline_baseline_fits.csv")
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

def getHistosFromFile(inFile, histoType = "TH2D", name = ""):
    inFile.cd()
    histos = []
    for key in inFile.GetListOfKeys():
        plotName = key.GetName()
        plotType = key.GetClassName()
        if plotType.find(histoType) != -1 and (plotName.find(name) != -1):
            histos.append(inFile.Get(plotName))
    return histos

def getOfflineFitTuple(inFile, key):
    print("Grabbing Ntuples from TTree")
    channel, svt_id, mean, sigma, norm, chi2, ndf, fitlow, fithigh, RMS, lowdaq, lowstats = ([] for i in range(12))
    myTree = inFile.gaus_fit
    for fitData in myTree:
        tupleKey = str(fitData.halfmodule_hh)
        if key in tupleKey:
            channel.append(fitData.channel)
            mean.append(round(fitData.BlFitMean,3))
            sigma.append(round(fitData.BlFitSigma,3))
            norm.append(round(fitData.BlFitNorm,3))
            fitlow.append(fitData.BlFitRangeLower)
            fithigh.append(fitData.BlFitRangeUpper)
            chi2.append(fitData.BlFitChi2)
            ndf.append(fitData.BlFitNdf)
            lowdaq.append(fitData.lowdaq)
            svt_id.append(int(fitData.svt_id))
            lowstats.append(fitData.minStatsFailure)

    fitTuple = (svt_id, channel, mean, sigma, norm, ndf, fitlow, fithigh, RMS, lowdaq, lowstats)

    #Tuple map for reference
    svt_id = fitTuple[0]
    channel = fitTuple[1]
    mean = fitTuple[2]
    sigma = fitTuple[3]
    norm = fitTuple[4]
    ndf = fitTuple[5]
    offlinelow = fitTuple[6]
    offlinehigh = fitTuple[7]
    rms = fitTuple[8]
    lowdaq = fitTuple[9]
    lowstats = fitTuple[10]
    return fitTuple

def getOnlineFitTuple(inFile, hwtag):
    mean = []
    sigma = []
    channel = []

    #if(inFile.find(".root") != -1):
    #    onlineFile = r.TFile(options.onlineBaselines, "READ")
    #    plotName = "baseline/baseline_"+hwtag + "_ge"
    #    print('retrieving online baseline fits from:',plotName)
    #    plot = onlineFile.Get(plotName)
    #    print("successfully loaded online baseline")
    #    x = (plot.GetX())
    #    channel = list(x)
    #    y =(plot.GetY())
    #    mean = list(y)
    #    sigma = []
    #    for c in channel:
    #        sigma.append(plot.GetErrorY(int(c)))

    if(".dat" in inFile):
        try: 
            f = open(inFile, 'r')
        except:
            print("ERROR! ONLINE BASELINE FILE DOES NOT EXIST!")
            return ()

        for i in range(6):
            channel.append([])
            mean.append([])
            sigma.append([])
            for line in open(inFile, 'r'):
                svtID = int(line.split()[0])
                if svtID in svt_id: 
                    channel[i].append(float(svt_id.index(datsvtID)))
                    mean[i].append(float(line.split()[1+i]))
                    sigma[i].append(float(line.split()[i+7]))
    return (channel, mean, sigma)

def writeBaselineFitsToDatabase(outFile, offlineTuple, onlineTuple):
    #offline values
    svt_id = offlineTuple[0]
    channel = offlineTuple[1]
    mean = offlineTuple[2]
    sigma = offlineTuple[3]
    lowdaq = offlineTuple[9]
    lowstats = offlineTuple[10]

    #online values
    onlineSvtId = ()
    onlineMean = ()
    onlineSigma = ()

    if onlineTuple:
        onlineSvtId = onlineTuple[0]
        onlineMean = onlineTuple[1]
        onlineSigma = onlineTuple[2]

    #Write baselines to csv file
    with open(outFile,'a') as f:
        writer = csv.writer(f, delimiter = ' ')
        for c in range(len(channel)):
            row = [svt_id[c]]
            #Check if offline fit had any failures
            #If yes, use the loaded Online fit values instead
            if(lowdaq[c] == 1.0 or lowstats[c] == 1.0):
                if(loadOnlineBaselines):
                    try:
                        for i in range(6):
                            row.append(onlineMean[i][c])
                        for i in range(6):
                            row.append(onlineMean[i][c])
                    except:
                        for i in range(6):
                            row.append(999999.9)
                        for i in range(6):
                            row.append(999999.9)
            else:
                #Append offline fit value
                row.append(mean[c])
                #For 5 remaining time samples, load online values
                for i in range(5):
                    try:
                        row.append(round(onlineMean[i+1][c],3))
                    except:
                        row.append(999999.9)
                #Append offline fit value
                row.append(sigma[c])
                #For 5 remaining time samples, load online values
                for i in range(5):
                    try:
                        row.append(round(onlineSigma[i+1][c],3))
                    except:
                        row.append(999999.9)
            writer.writerow(row)

def plotOfflineOnlineFitDiff(outFile, hybrid, hybrid_hh, offlineTuple, onlineTuple, rootdir = ""):

    #If hybrid has no data, skip
    if hybrid_hh.GetEntries() == 0:
        return

    #If online fit data is empty, skip
    if not onlineTuple:
        return

    outFile.cd()
    if not outFile.GetDirectory("%s"%(rootdir)):
        #Create subdirectory for hybrid
        r.gDirectory.mkdir("%s"%(rootdir))

    #Save output to directory
    outFile.cd("%s"%(rootdir))

    #Get fit tuple values for hybrid
    fitTuple = getFitTuple(inFile, entry[1].GetName())
    svt_id = fitTuple[0]
    channel = fitTuple[1]
    mean = fitTuple[2]
    sigma = fitTuple[3]
    norm = fitTuple[4]
    ndf = fitTuple[5]
    fitlow = fitTuple[6]
    fithigh = fitTuple[7]
    rms = fitTuple[8]
    lowdaq = fitTuple[9]

    #online values for sample 0
    onlineSvtId = onlineTuple[0][0]
    onlineMean = onlineTuple[1][0]
    onlineSigma = onlineTuple[2][0]
    
    #Take difference between Online and Offline sample0 fit values
    diffMean = [x1 - x2 for (x1,x2) in zip(mean,onlineMean)]
    canvas = r.TCanvas("%s_mean_diff"%(hybrid), "testCanv",1800,800)
    canvas.cd()
    gr = r.TGraph(len(channel),np.array(channel, dtype = float), np.array(diffMean, dtype = float))
    gr.SetName("%s_baseline_mean_diff;channel;Mean [ADC]"%(hybrid))

    lowdaq_gr_x = np.array(channel, dtype = float)
    lowdaq_gr_y = np.array([-1000.0 + 600.0* x for x in lowdaq], dtype=float)
    lowdaq_gr = buildTGraph("lowdaqflag_%s"%(hybrid),"low-daq channels_%s;Channel;ADC"%(hybrid),len(lowdaq_gr_x),lowdaq_gr_x, lowdaq_gr_y,3)
    lowdaq_gr.SetMarkerStyle(8)
    lowdaq_gr.SetMarkerSize(1)
    lowdaq_gr.SetMarkerColor(2)

    gr.Draw()
    lowdaq_gr.Draw("psame")

    canvas.Write()
    canvas.Close()

def plot2DBaselineFits(outFile, hybrid, hybrid_hh, offlineTuple, onlineTuple):

    outFile.cd()
    
    #Offline Fit Values for hybrid
    svt_id = offlineTuple[0]
    channel = offlineTuple[1]
    mean = offlineTuple[2]
    sigma = offlineTuple[3]
    norm = offlineTuple[4]
    ndf = offlineTuple[5]
    fitlow = offlineTuple[6]
    fithigh = offlineTuple[7]
    rms = offlineTuple[8]
    lowdaq = offlineTuple[9]

    #Plot Offline Baseline Fit Mean Values on top of RawSvtHit 2D Histograms
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

    #If online baselines were loaded, plot online baselines over offline baseline hh 
    if onlineTuple:
        onlineSvtId = onlineTuple[0]
        onlineMean = onlineTuple[1]
        onlineSigma = onlineTuple[2]
        onlineChannel = []
        for svtid in onlineSvtId:
            onlineChannel.append(channel[svt_id.index(svtid)])
        bl_gr_x = np.array(onlineChannel[0], dtype = float)
        bl_gr_y = np.array(onlineMean[0], dtype = float)
        bl_gr = buildTGraph("onlineBlFits_%s"%(hybrid),"Online Baseline Fits_%s;Channel;ADC"%(hybrid),len(bl_gr_x),bl_gr_x,bl_gr_y,1)
        bl_gr.Draw("same")

    legend = r.TLegend(0.1,0.75,0.28,0.9)
    legend.AddEntry(mean_gr,"offline baselines using hpstr processor","l")
    if onlineTuple:
        legend.AddEntry(bl_gr,"online baseline fits","l")
    legend.AddEntry(lowdaq_gr,"low-daq threshold","p")
    legend.Draw()
    canvas.Write()
    #savePNG(canvas,".","%s_%s_gausFit"%(run,hybrid))
    canvas.Close()
    
######################################################################################################
#********* CODE SUMMARY **********#
#This script reads in the 2D Histograms of RawSvtHits and Offline Baseline Fit Tree
#produced by HPSTR's Offline Baseline Fitting processor.

############## To generate Offline Baseline Fits using HPSTR #############################
    #Reconstructed SLCIO for Run -> Root Tuple
        # hpstr recoTuple_cfg.py -i <inLcioFile> -o <outRootFile> -t <1=isData|0=isMC> -y <2016|2019>
    #Generate RawSvtHits 2D histograms from Root Tuple
        # hpstr anaSvtBl2D_cfg.py -i <inTupleRootFile> -o <outRootFile>
    #Fit Baselines from RawSvtHits 2D Histo channels
        # hpstr fitBL_cfg.py -i <inSvtBl2DRootFile> -o <outRootFile> -l L<0-6>
    #The output above is what this script runs as the input file!

#Read Offline Baseline Fits generated by HPSTR fitBL_cfg 
inFile = r.TFile(options.inFilename, "READ")
run = options.runNumber

#Create output root file
outFile = r.TFile(options.outFilename,"RECREATE")

#Create Offline Baseline Fits output file in format of HPS Collections Database
csvOutFile = options.csvOut
if(path.exists(csvOutFile)):
    csvOutFile = os.path.splitext(csvOutFile)[0] + "_" + time.strftime("%H%M%S") + ".csv"

#***The HPS Collections Database Baselines file format requires channel fit values for 6 time samples...
#...Offline fits returned by HPSTR only fit time sample 0...
#...The remaining 5 time sample fit values are provided by the Online Baseline Fits loaded from the database*

#Load Online Baselines from HPS collections database
loadOnlineBaselines = False
if(options.onlineBaselines) != "":
    loadOnlineBaselines = True

#Get 2D histograms of channel ADCs for each hybrid
histos = getHistosFromFile(inFile, "TH2F", "")

#Translate hybrid strings to hardware name.
#Sort hardware names to order svt_ids properly
hybridHwDict = {}
mmapper = ModuleMapper.ModuleMapper()
string_to_hardware = mmapper.string_to_hw
for histo in histos:
    for string in string_to_hardware:
        if string in histo.GetName(): 
            print("Found ", string, " in ", histo.GetName())
            hybridHwDict[string_to_hardware[string]] = histo
            break
hybridHwDict = sorted(hybridHwDict.items())

#Loop over histograms in dictionary
for entry in hybridHwDict:
    #Feb and Hybrid values
    hh = entry[1]
    hwtag = entry[0] 
    feb = hwtag[0:2]
    hyb = hwtag[2:]
    hybrid = mmapper.get_hw_to_string(hwtag)

    #Get Offline Baseline Fit values for hybrid
    offlineFitTuple = getOfflineFitTuple(inFile, hh.GetName())

    #Get Online Baseline Fit Tuple
    onlineFitTuple = ()
    if(options.onlineBaselines) != "":
        loadOnlineBaselines = True
        onlineFitTuple = getOnlineFitTuple(options.onlineBaselines, hwtag)

    plot2DBaselineFits(outFile, hybrid, hh, offlineFitTuple, onlineFitTuple)

    plotOfflineOnlineFitDiff(outFile, hybrid, hh, offlineFitTuple, onlineFitTuple, hybrid)

    writeBaselineFitsToDatabase(csvOutFile, offlineFitTuple, onlineFitTuple)

outFile.Write()

