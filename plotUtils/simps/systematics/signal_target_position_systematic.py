#!/usr/bin/env python
# coding: utf-8

# In[76]:


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
sys.path.append('/sdf/group/hps/user-data/alspellm/2016/plotting')
import hps_plot_utils as utils

get_ipython().run_line_magic('matplotlib', 'inline')
mpl.style.use(mplhep.style.ROOT)
import math
import pickle

sys.path.append('/sdf/home/a/alspellm/src/hpstr_v62208/plotUtils/simps')
import simp_signal_2016
from simp_theory_equations import SimpEquations as simpeqs
import copy
# Set global font sizes
plt.rcParams.update({'font.size': 40,           # Font size for text
                     'axes.titlesize': 40,      # Font size for titles
                     'axes.labelsize': 40,      # Font size for axis labels
                     'xtick.labelsize': 40,     # Font size for x-axis tick labels
                     'ytick.labelsize': 40,     # Font size for y-axis tick labels
                     'lines.linewidth':3.0,
                     'legend.fontsize': 40})    # Font size for legend
plt.rcParams['font.family'] = 'DejaVu Sans' 


# In[2]:


samples = {}
branches = ['unc_vtx_ele_track_z0','unc_vtx_pos_track_z0', 'unc_vtx_z', 'unc_vtx_mass', 'unc_vtx_proj_sig']
infile = '/sdf/group/hps/user-data/alspellm/2016/systematics/simps/target_unc/Mpt5/hadd_simp_mass_60_nobeam_target_Mpt5_recon_ana.root'
infile = '/sdf/group/hps/user-data/alspellm/2016/systematics/simps/target_unc/Mpt5/hadd_simp_mass_60_nobeam_target_Mpt5_recon_ana.root'
infile = '/sdf/group/hps/user-data/alspellm/2016/systematics/simp_target_unc/nov0proj/nominal.root'
signalProcessor = simp_signal_2016.SignalProcessor(np.pi*4., 1.5)
selection = 'vtxana_radMatchTight_2016_simp_SR_analysis' #USE RADMATCHTIGHT!
samples['Mpt5'] = signalProcessor.load_data(infile, selection, cut_expression='((unc_vtx_psum > 1.0) & (unc_vtx_psum < 1.9) )', expressions=branches)
samples['Mpt5']['weight'] = 1.0

infile = '/sdf/group/hps/user-data/alspellm/2016/systematics/simps/nominal/hadd_simp_mass_60_nobeam_nominal_recon_ana.root'
signalProcessor = simp_signal_2016.SignalProcessor(np.pi*4., 1.5)
samples['nominal'] = signalProcessor.load_data(infile, selection, cut_expression='((unc_vtx_psum > 1.0) & (unc_vtx_psum < 1.9) )', expressions=branches)
samples['nominal']['weight'] = 1.0


# In[ ]:


z0_h = (
    hist.Hist.new
    .StrCategory(list(samples.keys()), name='samples')
    .Reg(60,-3.0, 3.0,label='Track y0 [mm]')
    .Double()
)
for sname, sample in samples.items():
    z0_h.fill(sname, samples[sname].unc_vtx_ele_track_z0)
    z0_h.fill(sname, samples[sname].unc_vtx_pos_track_z0)
fig, ax = plt.subplots(figsize=(20,10))
z0_h.plot()
plt.legend()

miny0_h = (
    hist.Hist.new
    .StrCategory(list(samples.keys()), name='samples')
    .Reg(60,0, 3.0,label='Min y0 [mm]')
    .Double()
)
for sname, sample in samples.items():
    miny0_h.fill(sname, samples[sname].unc_vtx_min_z0)

fig, ax = plt.subplots(figsize=(20,10))
miny0_h.plot()
nomsum = ak.sum(samples['nominal'].weight)
mpt5sum = ak.sum(samples['Mpt5'].weight)
mpt5sum/nomsum


# In[36]:


vprojsig_h = (
    hist.Hist.new
    .StrCategory(list(samples.keys()), name='samples')
    .Reg(100,0, 20.0,label='vprojsig')
    .Double()
)
for sname, sample in samples.items():
    vprojsig_h.fill(sname, samples[sname].unc_vtx_proj_sig)
vprojsig_h.plot()


# In[ ]:





# In[33]:


nominal_mask = signalProcessor.minz0_sel(samples['nominal'])
Mpt5_mask = signalProcessor.minz0_sel(samples['Mpt5'])
print(len(samples['nominal'][nom_mask].unc_vtx_z))
print(len(samples['Mpt5'][Mpt5_mask].unc_vtx_z))


# In[181]:


infile = '/sdf/group/hps/user-data/alspellm/2016/systematics/simp_target_unc/nov0proj/nominal.root'
with uproot.open(infile) as f:
    nominal_h = f['expected_signal_ap_h'].to_hist()
    test_h = f['expected_signal_vd_h'].to_hist()

infile = '/sdf/group/hps/user-data/alspellm/2016/systematics/simp_target_unc/nov0proj/target_Mpt5.root'
with uproot.open(infile) as f:
    Mpt5_h = f['expected_signal_ap_h'].to_hist()
    ratio_Mpt5_h = f['expected_signal_ap_h'].to_hist().reset()
    
infile = '/sdf/group/hps/user-data/alspellm/2016/systematics/simp_target_unc/nov0proj/target_Ppt5.root'
with uproot.open(infile) as f:
    Ppt5_h = f['expected_signal_ap_h'].to_hist()
    ratio_Ppt5_h = f['expected_signal_ap_h'].to_hist().reset()


# In[182]:


nominal_h.plot()
plt.show()
test_h.plot()
plt.show()
Mpt5_h.plot()
plt.show()
Ppt5_h.plot()


# In[79]:


#take ratio of densities, misaligned to nominal
ratio_Mpt5 = Mpt5_h.values()/nominal_h.values()
mask = nominal_h.values() < 0.0
ratio_Mpt5[mask] = 0

xbins = ratio_Mpt5_h.axes[0].centers
ybins = ratio_Mpt5_h.axes[1].centers
xgrid, ygrid = np.meshgrid(xbins, ybins, indexing='ij')
ratio_Mpt5_h.reset()
ratio_Mpt5_h.fill(xgrid.flatten(), ygrid.flatten(), weight=ratio_Mpt5.flatten())
fig, ax = plt.subplots(figsize=(25,15))
#ratio_Mpt5_h.plot(cmin=0.9, cmax=np.max(ratio_Mpt5.flatten()))
ratio_Mpt5_h.plot(cmin=0.8, cmax=1.2, cmap='RdYlBu')


# In[80]:


#take ratio of densities, misaligned to nominal
ratio_Ppt5 = Ppt5_h.values()/nominal_h.values()
mask = nominal_h.values() < 0.0
ratio_Ppt5[mask] = 0

xbins = ratio_Ppt5_h.axes[0].centers
ybins = ratio_Ppt5_h.axes[1].centers
xgrid, ygrid = np.meshgrid(xbins, ybins, indexing='ij')
ratio_Ppt5_h.reset()
ratio_Ppt5_h.fill(xgrid.flatten(), ygrid.flatten(), weight=ratio_Ppt5.flatten())
fig, ax = plt.subplots(figsize=(25,15))
ratio_Ppt5_h.plot(cmin=0.8, cmax=1.2, cmap='seismic')


# In[184]:


##### take ratio of densities, misaligned to nominal
ratio_PM = Ppt5_h.values()/Mpt5_h.values()
#mask = Ppt5_h.values() < 0.5
#ratio_PM[mask] = 0

xbins = ratio_Ppt5_h.axes[0].centers
ybins = ratio_Ppt5_h.axes[1].centers
xgrid, ygrid = np.meshgrid(xbins, ybins, indexing='ij')
ratio_Ppt5_h.reset()
ratio_Ppt5_h.fill(xgrid.flatten(), ygrid.flatten(), weight=ratio_PM.flatten())
fig, ax = plt.subplots(figsize=(25,15))
ratio_Ppt5_h.plot(cmin=0.90, cmax=1.1, cmap='seismic')
plt.text(124, -5.4,'Expected Signal Ratio\n Between Off-Nominal Targets' , horizontalalignment='center')
plt.ylim(-6.25, -4.7)
#plt.xlim(79.7,199.2)
plt.xlim(50.0,210.0)
plt.ylabel(r'$\log{\epsilon^2}$', fontsize=50)
plt.xlabel('A\' Invariant Mass [MeV]')
plt.savefig('signal_target_uncertainty_offnominal_ratio_2d.png')

masses = []
minvalues = []
for m, mass in enumerate(ratio_Ppt5_h.axes[0].centers):
    values = ratio_Ppt5_h.values()[m]
    ybins = ratio_Ppt5_h.axes[1].centers
    mask = np.where((ybins > -6.25) & (ybins < -4.7) )
    values_masked = values[mask]
    minv = np.min(values_masked)
    if not np.isfinite(minv):
        continue
    masses.append(mass)
    minvalues.append(minv)

coefficients = np.polyfit(masses, minvalues, 4)
print(coefficients)
fitfunc = np.poly1d(coefficients)
xfit = np.linspace(min(masses), max(masses),100)
yfit = fitfunc(xfit)

fig, ax = plt.subplots(figsize=(25,15))
plt.scatter(masses, minvalues, s=800, marker='+',  color='black', label='Minimum Ratio')
plt.plot(xfit, yfit, color='darkred', label='4th-Order Fit')
plt.axhline(y=1.0, linestyle='--', color='black')
plt.ylim(0.5,1.2)
plt.xlim(50.0,210.)
plt.xlabel('A\' Invariant Mass [MeV]')
plt.ylabel('Expected Signal Ratio')
plt.legend()
plt.savefig('signal_target_uncertainty_offnominal_v2.png')


# In[ ]:





# In[180]:


import numpy as np
import scipy.stats as stats

n_sigma = 1.5
percentage = stats.norm.cdf(n_sigma) - stats.norm.cdf(-n_sigma)
print(percentage)

n_sigma = 1.5 - (1.5*0.087)
percentage_2 = stats.norm.cdf(n_sigma) - stats.norm.cdf(-n_sigma)
print(percentage_2)
print(1 -(percentage_2/percentage))


# In[147]:




