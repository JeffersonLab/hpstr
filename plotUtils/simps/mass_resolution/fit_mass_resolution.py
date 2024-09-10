#!/usr/bin/python3
"""
This script fits the MC signal mass resolution as a function of invariant mass.
The mass resolution uses SIMP signal reconstructed vertex invariant mass with radMatchTight selection (truth matched ele).
Fill a histogram with invariant mass and fit the distribution with a Gaussian fit function.
Fit the mass resolution as a function of invariant mass with a polynomial. Use p-test to find best fit order.
"""
import os
import awkward as ak
import numpy as np
import hist
from hist import Hist
import uproot
import ROOT as r
import matplotlib.pyplot as plt
import matplotlib as mpl

# SIMP tools defined in hpstr
import sys
hpstr_base = os.getenv('HPSTR_BASE')
sys.path.append(f'{hpstr_base}/plotUtils/simps')
import simp_plot_utils as utils

plt.rcParams.update({'font.size': 40,           # Font size for text
                     'axes.titlesize': 40,      # Font size for titles
                     'axes.labelsize': 40,      # Font size for axis labels
                     'xtick.labelsize': 40,     # Font size for x-axis tick labels
                     'ytick.labelsize': 40,     # Font size for y-axis tick labels
                     'lines.linewidth':4.0,
                     'legend.fontsize': 40})    # Font size for legend

#=======================================================================================================================================
# INITIALIZE
#=======================================================================================================================================
# --outdir: Specify output directory.

import argparse
parser = argparse.ArgumentParser(description='Process some inputs.')
parser.add_argument('--outdir', type=str, default='moller_mass_fits')
parser.add_argument('--outfilename', type=str, default='mass_resolution')
args = parser.parse_args()
outdir = args.outdir
outfilename = args.outfilename

#=======================================================================================================================================
# FUNCTIONS
#=======================================================================================================================================

def load_signal(filepath, selection, cuts=None, expressions=None):
    #load signal with the hpstr ana processor flat tuple struct
    with uproot.open(filepath) as f:
        events = f[f'{selection}/{selection}_tree'].arrays(
            expressions = expressions, cut = cuts
        )
    return events

def gaus_fit(histo, xmin, xmax, smean, swidth, snorm, nsigma=2.0, isData=False):
    
    print('seeds:', xmin, xmax, smean, swidth, snorm)
    #initial fit with seeds
    fitfunc = r.TF1("gaus","gaus")
    fitfunc.SetParameter(0, snorm)
    fitfunc.SetParameter(1, smean)
    fitfunc.SetParameter(2, swidth)
    fitRes = histo.Fit(fitfunc,"QLES","", xmin, xmax)
    try:
        params = fitRes.Parameters()
        chi2 = fitRes.Chi2()
        ndf = fitRes.Ndf()
    
        best_chi2 = chi2/ndf
        best_params = params
    except:
        best_chi2 = 999999.9
        params = [snorm, smean, swidth]
        best_params=params
    
    niters = 100
    for n in range(niters):
        norm = params[0]#*np.random.uniform(80,120)*0.01
        mu = params[1]#*np.random.uniform(80,120)*0.01
        sigma = params[2]#*np.random.uniform(80,120)*0.01
        
        xminx = mu - nsigma*sigma
        xmaxx = mu + nsigma*sigma
        if isData:
            if xminx < xmin:
                xminx = xmin
            if xmaxx > xmax:
                xmaxx = xmax
        fitfunc.SetParameter(0, norm)
        fitfunc.SetParameter(1, mu)
        fitfunc.SetParameter(2, sigma)
        fitRes = histo.Fit(fitfunc,"QLES","", xminx, xmaxx)
        try:
            if fitRes.Parameters()[1] < xminx or fitRes.Parameters()[1] > xmaxx or fitRes.Ndf() < 1:
                continue
        except:
            continue
            
        params = fitRes.Parameters()
        #print(params)
        chi2 = fitRes.Chi2()
        ndf = fitRes.Ndf()
        if chi2/ndf < best_chi2:
            best_params = params
        
    fitfunc.SetParameter(0, best_params[0])
    fitfunc.SetParameter(1, best_params[1])
    fitfunc.SetParameter(2, best_params[2])
    xminx = best_params[1] - nsigma*best_params[2]
    xmaxx = best_params[1] + nsigma*best_params[2]
    
    if isData:
        if xminx < xmin:
            xminx = xmin
        if xmaxx > xmax:
            xmaxx = xmax
        
    print('result: ', xminx, xmaxx, best_params[1], best_params[2], best_params[0])
    fitRes = histo.Fit(fitfunc,"QLES","", xminx, xmaxx)
    params = fitRes.Parameters()
    errors = fitRes.Errors()
    chi2 = fitRes.Chi2()
    ndf = fitRes.Ndf()
    if ndf > 0:
        chi2 = chi2/ndf
    else:
        chi2 = 99999.9
    return histo, params, errors, chi2

def fit_with_poly(tgrapherrs):
    polys = []
    chi2s = []
    fstats = []
    fitResults = []
    npoints = len(tgrapherrs.GetX())
    
    for n in range(10):
        fitfunc = r.TF1(f'pol{n}',f'pol{n}', tgrapherrs.GetX()[0], tgrapherrs.GetX()[-1])
        fitfunc.SetRange(30.0, 124)
        fitfunc.SetLineColor(r.kRed)
        fitfunc.SetMarkerSize(0.0)
        fitRes = tgrapherrs.Fit(fitfunc,"SRQ")
        chi2s.append(fitRes.Chi2())
        polys.append(n)
        fitResults.append(fitRes)
        
        #Perform fstat test to see how much fit improves with additional order (why does this work?)
        if n > 0:
            fstats.append( (chi2s[n-1]-chi2s[n])*(npoints-n-1)/(chi2s[n]))
        else:
            fstats.append(0.0)
        
    #Pick the order that shows greatest positive improvement in fstat
    best_diff = 0.0
    best_n = None
    for n,fstat in enumerate(fstats):
        if n == 0:
            continue
        diff = fstats[n-1] - fstat
        if diff > 0 and diff > best_diff:
            best_diff = diff
            best_n = n
            
    print(f'best n: {best_n}')
    return fitResults[best_n] 

#=======================================================================================================================================
# LOAD MC SIGNAL
#=======================================================================================================================================

# Load MC signal with momentum smearing
indir = '/sdf/group/hps/user-data/alspellm/2016/simp_mc/pass4b/beam/smeared'
infilename = lambda mass: f'mass_{mass}_hadd-simp-beam_ana_smeared_corr.root'
# Masses of each MC generated signal mass 
masses = [x for x in range(30,126,2)]

# Specify any cuts. Here we put a cut on the number of hits on track based on the 2016 SIMP L1L1 analysis
inv_masses_h={}
cuts = '( (unc_vtx_psum > 1.0) & (unc_vtx_ele_track_nhits >= 7) & (unc_vtx_pos_track_nhits >= 7) )'
# Load each MC signal mass
for mass in masses:
    infile = os.path.join(indir, infilename(mass))
    selection = 'vtxana_radMatchTight_2016_simp_SR_analysis' # Use radMatchTight 
    branches = ['unc_vtx_mass', 'unc_vtx_ele_track_nhits', 'unc_vtx_pos_track_nhits']
    # Load events from hpstr vertex analysis processor
    tree = load_signal(infile,selection, expressions=branches, cuts=cuts) 
    
    # Initialize histogram
    inv_masses_h[f'mass_{mass}'] = (
        hist.Hist.new
        .Reg(800,17.5,217.5,label='Invariant Mass [MeV]')
        .Double()
    )
    
    # Fill histogram
    inv_masses_h[f'mass_{mass}'].fill(tree.unc_vtx_mass*1000.)
    
# Load MC signal WITHOUT momentum smearing
indir = '/sdf/group/hps/user-data/alspellm/2016/simp_mc/pass4b/beam/nosmearing'
infilename = lambda mass: f'mass_{mass}_hadd-simp-beam-ana-nosmearing.root'
inv_masses_unsm_h={}
for mass in masses:
    infile = os.path.join(indir, infilename(mass))
    selection = 'vtxana_radMatchTight_2016_simp_SR_analysis'
    branches = ['unc_vtx_mass' ]
    tree = load_signal(infile,selection, expressions=branches, cuts=cuts) 
    
    inv_masses_unsm_h[f'mass_{mass}'] = (
        hist.Hist.new
        .Reg(800,17.5,217.5,label='Invariant Mass [MeV]')
        .Double()
    )
    
    inv_masses_unsm_h[f'mass_{mass}'].fill(tree.unc_vtx_mass*1000.)
  

# Convert Histograms to ROOT for convenient fitting
for key, histo in inv_masses_h.items():
    h = utils.cnvHistogramToROOT(histo)
    h.SetName(f'{key}_smeared')
    inv_masses_h[key] = h
    
for key, histo in inv_masses_unsm_h.items():
    h = utils.cnvHistogramToROOT(histo)
    h.SetName(f'{key}_unsmeared')
    inv_masses_unsm_h[key] = h


# Initialize output ROOT file to save fit results
outfile = r.TFile(f'{outdir}/{outfilename}',"RECREATE")
outfile.cd()
fit_results = {}

#=======================================================================================================================================
# CALCULATE MC SIGNAL MASS RESOLUTION WITH SMEARING
#=======================================================================================================================================

# Run over smeared MC signal and fit
for mass, histo in inv_masses_h.items():  
    print(f'Fitting {mass}')
    fit_histo, params, errors, chi2ndf = gaus_fit(histo, histo.GetXaxis().GetBinLowEdge(histo.FindFirstBinAbove(0)), histo.GetXaxis().GetBinLowEdge(histo.FindLastBinAbove(0)), histo.GetMean(), histo.GetRMS(), histo.GetMaximum(), nsigma=2.0, isData=False)
     
    #Draw Fit result to canvas, and save to png and to root file
    text = [f'\mu = {round(params[1],2)}',f'\sigma = {round(params[2],2)}',f'\chi^{2}/n.d.f = {round(chi2ndf,2)}'] 
    xmin = params[1] - 4.0*params[2]
    xmax = params[1] + 4.0*params[2]
    ymax = params[0]*1.1
    c = utils.drawTH1s([histo], histo.GetName(), drawOpts=['hist'],xrange=[xmin,xmax], yrange=[0.0,ymax],size=(2000,1500), text=text, text_pos=[0.2,0.78], line_spacing=0.05)
    c.Write()
    c.SaveAs(f'{outdir}/{histo.GetName()}.png')
    c.Close()
    #save fit results
    fit_results[mass] = [fit_histo, params, errors]

# Fit the fitted mass resolution results as a function of invariant mass
fit_masses = sorted(np.array([float(x.replace('mass_','')) for x in fit_results.keys()], dtype=float))
fit_res = np.array([fit_results[f'mass_{int(mass)}'][1][2] for mass in fit_masses], dtype=float)
fit_errs = np.array([fit_results[f'mass_{int(mass)}'][2][2] for mass in fit_masses], dtype=float)
zeros = np.array([0.0 for mass in fit_masses], dtype=float)

# Make TGraphErrors to fit with polynomial and get mass resolution as function of mass
massRes_smeared_ge = r.TGraphErrors(len(fit_masses), np.array(fit_masses), np.array(fit_res), np.array(zeros), np.array(fit_errs))
massRes_smeared_ge.GetXaxis().SetTitle('Invariant Mass [MeV]')
massRes_smeared_ge.GetYaxis().SetTitle('Invariant Mass Resolution [MeV]')
massRes_smeared_ge.GetYaxis().SetTitleOffset(1.)
fitResult_smeared = fit_with_poly(massRes_smeared_ge)
text_smeared = [f'Smeared: {fitResult_smeared.Parameters()}']
c_smeared = utils.drawTGraphs([massRes_smeared_ge], 'smeared_mc', drawOpts=['AP'], text=text_smeared, 
                      text_pos=[0.2,0.8])
c_smeared.SaveAs(f'{outdir}/invariant_mass_resolution_function_smeared.png')
c_smeared.Write()

#=======================================================================================================================================
# CALCULATE MC SIGNAL MASS RESOLUTION WITH SMEARING
#=======================================================================================================================================

# Run over unsmeared MC signal and fit
for mass, histo in inv_masses_unsm_h.items():  
    print(f'Fitting {mass}')
    fit_histo, params, errors, chi2ndf = gaus_fit(histo, histo.GetXaxis().GetBinLowEdge(histo.FindFirstBinAbove(0)), histo.GetXaxis().GetBinLowEdge(histo.FindLastBinAbove(0)), histo.GetMean(), histo.GetRMS(), histo.GetMaximum(), nsigma=2.0, isData=False)
     
    # Draw Fit result to canvas, and save to png and to root file
    text = [f'\mu = {round(params[1],2)}',f'\sigma = {round(params[2],2)}',f'\chi^{2}/n.d.f = {round(chi2ndf,2)}'] 
    xmin = params[1] - 4.0*params[2]
    xmax = params[1] + 4.0*params[2]
    ymax = params[0]*1.1
    c = utils.drawTH1s([histo], f'{histo.GetName()}_unsmeared', drawOpts=['hist'],xrange=[xmin,xmax], yrange=[0.0,ymax],size=(2000,1500), text=text, text_pos=[0.2,0.78], line_spacing=0.05)
    c.Write()
    c.SaveAs(f'{outdir}/{histo.GetName()}.png')
    c.Close()
    
    # Save fit results
    fit_results[mass] = [fit_histo, params, errors]

# Fit the fitted mass resolution results as a function of invariant mass
fit_masses = sorted(np.array([float(x.replace('mass_','')) for x in fit_results.keys()], dtype=float))
fit_res = np.array([fit_results[f'mass_{int(mass)}'][1][2] for mass in fit_masses], dtype=float)
fit_errs = np.array([fit_results[f'mass_{int(mass)}'][2][2] for mass in fit_masses], dtype=float)
zeros = np.array([0.0 for mass in fit_masses], dtype=float)

# Make TGraphErrors to fit with polynomial and get mass resolution as function of mass
massRes_unsmeared_ge = r.TGraphErrors(len(fit_masses), np.array(fit_masses), np.array(fit_res), np.array(zeros), np.array(fit_errs))
massRes_unsmeared_ge.GetXaxis().SetTitle('Invariant Mass [MeV]')
massRes_unsmeared_ge.GetYaxis().SetTitle('Invariant Mass Resolution [MeV]')
massRes_unsmeared_ge.GetYaxis().SetTitleOffset(1.0)
fitResult_unsmeared = fit_with_poly(massRes_unsmeared_ge)
text_unsmeared = [f'unsmeared: {fitResult_unsmeared.Parameters()}']
c_unsmeared = utils.drawTGraphs([massRes_unsmeared_ge], 'unsmeared_mc', drawOpts=['AP'], text=text_unsmeared, 
                      text_pos=[0.2,0.8])
print(fitResult_unsmeared.Parameters())
c_unsmeared.SaveAs(f'{outdir}/invariant_mass_resolution_function_unsmeared.png')
c_unsmeared.Write()


#=======================================================================================================================================
# SUMMARY PLOT
#=======================================================================================================================================

colors = utils.getColorsHPS()
#format smeared
utils.format_th1(massRes_smeared_ge, title='Smeared MC', linecolor=r.kBlack, markerstyle=20, markercolor=r.kBlack)
massRes_smeared_ge.SetMarkerSize(2)
massRes_smeared_ge.GetYaxis().SetTitleOffset(0.5)
massRes_smeared_ge.GetYaxis().SetRangeUser(0.0,8.0)
#format unsmeared
utils.format_th1(massRes_unsmeared_ge, title='Un-smeared MC', linecolor=r.kBlack, markerstyle=4, markercolor=r.kBlack)
massRes_unsmeared_ge.SetMarkerSize(2)
massRes_unsmeared_ge.GetListOfFunctions().At(0).SetLineColor(colors[1])


c = r.TCanvas('c','c',2500,1500)
c.cd()
massRes_smeared_ge.Draw('AP')
massRes_unsmeared_ge.Draw('PSAME')
c.Draw()
legend = utils.buildLegend([massRes_smeared_ge, massRes_unsmeared_ge], position=(0.3,0.7, 0.4, 0.8), titles=['Smeared MC', 'Un-smeared MC'])
legend.SetTextSize(0.03)
legend.Draw()
c.SaveAs(f'{outdir}/invariant_mass_resolution_fits.png')

