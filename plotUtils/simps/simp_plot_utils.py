import ROOT as r
import numpy as np
from array import array
import copy
import uproot

def getColorsHPS():
    colors = [r.kBlue+2, r.kCyan+2, r.kRed+2, r.kOrange+10, r.kYellow+2, r.kGreen-1, r.kAzure-2, r.kGreen-8, r.kOrange+3, r.kYellow+2, r.kRed+2, r.kBlue+2, r.kGreen-8, r.kOrange+3, r.kYellow+2, r.kRed+2, r.kBlue+2, r.kGreen-8, r.kOrange+3, r.kYellow+2, r.kRed+2, r.kBlue+2, r.kGreen-8, r.kOrange+3, r.kYellow+2, r.kRed+2, r.kBlue+2, r.kGreen-8, r.kOrange+3]
    return colors

def plot_zbi_multigraph(graph_nsig, graph_nbkg, graph_zbi, canvas_name, leftyaxis_title, rightyaxis_title):
    # Create TMultiGraph to hold all TGraphs
    multi_graph = r.TMultiGraph()

    # Add nsig and nbkg graphs to the multi_graph (use left y-axis)
    multi_graph.Add(graph_nsig)
    multi_graph.Add(graph_nbkg)

    # Create a TCanvas to draw the multi_graph
    canvas = r.TCanvas(canvas_name, canvas_name, 2040, 1080)

    # Draw the multi_graph with left y-axis (default)
    multi_graph.Draw("AP")

    c.Draw()
    # Set axis labels and titles for the left y-axis
    multi_graph.GetYaxis().SetTitle(leftyaxis_title)
    multi_graph.GetYaxis().SetTitleOffset(1.2)

    # Create a second y-axis for the ZBi graph
    axis_zbi = multi_graph.GetHistogram().GetXaxis() # Copy the x-axis
    axis_zbi.SetTitle(rightyaxis_title)  # Set title for the right y-axis
    axis_zbi.SetTitleOffset(1.2)  # Adjust title offset for the right y-axis
    axis_zbi.SetLabelOffset(999)  # Hide labels for the right y-axis

    # Draw the ZBi graph with the right y-axis
    graph_zbi.SetLineColor(r.kRed)  # Set color for better visibility
    graph_zbi.Draw("same")
    graph_zbi.SetMarkerColor(r.kRed)  # Set marker color

    # Set the second y-axis for the ZBi graph
    multi_graph.GetHistogram().GetListOfFunctions().Add(axis_zbi)

    # Update the canvas
    canvas.Update()

    # Return the canvas
    return canvas

#def SetMyStyle(tsize=0.025, tzsize=0.025, font=42, setOptTitle=0, setOptStat=0, setOptFit=0):
def SetMyStyle(tsize=0.05, tzsize=0.05, font=42, setOptTitle=0, setOptStat=0, setOptFit=0):
    print("SETTING MY STYLE")

    colors = getColorsHPS()
    r.gROOT.SetBatch(1)

    myStyle = r.TStyle("myStyle", "my style")

    # Set your custom attributes here
    myStyle.SetOptTitle(setOptTitle)
    myStyle.SetOptStat(setOptStat)
    myStyle.SetOptFit(setOptFit)
    myStyle.SetTitleFont(font)
    myStyle.SetTitleSize(tsize)
    #myStyle.SetTitleX(0.5)
    #myStyle.SetTitleY(0.98)

    #Set legend text size
    myStyle.SetLegendTextSize(0.02)

    # Set the title text color to black
    myStyle.SetTitleTextColor(r.kBlack)

    # use plain black on white colors
    icol = 0
    myStyle.SetFrameBorderMode(icol)
    myStyle.SetCanvasBorderMode(icol)
    myStyle.SetPadBorderMode(icol)
    myStyle.SetPadColor(icol)
    myStyle.SetCanvasColor(icol)
    myStyle.SetStatColor(icol)

    # set the paper & margin sizes
    myStyle.SetPaperSize(20, 26)
    myStyle.SetPadTopMargin(0.10)
    myStyle.SetPadRightMargin(0.05)
    myStyle.SetPadRightMargin(0.10)
    myStyle.SetPadBottomMargin(0.15)
    myStyle.SetPadLeftMargin(0.10)

    myStyle.SetTextSize(tsize)
    myStyle.SetLabelFont(font, "x")
    myStyle.SetTitleFont(font, "x")
    myStyle.SetLabelFont(font, "y")
    myStyle.SetTitleFont(font, "y")
    myStyle.SetLabelFont(font, "z")
    myStyle.SetTitleFont(font, "z")
    
    myStyle.SetLabelSize(tsize, "x")
    myStyle.SetTitleSize(tsize, "x")
    myStyle.SetLabelSize(tsize, "y")
    myStyle.SetTitleSize(tsize, "y")
    myStyle.SetLabelSize(tzsize, "z")
    myStyle.SetTitleSize(tzsize, "z")

    myStyle.SetTitleOffset(1.25, "y")
    myStyle.SetTitleOffset(1.5, "x")

    #use bold lines and markers
    myStyle.SetMarkerSize(1.0)
    myStyle.SetMarkerStyle(8)
    myStyle.SetMarkerColor(1)
    myStyle.SetLineColor(1)
    myStyle.SetHistLineWidth(3)
    #myStyle.SetLineStyleString(2, "[12 12]")  # postscript dashes

    # put tick marks on top and RHS of plots
    #myStyle.SetPadTickX(1)
    #myStyle.SetPadTickY(1)
    
    r.gROOT.SetStyle("myStyle")
    r.gROOT.ForceStyle()

    NRGBs = 5
    NCont = 255

    stops = array("d", [0.00, 0.34, 0.61, 0.84, 1.00])
    red = array("d", [0.00, 0.00, 0.87, 1.00, 0.51])
    green = array("d", [0.00, 0.81, 1.00, 0.20, 0.00])
    blue = array("d", [0.51, 1.00, 0.12, 0.00, 0.00])
    r.TColor.CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont)

    return myStyle



def buildLegend(graphs,titles=[], position=(0.50,0.6,0.85,0.9),clear_legend=True, text_size=0.030, entry_format=None):
    legend = r.TLegend(*position)
    #legend = r.TLegend()
    # Set the legend to transparent (clear) if the option is specified
    if clear_legend:
        legend.SetFillStyle(0)
        legend.SetFillColor(0)
        legend.SetLineColor(0)
        legend.SetBorderSize(0)
    legend.SetTextSize(text_size)

    for i,graph in enumerate(graphs):
        if entry_format is None:
            if len(titles) < 1:
                legend.AddEntry(graph, graph.GetTitle())
            else:
                legend.AddEntry(graph, titles[i])
        else:
            if len(titles) < 1:
                legend.AddEntry(graph, graph.GetTitle(),"%s"%(entry_format[i]))
            else:
                legend.AddEntry(graph, titles[i],"%s"%(entry_format[i]))
    return legend


def setTitle(hist, title=None, titlex=None, titley=None, showTitle=False):
    if title is not None:
        hist.SetTitle(title)
    if titlex is not None:
        hist.GetXaxis().SetTitle(titlex)
    if titley is not None:
        hist.GetYaxis().SetTitle(titley)
    if showTitle:
        r.gStyle.SetOptTitle(1)
    else:
        r.gStyle.SetOptTitle(0)
        
def drawStatsBox(hist, statsPos=[0.7,0.9,0.7,0.9]):
        statsbox = hist.FindObject("stats")
        statsbox.SetX1NDC(statsPos[0])
        statsbox.SetX2NDC(statsPos[1])
        statsbox.SetY1NDC(statsPos[2])
        statsbox.SetY2NDC(statsPos[3])
    
def drawTH2(hist, canvas_name, drawOpt='colz', xrange = (None,None), yrange=(None,None),rebinx=None, 
            rebiny=None, size=(2200,1600), logX=False, logY=False, logZ=False, save=False, outdir='.',
            drawStats=False, statsPos=[0.7,0.9,0.7,0.9],
            text=[], text_pos = [0.2, 0.8], line_spacing=0.03, text_size=0.025, Hps=True):
    
    c = r.TCanvas(f'{canvas_name}',f'{canvas_name}',size[0], size[1])
    c.cd() 
    r.gROOT.ForceStyle() 
    hist.Draw(f'{drawOpt}')
    r.gPad.Update()
    c.UseCurrentStyle()
    if rebinx is not None:
        hist.RebinX(rebinx)
    if rebiny is not None:
        hist.RebinY(rebiny)
    
    if drawStats:
        drawStatsBox(hist, statsPos)
    else:
        hist.SetStats(0)
        
    #Set Xrange
    xmin = hist.GetXaxis().GetXmin()
    xmax = hist.GetXaxis().GetXmax()
    if xrange[0] is not None:
        xmin = xrange[0]
    if xrange[1] is not None:
        xmax = xrange[1]
    hist.GetXaxis().SetRangeUser(0.9*xmin,1.1*xmax)
    
    #Set Yrange
    ymin = hist.GetYaxis().GetXmin()
    ymax = hist.GetYaxis().GetXmax()
    if yrange[0] is not None:
        ymin = yrange[0]
    if yrange[1] is not None:
        ymax = yrange[1]
    hist.GetYaxis().SetRangeUser(0.9*ymin,1.1*ymax)
    
    #Set Log
    if logX:
        c.SetLogx(1)
    else:
        c.SetLogx(0)
    if logY:
        c.SetLogy(1)
    else:
        c.SetLogy(0)
    if logZ:
        c.SetLogz(1)
    else:
        c.SetLogz(0)
        
    #Draw latex text
    if len(text) > 0 or Hps:
        drawText = insertText(text, text_pos, line_spacing, text_size, Hps)
        drawText.Draw()
        r.gPad.Update()
        
    if save:
        c.SaveAs(f'{outdir}/{canvas_name}.png')
        c.Close()
    else:
        return c
    
def insertText(text=[], text_pos = [0.2, 0.98], line_spacing=0.04, text_size=0.035, Hps=True):
    latex = r.TLatex()
    latex.SetTextSize(text_size)
    text_x = text_pos[0]
    text_y = text_pos[1]
    if (Hps):
        latex.DrawLatexNDC(text_x, text_y,'#bf{#it{HPS}} Internal')
        text_y = text_y - line_spacing

    for line in text:
        latex.DrawLatexNDC(text_x, text_y,line)
        text_y = text_y - line_spacing
    return latex

def drawTH1s(histograms, canvas_name, drawOpts=[], xrange = (None,None), yrange=(None,None),rebinx=None, legend=None,
            rebiny=None, size=(1280,720), logX=False, logY=False, logZ=False, save=False, outdir='.',
            drawStats=False, statsPos=[0.7,0.9,0.7,0.9], text=[], text_pos = [0.15, 0.8], line_spacing=0.03, text_size=0.025, Hps=True, freezeXaxis=True):
    
    c = r.TCanvas(f'{canvas_name}',f'{canvas_name}',size[0], size[1])
    c.cd() 
    r.gROOT.ForceStyle() 
    c.UseCurrentStyle()
    
    # Find the maximum x and y values among all histograms
    min_x = min(h.GetBinLowEdge(h.FindFirstBinAbove(0.0)) for h in histograms)
    max_x, max_y = max(h.GetBinLowEdge(h.FindLastBinAbove(0.0)) + h.GetBinWidth(0) for h in histograms), max(h.GetMaximum() for h in histograms)
    min_y = 1e10
    for h in histograms:
        local_miny = 1e10
        min_y_l = h.GetBinContent(h.FindFirstBinAbove(0.0))
        min_y_u = h.GetBinContent(h.FindLastBinAbove(0.0))
        if min_y_l < min_y_u:
            local_miny = min_y_l
        else:
            local_miny = min_y_u
        if local_miny < min_y:
            min_y = local_miny
        if(freezeXaxis == False):
            h.SetAxisRange(0.95*min_x, 1.05*max_x, "X")
            h.GetXaxis().SetRangeUser(0.95*min_x,1.05*max_x)
        h.SetAxisRange(min_y, 1.05 * max_y, "Y")
        h.GetYaxis().SetRangeUser(min_y, 1.05 * max_y)
            
    for i, gr in enumerate(histograms):
        if xrange[0] is not None and xrange[0] is not None:
            gr.GetXaxis().SetRangeUser(0.95*xrange[0], 1.05*xrange[1])
        if yrange[0] is not None:
            min_y = yrange[0]
        if yrange[1] is not None:
            max_y = yrange[1]
        if min_y < 0:
            gr.GetYaxis().SetRangeUser(0.0, 1.05*max_y)
        else:
            gr.GetYaxis().SetRangeUser(0.95*min_y, 1.05*max_y)
        
        #Draw histograms
        r.gPad.Update()
        if i < 1:
            gr.Draw('%s'%(drawOpts[i]))
            gr.SetLineColor(colors[i])
            r.gPad.Update()
            if drawStats:
                drawStatsBox(gr, statsPos)
            else:
                gr.SetStats(0)
        else:
            gr.Draw('%sSAME'%(drawOpts[i]))
            gr.SetLineColor(colors[i])
            r.gPad.Update()
            if drawStats:
                drawStatsBox(gr, statsPos)
            else:
                gr.SetStats(0)
        
        #Draw fit functions if present
        if len(gr.GetListOfFunctions()) > 0:
            func_name = gr.GetListOfFunctions().At(0).GetName()
            func = gr.GetFunction("%s"%(func_name))
            func.Draw("%sSAME"%(drawOpts[i]))
            func.SetLineColor(r.kRed)
            
    #Draw legend
    if legend is not None:
        legend.Draw()
        c.Update()
        
        #Set Log
    if logX:
        c.SetLogx(1)
    else:
        c.SetLogx(0)
    if logY:
        c.SetLogy(1)
    else:
        c.SetLogy(0)
    if logZ:
        c.SetLogz(1)
    else:
        c.SetLogz(0)
        
    #Draw latex text
    if len(text) > 0 or Hps:
        drawText = insertText(text, text_pos, line_spacing, text_size, Hps)
        drawText.Draw()
        c.Update()
        
    c.Draw()
    if save:
        c.SaveAs(f'{outdir}/{canvas_name}.png')
        c.Close()
    else:
        return c

def drawTGraphs(graphs, canvas_name, drawOpts=[], xrange = (None,None), yrange=(None,None),rebinx=None, legend=None,
            rebiny=None, size=(2200,1600), logX=False, logY=False, logZ=False, save=False, outdir='.',
            drawStats=False, statsPos=[0.7,0.9,0.7,0.9], text=[], text_pos = [0.15, 0.8], line_spacing=0.03, text_size=0.025, Hps=True):
    
    c = r.TCanvas(f'{canvas_name}',f'{canvas_name}',size[0], size[1])
    c.cd() 
    r.gROOT.ForceStyle() 
    c.UseCurrentStyle()

    #Set Range
    ymin = 9999.9
    ymax = -9999.9
    for i, gr in enumerate(graphs):
        num_points = gr.GetN()
        y_values = np.array([gr.GetY()[i] for i in range(num_points)])
        local_ymax = np.max(y_values)
        if local_ymax > ymax:
            ymax = local_ymax
        local_ymin = np.min(y_values)
        if local_ymin < ymin:
            ymin = local_ymin
            
    for i, gr in enumerate(graphs):
        if xrange[0] is not None and xrange[0] is not None:
            gr.GetHistogram().GetXaxis().SetRangeUser(0.9*xrange[0], 1.1*xrange[1])
        if yrange[0] is not None:
            ymin = yrange[0]
        if yrange[1] is not None:
            ymax = yrange[1]
        if ymin < 0:
            gr.GetHistogram().GetYaxis().SetRangeUser(1.2*ymin, 1.2*ymax)
        else:
            gr.GetHistogram().GetYaxis().SetRangeUser(0.8*ymin, 1.2*ymax)
        
        #Draw Graphs
        if i < 1:
            gr.Draw('A%s'%(drawOpts[i]))
            r.gPad.Update()
            if drawStats:
                drawStatsBox(gr, statsPos)
            else:
                gr.GetHistogram().SetStats(0)
        else:
            gr.Draw('%sSAME'%(drawOpts[i]))
            r.gPad.Update()
            if drawStats:
                drawStatsBox(gr, statsPos)
            else:
                gr.GetHistogram().SetStats(0)
        
        #Draw fit functions if present
        if len(gr.GetListOfFunctions()) > 0:
            func_name = gr.GetListOfFunctions().At(0).GetName()
            func = gr.GetFunction("%s"%(func_name))
            func.Draw("%sSAME"%(drawOpts[i]))
            
    #Draw legend
    if legend is not None:
        legend.Draw()
        c.Update()
        
        #Set Log
    if logX:
        c.SetLogx(1)
    else:
        c.SetLogx(0)
    if logY:
        c.SetLogy(1)
    else:
        c.SetLogy(0)
    if logZ:
        c.SetLogz(1)
    else:
        c.SetLogz(0)
        
    #Draw latex text
    if len(text) > 0 or Hps:
        drawText = insertText(text, text_pos, line_spacing, text_size, Hps)
        drawText.Draw()
        c.Update()
        
    c.Draw()
    if save:
        c.SaveAs(f'{outdir}/{canvas_name}.png')
        c.Close()
    else:
        return c

def readROOTHisto(infilename, histoname):
    infile = r.TFile(f'{infilename}',"READ")
    histo = copy.deepcopy(infile.Get(f'{histoname}'))
    infile.Close()
    return histo

def cnvHistosToROOT(histos=[], tempname='temporary_uproot'):
    return_histos = []
    uproot_file = uproot.recreate(f'trash_{tempname}.root')
    for i, histo in enumerate(histos):
        uproot_file[f'histo_{i}'] = histo
    uproot_file.close()
    infile = r.TFile(f'trash_{tempname}.root',"READ")
    for i, histo in enumerate(histos):
        return_histos.append(copy.deepcopy(infile.Get(f'histo_{i}')))
    infile.Close()
    return return_histos

def cnvHistogramToROOT(histo, tempname='temporary_uproot'):
    uproot_file = uproot.recreate(f'trash_{tempname}.root')
    uproot_file['histogram'] = histo
    root_hist = readROOTHisto(f'trash_{tempname}.root', 'histogram;1')
    uproot_file.close()
    return root_hist

def cnvHistoToROOT(histoname, histo, tempname='temporary_uproot'):
    uproot_file = uproot.recreate(f'trash_{tempname}.root')
    uproot_file[histoname] = histo
    infile = r.TFile(f'trash_{tempname}.root',"READ")
    root_hist = copy.deepcopy(infile.Get(f'{histoname}'))
    infile.Close()
    uproot_file.close()
    return root_hist

def quickDraw(plot, name='c', drawOpts=""):
    c = r.TCanvas(name,name,1400,700)
    c.cd()
    plot.Draw(drawOpts)
    c.Draw()
    return c

def format_th1(histo, name=None, title=None, xlabel=None, ylabel=None, linecolor=None, linewidth=None, markerstyle=None, 
        markercolor=None, xrang=(None, None), yrang=(None,None), rebin=None, labelsize=None,
        titlesize=None, xtitleoffset=None, ytitleoffset=None):
    if name:
        histo.SetName(name)
    if title:
        histo.SetTitle(title)
    if xlabel:
        histo.GetXaxis().SetTitle(xlabel)
    if ylabel:
        histo.GetYaxis().SetTitle(ylabel)
    if linecolor:
        histo.SetLineColor(linecolor)
    if linewidth:
        histo.SetLineWidth(linewidth)
    if markerstyle:
        histo.SetMarkerStyle(markerstyle)
    if markercolor:
        histo.SetMarkerColor(markercolor)
    if xrang[0]:
        histo.GetXaxis().SetRangeUser(xrang[0],xrang[1])
    if yrang[0]:
        histo.GetYaxis().SetRangeUser(yrang[0], yrang[1])
    if rebin:
        histo.Rebin(rebin)
    if labelsize:
        histo.GetXaxis().SetLabelSize(labelsize)
        histo.GetYaxis().SetLabelSize(labelsize)
    if titlesize:
        histo.GetXaxis().SetTitleSize(titlesize)
        histo.GetYaxis().SetTitleSize(titlesize)
    if xtitleoffset:
        histo.GetXaxis().SetTitleOffset(xtitleoffset)
    if ytitleoffset:
        histo.GetYaxis().SetTitleOffset(ytitleoffset)

def plot_ratio_th1s(top_plots, bot_plots, cname, size=(1800,1400), top_drawOpts="pe", bot_drawOpts="pe", 
        topx1=0.0, topy1=0.4, topx2=1.0, topy2=1.0, top_logY=False,
        botx1=0.0, boty1=0.0, botx2=1.0, boty2=0.4, bot_logY=False,
        top_botmarg=0.1, top_topmarg=None, bot_topmarg=None, bot_botmarg=0.1, leftmarg=0.1):

    c = r.TCanvas(cname, cname, size[0], size[1])
    top = r.TPad("%s_top"%(cname), "%s_top"%(cname), topx1, topy1, topx2, topy2)
    top.SetBottomMargin(top_botmarg)
    top.SetLeftMargin(leftmarg)
    if top_topmarg:
        top.SetTopMargin(top_topmarg)
    top.Draw()



    bot = r.TPad("%s_bot"%(cname), "%s_bot"%(cname), botx1, boty1, botx2, boty2)
    bot.SetBottomMargin(bot_botmarg)
    bot.SetLeftMargin(leftmarg)
    if bot_topmarg:
        bot.SetTopMargin(bot_topmarg)
    bot.Draw()

    top.cd()
    for i,plot in enumerate(top_plots):
        if i < 1:
            plot.Draw(top_drawOpts)
        else:
            plot.Draw("%ssame"%(top_drawOpts))

    r.gPad.SetLogy(top_logY)


    bot.cd()
    for i,plot in enumerate(bot_plots):
        if i < 1:
            plot.Draw(bot_drawOpts)

        else:
            plot.Draw("%ssame"%(bot_drawOpts))

        #Draw fit functions if present
        if len(plot.GetListOfFunctions()) > 0:
            func_name = plot.GetListOfFunctions().At(0).GetName()
            func = plot.GetFunction("%s"%(func_name))
            func.Draw("%ssame"%(bot_drawOpts))

    return c, top, bot



def draw_th1(histo, cname, size=(1800,1400), drawOpts="", logY=False, color=None):

    c = r.TCanvas(cname, cname, size[0], size[1])
    c.cd()
    if color:
        histo.SetLineColor(color)
    histo.Draw("%s"%(drawOpts))

    #Draw fit functions if present
    if len(histo.GetListOfFunctions()) > 0:
        func_name = histo.GetListOfFunctions().At(0).GetName()
        func = histo.GetFunction("%s"%(func_name))
        if color:
            func.SetLineColor(color)
        func.Draw("same")

    if logY:
        c.SetLogy(1)

    return c

style = SetMyStyle(setOptStat=0)
colors = getColorsHPS()

