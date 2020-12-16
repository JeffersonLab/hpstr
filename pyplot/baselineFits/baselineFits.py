import ROOT as r
from copy import deepcopy
import os.path
import numpy as np
import ModuleMapper as mmap
import utilities as utils
from optparse import OptionParser



def get_comma_separated_args(option, opt, value, parser):
    setattr(parser.values, option.dest, value.split(','))

parser=OptionParser()

parser.add_option("-g", type="string", dest="show_graphs", help="If == show, print all graphs associate with specified channels", default="")

parser.add_option("-i", type="string", dest="inFilename", help="Input SvtBlFitHistoProcessor output root file",default="")

parser.add_option("-b", type="string", dest="onlineBaselines", help="online baseline fits root file",default="")

parser.add_option("-d", type="string", dest="hpstrpath", help="Path to hpstr",default="")

parser.add_option("-t", type="string", dest="tag", help="file extension tag",default="")

parser.add_option("-s", "--hybrid", type="string", dest="hybrid",
        help="L<#><T/B>_<axial/stereo>_<ele/pos>", default="")

(options, args) = parser.parse_args()

r.gROOT.SetBatch(r.kTRUE)
r.gStyle.SetOptStat(0)
#utils.SetStyle()
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

def buildTGraph(name,title, n_points, x, y,color):
    g = r.TGraph(n_points,np.array(x, dtype = float), np.array(y, dtype = float))
    g.SetName("%s"%(name))
    g.SetTitle("%s"%(title))
    g.SetLineColor(color)
    return g

def newCanvas(name):
        canvas = r.TCanvas("%s"%(name), "c", 1800,800)
        return canvas

def savePNG(canvas,directory,name):
        #canvas.Draw()

        canvas.SaveAs("%s/%s.png"%(directory,name))
        canvas.Close()



######################################################################################################
hpstrpath = options.hpstrpath
#directory = "%s/pyplot/baselineFits/fit_data/"%(hpstrpath)
SvtBl2D_file = options.inFilename
inFile = r.TFile(SvtBl2D_file, "READ")
hybrid =options.hybrid


#Get SvtBl2D histogram keys from input file
histokeys_hh = getHistoKeys(inFile,"TH2", options.hybrid,"")
print histokeys_hh
outFile = r.TFile("%s_analysis.root"%(options.inFilename[:-5]),"RECREATE")
loadonline = False

for key in histokeys_hh:

#######Read online baseline fit values from root file
    if options.onlineBaselines != "":
        loadonline = True
        onBlKey = key.replace('raw_hits_','').replace('_SvtHybrids0_hh','')
        print onBlKey

        if options.onlineBaselines != "":
            hwtag = mmap.str_to_hw(onBlKey)
            print hwtag
            plotname = "baseline_%s_ge"%(hwtag)
            print plotname
            bf_inFile = r.TFile(options.onlineBaselines,"READ")
            tgraph = bf_inFile.Get("baseline/%s"%(plotname))
            bf_inFile.Close()

        bl_ch = tgraph.GetX()
        bl_mean = tgraph.GetY()

#############################################################################

    print key
    sensor = key.replace('raw_hits_','').replace('_hh','')
    histo_hh = readhistoFromFile(inFile, key)
    inFile.cd()

    #Fit Parameters
    histo_key=[]
    channel=[]
    mean=[]
    sigma=[]
    norm = []
    range_lower=[]
    range_upper=[]
    Chi2=[]
    Ndf=[]

    #Flags
    minStatsFailure=[]
    noisy=[]
    lowdaq=[]

    ##If 2D histogram is empty, skip histogram
    if histo_hh.GetEntries() == 0:
        continue

    #Get variables from Flat Tuple stored in input file
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
            Chi2.append(fitData.BlFitChi2)
            Ndf.append(fitData.BlFitNdf)
            minStatsFailure.append(fitData.minStatsFailure)
            noisy.append(fitData.noisy)
            lowdaq.append(fitData.lowdaq)

    #Output ROOT FILE
    #outFile = r.TFile(directory+"/%s_fit_analysis%s.root"%(key[:-3],options.tag), "RECREATE")
    outFile.cd()

    #Plot gaus Fit mean of all channels over 2D Histogram
    canvas = r.TCanvas("%s_mean"%(sensor), "c", 1800,800)
    canvas.cd()
    canvas.SetTicky()
    canvas.SetTickx()
    mean_gr_x = np.array(channel, dtype = float)
    mean_gr_y = np.array(mean, dtype=float)
    mean_gr = buildTGraph("BlFitMean_%s"%(sensor),"BlFitMean_%s;Channel;ADC"%(sensor),len(mean_gr_x),mean_gr_x,mean_gr_y,2)

    if loadonline == True:
        bl_gr_x = np.array(bl_ch, dtype = float)
        bl_gr_y = np.array(bl_mean, dtype = float)
        bl_gr = buildTGraph("onlineBlFits_%s"%(sensor),"Online Baseline Fits_%s;Channel;ADC"%(sensor),len(bl_gr_x),bl_gr_x,bl_gr_y,1)

    lowdaq_gr_x = np.array(channel, dtype = float)
    lowdaq_gr_y = np.array([3500.0 * x for x in lowdaq], dtype=float)
    lowdaq_gr = buildTGraph("lowdaqflag_%s"%(sensor),"low-daq channels_%s;Channel;ADC"%(sensor),len(lowdaq_gr_x),lowdaq_gr_x, lowdaq_gr_y,utils.colors[2])

    histo_hh.GetYaxis().SetRangeUser(3000.0,7500.0)
    if histo_hh.GetName().find("L0") != -1 or histo_hh.GetName().find("L1") != -1:
        histo_hh.GetXaxis().SetRangeUser(0.0,512.0)
    histo_hh.Draw("colz")
    mean_gr.Draw("same")

    if loadonline == True:
        bl_gr.Draw("same")

    lowdaq_gr.SetMarkerStyle(8)
    lowdaq_gr.SetMarkerSize(1)
    lowdaq_gr.SetMarkerColor(utils.colors[10])
    lowdaq_gr.Draw("psame")

    legend = r.TLegend(0.1,0.75,0.28,0.9)
    legend.AddEntry(mean_gr,"offline baselines using hpstr processor","l")
    if loadonline == True:
        legend.AddEntry(bl_gr,"online baseline fits","l")
    legend.AddEntry(lowdaq_gr,"low-daq threshold","p")
    legend.Draw()
    canvas.Write()
    savePNG(canvas,".","%s_gausFit"%(key))
    canvas.Close()
    
    if loadonline == True:
        #Plot Difference between online and offline baselines
        canvas = r.TCanvas("%s_diff"%(sensor), "c", 1800,800)
        canvas.cd()
        canvas.SetTicky()
        canvas.SetTickx()
        diff_ch = []
        diff_mean = []
        diff_bl_mean = []
        
        for i in range(len(channel)):
            if mean[i] > 0:
                diff_ch.append(channel[i])
                diff_mean.append(mean[i])
                diff_bl_mean.append(bl_mean[i])

        diff_gr_x = np.array(diff_ch, dtype = float)
        diff_gr_y = np.array([x - y for x,y in zip(diff_mean,diff_bl_mean)], dtype=float)
        diff_gr = buildTGraph("FitDiff_%s"%(sensor),"(Offline - Online) BlFit Mean_%s;Channel;ADC Difference"%(sensor),len(diff_gr_x),diff_gr_x,diff_gr_y,1)

        diff_gr.GetYaxis().SetRangeUser(-400.0,400.0)
        if sensor.find("L0") != -1 or sensor.find("L1") != -1:
            diff_gr.GetXaxis().SetRangeUser(0.0,512.0)
        else:
            diff_gr.GetXaxis().SetRangeUser(0.0,640.0)

        lowdaq_gr_x = np.array(channel, dtype = float)
        lowdaq_gr_y = np.array([-1000.0 + 600.0* x for x in lowdaq], dtype=float)
        lowdaq_gr = buildTGraph("lowdaqflag_%s"%(sensor),"low-daq channels_%s;Channel;ADC"%(sensor),len(lowdaq_gr_x),lowdaq_gr_x, lowdaq_gr_y,utils.colors[2])

        diff_gr.Draw()

        lowdaq_gr.SetMarkerStyle(8)
        lowdaq_gr.SetMarkerSize(1)
        lowdaq_gr.SetMarkerColor(utils.colors[10])
        lowdaq_gr.Draw("psame")

        canvas.Write()
        savePNG(canvas,".","%s_fitdiff"%(key))
        canvas.Close()

    #1D Histogram of channel fit sigma
    sigma_h = r.TH1F("Fit_Sigma_%s"%(sensor),"Sigma_Distribution_%s;sigma;events"%(sensor),len(sigma),0.,max((sigma)))
    for i in range(len(sigma)):
        sigma_h.Fill(sigma[i],1.)
    sigma_h.Write()


    #Plot lowdaq
    ld_gr_y = np.array(lowdaq, dtype= float)
    ld_gr_x = np.array(channel, dtype=float)
    ld_gr = buildTGraph("lowdaq_flags_%s"%(sensor),"Low_Daq_Threshold_%s;Channel;(1 = lowdaq)"%(sensor),len(ld_gr_x),ld_gr_x,ld_gr_y,1)
    ld_gr.Draw()
    ld_gr.Write()

    #Plot Chi2/Ndf
    Chi2Ndf = [i / j for i, j in zip(Chi2,Ndf)]
    chi2_gr_y = np.array(Chi2Ndf, dtype= float)
    chi2_gr_x = np.array(channel, dtype=float)
    chi2_gr = buildTGraph("BlFitChi2_%s"%(sensor),"BlFit_Chi2/Ndf_%s;Channel;Status"%(sensor),len(chi2_gr_x),chi2_gr_x,chi2_gr_y,1)
    chi2_gr.Draw()
    chi2_gr.Write()

    ######################################################################################################
    ###Show Channel Fits
    cfdir = outFile.mkdir("%s_channel_fits"%(sensor))
    cfdir.cd()
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
    cfdir.Close()

#########################################################################################################
    ###Show Channel Graphs
    cgdir = outFile.mkdir("%s_channel_graphs"%(sensor))
    cgdir.cd()
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
    cgdir.Close() 

