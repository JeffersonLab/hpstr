#!/usr/bin/env python
import ROOT as r
import numpy as np
from copy import deepcopy
import utilities as utils


def getRMS(histo1d):
    rms = histo1d.GetRMS()
    return rms


#Build Half Module Names
def BuildHybridKeys(prefix, suffix):
    module=""
    ster="_stereo_"
    ax="_axial_"
    #layer 4-6
    ele="ele_"
    pos="pos_"
    half_module_names=["L0T","L0B","L1T","L1B","L2T","L2B","L3T","L3B","L4T","L4B","L5T","L5B","L6T","L6B"]
    hybrid_names=[]
    for i in range(len(half_module_names)):
        if i < 8:
            key=prefix+module+half_module_names[i]+ax+suffix
            hybrid_names.append(key)
            key=prefix+module+half_module_names[i]+ster+suffix
            hybrid_names.append(key)
        elif i >= 8:
            key=prefix+module+half_module_names[i]+ax+ele+suffix
            hybrid_names.append(key)
            key=prefix+module+half_module_names[i]+ax+pos+suffix
            hybrid_names.append(key)
            
            key=prefix+module+half_module_names[i]+ster+ele+suffix
            hybrid_names.append(key)

            key=prefix+module+half_module_names[i]+ster+pos+suffix
            hybrid_names.append(key)
    return hybrid_names

#Get a DeepCopy of hybrid_keys from TFile
def DeepCopy(inFile, plotNames):
   plots={}
   for plotName in plotNames:
       plot = deepcopy(getattr(inFile, plotName))
       plots[plotName] = plot
   return plots

def getYprojections(histo2d):
    yproj={}
    for cc in range(histo2d.GetNbinsX()):
        yproj[cc] = (histo2d.ProjectionY('%s_ch%i_h'%(histo2d.GetName(), cc),cc+1,cc+1,"e"))
    return yproj


def DrawGraph(outFile, Name, titles, x_in, data_in):

    outFile.cd()
    x=[]
    j=0.00
    for i in range(x_in):
        x.append(j)
        j=j+1.0
    n=len(x)
    gr=[]

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
    layers=[1.,2.,3.,4.,4.,3.,2.,1.,5.,6.,8.,7.,5.,6.,7.,8.,9.,9.,10.,10.,9.,9.,10.,10.,11.,11.,12.,12.,11.,11.,12.,12.,13.,13.,14.,14.,13.,13.,14.,14.]
    modules=[0.,0.,0.,0.,1.,1.,1.,1.,0.,0.,0.,0.,1.,1.,1.,1.,0.,2.,0.,2.,1.,3.,1.,3.,0.,2.,0.,2.,1.,3.,1.,3.,0.,2.,0.,2.,1.,3.,1.,3.]
    return layers, modules

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




