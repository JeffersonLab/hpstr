#!/usr/bin/python3
#=======================================================================================================================================
# Description: Calculates systematic uncertainty associated with target position uncertainty (0.5 mm)
# related to MC signal acceptance
# MC signal (NO beam) at nominal, -0.5 mm, and +0.5 mm

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

# Load nominal
infile = '/sdf/group/hps/user-data/alspellm/2016/systematics/simp_target_unc/nov0proj/nominal.root'
with uproot.open(infile) as f:
    nominal_h = f['expected_signal_ap_h'].to_hist()
    test_h = f['expected_signal_vd_h'].to_hist()

# Load -0.5 mm
infile = '/sdf/group/hps/user-data/alspellm/2016/systematics/simp_target_unc/nov0proj/target_Mpt5.root'
with uproot.open(infile) as f:
    Mpt5_h = f['expected_signal_ap_h'].to_hist()
    ratio_Mpt5_h = f['expected_signal_ap_h'].to_hist().reset()
    
# Load +0.5 mm
infile = '/sdf/group/hps/user-data/alspellm/2016/systematics/simp_target_unc/nov0proj/target_Ppt5.root'
with uproot.open(infile) as f:
    Ppt5_h = f['expected_signal_ap_h'].to_hist()
    ratio_Ppt5_h = f['expected_signal_ap_h'].to_hist().reset()




#take ratio of densities, misaligned to nominal

# Estimate systematic in a simple way by just taking the ratio of the expected signal rate between the two shifted target positions.
# This is probably too conservative, but what I did to finish my dissertation in time (Alic)

ratio_PM = Ppt5_h.values()/Mpt5_h.values()

xbins = ratio_Ppt5_h.axes[0].centers
ybins = ratio_Ppt5_h.axes[1].centers
xgrid, ygrid = np.meshgrid(xbins, ybins, indexing='ij')
ratio_Ppt5_h.reset()
ratio_Ppt5_h.fill(xgrid.flatten(), ygrid.flatten(), weight=ratio_PM.flatten())
fig, ax = plt.subplots(figsize=(25,15))
ratio_Ppt5_h.plot(cmin=0.90, cmax=1.1, cmap='seismic')
plt.text(124, -5.4,'Expected Signal Ratio\n Between Off-Nominal Targets' , horizontalalignment='center')
plt.ylim(-6.25, -4.7)
plt.xlim(50.0,210.0)
plt.ylabel(r'$\log{\epsilon^2}$', fontsize=50)
plt.xlabel('A\' Invariant Mass [MeV]')
plt.savefig('signal_target_uncertainty_offnominal_ratio_2d.png')

# The systematic uncertainty is a function of both mass and epsilon.
# I decided to just take the worst case scenario for each MC mass across all relevent values of epsilon (where by relevent, I mean
# values of epsilon where we were able to put a 90% upper limit on the signal rate). 
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

# Fit the systematic uncertainty as a function of mass 
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

