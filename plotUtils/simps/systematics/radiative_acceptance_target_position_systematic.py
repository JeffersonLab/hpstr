#!/usr/bin/python3
#=======================================================================================================================================
# Description: Calculates systematic uncertainty associated with target position uncertainty (0.5 mm)
# related to radiative trident acceptance.
# MC radiative tridents+beam at nominal, -0.5 mm, and +0.5 mm

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
import mplhep
import matplotlib.gridspec as gridspec
import sys
import math

# SIMP tools in hpstr
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
# LOAD MC RADIATIVE TRIDENTS FOR EACH TARGET POSITION
#=======================================================================================================================================

# Invariant mass search window size
search_window = 1.5
# Initialize signal processor that contains everything needed to calculate expected signal
signalProcessor = simp_signal_2016.SignalProcessor(args.mpifpi, search_window)

samples = {}
mcsamples = {}
branches = ["unc_vtx_mass"]

# Load nominal radiative tridents + beam MC
infile = '/sdf/group/hps/user-data/alspellm/2016/rad_mc/pass4b/rad_beam/rad-beam-hadd-10kfiles-ana-smeared-corr.root'
selection = 'vtxana_radMatchTight_nocuts'
samples['nominal_beam'] = signalProcessor.load_data(infile, selection, cut_expression='((unc_vtx_psum > 1.9) & (unc_vtx_psum < 2.4) )', expressions=branches)
#mc ana
infile = '/sdf/group/hps/user-data/alspellm/2016/rad_mc/pass4b/rad_nobeam/rad_nobeam_slic_hadd10ktuples_ana.root'
slicfile = r.TFile(infile, "READ")
mcsamples['nominal_beam'] = copy.deepcopy(slicfile.Get('mcAna/mcAna_mc622Mass_h'))
slicfile.Close()

# Load nominal-0.5 mm radiative tridents + beam MC
infile = '/sdf/group/hps/user-data/alspellm/2016/systematics/radacc/hadd_1937files_rad_beam_targetz_Mpt5_recon_ana.root'
selection = 'vtxana_radMatchTight_nocuts' #USE RADMATCHTIGHT!
samples['targetz_Mpt5_beam'] = signalProcessor.load_data(infile, selection, cut_expression='((unc_vtx_psum > 1.9) & (unc_vtx_psum < 2.4) )', expressions=branches)
# Load radiative trident mc ana (generated rate)
infile = '/sdf/group/hps/user-data/alspellm/2016/systematics/radacc/hadd_1937files_rad_beam_targetz_Mpt5_mc_ana.root'
slicfile = r.TFile(infile, "READ")
mcsamples['targetz_Mpt5_beam'] = copy.deepcopy(slicfile.Get('mcAna/mcAna_mc622Mass_h'))
slicfile.Close()

# Load nominal-0.5 mm radiative tridents + beam MC
infile = '/sdf/group/hps/user-data/alspellm/2016/systematics/radacc/hadd_1937files_rad_beam_targetz_Ppt5_recon_ana.root'
selection = 'vtxana_radMatchTight_nocuts' #USE RADMATCHTIGHT!
samples['targetz_Ppt5_beam'] = signalProcessor.load_data(infile, selection, cut_expression='((unc_vtx_psum > 1.9) & (unc_vtx_psum < 2.4) )', expressions=branches)
# Load radiative trident mc ana (generated rate)
infile = '/sdf/group/hps/user-data/alspellm/2016/systematics/radacc/hadd_1937files_rad_beam_targetz_Ppt5_mc_ana.root'
slicfile = r.TFile(infile, "READ")
mcsamples['targetz_Ppt5_beam'] = copy.deepcopy(slicfile.Get('mcAna/mcAna_mc622Mass_h'))
slicfile.Close()

#=======================================================================================================================================
# CALCULATE RADIATIVE TRIDENT ACCEPTANCE
#=======================================================================================================================================

# Initialize histogram used to calculate radiative trident acceptance
nbinsx = mcsamples['nominal_beam'].GetNbinsX()
first_bin = mcsamples['nominal_beam'].GetBinLowEdge(1)
last_bin = nbinsx*mcsamples['nominal_beam'].GetBinWidth(1)
invmass_h = (
    hist.Hist.new
    .StrCategory(list(samples.keys()), name='samples')
    .Reg(nbinsx,first_bin,last_bin,label='Invariant Mass [MeV]')
    .Double()
)

# Fill without weights, so that histos can be converted to ROOT and retain statistical uncertainty
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
#nonUniBinning(invmass_histos['nominal'], 150, 5)
#for sname, sample in samples.items():
#    invmass_histos[sname] = nonUniBinning(invmass_histos[sname], 150, 4)
#    mcsamples[sname] = nonUniBinning(mcsamples[sname], 150, 4)


# Calculate radiative trident acceptance for each case (nominal, -0.5 mm, +0.5 mm)
fits = {}
colors = ['#d62728', '#bcbd22', '#2ca02c', '#17becf', '#1f77b4', '#9467bd', '#7f7f7f']
colors = ['black', 'darkred', 'darkblue', 'darkgreen', 'darkorange']
fig, ax = plt.subplots(2,1,figsize=(35,25),gridspec_kw={'height_ratios': [3, 2]})
plt.subplot(2,1,1)
plt.xlabel('A\' Invariant Mass [MeV]')
plt.ylabel('Radiative Acceptance')
labels = ['Nominal (-4.3 mm)', '-4.8 mm', '-3.8 mm']
for i,(sname, histo) in enumerate(invmass_histos.items()):
    ratio = invmass_histos[sname].Clone()  
    ratio.Divide(mcsamples[sname]) # radiative trident acceptance

    # Fit the radiative trident acceptance ratio with a polynomial fit function
    fit_params,_ = signalProcessor.fit_plot_with_poly(ratio, specify_n=7, set_xrange=True, xrange=(30.0, 220.0))
    print(sname, fit_params)
    (xvals, yvals, errors), (x_fit, y_fit) = signalProcessor.cnv_root_to_np(ratio)
    plt.errorbar(xvals, yvals, yerr=errors, linestyle='', marker='o', color=colors[i], label=labels[i])
    plt.plot(x_fit, y_fit, linewidth=3.0, color=colors[i])
    fits[sname] = (x_fit, y_fit)
plt.ylim(0.0, .15)
plt.xlim(30.0,220.0)
plt.legend(fontsize=50)
plt.text(53,0.125,'MC Radiative Tridents\n + Beam', horizontalalignment='center', fontsize=40)

plt.subplot(2,1,2)
fit_ratio_Mpt5 = fits['targetz_Mpt5_beam'][1]/fits['nominal_beam'][1]
fit_ratio_Ppt5 = fits['targetz_Ppt5_beam'][1]/fits['nominal_beam'][1]
xvalues = fits['nominal_beam'][0]
plt.plot(xvalues, fit_ratio_Mpt5, color='darkred', marker='o', label='-4.8 mm : Nominal')
plt.plot(xvalues, fit_ratio_Ppt5, color='darkblue', marker='o', label='-3.8 mm : Nominal')
plt.axhline(y=1.0, linestyle='--', color='black')
plt.axhline(y=1.05, linestyle='--', color='black')
plt.xlim(30.0,220.)
plt.ylim(0.85,1.075)
plt.xlabel('A\' Invariant Mass [MeV]')
plt.ylabel('Ratio')
plt.legend()

plt.savefig(f'{outdir}/radiative_acceptance_target_deltaz.png')
