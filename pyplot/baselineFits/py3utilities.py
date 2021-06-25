import ROOT as r 
from array import array
from copy import deepcopy
import os.path

colors  = [r.kBlack,r.kBlue+2,r.kRed+2,r.kGreen-1,r.kYellow+2,r.kRed+2,r.kAzure-2,r.kGreen-8,r.kOrange+3,r.kYellow+2,r.kRed+2,r.kBlue+2,r.kGreen-8,r.kOrange+3,r.kYellow+2,r.kRed+2,r.kBlue+2,r.kGreen-8,r.kOrange+3,r.kYellow+2,r.kRed+2,r.kBlue+2,r.kGreen-8,r.kOrange+3,r.kYellow+2,r.kRed+2,r.kBlue+2,r.kGreen-8,r.kOrange+3]
markers = [r.kFullCircle,r.kFullTriangleUp,r.kFullSquare,r.kOpenSquare,r.kOpenTriangleUp,r.kOpenCircle,r.kFullCircle,r.kOpenSquare,r.kFullSquare,r.kOpenTriangleUp,r.kOpenCircle,r.kFullCircle,r.kOpenSquare,r.kFullSquare,r.kOpenTriangleUp,r.kOpenCircle,r.kFullCircle,r.kOpenSquare,r.kFullSquare,r.kOpenTriangleUp,r.kOpenCircle,r.kFullCircle,r.kOpenSquare,r.kFullSquare,r.kOpenTriangleUp,r.kOpenCircle,r.kFullCircle,r.kOpenSquare,r.kFullSquare,r.kOpenTriangleUp]

fillColors = [r.kRed-6+3,r.kAzure-4,r.kYellow+2]
blueColors = [r.kBlue,r.kBlue+1,r.kBlue+2]

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
    tsize=0.08
    tzsize = 0.045
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

    hpsStyle.SetTitleOffset(0.7,"y")
    hpsStyle.SetTitleOffset(1.15,"x")


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

