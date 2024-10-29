#!/usr/bin/python3
"""
This script is used to characterize the beampsot in data and MC. The beamspot is fitted with a rotated 2D gaussian, and the
fit results are saved to a json file that is loaded by the hpstr vertex analysis processor. 
The beamspot is estimated by projected unconstrained v0 vertices back to the target location.
"""
import os
import awkward as ak
import numpy as np
import hist
from hist import Hist
import uproot
import math
import ROOT as r
import glob as glob
import re
import json
import matplotlib.pyplot as plt
import matplotlib as mpl
import mplhep
get_ipython().run_line_magic('matplotlib', 'inline')
mpl.style.use(mplhep.style.ROOT)

# SIMP tools defined in hpstr
import sys
hpstr_base = os.getenv('HPSTR_BASE')
sys.path.append(f'{hpstr_base}/plotUtils/simps')
import simp_plot_utils as utils

#=======================================================================================================================================
# INITIALIZE
#=======================================================================================================================================
# --outdir: Specify output directory

import argparse
parser = argparse.ArgumentParser(description='Process some inputs.')
parser.add_argument('--outdir', type=str, default='fit_results')
args = parser.parse_args()
outfilename = args.outdir

#=======================================================================================================================================
# Functions
#=======================================================================================================================================
def show(*, filepath = None, text_ax = None):
    """my own plt.show() equivalent that adds the HPS labels
    and could save to a file if a filepath is given.

    Parameters
    ----------
    filepath: str, pathlib.Path
        path to where the plot should be written to, extension defines type
        If None (default), don't save the figure to a file.
    text_ax: mpl.axes.Axes
        axes to put HPS labels on, helpful if there are two axes
        (e.g. in a ratio plot). If None (default), the labels are
        just put on the current axes as defined by MPL (usually last
        axes drawn on).
    """
    #mplhep.label.exp_text('HPS','Internal', ax = text_ax)
    #mplhep.label.lumitext('10% Data', ax = text_ax)
    if filepath is not None:
        # use bbox_inches='tight' to resize canvas to tightly fit all
        # of the drawn stuff
        plt.savefig(filepath, bbox_inches='tight')
    plt.show()
    
def load_data(filepath, selection, cut_expression = None, expressions=None):
    with uproot.open(filepath) as f:
        events = f[f'{selection}/{selection}_tree'].arrays(
            cut=cut_expression,
            expressions = expressions
        )
        return events
       
# Set global font sizes
plt.rcParams.update({'font.size': 50,           # Font size for text
                     'axes.titlesize': 50,      # Font size for titles
                     'axes.labelsize': 50,      # Font size for axis labels
                     'xtick.labelsize': 50,     # Font size for x-axis tick labels
                     'ytick.labelsize': 50,     # Font size for y-axis tick labels
                     'lines.linewidth':4.0,
                     'legend.fontsize': 40})    # Font size for legend

def rotate_coordinates(x, y, angle):
    """
    The beamspot fit is performed in the rotated coordinate system according to the fitted rotation angle.
    """
    x_rotated = x*math.cos(angle) - y*math.sin(angle)
    y_rotated = x*math.sin(angle) + y*math.cos(angle)
    return (x_rotated, y_rotated)

def gauss2DFit_Rotated(x, par):
    """
    Defines the rotated 2D gaussian fit function used to fit the beamspot.
    """
    amplitude = par[0]
    meanx = par[1]
    meany = par[2]
    sigmax = par[3]
    sigmay = par[4]
    angle = par[5]
    x_rotated = x[0] * math.cos(angle) - x[1] * math.sin(angle)
    y_rotated = x[0] * math.sin(angle) + x[1] * math.cos(angle)

    exponent = -0.5 * ((x_rotated - meanx)**2 / sigmax**2 + (y_rotated - meany)**2 / sigmay**2)
    return amplitude * math.exp(exponent)

def gaus1DFit(histo, xmin, xmax):
    """
    1D Gaussian fit used to seed the 2D fit. 
    """
    fit = histo.Fit("gaus", "QRS", "", xmin, xmax)
    try:
        mean = fit.Parameter(1)
    except:
        return None, None
    sig = fit.Parameter(2)
    return mean, sig

def projectVertex(target_pos, vz, pz, px, py, vx, vy):
    """
    Projects vertex back to target location using vertex momentum.
    """
    projx = vx - ((vz-target_pos)*(px/pz))
    projy = vy - ((vz-target_pos)*(py/pz))

    return projx, projy

def runVertex2DFit(vtx_proj_hh, run_fit_params, run, outdir, outfile, nsigma=1.5):
    """
    Runs the target projected vertex beamspot fitting procedure.
    Saves the fit results to a json file. 

    Args:
        vtx_proj_hh (TH2F): 2D histogram of target projected veretx x and y positions.
        run_fit_params (list(float)): List to store fit parameter results.
        run (int): Run number.
        outdir (str): Output directory.
        outfile (str): Output ROOT file.
        nsigma (float, optional): Width of rotated 2D gaussian fit in x and y.
    """

    # Get the not-rotated x and y projections to seed the fits
    projy = vtx_proj_hh.ProjectionY('projy',0, -1, "")
    mean_y, sig_y = gaus1DFit(projy, -0.5, 0.5)
    projx = vtx_proj_hh.ProjectionX('projx',0, -1, "")
    mean_x, sig_x = gaus1DFit(projx, -2.0, 2.0)
    projy.Write()
    projx.Write()
    del projy
    del projx

    if mean_y is None or mean_x is None:
        return

    # Make dir for run being fit
    outfile.cd()
    outfile.mkdir('fit_results_run_%s'%(run))
    rundir = outfile.GetDirectory('fit_results_run_%s'%(run))
    rundir.cd()

    # Perform rotated 2d gaussian fit
    fitFunc = r.TF2("gaussian", gauss2DFit_Rotated, -5.0,5.0,-1.0, 1.0, 6)
    fitFunc.SetRange(mean_x - (nsigma*sig_x), mean_y - (nsigma*sig_y), mean_x+(nsigma*sig_x), mean_y+(nsigma*sig_y))
    fitFunc.SetParameters(1.0, mean_x, mean_y, sig_x, sig_y, 1.0)
    vtx_proj_hh.Fit(fitFunc, "RS")
    params = fitFunc.GetParameters()
    xpos = params[1]
    ypos = params[2]
    xsigma = params[3]
    ysigma = params[4]
    angle = params[5]
    xrot, yrot = rotate_coordinates(xpos, ypos, -angle)

    canvas = r.TCanvas('target_proj_vtx_fits_run_%s'%(run), "Run %s Target Vertex Projection Fit"%(run),2000, 1500)
    vtx_proj_hh.GetXaxis().SetTitleSize(0.05)
    vtx_proj_hh.GetYaxis().SetTitleSize(0.05)

    canvas.SetLeftMargin(0.15)   
    canvas.SetTopMargin(0.1)     
    canvas.SetBottomMargin(0.15)

    vtx_proj_hh.GetXaxis().SetRangeUser(-.8, .8)
    vtx_proj_hh.GetYaxis().SetRangeUser(-.3, .3)
    vtx_proj_hh.SetName('target_proj_vtx_fits_run_%s'%(run))
    vtx_proj_hh.Draw("COLZ")
    fitFunc.Draw("SAME")
    vtx_proj_hh.Write()
    canvas.Write()
    canvas.SaveAs(f'{outdir}/{canvas.GetName()}.png')
    canvas.Close()

    del fitFunc
    run_fit_params[run] = [xpos, ypos, xsigma, ysigma, angle, mean_x, mean_y, sig_x, sig_y]

def writeFitResultsJson(fit_results, output_json_file):
    #Save fit results to json file
    json_data = {}
    for key, values in fit_results.items():
        entry = {'target_position':-4.3, 'rotated_mean_x': values[0], 'rotated_mean_y':values[1], 'rotated_sigma_x':values[2], 'rotated_sigma_y':values[3], 'rotation_angle_mrad': 1000.*values[4], 'unrotated_mean_x':values[5], 'unrotated_mean_y':values[6], 'unrotated_sigma_x':values[7],'unrotated_sigma_y':values[8]}
        json_data[key] = entry
        with open(output_json_file, "w") as json_file:
            json.dump(json_data, json_file, indent=4)

def run_fit_beamspot(infile, selection, outdir, outfilename, target_pos=-4.3):
    """
    Run the target vertex projection and beamspot fitting procedure.

    Args:
        infile (str): Input ROOT file to load data. 
        selection (str): Selection defined in the input file. 
        outdir (str): Output directory.
        outfilename (str): Output ROOT file name.
        target_pos (float): Target location in z [mm]. Vertices are projected back to this location.
    """
    if not os.path.exists(outdir):
        os.makedirs(outdir)
        
    #load the necessary data
    branches = ['unc_vtx_psum', 'unc_vtx_x', 'unc_vtx_y', 'unc_vtx_z', 'unc_vtx_px', 'unc_vtx_py', 'unc_vtx_pz', 'run_number']
    data = load_data(infile, selection, expressions=branches)
    
    #outfile = uproot.recreate(f'{outdir}/{outfilename}.root')
    outfile = r.TFile(f'{outdir}/{outfilename}.root',"RECREATE")
    outfile_json = f'{outdir}/{outfilename}.json'
    
    #get unique list of runs in data
    run_list = np.unique(data.run_number)
    
    fit_results={}
    for run in run_list:
        run_data = data[data.run_number == run]
        run = int(run)
    
        #Build histogram of vertex position projected to target location
        proj_x_y_h = (
            hist.Hist.new
            .Reg(100, -3.0, 3.0,label='Target Projected Vertex X [mm]')
            .Reg(500, -1.5, 1.5,label='Target Projected Vertex Y [mm]')
            .Double()
        )

        projected_x, projected_y = projectVertex(target_pos, run_data.unc_vtx_z, run_data.unc_vtx_pz, run_data.unc_vtx_px, 
                                                 run_data.unc_vtx_py, run_data.unc_vtx_x, run_data.unc_vtx_y) 
        proj_x_y_h.fill(np.array(projected_x), np.array(projected_y))

        root_proj_x_y_h = utils.cnvHistogramToROOT(proj_x_y_h)
        outfile.cd()
        c = utils.drawTH2(root_proj_x_y_h, f'target_projected_vertex_x_y_run_{run}', logZ=True)
        c.Write()
        c.SaveAs(f'{outdir}/{c.GetName()}.png')
        
        runVertex2DFit(root_proj_x_y_h, fit_results, run, outdir, outfile, nsigma=1.5)
    
    #Fit the target projected vertex x/y distribution with rotated 2d gaussian.
    #fit_results = {}
    #runVertex2DFit(root_proj_x_y_h, fit_results, run, outdir, outfile, nsigma=1.5)
    writeFitResultsJson(fit_results, outfile_json)

    outfile.Close()

#=======================================================================================================================================
# FIT BEAMSPOTS IN DATA AND MC
#=======================================================================================================================================

# MC Tritrig beamspot
# These events are projected to the target and used to characterize the beamspot
infile = '/sdf/group/hps/user-data/alspellm/2016/tritrig_mc/pass4b/tritrig-beam-hadd-10kfiles-ana-smeared-corr.root'
selection = 'vtxana_Tight_nocuts'
outfilename = f'tritrig_mc_beamspot_fit_{selection}'
# Fit the beamspot
run_fit_beamspot(infile, selection, outdir, outfilename)

# Data beamspot
# These events are projected to the target and used to characterize the beamspot
infile = '/sdf/group/hps/user-data/alspellm/2016/data/hadd_BLPass4c_1959files.root'
selection = 'vtxana_Tight_nocuts'
outdir = 'data_beampsot_fit'
outfilename = f'data_beamspot_fit_{selection}'
# Fit the beamspot
run_fit_beamspot(infile, selection, outdir, outfilename)

#=======================================================================================================================================
# PLOT RESULTS
#=======================================================================================================================================

# Load data fits
data_json = f'{outdir}/data_beamspot_fit_vtxana_Tight_nocuts.json'

runs = []
xpositions = []
ypositions=[]
xwidths=[]
ywidths=[]
angles=[]

with open(data_json, 'r') as file:
    data = json.load(file)
    for run, values in data.items():
        runs.append(run)
        xpositions.append(values['unrotated_mean_x'])
        ypositions.append(values['unrotated_mean_y'])
        xwidths.append(values['unrotated_sigma_x'])
        ywidths.append(values['unrotated_sigma_y'])
        angles.append(values['rotation_angle_mrad'])
runs = [int(x) for x in runs]


# Load MC fits
mc_json = f'{outdir}/tritrig_mc_beamspot_fit_vtxana_Tight_nocuts.json'
with open(mc_json, 'r') as file:
    data = json.load(file)
    mc_rotated_mean_x = values['unrotated_mean_x']
    mc_rotated_mean_y = values['unrotated_mean_y']
    mc_rotated_sigma_x = values['unrotated_sigma_x']
    mc_rotated_sigma_y = values['unrotated_sigma_y']
    mc_rotation_angle_mrad = values['rotation_angle_mrad')

# Plot data and MC beamspot x and y positions
fig, ax = plt.subplots(figsize=(30,20))
plt.scatter(runs, xpositions, marker='o', s=150,label='Beamspot <x> data', color='darkblue') 
plt.scatter(runs, ypositions, marker='o', s=150,label='Beamspot <y> data', color='teal') 
plt.axhline(mc_unrotated_mean_x, linestyle='--', linewidth=5.0, color='darkblue', label ='Beamspot <x> MC')
plt.axhline(mc_unrotated_mean_y, linestyle='--', linewidth=5.0, color='teal', label ='Beamspot <y> MC')
plt.xlabel('Run Number')
plt.ylabel('Beamspot Position [mm]')
plt.legend(fontsize=40)
plt.savefig(f'{outdir}/data_fitted_beamspot_positions_unrotated.pdf')

# Plot data and MC beamspot x and y widths
fig, ax = plt.subplots(figsize=(30,20))
plt.scatter(runs, xwidths, marker='^', s=150,label='Beamspot $\sigma_{x}$ data', color='darkblue') 
plt.scatter(runs, ywidths, marker='^', s=150,label='Beamspot $\sigma_{y}$ data', color='teal') 
plt.axhline(mc_unrotated_sigma_x, linestyle='--', linewidth=5.0, color='darkblue', label ='Beamspot $\sigma_{x}$ MC')
plt.axhline(mc_unrotated_sigma_y, linestyle='--', linewidth=5.0, color='teal', label ='Beamspot $\sigma_{x}$ MC')
plt.xlabel('Run Number')
plt.ylabel('Beamspot Width [um]')
plt.legend(fontsize=40)
plt.savefig(f'{outdir}/data_fitted_beamspot_widths_unrotated.pdf')

# Plot data and MC beamspot rotation angles
fig, ax = plt.subplots(figsize=(30,20))
plt.scatter(runs, angles, marker='*', s=250,label='Beamspot Rotation Angle Data', color='darkred') 
plt.axhline(mc_rotation_angle_mrad, linestyle='--', linewidth=5.0, color='darkblue', label ='Beamspot Rotation Angle MC')
plt.xlabel('Run Number')
plt.ylabel('Beam Rotation Angle [mrad]')
plt.legend(fontsize=40)
plt.savefig(f'{outdir}/data_fitted_beamspot_rotation_angles.pdf')

