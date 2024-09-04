#!/usr/bin/python3
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

hpstr_base = os.getenv('HPSTR_BASE')
sys.path.append(f'{hpstr_base}/plotUtils/simps')
import simp_signal_2016

#format mpl plots
plt.rcParams.update({'font.size': 40,           # Font size for text
                     'axes.titlesize': 40,      # Font size for titles
                     'axes.labelsize': 40,      # Font size for axis labels
                     'xtick.labelsize': 40,     # Font size for x-axis tick labels
                     'ytick.labelsize': 40,     # Font size for y-axis tick labels
                     'lines.linewidth':3.0,
                     'legend.fontsize': 40})    # Font size for legend
plt.rcParams['font.family'] = 'DejaVu Sans' 

import argparse
parser = argparse.ArgumentParser(description='')
parser.add_argument('--outdir', type=str, default='./search_results')
parser.add_argument('--mpifpi', type=float, default=4.*np.pi)

args = parser.parse_args()
outdir = args.outdir
################################################################################################################################
def get_rand(x=False):
    if x:
        return np.random.uniform(80,120)*0.01
    else:
        return 1.0

def fit_root_gaussian(histo, fix_xmin=None, fix_xmax=None, max_attempts=50, nsigma=2.0):
    if fix_xmin:
        xmin = fix_xmin
    else:
        xmin = histo.GetXaxis().GetXmin()
    if fix_xmax:
        xmax = fix_xmax
    else:
        xmax = histo.GetXaxis().GetXmax()

    # Set initial guesses for Gaussian parameters
    gaussian = r.TF1("gaussian", "gaus", xmin, xmax)
    mean = histo.GetMean()
    sigma = histo.GetRMS()
    norm = histo.GetMaximum()
    gaussian.SetParameters(norm, mean, sigma)

    #initial fit
    fit_result = histo.Fit(gaussian, "ES", "", xmin, xmax)
    params = fit_result.Parameters()
    chi2 = fit_result.Chi2()
    ndf = fit_result.Ndf()
    best_chi2 = chi2/ndf
    best_params = params

    rand = False
    for attempt in range(max_attempts):
        gaussian.SetParameters(best_params[0]*get_rand(rand), best_params[1]*get_rand(rand), best_params[2]*get_rand(rand))
        if not fix_xmin:
            xmin = best_params[1] - nsigma*(best_params[2])
        if not fix_xmax:
            xmax = best_params[1] + nsigma*(best_params[2])
            
        fit_result = histo.Fit(gaussian, "QES", "", xmin, xmax)
       
        # Check if fit was successful
        if not fit_result.IsValid():
            rand = True
            continue
       
        params = fit_result.Parameters()
        chi2 = fit_result.Chi2()
        ndf = fit_result.Ndf()
        if chi2/ndf < best_chi2:
            best_chi2 = chi2/ndf
            best_params = params
            rand = False
        else:
            rand = True

    gaussian.SetParameters(best_params[0], best_params[1], best_params[2])
    if not fix_xmin:
        xmin = best_params[1] - nsigma*(best_params[2])
    if not fix_xmax:
        xmax = best_params[1] + nsigma*(best_params[2])
    fit_result = histo.Fit(gaussian, "QES", "", xmin, xmax)
    #params = np.round(fit_result.Parameters(),4)
    #errors = np.round(fit_result.Errors(),4)
    params = fit_result.Parameters()
    errors = fit_result.Errors()
    chi2 = fit_result.Chi2()
    ndf = fit_result.Ndf()

    params = np.array([float(f"{x:.2e}") for x in params])
    errors = np.array([float(f"{x:.2e}") for x in errors])

    return histo, params, errors, chi2/ndf

def cnv_root_to_np(histo):
    nbins = histo.GetNbinsX()
    xvals = np.array([histo.GetBinCenter(x+1) for x in range(nbins+1)])
    yvals = np.array([histo.GetBinContent(x+1) for x in range(nbins+1)])
    errors = np.array([histo.GetBinError(x+1) for x in range(nbins+1)])
    underflow = histo.GetBinContent(0)
    overflow = histo.GetBinContent(nbins+1)

    #add over/underflow
    xvals = np.insert(xvals, 0, xvals[0]-histo.GetBinWidth(1))
    yvals = np.insert(yvals, 0, underflow) 
    xvals = np.append(xvals, xvals[-1]+histo.GetBinWidth(1))
    yvals = np.append(yvals, overflow) 
    errors = np.insert(errors, 0, 0.0)
    errors = np.append(errors, 0.0) 

    #get fit function if it exist
    x_fit = None
    y_fit = None
    if len(histo.GetListOfFunctions()) > 0:
        fitfunc = histo.GetListOfFunctions()[0]
        x_fit = np.linspace(fitfunc.GetXmin(), fitfunc.GetXmax(), int((fitfunc.GetXmax()-fitfunc.GetXmin())/histo.GetBinWidth(1)))
        y_fit = np.array([fitfunc.Eval(x) for x in x_fit])

    return (xvals, yvals, errors), (x_fit, y_fit)

###################################################################################################################################

#Load signal processor
search_window = 1.5 #used in final search
signalProcessor = simp_signal_2016.SignalProcessor(args.mpifpi, search_window)

#Read in data, MC, and signal
samples = {}
branches = ["unc_vtx_ele_track_z0","unc_vtx_pos_track_z0"]

#Read 10% Data 
infile = '/sdf/group/hps/user-data/alspellm/2016/data/hadd_BLPass4c_1959files.root'
selection = 'vtxana_Tight_L1L1_nvtx1'
samples['data'] = signalProcessor.load_data(infile,selection, expressions=branches, cut_expression='((unc_vtx_psum > 1.0) & (unc_vtx_psum < 1.9) )')
samples['data']['weight'] = 1.0 #Assign weight of 10 to scale up to full lumi

#Load MC background
lumi = 10.7*.1 #pb-1
mc_scale = {'data' : 1.0,
            'tritrig' : 1.416e9*lumi/(50000*10000),
            'wab' : 0.1985e12*lumi/(100000*10000)}

#tritrig
infile = '/sdf/group/hps/user-data/alspellm/2016/tritrig_mc/pass4b/hadded_tritrig-beam-10kfiles-ana-smeared-corr_beamspotfix.root'
samples['tritrig'] = signalProcessor.load_data(infile, selection, cut_expression='((unc_vtx_psum > 1.0) & (unc_vtx_psum < 1.9) )', expressions=branches)
samples['tritrig']['weight'] = mc_scale['tritrig'] 

#wab
infile = '/sdf/group/hps/user-data/alspellm/2016/wab_mc/pass4b/hadded_wab-beam-10kfiles-ana-smeared-corr_beamspotfix.root'
samples['wab'] = signalProcessor.load_data(infile, selection, cut_expression='((unc_vtx_psum > 1.0) & (unc_vtx_psum < 1.9) )', expressions=branches)
samples['wab']['weight'] = mc_scale['wab'] 

#After smearing factor has been calculated, set to true to compare z0 distributions with smeared MC background
smear = True
if smear:
    mc_sigma = 0.1251
    data_sigma = 0.1348
    smearF = np.sqrt(data_sigma**2 - mc_sigma**2)
    
    #smear tritrig
    rel_smear = np.random.normal(0.0, 1.0, len(samples['tritrig'].unc_vtx_min_z0))
    smearfactors = rel_smear*smearF
    samples['tritrig']['unc_vtx_ele_track_z0'] = smearfactors + samples['tritrig']['unc_vtx_ele_track_z0'] 
    rel_smear = np.random.normal(0.0, 1.0, len(samples['tritrig'].unc_vtx_min_z0))
    smearfactors = rel_smear*smearF
    samples['tritrig']['unc_vtx_pos_track_z0'] = smearfactors + samples['tritrig']['unc_vtx_pos_track_z0'] 
    
    #smear wab
    rel_smear = np.random.normal(0.0, 1.0, len(samples['wab'].unc_vtx_min_z0))
    smearfactors = rel_smear*smearF
    samples['wab']['unc_vtx_ele_track_z0'] = smearfactors + samples['wab']['unc_vtx_ele_track_z0'] 
    rel_smear = np.random.normal(0.0, 1.0, len(samples['wab'].unc_vtx_min_z0))
    smearfactors = rel_smear*smearF
    samples['wab']['unc_vtx_pos_track_z0'] = smearfactors + samples['wab']['unc_vtx_pos_track_z0'] 


#Plot z0 for data and MC backgrounds
z0_h = (
    hist.Hist.new
    .StrCategory(list(samples.keys()), name='samples')
    .Reg(200, -1.,1.,label=r'Vertical Track Impact Parameter [mm]')
    .Double()
)

#Fill without weights, so that histos can be converted to ROOT and retain statistical uncertainty
z0_histos = {}
for sname, sample in samples.items():
    z0_h.fill(sname, sample.unc_vtx_ele_track_z0)#, weight=sample.weight/ak.sum(sample.weight))
    z0_h.fill(sname, sample.unc_vtx_pos_track_z0)#, weight=sample.weight/ak.sum(sample.weight))
    z0_histos[sname] = signalProcessor.cnvHistoToROOT(z0_h[sname,:])
    z0_histos[sname].Scale(mc_scale[sname])

#Scale Tritrig and WAB and combine with proper errors
z0_histos['tritrig_wab'] = z0_histos['tritrig'].Clone()
z0_histos['tritrig_wab'].Add(z0_histos['wab'])
#Normalize
for sname, sample in z0_histos.items():
    print(z0_histos[sname].Integral(0,-1))
    z0_histos[sname].Scale(1./z0_histos[sname].Integral(0,-1))

#Make plots of data vs MC background
fig, ax = plt.subplots(2,1, figsize=(25,30))
#Data
plt.subplot(2,1,1)
plt.xlabel('Vertical Track Impact Parameter [mm]')
plt.ylabel('Normalized Events')
_, params, fiterrors, _ = fit_root_gaussian(z0_histos['data'], nsigma=1.5)
(xvals, yvals, errors), (x_fit, y_fit) = cnv_root_to_np(z0_histos['data'])
plt.errorbar(xvals, yvals, yerr=errors, marker='o', color='black', linestyle='', label='10% Data')
plt.plot(x_fit, y_fit, color='red', label='Fit 10% Data', linewidth=2.0)
plt.text(0.25, 0.015, f'Norm={params[0]}$\pm${fiterrors[0]} \n$\mu$={params[1]}$\pm${fiterrors[1]} \n$\sigma$={params[2]}$\pm${fiterrors[2]}')
plt.legend()
#MC
plt.subplot(2,1,2)
plt.xlabel('Vertical Track Impact Parameter [mm]')
plt.ylabel('Normalized Events')
_, params, fiterrors, _ = fit_root_gaussian(z0_histos['tritrig_wab'], nsigma=1.5)
(xvals, yvals, errors), (x_fit, y_fit) = cnv_root_to_np(z0_histos['tritrig_wab'])
plt.errorbar(xvals, yvals, yerr=errors, marker='o', color='darkblue', linestyle='', label='MC Background')
plt.plot(x_fit, y_fit, color='red', label='Fit MC Background', linewidth=2.0)
plt.text(0.25, 0.015, f'Norm={params[0]}$\pm${fiterrors[0]} \n$\mu$={params[1]}$\pm${fiterrors[1]} \n$\sigma$={params[2]}$\pm${fiterrors[2]}')
plt.legend()
plt.ylim(0.0, 0.03)
plt.savefig(f'{outdir}/impact_parameter_data_v_mc_smeared_{smear}.png')

#######################################################################################################################################

#Smear the signal using ratio of data and MC widths
sysvals = []
masses = []
indir = '/sdf/group/hps/user-data/alspellm/2016/simp_mc/pass4b/beam/smeared_fixbeamspot'
for mass in range(30,120,4):
    masses.append(mass)
    signal_pre_readout_path = lambda mass: f'/sdf/group/hps/user-data/alspellm/2016/simp_mc/pass4b/nobeam/mass_{mass}_simp_2pt3_slic_hadd_ana.root'
    signal_path = lambda mass: f'{indir}/mass_{mass}_hadd-simp-beam_ana_smeared_corr.root'
    signal_selection = 'vtxana_radMatchTight_2016_simp_SR_analysis'
    signal = signalProcessor.load_signal(signal_path(signal_mass), signal_pre_readout_path(signal_mass), signal_mass, signal_selection)
    signal['weight']=1.0
    psum_sel = signalProcessor.psum_sel(signal, case='sr')
    
    #smearing factors calculated from comparing data and MC bkg z0 widths
    mc_sigma = 0.1251
    data_sigma = 0.1348
    smearF = np.sqrt(data_sigma**2 - mc_sigma**2)
    
    #smear signal minz0 
    rel_smear = np.random.normal(0.0, 1.0, len(signal.unc_vtx_min_z0))
    smearfactors = rel_smear*smearF
    signal['unc_vtx_ele_track_z0_smeared'] = smearfactors + signal['unc_vtx_ele_track_z0']
    
    rel_smear = np.random.normal(0.0, 1.0, len(signal.unc_vtx_min_z0))
    smearfactors = rel_smear*smearF
    signal['unc_vtx_pos_track_z0_smeared'] = smearfactors + signal['unc_vtx_pos_track_z0']
    
    #calculate smeared minz0
    signal['unc_vtx_min_z0_smeared'] = np.minimum(abs(signal['unc_vtx_ele_track_z0_smeared']), abs(signal['unc_vtx_pos_track_z0_smeared']))
    
    #Calculate change in efficiency
    unsmeared_sel = signal.unc_vtx_min_z0 > signalProcessor.minz0_cut_poly(signal.unc_vtx_mass*1000.)
    smeared_sel = signal.unc_vtx_min_z0_smeared > signalProcessor.minz0_cut_poly(signal.unc_vtx_mass*1000.)
    unsmeared_eff = ak.sum(signal[unsmeared_sel].weight)/ak.sum(signal.weight)
    smeared_eff = ak.sum(signal[smeared_sel].weight)/ak.sum(signal.weight)
    systematic = smeared_eff/unsmeared_eff
    sysvals.append(systematic)

    #final systematic stored in sysvals
    print(sysvals)

