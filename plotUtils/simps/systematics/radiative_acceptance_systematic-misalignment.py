#!/usr/bin/python3
#==================================================================================================================================
# Description: Compares the radiative acceptance between nominal and misaligned HPS detector versions
# Author: Alic Spellman
# Date: 09/05/2024
# Script to load MC samples, plot and compute misalignment systematics for 2016 simp L1L1 analysis
# Calculates both radiative trident acceptance and signal acceptance
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

#simp tools defined in hpstr
hpstr_base = os.getenv('HPSTR_BASE')
sys.path.append(f'{hpstr_base}/plotUtils/simps')
import simp_signal_2016

#======================================================================================================================================
#INITIALIZATION
#=======================================================================================================================================
# Set plotting parameters for matplotlib
plt.rcParams.update({'font.size': 40, 'axes.titlesize': 40, 'axes.labelsize': 40, 'xtick.labelsize': 40, 'ytick.labelsize': 40, 'lines.linewidth': 3.0, 'legend.fontsize': 40})
plt.rcParams['font.family'] = 'DejaVu Sans'

#parse input arguments
import argparse
parser = argparse.ArgumentParser(description='')
parser.add_argument('--outdir', type=str, default='./search_results')
parser.add_argument('--mpifpi', type=float, default=4.*np.pi)

args = parser.parse_args()
outdir = args.outdir

#=======================================================================================================================================
# LOAD DATA: Initialize signal processor and load radiative trident MC samples
#=======================================================================================================================================
search_window = 1.5 
signalProcessor = simp_signal_2016.SignalProcessor(args.mpifpi, search_window)

##Load MC samples for nominal and misaligned detectors
samples = {}
mcsamples = {}
branches = ["unc_vtx_mass", "unc_vtx_psum"]

#Load reconstructed and selected radiative events for nominal detector
infile = '/sdf/group/hps/user-data/alspellm/2016/systematics/radacc/misalignments/hadd_2kfiles_rad_nobeam_nominal_recon_ana.root'
selection = 'vtxana_radMatchTight_nocuts' #USE RADMATCHTIGHT!
samples['nominal'] = signalProcessor.load_data(infile, selection, cut_expression='((unc_vtx_psum > 1.9) & (unc_vtx_psum < 2.4) )', expressions=branches)

#Load generated events (mc ana) for nominal detector
infile = '/sdf/group/hps/user-data/alspellm/2016/systematics/radacc/misalignments/hadd_2kfiles_rad_nobeam_nominal_mc_ana.root'
slicfile = r.TFile(infile, "READ")
mcsamples['nominal'] = copy.deepcopy(slicfile.Get('mcAna/mcAna_mc622Mass_h'))
slicfile.Close()

#Load reconstructed and selected radiative events for misaligned detector
infile = '/sdf/group/hps/user-data/alspellm/2016/systematics/radacc/misalignments/hadd_2kfiles_rad_nobeam_misalignments_1_recon_ana.root'
selection = 'vtxana_radMatchTight_nocuts' #USE RADMATCHTIGHT!
samples['misaligned_v1'] = signalProcessor.load_data(infile, selection, cut_expression='((unc_vtx_psum > 1.9) & (unc_vtx_psum < 2.4) )', expressions=branches)

#Load generated events (mc ana) for misaligned detector
infile = '/sdf/group/hps/user-data/alspellm/2016/systematics/radacc/misalignments/hadd_2kfiles_rad_nobeam_misalignments_1_mc_ana.root'
slicfile = r.TFile(infile, "READ")
mcsamples['misaligned_v1'] = copy.deepcopy(slicfile.Get('mcAna/mcAna_mc622Mass_h'))
slicfile.Close()

#output file to store systematic results
outfile = uproot.recreate(f'{outdir}/radacc_misalignment_systematic_results.root')

#=======================================================================================================================================
# CHECK RADIATIVE PEAK: Ensure that the radiative peaks for misaligned and nominal are commensurate.
# Peak will be off if overly-misaligned
#=======================================================================================================================================
psum_h = (
    hist.Hist.new
    .StrCategory(list(samples.keys()), name='samples')
    .Reg(50,1.9, 2.4,label='Psum [GeV]')
    .Double()
)
colors=['black', 'darkred']
fig, ax = plt.subplots(figsize=(25,15))
for i,(sname, sample) in enumerate(samples.items()):
    psum_h.fill(sname, sample.unc_vtx_psum)
psum_h.plot(color=['black','darkred'], linewidth=3.0)
plt.legend()
plt.ylabel('Events')
plt.savefig(f'{outdir}/radiative_peak_misaligned.png')

#=======================================================================================================================================
# RADIATIVE ACCEPTANCE HISTOGRAMS: Initialize invariant mass histograms, rebin, and convert them to ROOT histograms
#=======================================================================================================================================
#invariant mass histogram binning must match mc ana invariant mass to take ratio
nbinsx = mcsamples['nominal'].GetNbinsX()
first_bin = mcsamples['nominal'].GetBinLowEdge(1)
last_bin = nbinsx*mcsamples['nominal'].GetBinWidth(1)
invmass_h = (
    hist.Hist.new
    .StrCategory(list(samples.keys()), name='samples')
    .Reg(nbinsx,first_bin,last_bin,label='Invariant Mass [MeV]')
    .Double()
)

#Fill mc components without weights and convert to ROOT, rebin
invmass_histos = {}
for sname, sample in samples.items():
    invmass_h.fill(sname, sample.unc_vtx_mass*1000.)
    invmass_histos[sname] = signalProcessor.cnvHistoToROOT(invmass_h[sname,:])
    invmass_histos[sname].Rebin(2)
    mcsamples[sname].Rebin(2)

def nonUniBinning(histo, start, size):
    edges_a = np.arange(histo.GetBinLowEdge(1),start+histo.GetBinWidth(1),histo.GetBinWidth(1)) 
    edges_b = np.arange(start,histo.GetBinLowEdge(histo.GetNbinsX()), size) 
    bin_edges = np.concatenate([edges_a, edges_b[1:]])
    histo_rebinned = r.TH1F(f'{histo.GetName()}_rebinned', f'{histo.GetTitle()}', len(bin_edges)-1, bin_edges)
    for bin in range(1, histo.GetNbinsX() + 1):
        content = histo.GetBinContent(bin)
        center = histo.GetBinCenter(bin)
        error = histo.GetBinError(bin)
        new_bin = histo_rebinned.FindBin(center)
        histo_rebinned.SetBinContent(new_bin, histo_rebinned.GetBinContent(new_bin)+content)
        histo_rebinned.SetBinError(new_bin, np.sqrt(histo_rebinned.GetBinError(new_bin)**2 + error**2))
    return histo_rebinned

#enable non-uniform binning
for sname, sample in samples.items():
    invmass_histos[sname] = nonUniBinning(invmass_histos[sname], 150, 4)
    mcsamples[sname] = nonUniBinning(mcsamples[sname], 150, 4)
    outfile[f'recon_{sname}'] = invmass_histos[sname]
    outfile[f'mc_{sname}'] = mcsamples[sname]

#=======================================================================================================================================
# RADIATIVE ACCEPTANCE: Compute and plot the radiative acceptance for nominal and misaligned
#=======================================================================================================================================

fits = {}
colors = ['#d62728', '#bcbd22', '#2ca02c', '#17becf', '#1f77b4', '#9467bd', '#7f7f7f']
colors = ['black', 'darkred', 'darkblue', 'darkgreen', 'darkorange']

#Figure to plot radiative acceptance and systematic uncertainty
fig, ax = plt.subplots(2,1,figsize=(20,20))
plt.subplot(2,1,1)
plt.xlabel('Invariant Mass [MeV]')
plt.ylabel('Radiative Acceptance')
plt.ylim(0.0, .15)
plt.xlim(20.0,206.0)

#Calculate radiative acceptance as ratio of recon+sel/generated for each detector
#these are root histograms. 
for i,(sname, histo) in enumerate(invmass_histos.items()):

    #divide recon+sel by generated
    ratio = invmass_histos[sname].Clone()
    ratio.Divide(mcsamples[sname])

    #fit radiative acceptance
    fitparams, _ = signalProcessor.fit_plot_with_poly(ratio, specify_n=7, set_xrange=True, xrange=(30.0, 220.0)) 
    outfile[f'rad_acc_{sname}'] = ratio
    print(sname,fitparams)

    #convert root histograms to numpy data for convenient plotting using mpl
    (xvals, yvals, errors), (x_fit, y_fit) = cnv_root_to_np(ratio)
    fits[sname] = (x_fit, y_fit)

    #plot
    plt.errorbar(xvals, yvals, yerr=errors, linestyle='', marker='o', color=colors[i], label=sname)
    plt.plot(x_fit, y_fit, linewidth=3.0, color=colors[i])

plt.legend(fontsize=20)
#plot the real radiative acceptance (includes beam)
#radacc_off = polynomial(-0.48922505, 0.073733061, -0.0043873158, 0.00013455495, -2.3630535e-06, 2.5402516e-08, -1.7090900e-10, 7.0355585e-13, -1.6215982e-15, 1.6032317e-18)
#plt.plot(xvals, radacc_off(xvals), label='rad+beam', marker='o', color='blue')

#=======================================================================================================================================
# CALCULATE SYSTEMATIC UNCERTAINTY: Using the ratio of the nominal and misaligned radiative acceptance functions
# If the radiative acceptance increases with misalignment, that represents a decrease in expected signal (no systematic)
# If the radiative acceptance decreases with misalignment, that will boost expected signal and must be accounted for.
#=======================================================================================================================================

#this is a subfigure of the figure above
plt.subplot(2,1,2)

#calculate ratio of the two radiative acceptance fits
#if ratio < 1.0, apply systematic to expected signal
fit_ratio = fits['misaligned_v1'][1]/fits['nominal'][1]
xvalues = fits['nominal'][0]

#plot the ratio
plt.plot(xvalues, fit_ratio, color='black', marker = '+', mew=5)
plt.axhline(y=1.0, linestyle='--', color='black')
plt.axhline(y=0.8, linestyle='--', color='black')
plt.xlim(20.0,206.)
plt.ylim(0.6,1.1)
plt.xlabel('A\' Invariant Mass [MeV]')
plt.ylabel('Systematic Uncertainty')
plt.savefig(f'{outdir}/radiative_acceptance_misalignment.png')


#fit the systematic uncertainty results and save the fit
sys_gr = r.TGraph(len(xvalues), xvalues, fit_ratio)
params_sys, errors_sys = signalProcessor.fit_plot_with_poly(sys_gr, tgraph=True, specify_n = 9, set_xrange=True, xrange=(50.0, 220.0))
(xvals, yvals, errors), (x_fit, y_fit) = signalProcessor.cnv_tgraph_to_np(sys_gr)
fig, ax = plt.subplots(figsize=(20,10))
plt.plot(xvals, yvals, marker='+', mew=3, markersize=10, color='darkblue')
plt.plot(x_fit, y_fit, linewidth=3.0, color='red')
outfile['misalignment_systematic'] = sys_gr

#=======================================================================================================================================
# SIGNAL MISALIGNMENT: calculate using the radiative acceptance fits from above. 
# 1. Use nominal signal (NO BEAM) and the nominal radiative acceptance (NO BEAM) to calculate expected signal.
# 2. Use misaligned signal (NO BEAM) and the misaligned radiative acceptance (NO BEAM) to calculate expected signal.
# 3. Calculate ratio between nominal and misaligned expected signal rates.
# *I've already done steps 1 and 2 externally, and am loading the results below.
#=======================================================================================================================================
#Load expected signal using nominal detector and nominal radiative acceptance (NO BEAM)
infile = '/sdf/group/hps/user-data/alspellm/2016/systematics/simp_radacc_misalignment_v1/simp_systematic_nominal.root'
with uproot.open(infile) as f:
    nominal_h = f['expected_signal_ap_h'].to_hist()

#Load expected signal using misaligned detector and misaligned radiative acceptance (NO BEAM)
infile = '/sdf/group/hps/user-data/alspellm/2016/systematics/simp_radacc_misalignment_v1/simp_systematic_misaligned.root'
with uproot.open(infile) as f:
    misaligned_h = f['expected_signal_ap_h'].to_hist()
    exp_sig_ratio_h = f['expected_signal_ap_h'].to_hist().reset() #make copy to use as ratio plot
outfile['expected_signal_nominal'] = nominal_h
outfile['expected_signal_misaligned'] = misaligned_h

# Calculate expected signal ratio between nominal and misaligned
ratio = nominal_h.values()/misaligned_h.values()
xbins = exp_sig_ratio_h.axes[0].centers
ybins = exp_sig_ratio_h.axes[1].centers
xgrid, ygrid = np.meshgrid(xbins, ybins, indexing='ij')
exp_sig_ratio_h.fill(xgrid.flatten(), ygrid.flatten(), weight=ratio.flatten())
outfile['signal_systematic_ratio'] = exp_sig_ratio_h

#Plot ratio in 2d
fig, ax = plt.subplots(figsize=(25,15))
exp_sig_ratio_h.plot(cmin=np.min(ratio.flatten()[ratio.flatten()>0.0]), cmax=np.max(ratio.flatten()))
plt.savefig(f'{outdir}/simp_radacc_misaligned_v1.png')

#=======================================================================================================================================
# CALCULATE SYSTEMATIC UNCERTAINTY: Using the 2d expected signal plot of nominal/misaligned, decide how to get systematic.
#=======================================================================================================================================

#Foddr each A' invariant mass, take the largest value, since this is in the numerator and reduces the expected signal rate
# The systematic uncertainty on the signal acceptance resulting from detector misaligned is calculated as a function of A' mass.
# For each mass, take the maximum ratio across the relevant range of epsilon^2 to be the uncertainty. 
sigsys_y = []
sigsys_x = []
for xbin,mass in enumerate(exp_sig_ratio_h.axes[0].centers):
    sigsys = np.max(exp_sig_ratio_h.values()[xbin])
    if sigsys == 0.0:
        continue
    sigsys_x.append(mass)
    sigsys_y.append(sigsys)

sigsys_gr = r.TGraph(len(sigsys_x), np.array(sigsys_x), np.array(sigsys_y))
params_sigsys, errors_sigsys = fit_plot_with_poly(sigsys_gr, tgraph=True, specify_n = 5, set_xrange=True, xrange=(sigsys_x[0], sigsys_x[-1]))
print(params_sigsys)
(sigsys_x, sigsys_y, sigsys_errors), (sigsys_xfit, sigsys_yfit) = cnv_tgraph_to_np(sigsys_gr)
plt.plot(sigsys_x, sigsys_y)
plt.plot(sigsys_xfit, sigsys_yfit)
sigsys_final = np.max(sigsys_yfit)
print(f'Signal misalignment acceptance systematic: {sigsys_final}')


#Combine the signal and radiative acceptance systematics
radsys_fitpoly = polynomial(-10.307720, 0.97578691, -0.036585723, 0.00077903787, -1.0393704e-05, 9.0187487e-08, -5.0948313e-10, 1.8078746e-12, -3.6566050e-15, 3.2111742e-18)
masses = np.array([float(x) for x in range(60,230,1)])
#Divide signal systematic by radiative acceptance
misalignmentsys = sigsys_final/radsys_fitpoly(masses)
fig, ax = plt.subplots(figsize=(25,15))
plt.plot(masses, misalignmentsys, marker='+', markersize=10, mew=3, color='black')

