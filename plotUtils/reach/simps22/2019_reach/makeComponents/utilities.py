from optparse import OptionParser
import ROOT as r
from array import array
from copy import deepcopy
import os
import sys
import glob

colors = [r.kBlue+2, r.kCyan+2, r.kRed+2, r.kOrange+10, r.kYellow+2, r.kGreen-1, r.kAzure-2, r.kGreen-8, r.kOrange+3, r.kYellow+2, r.kRed+2, r.kBlue+2, r.kGreen-8, r.kOrange+3, r.kYellow+2, r.kRed+2, r.kBlue+2, r.kGreen-8, r.kOrange+3, r.kYellow+2, r.kRed+2, r.kBlue+2, r.kGreen-8, r.kOrange+3, r.kYellow+2, r.kRed+2, r.kBlue+2, r.kGreen-8, r.kOrange+3]
markers = [r.kFullCircle, r.kFullTriangleUp, r.kFullSquare, r.kOpenSquare, r.kOpenTriangleUp, r.kOpenCircle, r.kFullCircle, r.kOpenSquare, r.kFullSquare, r.kOpenTriangleUp, r.kOpenCircle, r.kFullCircle, r.kOpenSquare, r.kFullSquare, r.kOpenTriangleUp, r.kOpenCircle, r.kFullCircle, r.kOpenSquare, r.kFullSquare, r.kOpenTriangleUp, r.kOpenCircle, r.kFullCircle, r.kOpenSquare, r.kFullSquare, r.kOpenTriangleUp, r.kOpenCircle, r.kFullCircle, r.kOpenSquare, r.kFullSquare, r.kOpenTriangleUp]

fillColors = [r.kRed-6+3, r.kAzure-4, r.kYellow+2]
blueColors = [r.kBlue, r.kBlue+1, r.kBlue+2]

#General configuration

bottomFraction = 0.4
bottomScale = 1./bottomFraction
topScale = 1./(1. - bottomFraction)
r.TProfile.Approximate(True)


parser = OptionParser()
parser.add_option("-f", "--inputFiles", dest="inputFiles", help="Space separated list of files", metavar="inputFiles", default="")
parser.add_option("-o", "--outdir", dest="outdir", help="outdir", metavar="outdir", default="")
parser.add_option("-i", "--indir", dest="indir", help="indir", metavar="indir", default="")
parser.add_option("-r", "--runNumber", dest="runNumber", help="runNumber", metavar="runNumber", default="")
parser.add_option("-s", "--selection", dest="selection", metavar="selection", default="")
parser.add_option("-l", "--Legends", dest="Legends", metavar="Legends", default="")
parser.add_option("-d", "--debug", dest="debug", action="store_true", help="Debug flag", metavar="debug", default=False)


#(config,sys.argv[1:]) = parser.parse_args(sys.argv[1:])


#Get a plot from a directory+file name
def getPlot(loc, fin, plot):
    print("Getting {}".format(plot))
    f = r.TFile.Open(loc+fin)
    histo = f.Get(plot)
    print(histo)
    histo.SetDirectory(0)

    return histo


#Get a plot from a file
def getPlot(fullpath, plot):
    print("Getting {} from {}".format(plot, fullpath))
    f = r.TFile.Open(fullpath)
    histo = f.Get(plot)
    print(histo)
    histo.SetDirectory(0)

    return histo


#Pass a list of files
def MakeHistoListFromFiles(listOfFiles, path, histoName):

    histolist = []
    for infile in listOfFiles:
        f = r.TFile.Open(infile)
        print(f)
        h = f.Get(path+histoName)
        print(path+histoName)
        print(h)
        h.SetDirectory(0)
        histolist.append(h)
    return histolist

#Pass a list of histogram names


def MakeHistoListFromSameFile(infile, path, histoNames):
    histolist = []
    for h_name in histoNames:
        print(h_name)
        f = r.TFile.Open(infile)
        print(f)

        h = f.Get(path+"/"+h_name)
        print(h)
        h.SetDirectory(0)
        histolist.append(h)
    return histolist


def InsertText(runNumber="", texts=[], line=0.87, xoffset=0.18, Hps=True, Colors=False):

    newline = 0.06

    text = r.TLatex()
    text.SetNDC()
    text.SetTextFont(42)
    text.SetTextSize(0.05)
    text.SetTextColor(r.kBlack)
    if (Hps):
        text.DrawLatex(xoffset, line, '#bf{#it{HPS}} Internal')
    if runNumber:
        line = line-newline
        if "MC" in runNumber:
            text.DrawLatex(xoffset, line, "MC Simulation")
        else:
            #text.DrawLatex(xoffset,line,"Run "+runNumber)
            text.DrawLatex(xoffset, line, runNumber)
    for iText in range(len(texts)):
        if texts[iText]:
            line = line-newline
            if (Colors):
                text.SetTextColor(colors[iText])
            text.DrawLatex(xoffset, line, texts[iText])

    return line


def SetStyle():
    r.gROOT.SetBatch(1)

    hpsStyle = r.TStyle("HPS", "HPS style")

    # use plain black on white colors
    icol = 0
    hpsStyle.SetFrameBorderMode(icol)
    hpsStyle.SetCanvasBorderMode(icol)
    hpsStyle.SetPadBorderMode(icol)
    hpsStyle.SetPadColor(icol)
    hpsStyle.SetCanvasColor(icol)
    hpsStyle.SetStatColor(icol)
#hpsStyle.SetFillColor(icol)

# set the paper & margin sizes
    hpsStyle.SetPaperSize(20, 26)
    hpsStyle.SetPadTopMargin(0.05)
    hpsStyle.SetPadRightMargin(0.05)
    hpsStyle.SetPadBottomMargin(0.18)
    hpsStyle.SetPadLeftMargin(0.14)

    # use large fonts
#font=72
    font = 42
    tsize = 0.08
    tzsize = 0.045
    hpsStyle.SetTextFont(font)

    hpsStyle.SetTextSize(tsize)
    hpsStyle.SetLabelFont(font, "x")
    hpsStyle.SetTitleFont(font, "x")
    hpsStyle.SetLabelFont(font, "y")
    hpsStyle.SetTitleFont(font, "y")
    hpsStyle.SetLabelFont(font, "z")
    hpsStyle.SetTitleFont(font, "z")

    hpsStyle.SetLabelSize(tsize, "x")
    hpsStyle.SetTitleSize(tsize, "x")
    hpsStyle.SetLabelSize(tsize, "y")
    hpsStyle.SetTitleSize(tsize, "y")
    hpsStyle.SetLabelSize(tzsize, "z")
    hpsStyle.SetTitleSize(tzsize, "z")

    hpsStyle.SetTitleOffset(0.7, "y")
    hpsStyle.SetTitleOffset(1.15, "x")


#use bold lines and markers
    #hpsStyle.SetMarkerStyle(20)
    hpsStyle.SetMarkerSize(1.0)
    hpsStyle.SetHistLineWidth(3)
    hpsStyle.SetLineStyleString(2, "[12 12]")  # postscript dashes

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

    r.gROOT.SetStyle("Plain")

#gStyle.SetPadTickX(1)
#gStyle.SetPadTickY(1)
    r.gROOT.SetStyle("HPS")
    r.gROOT.ForceStyle()
    r.gStyle.SetOptTitle(0)
    r.gStyle.SetOptStat(0)
    r.gStyle.SetOptFit(0)


# overwrite hps styles
    hpsStyle.SetPadLeftMargin(0.14)
    hpsStyle.SetPadRightMargin(0.06)
    hpsStyle.SetPadBottomMargin(0.11)
    hpsStyle.SetPadTopMargin(0.05)
    hpsStyle.SetFrameFillColor(0)

    NRGBs = 5
    NCont = 255

    stops = array("d", [0.00, 0.34, 0.61, 0.84, 1.00])
    red = array("d", [0.00, 0.00, 0.87, 1.00, 0.51])
    green = array("d", [0.00, 0.81, 1.00, 0.20, 0.00])
    blue = array("d", [0.51, 1.00, 0.12, 0.00, 0.00])
    r.TColor.CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont)
    r.gStyle.SetNumberContours(NCont)


def MakeRadFrac(name, outdir, histos, legends, oFext, xtitle="", ytitle="", ymin=0, ymax=1, noErrors=False, RebinFactor=0, runNumber="", additionalText=[], RatioType="Alternate", LogX=False, LogY=False, RatioMin=0.0, RatioMax=0.15, WriteMean=False, Normalise=False):

    if not os.path.exists(outdir):
        os.mkdir(outdir)

    Xmin = 0
    Xmax = 1

    can = r.TCanvas(name, name, 1200, 800)
    can.SetMargin(0, 0, 0, 0)
    top = r.TPad("top", "top", 0, 0.42, 1, 1)
    if LogX:
        top.SetLogx(1)
        bot.SetLogx(1)
    if LogY:
        top.SetLogy(1)

    bot = r.TPad("bot", "bot", 0, 0, 1, 0.38)

    #----------Histogram------------#

    top.Draw()
    top.SetBottomMargin(0)
    top.SetTopMargin(r.gStyle.GetPadTopMargin()*topScale)
    bot.Draw()
    bot.SetTopMargin(0)
    bot.SetBottomMargin(0.4)
    top.cd()
    plotsProperties = []
    histoStack = r.THStack(name+"_sh", "")

    for ih in range(len(histos)):

        if (Normalise):
            if (histos[ih].Integral() == 0):
                return None
            histos[ih].Scale(1./histos[ih].Integral())
            histos[ih].GetYaxis().SetRangeUser(0.00001, histos[ih].GetMaximum()*15000)

        histos[ih].GetXaxis().SetRangeUser(40.0, 400.)
        histos[ih].SetMarkerColor(colors[ih])
        histos[ih].SetMarkerStyle(markers[ih])
        histos[ih].SetLineColor(colors[ih])
        histos[ih].GetXaxis().CenterTitle()
        histos[ih].GetYaxis().CenterTitle()
        histos[ih].GetYaxis().SetTitleOffset(0.4)
        histos[ih].Write()

        plotsProperties.append(("#mu=%.4f" % round(histos[ih].GetMean(), 4))+(" #sigma=%.4f" % round(histos[ih].GetRMS(), 4)))

        if RebinFactor > 0:
            histos[ih].Rebin(RebinFactor)

        if ih > 0:
            histoStack.Add(histos[ih])
            if ih == 1:
                histoSum = deepcopy(histos[ih])
            else:
                histoSum.Add(histos[ih])

    histoStack.Draw('ep')
    histoStack.GetXaxis().SetRangeUser(40.0, 400.)
    histos[0].Draw('same ep')

    InsertText(runNumber, additionalText, 0.8, xoffset=0.75)
    if (WriteMean):
        InsertText("", plotsProperties, 0.8, 0.6, False)

    if len(legends) > 0:
        #print "building legend"
        #upperY=0.6
        upperY = 0.76
        linesep = 0.07
        lowerY = upperY - len(legends) * linesep
        #minX = 0.51
        minX = 0.75
        maxX = minX+0.15
        leg = r.TLegend(minX, upperY, maxX, lowerY)
        leg.SetBorderSize(0)
        leg.SetFillColor(0)
        leg.SetTextSize(0.04)
        for i_leg in range(len(legends)):
            #print "Adding Entry",i_leg, legends[i_leg]
            leg.AddEntry(histos[i_leg], legends[i_leg], "lpf")
            pass
        leg.Draw()
        pass

    #-------------Ratio---------------------#

    bot.cd()
    numerator = histos[0].Clone("numerator")
    numerator.GetYaxis().SetTitle("f_{rad}")
    numerator.GetYaxis().SetTitleSize(0.08)
    numerator.GetYaxis().SetTitleOffset(0.5)
    numerator.GetXaxis().SetTitleSize(0.1)
    numerator.GetXaxis().SetLabelSize(0.12)
    numerator.GetYaxis().SetRangeUser(RatioMin, RatioMax)
    numerator.GetXaxis().SetRangeUser(40.0, 400.)
    numerator.GetYaxis().SetNdivisions(508)
    numerator.GetYaxis().SetDecimals(True)
    numerator.Draw("axis")

    numerator.Divide(histoSum)
    nPoints = 0
    nBins = numerator.GetNbinsX()
    for ibin in range(nBins):
        if numerator.GetBinContent(ibin) > 0:
            nPoints += 1
        pass

    chi2s = []
    fstats = []
    for polyO in range(10):
        fitResult = numerator.Fit('pol%i' % polyO, "ES")
        chi2s.append(fitResult.Chi2())
        if polyO > 0:
            fstats.append((chi2s[polyO-1]-chi2s[polyO])*(nPoints-polyO-1)/(chi2s[polyO]))
        else:
            fitCon = fitResult.GetParams()[0]

    print("nPoints: {}".format(nPoints))
    print('Test Print!!')
    for polyO in range(1, 10):
        print('Order: %i    chi2: %f    f-stat: %f' % (polyO, chi2s[polyO], fstats[polyO-1]))
    fitResult = numerator.Fit('pol5', "ES")
    fitFunc = numerator.GetListOfFunctions().FindObject("pol5")
    fitFunc.SetLineColor(colors[3])
    numerator.DrawCopy("pe same")

    line = r.TLine()
    line.SetLineStyle(r.kDashed)
    line.DrawLine(0.00, fitCon, 400.0, fitCon)

    can.SaveAs(outdir+"/"+name+oFext)
    return deepcopy(can)


def MakePlot(name, outdir, histos, legends, oFext, xtitle="", ytitle="", ymin=0, ymax=1, noErrors=False, RebinFactor=0, runNumber="", additionalText=[], RatioType="Alternate", LogX=False, LogY=False, RatioMin=0.25, RatioMax=1.75, WriteMean=False, Normalise=False, doFit=False, drawOptions="hist", Xmin=-999, Xmax=-999):

    if not os.path.exists(outdir):
        os.mkdir(outdir)

    #Make 1D plots only
    if "_hh" in name:
        return None

    can = r.TCanvas(name, name, 1200, 800)
    can.SetMargin(0, 0, 0, 0)
    top = r.TPad("top", "top", 0, 0.42, 1, 1)
    if LogX:
        top.SetLogx(1)
        bot.SetLogx(1)
    if LogY:
        top.SetLogy(1)

    bot = r.TPad("bot", "bot", 0, 0, 1, 0.38)

    #----------Histogram------------#

    top.Draw()
    top.SetBottomMargin(0)
    top.SetTopMargin(r.gStyle.GetPadTopMargin()*topScale)
    bot.Draw()
    bot.SetTopMargin(0)
    bot.SetBottomMargin(0.4)
    top.cd()
    plotsProperties = []
    Ymax = -999
    for ih in range(len(histos)):
        if Ymax < histos[ih].GetMaximum()*1.9:
            Ymax = histos[ih].GetMaximum()*1.9

    for ih in range(len(histos)):

        if (Normalise):

            if (histos[ih].Integral() == 0):
                return None
            histos[ih].Scale(1./histos[ih].Integral())
            if LogY:
                histos[ih].GetYaxis().SetRangeUser(0.00001, histos[ih].GetMaximum()*15000)
            else:
                histos[ih].GetYaxis().SetRangeUser(0.00001, histos[ih].GetMaximum()*2.2)

        elif not Normalise:
            if LogY:
                histos[ih].GetYaxis().SetRangeUser(0.00001, histos[ih].GetMaximum()*15000)
            else:
                histos[ih].GetYaxis().SetRangeUser(0, Ymax)

        histos[ih].SetMarkerColor(colors[ih])
        histos[ih].SetMarkerStyle(markers[ih])
        histos[ih].SetLineColor(colors[ih])
        histos[ih].GetXaxis().CenterTitle()
        histos[ih].GetYaxis().CenterTitle()

        if (not doFit):
            plotsProperties.append(("#mu=%.4f" % round(histos[ih].GetMean(), 4))+(" #sigma=%.4f" % round(histos[ih].GetRMS(), 4)))

        if RebinFactor > 0:
            histos[ih].Rebin(RebinFactor)

        if doFit:
            #histo = histos[ih].Clone()
            fit_funcs.append(TF1("fit_func"+str(ih), "gaus", -2.5, 2.5))
            print(len(fit_funcs))
            bad_fit = histos[ih].Fit(fit_funcs[ih], "RQN")

            mu = fit_funcs[ih].GetParameter(1)
            mu_err = fit_funcs[ih].GetParError(1)
            sigma = fit_funcs[ih].GetParameter(2)
            sigma_err = fit_funcs[ih].GetParError(2)

            plotsProperties.append((" #mu=%.3f" % round(mu, 3))+("+/- %.3f" % round(mu_err, 3))
                                   + (" #sigma=%.3f" % round(sigma, 3)) + ("+/- %.3f" % round(sigma_err, 3)))

            fit_funcs[ih].SetLineColor(histos[ih].GetLineColor())

        if ih == 0:

            if (Xmin != -999 and Xmax != -999):
                histos[ih].GetXaxis().SetRangeUser(Xmin, Xmax)
            if noErrors:
                #histos[ih].GetXaxis().SetTextSize(0.045)
                #histos[ih].GetYaxis().SetTextSize(0.045)
                histos[ih].Draw("e")

            else:
                histos[ih].Draw("e")
            if xtitle:
                histos[ih].GetXaxis().SetTitle(xtitle)
            if ytitle:
                histos[ih].GetYaxis().SetTitle(ytitle)
        else:

            if noErrors:
                histos[ih].Draw("same "+drawOptions)
            else:
                histos[ih].Draw("same e "+drawOptions)

        if (doFit):
            fit_funcs[ih].Draw("same")

    linevalue = InsertText(runNumber, additionalText, 0.8, xoffset=0.75)
    if (WriteMean):
        InsertText("", plotsProperties, 0.8, 0.6, False, True)

    if len(legends) > 0:
        #print "building legend"
        #upperY=0.6
        upperY = linevalue-0.04
        linesep = 0.07
        lowerY = upperY - len(legends) * linesep
        #minX = 0.51
        minX = 0.75
        maxX = minX+0.15
        leg = r.TLegend(minX, upperY, maxX, lowerY)
        leg.SetBorderSize(0)
        leg.SetFillColor(0)
        leg.SetTextSize(0.04)
        for i_leg in range(len(legends)):
            #print "Adding Entry",i_leg, legends[i_leg]
            leg.AddEntry(histos[i_leg], legends[i_leg], "lpf")
            pass
        leg.Draw()
        pass

    #-------------Ratio---------------------#

    bot.cd()
    reference = histos[0].Clone("reference")
    reference.GetXaxis().SetLabelSize(0.1)
    reference.GetXaxis().SetTitleSize(0.1)
    reference.GetYaxis().SetTitle("Ratio")
    reference.GetYaxis().SetTitleSize(0.1)
    reference.GetYaxis().SetLabelSize(0.1)
    reference.GetYaxis().SetTitleOffset(reference.GetYaxis().GetTitleOffset()*0.7)
    reference.GetYaxis().SetRangeUser(RatioMin, RatioMax)
    reference.GetYaxis().SetNdivisions(508)
    reference.GetYaxis().SetDecimals(True)
    reference.Draw("axis")

    if (RatioType == "Sequential"):

        for ih in range(1, len(histos)):
            #ForRatio=None
            #if type(histos[ih]) is TProfile:
            #    ForRatio = histos[ih].ProjectionX("ForRatio"+str(ih)+histos[ih].GetName())
            #else:
            ForRatio = histos[ih].Clone("ForRatio"+str(ih)+histos[ih].GetName())

            ForRatio.SetMaximum(100.)
            ForRatio.Divide(reference)
            ForRatio.DrawCopy("pe same")

    elif (RatioType == "Alternate"):

        print("in alternate ratio")
        for ih in range(1, len(histos), 2):

            numerator = histos[ih].Clone("numerator")
            #if isinstance(histos[ih],TProfile):

            #    numerator=histos[ih].ProjectionX("numerator")

            #else:

            numerator.SetMaximum(100.)
            numerator.Divide(histos[ih-1])
            numerator.DrawCopy("pe same")

    elif (RatioType == "Alternate2"):
        print("in Alternate (h1 - h2) / h2")

        numerator = histos[ih].Clone("numerator")
        numerator.Add(histos[ih-1], -1)
        numerator.SetMaximum(100.)
        numerator.Divide(histos[ih-1])

        numerator.DrawCopy("hist p same")

    line = r.TLine()
    line.SetLineStyle(r.kDashed)
    if (Xmin != -999 and Xmax != -999):
        line.DrawLine(Xmin, 1, Xmax, 1)
    else:
        line.DrawLine(reference.GetXaxis().GetXmin(), 1, reference.GetXaxis().GetXmax(), 1)

    can.SaveAs(outdir+"/"+name+oFext)
    return deepcopy(can)


def DivideHistos(h1, h2):
    for ibin in range(1, h1.GetNbinsX()+1):
        if (h2.GetBinContent(ibin) == 0):
            continue
        h1.SetBinContent(ibin, h1.GetBinContent(ibin) / h2.GetBinContent(ibin))
    return h1


def Make1Dplots(name, outdir, histos, colors, markers, legends, oFext, xtitle="", ytitle="", ymin=0, ymax=1, noErrors=False, RebinFactor=0, runNumber="", additionalText=[], LogY=False, WriteMean=False, multiLeg=False):

    if not os.path.exists(outdir):
        os.mkdir(outdir)

    can = r.TCanvas(name, name, 1500, 1000)
    if LogY:
        can.SetLogy(1)

    means = []
    meansErr = []

    for ih in range(len(histos)):

        means.append(histos[ih].GetMean(2))
        meansErr.append(histos[ih].GetMeanError(2))

        histos[ih].SetMarkerColor(colors[ih])
        histos[ih].SetMarkerStyle(markers[ih])
        histos[ih].SetLineColor(colors[ih])
        histos[ih].GetYaxis().SetRangeUser(ymin, ymax)
        histos[ih].GetXaxis().CenterTitle()
        histos[ih].GetYaxis().CenterTitle()
        if ("pT" in name or "pt" in name):
            histos[ih].GetXaxis().SetRangeUser(1., 20.)
        #histos[ih].SetMarkerSize(0.5)
        if RebinFactor > 0:
            histos[ih].Rebin(RebinFactor)

        if ih == 0:
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

    InsertText(runNumber, additionalText, 0.8, xoffset=0.7)

    if len(legends) > 0:
        #print "building legend"
        upperY = 0.6
        linesep = 0.10
        lowerY = upperY - len(legends) * linesep
        minX = 0.51
        maxX = minX+0.33
        leg2 = None

        if len(legends) > 10:
            leg2 = r.TLegend(minX, upperY, maxX-0.02, lowerY)
            leg2.SetBorderSize(0)
            leg2.SetFillColor(0)
            entry = leg2.AddEntry("Todo", "To do", "p")
            entry.SetMarkerStyle(kOpenSquare)

            entry2 = leg2.AddEntry("Todo", "Todo", "p")
            entry2.SetMarkerStyle(kOpenCircle)
            leg2.Draw()

            minX = 0.87
            maxX = minX+0.12

        leg = r.TLegend(minX, upperY, maxX, lowerY)
        leg.SetBorderSize(0)
        leg.SetFillColor(0)
        leg.SetTextSize(0.031)

        for i_leg in range(len(legends)):

            if not WriteMean:
                print("Adding Entry {} {}".format(i_leg, legends[i_leg]))
            else:
                print("Adding Entry {} {} #epsilon: {} ".format(i_leg, legends[i_leg], str(round(means[i_leg], 2))))

            #leg.AddEntry(histos[i_leg],"#splitline{"+legends[i_leg] + "}{Average=" + str(round(means[i_leg]*100.,3))+"#pm"+str(round(meansErr[i_leg]*100,3))+"%}" ,"lpf")
            #leg.AddEntry(histos[i_leg],legends[i_leg] + " Average=" + str(round(means[i_leg]*100.,3))+"#pm"+str(round(meansErr[i_leg]*100,3))+"%" ,"lpf")
            if not multiLeg:
                leg.AddEntry(histos[i_leg], legends[i_leg] + " Avg=" + str(round(means[i_leg]*100., 3))+"#pm"+str(round(meansErr[i_leg]*100, 3))+"%", "lpf")
            else:
                leg.AddEntry(histos[i_leg], "#splitline{"+legends[i_leg] + "}{Avg=" + str(round(means[i_leg]*100., 3))+"#pm"+str(round(meansErr[i_leg]*100, 3))+"%}", "lpf")

            leg.Draw()

    #BuildLegend(legends,can,histos,0.55,0.9,0.8,0.75)

    can.SetBottomMargin(0.18)
    can.SetLeftMargin(0.15)

    can.SaveAs(outdir+"/"+name+oFext)


def Make2DRatio(name, outdir, histo1, histo2, xtitle="", ytitle="", ztitle="", runNumber="", legends=[], oFext=".png"):
    if not os.path.exists(outdir):
        os.mkdir(outdir)

    ratio = histo1.Clone()
    ratio.Divide(histo2)

    can = r.TCanvas()
    can.SetRightMargin(0.2)

    ratio.GetZaxis().SetRangeUser(0.9, 1.1)
    ratio.GetXaxis().SetTitle(xtitle)
    ratio.GetYaxis().SetTitle(ytitle)
    ratio.GetZaxis().SetTitle(ztitle)
    ratio.Draw("colz text")

    can.SaveAs(outdir+"/"+name+oFext)


def Make2DPlots(name, outdir, histolist, xtitle, ytitle, ztitle="", text="", zmin="", zmax=""):
    oFext = ".pdf"
    if not os.path.exists(outdir):
        os.mkdir(outdir)
    canvs = []
    for ih in range(0, len(histolist)):
        can = r.TCanvas()
        can.SetRightMargin(0.2)

        #histolist[ih].GetZaxis().SetRangeUser(zmin,zmax)
        histolist[ih].GetXaxis().SetTitle(xtitle[ih])
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
        histolist[ih].GetYaxis().SetTitle(ytitle[ih])

        histolist[ih].Draw("colz")

        InsertText(text, "")

        #print "saving..."
        #if (len(legends) == len(histolist)):
        can.SaveAs(outdir+"/"+name[ih]+oFext)
        canvs.append(can)
    return deepcopy(canvs)
    #else:
    #    print "ERROR: Not enough names for all the histos"


def Profile2DPlot(name, outdir, histolist, axis="X", xtitle="", ytitle="", ztitle="", runNumber="", legends=[], zmin="", zmax=""):
    oFext = ".pdf"
    if not os.path.exists(outdir):
        os.mkdir(outdir)

    p = None

    if (axis == "X"):
        p = ProjectionX()


def makeHTML(outDir, title, selection):
    os.chdir(outDir)
    plots = sorted(glob.glob('*.png') + glob.glob('*.pdf'))
    outfilebase = os.path.split(outDir)[1]
    f = open(outfilebase+'.html', "w+")
    f.write("<!DOCTYPE html\n")
    f.write(" PUBLIC \"-//W3C//DTD HTML 3.2//EN\">\n")
    f.write("<html>\n")
    f.write("<head><title>" + title + " </title></head>\n")
    f.write("<body bgcolor=\"EEEEEE\">\n")
    f.write("<table border=\"0\" cellspacing=\"5\" width=\"100%\">\n")
    for i in range(0, len(plots)):
        pname = ""
        if selection[0] in plots[i]:
            pname = plots[i].replace(selection[0]+'_', '')

        offset = 1
        if i == 0 or i % 2 == 0:
            f.write("<tr>\n")
        f.write("<td width=\"10%\"><a target=\"_blank\" href=\"" + plots[i] + "\"><img src=\"" + plots[i] + "\" alt=\"" + plots[i] + "\" title=\"" + pname + "\" width=\"85%\" ></a></td>\n")
        if i == offset:
            f.write("</tr>\n")
        elif (i > offset and (i-offset) % 2 == 0) or i == len(plots):
            f.write("</tr>\n")

    f.write("</table>\n")
    f.write("</body>\n")
    f.write("</html>")
    f.close()


def MakeTotalRadAcc(name, outdir, histos, legends, oFext, xtitle="", ytitle="", ymin=0, ymax=1, noErrors=False, RebinFactor=0, runNumber="", additionalText=[], RatioType="Alternate", LogX=False, LogY=False, RatioMin=0.0, RatioMax=0.15, WriteMean=False, Normalise=False):

    if not os.path.exists(outdir):
        os.mkdir(outdir)

    Xmin = 0
    Xmax = 1

    can = r.TCanvas(name, name, 1200, 800)
    can.SetMargin(0, 0, 0, 0)
    top = r.TPad("top", "top", 0, 0.42, 1, 1)
    if LogX:
        top.SetLogx(1)
        bot.SetLogx(1)
    if LogY:
        top.SetLogy(1)

    bot = r.TPad("bot", "bot", 0, 0, 1, 0.38)

    #----------Histogram------------#

    top.Draw()
    top.SetBottomMargin(0)
    top.SetTopMargin(r.gStyle.GetPadTopMargin()*topScale)
    bot.Draw()
    bot.SetTopMargin(0)
    bot.SetBottomMargin(0.4)
    top.cd()
    plotsProperties = []
    histoStack = r.THStack(name+"_sh", "")

    for ih in range(len(histos)):
        print("looping over histo ", histos[ih].GetName())
        if (Normalise):
            if (histos[ih].Integral() == 0):
                return None
            histos[ih].Scale(1./histos[ih].Integral())
            histos[ih].GetYaxis().SetRangeUser(0.00001, histos[ih].GetMaximum()*15000)

        histos[ih].GetYaxis().SetRangeUser(0.1, 100000000.)
        histos[ih].GetXaxis().SetRangeUser(50.0, 400.)
        histos[ih].SetMarkerColor(colors[ih])
        histos[ih].SetMarkerStyle(markers[ih])
        histos[ih].SetLineColor(colors[ih])
        histos[ih].GetXaxis().CenterTitle()
        histos[ih].GetYaxis().CenterTitle()
        histos[ih].GetYaxis().SetTitleOffset(0.4)

        plotsProperties.append(("#mu=%.4f" % round(histos[ih].GetMean(), 4))+(" #sigma=%.4f" % round(histos[ih].GetRMS(), 4)))

        if RebinFactor > 0:
            histos[ih].Rebin(RebinFactor)

        if ih > 0:
            histoStack.Add(histos[ih])
            if ih == 1:
                histoSum = deepcopy(histos[ih])
            else:
                histoSum.Add(histos[ih])

        histos[ih].Write()

    histoStack.Draw('ep')
    histoStack.GetYaxis().SetRangeUser(0.1, 100000000.)
    histos[0].Draw('same ep')
    histos[0].GetYaxis().SetRangeUser(0.1, 100000000.)
    histoStack.Write()

    InsertText(runNumber, additionalText, 0.8, xoffset=0.75)
    if (WriteMean):
        InsertText("", plotsProperties, 0.8, 0.6, False)

    if len(legends) > 0:
        #print "building legend"
        #upperY=0.6
        upperY = 0.76
        linesep = 0.07
        lowerY = upperY - len(legends) * linesep
        #minX = 0.51
        minX = 0.75
        maxX = minX+0.15
        leg = r.TLegend(minX, upperY, maxX, lowerY)
        leg.SetBorderSize(0)
        leg.SetFillColor(0)
        leg.SetTextSize(0.04)
        for i_leg in range(len(legends)):
            #print "Adding Entry",i_leg, legends[i_leg]
            leg.AddEntry(histos[i_leg], legends[i_leg], "lpf")
            pass
        leg.Draw()
        pass

    #-------------Ratio---------------------#

    bot.cd()
    numerator = histos[0].Clone("numerator")
    print("numerator is ", histos[0].GetName())
    numerator.GetYaxis().SetTitle("Zeta")
    numerator.GetYaxis().SetTitleSize(0.08)
    numerator.GetYaxis().SetTitleOffset(0.5)
    numerator.GetXaxis().SetTitleSize(0.1)
    numerator.GetXaxis().SetLabelSize(0.12)
    numerator.GetYaxis().SetRangeUser(RatioMin, RatioMax)
    numerator.GetXaxis().SetRangeUser(50.0, 400.)
    numerator.GetYaxis().SetNdivisions(508)
    numerator.GetYaxis().SetDecimals(True)
    numerator.Draw("axis")

    numerator.Divide(histoSum)
    print("histosum ", histoSum)
    nPoints = 0
    nBins = numerator.GetNbinsX()
    print("n numerator bins ", nBins)
    for ibin in range(nBins):
        if numerator.GetBinContent(ibin) > 0:
            nPoints += 1
        pass
    numerator.Write()

    chi2s = []
    fstats = []
    for polyO in range(10):
        fitResult = numerator.Fit('pol%i' % polyO, "ES", "", 50.0, 400.0)
        chi2s.append(fitResult.Chi2())
        if polyO > 0:
            fstats.append((chi2s[polyO-1]-chi2s[polyO])*(nPoints-polyO-1)/(chi2s[polyO]))
        else:
            fitCon = fitResult.GetParams()[0]

    print("nPoints: {}".format(nPoints))
    for polyO in range(1, 10):
        print("Order: %i    chi2: %f    f-stat: %f" % (polyO, chi2s[polyO], fstats[polyO-1]))
    fitResult = numerator.Fit('pol9', "ES", "", 50.0, 400.0)
    fitFunc = numerator.GetListOfFunctions().FindObject("pol9")
    fitFunc.SetLineColor(colors[3])
    numerator.DrawCopy("pe same")

    line = r.TLine()
    line.SetLineStyle(r.kDashed)
    line.DrawLine(45., fitCon, 350., fitCon)

    can.SaveAs(outdir+"/"+name+oFext)
    return deepcopy(can)
