import glob
import pprint

import ROOT
ROOT.gROOT.SetBatch(ROOT.kTRUE)
ROOT.gStyle.SetOptStat(0)

def get_hist(filename,histname='preselTrks/preselTrks_chi2ndf_h'):
    tfile = ROOT.TFile.Open(filename,"READ")
    hist = tfile.Get(histname)
    if not hist:
        return 0
    hist.SetDirectory(0)
    return hist

def get_canvas(name):

    can = ROOT.TCanvas(name,name,800,800)
    can.SetBorderSize(0);
    can.SetFillColor(ROOT.kWhite);
    can.SetTickx();
    can.SetTicky();
    can.SetBottomMargin(0.12)
    can.SetTopMargin(0.10)
    can.SetLeftMargin(0.15)
    ROOT.gStyle.SetOptStat(0)
    return can

def style_hist(hist,yaxis,marker,color):
    hist.GetXaxis().SetRangeUser(10,15)
    hist.GetXaxis().SetNdivisions(5)
    hist.GetYaxis().SetRangeUser(0,2.25)
    hist.GetYaxis().SetTitleSize(0.05)
    hist.GetYaxis().SetLabelSize(0.05)
    hist.GetXaxis().SetTitleSize(0.05)
    hist.GetXaxis().SetLabelSize(0.05)
    hist.GetYaxis().SetTitleOffset(1.32)
    hist.GetYaxis().SetTitle(yaxis)
    hist.SetMarkerStyle(marker)
    hist.SetMarkerColor(color)
    hist.SetLineColor(color)
    hist.SetLineWidth(2)
    hist.SetMarkerSize(2)
    hist.SetTitle("")


filename="/sdf/data/hps/users/mgignac/software/smearing/hpstr/plotUtils/out_v9.root"
#/sdf/data/hps/users/mgignac/software/smearing/hpstr/plotUtils/TrackSmearing_inclusive.root"
filename8="/sdf/data/hps/users/mgignac/software/smearing/hpstr/plotUtils/TrackSmearing_beam8um.root"
filename15="/sdf/data/hps/users/mgignac/software/smearing/hpstr/plotUtils/TrackSmearing_beam15um.root"

#inc = get_hist(filename,"KalmanFullTracks_p_vs_nHits_hh_smearing")
#inc8 = get_hist(filename8,"KalmanFullTracks_p_vs_nHits_hh_smearing")

data = get_hist(filename,"KalmanFullTracks_p_vs_nHits_top_hh_sigma_data")
datab = get_hist(filename,"KalmanFullTracks_p_vs_nHits_bot_hh_sigma_data")

#inc = get_hist(filename,"KalmanFullTracks_p_vs_nHits_hh_sigma_mc")
#inc8 = get_hist(filename8,"KalmanFullTracks_p_vs_nHits_hh_sigma_mc")
#inc15 = get_hist(filename15,"KalmanFullTracks_p_vs_nHits_hh_sigma_mc")

#top = get_hist(filename, "KalmanFullTracks_p_vs_nHits_top_hh_smearing")
#bot = get_hist(filename,"KalmanFullTracks_p_vs_nHits_bot_hh_smearing")

can = get_canvas("")

style_hist(data,"#sigma [GeV]",8,1)
style_hist(datab,"#sigma [GeV]",8,2)
#style_hist(inc,"Smearing parameter #Sigma",8,2)
#style_hist(inc8,"Smearing parameter #Sigma",8,4)
#style_hist(inc15,"Smearing parameter #Sigma",8,6)
#style_hist(top,"Smearing parameter #Sigma",8,2)
#style_hist(bot,"Smearing parameter #Sigma",8,3)

leg = ROOT.TLegend(0.30,0.68,0.57,0.85)
leg.SetBorderSize(0);
leg.SetFillColor(ROOT.kWhite);
leg.SetTextSize(0.05);

leg.AddEntry(data,"Data: Top","L")
leg.AddEntry(datab,"Data: Bottom","L")
#leg.AddEntry(inc,"Unsmeared MC","L")
#leg.AddEntry(inc8,"Hit level smearing [8um]","L")
#leg.AddEntry(inc15,"Hit level smearing [15um]","L")
#leg.AddEntry(top,"Top","L")
#leg.AddEntry(bot,"Bottom","L")

data.Draw("hist")
datab.Draw("hist same")
#inc.Draw("hist same")
#inc8.Draw("hist same")
#inc15.Draw("hist same")
#top.Draw("hist same")
#bot.Draw("hist same")
leg.Draw("same")
can.SaveAs("track_smearing_sigma_topVsBottom_nHits.png")
