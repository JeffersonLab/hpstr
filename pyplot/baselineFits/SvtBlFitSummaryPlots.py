import ROOT as r
from copy import deepcopy
import os.path
import numpy as np
import ModuleMapper as MM
import csv
from optparse import OptionParser

parser=OptionParser()

parser.add_option("-i", type="string", dest="inFilename", help="Input SvtBlFitHistoProcessor output root file",default="")

parser.add_option("-d", type="string", dest="hpstrpath", help="Path to hpstr",default="")

(options, args) = parser.parse_args()

r.gROOT.SetBatch(r.kTRUE)

#####################################################################################################
inFile = r.TFile(options.inFilename,"READ")
outFile = r.TFile("%s_SvtBLFitSummary.root"%(options.inFilename[:-5]),"RECREATE")
myTree = inFile.gaus_fit

lowdaqDict = {}
<<<<<<< HEAD
=======
nofitposDict = {}
lowdaqposDict = {}
>>>>>>> 730c4d174360a93940735b9849bba756e7ab3207
channelDict = {}
winsizeDict = {}
sigmaDict = {}
chi2NdfDict = {}

fit_hh = r.TH2F("fit_hh","Channels that were Fit;layer;module",14,0.5,14.5,4,-0.5,3.5)
fitloca_h = r.TH1F("fitloc_h","Location of Fit Channels;Channel;Entries",640,0,640)
badch_hh = r.TH2F("badch_hh","LowDaq or Low-Stats Channels;layer;module",14,0.5,14.5,4,-0.5,3.5)
nofitloca_h = r.TH1F("nofitloc_h","Location of No-Fit Channels;Channel;Entries",640,0,640)

lowdaq_hh = r.TH2F("lowdaq_ch_hh","Number of LowDaq Threshold Channels;layer;module",14,0.5,14.5,4,-0.5,3.5)

lowwin = 100 #This parameter is determined by looking at window size distributions 
lowwinsize_hh = r.TH2F("low_window_size_hh","Channels with window size <  %i;layer;module"%(lowwin),14,0.5,14.5,4,-0.5,3.5)

lowdaqtotwinsize_h = r.TH1F("lowdaqtotwinsize_h","Fit Window Size for Detector lowdaq;Window Size [ADC];Entries",250,0,1000)
normdaqtotwinsize_h = r.TH1F("normdaqtotwinsize_h","Fit Window Size for Detector normdaq;Window Size [ADC];Entries",250,0,1000)

for fitData in myTree:
    #Get Sensor name, and from that layer and module number
    sensor = str(fitData.SvtAna2DHisto_key)
    sensor = sensor.split('baseline0_')[1]
    hw = MM.str_to_sw(sensor)
    ly = float(hw.split('_')[0][2:])
    mod = float(hw.split('_')[1][1:])
    
    #Fill Dictionaries
    if fitData.minbinFail == 0.0:
        lowdaqDict.setdefault(sensor,[]).append(fitData.lowdaq)
        channelDict.setdefault(sensor,[]).append(fitData.channel)
        winsizeDict.setdefault(sensor,[]).append(fitData.BlFitRangeUpper - fitData.BlFitRangeLower)
        sigmaDict.setdefault(sensor,[]).append(fitData.BlFitSigma)
        chi2NdfDict.setdefault(sensor,[]).append(fitData.BlFitChi2/fitData.BlFitNdf)

    #Fill 2D Histograms

    #Minimum Statistics Failure
    if fitData.minbinFail == 0.0:
        fit_hh.Fill(float(ly),float(mod),1.)
        fitloca_h.Fill(fitData.channel)
<<<<<<< HEAD

    if sensor.find("L0") != -1 or sensor.find("L1") != -1:
        chlimit = 511.0
    else:
        chlimit = 640.0
    if (fitData.minbinFail == 1.0 or fitData.lowdaq == 1.0) and fitData.channel <= chlimit:
        badch_hh.Fill(float(ly),float(mod),1.)
   #     nofitloca_h.Fill(fitData.channel)

    if fitData.minbinFail == 0.0:
        #lowdaq flagged channels
        lowdaq_hh.Fill(float(ly),float(mod),fitData.lowdaq)

        #Fit Window Sizes
        if fitData.BlFitRangeUpper - fitData.BlFitRangeLower < lowwin: 
            lowwinsize_hh.Fill(ly,mod,1.)
        r.gStyle.SetNumberContours(999)
        lowdaq_hh.SetContour(999)
        lowwinsize_hh.SetContour(999)
        lowdaq_hh.SetContour(999)
    
outFile.cd()
fit_hh.Write()
fitloca_h.Write()
badch_hh.Write()
#nofitloca_h.Write()
lowdaq_hh.Write()
lowwinsize_hh.Write()

for sensor in channelDict:

    #Low Daq channels
    lowdaqwinsize_h = r.TH1F("%s_lowdaqwinsize_h"%(sensor),"Fit Window Size for lowdaq %s;Window Size [ADC];Entries"%(sensor),250,0,1000)
    lowdaqwinsizech_h = r.TH1F("%s_lowdaqwinsizech_h"%(sensor),"Fit Window Size for lowdaq %s;Channel;Window Size [ADC]"%(sensor),640,0,640)

    lowdaq_h = r.TH1F("%s_lowdaq_h"%(sensor),"Low DAQ Threshold %s;Channel;Status (1=low)"%(sensor),640,0.0,640.0)

    lowdaq_sigma_h = r.TH1F("%s_lowdaq_sigma_h"%(sensor),"Low DAQ Fit Sigma %s;Channel;Sigma [ADC]"%(sensor),640,0.0,640.0)
    lowdaq_chi2Ndf_h = r.TH1F("%s_lowdaq_chi2Ndf_h"%(sensor),"Low DAQ Fit Sigma %s;Channel;Sigma [ADC]"%(sensor),640,0.0,640.0)

    #Normal Daq Channels
    normdaqwinsize_h = r.TH1F("%s_normdaqwinsize_h"%(sensor),"Fit Window Size for normdaq %s;Window Size [ADC];Entries"%(sensor),250,0,1000)
    normdaqwinsizech_h = r.TH1F("%s_normdaqwinsizech_h"%(sensor),"Fit Window Size for normdaq %s;Channel;Window Size [ADC]"%(sensor),640,0,640)


    normdaq_sigma_h = r.TH1F("%s_normdaq_sigma_h"%(sensor),"Normal DAQ Fit Sigma %s;Channel;Sigma [ADC]"%(sensor),640,0.0,640.0)
    normdaq_chi2Ndf_h = r.TH1F("%s_normdaq_chi2Ndf_h"%(sensor),"Normal DAQ Fit Chi2/Ndf %s;Channel;Sigma [ADC]"%(sensor),640,0.0,640.0)

    

    for i, cc in enumerate(channelDict[sensor]): 
        lowdaq_h.SetBinContent(int(cc),lowdaqDict[sensor][i])

        if lowdaqDict[sensor][i] == 1.0:
            lowdaqwinsize_h.Fill(winsizeDict[sensor][i],1.)
            lowdaqwinsizech_h.SetBinContent(int(cc),winsizeDict[sensor][i])
            lowdaqtotwinsize_h.Fill(winsizeDict[sensor][i],1.)
            lowdaq_sigma_h.SetBinContent(int(cc),sigmaDict[sensor][i])
            lowdaq_sigma_h.SetLineColor(2)
            lowdaq_chi2Ndf_h.SetBinContent(int(cc),chi2NdfDict[sensor][i])
        else:
            normdaqwinsize_h.Fill(winsizeDict[sensor][i],1.)
            normdaqwinsizech_h.SetBinContent(int(cc),winsizeDict[sensor][i])
            normdaq_sigma_h.SetBinContent(int(cc),sigmaDict[sensor][i])
            normdaq_chi2Ndf_h.SetBinContent(int(cc),chi2NdfDict[sensor][i])
            normdaqtotwinsize_h.Fill(winsizeDict[sensor][i],1.)
            
    lowdaq_h.Write()
    lowdaq_sigma_h.Write()
    lowdaq_chi2Ndf_h.Write()
    lowdaqwinsize_h.Write()
    lowdaqwinsizech_h.Write()

    normdaq_sigma_h.Write()
    normdaq_sigma_h.Write()
    normdaqwinsize_h.Write()
    normdaqwinsizech_h.Write()

lowdaqtotwinsize_h.Write()
normdaqtotwinsize_h.Write()
    
