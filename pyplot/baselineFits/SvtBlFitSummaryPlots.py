import ROOT as r
from copy import deepcopy
import os.path
import numpy as np
import ModuleMapper as MM
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
channelDict = {}
winsizeDict = {}
sigmaDict = {}

lowdaq_hh = r.TH2F("lowdaq_ch_hh","Number of LowDaq Threshold Channels;layer;module",14,0.5,14.5,4,-0.5,3.5)

lowwin = 100 #This parameter is determined by looking at window size distributions 
lowwinsize_hh = r.TH2F("low_window_size_hh","Channels with window size <  %i layer;module"%(lowwin),14,0.5,14.5,4,-0.5,3.5)

totwinsize_h = r.TH1F("totwinsize_h","Fit Window Size for Detector;Window Size [ADC];Entries",250,0,1000)

for fitData in myTree:
    #Get Sensor name, and from that layer and module number
    sensor = str(fitData.SvtAna2DHisto_key)
    sensor = sensor.split('baseline0_')[1]
    hw = MM.str_to_sw(sensor)
    ly = float(hw.split('_')[0][2:])
    mod = float(hw.split('_')[1][1:])
    
    lowdaqDict.setdefault(sensor,[]).append(fitData.lowdaq)
    channelDict.setdefault(sensor,[]).append(fitData.channel)
    winsizeDict.setdefault(sensor,[]).append(fitData.BlFitRangeUpper - fitData.BlFitRangeLower)
    sigmaDict.setdefault(sensor,[]).append(fitData.BlFitSigma)

    #Fill 2D Histograms
    lowdaq_hh.Fill(float(ly),float(mod),fitData.lowdaq)
    lowwinsize_hh.Fill(ly,mod,fitData.BlFitRangeUpper - fitData.BlFitRangeLower)
    r.gStyle.SetNumberContours(999)
    lowdaq_hh.SetContour(999)
    lowwinsize_hh.SetContour(999)
    lowdaq_hh.SetContour(999)
    
outFile.cd()
lowdaq_hh.Write()
lowwinsize_hh.Write()

for sensor in channelDict:

    winsize_h = r.TH1F("%s_winsize_h"%(sensor),"Fit Window Size %s;Channel;Window Size [ADC]"%(sensor),250,0,1000)

    lowdaq_h = r.TH1F("%s_lowdaq_h"%(sensor),"Low DAQ Threshold %s;Channel;Status (1=low)"%(sensor),640,0.0,640.0)

    lowdaq_sigma_h = r.TH1F("%s_lowdaq_sigma_h"%(sensor),"Low DAQ Fit Sigma %s;Channel;Sigma [ADC]"%(sensor),640,0.0,640.0)

    normdaq_sigma_h = r.TH1F("%s_normdaq_sigma_h"%(sensor),"Normal DAQ Fit Sigma %s;Channel;Sigma [ADC]"%(sensor),640,0.0,640.0)


    for i, cc in enumerate(channelDict[sensor]): 
        winsize_h.Fill(winsizeDict[sensor][i],1.)
        lowdaq_h.SetBinContent(int(cc),lowdaqDict[sensor][i])
        totwinsize_h.Fill(winsizeDict[sensor][i],1.)
        if lowdaqDict[sensor][i] == 1.0:
            lowdaq_sigma_h.SetBinContent(int(cc),sigmaDict[sensor][i])
            lowdaq_sigma_h.SetLineColor(2)
        else:
            normdaq_sigma_h.SetBinContent(int(cc),sigmaDict[sensor][i])
            
    lowdaq_h.Write()
    lowdaq_sigma_h.Write()
    normdaq_sigma_h.Write()
    winsize_h.Write()
totwinsize_h.Write()
    
