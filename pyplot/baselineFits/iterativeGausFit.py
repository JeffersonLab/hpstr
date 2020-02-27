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

parser.add_option("-L", type="string", dest="layer", help="layer subdirectory",default="")

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


def getGausFitParameters(inFile, key):
    inFile.cd()
    histo_key=[]
    channel=[]
    mean=[]
    sigma=[]
    norm = []
    range_lower=[]
    range_upper=[]

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
directory = "/home/alic/src/hpstr/pyplot/baselineFits/fit_data/"
SvtBl2D_file = options.inFilename
inFile = r.TFile(SvtBl2D_file, "READ")
hybrid =options.hybrid

r.gROOT.SetBatch(r.kTRUE)
#Read in channels to be examined
#channels_in = options.channels
#if options.channels is not None:
#    if options.channels == "all":
#        channels_in = range(0,640)
#    else:
#        channels_in = [int(i) for i in options.channels]
#        for i in range(len(channels_in)):
#            if i > 0:
#                temp = range(channels_in[i-1],channels_in[i])
#                print temp
#        channels_in=[]
#        for cc in temp:
#            channels_in.append(cc)

#Get SvtBl2D histogram keys from input file
histokeys_hh = getHistoKeys(inFile,"TH2", options.hybrid,"")
print histokeys_hh


for key in histokeys_hh:

    histo_hh = readhistoFromFile(inFile, key)
    print key
    sensor= key.replace('raw_hits_','').replace('_hh','')
    channels_in= range(0,int(histo_hh.GetNbinsX()))

    ##If 2D histogram is empty, skip histogram
    if histo_hh.GetEntries() == 0:
        continue

    #Get baseline fit parameters from TTree
    channel, mean, sigma, histo_key,norm, range_lower, range_upper = getGausFitParameters(inFile,key)
    #Output ROOT FILE
    outFile = r.TFile(directory+options.layer+"/%s_fit_analysis.root"%(key[:-3]), "RECREATE")
    outFile.cd()

    #Plot gaus Fit mean of all channels over 2D Histogram
    canvas = r.TCanvas("%s_mean"%(sensor), "c", 1800,800)
    canvas.cd()
    mean_gr_x = np.array(channel, dtype = float)
    mean_gr_y = np.array(mean, dtype=float)
    mean_gr = buildTGraph("BlFitMean_%s"%(sensor),"BlFitMean_%s;Channel;ADC"%(sensor),len(mean_gr_x),mean_gr_x,mean_gr_y,2)

    histo_hh.Draw("colz")
    mean_gr.Draw("same")
    canvas.Write()
    savePNG(canvas,directory+"hybrid_fits/","%s_gausFit"%(key))
    canvas.Close()

    #1D Histogram of channel fit sigma
    sigma_h = r.TH1F("Fit_Sigma_%s"%(sensor),"Sigma_Distribution_%s;sigma;events"%(sensor),len(sigma),0.,max((sigma)))
    for i in range(len(sigma)):
        sigma_h.Fill(sigma[i],1.)
    sigma_h.Write()

    
    #Plot gaus Fit mean+sigma of all channels over 2D Histogram
    canvas = r.TCanvas("%s_sigma"%(sensor), "c", 1800,800)
    canvas.cd()

    sigma_gr_y = np.array(sigma, dtype= float)
    sigma_gr_x = np.array(channel, dtype=float)
    sigma_gr = buildTGraph("BlFit_sigma_%s"%(sensor),"BlFit_sigma_%s;Channel;Sigma"%(sensor),len(sigma_gr_x),sigma_gr_x,sigma_gr_y,3)
    sigma_gr.Draw()
    sigma_gr.Write()

    histo_hh.Draw("colz")
    sigma_gr.Draw("same")
    mean_gr.Draw("same")
    canvas.Write()
    canvas.Close()
    #savePNG(canvas,directory+"hybrid_fits/","baseline_gausFit_%s"%(key))



    ######################################################################################################
    ###Show Channel Fits
    for cc in range(len(channel)): 
        canvas = r.TCanvas("%s_ch_%i_h"%(sensor,channel[cc]), "c", 1800,800)
        canvas.cd()
        yproj_h = histo_hh.ProjectionY('%s_ch%i_h'%(sensor,channel[cc]),int(channel[cc]+1),int(channel[cc]+1),"e")
        if yproj_h.GetEntries() == 0:
            continue
        func = r.TF1("m1","gaus",range_lower[cc],range_upper[cc])
        func.SetParameter(0,norm[cc])
        func.SetParameter(2,sigma[cc])
        func.SetParameter(1,mean[cc])

        yproj_h.SetTitle("%s_ch_%i_h"%(sensor,channel[cc]))
        yproj_h.Draw()
        func.Draw("same")
        canvas.Write()
        canvas.Close()
        #savePNG(canvas,directory+"channel_fits/","baseline_gausFit_%s_ch_%i"%(key[:-3],cc))

#########################################################################################################
    ###Show Channel Graphs
    bw=histo_hh.GetXaxis().GetBinWidth(1)
    if options.show_graphs == "show":
        myTree = inFile.gaus_fit
        for fitData in myTree:
            SvtAna2DHisto_key = str(fitData.SvtAna2DHisto_key)
            if key == SvtAna2DHisto_key+"_hh":
                cc=(fitData.channel)

                mean_gr = buildTGraph("iterMean_%s_ch_%i"%(sensor,cc),"iterMean_vs_Position_%s_ch_%i;FitRangeEnd;mean"%(sensor,cc),len(fitData.iterFitRangeEnd),np.array(fitData.iterFitRangeEnd, dtype = float) ,np.array(fitData.iterMean, dtype = float),1)

                chi2_gr = buildTGraph("iterChi2_%s_ch_%i"%(sensor,cc),"iterChi2/NDF_vs_Position_%s_ch_%i;FitRangeEnd;chi2"%(sensor,cc),len(fitData.iterFitRangeEnd),np.array(fitData.iterFitRangeEnd, dtype = float) ,np.array(fitData.iterChi2NDF, dtype = float),1)
                chi2_2Der_gr = buildTGraph("iterFit_chi2/NDF_2Der_%s_ch_%i"%(sensor,cc),"iterChi2_2Der_%s_ch_%i;FitRangeEnd;chi2_2ndDeriv"%(sensor,cc),len(fitData.iterChi2NDF_derRange),np.array(fitData.iterChi2NDF_derRange, dtype = float) ,np.array(fitData.iterChi2NDF_2der, dtype = float),1)

                chi2_1Der_gr = buildTGraph("iterFit_chi2/NDF_1Der_%s_ch_%i"%(sensor,cc),"iterChi2_1Der_%s_ch_%i;FitRangeEnd;chi2_1ndDeriv"%(sensor,cc),len(fitData.iterChi2NDF_derRange),np.array(fitData.iterChi2NDF_derRange, dtype = float) ,np.array(fitData.iterChi2NDF_1der, dtype = float),1)

                temp=fitData.iterChi2NDF
                ratio = [i / j for i,j in zip(fitData.iterChi2NDF_2der,temp[3:])]

                ratio_gr = buildTGraph("ratio_%s_ch_%i"%(sensor,cc),"ratio_2Der/Chi2_%s_ch_%i;FitRangeEnd;chi2NDF_2der/chi2NDF"%(sensor,cc),len(fitData.iterChi2NDF_derRange),np.array(fitData.iterChi2NDF_derRange, dtype = float) ,np.array(ratio, dtype = float),1)


                mean_gr.Write()
                chi2_gr.Write()
                chi2_1Der_gr.Write()
                chi2_2Der_gr.Write()
                ratio_gr.Write()
                

