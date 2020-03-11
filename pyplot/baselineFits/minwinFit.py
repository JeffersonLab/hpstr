import ROOT as r
from copy import deepcopy
import os.path
import numpy as np
from optparse import OptionParser

def get_comma_separated_args(option, opt, value, parser):
    setattr(parser.values, option.dest, value.split(','))

parser=OptionParser()

parser.add_option('-c', '--channels', type='string',action='callback', callback=get_comma_separated_args,dest = "channels")

parser.add_option("-g", type="string", dest="show_graphs", help="If == show, print all graphs associate with specified channels", default="")

parser.add_option("-f", type="string", dest="show_fits", help="If == show, print all channel fits for selected channels",default="")

parser.add_option("-i", type="string", dest="inFilename", help="Input SvtBlFitHistoProcessor output root file",default="")

parser.add_option("-t", type="string", dest="tag", help="file extension tag",default="")

parser.add_option("-s", "--hybrid", type="string", dest="hybrid",
        help="L<#><T/B>_<axial/stereo>_<ele/pos>", default="")

(options, args) = parser.parse_args()
######################################################################################################

def getHistoKeys(inFile,cType="", attr1="", attr2=""):
    inFile.cd()
    histo_keys=[]
    for key in inFile.GetListOfKeys():
        kname = key.GetName()
        classType = key.GetClassName()
        if classType.find(cType) != -1 and kname.find(attr1) != -1 and kname.find(attr2) != -1:
            histo_keys.append(kname)
    return histo_keys
              
def readhistoFromFile(inFile, histo_key):
    histo = deepcopy(getattr(inFile, histo_key))
    return histo

def getChannelFitsPlots(inFile,channels):
    inFile.cd()

def getTupleFlags(inFile,key):
    inFile.cd()
    minbinFail=[]
    noisy=[]
    lowdaq=[]
    channels=[]
    myTree = inFile.gaus_fit
    for fitData in myTree:
        SvtAna2DHisto_key = str(fitData.SvtAna2DHisto_key)
        if key == SvtAna2DHisto_key+"_hh":
            channels.append(fitData.channel)
            minbinFail.append(fitData.minbinFail)
            noisy.append(fitData.noisy)
            lowdaq.append(fitData.lowdaq)
    return channels, minbinFail, noisy, lowdaq



def getGausFitParameters(inFile, key):
    inFile.cd()
    histo_key=[]
    channel=[]
    mean=[]
    sigma=[]
    norm = []
    range_lower=[]
    range_upper=[]
    Chi2=[]
    Ndf=[]

    myTree = inFile.gaus_fit
    for fitData in myTree:
        SvtAna2DHisto_key = str(fitData.SvtAna2DHisto_key)
        if key == SvtAna2DHisto_key+"_hh":
            histo_key.append(SvtAna2DHisto_key)
            channel.append(fitData.channel)
            mean.append(fitData.BlFitMean)
            sigma.append(fitData.BlFitSigma)
            norm.append(fitData.BlFitNorm)
            range_lower.append(fitData.BlFitRangeLower)
            range_upper.append(fitData.BlFitRangeUpper)

    return channel,mean, sigma, histo_key,norm, range_lower, range_upper
               
def buildTGraph(name,title, n_points, x, y,color):
    g = r.TGraph(n_points,np.array(x, dtype = float), np.array(y, dtype = float))
    g.SetName("%s"%(name))
    g.SetTitle("%s"%(title))
    g.SetLineColor(color)
    return g

def getVarWithKey(var,var_length, key, tuple_key, datatype):
    x=[]
    for i in range(var_length):
        if key == tuple_key[i]:
            x.append(var[i])
    x=np.array(x,dtype = datatype)
    if len(x) == 0:
        print "No tuple_key was found matching input key"
        return 0
    return x

def newCanvas(name):
        canvas = r.TCanvas("%s"%(name), "c", 1800,800)
        return canvas

def savePNG(canvas,directory,name):
        canvas.Draw()
        canvas.SaveAs("%s/%s.png"%(directory,name))
        canvas.Close()



######################################################################################################
import numpy as np
directory = "/home/alic/src/hpstr/pyplot/baselineFits/fit_data/"
SvtBl2D_file = options.inFilename
inFile = r.TFile(SvtBl2D_file, "READ")
hybrid =options.hybrid

r.gROOT.SetBatch(r.kTRUE)

#Get SvtBl2D histogram keys from input file
histokeys_hh = getHistoKeys(inFile,"TH2", options.hybrid,"")
histo = readhistoFromFile(inFile,histokeys_hh[0])
projy = histo.ProjectionY("%s_proY_ch%i"%(histokeys_hh[0],1),2,2,"e");
binwidth = projy.GetBinWidth(0)
print binwidth
#Plot 1D Histogram of window size
winOut = r.TFile("./fit_data/minwin.root","RECREATE")
winOut.cd()

oglowdaqwin_h = r.TH1F("og_lowdaq_window_size","og_lowdaq_windowsize_distribution;size;events",375,0,1500)
myTree = inFile.gaus_fit
for fitData in myTree:
    if fitData.lowdaq == 1:
        dif = fitData.ogxmax - fitData.ogxmin
        oglowdaqwin_h.Fill(dif,1.)
oglowdaqwin_h.Write()

ogwin_h = r.TH1F("og_window_size","og_windowsize_distribution;size;events",375,0,1500)
myTree = inFile.gaus_fit
for fitData in myTree:
    dif = fitData.ogxmax - fitData.ogxmin
    ogwin_h.Fill(dif,1.)
ogwin_h.SetLineColor(3)
    #if dif > 1000:
     #   print fitData.SvtAna2DHisto_key
      #  print fitData.channel
ogwin_h.Write()

lowdaqwin_h = r.TH1F("final_lowdaq_window_size","final_lowdaq_windowsize_distribution;size;events",375,0,1500)
myTree = inFile.gaus_fit
for fitData in myTree:
    if fitData.lowdaq == 1:
        dif = fitData.BlFitRangeUpper - fitData.BlFitRangeLower
        lowdaqwin_h.Fill(dif,1.)
        if dif > 1000:
            print fitData.SvtAna2DHisto_key
            print fitData.channel
lowdaqwin_h.SetLineColor(6)
lowdaqwin_h.Write()

win_h = r.TH1F("final_window_size","final_windowsize_distribution;size;events",375,0,1500)
myTree = inFile.gaus_fit
for fitData in myTree:
    dif = fitData.BlFitRangeUpper - fitData.BlFitRangeLower
    win_h.Fill(dif,1.)
win_h.SetLineColor(2)
    #if dif > 1000:
    #    print fitData.SvtAna2DHisto_key
    #    print fitData.channel
win_h.Write()


#N sigma value that makes a channel flagged as lowdaq
channel = []
mean = []
sigma = []
norm = []
range_lower = []
range_upper = []
minbinFail = []
for fitData in myTree:
    channel.append(fitData.channel)
    mean.append(fitData.BlFitMean)
    sigma.append(fitData.BlFitSigma)
    norm.append(fitData.BlFitNorm)
    range_lower.append(fitData.BlFitRangeLower)
    range_upper.append(fitData.BlFitRangeUpper)
    minbinFail.append(fitData.minbinFail)

print "Length of Channels is %i"%(len(channel))
lowdaqN_h = r.TH1F("N*Sigma","N*Sigma for LowDaq;N Sigma;events",100,-5,5)
rang = np.linspace(-5,5,500)
for cc in range(len(channel)): 
    #if minbinFail == 1.0:
    #    continue
    for N in rang:
        if range_upper[cc] < mean[cc] + N*sigma[cc]:
            lowdaqN_h.Fill(N)
            break
lowdaqN_h.Write()
    #func = r.TF1("m1","gaus",range_lower[cc],range_upper[cc])
    #func.SetParameter(0,norm[cc])
    #func.SetParameter(2,sigma[cc])
    #func.SetParameter(1,mean[cc])
winOut.Close()

   
