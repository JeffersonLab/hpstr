from ROOT import *
from array import array
from copy import deepcopy
import os,sys

colors  = [kBlack,kBlue+2,kRed+2,kGreen-1,kYellow+2,kRed+2,kAzure-2,kGreen-8,kOrange+3,kYellow+2,kRed+2,kBlue+2,kGreen-8,kOrange+3,kYellow+2,kRed+2,kBlue+2,kGreen-8,kOrange+3,kYellow+2,kRed+2,kBlue+2,kGreen-8,kOrange+3,kYellow+2,kRed+2,kBlue+2,kGreen-8,kOrange+3]
markers = [kFullCircle,kFullTriangleUp,kFullSquare,kOpenSquare,kOpenTriangleUp,kOpenCircle,kFullCircle,kOpenSquare,kFullSquare,kOpenTriangleUp,kOpenCircle,kFullCircle,kOpenSquare,kFullSquare,kOpenTriangleUp,kOpenCircle,kFullCircle,kOpenSquare,kFullSquare,kOpenTriangleUp,kOpenCircle,kFullCircle,kOpenSquare,kFullSquare,kOpenTriangleUp,kOpenCircle,kFullCircle,kOpenSquare,kFullSquare,kOpenTriangleUp]


#General configuration

bottomFraction = 0.4
bottomScale = 1./bottomFraction
topScale    = 1./(1. - bottomFraction)
TProfile.Approximate(True)


def OptParsing():
    
    from optparse import OptionParser
    parser=OptionParser()
    parser.add_option("--inputFile",dest="inputFile",help="inputFile",default="")
    parser.add_option("--outdir",dest="outdir",help="outdir",default="")
    parser.add_option("--indir",dest="indir",help="indir",default="")
    parser.add_option("--runNumber",dest="runNumber",help="runNumber",default="")
    parser.add_option("--Selections",dest="Selections",default="LooseTracks,LooseL2VTracks,LooseT2VTracks")
    parser.add_option("--Legends",dest="Legends",default="Loose,Loose + L2V,Loose + T2V")
    parser.add_option("--Legends2",dest="Legends2",default="Tracks,Tracks + Truth,Tracks_L2V,Tracks_L2V + Truth,Tracks_T2V,Tracks_T2V + Truth")
    parser.add_option("--dataFile",dest="dataFile",default="")
    (config,sys.argv[1:]) = parser.parse_args(sys.argv[1:])
    return config


#Get a plot from a directory+file name
def getPlot(loc,fin,plot):
    print "Getting", plot
    f = TFile.Open(loc+fin)
    histo = f.Get(plot)
    print histo
    histo.SetDirectory(0)
    
    return histo


#Get a plot from a file
def getPlot(fullpath,plot):
    print "Getting", plot
    f = TFile.Open(fullpath)
    histo = f.Get(plot)
    print histo
    histo.SetDirectory(0)
    
    return histo


#Pass a list of files
def MakeHistoListFromFiles(listOfFiles,path,histoName):

    histolist = []
    for infile in listOfFiles:
        f = TFile.Open(infile)
        print f
        h = f.Get(path+histoName)
        print path+histoName
        print h
        h.SetDirectory(0)
        histolist.append(h)
    return histolist

#Pass a list of histogram names
def MakeHistoListFromSameFile(infile,path,histoNames):
    histolist = []
    for h_name in histoNames:
        print h_name
        f = TFile.Open(infile)
        print f
        
        h = f.Get(path+"/"+h_name)
        print h
        h.SetDirectory(0)
        histolist.append(h)
    return histolist    


def InsertText(runNumber="",texts=[],line=0.87,xoffset=0.18,Hps=True):

    
    newline = 0.06

    text = TLatex()
    text.SetNDC()
    text.SetTextFont(42)
    text.SetTextSize(0.05)
    text.SetTextColor(kBlack)
    if (Hps):
        text.DrawLatex(xoffset,line,'#bf{#it{HPS} Internal}')
    if runNumber:
        line=line-newline
        if "MC" in runNumber:
            text.DrawLatex(xoffset,line,"MC Simulation")
        else:
            #text.DrawLatex(xoffset,line,"Run "+runNumber)
            text.DrawLatex(xoffset,line,runNumber)
    for iText in xrange(len(texts)):
        if texts[iText]:
            line=line-newline
            text.DrawLatex(xoffset,line,texts[iText])
        


def SetStyle():
    gROOT.SetBatch(1)

    hpsStyle= TStyle("HPS","HPS style")
    
    # use plain black on white colors
    icol=0
    hpsStyle.SetFrameBorderMode(icol)
    hpsStyle.SetCanvasBorderMode(icol)
    hpsStyle.SetPadBorderMode(icol)
    hpsStyle.SetPadColor(icol)
    hpsStyle.SetCanvasColor(icol)
    hpsStyle.SetStatColor(icol)
#hpsStyle.SetFillColor(icol)
    
# set the paper & margin sizes
    hpsStyle.SetPaperSize(20,26) 
    hpsStyle.SetPadTopMargin(0.05)
    hpsStyle.SetPadRightMargin(0.05)
    hpsStyle.SetPadBottomMargin(0.18)
    hpsStyle.SetPadLeftMargin(0.14)
    
    # use large fonts
#font=72
    font=42
    tsize=0.07
    tzsize = 0.055
    hpsStyle.SetTextFont(font)

    
    hpsStyle.SetTextSize(tsize)
    hpsStyle.SetLabelFont(font,"x")
    hpsStyle.SetTitleFont(font,"x")
    hpsStyle.SetLabelFont(font,"y")
    hpsStyle.SetTitleFont(font,"y")
    hpsStyle.SetLabelFont(font,"z")
    hpsStyle.SetTitleFont(font,"z")
    
    hpsStyle.SetLabelSize(tsize,"x")
    hpsStyle.SetTitleSize(tsize,"x")
    hpsStyle.SetLabelSize(tsize,"y")
    hpsStyle.SetTitleSize(tsize,"y")
    hpsStyle.SetLabelSize(tzsize,"z")
    hpsStyle.SetTitleSize(tzsize,"z")

    hpsStyle.SetTitleOffset(0.8,"y")
    hpsStyle.SetTitleOffset(1.3,"x")
    
    
#use bold lines and markers
    #hpsStyle.SetMarkerStyle(20)
    hpsStyle.SetMarkerSize(1.0)
    hpsStyle.SetHistLineWidth(3)
    hpsStyle.SetLineStyleString(2,"[12 12]") # postscript dashes
    
#get rid of X error bars and y error bar caps
#hpsStyle.SetErrorX(0.001)
    
#do not display any of the standard histogram decorations
    hpsStyle.SetOptTitle(0)
#hpsStyle.SetOptStat(1111)
    hpsStyle.SetOptStat(0)
#hpsStyle.SetOptFit(1111)
    hpsStyle.SetOptFit(0)
    
# put tick marks on top and RHS of plots
    hpsStyle.SetPadTickX(1) 
    hpsStyle.SetPadTickY(1)
    
    gROOT.SetStyle("Plain")

#gStyle.SetPadTickX(1)
#gStyle.SetPadTickY(1)
    gROOT.SetStyle("HPS")
    gROOT.ForceStyle() 
    gStyle.SetOptTitle(0)
    gStyle.SetOptStat(0) 
    gStyle.SetOptFit(0) 


# overwrite hps styles
    hpsStyle.SetPadLeftMargin(0.14)
    hpsStyle.SetPadRightMargin(0.06)    
    hpsStyle.SetPadBottomMargin(0.11)     
    hpsStyle.SetPadTopMargin(0.05) 
    hpsStyle.SetFrameFillColor(0)

    NRGBs = 5;
    NCont = 255;
    
    stops = array("d",[ 0.00, 0.34, 0.61, 0.84, 1.00 ])
    red   = array("d",[ 0.00, 0.00, 0.87, 1.00, 0.51 ])
    green = array("d",[ 0.00, 0.81, 1.00, 0.20, 0.00 ])
    blue  = array("d",[ 0.51, 1.00, 0.12, 0.00, 0.00 ])
    TColor.CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
    gStyle.SetNumberContours(NCont);


def MakePlot(name,outdir,histos,legends,oFext,xtitle="",ytitle="",ymin=0,ymax=1,noErrors=False,RebinFactor=0,runNumber="",additionalText=[],RatioType="Alternate",LogX=False,LogY=False,RatioMin=0.25,RatioMax=1.75,WriteMean=False,Normalise=False):
    
    
    if not os.path.exists(outdir):
        os.mkdir(outdir)
        
    Xmin=0
    Xmax=1
        
    can = TCanvas(name, name, 1200, 800)
    can.SetMargin(0,0,0,0)
    top = TPad("top","top",0,0.42,1,1)
    if LogX:
        top.SetLogx(1)
        bot.SetLogx(1)
    if LogY:
        top.SetLogy(1)
    
    bot = TPad("bot","bot",0,0,1,0.38)

    #----------Histogram------------#
    
    top.Draw()
    top.SetBottomMargin(0)
    top.SetTopMargin(gStyle.GetPadTopMargin()*topScale)
    bot.Draw()
    bot.SetTopMargin(0)
    bot.SetBottomMargin(0.4)
    top.cd()
    plotsProperties=[]
    
    for ih in xrange(len(histos)):

        if (Normalise):
            histos[ih].Scale(1./histos[ih].Integral())
            histos[ih].GetYaxis().SetRangeUser(0.00001,histos[ih].GetMaximum()*15000)
            
                    
        histos[ih].SetMarkerColor(colors[ih])
        histos[ih].SetMarkerStyle(markers[ih])
        histos[ih].SetLineColor(colors[ih])
        histos[ih].GetXaxis().CenterTitle()
        histos[ih].GetYaxis().CenterTitle()
        
        plotsProperties.append(("#mu=%.4f"%round(histos[ih].GetMean(),4))+(" #sigma=%.4f"%round(histos[ih].GetRMS(),4)))
        
        
        if RebinFactor>0:
            histos[ih].Rebin(RebinFactor)

        if ih==0:
            #histos[ih].GetYaxis().SetRangeUser(ymin,ymax)
            if noErrors:
                #histos[ih].GetXaxis().SetTextSize(0.045)
                #histos[ih].GetYaxis().SetTextSize(0.045)
                histos[ih].Draw("pe")
                
            else:
                histos[ih].Draw("pe")
            if xtitle:
                histos[ih].GetXaxis().SetTitle(xtitle)
            if ytitle:
                histos[ih].GetYaxis().SetTitle(ytitle)
        else:
            if noErrors:
                histos[ih].Draw("same hist")
            else:
                histos[ih].Draw("same hist")


    InsertText(runNumber,additionalText,0.8,xoffset=0.75)
    if (WriteMean):
        InsertText("",plotsProperties,0.8,0.6,False)

    if len(legends)>0:
        #print "building legend"
        #upperY=0.6
        upperY=0.76
        linesep = 0.07
        lowerY=upperY - len(legends)* linesep
        #minX = 0.51
        minX  = 0.75
        maxX = minX+0.15
        leg=TLegend(minX,upperY,maxX,lowerY)
        leg.SetBorderSize(0)
        leg.SetFillColor(0)
        leg.SetTextSize(0.04)
        for i_leg in xrange(len(legends)):
            #print "Adding Entry",i_leg, legends[i_leg] 
            leg.AddEntry(histos[i_leg],legends[i_leg],"lpf") 
            pass
        leg.Draw()
        pass
        
    #-------------Ratio---------------------#
        
    bot.cd()
    reference = histos[0].Clone("reference")
    reference.GetYaxis().SetTitle("Ratio")
    reference.GetYaxis().SetTitleSize(0.06)
    reference.GetXaxis().SetTitleSize(0.1)
    reference.GetXaxis().SetLabelSize(0.12)
    reference.GetYaxis().SetRangeUser(RatioMin,RatioMax)
    reference.GetYaxis().SetNdivisions(508)
    reference.GetYaxis().SetDecimals(True)
    reference.Draw("axis")

    if (RatioType=="Sequential"):
        
        
        for ih in range(1,len(histos)):
            #ForRatio=None
            #if type(histos[ih]) is TProfile:
            #    ForRatio = histos[ih].ProjectionX("ForRatio"+str(ih)+histos[ih].GetName())
            #else:
            ForRatio = histos[ih].Clone("ForRatio"+str(ih)+histos[ih].GetName())
            
            ForRatio.SetMaximum(100.)
            ForRatio.Divide(reference)
            ForRatio.DrawCopy("pe same")


    elif (RatioType=="Alternate"):
        
        print "in alternate ratio"
        for ih in range(1,len(histos),2):
            
            numerator=histos[ih].Clone("numerator")
            #if isinstance(histos[ih],TProfile):
                
            #    numerator=histos[ih].ProjectionX("numerator")
                
            #else:
            
                
            
            numerator.SetMaximum(100.)
            numerator.Divide(histos[ih-1])
            numerator.DrawCopy("pe same")


    elif (RatioType=="Alternate2"):
          print "in Alternate (h1 - h2) / h2"
          

          numerator=histos[ih].Clone("numerator")
          numerator.Add(histos[ih-1],-1)
          numerator.SetMaximum(100.)
          numerator.Divide(histos[ih-1])
          
          numerator.DrawCopy("hist p same")
          
          
          
    line = TLine()
    line.SetLineStyle(kDashed)
    line.DrawLine(reference.GetXaxis().GetXmin(),1,reference.GetXaxis().GetXmax(),1)
    
    can.SaveAs(outdir+"/"+name+oFext)
    return deepcopy(can)
    

def DivideHistos(h1,h2):
    for ibin in range(1,h1.GetNbinsX()+1):
        if (h2.GetBinContent(ibin) == 0):
            continue
        h1.SetBinContent(ibin,h1.GetBinContent(ibin) / h2.GetBinContent(ibin))
    return h1



def Make1Dplots(name,outdir,histos,colors,markers,legends,oFext,xtitle="",ytitle="",ymin=0,ymax=1,noErrors=False,RebinFactor=0,runNumber="",additionalText=[],LogY=False,WriteMean=False,multiLeg=False):
        

    if not os.path.exists(outdir):
        os.mkdir(outdir)

                
    can = TCanvas(name,name,1500,1000)
    if LogY:
        can.SetLogy(1)

    means   = []
    meansErr = []

    
    for ih in xrange(len(histos)):
        
        means.append(histos[ih].GetMean(2))
        meansErr.append(histos[ih].GetMeanError(2))
        
        histos[ih].SetMarkerColor(colors[ih])
        histos[ih].SetMarkerStyle(markers[ih])
        histos[ih].SetLineColor(colors[ih])
        histos[ih].GetYaxis().SetRangeUser(ymin,ymax)
        histos[ih].GetXaxis().CenterTitle()
        histos[ih].GetYaxis().CenterTitle()
        if ("pT" in name or "pt" in name):
            histos[ih].GetXaxis().SetRangeUser(1.,20.)
        #histos[ih].SetMarkerSize(0.5)
        if RebinFactor>0:
            histos[ih].Rebin(RebinFactor)

        if ih==0:
            if noErrors:
                histos[ih].GetXaxis().SetTextSize(0.045)
                histos[ih].GetYaxis().SetTextSize(0.045)
                histos[ih].Draw("hist p")
            else:
                histos[ih].Draw()
            if xtitle:
                histos[ih].GetXaxis().SetTitle(xtitle)
            if ytitle:
                histos[ih].GetYaxis().SetTitle(ytitle)
        else:
            if noErrors:
                histos[ih].Draw("same hist p")
            else:
                histos[ih].Draw("same")
                        
    InsertText(runNumber,additionalText,0.8,xoffset=0.7)

    if len(legends)>0:
        #print "building legend"
        upperY=0.6
        linesep = 0.10
        lowerY=upperY - len(legends)* linesep
        minX = 0.51
        maxX = minX+0.33
        leg2 = None

        if len(legends) > 10:
            leg2=TLegend(minX,upperY,maxX-0.02,lowerY)
            leg2.SetBorderSize(0)
            leg2.SetFillColor(0)
            entry=leg2.AddEntry("Todo","To do","p")
            entry.SetMarkerStyle(kOpenSquare)
            
            entry2=leg2.AddEntry("Todo","Todo","p")
            entry2.SetMarkerStyle(kOpenCircle)
            leg2.Draw()
            

            
            minX=0.87
            maxX=minX+0.12
            

        leg=TLegend(minX,upperY,maxX,lowerY)
        leg.SetBorderSize(0)
        leg.SetFillColor(0)
        leg.SetTextSize(0.031)
        

                
        for i_leg in xrange(len(legends)):
            
            if not WriteMean:
                print "Adding Entry",i_leg, legends[i_leg] 
            else:
                print "Adding Entry",i_leg, legends[i_leg]+" #epsilon: " + str(round(means[i_leg],2))
            

            #leg.AddEntry(histos[i_leg],"#splitline{"+legends[i_leg] + "}{Average=" + str(round(means[i_leg]*100.,3))+"#pm"+str(round(meansErr[i_leg]*100,3))+"%}" ,"lpf")
            #leg.AddEntry(histos[i_leg],legends[i_leg] + " Average=" + str(round(means[i_leg]*100.,3))+"#pm"+str(round(meansErr[i_leg]*100,3))+"%" ,"lpf")
            if not multiLeg:
                leg.AddEntry(histos[i_leg],legends[i_leg] + " Avg=" + str(round(means[i_leg]*100.,3))+"#pm"+str(round(meansErr[i_leg]*100,3))+"%" ,"lpf")
            else:
                leg.AddEntry(histos[i_leg],"#splitline{"+legends[i_leg] + "}{Avg=" + str(round(means[i_leg]*100.,3))+"#pm"+str(round(meansErr[i_leg]*100,3))+"%}" ,"lpf")
            
            leg.Draw()

    #BuildLegend(legends,can,histos,0.55,0.9,0.8,0.75)
    
        
    can.SetBottomMargin(0.18)
    can.SetLeftMargin(0.15)

    can.SaveAs(outdir+"/"+name+oFext)
    


def Make2DRatio(name,outdir,histo1,histo2,xtitle="",ytitle="",ztitle="",runNumber="",legends=[]):
    oFext=".pdf"
    if not os.path.exists(outdir):
        os.mkdir(outdir)

    ratio = histo1.Clone()
    ratio.Divide(histo2)
    
    can = TCanvas()
    can.SetRightMargin(0.2)
    
    ratio.GetZaxis().SetRangeUser(0.9,1.1)
    ratio.GetXaxis().SetTitle(xtitle)
    ratio.GetYaxis().SetTitle(ytitle)
    ratio.GetZaxis().SetTitle(ztitle)
    ratio.Draw("colz text")
    
    can.SaveAs(outdir+"/"+name+oFext)
    
    

def Make2DPlots(name,outdir,histolist,xtitle="",ytitle="",ztitle="",text="",zmin="",zmax=""):
    oFext=".pdf"
    if not os.path.exists(outdir):
        os.mkdir(outdir)

    for ih in range(0,len(histolist)):
        can = TCanvas()
        can.SetRightMargin(0.2)
        
        histolist[ih].GetZaxis().SetRangeUser(zmin,zmax)
        histolist[ih].GetXaxis().SetTitle(xtitle)
        histolist[ih].GetXaxis().SetTitleSize(
            histolist[ih].GetXaxis().GetTitleSize()*0.7)
        histolist[ih].GetXaxis().SetLabelSize(
            histolist[ih].GetXaxis().GetLabelSize()*0.75)
        histolist[ih].GetXaxis().SetTitleOffset(
            histolist[ih].GetXaxis().GetTitleOffset()*0.8)
        
        histolist[ih].GetYaxis().SetTitleSize(
            histolist[ih].GetYaxis().GetTitleSize()*0.7)
        histolist[ih].GetYaxis().SetLabelSize(
            histolist[ih].GetYaxis().GetLabelSize()*0.75)
        histolist[ih].GetYaxis().SetTitleOffset(
            histolist[ih].GetYaxis().GetTitleOffset()*1.7)
        histolist[ih].GetYaxis().SetTitle(ytitle)
        
        
        histolist[ih].Draw("colz")
        
        InsertText(text,"")
        
        print "saving..."
        #if (len(legends) == len(histolist)):
        can.SaveAs(outdir+"/"+name+oFext)
        #else:            
        #    print "ERROR: Not enough names for all the histos"
            

def Profile2DPlot(name,outdir,histolist,axis="X",xtitle="",ytitle="",ztitle="",runNumber="",legends=[],zmin="",zmax=""):
    oFext=".pdf"
    if not os.path.exists(outdir):
        os.mkdir(outdir)
        
    p = None
    
    if (axis == "X"):
        p = ProjectionX()
    
