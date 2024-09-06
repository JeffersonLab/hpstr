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
#######################################################################################################################################

#Load signal processor
search_window = 1.5 #used in final search
signalProcessor = simp_signal_2016.SignalProcessor(args.mpifpi, search_window)

#Read in data and MC bkg
samples = {}
branches = ["unc_vtx_proj_sig","unc_vtx_ele_track_z0","unc_vtx_pos_track_z0"]

#Read 10% Data 
infile = '/sdf/group/hps/user-data/alspellm/2016/data/hadd_BLPass4c_1959files.root'
selection = 'vtxana_Tight_L1L1_nvtx1'
samples['data'] = signalProcessor.load_data(infile,selection, expressions=branches, cut_expression='((unc_vtx_psum > 1.0) & (unc_vtx_psum < 1.9) )')
samples['data']['weight'] = 1.0 #Assign weight of 10 to scale up to full lumi

#Load MC background
lumi = 10.7*.1 #pb-1
mc_scale = {'tritrig' : 1.416e9*lumi/(50000*10000),
            'wab' : 0.1985e12*lumi/(100000*10000)}
#Load tritrig
infile = '/sdf/group/hps/user-data/alspellm/2016/tritrig_mc/pass4b/hadded_tritrig-beam-10kfiles-ana-smeared-corr_beamspotfix.root'
samples['tritrig'] = signalProcessor.load_data(infile, selection, cut_expression='((unc_vtx_psum > 1.2) & (unc_vtx_psum < 1.9) )', expressions=branches)
samples['tritrig']['weight'] = mc_scale['tritrig'] 

#Load wab
infile = '/sdf/group/hps/user-data/alspellm/2016/wab_mc/pass4b/hadded_wab-beam-10kfiles-ana-smeared-corr_beamspotfix.root'
samples['wab'] = signalProcessor.load_data(infile, selection, cut_expression='((unc_vtx_psum > 1.2) & (unc_vtx_psum < 1.9) )', expressions=branches)
samples['wab']['weight'] = mc_scale['wab'] 

#Combine tritrig and wab
samples['tritrig+wab+beam'] = ak.concatenate([samples['tritrig'], samples['wab']])

#init histogram of v0 projection significance values to compare data and MC background
v0projsig_h = (
    hist.Hist.new
    .StrCategory(list(samples.keys()), name='samples')
    .Reg(300, 0.0,30,label=r'Target Projected Vertex Significance $[N\sigma_{\text{V0proj}}]$')
    .Double()
)

#Fill without weights, so that histos can be converted to ROOT and retain statistical uncertainty
for sname, sample in samples.items():
    v0projsig_h.fill(sname, sample.unc_vtx_proj_sig, weight=sample.weight/ak.sum(sample.weight))

#Events that pass v0projsig in data vs MC bkg
eff_mc = round(v0projsig_h['tritrig+wab+beam',:][:hist.loc(2.0):sum]/v0projsig_h['tritrig+wab+beam',:][::sum],2)
eff_data = round(v0projsig_h['data',:][:hist.loc(2.0):sum]/v0projsig_h['data',:][::sum],2)

fig, ax = plt.subplots(figsize=(60,40))
v0projsig_h['data',:].plot(linewidth=3.0, label='10% Data', color='black')
v0projsig_h['tritrig+wab+beam',:].plot(linewidth=3.0, label='Tritrig+WAB+Beam', color='blue')
plt.axvline(x=2.0, linestyle='--', color='red', label='Cut > 2.0')
plt.text(15.0, 3e-3, f'Data Eff: {eff_data}\nMC Bkg Eff: {eff_mc}')
plt.legend()
plt.ylabel('Normalized Events')
plt.yscale('log')
plt.savefig(f'{outdir}/v0projsig_systematic_lowpsum.png')
