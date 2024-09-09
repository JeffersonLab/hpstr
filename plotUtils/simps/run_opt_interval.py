#!/usr/bin/python3
"""
This script runs the optimum interval method to calculate a 90% confidence upper limit on the SIMP signal rate as a function of
mass and epsilon.

Load in the data (in the form of the flat tuple output by the hpstr vertex analysis processor) and apply all selection criteria.
    *The vertex ana processor applies Preselection, and a few Tight cuts, but you need apply all remaining tight cuts.
Load in the MC signal (for each generated mass), and apply all selection criteria.

The reconstructed vertex z distribution of the remaining data events is transformed into a normalized uniform distribution according
to the expected MC signal shape in reconstructed vertex z. Since this shape is a function of epsilon^2, upper limit depends on both
mass and epsilon^2.

Calculating the upper limit here requires an external lookuptable that is generated using cmax.py.
"""
import os
import awkward as ak
import numpy as np
import hist
from hist import Hist
import uproot
import math
import ROOT as r
import simp_signal_2016
from simp_theory_equations import SimpEquations as simpeqs
import copy
import pickle

#=======================================================================================================================================
# FUNCTIONS
#=======================================================================================================================================
def kLargestIntervals(list_of_energies, spectrumCDF = lambda x: x):
    """
    Returns a list of the sizes of the K-largest intervals in that run according to the energy spectrum (given as a CDF).
    That is, kLargestIntervals(...)[i] is the size of the largest interval containing i events, where ‘largest’ is defined above.
    
    * Transform energies to energy cumulants
    * Add events at 0 and 1
    * Foreach k, compute interval sizes, take max
    """
    answer = {}
    list_of_energies.sort()
    energy_cumulants = spectrumCDF(list_of_energies)
    for i,interval_size in enumerate(range(len(energy_cumulants))):
        if (1 + interval_size) >= len(energy_cumulants):
            continue
            
        temp_data = energy_cumulants.copy()
        gap_sizes = temp_data[(1+interval_size):] - temp_data[0:-1*(1 + interval_size)] 

        answer[interval_size] = np.max(gap_sizes)
    return answer
#=======================================================================================================================================
# INITIALIZE
#=======================================================================================================================================
# --outfilename: Specify output file name.
# --tenpct: If True run OIM on 10% data (or single hpstr vertex ana output tuple).
# --highPsum: If True, run OIM in high Psum (CR).
# --mpifpi: Ratio of dark pion mass to dark pion decay constant (benchmarks are 3 and 4pi).
# --signal_sf: Scale the signal.
# --nsigma: Size of the signal invariant mass search window (+-nsigma)

import argparse
parser = argparse.ArgumentParser(description='Process some inputs.')
parser.add_argument('--outfilename', type=str, default='oim_results')
parser.add_argument('--tenpct', type=int, default=1)
parser.add_argument('--highPsum', type=int, default=0)
parser.add_argument('--mpifpi', type=float, default=4*np.pi)
parser.add_argument('--signal_sf', type=float, default=1.0)
parser.add_argument('--nsigma', type=float, default=1.5)

args = parser.parse_args()
outfilename = args.outfilename
mpifpi = args.mpifpi
nsigma = args.nsigma
signal_sf = args.signal_sf
tenpct = args.tenpct
print(f'Search Window Size: +-', nsigma)

#=======================================================================================================================================
# LOAD DATA
#=======================================================================================================================================
#Initialize Signal Processor
signalProcessor = simp_signal_2016.SignalProcessor(mpifpi=mpifpi, nsigma=nsigma)

data = ak.Array([])
if args.tenpct:
    # If tenpct True, run OIM on 10% data (or a single file)
    outfilename = f'{outfilename}_10pct'
    inv_mass_range = (30,124)
    print('Loading 10% Data')
    branches = ["unc_vtx_mass","unc_vtx_psum", "unc_vtx_ele_track_z0", "unc_vtx_pos_track_z0", "unc_vtx_z", "unc_vtx_proj_sig"]
    infile = '/sdf/group/hps/user-data/alspellm/2016/data/full_hadd_blpass4c_ana.root'
    selection = 'vtxana_Tight_2016_simp_reach_SR'
    data = signalProcessor.load_data(infile,selection, expressions=branches)
    data['weight'] = 1.0

else:
    # Run OIM on 100% data (multiple files in a single directory)
    outfilename = f'{outfilename}_100pct'
    print('Loading 100% Data')
    inv_mass_range = (30,200)
    branches = ["unc_vtx_mass","unc_vtx_psum", "unc_vtx_ele_track_z0", "unc_vtx_pos_track_z0", "unc_vtx_z", "unc_vtx_proj_sig"]
    indir = '/fs/ddn/sdf/group/hps/users/alspellm/data_storage/pass4kf/pass4kf_ana_20240513'

    # If highPsum is True, can look at all masses
    if args.highPsum:
        selection = 'vtxana_Tight_2016_simp_reach_CR'
        mass_safety = 'unc_vtx_mass*1000. >= 0'

    # If highPsum is False, can only look at 10% data before unblinding
    else:
        selection = 'vtxana_Tight_2016_simp_reach_SR'
        #mass_safety = 'unc_vtx_mass*1000. > 135' #CANT LOOK BELOW THIS MASS UNTIL UNBLINDING!
        mass_safety = 'unc_vtx_mass*1000. > 0.0' #UNBLINDED!
        inv_mass_range = (30, 124)

    # Loop over all input files and combine into single data array
    for filename in sorted(os.listdir(indir)):
        if not filename.endswith('.root'):
            continue
        run = filename.split('_')[4]
        print('Loading Run ', run)
        infile = os.path.join(indir,filename)
        data = ak.concatenate([data, signalProcessor.load_data(infile, selection, cut_expression=mass_safety, expressions=branches)])
    data['weight'] = 1.0


# Load the differential radiative trident rate lokup table. This scales the expected signal to the data
print('Load lookup table')
cr_data = '/sdf/group/hps/user-data/alspellm/2016/data/hadd_BLPass4c_1959files.root' # If using 10% data.
full_lumi_path = '/fs/ddn/sdf/group/hps/users/alspellm/data_storage/pass4kf/pass4kf_ana_20240513' # If using 100% data.
preselection = "vtxana_Tight_nocuts"
signal_mass_range = [x for x in range(20,130,1)]
signalProcessor.set_diff_prod_lut(cr_data, preselection, signal_mass_range, tenpct, full_lumi_path)

#=======================================================================================================================================
# INITIALIZE HISTOGRAMS
#=======================================================================================================================================

masses = [x for x in range(inv_mass_range[0], inv_mass_range[-1]+2,2)]
ap_masses = [round(x*signalProcessor.mass_ratio_ap_to_vd,1) for x in masses]
eps2_range = np.logspace(-4.0,-8.0,num=1000)
logeps2_range = np.log10(eps2_range)
min_eps = min(np.log10(eps2_range))
max_eps = max(np.log10(eps2_range))
num_bins = len(eps2_range)

exclusion_conf_h = (
    hist.Hist.new
    .Reg(len(masses)-1, np.min(masses),np.max(masses),label='v_{D} Invariant Mass [MeV]')
    .Reg(len(eps2_range), min_eps,max_eps,label=r'$log10(\epsilon^2)$')
    .Double()
)
exclusion_bestk_h = (
    hist.Hist.new
    .Reg(len(masses)-1, np.min(masses),np.max(masses),label='v_{D} Invariant Mass [MeV]')
    .Reg(len(eps2_range), min_eps,max_eps,label=r'$log10(\epsilon^2)$')
    .Double()
)
total_yield_h = (
    hist.Hist.new
    .Reg(len(masses)-1, np.min(masses),np.max(masses),label='v_{D} Invariant Mass [MeV]')
    .Reg(len(eps2_range), min_eps,max_eps,label=r'$log10(\epsilon^2)$')
    .Double()
)
excluded_signal_h = (
    hist.Hist.new
    .Reg(len(masses)-1, np.min(masses),np.max(masses),label='v_{D} Invariant Mass [MeV]')
    .Reg(len(eps2_range), min_eps,max_eps,label=r'$log10(\epsilon^2)$')
    .Double()
)
sensitivity_h = (
    hist.Hist.new
    .Reg(len(masses)-1, np.min(masses),np.max(masses),label='v_{D} Invariant Mass [MeV]')
    .Reg(len(eps2_range), min_eps,max_eps,label=r'$log10(\epsilon^2)$')
    .Double()
)

ap_masses = [signalProcessor.mass_ratio_ap_to_vd*x for x in range(inv_mass_range[0], inv_mass_range[-1]+2,2)]
total_yield_ap_h = (
    hist.Hist.new
    .Reg(len(ap_masses), np.min(ap_masses),np.max(ap_masses),label='A\' Invariant Mass [MeV]')
    .Reg(len(eps2_range), min_eps,max_eps,label=r'$log10(\epsilon^2)$')
    .Double()
)
excluded_signal_ap_h = (
    hist.Hist.new
    .Reg(len(ap_masses), np.min(ap_masses),np.max(ap_masses),label='A\' Invariant Mass [MeV]')
    .Reg(len(eps2_range), min_eps,max_eps,label=r'$log10(\epsilon^2)$')
    .Double()
)
sensitivity_ap_h = (
    hist.Hist.new
    .Reg(len(ap_masses), np.min(ap_masses),np.max(ap_masses),label='A\' Invariant Mass [MeV]')
    .Reg(len(eps2_range), min_eps,max_eps,label=r'$log10(\epsilon^2)$')
    .Double()
)

#Plot the excluded signal value right before reaching 90% confidence. Debugging purposes
excluded_signal_minus1_h = (
    hist.Hist.new
    .Reg(len(masses)-1, np.min(masses),np.max(masses),label='v_{D} Invariant Mass [MeV]')
    .Reg(len(eps2_range), min_eps,max_eps,label=r'$log10(\epsilon^2)$')
    .Double()
)
exclusion_conf_minus1_h = (
    hist.Hist.new
    .Reg(len(masses)-1, np.min(masses),np.max(masses),label='v_{D} Invariant Mass [MeV]')
    .Reg(len(eps2_range), min_eps,max_eps,label=r'$log10(\epsilon^2)$')
    .Double()
)

#=======================================================================================================================================
# RUN OPTIMUM INTERVAL METHOD
#=======================================================================================================================================

# Load OIM lookup table generated using cmax.py
lookuptable_path = '/fs/ddn/sdf/group/hps/users/alspellm/mc_storage/opt_int_lookuptable_max25_10ktoys_0.05steps_v2.p'

# Number of toy events thrown for each mu in the loaded lookup table
ntrials = 10000 # This value is defined in cmax.py when the lookup table is generated. It MUST MATCH the value used. 
with open(lookuptable_path, 'rb') as f:
    lookupTable = pickle.load(f)
    
# Open an output file to store the results
outfile = uproot.recreate(f'{outfilename}.root')

# Run OIM for each MC generated signal mass (30-124 @ 2 MeV intervals)
for signal_mass in masses:

    # Initialize histograms
    confidence_level_mass_h = (
        hist.Hist.new
        .Reg(300, 0, 30.0,label='mu')
        .Reg(len(eps2_range), min_eps,max_eps,label=r'$log10(\epsilon^2)$')
        .Double()
    )
    best_kvalue_mass_h = (
        hist.Hist.new
        .Reg(300, 0, 30.0,label='mu')
        .Reg(len(eps2_range), min_eps,max_eps,label=r'$log10(\epsilon^2)$')
        .Double()
    )

    print(f'Signal Mass {signal_mass}') 

    # Define the invariant mass search window boundaries based on the search window size
    mass_low = signal_mass - signalProcessor.mass_resolution(signal_mass)*nsigma
    mass_high = signal_mass + signalProcessor.mass_resolution(signal_mass)*nsigma

    # Build the final selection used in the signal search and apply to the data
    zcut_sel = signalProcessor.zcut_sel(data) # zcut on target position at -4.3 mm.
    vprojsig_sel = signalProcessor.vprojsig_sel(data) # Require target projected vertex significance < 2.0
    minz0_sel = signalProcessor.minz0_sel(data) # Cut on minimum track vertical impact parameter z0 (aka y0)
    sameside_sel = signalProcessor.sameside_z0_cut(data) # Cut events where both tracks have same side z0
    masswindow_sel = signalProcessor.mass_sel(data, signal_mass) # Define search window mass boundaries

    # Set the Psum selection
    if not args.tenpct and args.highPsum:
        psum_sel = signalProcessor.psum_sel(data, case='cr')
    elif args.tenpct and not args.highPsum:
        psum_sel = signalProcessor.psum_sel(data, case='sr')
    elif args.tenpct and args.highPsum:
        psum_sel = signalProcessor.psum_sel(data, case='cr')
    else:
        psum_sel = signalProcessor.psum_sel(data, case='sr')

    # Combine the selections and apply to data
    tight_sel = np.logical_and.reduce([zcut_sel, vprojsig_sel, sameside_sel, psum_sel, minz0_sel, masswindow_sel])
    data_z = data[tight_sel].unc_vtx_z
        
    #==================================================================================================================================
    # LOAD MC SIGNAL
    #==================================================================================================================================

    indir = '/sdf/group/hps/user-data/alspellm/2016/simp_mc/pass4b/beam/smeared_fixbeamspot'

    # hpstr MC ana processor output file that stores the pre-readout MC signal truth information
    signal_pre_readout_path = lambda mass: f'/sdf/group/hps/user-data/alspellm/2016/simp_mc/pass4b/nobeam/mass_{mass}_simp_2pt3_slic_hadd_ana.root'
    # hpstr vertex ana processor output tuple
    signal_path = lambda mass: f'{indir}/mass_{mass}_hadd-simp-beam_ana_smeared_corr.root'
    signal_selection = 'vtxana_radMatchTight_2016_simp_SR_analysis'

    # Calculate the total A' production rate per epsilon^2
    total_yield_per_epsilon2 = signalProcessor.total_signal_production_per_epsilon2(signal_mass)
    print('Total Yield Per eps2: ', total_yield_per_epsilon2)

    # Load signal before tight selection
    print('Load Signal ', signal_path(signal_mass))
    signal = signalProcessor.load_signal(signal_path(signal_mass), signal_pre_readout_path(signal_mass), signal_mass, signal_selection)

    # Build final selection for signal
    zcut_sel = signalProcessor.zcut_sel(signal)
    vprojsig_sel = signalProcessor.vprojsig_sel(signal)
    minz0_sel = signalProcessor.minz0_sel(signal)
    sameside_sel = signalProcessor.sameside_z0_cut(signal)
    masswindow_sel = signalProcessor.mass_sel(signal, signal_mass)

    # Set the Psum selection
    if not args.tenpct and args.highPsum:
        psum_sel = signalProcessor.psum_sel(signal, case='cr')
    elif args.tenpct and not args.highPsum:
        psum_sel = signalProcessor.psum_sel(signal, case='sr')
    elif args.tenpct and args.highPsum:
        psum_sel = signalProcessor.psum_sel(signal, case='cr')
    else:
        psum_sel = signalProcessor.psum_sel(signal, case='sr')
        print('UNBLINDED!')

    # Combine the selections and apply to MC signal
    tight_sel = np.logical_and.reduce([zcut_sel, vprojsig_sel, sameside_sel, psum_sel, minz0_sel, masswindow_sel])
    signal = signal[tight_sel]

    #==================================================================================================================================
    # CALCULATE UPPER LIMIT ON SIGNAL: As function of epsilon^2
    #==================================================================================================================================
    for i, eps2 in enumerate(eps2_range): 
        signal = signalProcessor.get_exp_sig_eps2(signal_mass, signal, eps2)
        total_yield = ak.sum(signal['reweighted_accxEff'])*total_yield_per_epsilon2*eps2
        if i%20 == 0:
            print(f'eps2 = {eps2}')
            print(total_yield)
        
        # Signal acceptance*efficiency*dark_vector_probability in reconstructed vertex z.
        # This represents the shape of the signal in 1D.
        exp_sig_eff_z = (
            hist.Hist.new
            .Reg(140, -40.0,100.0,label=r'Recon z [mm]')
            .Double()
        )
        exp_sig_eff_z.fill(signal.unc_vtx_z, weight=signal.reweighted_accxEff*total_yield_per_epsilon2*eps2)
        
        # Convert the remaining events in data to a normalized uniform distribution in reconstructed vertex z according to signal shape.
        data_uniform_z = (
            hist.Hist.new
            .Reg(101, -0.005,1.005,label=r'Recon z [mm]')
            .Double()
        )
        
        # Initialize an array to store the new data events that are transformed according to the signal shape.
        # Add endpoints to the new uniform data array, 0 in front, and 1.0 and the end. 
        dataArray = np.zeros(len(data_z)+2)
        dataArray[0] = 0.0
        for k in range (0, len(data_z)):
            thisX = data_z[k]
            dataArray[k+1] = total_yield -  exp_sig_eff_z[hist.loc(thisX)::sum] #transformation
            
        dataArray[len(data_z)+1] = total_yield
        dataArray = dataArray/total_yield # normalize distribution based on total signal rate
        dataArray = np.nan_to_num(dataArray, nan=1.0)
        dataArray[0] = 0.0
        dataArray.sort()
        data_uniform_z.fill(dataArray)
        
        # Calculate maximum gaps with k events allowed between events
        kints = kLargestIntervals(dataArray)
        
        # Loop through the lookup table to find what upper limit on the signal rate results in 90% confidence
        mu_90p = 99999.9
        k_90p = -1
        conf_90p = -1.0
        
        previous_mu = 999999.9
        previous_conf = -9.9
        # Loop over values of mu (mean expected signal rate)
        for i,mu in enumerate(sorted(lookupTable.keys())):
            best_k = -1
            best_conf = -1.0 # Store best confidence level across all values of k

            # Loop over all values of k (k events allowed in gap between data events)
            for k in sorted(lookupTable[mu].keys()):
                if k > len(kints)-1:
                    break
                x = np.max(kints[k])
                conf = np.where(lookupTable[mu][k] < x)[0].size / (ntrials) # Confidence level for this mu and k
                if conf > best_conf:
                    best_k = k
                    best_conf = conf

            # Debug histos 
            confidence_level_mass_h.fill(mu, np.log10(eps2), weight=best_conf)
            best_kvalue_mass_h.fill(mu, np.log10(eps2), weight=best_k)

            # If the condience level is >= 90%, this value of mu is the upper limit
            if best_conf >= 0.9:
                mu_90p = mu
                k_90p = best_k
                conf_90p = best_conf

                # Fill debug histos
                excluded_signal_minus1_h.fill(signal_mass, np.log10(eps2), weight=previous_mu)
                exclusion_conf_minus1_h.fill(signal_mass, np.log10(eps2), weight=previous_conf)
                break

            # More debug
            previous_mu = mu
            previous_conf = best_conf

                
        # Fill OIM results in histograms
        exclusion_conf_h.fill(signal_mass, np.log10(eps2), weight=conf_90p)
        exclusion_bestk_h.fill(signal_mass, np.log10(eps2), weight=k_90p)
        total_yield_h.fill(signal_mass, np.log10(eps2), weight=total_yield)
        excluded_signal_h.fill(signal_mass, np.log10(eps2), weight=mu_90p)
        sensitivity_h.fill(signal_mass, np.log10(eps2), weight=(total_yield/mu_90p))

        total_yield_ap_h.fill(signalProcessor.mass_ratio_ap_to_vd*signal_mass, np.log10(eps2), weight=total_yield)
        excluded_signal_ap_h.fill(signalProcessor.mass_ratio_ap_to_vd*signal_mass, np.log10(eps2), weight=mu_90p)
        sensitivity_ap_h.fill(signalProcessor.mass_ratio_ap_to_vd*signal_mass, np.log10(eps2), weight=(total_yield/mu_90p))

    # Save mass dependent histograms
    outfile[f'masses/confidence_levels_{signal_mass}_h'] = confidence_level_mass_h
    outfile[f'masses/best_kvalues_{signal_mass}_h'] = best_kvalue_mass_h

# Save results across all masses
outfile['total_yield_h'] = total_yield_h
outfile['excluded_signal_h'] = excluded_signal_h
outfile['sensitivity_h'] = sensitivity_h
outfile['confidence_level_h'] = exclusion_conf_h
outfile['best_exclusion_k_h'] = exclusion_bestk_h

outfile['total_yield_ap_h'] = total_yield_ap_h
outfile['excluded_signal_ap_h'] = excluded_signal_ap_h
outfile['sensitivity_ap_h'] = sensitivity_ap_h

# Save debug plots
outfile['excluded_signal_minus1_h'] = excluded_signal_minus1_h
outfile['exclusion_conf_minus1_h'] = exclusion_conf_minus1_h

'''
#Get sensitivity contour
sens = sensitivity_h.values()
print(sens)
indices = np.where(sens >= 0.1)
print(indices)
if len(indices) > 0:
    massx = 
    x = np.array(sensitivity_h.axes[0].centers[indices[0]])
    y = np.array(sensitivity_h.axes[1].centers[indices[1]])
    #get the contour
    cupper = []
    clower = []
    for i in set(x):
        cupper = y[np.where(x==i)[0][-1]]
        clower = y[np.where(x==i)[0][0]]
    cupper = cupper + list(reversed(clower)) + cupper[0]

    contour_g = r.TGraph(len(x), np.array(cupper,dtype=float), np.array(clower,dtype=float))
    contour_g.SetName('exclusion_contour_g')
    contour_g.SetTitle('Exclusion Contour;Invariant Mass [MeV];#epsilon^{2}')
    outfile['contour_g'] = contour_g
'''

'''
fig, ax = plt.subplots(figsize=(30,20))
total_yield_h.plot()
#plt.xlim(20.0, 126.0)
#plt.ylim(-10.8, -3.8)
plt.show()

fig, ax = plt.subplots(figsize=(30,20))
excluded_signal_h.plot()
#plt.xlim(20.0, 126.0)
#plt.ylim(-10.8, -3.8)
#plt.show()

fig, ax = plt.subplots(figsize=(30,20))
sensitivity_h.plot(cmin=1.0)
#plt.xlim(20.0, 126.0)
#plt.ylim(-10.8, -3.8)
plt.show()
'''





