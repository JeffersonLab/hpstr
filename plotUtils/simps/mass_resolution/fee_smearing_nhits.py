#!/usr/bin/python3
"""
This script calculates the FEE momentum smearing required to match MC and data.
FEE's are selected in MC and data, and their vertex Psum FEE peak is fit with a Gaussian function to measure
the FEE momentum resolution.
A smearing factor is calculated as a function of the number of hits on track to match MC to data. 
"""
import os
import numpy as np
import math
import ROOT as r

#=======================================================================================================================================
# INITIALIZE
#=======================================================================================================================================
# --outdir: Specify output directory.

import argparse
parser = argparse.ArgumentParser(description='Process some inputs.')
parser.add_argument('--outdir', type=str, default='moller_mass_fits')
args = parser.parse_args()
outdir = args.outdir

#=======================================================================================================================================
# FUNCTIONS
#=======================================================================================================================================
def gaus_fit(histo, xmin, xmax, smean, swidth, snorm, nsigma=2.0, isData=False):
    
    #initial fit with seeds
    fitfunc = r.TF1("gaus","gaus")
    fitfunc.SetParameter(0, snorm)
    fitfunc.SetParameter(1, smean)
    fitfunc.SetParameter(2, swidth)
    fitRes = histo.Fit(fitfunc,"QLES","", xmin, xmax)
    params = fitRes.Parameters()
    chi2 = fitRes.Chi2()
    ndf = fitRes.Ndf()
    
    #set first fist to be best fit
    best_chi2 = chi2/ndf
    best_params = params
    
    #iterate over randomly fluctuated fit parameters. Keep the best resulting fit
    niters = 100
    for n in range(niters):
        norm = params[0]*np.random.uniform(80,120)*0.01
        mu = params[1]*np.random.uniform(80,120)*0.01
        sigma = params[2]*np.random.uniform(80,120)*0.01
        
        #Data has shoulders, so we can specify the xmin and xmax to do an asymmetric fit window
        if isData:
            xminx = mu - nsigma*sigma
            xmaxx = mu + nsigma*sigma
            if xminx < xmin:
                xminx = xmin
            if xmaxx > xmax:
                xmaxx = xmax
        fitfunc.SetParameter(0, norm)
        fitfunc.SetParameter(1, mu)
        fitfunc.SetParameter(2, sigma)
        fitRes = histo.Fit(fitfunc,"QLES","", xminx, xmaxx)
        #If fit fails, skip
        try:
            if fitRes.Parameters()[1] < xmin or fitRes.Parameters()[1] > xmax or fitRes.Ndf() < 1:
                continue
        except:
            continue
            
        params = fitRes.Parameters() #these results seed the next iteration...maybe should only do if improved?
        chi2 = fitRes.Chi2()
        ndf = fitRes.Ndf()
        #replace best fit
        if chi2/ndf < best_chi2:
            best_params = params
        
    #Do the final fit using the best parameters found
    fitfunc.SetParameter(0, best_params[0])
    fitfunc.SetParameter(1, best_params[1])
    fitfunc.SetParameter(2, best_params[2])
    xminx = best_params[1] - nsigma*best_params[2]
    xmaxx = best_params[1] + nsigma*best_params[2]

    #again, if data, use asymmetric fit window to avoid the left shoulder
    if isData:
        if xminx < xmin:
            xminx = xmin
        if xmaxx > xmax:
            xmaxx = xmax
        
    fitRes = histo.Fit(fitfunc,"QLES","", xminx, xmaxx)
    params = fitRes.Parameters()
    errors = fitRes.Errors()
    chi2 = fitRes.Chi2()
    ndf = fitRes.Ndf()
    return histo, params, errors, chi2/ndf

#=======================================================================================================================================
# LOAD DATA AND FIT FEE PEAK
#=======================================================================================================================================

# Load FEEs tracks in data
data_results = {}
infilename = '/sdf/group/hps/user-data/alspellm/2016/fee_smearing/run7800/hadd/hadd_fee_2pt3_recon_fee_histos.root' #FEE skimmed tracks from hpstr track analysis processor

# Read track hit histograms
histoname = 'KalmanFullTracks/KalmanFullTracks_p_vs_nHits_top_hh' #top
infile = r.TFile(f'{infilename}',"READ")
top_h = copy.deepcopy(infile.Get(f'{histoname}'))
histoname = 'KalmanFullTracks/KalmanFullTracks_p_vs_nHits_bot_hh' #bot
bot_h = copy.deepcopy(infile.Get(f'{histoname}')) 
infile.Close()

# Change the names to use as keys
top_h.SetName('top')
bot_h.SetName('bot')

# Fit the FEE peak for each category of nhits. Just have access to 10, 11, 12 for now
for h in [top_h, bot_h]:
    histo = h
    for nhits in [10, 11, 12]:
        # Get the nhits momentum projection
        proj = histo.ProjectionY(f'proj_{h.GetName()}_{nhits}hits', histo.GetXaxis().FindBin(nhits), histo.GetXaxis().FindBin(nhits),"")
        # Fit the data
        _, params, errors, chi2ndf = gaus_fit(proj, 2.0, 2.5, 2.4, 0.47, 12000, nsigma=1.5, isData=True)

        # Store the results [mu,sigma] for top/bot nhits=<n>
        data_results[f'{h.GetName()}_nhits_{nhits}'] = [params[1], params[2]]

#=======================================================================================================================================
# LOAD MC AND FIT FEE PEAK
#=======================================================================================================================================

# Load MC FEE's from hpstr track analysis processor
mc_results = {}
infilename= '/sdf/group/hps/user-data/alspellm/2016/fee_smearing/tritrig/hadd/hadd_fee_2pt3_recon_tritrig_histos.root'

# Read track hit histograms
histoname = 'KalmanFullTracks/KalmanFullTracks_p_vs_nHits_top_hh' #top
infile = r.TFile(f'{infilename}',"READ")
top_h = copy.deepcopy(infile.Get(f'{histoname}'))
histoname = 'KalmanFullTracks/KalmanFullTracks_p_vs_nHits_bot_hh' #bot
bot_h = copy.deepcopy(infile.Get(f'{histoname}')) 
infile.Close()

#Change the names to use as keys
top_h.SetName('top')
bot_h.SetName('bot')

for h in [top_h, bot_h]:
    histo = h
    for nhits in [10, 11, 12]:
        # Get the nhits momentum projection
        proj = histo.ProjectionY(f'proj_{h.GetName()}_{nhits}hits', histo.GetXaxis().FindBin(nhits), histo.GetXaxis().FindBin(nhits),"")
        # Fit the data
        _, params, errors, chi2ndf = gaus_fit(proj, 2.1, 2.5, 2.2, 0.1, proj.GetMaximum(), nsigma=1.5)
        # Store the results [mu, sigma] for top/bot nhits=<n>
        mc_results[f'{h.GetName()}_nhits_{nhits}'] = [params[1], params[2]]

#=======================================================================================================================================
# CALCULATE MOMENTUM SMEARING FACTORS
#=======================================================================================================================================
# Store momentum smearing factors in ROOT file
outfile = r.TFile(f'{outdir}/smearingFile_2016_nhits.root',"RECREATE")
outfile.cd()
# Calculate smearing for Top and Bot
smtop_h = r.TH1F('KalmanFullTracks_p_vs_nHits_hh_smearing_rel_top','p_vs_nHits_smearing_rel_top;nhits;smear factor', 3, 9.5, 12.5)
smbot_h = r.TH1F('KalmanFullTracks_p_vs_nHits_hh_smearing_rel_bot','p_vs_nHits_smearing_rel_bot;nhits;smear factor', 3, 9.5, 12.5)

# Calculate smearing factor according to 2016 Bump Hunt
smear_fac = lambda mu_data, sig_data, mu_mc, sig_mc : np.sqrt(np.square(sig_data/mu_data) - np.square(sig_mc/mu_mc))
for key, vals in data_results.items():
    istop = False
    if 'top' in key:
        istop = True
    nhits = float(key.split('_')[2])
    mu_data = vals[0]
    sig_data = vals[1]
    mu_mc = mc_results[key][0]
    sig_mc = mc_results[key][1]
    sf = smear_fac(mu_data, sig_data, mu_mc, sig_mc)
    print(f'{key} sf={sf}')
    
    #save results
    if istop:
        smtop_h.SetBinContent(smtop_h.GetXaxis().FindBin(nhits), sf)
    else:
        smbot_h.SetBinContent(smbot_h.GetXaxis().FindBin(nhits), sf)
                            
outfile.Write()
