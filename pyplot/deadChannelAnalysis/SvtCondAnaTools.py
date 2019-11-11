#!/usr/bin/env python
import ROOT as r
import numpy as np
from copy import deepcopy
import utilities as utils

#RMS cutoff at 150 or less
def RatioDeadAlive(RMScut, scADCyps):
    #ratio of dead/alive channels for each half_module
    hName = scADCyps.GetNames
    numDead=0
    numChannels=0
    ratio=float(0.0)
    deadChannels=[]
    for idx, cc in enumerate(scADCrms[0:cmax]):
      # print cc
        if cc >= 0.0:
            numChannels=numChannels+1
        if cc >= 0.0 and cc <= RMScut:
            numDead=numDead+1
            deadChannels.append(idx)
    if numChannels!=0:
        ratio=float (numChannels-numDead)/numChannels
        return round(ratio*100,1), deadChannels
    else:
        return 0

def AvgRMS(scADCrms,cmax):
    Total=0.0
    for e in scADCrms[0:cmax]:
        Total=Total+e
    return Total/cmax

def RMSrms(scADCrms, cmax):
    squares=[]
    summ=0
    for e in scADCrms[0:cmax]:
       squares.append(np.square(e))
    for i in range(len(squares)):
        summ=summ+squares[i]
    return np.sqrt(summ/len(squares))
        

#Build Half Module Names
def BuildHybridKeys(sampleNumber):
    module="raw_hits_"
    ster="_stereo_"
    ax="_axial_"
    #layer 4-6
    ele="ele_"
    pos="pos_"
    ts="timesample_"
    half_module_names=["L0T","L0B","L1T","L1B","L2T","L2B","L3T","L3B","L4T","L4B","L5T","L5B","L6T","L6B"]
    hybrid_names=[]
    for i in range(len(half_module_names)):
        if i < 8:
            key=module+half_module_names[i]+ax+ts+str(sampleNumber)
            hybrid_names.append(key)
            key=module+half_module_names[i]+ster+ts+str(sampleNumber)
            hybrid_names.append(key)
        elif i >= 8:
            key=module+half_module_names[i]+ax+ele+ts+str(sampleNumber)
            hybrid_names.append(key)
            key=module+half_module_names[i]+ax+pos+ts+str(sampleNumber)
            hybrid_names.append(key)
            
            key=module+half_module_names[i]+ster+ele+ts+str(sampleNumber)
            hybrid_names.append(key)

            key=module+half_module_names[i]+ster+pos+ts+str(sampleNumber)
            hybrid_names.append(key)
    return hybrid_names

#Get a DeepCopy of hybrid_keys from TFile
def DeepCopy(inFile, hybrid_keys, plotTail):
   plots=[]
   for i in range(len(hybrid_keys)):
       plot = deepcopy(getattr(inFile, hybrid_keys[i]+plotTail))
       plots.append(plot)
   return plots

#Get the RMS Value of an array of 2D Histograms
def GetYPs(hybridHistos):
    #Get Y Projection and RMs Values
    scADCyps={}
    for i, histo in enumerate(hybridHistos):
        layers, modules = swModuleMapper()
        layer = layers[i]
        if layer > 4.5: cmax = 640
        else: cmax = 510
        hName = histo.GetName()
        scADCyps[hName] = []
        for cc in range(cmax):
            scADCyps[hname].append(histo.ProjectionY('%s_ch%i_h'%(hName, cc),cc+1,cc+1,"e"))
    return scADCyps

def DrawGraph(outFile, Name, titles, xdata_in, ydata_in):

    outFile.cd()
    x=[]
    j=0.00
    for i in range(len(data_in)):
        x.append(j)
        j=j+1.0
    n=len(x)
    gr=[]
    num=len(data_in)

    gr=r.TGraph(n,np.array(x),np.array(data_in))
    utils.SetStyle()
    gr.SetName(Name)
    gr.SetTitle(titles)
    SetStyle(gr)
    
    gr.Draw()
    gr.Write()

def SetStyle(histo,label_size=0.05, title_size=0.05, title_offset=1,line_width=1, line_color=1):

    histo.GetXaxis().SetLabelSize(label_size)
    histo.GetXaxis().SetTitleSize(title_size)
    histo.GetXaxis().SetTitleOffset(title_offset)
    histo.GetYaxis().SetLabelSize(label_size)
    histo.GetYaxis().SetTitleSize(title_size)

    histo.SetLineWidth(line_width)
    histo.SetLineColor(line_color)

def swModuleMapper():
    layer=[1.,2.,3.,4.,4.,3.,2.,1.,5.,6.,8.,7.,5.,6.,7.,8.,9.,9.,10.,10.,9.,9.,10.,10.,11.,11.,12.,12.,11.,11.,12.,12.,13.,13.,14.,14.,13.,13.,14.,14.]
    module=[0.,0.,0.,0.,1.,1.,1.,1.,0.,0.,0.,0.,1.,1.,1.,1.,0.,2.,0.,2.,1.,3.,1.,3.,0.,2.,0.,2.,1.,3.,1.,3.,0.,2.,0.,2.,1.,3.,1.,3.]
    return layer, module

def Histo1D(out_file_name,Name, Title, Xlabel, Ylabel, xbins, xlow, xmax, data_in=[], update="RECREATE", end=False):

    outFilename=out_file_name
    print outFilename
    outFile=r.TFile(outFilename,update)
    outFile.cd()
    

    histo=r.TH1F(Name, Title+";"+Xlabel+";"+Ylabel, xbins,xlow,xmax)
    SetStyle(histo)

    for i in range(len(data_in)):
        histo.Fill(data_in[i])
        
    histo.Draw("colz")
    histo.Write()
    
    if end==True:
        outFile.Close()


def Histo2D(out_file_name,Name, Title, Xlabel, Ylabel, xbins, xlow, xmax, ybins, ylow, ymax, FillX=[], FillY=[], data_in=[], update="RECREATE", end=False):

    outFilename=out_file_name
    outFile=r.TFile(outFilename,update)
    outFile.cd()
    

    histo=r.TH2D(Name, Title+";"+Xlabel+";"+Ylabel, xbins,xlow,xmax,ybins,ylow,ymax)
    #utils.SetStyle()
    SetStyle(histo)
    r.gStyle.SetNumberContours(999)
    histo.SetContour(999)

    for i in range(len(data_in)):
        histo.Fill(FillX[i],FillY[i],data_in[i])
        
    histo.Draw("colz")
    histo.Write()
    
    if end==True:
        outFile.Close()


def OpenCanvas(Name):
    r.gROOT.SetBatch(r.kTRUE)
    c=r.TCanvas(Name, "c", 1800, 800)
    #c.cd()
    return c
   
def SavePNG(c, outFile):
    c.Write()
    c.SaveAs(outFile)
    

def MakeCanvas(PNGoutFile, Name):
    r.gROOT.SetBatch(r.kTRUE)
    c=r.TCanvas(Name,"c", 1800, 800)
    c.cd()
    c.SetName(Name)
    c.Write()
    c.SaveAs(PNGoutFile)




