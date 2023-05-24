import ROOT as r
import time
from copy import deepcopy
import os.path
from os import path
import numpy as np
import argparse
import csv
import math
import ModuleMapper

parser=argparse.ArgumentParser(description="Configuration options for baseline fit")
parser.add_argument("-i", type=str, dest="inFilename", help="Input SvtBlFitHistoProcessor output root file",default="")
parser.add_argument("-o", type=str, dest="outFilename", help="output root file",default="baselineFitTool.root")
parser.add_argument("-b", type=str, dest="onlineBaselines", help="online baseline fits root file",default="")
parser.add_argument("-dbo", type=str, dest="dbOut", help="File name for database baseline output file",default="offline_baseline_fits.dat")
parser.add_argument("-r", type=str, dest="runNumber", help="Run Number",default="")
parser.add_argument("-y", type=str, dest="year", help="year",default=2019)
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
              
def buildTGraph(name, title, n_points, x, y, color, markerStyle = 1, markerSize = 1):
    g = r.TGraph(n_points,np.array(x, dtype = float), np.array(y, dtype = float))
    g.SetName("%s"%(name))
    g.SetTitle("%s"%(title))
    g.SetLineColor(color)

    g.SetMarkerStyle(markerStyle)
    g.SetMarkerSize(markerSize)
    return g

def buildTGraphErrors(name, title, n_points, x, y, ey, color):
    #filter out errors less than 0
    #ey = [x for x in ey if x > 0]
    ex = [0]*len(x)
    ey = [e if e > 0 else 0 for e in ey]

    g = r.TGraphErrors(n_points, np.array(x, dtype = float), np.array(y, dtype = float), np.array(ex, dtype = float), np.array(ey, dtype = float))
    
    g.SetName("%s"%(name))
    g.SetTitle("%s"%(title))
    g.SetLineColor(color)
    return g

def get2DHistos(inFile, histoType = "TH2D", name = ""):
    inFile.cd()
    histos = []
    for key in inFile.GetListOfKeys():
        plotName = key.GetName()
        plotType = key.GetClassName()
        if plotType.find(histoType) != -1 and (plotName.find(name) != -1):
            histos.append(inFile.Get(plotName))
    return histos

def readOfflineBaselineFits(inFile, key):
    print("Grabbing Ntuples for %s from TTree"%(key))

    channel, svt_id, mean, sigma, norm, chi2, ndf, fitlow, fithigh, RMS, lowdaq, lowstats, badfit, hm_string, superlowDaq, threshold, minthreshold = ([] for i in range(17))
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
            lowstats.append(fitData.lowStats)
            badfit.append(fitData.badfit)
            hm_string.append(fitData.halfmodule_hh)
            superlowDaq.append(fitData.suplowDaq)
            threshold.append(fitData.threshold)
            minthreshold.append(fitData.minthreshold)

    offlineBaselineResults = {'svtid':svt_id,'channel':channel,'mean':mean,'sigma':sigma , 'norm':norm , 'ndf':ndf , 'fitlow':fitlow , 'fithigh':fithigh , 'RMS':RMS , 'lowdaq':lowdaq }
    fitTuple = (svt_id, channel, mean, sigma, norm, ndf, fitlow, fithigh, RMS, lowdaq, lowstats, badfit, hm_string, superlowDaq, threshold, minthreshold)

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
    badfit = fitTuple[11]
    hm_string = fitTuple[12]
    superlowDaq = fitTuple[13]
    thresholds = fitTuple[14]

    return fitTuple

def readOnlineBaselines(inFile, svt_id_range):
    mean = []
    sigma = []
    channel = []
    svt_id_out = []

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
            svt_id_out.append([])
            for line in open(inFile, 'r'):
                svtID = int(line.split()[0])
                if svtID in svt_id_range:
                    #channel[i].append(float(svt_id_range.index(svtID)))
                    #channel[i].append(float(svt_ids.index(datsvtID)))
                    #channel[i].append(float(offline_channels[offline_svt_ids.index(svtID)]))
                    svt_id_out[i].append(float(svtID))
                    mean[i].append(float(line.split()[1+i]))
                    sigma[i].append(float(line.split()[i+7]))
        svt_id_out = svt_id_out[0]
    return (svt_id_out, mean, sigma)

def makeDatabaseBaselinesOnlineNoise(offlineTuple, onlineTuple):
    svt_id = offlineTuple[0]
    channel = offlineTuple[1]
    mean = offlineTuple[2]
    sigma = offlineTuple[3]
    lowdaq = offlineTuple[9]
    lowstats = offlineTuple[10]
    badfit = offlineTuple[11]
    hm_string = offlineTuple[12]
    superlowDaq = offlineTuple[13]

    onlineSvtId = onlineTuple[0]
    onlineMean = onlineTuple[1]
    onlineSigma = onlineTuple[2]

    baselines = []
    #Write baselines to database formatted array
    for c in range(len(channel)):
        #database format is row: svt_id mean0 mean1 mean2 mean3 mean4 mean5 sigma0 sigma1...sigma5
        baselines.append([])
        row = [svt_id[c]]

        #Check if offline fit had any failures
        #If yes, use the loaded Online fit values instead
        if(badfit[c] == 1.0 or lowstats[c] == 1.0):
            for i in range(6):
                row.append(onlineMean[i][c])
            for i in range(6):
                row.append(onlineSigma[i][c])
        else:
            #Append offline fit value
            row.append(mean[c])
            #For 5 remaining time samples, load online values
            for i in range(5):
                #take difference between offline and online sample 0
                diff = mean[c] - onlineMean[0][c]
                #apply difference to 5 other online samples to adjust
                row.append(round(onlineMean[i+1][c] + diff,3))

            #Append ONLINE noise values
            for i in range(6):
                row.append(onlineSigma[i][c])
        baselines[c] = row
    return baselines

def makeDatabaseBaselines(offlineTuple, onlineTuple):
    svt_id = offlineTuple[0]
    channel = offlineTuple[1]
    mean = offlineTuple[2]
    sigma = offlineTuple[3]
    lowdaq = offlineTuple[9]
    lowstats = offlineTuple[10]
    badfit = offlineTuple[11]
    hm_string = offlineTuple[12]
    superlowDaq = offlineTuple[13]

    onlineSvtId = onlineTuple[0]
    onlineMean = onlineTuple[1]
    onlineSigma = onlineTuple[2]

    baselines = []
    #Write baselines to database formatted array
    for c in range(len(channel)):
        #database format is row: svt_id mean0 mean1 mean2 mean3 mean4 mean5 sigma0 sigma1...sigma5
        baselines.append([])
        row = [svt_id[c]]

        #Check if offline fit had any failures
        #If yes, use the loaded Online fit values instead
        if(badfit[c] == 1.0 or lowstats[c] == 1.0):
            for i in range(6):
                row.append(onlineMean[i][c])
            for i in range(6):
                row.append(onlineSigma[i][c])
        else:
            #Append offline fit value
            row.append(mean[c])
            #For 5 remaining time samples, load online values
            for i in range(5):
                #take difference between offline and online sample 0
                diff = mean[c] - onlineMean[0][c]
                #apply difference to 5 other online samples to adjust
                row.append(round(onlineMean[i+1][c] + diff,3))

            #Append offline fit value
            row.append(sigma[c])
            #For 5 remaining time samples, load online values
            for i in range(5):
                #take offline/online sample 0 noise ratio
                ratio = sigma[c]/onlineSigma[0][c]
                #apply ratio factor to 5 other online samples to adjust
                row.append(round(onlineSigma[i+1][c]*ratio,3))
        baselines[c] = row
    return baselines

def plotOfflineOnlineFitDiff(outFile, hybrid, hybrid_hh, offlineTuple, onlineTuple, rootdir = ""):

    #If hybrid has no data, skip
    if hybrid_hh.GetEntries() == 0:
        return

    #If online fit data is empty, skip
    if not onlineTuple:
        return

    #outFile.cd()
    #if not outFile.GetDirectory("%s"%(rootdir)):
    #    #Create subdirectory for hybrid
    #    r.gDirectory.mkdir("%s"%(rootdir))

    #Save output to directory
    #outFile.cd("%s"%(rootdir))

    #Get fit tuple values for hybrid
    #fitTuple = getFitTuple(inFile, entry[1].GetName())
    fitTuple = offlineTuple
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
    lowstats = fitTuple[10]
    badfit = fitTuple[11]
    superlowDaq = fitTuple[13]

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

    lowstats_gr_x = np.array(channel, dtype = float)
    lowstats_gr_y = np.array([-1000.0 + 500.0* x for x in lowstats], dtype=float)
    lowstats_gr = buildTGraph("lowstatsflag_%s"%(hybrid),"low-stats channels_%s;Channel;ADC"%(hybrid),len(lowstats_gr_x),lowstats_gr_x, lowstats_gr_y,3)
    lowstats_gr.SetMarkerStyle(47)
    lowstats_gr.SetMarkerSize(1)
    lowstats_gr.SetMarkerColor(6)

    gr.Draw()
    lowdaq_gr.Draw("psame")
    lowstats_gr.Draw("psame")

    canvas.Write()
    canvas.Close()

def debugBadFits(hybrid, offlineTuple):
    
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
    lowstats = offlineTuple[10]
    badfit = offlineTuple[11]

    print("Checking Bad Fits for ",hybrid)
    for i, c in  enumerate(channel):
        if badfit[i] == 1:
            print("Bad Fit Channel ", c)
            print("Mean: ",mean[i])
            print("Sigma: ",sigma[i])
            print("norm: ",norm[i])
            print("minx: ",fitlow[i])
            print("maxx: ",fithigh[i])

def plotBaselineFitResults(outFile, hybrid, hybrid_hh, offlineTuple, onlineTuple):

    print("Plotting Baseline Fit Results")
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
    lowstats = offlineTuple[10]
    badfit = offlineTuple[11]
    superlowDaq = offlineTuple[13]

    #Plot Offline Baseline Fit Mean Values on top of RawSvtHit 2D Histograms
    canvas = r.TCanvas("%s_mean"%(hybrid), "c", 1800,800)
    canvas.cd()
    canvas.SetTicky()
    canvas.SetTickx()
    #mean_gr_x = np.array([m if l < 1 else 0 for m,l in zip(channel,lowstats)], dtype = float)
    #mean_gr_x = np.array([c if l < 1 for c,l in zip(channel,lowstats)], dtype = float)
    #mean_gr_x = np.array([c for c in channel if l < 1 for l in lowstats], dtype = float)
    mean_gr_x = np.array([c for c,l in zip(channel,lowstats) if l < 1], dtype = float)
    mean_gr_y = np.array([m for m,l in zip(mean,lowstats) if l < 1], dtype = float)
    if(mean_gr_x.any()):
        mean_gr = buildTGraphErrors("%s_BlFitMean_%s"%(run,hybrid),"%s_BlFitMean_%s;Channel;ADC"%(run,hybrid),len(mean_gr_x),mean_gr_x,mean_gr_y, sigma, 2)
        mean_gr.SetLineWidth(1)

    lowdaq_gr_x = np.array([c for c,l in zip(channel,lowdaq) if l > 0], dtype = float)
    lowdaq_gr_y = np.array([m for m,l in zip(mean,lowdaq) if l > 0], dtype = float)
    if(lowdaq_gr_x.any()):
        lowdaq_gr = buildTGraph("lowdaqflag_%s"%(hybrid),"low-daq channels_%s;Channel;ADC"%(hybrid),len(lowdaq_gr_x),lowdaq_gr_x, lowdaq_gr_y,2)
        lowdaq_gr.SetMarkerStyle(34)
        lowdaq_gr.SetMarkerSize(1)
        lowdaq_gr.SetMarkerColor(3)

    superlowDaq_gr_x = np.array([c for c,l in zip(channel,superlowDaq) if l > 0], dtype = float)
    superlowDaq_gr_y = np.array([m for m,l in zip(mean,superlowDaq) if l > 0], dtype = float)
    if(superlowDaq_gr_x.any()):
        superlowDaq_gr = buildTGraph("superlowDaqflag_%s"%(hybrid),"super low-daq channels_%s;Channel;ADC"%(hybrid),len(superlowDaq_gr_x),superlowDaq_gr_x, superlowDaq_gr_y,2)
        superlowDaq_gr.SetMarkerStyle(47)
        superlowDaq_gr.SetMarkerSize(1)
        superlowDaq_gr.SetMarkerColor(6)

    badfit_gr_x = np.array([c for c,l in zip(channel,badfit) if l > 0], dtype = float)
    badfit_gr_y = np.array([m for m,l in zip(mean,badfit) if l > 0], dtype = float)
    if(badfit_gr_x.any()):
        badfit_gr = buildTGraph("badfitflag_%s"%(hybrid),"bad-fit channels_%s;Channel;ADC"%(hybrid),len(badfit_gr_x),badfit_gr_x, badfit_gr_y,3)
        badfit_gr.SetMarkerStyle(39)
        badfit_gr.SetMarkerSize(1)
        badfit_gr.SetMarkerColor(2)

    hybrid_hh.GetYaxis().SetRangeUser(2000.0,7500.0)
    if hybrid_hh.GetName().find("L0") != -1 or hybrid_hh.GetName().find("L1") != -1:
        hybrid_hh.GetXaxis().SetRangeUser(0.0,512.0)
    hybrid_hh.SetStats(r.kFALSE)
    hybrid_hh.Draw("colz")
    if(mean_gr_x.any()):
        mean_gr.Draw("same")
    if(lowdaq_gr_x.any()):
        lowdaq_gr.Draw("psame")
    if(superlowDaq_gr_x.any()):
        superlowDaq_gr.Draw("psame")
    #lowstats_gr.Draw("psame")
    if(badfit_gr_x.any()):
        badfit_gr.Draw("psame")

    #If online baselines were loaded, plot online baselines over offline baseline hh 
    if onlineTuple:
        onlineSvtId = onlineTuple[0]
        onlineMean = onlineTuple[1]
        onlineSigma = onlineTuple[2]
        onlineChannel = []
        for svtid in onlineSvtId:
            onlineChannel.append(channel[svt_id.index(svtid)])
        bl_gr_x = np.array(onlineChannel, dtype = float)
        bl_gr_y = np.array(onlineMean[0], dtype = float)
        bl_gr = buildTGraph("onlineBlFits_%s"%(hybrid),"Online Baseline Fits_%s;Channel;ADC"%(hybrid),len(bl_gr_x),bl_gr_x,bl_gr_y,1)
        bl_gr.SetLineWidth(1)
        bl_gr.Draw("same")

    legend = r.TLegend(0.1,0.75,0.28,0.9)
    if(mean_gr_x.any()):
        legend.AddEntry(mean_gr,"offline baselines using hpstr processor","l")
    if onlineTuple:
        legend.AddEntry(bl_gr,"online baseline fits","l")
    if(lowdaq_gr_x.any()):
        legend.AddEntry(lowdaq_gr,"low-daq threshold","p")
    if(superlowDaq_gr_x.any()):
        legend.AddEntry(superlowDaq_gr,"super low-daq threshold","p")
    if(badfit_gr_x.any()):
        legend.AddEntry(badfit_gr,"failed fit","p")
    legend.Draw()
    canvas.Write()
    canvas.Close()

def plotLowDaq(hybrid, offlineTuple, outFile):
    #Offline Fit Values for hybrid
    channel = offlineTuple[1]
    lowdaq = offlineTuple[9]

    y = np.array(lowdaq, dtype= float)
    x = np.array(channel, dtype=float)

    #canvas = r.TCanvas("%s_lowdaq"%(hybrid), "%s_lowdaq"%(hybrid), 1800, 800)
    #canvas.cd()

    g = buildTGraph("lowdaq_%s"%(hybrid),"Low_Daq_%s;Channel;(1 = lowdaq)"%(hybrid),len(x),x,y,1,1,10)
    g.SetMinimum(-1)
    g.SetMaximum(2)
    g.Draw("AP")
    g.Write()

    #canvas.Update()
    #canvas.Write()

def plotFitSigma(hybrid, offlineTuple, outFile):

    #Offline Fit Values for hybrid
    channel = offlineTuple[1]
    sigma = offlineTuple[3]

    #TGraph of channel fit sigma
    sigma_g = buildTGraph("Fit_Sigma_%s"%(hybrid),"Fit_Sigma_%s;sigma;channel"%(hybrid),len(channel),np.array(channel, dtype = float),np.array(sigma, dtype=float),1)
    sigma_g.SetMinimum(-100)
    sigma_g.SetMaximum(400)
    sigma_g.Write()

def plotOfflineChannelFits(hybrid, hh, offlineTuple, outFile):

    outFile.cd()
    #Create subdirectory for channel fits in hybrid dir
    r.gDirectory.mkdir("%s/channel_fits"%(hybrid))
    #Save output to directory
    outFile.cd("%s/channel_fits"%(hybrid))

    #offline fit values
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
    threshold = offlineTuple[14]
    minthreshold = offlineTuple[15]

    for cc in range(len(channel)):
        canvas = r.TCanvas("%s_ch_%i_h"%(hybrid,channel[cc]), "c", 1800,800)
        canvas.cd()
        projy_h = hh.ProjectionY('%s_ch%i_h'%(hybrid,channel[cc]),int(channel[cc]+1),int(channel[cc]+1),"e")
        if projy_h.GetEntries() == 0:
            continue
        projy_h.SetTitle("%s_ch_%i_h"%(hybrid,channel[cc]))

        func = r.TF1("m1","gaus",fitlow[cc],fithigh[cc])
        func.SetParameter(0,norm[cc])
        func.SetParameter(2,sigma[cc])
        func.SetParameter(1,mean[cc])

        projy_h.SetTitle("%s_ch_%i_h"%(hybrid,channel[cc]))
        projy_h.Draw()

        func.Draw("same")

        tbin = projy_h.FindBin(threshold[cc])
        tbinVal = projy_h.GetBinContent(tbin)
        maxline = r.TLine(threshold[cc],0,threshold[cc],tbinVal)
        maxline.SetLineWidth(2)
        maxline.Draw("same")

        
        tbin = projy_h.FindBin(minthreshold[cc])
        tbinVal = projy_h.GetBinContent(tbin)
        minline = r.TLine(minthreshold[cc],0,minthreshold[cc],tbinVal)
        minline.SetLineWidth(2)
        minline.Draw("same")

        canvas.Write()
        canvas.Close()
    outFile.cd()

def F5H1Thresholds(svtThresholdsFile, thresholdsFileOut):
    thresholds = []
    for i in range(5):
        thresholds.append([])
    feb = 5
    hybrid = 1
    with open(svtThresholdsFile) as ti:
        lines = ti.readlines()
        for line in lines:
            f = int(line.split()[0])
            h = int(line.split()[1])
            if f==feb and h==hybrid:
                a = int(line.split()[2])
                thresholds[a] = line.split()
            else:
                continue
    with open(thresholdsFileOut,'a') as f:
        writer = csv.writer(f, delimiter = ' ', escapechar=' ', quoting=csv.QUOTE_NONE)
        for row in thresholds:
            writer.writerow(row)

def generateThresholds(outFile, outputFile, offlineBaselinesTuple, onlineBaselinesTuple, febnum, hybnum, hybrid):

    channel = list(offlineBaselinesTuple[1])
    mean = list(offlineBaselinesTuple[2])
    sigma = list(offlineBaselinesTuple[3])
    lowdaq = list(offlineBaselinesTuple[9])
    lowstats = list(offlineBaselinesTuple[10])
    badfit = list(offlineBaselinesTuple[11])
    superlowDaq = list(offlineBaselinesTuple[13])

    onmean = onlineBaselinesTuple[1][0]
    onsigma = onlineBaselinesTuple[2][0]

    if(int(febnum) == 5 and int(hybnum) == 1):
        return
    else:
        doublethresholds = [m + 2.5*s if (b < 1 and sl < 1 and l < 1) else om + 2.5*os for m, s, om, os, b, l, sl in zip(mean, sigma, onmean, onsigma, badfit, lowstats, superlowDaq)]
    thresholds = [str(hex(math.floor(n))) for n in doublethresholds]

    #L0-L1 have 4 APVs, else 5 APVs
    napvs = 5
    if int(febnum) < 2:
        napvs = 4

    #Write baselines to csv file
    with open(outFile,'a') as f:
        writer = csv.writer(f, delimiter = ' ', escapechar=' ', quoting=csv.QUOTE_NONE)
    
        #Change APV channel mapping for L0 sensors
        if int(febnum) < 2:
            for apv in range(napvs):
                #output format is: Febn Hybn APVn ch0_thresh ch1_thres ch2_thresh ...
                row = [febnum, hybnum]
                row.append(apv)
                if apv == 0:
                    row.append(' '.join(thresholds[128:256]))
                    #print("Feb %i APV %i has %i channel thresholds"%(int(febnum),apv, len(thresholds[128:256])))
                elif apv == 1:
                    row.append(' '.join(thresholds[0:128]))
                    #print("Feb %i APV %i has %i channel thresholds"%(int(febnum),apv, len(thresholds[0:128])))
                elif apv == 2:
                    row.append(' '.join(thresholds[256:384]))
                    #print("Feb %i APV %i has %i channel thresholds"%(int(febnum),apv, len(thresholds[256:384])))
                elif apv == 3:
                    row.append(' '.join(thresholds[384:512]))
                    #print("Feb %i APV %i has %i channel thresholds"%(int(febnum),apv, len(thresholds[384:512])))
                writer.writerow(row)
        else: 
            #change APV channel mapping for non-L0 sensors
            for apv in range(napvs):
                row = [febnum, hybnum]
                row.append(apv)
                if apv == 0:
                    row.append(' '.join(thresholds[512:640]))
                    #print("Feb %i APV %i has %i channel thresholds"%(int(febnum),apv, len(thresholds[512:640])))
                elif apv == 1:
                    row.append(' '.join(thresholds[384:512]))
                    #print("Feb %i APV %i has %i channel thresholds"%(int(febnum),apv, len(thresholds[384:512])))
                elif apv == 2:
                    row.append(' '.join(thresholds[256:384]))
                    #print("Feb %i APV %i has %i channel thresholds"%(int(febnum),apv, len(thresholds[256:384])))
                elif apv == 3:
                    row.append(' '.join(thresholds[128:256]))
                    #print("Feb %i APV %i has %i channel thresholds"%(int(febnum),apv, len(thresholds[128:256])))
                elif apv == 4:
                    row.append(' '.join(thresholds[0:128]))
                    #print("Feb %i APV %i has %i channel thresholds"%(int(febnum),apv, len(thresholds[0:128])))
                writer.writerow(row)


    #Plot offline vs online thresholds to check the changes
    outputFile.cd()
    canvas = r.TCanvas("%s_thresholds"%(hybrid), "c", 1800,800)
    canvas.cd()
    canvas.SetTicky()
    canvas.SetTickx()

    gx1 = np.array(channel, dtype = float)
    gy1 = np.array(doublethresholds, dtype = float)
    #gy1 = np.array([m + 3*s if l < 1 and b < 1 else om + 3*os for m, s, om, os, b, l in zip(mean, sigma, onmean, onsigma, badfit, lowstats)], dtype = float)
    gr1 = buildTGraph("%s_offline_thresholds"%(hybrid),"Offline_Thresholds_%s;Channel;ADC"%(hybrid),len(gx1),gx1,gy1,1)

    gx2 = np.array(channel, dtype = float)
    gy2 = np.array([m + 3*s for m, s in zip(onmean, onsigma)], dtype = float)
    gr2 = buildTGraph("%s_online_thresholds"%(hybrid),"Online_Thresholds_%s;Channel;ADC"%(hybrid),len(gx2),gx2,gy2,1)

    gr1.SetLineWidth(2)
    gr2.SetLineWidth(2)
    gr2.SetLineColor(2)

    lowdaq_gr_x = np.array(channel, dtype = float)
    lowdaq_gr_y = np.array([l * m for l,m in zip(lowdaq,mean)], dtype=float)
    lowdaq_gr = buildTGraph("lowdaqflag_%s"%(hybrid),"low-daq channels_%s;Channel;ADC"%(hybrid),len(lowdaq_gr_x),lowdaq_gr_x, lowdaq_gr_y,2)
    lowdaq_gr.SetMarkerStyle(49)
    lowdaq_gr.SetMarkerSize(2)
    lowdaq_gr.SetMarkerColor(3)

    lowstats_gr_x = np.array(channel, dtype = float)
    lowstats_gr_y = np.array([l * m for l,m in zip(lowstats,mean)], dtype=float)
    lowstats_gr = buildTGraph("lowstatsflag_%s"%(hybrid),"low-stats channels_%s;Channel;ADC"%(hybrid),len(lowstats_gr_x),lowstats_gr_x, lowstats_gr_y,3)
    lowstats_gr.SetMarkerStyle(47)
    lowstats_gr.SetMarkerSize(2)
    lowstats_gr.SetMarkerColor(6)

    badfit_gr_x = np.array(channel, dtype = float)
    badfit_gr_y = np.array([b * m for b,m in zip(badfit,mean)], dtype=float)
    badfit_gr = buildTGraph("badfitflag_%s"%(hybrid),"bad-fit channels_%s;Channel;ADC"%(hybrid),len(badfit_gr_x),badfit_gr_x, badfit_gr_y,3)
    badfit_gr.SetMarkerStyle(39)
    badfit_gr.SetMarkerSize(2)
    badfit_gr.SetMarkerColor(1)

    gr1.Draw()
    gr2.Draw("same")
    lowstats_gr.Draw("psame")
    badfit_gr.Draw("psame")

    legend = r.TLegend(0.1,0.75,0.28,0.9)
    legend.AddEntry(gr1,"offline thresholds","l")
    legend.AddEntry(gr2,"online thresholds","l")
    legend.AddEntry(lowstats_gr,"low-stats channel","p")
    legend.AddEntry(badfit_gr,"failed fit","p")
    legend.Draw()
    canvas.Write()
    canvas.Close()

def graphFitSample0(outFile, baselines, hybrid):

    #Build TGraphErrors of Sample0 baselines
    mean_s0 = []
    sigma_s0 = []
    channels = []
    for c, channel in enumerate(baselines):
        mean_s0.append(channel[1])
        sigma_s0.append(channel[7])
        channels.append(c)
    blgr = r.TGraphErrors(len(channels), np.array(channels, dtype = float), np.array(mean_s0, dtype = float), np.array([0]*len(channels), dtype = float), np.array(sigma_s0, dtype = float))
    blgr.SetName("%s_baseline_0"%(hybrid))
    blgr.SetTitle("%s_baseline_0"%(hybrid))
    blgr.SetLineWidth(2)
    blgr.Draw()
    blgr.Write()

######################################################################################################
#********* CODE SUMMARY **********#
#This script reads in the 2D Histograms of RawSvtHits and Offline Baseline Fit Tree
#produced by HPSTR's Offline Baseline Fitting processor.

############## To generate Offline Baseline Fits using HPSTR #############################
    #Reconstructed SLCIO for Run -> Root Tuple
        # hpstr recoTuple_cfg.py -i <inLcioFile> -o <outputFile> -t <1=isData|0=isMC> -y <2016|2019>
    #Generate RawSvtHits 2D histograms from Root Tuple
        # hpstr anaSvtBl2D_cfg.py -i <inTupleRootFile> -o <outputFile>
    #Fit Baselines from RawSvtHits 2D Histo channels
        # hpstr fitBL_cfg.py -i <inSvtBl2DRootFile> -o <outputFile> -l L<0-6>
    #The output above is what this script runs as the input file!

#Read Offline Baseline Fits generated by HPSTR fitBL_cfg 
inFile = r.TFile(options.inFilename, "READ")
run = options.runNumber
year = int(options.year)

#Create output root file
outputFile = r.TFile(options.outFilename,"RECREATE")

#Create Offline Baseline Fits output file in format of HPS Collections Database
databaseOut = options.dbOut
if(path.exists(databaseOut)):
    databaseOut = os.path.splitext(databaseOut)[0] + "_" + time.strftime("%H%M%S") + ".dat"

#Get 2D histograms of channel ADCs for each hybrid
histos = get2DHistos(inFile, "TH2F", "")

#Translate hybrid strings to hardware name.
#Sort hardware names to order svt_ids properly
hybridHwDict = {}
mmapper = ModuleMapper.ModuleMapper(year)
string_to_hardware = mmapper.string_to_hw
for histo in histos:
    for string in string_to_hardware:
        if string in histo.GetName(): 
            print("Found ", string, " in ", histo.GetName())
            hybridHwDict[string_to_hardware[string]] = histo
            break
hybridHwDict = sorted(hybridHwDict.items())

nFailedFits = 0

#Loop over histograms in dictionary
for entry in hybridHwDict:
    #Feb and Hybrid values
    hh = entry[1]
    hwtag = entry[0] 
    febn = hwtag[1:2]
    hybn = hwtag[3:]
    hybrid = mmapper.get_hw_to_string(hwtag)

    #Get Offline Baseline Fit values for hybrid
    offlineBaselinesTuple = readOfflineBaselineFits(inFile, hh.GetName())

    #Count failed fits
    for i in offlineBaselinesTuple[11]:
        if i > 0.0:
            nFailedFits = nFailedFits + 1
    print("N Failed Fits: ",nFailedFits)

    hybrid_svtids = offlineBaselinesTuple[0]
    hybrid_channels = offlineBaselinesTuple[1]

    #Get Online Baseline Fit Tuple
    onlineBaselinesTuple = readOnlineBaselines(options.onlineBaselines, hybrid_svtids)

    #Combine online and offline baselines to make database baseline file
    #database_baselines = makeDatabaseBaselines(offlineBaselinesTuple, onlineBaselinesTuple)

    #Use offline pedestal values, when possible.
    #Default use ONLINE NOISES
    database_baselines = makeDatabaseBaselinesOnlineNoise(offlineBaselinesTuple, onlineBaselinesTuple)
    with open(databaseOut,'a') as f:
        writer = csv.writer(f, delimiter = ' ')
        for channel in database_baselines:
            writer.writerow(channel)

    plotBaselineFitResults(outputFile, hybrid, hh, offlineBaselinesTuple, onlineBaselinesTuple)

    #Create subdirs for each hybrid
    outputFile.cd()
    if not outputFile.GetDirectory("%s"%(hybrid)):
        #Create subdirectory for hybrid
        r.gDirectory.mkdir("%s"%(hybrid))
    #Save output to directory
    outputFile.cd("%s"%(hybrid))

    graphFitSample0(outputFile, database_baselines, hybrid)
    plotOfflineOnlineFitDiff(outputFile, hybrid, hh, offlineBaselinesTuple, onlineBaselinesTuple, hybrid)

    plotFitSigma(hybrid,offlineBaselinesTuple,outputFile)
    plotLowDaq(hybrid, offlineBaselinesTuple, outputFile)
    plotOfflineChannelFits(hybrid, hh, offlineBaselinesTuple, outputFile)

outputFile.Write()

print("N Failed Fits: ",nFailedFits)
