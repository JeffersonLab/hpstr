#!/usr/bin/python3
"""
This script selects Moller events in MC and data, plots the invariant mass distributions, and fits them with a Gaussian
fit function to calculate the Moller mass resolutions.
This script compares data, unsmeared MC, and smeared MC.
"""
import os
import awkward as ak
import numpy as np
import hist
from hist import Hist
import uproot
import math
import ROOT as r
import matplotlib as mpl
import matplotlib.pyplot as plt
import sys

# SIMP tools defined in hpstr
hpstr_base = os.getenv('HPSTR_BASE')
sys.path.append(f'{hpstr_base}/plotUtils/simps')
import simp_plot_utils as utils


plt.rcParams.update({'font.size': 40,           # Font size for text
                     'axes.titlesize': 40,      # Font size for titles
                     'axes.labelsize': 40,      # Font size for axis labels
                     'xtick.labelsize': 40,     # Font size for x-axis tick labels
                     'ytick.labelsize': 40,     # Font size for y-axis tick labels
                     'lines.linewidth':4.0,
                     'legend.fontsize': 40})    # Font size for legend

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

def load_data(filepath, selection, cut_expression = None, expressions=None):
    with uproot.open(filepath) as f:
        events = f[f'{selection}/{selection}_tree'].arrays(
            cut=cut_expression,
            expressions = expressions
        )
        return events

def TrackFiducial(array, isData=False, isMC=False):
    """
    The Moller track fiducial regions are defined in the 2016 Bump Hunt analysis note.
    The selection is slightly different between MC and data.
    """
    ele_fid = ak.full_like(array.unc_vtx_ele_track_ecal_y, False, dtype=bool)
    pos_fid = ak.full_like(array.unc_vtx_pos_track_ecal_y, False, dtype=bool)
    
    if isData:
        ele_condition_1 = (
            (array.unc_vtx_ele_track_ecal_y > 0) &
            (array.unc_vtx_ele_track_ecal_y < 42.0) &
            (array.unc_vtx_ele_track_ecal_y < 13 - 0.26 * array.unc_vtx_ele_track_ecal_x) &
            (array.unc_vtx_ele_track_ecal_y > 18 - 0.08 * array.unc_vtx_ele_track_ecal_x) &
            (((array.unc_vtx_ele_track_ecal_x > -125) & (array.unc_vtx_ele_track_ecal_x < -95)) |
             ((array.unc_vtx_ele_track_ecal_x > -85) & (array.unc_vtx_ele_track_ecal_x < -55)))
        )
        ele_condition_2 = (
            (array.unc_vtx_ele_track_ecal_y < 0) &
            (array.unc_vtx_ele_track_ecal_y < -23) &
            (array.unc_vtx_ele_track_ecal_y < -15 + 0.08 * array.unc_vtx_ele_track_ecal_x) &
            (array.unc_vtx_ele_track_ecal_y > -18 + 0.22 * array.unc_vtx_ele_track_ecal_x) &
            (((array.unc_vtx_ele_track_ecal_x > -75) & (array.unc_vtx_ele_track_ecal_x < -45)) |
             ((array.unc_vtx_ele_track_ecal_x > -110) & (array.unc_vtx_ele_track_ecal_x < -95)))
        )
        ele_fid = ele_condition_1 | ele_condition_2
        
        pos_condition_1 = (
            (array.unc_vtx_pos_track_ecal_y > 0) &
            (array.unc_vtx_pos_track_ecal_y < 42.0) &
            (array.unc_vtx_pos_track_ecal_y < 13 - 0.26 * array.unc_vtx_pos_track_ecal_x) &
            (array.unc_vtx_pos_track_ecal_y > 18 - 0.08 * array.unc_vtx_pos_track_ecal_x) &
            (((array.unc_vtx_pos_track_ecal_x > -125) & (array.unc_vtx_pos_track_ecal_x < -95)) |
             ((array.unc_vtx_pos_track_ecal_x > -85) & (array.unc_vtx_pos_track_ecal_x < -55)))
        )
        pos_condition_2 = (
            (array.unc_vtx_pos_track_ecal_y < 0) &
            (array.unc_vtx_pos_track_ecal_y < -23) &
            (array.unc_vtx_pos_track_ecal_y < -15 + 0.08 * array.unc_vtx_pos_track_ecal_x) &
            (array.unc_vtx_pos_track_ecal_y > -18 + 0.22 * array.unc_vtx_pos_track_ecal_x) &
            (((array.unc_vtx_pos_track_ecal_x > -75) & (array.unc_vtx_pos_track_ecal_x < -45)) |
             ((array.unc_vtx_pos_track_ecal_x > -110) & (array.unc_vtx_pos_track_ecal_x < -95)))
        )
        pos_fid = pos_condition_1 | pos_condition_2

    elif isMC:
        ele_condition_1 = (
            (array.unc_vtx_ele_track_ecal_y > 0) &
            (array.unc_vtx_ele_track_ecal_y > 23) &
            (array.unc_vtx_ele_track_ecal_y > 15 - 0.1 * array.unc_vtx_ele_track_ecal_x) &
            (array.unc_vtx_ele_track_ecal_y < 12 - 0.3 * array.unc_vtx_ele_track_ecal_x) &
            (((array.unc_vtx_ele_track_ecal_x > -75) & (array.unc_vtx_ele_track_ecal_x < -50)) |
             ((array.unc_vtx_ele_track_ecal_x > -130) & (array.unc_vtx_ele_track_ecal_x < -95)))
        )
        ele_condition_2 = (
            (array.unc_vtx_ele_track_ecal_y < 0) &
            (array.unc_vtx_ele_track_ecal_y < -22) &
            (array.unc_vtx_ele_track_ecal_y < -15 + 0.1 * array.unc_vtx_ele_track_ecal_x) &
            (array.unc_vtx_ele_track_ecal_y > -15 + 0.25 * array.unc_vtx_ele_track_ecal_x) &
            (((array.unc_vtx_ele_track_ecal_x > -120) & (array.unc_vtx_ele_track_ecal_x < -94)) |
             ((array.unc_vtx_ele_track_ecal_x > -75) & (array.unc_vtx_ele_track_ecal_x < -50)))
        )
        ele_fid = ele_condition_1 | ele_condition_2
        
        pos_condition_1 = (
            (array.unc_vtx_pos_track_ecal_y > 0) &
            (array.unc_vtx_pos_track_ecal_y > 23) &
            (array.unc_vtx_pos_track_ecal_y > 15 - 0.1 * array.unc_vtx_pos_track_ecal_x) &
            (array.unc_vtx_pos_track_ecal_y < 12 - 0.3 * array.unc_vtx_pos_track_ecal_x) &
            (((array.unc_vtx_pos_track_ecal_x > -75) & (array.unc_vtx_pos_track_ecal_x < -50)) |
             ((array.unc_vtx_pos_track_ecal_x > -130) & (array.unc_vtx_pos_track_ecal_x < -95)))
        )
        pos_condition_2 = (
            (array.unc_vtx_pos_track_ecal_y < 0) &
            (array.unc_vtx_pos_track_ecal_y < -22) &
            (array.unc_vtx_pos_track_ecal_y < -15 + 0.1 * array.unc_vtx_pos_track_ecal_x) &
            (array.unc_vtx_pos_track_ecal_y > -15 + 0.25 * array.unc_vtx_pos_track_ecal_x) &
            (((array.unc_vtx_pos_track_ecal_x > -120) & (array.unc_vtx_pos_track_ecal_x < -94)) |
             ((array.unc_vtx_pos_track_ecal_x > -75) & (array.unc_vtx_pos_track_ecal_x < -50)))
        )
        pos_fid = pos_condition_1 | pos_condition_2

    array['ele_fid'] = ele_fid
    array['pos_fid'] = pos_fid
    
    filtered_array = array[ele_fid & pos_fid]
    return filtered_array

def fit_w_gaussian(histo, nsigma=2.0):
    
    #Seed initial fit with a simple gaussian fit over xmin and xmax
    xmin = histo.GetXaxis().GetBinLowEdge(histo.FindFirstBinAbove(0))
    xmax = histo.GetXaxis().GetBinLowEdge(histo.FindLastBinAbove(0))
    fitfunc = r.TF1("gaus","gaus", xmin, xmax)
    fitRes = histo.Fit(fitfunc,"QMES")
    params = fitRes.Parameters()
    errors = fitRes.Errors()
    chi2 = fitRes.Chi2()
    ndf = fitRes.Ndf()
    
    #Iterate fit by randomly seeding sigma, keep fit with best chi2
    best_chi2ndf = chi2/ndf
    best_params = params
    best_errors = errors
    for i in range(80):
        norm = best_params[0]
        mu = best_params[1]
        sigma = np.random.uniform(100,200)*0.01
        
        #establish new fit range
        xmin = mu - nsigma*sigma
        xmax = mu + nsigma*sigma
        fitfunc.SetParameter(0, norm)
        fitfunc.SetParameter(1, mu)
        fitfunc.SetParameter(2, sigma)
        fitRes = histo.Fit(fitfunc,"QMES","", xmin, xmax)
        
        params = fitRes.Parameters()
        errors = fitRes.Errors()
        chi2ndf = fitRes.Chi2()/fitRes.Ndf()
        
        if chi2ndf < best_chi2ndf:
            best_params = params
            best_chi2ndf = chi2ndf
            best_errors = errors
        
    #Refit with best parameters
    fitfunc.FixParameter(0, best_params[0])
    fitfunc.FixParameter(1, best_params[1])
    fitfunc.FixParameter(2, best_params[2])
    sigma = best_params[2]
    xmin = mu - nsigma*sigma
    xmax = mu + nsigma*sigma
    histo.Fit(fitfunc,"QMES","", xmin, xmax)
    print(fitfunc.GetParError(2))
    return histo, best_params, best_errors, best_chi2ndf

#=======================================================================================================================================
# LOAD DATA
#=======================================================================================================================================

samples = {}
branches = ["unc_vtx_mass","unc_vtx_psum", "unc_vtx_ele_track_t", "unc_vtx_pos_track_t","unc_vtx_ele_track_ecal_x",
           "unc_vtx_ele_track_ecal_y", "unc_vtx_pos_track_ecal_x", "unc_vtx_pos_track_ecal_y",
           "unc_vtx_ele_clust_x", "unc_vtx_ele_clust_y", "unc_vtx_pos_clust_x", "unc_vtx_pos_clust_y",
           "unc_vtx_ele_track_px","unc_vtx_ele_track_py","unc_vtx_pos_track_px","unc_vtx_pos_track_py",
           "unc_vtx_ele_track_pz", "unc_vtx_pos_track_pz",
            "unc_vtx_ele_track_nhits", "unc_vtx_pos_track_nhits",
           "unc_vtx_px", "unc_vtx_py", "unc_vtx_pz"]

# Read Data Mollers Unconstrained
infile = '/sdf/group/hps/user-data/alspellm/run/new_run_scripts/mollers/hadd/hadd-sample0-moller-ana.root'
cut_expression = ('( (unc_vtx_ele_track_t > -3.0) & (unc_vtx_pos_track_t > -3.0) & (unc_vtx_ele_track_t < 2.5) & (unc_vtx_pos_track_t < 2.5) & (unc_vtx_psum > 2.1) & (unc_vtx_psum < 2.45))')
selection = 'vtxana_Tight_nocuts'
samples['data'] = load_data(infile,selection, cut_expression=cut_expression, expressions=branches)
samples['data']['vertex_psum'] = np.sqrt(np.square(samples['data'].unc_vtx_px) + np.square(samples['data'].unc_vtx_py) + np.square(samples['data'].unc_vtx_pz))

# Read MC mollers unconstrained no smearing
infile = '/sdf/group/hps/user-data/alspellm/run/new_run_scripts/mollers/hadd/hadd-molv4-beamv6-PhysicsRun2016-Pass2_iss650_singles0_ana.root'
cut_expression = ('( abs(unc_vtx_ele_track_t - unc_vtx_pos_track_t) < 2.5) & (unc_vtx_psum > 2.1) & (unc_vtx_psum < 2.45) ')
samples['mc'] = load_data(infile,selection, cut_expression = cut_expression, expressions=branches)
samples['mc']['vertex_psum'] = np.sqrt(np.square(samples['mc'].unc_vtx_px) + np.square(samples['mc'].unc_vtx_py) + np.square(samples['mc'].unc_vtx_pz))

# Read MC mollers unconstrained with smearing
infile = '/sdf/group/hps/user-data/alspellm/run/new_run_scripts/mollers/hadd/hadd-molv4-beamv6_HPS-PhysicsRun2016-Pass2_iss650_singles0_ana_smeared_topbot_corr.root'
samples['mc_smear'] = load_data(infile,selection, cut_expression=cut_expression, expressions=branches)
samples['mc_smear']['vertex_psum'] = np.sqrt(np.square(samples['mc_smear'].unc_vtx_px) + np.square(samples['mc_smear'].unc_vtx_py) + np.square(samples['mc_smear'].unc_vtx_pz))


# Apply fiducial cuts
samples['data'] = TrackFiducial(samples['data'], isData=True)
samples['data_cons'] = TrackFiducial(samples['data_cons'], isData=True)
samples['mc'] = TrackFiducial(samples['mc'], isData=False, isMC=True)
samples['mc_smear'] = TrackFiducial(samples['mc_smear'], isData=False, isMC=True)

#Cut on angular relationship?
samples['data']['theta_1'] = np.arctan( np.sqrt(np.square(samples['data'].unc_vtx_ele_track_py) + np.square(samples['data'].unc_vtx_ele_track_px))/samples['data'].unc_vtx_ele_track_pz )
samples['data']['theta_2'] = np.arctan( np.sqrt(np.square(samples['data'].unc_vtx_pos_track_py) + np.square(samples['data'].unc_vtx_pos_track_px))/samples['data'].unc_vtx_pos_track_pz )
samples['data_cons']['theta_1'] = np.arctan( np.sqrt(np.square(samples['data_cons'].unc_vtx_ele_track_py) + np.square(samples['data_cons'].unc_vtx_ele_track_px))/samples['data_cons'].unc_vtx_ele_track_pz )
samples['data_cons']['theta_2'] = np.arctan( np.sqrt(np.square(samples['data_cons'].unc_vtx_pos_track_py) + np.square(samples['data_cons'].unc_vtx_pos_track_px))/samples['data_cons'].unc_vtx_pos_track_pz )
samples['mc']['theta_1'] = np.arctan( np.sqrt(np.square(samples['mc'].unc_vtx_ele_track_py) + np.square(samples['mc'].unc_vtx_ele_track_px))/samples['mc'].unc_vtx_ele_track_pz )
samples['mc']['theta_2'] = np.arctan( np.sqrt(np.square(samples['mc'].unc_vtx_pos_track_py) + np.square(samples['mc'].unc_vtx_pos_track_px))/samples['mc'].unc_vtx_pos_track_pz )
samples['mc_smear']['theta_1'] = np.arctan( np.sqrt(np.square(samples['mc_smear'].unc_vtx_ele_track_py) + np.square(samples['mc_smear'].unc_vtx_ele_track_px))/samples['mc_smear'].unc_vtx_ele_track_pz )
samples['mc_smear']['theta_2'] = np.arctan( np.sqrt(np.square(samples['mc_smear'].unc_vtx_pos_track_py) + np.square(samples['mc_smear'].unc_vtx_pos_track_px))/samples['mc_smear'].unc_vtx_pos_track_pz )

#=======================================================================================================================================
# PLOT INVARIANT MASS
#=======================================================================================================================================

# Initialize invariant mass histograms
invm_h = (
        hist.Hist.new
        .StrCategory(list(samples.keys()), name='sel')
        .Reg(100, 30.0, 70,label='Invariant Mass [MeV]')
        .Double()
)

# Fill histograms
invm_h.fill('data', samples['data'].unc_vtx_mass*1000.)#, weight = 1./len(samples['data'].unc_vtx_mass))
invm_h.fill('data_cons', samples['data_cons'].unc_vtx_mass*1000.)#, weight = 1./len(samples['data_cons'].unc_vtx_mass))
invm_h.fill('mc', samples['mc'].unc_vtx_mass*1000.)#, weight = 0.7/len(samples['mc'].unc_vtx_mass))
invm_h.fill('mc_smear', samples['mc_smear'].unc_vtx_mass*1000.)#, weight = 1.7/len(samples['mc_smear'].unc_vtx_mass))

# Plot invariant mass histograms
fig, ax = plt.subplots(figsize=(25,15))
invm_h.plot()
plt.legend()
plt.xlabel('Invariant Mass [MeV]')


# Plot showing fiducial cuts
fig, ax = plt.subplots(figsize=(25,15))
plt.scatter(samples['mc'].unc_vtx_ele_track_ecal_x, samples['mc'].unc_vtx_ele_track_ecal_y)
plt.scatter(samples['mc'].unc_vtx_pos_track_ecal_x, samples['mc'].unc_vtx_pos_track_ecal_y)
plt.xlabel('Track at Ecal x [mm]')
plt.ylabel('Track at Ecal y [mm]')
# Generate x data points
x = np.linspace(-140, -40, 1000)

# Data Cuts
# Positive Y Region
y1_data_pos = 13 - 0.26 * x
y2_data_pos = 18 - 0.08 * x

# Negative Y Region
y1_data_neg = -15 + 0.08 * x
y2_data_neg = -18 + 0.22 * x
# Plot Data Cuts
plt.plot(x, y1_data_pos, label='Data Cut y < 13 - 0.26 * x',color='black')
plt.plot(x, y2_data_pos, label='Data Cut y > 18 - 0.08 * x',color='black')
plt.plot(x, y1_data_neg, label='Data Cut y < -15 + 0.08 * x',color='black')
plt.plot(x, y2_data_neg, label='Data Cut y > -18 + 0.22 * x',color='black')
plt.text(-100,0,'Data')
plt.xlim(-160,-35)
plt.ylim(-50,50)


# Plot angles theta1 and theta2 around beam axis
# Possibly useful to get higher purity Moller sample
fig, ax = plt.subplots(figsize=(25,15))
plt.scatter(samples['mc'].theta_1, samples['mc'].theta_2)
plt.xlabel('arctan(sqrt(px^2+py^2)/pz) track 1')
plt.ylabel('arctan(sqrt(px^2+py^2)/pz) track 2')
plt.plot([0.02, 0.05],[0.05,0.02], color='red')
plt.plot([0.02, 0.058],[0.058,0.02], color='red')
coefficients = np.polyfit([0.02, 0.05],[0.05,0.02], 1)
slope, intercept = coefficients
coefficients = np.polyfit([0.02, 0.058],[0.058,0.02], 1)
slope, intercept = coefficients


fig, ax = plt.subplots(figsize=(25,15))
plt.scatter(samples['data'].theta_1, samples['data'].theta_2)
plt.xlabel('arctan(sqrt(px^2+py^2)/pz) track 1')
plt.ylabel('arctan(sqrt(px^2+py^2)/pz) track 2')
plt.plot([0.02, 0.05],[0.05,0.02], color='red')
plt.plot([0.02, 0.058],[0.058,0.02], color='red')
coefficients = np.polyfit([0.02, 0.05],[0.05,0.02], 1)
slope, intercept = coefficients
coefficients = np.polyfit([0.02, 0.058],[0.058,0.02], 1)
slope, intercept = coefficients

#=======================================================================================================================================
# FIT MOLLER MASS
#=======================================================================================================================================

# Convert invariant mass histograms to ROOT histograms
moller_masses_h = {}
for sname, sample in samples.items():
    h = utils.cnvHistogramToROOT(invm_h[sname,:])
    moller_masses_h[sname] = h


# Fit the data Moller mass peak
histo = moller_masses_h['data']
fit_histo, params, errors, chi2ndf = fit_w_gaussian(histo, nsigma=2.0)

# Plot data fit
text = [f'\mu = {round(params[1],2)} \pm {round(errors[1],3)}',f'\sigma = {round(params[2],2)} \pm {round(errors[2],3)}',f'\chi^{2}/n.d.f = {round(chi2ndf,4)}'] 
xmin = params[1] - 4.0*params[2]
xmax = params[1] + 4.0*params[2]
ymax = params[0]*1.1
c = utils.drawTH1s([histo], histo.GetName(), drawOpts=['hist'],xrange=[xmin,xmax], yrange=[0.0,ymax],size=(2040,1080), text=text, text_pos=[0.2,0.78], line_spacing=0.05)
c.SaveAs('moller_fit_data.png')

# Fit the MC Moller mass peak without FEE calibrated momentum smearing
histo = moller_masses_h['mc']
fit_histo, params, errors, chi2ndf = fit_w_gaussian(histo, nsigma=2.0)

# Plot MC Moller unsmeared fit
text = [f'\mu = {round(params[1],2)} \pm {round(errors[1],3)}',f'\sigma = {round(params[2],2)} \pm {round(errors[2],3)}',f'\chi^{2}/n.d.f = {round(chi2ndf,4)}'] 
xmin = params[1] - 4.0*params[2]
xmax = params[1] + 4.0*params[2]
ymax = params[0]*1.1
c = utils.drawTH1s([histo], histo.GetName(), drawOpts=['hist'],xrange=[xmin,xmax], yrange=[0.0,ymax],size=(2040,1080), text=text, text_pos=[0.2,0.78], line_spacing=0.05)
c.SaveAs('moller_fit_mc_unsmeared.png')


# Fit the MC Moller mass peak WITH FEE calibrated momentum smearing applied
histo = moller_masses_h['mc_smear']
fit_histo, params, errors, chi2ndf = fit_w_gaussian(histo, nsigma=2.0)

# Plot MC Moller SMEARED fit
text = [f'\mu = {round(params[1],2)} \pm {round(errors[1],3)}',f'\sigma = {round(params[2],2)} \pm {round(errors[2],3)}',f'\chi^{2}/n.d.f = {round(chi2ndf,4)}'] 
xmin = params[1] - 4.0*params[2]
xmax = params[1] + 4.0*params[2]
ymax = params[0]*1.1
c = utils.drawTH1s([histo], histo.GetName(), drawOpts=['hist'],xrange=[xmin,xmax], yrange=[0.0,ymax],size=(2040,1080), text=text, text_pos=[0.2,0.78], line_spacing=0.05)
c.SaveAs('moller_fit_mc_smeared.png')
