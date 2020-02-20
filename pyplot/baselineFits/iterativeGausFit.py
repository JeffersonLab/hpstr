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

    iterativeFit_chi2_NDF=[]
    iterativeFit_range_end=[]
    iterativeFit_mean=[]
    iterativeFit_chi2_2ndDerivative=[]
    iterativeFit_chi2_2Der_range=[]

    myTree = inFile.gaus_fit
    for fitData in myTree:
        SvtAna2DHisto_key = str(fitData.SvtAna2DHisto_key)
        if key == SvtAna2DHisto_key+"_hh":
            histo_key.append(SvtAna2DHisto_key)
            channel.append(fitData.channel)
            mean.append(fitData.baseline_gausFit_mean)
            sigma.append(fitData.baseline_gausFit_sigma)
            norm.append(fitData.baseline_gausFit_norm)
            range_lower.append(fitData.baseline_gausFit_range_lower)
            range_upper.append(fitData.baseline_gausFit_range_upper)

            iterativeFit_mean.append(fitData.iterativeFit_mean)
            iterativeFit_chi2_2ndDerivative.append(fitData.iterativeFit_chi2_2ndDerivative)
            iterativeFit_chi2_2Der_range.append(fitData.iterativeFit_chi2_2Der_range)
            iterativeFit_chi2_NDF.append(fitData.iterativeFit_chi2_NDF)
            iterativeFit_range_end.append(fitData.iterativeFit_range_end)
    return channel,mean, sigma, histo_key,norm, range_lower, range_upper, iterativeFit_mean, iterativeFit_chi2_2ndDerivative, iterativeFit_chi2_2Der_range, iterativeFit_chi2_NDF, iterativeFit_range_end
  

               
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
channels_in = options.channels
if options.channels is not None:
    if options.channels == "all":
        channels_in = range(0,640)
    else:
        channels_in = [int(i) for i in options.channels]
        for i in range(len(channels_in)):
            if i > 0:
                temp = range(channels_in[i-1],channels_in[i])
                print temp
        channels_in=[]
        for cc in temp:
            channels_in.append(cc)

#Get SvtBl2D histogram keys from input file
histokeys_hh = getHistoKeys(inFile,"TH2", options.hybrid,"")
print histokeys_hh


for key in histokeys_hh:

    histo_hh = readhistoFromFile(inFile, key)
    print key
    if histo_hh.GetEntries() == 0:
        continue
    #Get flat_tuple variables from inFile
    channel, mean, sigma, histo_key,norm, range_lower, range_upper, iterativeFit_mean, iterativeFit_chi2_2ndDerivative, iterativeFit_2Der_range, iterativeFit_chi2_NDF, iterativeFit_range_end = getGausFitParameters(inFile,key)

    outFile = r.TFile(directory+"%s_fit_analysis.root"%(key[:-3]), "RECREATE")
    outFile.cd()
    #Plot gaus Fit mean of all channels over 2D Histogram
    fit_gr_x = np.array(channel, dtype = float)
    fit_gr_y = np.array(mean, dtype=float)
    fit_gr = buildTGraph("baseline_gaus_fit_mean_%s"%(key),"baseline_gaus_fit_mean_%s;Channel;ADC"%(key),len(fit_gr_x),fit_gr_x,fit_gr_y,2)

    #Save PNG of 2D Histogram with Channel Mean ADC overlayed    
    canvas = r.TCanvas("%s"%(key), "c", 1800,800)
    canvas.cd()
    histo_hh.Draw("colz")
    fit_gr.Draw("same")
    canvas.Write()
    savePNG(canvas,directory+"hybrid_fits/","baseline_gausFit_%s"%(key))


    ###Show Channel Fits
    if options.show_fits == "show" and hybrid == "":
        print ""
        print "ERROR!"
        print "Must specify hyrid to show channel fits"
        print ""
    elif options.show_fits == "show" and hybrid != "":
        for cc in channels_in:
            canvas = r.TCanvas("%s_ch_%i_h"%(key[:-3],cc), "c", 1800,800)
            canvas.cd()

            yproj_h = histo_hh.ProjectionY('%s_ch%i_h'%(key[:-3],cc),cc+1,cc+1,"e")

            #func = r.TF1("cc_fit", "[norm] * ROOT::Math::normal_pdf(x, [sigma], [mean])",range_lower[cc],range_upper[cc]); 
            func = r.TF1("m1","gaus",range_lower[cc],range_upper[cc])
            func.SetParameter(0,norm[cc])
            func.SetParameter(2,sigma[cc])
            func.SetParameter(1,mean[cc])

            yproj_h.SetTitle("%s_ch_%i_h"%(key[:-3],cc))
            yproj_h.Draw()
            func.Draw("same")
            canvas.Write()
            savePNG(canvas,directory+"channel_fits/","baseline_gausFit_%s_ch_%i"%(key[:-3],cc))



    ###Show Channel Graphs
    if options.show_graphs == "show" and hybrid == "":
        print "ERROR! PLEASE SPECIFY HYBRID TO SHOW CHANNEL GRAPHS"
    elif options.show_graphs == "show" and hybrid != "":
        for cc in channels_in:
            canvas = r.TCanvas("mean_%s_ch_%i"%(key,cc), "c", 1800,800)
            canvas.cd()
            mean_gr = buildTGraph("iterative_fit_mean_%s_ch_%i"%(key,cc),"iterative_fit_mean_vs_position_%s_ch_%i;FitRangeEnd;mean"%(key,cc),len(iterativeFit_range_end[cc]),np.array(iterativeFit_range_end[cc], dtype = float) ,np.array(iterativeFit_mean[cc], dtype = float),1)
            mean_gr.Draw()
            canvas.Write()
            #savePNG(canvas,directory+"channel_fits/","iterative_fit_mean_%s_ch_%i"%(key,cc))

            canvas = r.TCanvas("chi2_%s_ch_%i"%(key,cc), "c", 1800,800)
            canvas.cd()
            chi2_gr = buildTGraph("iterative_fit_chi2_%s_ch_%i"%(key,cc),"iterative_fit_chi2_vs_position_%s_ch_%i;FitRangeEnd;chi2"%(key,cc),len(iterativeFit_range_end[cc]),np.array(iterativeFit_range_end[cc], dtype = float) ,np.array(iterativeFit_chi2_NDF[cc], dtype = float),1)
            chi2_gr.Draw()
            canvas.Write()
            #savePNG(canvas,directory+"channel_fits/","iterative_fit_chi2_%s_ch_%i"%(key,cc))

            canvas = r.TCanvas("chi2_2Der_%s_ch_%i"%(key,cc), "c", 1800,800)
            canvas.cd()
            chi2_2Der_gr = buildTGraph("iterative_fit_chi2_2Der_%s_ch_%i"%(key,cc),"iterative_fit_chi2_2Der_vs_position_%s_ch_%i;FitRangeEnd;chi2_2ndDeriv"%(key,cc),len(iterativeFit_2Der_range[cc]),np.array(iterativeFit_2Der_range[cc], dtype = float) ,np.array(iterativeFit_chi2_2ndDerivative[cc], dtype = float),1)
            chi2_2Der_gr.Draw()
            canvas.Write()
            #savePNG(canvas,directory+"channel_fits/","iterative_fit_chi2_2nd_Der%s_ch_%i"%(key,cc))
    outFile.Close()



