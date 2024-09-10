#!/usr/bin/env python
# coding: utf-8

# In[1]:


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


# In[ ]:


infile = '/sdf/group/hps/user-data/alspellm/2016/systematics/simps/expected_signals/simp_systematic_nominal.root'
with uproot.open(infile) as f:
    nominal_h = f['expected_signal_ap_h'].to_hist()

infile = '/sdf/group/hps/user-data/alspellm/2016/systematics/simps/expected_signals/simp_systematic_misaligned.root'
with uproot.open(infile) as f:
    misaligned_h = f['expected_signal_ap_h'].to_hist()
    ratio_h = f['expected_signal_ap_h'].to_hist().reset()

nominal_h.plot()
plt.show()
misaligned_h.plot()
plt.show()

#take ratio of densities, misaligned to nominal
ratio = misaligned_h.values()/nominal_h.values()
# Find where nominal_h values are less than 1.0
mask = nominal_h.values() < 1.0
# Set corresponding ratio values to 0
ratio[mask] = 0


xbins = ratio_h.axes[0].centers
ybins = ratio_h.axes[1].centers
xgrid, ygrid = np.meshgrid(xbins, ybins, indexing='ij')
ratio_h.reset()
ratio_h.fill(xgrid.flatten(), ygrid.flatten(), weight=ratio.flatten())
fig, ax = plt.subplots(figsize=(25,15))
ratio_h.plot(cmin=0.0, cmax=np.max(ratio.flatten()))



# In[ ]:


#

