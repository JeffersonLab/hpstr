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

    iterChi2NDF=[]
    iterFitRangeEnd=[]
    iterMean=[]
    iterChi2NDF_2der=[]
    iterChi2NDF_2derRange=[]

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

            iterMean.append(fitData.iterMean)
            iterChi2NDF_2der.append(fitData.iterChi2NDF_2der)
            iterChi2NDF_2derRange.append(fitData.iterChi2NDF_2derRange)
            iterChi2NDF.append(fitData.iterChi2NDF)
            iterFitRangeEnd.append(fitData.iterFitRangeEnd)

    return channel,mean, sigma, histo_key,norm, range_lower, range_upper, iterMean, iterChi2NDF_2der, iterChi2NDF_2derRange, iterChi2NDF, iterFitRangeEnd
  

               
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
directory = "/home/alic/src/hpstr/pyplot/baselineFits/fit_images/"
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
    channels_in= range(0,int(histo_hh.GetNbinsX()))
    print key
    if histo_hh.GetEntries() == 0:
        continue
    #Get flat_tuple variables from inFile
    channel, mean, sigma, histo_key,norm, range_lower, range_upper, iterMean, iterChi2NDF_2der, iterativeFit_2Der_range, iterChi2NDF, iterFitRangeEnd = getGausFitParameters(inFile,key)

    #Output ROOT FILE
    outFile = r.TFile(directory+"%s_fit_analysis.root"%(key[:-3]), "RECREATE")
    outFile.cd()

    #Plot gaus Fit mean of all channels over 2D Histogram
    mean_gr_x = np.array(channel, dtype = float)
    mean_gr_y = np.array(mean, dtype=float)
    mean_gr = buildTGraph("baseline_gaus_fit_mean_%s"%(key),"baseline_gaus_fit_mean_%s;Channel;ADC"%(key),len(mean_gr_x),mean_gr_x,mean_gr_y,2)

    #Save PNG of 2D Histogram with Channel Mean ADC overlayed    
    canvas = r.TCanvas("%s_mean"%(key), "c", 1800,800)
    canvas.cd()
    histo_hh.Draw("colz")
    mean_gr.Draw("same")
    canvas.Write()
    canvas.Close()
    #savePNG(canvas,directory+"hybrid_fits/","baseline_gausFit_%s"%(key))

    #1D Histogram of channel fit sigma
    fit_sigma = np.array(sigma, dtype= float)
    fit_channel = np.array(channel, dtype=float)
    sigma_h = r.TH1F("Fit_Sigma_%s"%(key),"Channel_Fit_Sigma_Distribution_%s;sigma;events"%(key),len(fit_sigma),0.,max((sigma)))

    for i in range(len(sigma)):
        sigma_h.Fill(sigma[i],1.)
    sigma_h.Write()

    
    #Plot gaus Fit mean+sigma of all channels over 2D Histogram
    sig_gr_x = np.array(channel, dtype = float)
    sig_gr_y = np.array(sigma, dtype=float)
    #mean_sig = [mean[i] + sigma[i] for i in range(len(mean))]
    #mean_sig_y = np.array(mean_sig, dtype = float)
    sig_gr = buildTGraph("baseline_gaus_fit_sigma_%s"%(key),"baseline_gaus_fit_sigma_%s;Channel;sigma"%(key),len(sig_gr_x),sig_gr_x,sig_gr_y,3)
    sig_gr.Draw()
    sig_gr.Write()

    canvas = r.TCanvas("%s_sigma"%(key), "c", 1800,800)
    canvas.cd()
    histo_hh.Draw("colz")
    sig_gr.Draw("same")
    mean_gr.Draw("same")
    canvas.Write()
    canvas.Close()
    #savePNG(canvas,directory+"hybrid_fits/","baseline_gausFit_%s"%(key))


    #If Channel SIGMA > Value determined by sigma distribution
    noisey_ch = [i for i, j in zip(channel,sigma) if j >= 85]
    print noisey_ch


    ######################################################################################################
    ###Show Channel Fits

    for cc in channels_in:

        canvas = r.TCanvas("%s_ch_%i_h"%(key[:-3],cc), "c", 1800,800)
        canvas.cd()
        yproj_h = histo_hh.ProjectionY('%s_ch%i_h'%(key[:-3],cc),cc+1,cc+1,"e")
        if yproj_h.GetEntries() == 0:
            continue
        #func = r.TF1("cc_fit", "[norm] * ROOT::Math::normal_pdf(x, [sigma], [mean])",range_lower[cc],range_upper[cc]); 
        func = r.TF1("m1","gaus",range_lower[cc],range_upper[cc])
        func.SetParameter(0,norm[cc])
        func.SetParameter(2,sigma[cc])
        func.SetParameter(1,mean[cc])

        yproj_h.SetTitle("%s_ch_%i_h"%(key[:-3],cc))
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
                iterMean=(fitData.iterMean)
                #iterChi2NDF_2der=(fitData.iterChi2NDF_2der)
                iterChi2NDF_2derRange=(fitData.iterChi2NDF_2derRange)
                iterChi2NDF=(fitData.iterChi2NDF)
                der=[]
                for i in range(len(iterChi2NDF)):
                    if (i>3 and i < len(iterChi2NDF) - 3):
                        slope2 = (iterChi2NDF[i+3] - iterChi2NDF[i])/(3*bw)
                        slope1 = (iterChi2NDF[i] - iterChi2NDF[i-3])/(3*bw)
                        slopeDiff = slope2 - slope1
                        der.append(slopeDiff)
                    
                iterFitRangeEnd=(fitData.iterFitRangeEnd)
                mean_gr = buildTGraph("iterative_fit_mean_%s_ch_%i"%(key,cc),"iterative_fit_mean_vs_position_%s_ch_%i;FitRangeEnd;mean"%(key,cc),len(fitData.iterFitRangeEnd),np.array(fitData.iterFitRangeEnd, dtype = float) ,np.array(fitData.iterMean, dtype = float),1)

                chi2_gr = buildTGraph("iterative_fit_chi2_%s_ch_%i"%(key,cc),"iterative_fit_chi2/NDF_vs_position_%s_ch_%i;FitRangeEnd;chi2"%(key,cc),len(fitData.iterFitRangeEnd),np.array(fitData.iterFitRangeEnd, dtype = float) ,np.array(fitData.iterChi2NDF, dtype = float),1)

                chi2_2Der_gr = buildTGraph("iterative_fit_chi2_2Der_%s_ch_%i"%(key,cc),"iterative_fit_chi2_2Der_vs_position_%s_ch_%i;FitRangeEnd;chi2_2ndDeriv"%(key,cc),len(fitData.iterChi2NDF_2derRange),np.array(fitData.iterChi2NDF_2derRange, dtype = float) ,np.array(der, dtype = float),1)
                mean_gr.Write()
                chi2_gr.Write()
                chi2_2Der_gr.Write()

