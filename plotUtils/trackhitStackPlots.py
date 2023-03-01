import ROOT as r
import argparse


def getColors():
    colors = [r.kBlue, r.kGreen, r.kOrange, r.kRed, r.kYellow, r.kMagenta, r.kCyan, r.kPink+1, r.kSpring+10, r.kViolet+2, r.kTeal-1, r.kOrange+7, r.kMagenta-3, r.kYellow-3, r.kBlue+2, r.kPink-9]
    return colors


def getRootSubDirs(infile, keyword=""):
    subdirs = []
    for key in infile.GetListOfKeys():
        if keyword in key.GetName():
            subdirs.append(key.GetName())
    return subdirs


def readHistogram(infile, histoname, subDir=None):
    print("Looking for %s/%s" % (subDir, histoname))
    infile.cd()
    histo = None
    keys = None
    if subDir is not None:
        subDir = infile.Get("%s" % (subDir))
    else:
        subDir = infile

    for key in subDir.GetListOfKeys():
        if histoname in key.GetName():
            histo = subDir.Get("%s" % (key.GetName()))

    if histo is not None:
        print("Found %s" % (histo.GetName()))
    else:
        print("FAILED TO FIND HISTOGRAM %s/%s IN FILE" % (subDir, key.GetName()))

    return histo


def formatHistogram(h, linecolor=1, linewidth=1, name=None, title=None, xlabel=None, ylabel=None, fillcolor=None):
    h.SetLineColor(linecolor)
    h.SetLineWidth(linewidth)
    if name is not None:
        h.SetName("%s" % (name))
    if title is not None:
        h.SetTitle("%s" % (title))
    if xlabel is not None:
        h.GetXaxis().SetTitle("%s" % (xlabel))
    if ylabel is not None:
        h.GetYaxis().SetTitle("%s" % (ylabel))
    if fillcolor is not None:
        h.SetFillColor(fillcolor)


def sortHistograms(histograms):

    sortedHistograms = sorted(histograms, key=lambda x: x.GetEntries(), reverse=False)

    return sortedHistograms


def setTHStackXRange(tstack):
    xmin = 99999999
    xmax = -99999999
    for histo in tstack:
        ymax = histo.GetBinContent(histo.GetMaximumBin())
        hmin = histo.GetBinLowEdge(histo.FindFirstBinAbove(ymax*0.05))
        hmax = histo.GetBinLowEdge(histo.FindLastBinAbove(ymax*0.05))
        if hmin < xmin:
            xmin = hmin
        if hmax > xmax:
            xmax = hmax
    tstack.GetXaxis().SetRangeUser(xmin, xmax)
    return [xmin, xmax]


def buildLegend(canvas, x1=0.7, y1=0.24, x2=0.94, y2=0.88, textsize=0.04, separation=1.0, fill=0, border=0):
    #canvas.cd()
    legend = canvas.BuildLegend(x1, y1, x2, y2)
    legend.Draw()
    legend.SetTextSize(textsize)
    legend.SetEntrySeparation(separation)
    legend.SetFillStyle(fill)
    legend.SetBorderSize(border)


##################################################################################################
parser = argparse.ArgumentParser(description="baseConfig options ")
parser.add_argument('--infile', '-i', type=str, dest="inFilename", metavar='infiles',
                    help="Input files, specify on or more.")
parser.add_argument("-o", "--outFile", type=str, dest="outFilename", action='store',
                    help="Output filename.", metavar="outFilename", default="out.root")
parser.add_argument('--outDir', '-d', type=str, dest="outDir", action='store',
                    help="Specify the output directory.", metavar="outDir", default=".")
parser.add_argument('--run', '-r', type=str, dest="run", action='store',
                    help="Specify run number.", metavar="run", default="")
options = parser.parse_args()

infile = r.TFile("%s" % (options.inFilename), "READ")
outfile = r.TFile("%s" % (options.outFilename), "RECREATE")
outdir = options.outDir
run = options.run

colorsMap = {15: r.kBlue, 14: r.kGreen, 12: r.kOrange, 13: r.kRed, 11: r.kYellow, 8: r.kMagenta, 9: r.kCyan, 10: r.kPink+1, 4: r.kSpring+10, 6: r.kViolet+2, 7: r.kTeal-1, 3: r.kOrange+7, 5: r.kMagenta-3, 2: r.kYellow-3, 1: r.kBlue+2, 0: r.kPink-9}

#Get hit codes from file
hitcodes = getRootSubDirs(infile, "hc")

#Build names for category of histograms to read from file and make THStack
#All histograms that share 'histoname' in file will be stacked
histonames = []
charges = ["Pos", "Ele"]
halves = ["top", "bot"]
params = ["TanLambda", "Z0", "d0", "Phi", "p_h"]
for param in params:
    for charge in charges:
        for half in halves:
            name = half+charge+"_"+param
            histonames.append(name)

#Initialize list to hold all THStack plots
tStacks = []

#Read histograms to make a THStack for each type of histogram
for histoname in histonames:
    histograms = []
    for hitcode in hitcodes:
        histogram = readHistogram(infile, histoname, hitcode)
        if histogram.GetEntries() > 0:
            histograms.append(histogram)
    #sort histograms by number of entries, makes THStack easier to read
    histograms = sortHistograms(histograms)

    tstack = r.THStack("%s_run_%s_hit_combinations" % (histoname, run), "%s_run_%s_hit_combinations" % (histoname, run))
    for histo in histograms:
        print(histo.GetName())
        splitname = histo.GetName().split("_")
        inthitcode = int(splitname[0].replace("hc", ''))
        hitcode = splitname[0]+"_"+splitname[1]
        col = colorsMap[inthitcode]
        formatHistogram(histo, linewidth=1, name=hitcode, title=hitcode, fillcolor=col)
        tstack.Add(histo)
    tStacks.append(tstack)

#Change to outroot file
outfile.cd()

#Separate THStacks into plotting categories...
#Want to plot topTanLambda and botTanLambda THStacks on same canvas, charge separated
plotSames = []
plotSamesNames = ["Ele_TanLambda", "Pos_TanLambda"]
tanlam_ele = []
tanlam_pos = []
#All other THstacks will be plotted on individual canvas
plots = []
for tstack in tStacks:
    if "Ele_TanLambda" in tstack.GetName():
        tanlam_ele.append(tstack)
    elif "Pos_TanLambda" in tstack.GetName():
        tanlam_pos.append(tstack)
    else:
        plots.append(tstack)
plotSames.append(tanlam_ele)
plotSames.append(tanlam_pos)

#Loop over all THStack plots, Draw, Format, and Save them
#Find ymax for stack
for tstack in plots:
    c = r.TCanvas("%s" % (tstack.GetName()), "%s" % (tstack.GetName()), 1800, 1000)
    c.cd()
    tstack.Draw("HIST")
    setTHStackXRange(tstack)
    if "Z0" in tstack.GetName():
        tstack.GetXaxis().SetRangeUser(-1.0, 1.0)
    tstack.GetXaxis().SetTitle("%s" % (tstack.GetName().split("_")[1]))
    if "Pos_Phi" in tstack.GetName():
        buildLegend(c, x1=0.2, x2=0.38)
    else:
        buildLegend(c)
    c.Write()
    c.SaveAs("%s/%s.png" % (outdir, tstack.GetName()))

#Loop over THStacks that will be plotted on same canvas
for i, tstacks in enumerate(plotSames):
    ymax = 0.0
    for tstack in tstacks:
        y = tstack.GetMaximum()
        if y > ymax:
            ymax = y

    c = r.TCanvas("%s_%s_hit_combinations" % (plotSamesNames[i], run), "%s_%s_hit_combinations" % (plotSamesNames[i], run), 1800, 1000)
    c.cd()
    for j, tstack in enumerate(tstacks):
        if j < 1:
            tstack.Draw("HIST")
            buildLegend(c)
        else:
            tstack.Draw("HISTsame")
        tstack.SetMaximum(ymax*1.10)
        tstack.GetXaxis().SetRangeUser(-0.1, 0.1)
        tstack.GetXaxis().SetTitle("%s" % (tstack.GetName().split("_")[1]))
        tstack.SetTitle("%s_run_%s_hit_combinations" % (plotSamesNames[i], run))

    c.Write()
    c.SaveAs("%s/%s.png" % (outdir, c.GetName()))
