from ROOT import *
from utilities import *
import os

#Simple macro for producing the run-by-run vertex
#transverse positions and sigma from the fit results.
# input: simple txt file with columns:
#        run vxMean vxSigma vyMean vySigma ProjVxMean ProjVxSigma ProjVyMean ProjVySigma

SetStyle()


def MakeRunByRunPlot(calibFile="beam_params.txt", name="graph",
                     columns=[1, 2, 3], ytitle="vx_mean",
                     color=1, outroot=None, plotError=False):

    print calibFile
    vgraph = TGraphErrors()
    vgraph.SetName(name)
    vgraph.SetTitle("")
    vgraph.SetMarkerStyle(20)
    vgraph.SetMarkerSize(1)
    vgraph.SetMarkerColor(color)
    vgraph.SetLineColor(color)
    vgraph.SetLineWidth(2)
    ipoint = 0

    x_mc = -999.
    y_mc = -999.
    sigma_mc = -999.

    for line in calibFile.readlines():
        if ("#" in line):
            continue
        list_line = (line.strip()).split()
        x = list_line[columns[0]]
        y = list_line[columns[1]]
        sigmay = list_line[columns[2]]

        if (x == "5772"):
            x_mc = x
            y_mc = y
            sigma_mc = sigmay
        else:

            vgraph.SetPoint(ipoint, float(x), float(y))
            if plotError:
                vgraph.SetPointError(ipoint, 0, float(sigmay))
            ipoint += 1

    c = TCanvas()
    c.SetGridx(1)
    c.SetGridy(1)
    vgraph.Draw("AP")
    vgraph.GetXaxis().SetTitle("Run number")
    vgraph.GetYaxis().SetTitle(ytitle)
    outroot.cd()
    c.Write("c_"+vgraph.GetName())
    vgraph.Write(vgraph.GetName())
    calibFile.seek(0)
    return (c, vgraph)


outroot = TFile("beam_parameters.root", "RECREATE")
beamParamsFile = os.environ['HPSTR_BASE']+"/analysis/data/runByrun_VxVy_calibration.txt"
infile = open(beamParamsFile)

tuple_xm = MakeRunByRunPlot(infile, "vx_mean",
                            [0, 1, 2], "<V_{x}> [mm]",
                            kBlack, outroot)

tuple_xs = MakeRunByRunPlot(infile, "vx_sigma",
                            [0, 2, 1], "#sigma_{V_{x}} [mm]",
                            kBlack, outroot)

tuple_ym = MakeRunByRunPlot(infile, "vy_mean",
                            [0, 3, 4], "<V_{y}> [mm]",
                            kBlue, outroot)

tuple_ys = MakeRunByRunPlot(infile, "vy_sigma",
                            [0, 4, 3], "#sigma_{V_{y}} [mm]",
                            kBlue, outroot)

tuple_zm = MakeRunByRunPlot(infile, "vz_mean",
                            [0, 5, 6], "<V_{z}> [mm]",
                            kRed, outroot)

tuple_zs = MakeRunByRunPlot(infile, "vz_sigma",
                            [0, 6, 5], "#sigma_{V_{z}} [mm]",
                            kRed, outroot)

tuple_vxmp = MakeRunByRunPlot(infile, "vx_mean_proj",
                              [0, 7, 8], "Target Proj <V_{x}> [mm]",
                              kBlack, outroot)

tuple_vxsp = MakeRunByRunPlot(infile, "vx_sigma_proj",
                              [0, 8, 7], "Target Proj #sigma_{V_{x}} [mm]",
                              kBlack, outroot)

tuple_vymp = MakeRunByRunPlot(infile, "vy_mean_proj",
                              [0, 9, 10], "Target Proj <V_{y}> [mm]",
                              kBlue, outroot)

tuple_vysp = MakeRunByRunPlot(infile, "vy_sigma_proj",
                              [0, 10, 9], "Target Proj #sigma_{V_{y}} [mm]",
                              kBlue, outroot)

#MC values
all_lines = infile.readlines()
mcline = ((all_lines[1]).strip()).split()
mcrun = mcline[0]
mcVxmean = float(mcline[1])
mcVxsigma = float(mcline[2])
mcVymean = float(mcline[3])
mcVySigma = float(mcline[4])
mcVzmean = float(mcline[5])
mcVzsigma = float(mcline[6])
mcVxPtgt = float(mcline[7])
mcVxsptgt = float(mcline[8])
mcVyptgt = float(mcline[9])
mcVysptgt = float(mcline[10])


#Make text
text = TLatex()
text.SetNDC()
text.SetTextFont(42)
text.SetTextSize(0.05)
text.SetTextColor(kBlack)


#Draw Z plot
can = TCanvas()
can.SetGridx(1)
can.SetGridy(1)
vz_graph = tuple_zm[1]
vz_graph.GetYaxis().SetRangeUser(-4.4, -3.8)
vz_graph.Draw("AP")
leg = TLegend(0.15, 0.85, 0.4, 0.75)
entry = leg.AddEntry(vz_graph, "z pos data", "p")
entry = leg.AddEntry(vz_graph, "z pos mc", "l")
leg.SetBorderSize(0)
leg.Draw()
lineZ = TLine(vz_graph.GetXaxis().GetXmin(), mcVzmean, vz_graph.GetXaxis().GetXmax(), mcVzmean)
lineZ.SetLineColor(kRed)
lineZ.SetLineWidth(2)
lineZ.Draw("same")
text.DrawLatex(0.17, 0.90, '#bf{#it{HPS} Internal}')
outroot.cd()
can.Write("z_final")
can.SaveAs("z_final.pdf")

#Draw XY Plot

can = TCanvas()
can.SetGridx(1)
can.SetGridy(1)
vx_graph = tuple_xm[1]
vx_graph.GetYaxis().SetRangeUser(-0.3, 0.3)
vx_graph.GetYaxis().SetTitle("Mean transverse position [mm]")
vy_graph = tuple_ym[1]
vx_graph.Draw("AP")
vy_graph.Draw("Psame")

leg = TLegend(0.15, 0.8, 0.4, 0.6)
entry = leg.AddEntry(vx_graph, "<x> pos data", "p")
entry = leg.AddEntry(vx_graph, "<x> pos mc", "l")
entry = leg.AddEntry(vy_graph, "<y> pos data", "p")
entry = leg.AddEntry(vy_graph, "<y> pos mc", "l")
leg.SetBorderSize(0)
leg.Draw()

lineX = TLine(vx_graph.GetXaxis().GetXmin(), mcVxmean, vx_graph.GetXaxis().GetXmax(), mcVxmean)
lineX.SetLineColor(kBlack)
lineX.SetLineWidth(2)
lineX.Draw("same")

lineY = TLine(vx_graph.GetXaxis().GetXmin(), mcVymean, vx_graph.GetXaxis().GetXmax(), mcVymean)
lineY.SetLineColor(kBlack)
lineY.SetLineWidth(2)
lineY.Draw("same")


text = TLatex()
text.SetNDC()
text.SetTextFont(42)
text.SetTextSize(0.05)
text.SetTextColor(kBlack)
text.DrawLatex(0.17, 0.90, '#bf{#it{HPS} Internal}')

can.Write("xy_final")
can.SaveAs("xy_final.pdf")


#Draw XY Plot Target Projection

can = TCanvas()
can.SetGridx(1)
can.SetGridy(1)
vx_graph = tuple_vxmp[1]
vx_graph.GetYaxis().SetRangeUser(-0.3, 0.3)
vx_graph.GetYaxis().SetTitle("Mean transverse Target Proj [mm]")
vy_graph = tuple_vymp[1]
vx_graph.Draw("AP")
vy_graph.Draw("Psame")

leg = TLegend(0.15, 0.8, 0.4, 0.6)
entry = leg.AddEntry(vx_graph, "<x> proj data", "p")
entry = leg.AddEntry(vx_graph, "<x> proj mc", "l")
entry = leg.AddEntry(vy_graph, "<y> proj data", "p")
entry = leg.AddEntry(vy_graph, "<y> proj mc", "l")
leg.SetBorderSize(0)
leg.Draw()

lineX = TLine(vx_graph.GetXaxis().GetXmin(), mcVxPtgt, vx_graph.GetXaxis().GetXmax(), mcVxPtgt)
lineX.SetLineColor(kBlack)
lineX.SetLineWidth(2)
lineX.Draw("same")

lineY = TLine(vx_graph.GetXaxis().GetXmin(), mcVyptgt, vx_graph.GetXaxis().GetXmax(), mcVyptgt)
lineY.SetLineColor(kBlue)
lineY.SetLineWidth(2)
lineY.Draw("same")

text = TLatex()
text.SetNDC()
text.SetTextFont(42)
text.SetTextSize(0.05)
text.SetTextColor(kBlack)
text.DrawLatex(0.17, 0.90, '#bf{#it{HPS} Internal}')

can.Write("xy_proj_final")
can.SaveAs("xy_proj_final.pdf")


#Draw XY Unc Sigma plot

can = TCanvas()
can.SetGridx(1)
can.SetGridy(1)
vx_graph = tuple_xs[1]
vx_graph.GetYaxis().SetRangeUser(0., 0.4)
vx_graph.GetYaxis().SetTitle("Transverse Position #sigma [mm]")
vy_graph = tuple_ys[1]
vx_graph.Draw("AP")
vy_graph.Draw("Psame")

leg = TLegend(0.15, 0.6, 0.4, 0.4)
entry = leg.AddEntry(vx_graph, "#sigma_{x} pos data", "p")
entry = leg.AddEntry(vx_graph, "#sigma_{x} pos mc", "l")
entry = leg.AddEntry(vy_graph, "#sigma_{y} pos data", "p")
entry = leg.AddEntry(vy_graph, "#sigma_{y} pos mc", "l")
leg.SetBorderSize(0)
leg.Draw()

lineX = TLine(vx_graph.GetXaxis().GetXmin(), mcVxsigma, vx_graph.GetXaxis().GetXmax(), mcVxsigma)
lineX.SetLineColor(kBlack)
lineX.SetLineWidth(2)
lineX.Draw("same")

lineY = TLine(vx_graph.GetXaxis().GetXmin(), mcVySigma, vx_graph.GetXaxis().GetXmax(), mcVySigma)
lineY.SetLineColor(kBlue)
lineY.SetLineWidth(2)
lineY.Draw("same")

text = TLatex()
text.SetNDC()
text.SetTextFont(42)
text.SetTextSize(0.05)
text.SetTextColor(kBlack)
text.DrawLatex(0.17, 0.90, '#bf{#it{HPS} Internal}')

can.Write("sigma_xy_final")
can.SaveAs("sigma_xy_final.pdf")


#Draw XY TGT Proj Sigma plot

can = TCanvas()
can.SetGridx(1)
can.SetGridy(1)
vx_graph = tuple_vxsp[1]
vx_graph.GetYaxis().SetRangeUser(0., 0.4)
vx_graph.GetYaxis().SetTitle("Transverse Target Projection #sigma [mm]")
vy_graph = tuple_vysp[1]
vx_graph.Draw("AP")
vy_graph.Draw("Psame")

leg = TLegend(0.15, 0.6, 0.4, 0.4)
entry = leg.AddEntry(vx_graph, "#sigma_{x} proj data", "p")
entry = leg.AddEntry(vx_graph, "#sigma_{x} proj mc", "l")
entry = leg.AddEntry(vy_graph, "#sigma_{y} proj data", "p")
entry = leg.AddEntry(vy_graph, "#sigma_{y} proj mc", "l")
leg.SetBorderSize(0)
leg.Draw()

lineX = TLine(vx_graph.GetXaxis().GetXmin(), mcVxsptgt, vx_graph.GetXaxis().GetXmax(), mcVxsptgt)
lineX.SetLineColor(kBlack)
lineX.SetLineWidth(2)
lineX.Draw("same")

lineY = TLine(vx_graph.GetXaxis().GetXmin(), mcVysptgt, vx_graph.GetXaxis().GetXmax(), mcVysptgt)
lineY.SetLineColor(kBlue)
lineY.SetLineWidth(2)
lineY.Draw("same")

text = TLatex()
text.SetNDC()
text.SetTextFont(42)
text.SetTextSize(0.05)
text.SetTextColor(kBlack)
text.DrawLatex(0.17, 0.90, '#bf{#it{HPS} Internal}')

can.Write("sigma_xy_proj_final")
can.SaveAs("sigma_xy_proj_final.pdf")


outroot.Close()
infile.close()
