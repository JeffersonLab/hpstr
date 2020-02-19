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

parser.add_option("-s", "--hybrid", type="string", dest="hybrid",
        help="L<#><T/B>_<axial/stereo>_<ele/pos>", default="")

(options, args) = parser.parse_args()
baseline= "baseline0"
hybrid =options.hybrid

def gethistos2DKeys(inFile, hybrid):
    inFile.cd()
    histos2D=[]
    for key in inFile.GetListOfKeys():
        kname = key.GetName()
        classType = key.GetClassName()
        if classType.find("TH2") != -1 and kname.find(hybrid) != -1:
            histos2D.append(kname)
    return histos2D
              
def readhistoFromFile(inFile, histo_key):
    histo = deepcopy(getattr(inFile, histo_key))
    return histo

def getChannelFitsPlots(inFile,channels):
    inFile.cd()


def getGausFitParameters(inFile):
    inFile.cd()
    channel=[]
    mean=[]
    sigma=[]
    histo_key=[]
    myTree = inFile.gaus_fit
    for fitData in myTree:
        SvtAna2DHisto_key = str(fitData.SvtAna2DHisto_key)
        channel.append(fitData.channel)
        mean.append(fitData.baseline_gausFit_mean)
        #baseline = fitData.baseline_gausFit_norm
        sigma.append(fitData.baseline_gausFit_sigma)
        histo_key.append(SvtAna2DHisto_key[:-2])
    return channel,mean, sigma, histo_key

def getChannelFitGraphs(inFile):
    inFile.cd()
    channel=[]
    histo_key=[]
    iterativeFit_chi2_NDF=[]
    iterativeFit_range_end=[]
    iterativeFit_mean=[]
    iterativeFit_chi2_2ndDerivative=[]
    iterativeFit_chi2_2Der_range=[]
    myTree = inFile.gaus_fit
    for fitData in myTree:
        SvtAna2DHisto_key = str(fitData.SvtAna2DHisto_key)
        channel.append(fitData.channel)
        iterativeFit_mean.append(fitData.iterativeFit_mean)
        iterativeFit_chi2_2ndDerivative.append(fitData.iterativeFit_chi2_2ndDerivative)
        iterativeFit_chi2_2Der_range.append(fitData.iterativeFit_chi2_2Der_range)
        iterativeFit_chi2_NDF.append(fitData.iterativeFit_chi2_NDF)
        iterativeFit_range_end.append(fitData.iterativeFit_range_end)
        histo_key.append(SvtAna2DHisto_key[:-2])
    return channel,histo_key,iterativeFit_mean, iterativeFit_chi2_2ndDerivative, iterativeFit_2Der_range, iterativeFit_chi2_NDF, iterativeFit_range_end
        
def getChannelsFitKeys(inFile, channels, hybrid):
    inFile.cd()
    channel_fit_keys=[]
    for key in inFile.GetListOfKeys():
        kname = key.GetName()
        classType = key.GetClassName()
        for e in channels:
            #if kname.find("_"+str(e)) != -1 and kname.find(hybrid) != -1:
            if kname.rsplit('_',1)[1] == str(e) and kname.find(hybrid) != -1:
                channel_fit_keys.append(kname)
    return channel_fit_keys
                


######################################################################################################
SvtBl2D_file = options.inFilename
#out_File = '%s_iterativeFit_.root'%(SvtBl2D_file[:-5], channel)
#print out_File

inFile = r.TFile(SvtBl2D_file, "READ")
#outFile = r.TFile(out_File, "RECREATE")
histokeys_hh = gethistos2DKeys(inFile, options.hybrid)

r.gROOT.SetBatch(r.kTRUE)

channels_in = options.channels
if options.channels is not None:
    channels_in = [int(i) for i in options.channels]
    print channels_in


for k in histokeys_hh:
    histo_hh = readhistoFromFile(inFile, k)
    print k
    channel, mean, sigma, histo_key = getGausFitParameters(inFile)
    n = len(channel)
    m =0
    x=[]
    y=[]
    for i in range(n):
        if k == histo_key[i]+"_hh":
            m = m + 1
            x.append(channel[i])
            y.append(mean[i])
    if len(x) == 0 or len(y) == 0:
        print "No matching fit results were found for %s"%(k)
        break

    fit_gr = r.TGraph(m,np.array(x, dtype = float), np.array(y, dtype = float))
    fit_gr.SetName("baseline_gaus_fit_mean_%s"%(k))
    fit_gr.SetTitle("baseline_gaus_fit_mean_%s;Channel;ADC"%(k))

    canvas = r.TCanvas("%s"%(k), "c", 1800,800)
    canvas.cd()
    histo_hh.Draw("colz")
    fit_gr.SetLineColor(2)
    fit_gr.Draw("same")
    canvas.Draw()
    canvas.SaveAs("/home/alic/src/hpstr/pyplot/baselineFits/fit_images/hybrid_fits/baseline_gausFit_%s.png"%(k))
    canvas.Close()


    ###Show Channel Fits
    if options.show_fits == "show" and hybrid == "":
        print ""
        print "ERROR!"
        print "Must specify hyrid to show channel fits"
        print ""
    elif options.show_fits == "show" and hybrid != "":
        channel_fit_keys = getChannelsFitKeys(inFile,channels_in, hybrid)
        for e in channel_fit_keys:
            c_fit_h = readhistoFromFile(inFile,e)
            print e
            ch_canvas = r.TCanvas("%s_%s"%(k,e),"c", 1800,800)
            ch_canvas.cd()
            c_fit_h.Draw()
            ch_canvas.Draw()
            ch_canvas.SaveAs("/home/alic/src/hpstr/pyplot/baselineFits/fit_images/channel_fits/%s.png"%(e))
            ch_canvas.Close()




    ###Show Channel Graphs
    gmean=[]
    gchi2NDF=[]
    gfit_xmax=[]
    gchi22D=[]
    gchi22D_xmax=[]

    if options.show_graphs == "show" and hybrid == "":
        print "ERROR! PLEASE SPECIFY HYBRID TO SHOW CHANNEL GRAPHS"
    elif options.show_graphs == "show" and hybrid != "":
        channel,histo_key,iterativeFit_mean, iterativeFit_chi2_2ndDerivative, iterativeFit_2Der_range, iterativeFit_chi2_NDF, iterativeFit_range_end = getChannelFitGraphs(inFile)
        for e in channels_in:
            for i in range(len(channel)):
                if k == histo_key(i)+"_hh" and i==e:
                    break
        

        

