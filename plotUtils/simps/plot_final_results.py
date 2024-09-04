#!/usr/bin/env python
import os
import awkward as ak
import numpy as np
import hist
from hist import Hist
import uproot
import ROOT as r
import copy
import matplotlib.pyplot as plt
import matplotlib as mpl
import matplotlib.gridspec as gridspec
import sys
import math

#Import simp class where systematics info is stored
import simp_signal_2016

#format mpl plots. Should make a template...
plt.rcParams.update({'font.size': 60,           # Font size for text
                     'axes.titlesize': 60,      # Font size for titles
                     'axes.labelsize': 60,      # Font size for axis labels
                     'xtick.labelsize': 60,     # Font size for x-axis tick labels
                     'ytick.labelsize': 60,     # Font size for y-axis tick labels
                     'lines.linewidth':5.0,
                     'legend.fontsize': 60})    # Font size for legend
plt.rcParams['font.family'] = 'DejaVu Sans' 

import argparse
parser = argparse.ArgumentParser(description='')
parser.add_argument('--infile_signal_search', type=str, default='.simp_2016_results/unblinded_result_100pct.root')
parser.add_argument('--infile_signal_opt_interval', type=str, default='.simp_2016_results/unblinded_result_100pct.root')
parser.add_argument('--outdir', type=str, default='./search_results')
parser.add_argument('--mpifpi', type=float, default=4.*np.pi)

args = parser.parse_args()
########################################################################################################################################

#Input file should be output of run_signal_search.py
infile_signal_search = args.infile_signal_search
#Grab search results (exp bkg, obs, local pvalues)
with uproot.open(infile) as f:
    expected_bkg = f['expected_background'].values()
    expected_bkg_errlow = f['expected_background'].errors('low')[1]
    expected_bkg_errhigh = f['expected_background'].errors('high')[1]
    Nobs = f['Nobs'].values()
    local_pvalue = f['local_pvalue'].values()
    local_pvalue_errlow = f['local_pvalue'].errors('low')[1]
    local_pvalue_errhigh = f['local_pvalue'].errors('high')[1]
    
#Calculate the "Look-Elsewhere Effect" correction. Total search window mass range divided by avg mass resolution
search_window = 1.5 #used in final analysis
signalProcessor = simp_signal_2016.SignalProcessor(args.mpifpi., search_window)
masses = expected_bkg[0]
masses = masses[np.where(masses <= 124.0)[0]] #I restricted search for signal to 124 MeV
avg_resolution = np.average( np.array([signalProcessor.mass_resolution(x) for x in masses]) )
look_elsewhere = (masses[-1] - masses[0])/(avg_resolution)
print(f'Average mass resolution: {avg_resolution}')
print(f'Look elsewhere effect: {look_elsewhere}')
thresholds = []
thresholds_lew = [] #thresholds corrected for look elsewhere
from scipy.stats import norm
for nsigma in [1,2,3,4,5]:
    gaus_cdf = norm.cdf(nsigma)
    threshold = (1.0 - gaus_cdf)/look_elsewhere
    thresholds_lew.append(threshold)
    thresholds.append((1.0 - gaus_cdf))
    
#top plot: expected background and observed events for all search windows
fig, ax  = plt.subplots(2, 1, figsize=(60,60), gridspec_kw={'height_ratios': [3, 2]})
gs = gridspec.GridSpec(2, 1, height_ratios=[10, 1])
plt.subplot(2,1,1)
plt.errorbar(expected_bkg[0], expected_bkg[1], yerr=(expected_bkg_errlow, expected_bkg_errhigh), marker='o', markersize=20, color='blue', label='Estimated Background')
plt.plot(Nobs[0], Nobs[1], marker='o', color='black', markersize=20, label='Observed Events')
plt.legend()
plt.xlabel('$V_{D}$ Invariant Mass Search Window [MeV]')
plt.ylabel('Number Of Events')
plt.xlim(29.5,124.0)

#bottom plot:  local p-values and significance thresholds
plt.subplot(2,1,2)
plt.errorbar(local_pvalue[0], local_pvalue[1], yerr=(local_pvalue_errlow, local_pvalue_errhigh), marker='o', markersize=20, color='black', label='Local p-value')
from scipy.stats import norm
for n,nsigma in enumerate(thresholds):
    if n < len(thresholds)-1:
        plt.axhline(y=nsigma, linestyle='--', linewidth=4.0, color='black')
        plt.axhline(y=thresholds_lew[n], linestyle='--', linewidth=4.0, color='red')
    else:
        plt.axhline(y=nsigma, linestyle='--', linewidth=4.0, color='black', label='Local $N\sigma$')
        plt.axhline(y=thresholds_lew[n], linestyle='--', linewidth=4.0, color='red', label='Global LEE $N\sigma$')
plt.xlabel('$V_{D}$ Invariant Mass Search Window [MeV]')
plt.ylabel('p-value')
plt.legend()

#Identify search window with larges p-value fluctuation (smallest p-value)
pvalmin = np.min(local_pvalue[1])
pvalmin_mass = local_pvalue[0][np.argmin(local_pvalue[1])]
nsigma_local = np.round(norm.ppf(1 - pvalmin / 2),1)
nsigma_global = np.round(norm.ppf(1 - (pvalmin*look_elsewhere) / 2),1)
t = plt.text(80.0, .9e-4, f'Smallest p-value: {pvalmin} at {pvalmin_mass} MeV\nLocal Significance: {nsigma_local}$\sigma$\nGlobal Significance: {nsigma_global}$\sigma$')
t.set_bbox(dict(facecolor='white', alpha=1.0, edgecolor='black'))
plt.yscale('log')
plt.ylim(1e-6,1.1)
plt.xlim(29.5,124.0)
plt.savefig(f'{outdir}/signal_search_results.png')

##########################################################################################################################
#Calculate exclusion contour with systematic uncertainties included
infile_signal_opt_interval = args.infile_signal_opt_interval
#no systematics included in these results. They are applied in this script by reducing the expected signal rate.
#Published result may prefer to generate more MC signal masses, re-calculate OIM, use ratio (OIM w sys)/(OIM w/o sys)...
with uproot.open(infile_signal_opt_interval) as f:
    exclusion_h = f['sensitivity_ap_h'].to_hist() 
    expected_signal_h = f['total_yield_ap_h'].to_hist() 
    excluded_signal_h = f['excluded_signal_ap_h'].to_hist()

sysProc = simp_signal_2016.SignalProcessor(args.mpifpi, search_window)
sysProc.systematic_uncertainties()
masses = expected_signal_h.axes[0].centers
sys_uncertainties = []
for mass in masses:
    #systematics are accessed through SignalProcessor
    rad_targ_nom, rad_targ_mpt5, rad_targ_ppt5, simp_targ, mass_unc, radfrac_unc = sysProc.evaluate_polynomials(mass) 
    #for rad targ, take ratio of off-nominal to nominal. Max value is used. 
    rad_targ = np.maximum(rad_targ_mpt5/rad_targ_nom, rad_targ_ppt5/rad_targ_nom) - 1.0
    if rad_targ < 0.0:
        rad_targ = 0.0
    #for simp targ, if ratio is greater than 1, set to 1
    simp_targ = 1.0-simp_targ
    if simp_targ < 0.0:
        simp_targ = 0.0
    sys_unc = np.sqrt( rad_targ**2 + simp_targ**2 + radfrac_unc**2 + mass_unc**2 )
    sys_uncertainties.append(sys_unc)
    
#Plot systematic uncertaint as function of mass
fig, ax = plt.subplots(figsize=(40,20))
plt.plot(masses, sys_uncertainties, marker='o',markersize=10, color='black')
plt.xlabel('A\' Invariant Mass [MeV]')
plt.ylabel('Systematic Uncertainty')
plt.ylim(0.0, 0.2)
plt.axhline(y=0.05, linestyle='--', color='darkred')
plt.axhline(y=0.1, linestyle='--', color='darkred')
plt.text(160, 0.15, 'HPS Preliminary\n(incomplete)', horizontalalignment='center')
plt.savefig(f'{outdir}/systematic_uncertainty_summary.png')

#Rescale the expected signal according to the systematic uncertainty
values = expected_signal_h.values()
rescaled_values = np.zeros_like(values)
for m, mass in enumerate(masses):
    rescaled_values[m, :] = values[m, :] * (1.0-sys_uncertainties[m])
rescaled_signal_h = expected_signal_h.copy()
rescaled_signal_h[...] = rescaled_values

#Divide rescaled signal by upper limit to get exclusion contour
new_exclusion = rescaled_values/excluded_signal_h.values()
rescaled_exclusion_h = exclusion_h.copy()
rescaled_exclusion_h[...] = new_exclusion

#Apply systematics to expected signal...
fig, ax = plt.subplots(figsize=(40,30))
rescaled_signal_h.plot()
plt.xlim(50,180)
plt.ylim(-6.5, -4.0)
plt.xlabel('A\' Invariant Mass [MeV]', fontsize=80)
plt.ylabel('$\log{\epsilon^2}$', fontsize=80)
plt.text(150, -4.4,f'HPS PRELIMINARY\n(Partial Systematics)', color='white', weight='bold', fontsize=80, horizontalalignment='center')
plt.text(156, -3.97,'Expected Signal', fontsize=80)
plt.savefig(f'{outdir}/expected_signal_2d.png')

#save upper limit
fig, ax = plt.subplots(figsize=(40,30))
excluded_signal_h.plot()
plt.xlim(50,180)
plt.ylim(-6.5, -4.0)
plt.xlabel('A\' Invariant Mass [MeV]', fontsize=80)
plt.ylabel('$\log{\epsilon^2}$', fontsize=80)
plt.text(165, -3.97,'Upper Limit', fontsize=80)
plt.text(150, -4.4,f'HPS PRELIMINARY', color='white', weight='bold', fontsize=80, horizontalalignment='center')
plt.savefig(f'{outdir}/upper_limit_2d.png')

#Plot exclusion contour with systematics
fig, ax = plt.subplots(figsize=(40,30))
rescaled_exclusion_h.plot(cmin=0.0)
plt.xlim(50,180)
plt.ylim(-6.5, -4.0)
plt.xlabel('A\' Invariant Mass [MeV]', fontsize=80)
plt.ylabel('$\log{\epsilon^2}$', fontsize=80)
plt.text(150, -4.4,f'HPS PRELIMINARY\n(Partial Systematics)', color='white', weight='bold', fontsize=80, horizontalalignment='center')
recenters_x = rescaled_exclusion_h.axes[0].centers
recenters_y = rescaled_exclusion_h.axes[1].centers
revalues = rescaled_exclusion_h.values()
reX, reY = np.meshgrid(recenters_x, recenters_y)
contour_levels = [1.0]
colors = ['red']
recontour = ax.contour(reX, reY, revalues.T, levels=contour_levels, colors=colors)  # Transpose values for correct orientation
ax.clabel(recontour, inline=1, fontsize=60)
plt.savefig(f'{outdir}/exclusion_contour_2d.png')

#Write exclusion contour to .dat files
import csv
for l,level in enumerate(contour_levels):
    for line in range(len(recontour.allsegs[l])):
        print(line)
        with open(f'{outdir}/exclusion_mpifpi_{args.mpifpi}_contour_line{line+1}.dat', 'w', newline='') as csvfile:
            contour_line = recontour.allsegs[l][line]
            data = [[x[0], np.sqrt(10**x[1])] for x in contour_line] 
            data.append([contour_line[0][0], np.sqrt(10**contour_line[0][1])])
            writer = csv.writer(csvfile, delimiter=' ')
            writer.writerows(data)
