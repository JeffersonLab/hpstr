#!/usr/bin/env python
# coding: utf-8

# In[27]:


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

def cnv_tgraph_to_np(tgraph):
    # Number of points in the TGraph
    npoints = tgraph.GetN()
    
    # Retrieve X and Y values
    xvals = np.array([tgraph.GetX()[i] for i in range(npoints)])
    yvals = np.array([tgraph.GetY()[i] for i in range(npoints)])
    
    # Errors are not directly available in TGraph; setting them to zero
    errors = np.zeros(npoints)
    
    # Handle fit function if it exists
    x_fit = None
    y_fit = None
    if len(tgraph.GetListOfFunctions()) > 0:
        fitfunc = tgraph.GetListOfFunctions()[0]
        x_fit = np.linspace(fitfunc.GetXmin(), fitfunc.GetXmax(), 100)  # 100 points for the fit
        y_fit = np.array([fitfunc.Eval(x) for x in x_fit])
    
    return (xvals, yvals, errors), (x_fit, y_fit)

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
    
def fit_plot_with_poly(plot, tgraph=False, specify_n=None, set_xrange=False, xrange=(0.0, 1.0)):
    polys = []
    chi2s = []
    fstats = []
    fit_resultults = []
    
    if tgraph:
        npoints = plot.GetN()
    else:
        npoints = 0
        nBins = plot.GetNbinsX()
        for ibin in range(nBins):
            if plot.GetBinContent(ibin) > 0:
                npoints += 1
            pass
    
        
    if not specify_n:
        for n in range(12):
            fitfunc = r.TF1(f'pol{n}',f'pol{n}')
            fitfunc.SetLineColor(r.kRed)
            if set_xrange:
                fitfunc.SetRange(xrange[0], xrange[1])
                fit_result = plot.Fit(fitfunc,"RSQ")
            else:
                fit_result = plot.Fit(fitfunc,"SQ")
            fitfunc.SetLineColor(r.kRed)
            fitfunc.SetMarkerSize(0.0)
            chi2s.append(fit_result.Chi2())
            polys.append(n)
            fit_resultults.append(fit_result)

            #Perform fstat test to see how much fit improves with additional order (why does this work?)
            if n > 0:
                fstats.append( (chi2s[n-1]-chi2s[n])*(npoints-n-1)/(chi2s[n]))
            else:
                fstats.append(0.0)
        
        print(fstats)
        return None, None
    else:
        fitfunc = r.TF1(f'pol{specify_n}',f'pol{specify_n}')
        fitfunc.SetLineColor(r.kRed)
        fitfunc.SetLineWidth(5)
        if set_xrange:
            fitfunc.SetRange(xrange[0], xrange[1])
            fit_result = plot.Fit(fitfunc,"RSQ")
        else:
            fit_result = plot.Fit(fitfunc,"SQ")
        #params = np.round(fit_result.Parameters(),4)
        #errors = np.round(fit_result.Errors(),4)
        params = fit_result.Parameters()
        errors = fit_result.Errors()
        #return fit_result
        return params, errors

def polynomial(*coefficients):
    def _implementation(x):
        return sum([
            coefficient * x**power
            for power, coefficient in enumerate(coefficients)
        ])
    return _implementation


# In[3]:


signalProcessor = simp_signal_2016.SignalProcessor(np.pi*4., 1.5)
#V0 Projection Significance Data vs MC efficiency

samples = {}
mcsamples = {}
branches = ["unc_vtx_mass", "unc_vtx_psum"]

#LOAD NOMINAL
#rad
infile = '/sdf/group/hps/user-data/alspellm/2016/systematics/radacc/misalignments/hadd_2kfiles_rad_nobeam_nominal_recon_ana.root'
selection = 'vtxana_radMatchTight_nocuts' #USE RADMATCHTIGHT!
samples['nominal'] = signalProcessor.load_data(infile, selection, cut_expression='((unc_vtx_psum > 1.9) & (unc_vtx_psum < 2.4) )', expressions=branches)
#mc ana
infile = '/sdf/group/hps/user-data/alspellm/2016/systematics/radacc/misalignments/hadd_2kfiles_rad_nobeam_nominal_mc_ana.root'
slicfile = r.TFile(infile, "READ")
mcsamples['nominal'] = copy.deepcopy(slicfile.Get('mcAna/mcAna_mc622Mass_h'))
slicfile.Close()

#LOAD MISALIGNED
#rad
infile = '/sdf/group/hps/user-data/alspellm/2016/systematics/radacc/misalignments/hadd_2kfiles_rad_nobeam_misalignments_1_recon_ana.root'
selection = 'vtxana_radMatchTight_nocuts' #USE RADMATCHTIGHT!
samples['misaligned_v1'] = signalProcessor.load_data(infile, selection, cut_expression='((unc_vtx_psum > 1.9) & (unc_vtx_psum < 2.4) )', expressions=branches)
#mc ana
infile = '/sdf/group/hps/user-data/alspellm/2016/systematics/radacc/misalignments/hadd_2kfiles_rad_nobeam_misalignments_1_mc_ana.root'
slicfile = r.TFile(infile, "READ")
mcsamples['misaligned_v1'] = copy.deepcopy(slicfile.Get('mcAna/mcAna_mc622Mass_h'))
slicfile.Close()


# In[4]:


outfile = uproot.recreate('radacc_misalignment_systematic_results.root')


# In[26]:


#plot radiative peak
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
    #xvals = psum_h[sname,:].axes[0].centers
    #yvals = psum_h[sname,:].values()
    #plt.plot(xvals, yvals, color=colors[i],marker='o', markersize=20, mew=3, linestyle='',label=sname)
psum_h.plot(color=['black','darkred'], linewidth=3.0)
plt.legend()
plt.ylabel('Events')
plt.savefig('radiative_peak_misaligned.png')


# In[ ]:





# In[102]:


nbinsx = mcsamples['nominal'].GetNbinsX()
first_bin = mcsamples['nominal'].GetBinLowEdge(1)
last_bin = nbinsx*mcsamples['nominal'].GetBinWidth(1)
invmass_h = (
    hist.Hist.new
    .StrCategory(list(samples.keys()), name='samples')
    .Reg(nbinsx,first_bin,last_bin,label='Invariant Mass [MeV]')
    .Double()
)

#Fill without weights, so that histos can be converted to ROOT and retain statistical uncertainty
invmass_histos = {}
for sname, sample in samples.items():
    invmass_h.fill(sname, sample.unc_vtx_mass*1000.)
    invmass_histos[sname] = signalProcessor.cnvHistoToROOT(invmass_h[sname,:])
    invmass_histos[sname].Rebin(2)
    mcsamples[sname].Rebin(2)


# In[103]:


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
for sname, sample in samples.items():
    invmass_histos[sname] = nonUniBinning(invmass_histos[sname], 150, 4)
    mcsamples[sname] = nonUniBinning(mcsamples[sname], 150, 4)
    outfile[f'recon_{sname}'] = invmass_histos[sname]
    outfile[f'mc_{sname}'] = mcsamples[sname]


# In[110]:


#calculate radiative acceptance
fits = {}
colors = ['#d62728', '#bcbd22', '#2ca02c', '#17becf', '#1f77b4', '#9467bd', '#7f7f7f']
colors = ['black', 'darkred', 'darkblue', 'darkgreen', 'darkorange']
fig, ax = plt.subplots(2,1,figsize=(20,20))
plt.subplot(2,1,1)
plt.xlabel('Invariant Mass [MeV]')
plt.ylabel('Radiative Acceptance')
plt.ylim(0.0, .15)
plt.xlim(20.0,206.0)
for i,(sname, histo) in enumerate(invmass_histos.items()):
    ratio = invmass_histos[sname].Clone()
    ratio.Divide(mcsamples[sname])
    fitparams, _ = fit_plot_with_poly(ratio, specify_n=7, set_xrange=True, xrange=(30.0, 220.0))
    outfile[f'rad_acc_{sname}'] = ratio
    print(sname,fitparams)
    #fit_plot_with_poly(ratio, set_xrange=True, xrange=(30.0, 220.0))
    (xvals, yvals, errors), (x_fit, y_fit) = cnv_root_to_np(ratio)
    fits[sname] = (x_fit, y_fit)
    plt.errorbar(xvals, yvals, yerr=errors, linestyle='', marker='o', color=colors[i], label=sname)
    plt.plot(x_fit, y_fit, linewidth=3.0, color=colors[i])
plt.legend(fontsize=20)
#plot the real radiative acceptance (includes beam)
#radacc_off = polynomial(-0.48922505, 0.073733061, -0.0043873158, 0.00013455495, -2.3630535e-06, 2.5402516e-08, -1.7090900e-10, 7.0355585e-13, -1.6215982e-15, 1.6032317e-18)
#plt.plot(xvals, radacc_off(xvals), label='rad+beam', marker='o', color='blue')


plt.subplot(2,1,2)
fit_ratio = fits['misaligned_v1'][1]/fits['nominal'][1]
xvalues = fits['nominal'][0]
plt.plot(xvalues, fit_ratio, color='black', marker = '+', mew=5)
plt.axhline(y=1.0, linestyle='--', color='black')
plt.axhline(y=0.8, linestyle='--', color='black')
plt.xlim(20.0,206.)
plt.ylim(0.6,1.1)
plt.xlabel('A\' Invariant Mass [MeV]')
plt.ylabel('Systematic Uncertainty')

plt.savefig('radiative_acceptance_misalignment.png')


# In[105]:


sys_gr = r.TGraph(len(xvalues), xvalues, fit_ratio)
print(xvalues)
params_sys, errors_sys = fit_plot_with_poly(sys_gr, tgraph=True, specify_n = 9, set_xrange=True, xrange=(50.0, 220.0))
print(params_sys)
(xvals, yvals, errors), (x_fit, y_fit) = cnv_tgraph_to_np(sys_gr)
fig, ax = plt.subplots(figsize=(20,10))
plt.plot(xvals, yvals, marker='+', mew=3, markersize=10, color='darkblue')
plt.plot(x_fit, y_fit, linewidth=3.0, color='red')
test = polynomial(-8.7913353, 0.61710096, -0.014554635, 0.00011685881, 1.3328346e-06, -4.2065138e-08, 4.6959958e-10, -2.9405730e-12, 1.0885979e-14, -2.2317805e-17, 1.9584455e-20)
outfile['misalignment_systematic'] = sys_gr


# In[29]:


#Signal misalignment using the nominal no-beam radiative acceptance
infile = '/sdf/group/hps/user-data/alspellm/2016/systematics/simp_radacc_misalignment_v1/simp_systematic_nominal.root'
with uproot.open(infile) as f:
    nominal_h = f['expected_signal_ap_h'].to_hist()

infile = '/sdf/group/hps/user-data/alspellm/2016/systematics/simp_radacc_misalignment_v1/simp_systematic_misaligned.root'
with uproot.open(infile) as f:
    misaligned_h = f['expected_signal_ap_h'].to_hist()
    ratio_h = f['expected_signal_ap_h'].to_hist().reset()
outfile['expected_signal_nominal'] = nominal_h
outfile['expected_signal_misaligned'] = misaligned_h

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
outfile['signal_systematic_ratio'] = ratio_h

fig, ax = plt.subplots(figsize=(25,15))
ratio_h.plot(cmin=np.min(ratio.flatten()[ratio.flatten()>0.0]), cmax=np.max(ratio.flatten()))
plt.savefig('simp_radacc_misaligned_v1.png')


# In[66]:


#Combine radiative acceptance and signal systematics
#For each A' invariant mass, take the largest value, since this is in the numerator and reduces the expected signal rate
sigsys_y = []
sigsys_x = []
for xbin,mass in enumerate(ratio_h.axes[0].centers):
    sigsys = np.max(ratio_h.values()[xbin])
    if sigsys == 0.0:
        continue
    sigsys_x.append(mass)
    sigsys_y.append(sigsys)

sigsys_gr = r.TGraph(len(sigsys_x), np.array(sigsys_x), np.array(sigsys_y))
params_sigsys, errors_sigsys = fit_plot_with_poly(sigsys_gr, tgraph=True, specify_n = 5, set_xrange=True, xrange=(sigsys_x[0], sigsys_x[-1]))
#params_sigsys, errors_sigsys = fit_plot_with_poly(sigsys_gr, tgraph=True, set_xrange=True, xrange=(sigsys_x[0], sigsys_x[-1]))
print(params_sigsys)
(sigsys_x, sigsys_y, sigsys_errors), (sigsys_xfit, sigsys_yfit) = cnv_tgraph_to_np(sigsys_gr)
plt.plot(sigsys_x, sigsys_y)
plt.plot(sigsys_xfit, sigsys_yfit)
#Looks like we should just choose the overall maximum...
sigsys_final = np.max(sigsys_yfit)
print(f'Signal misalignment acceptance systematic: {sigsys_final}')


# In[93]:


#Combine the signal and radiative acceptance systematics
radsys_fitpoly = polynomial(-10.307720, 0.97578691, -0.036585723, 0.00077903787, -1.0393704e-05, 9.0187487e-08, -5.0948313e-10, 1.8078746e-12, -3.6566050e-15, 3.2111742e-18)
masses = np.array([float(x) for x in range(60,230,1)])
#Divide signal systematic by radiative acceptance
misalignmentsys = sigsys_final/radsys_fitpoly(masses)
fig, ax = plt.subplots(figsize=(25,15))
plt.plot(masses, misalignmentsys, marker='+', markersize=10, mew=3, color='black')


# In[ ]:




