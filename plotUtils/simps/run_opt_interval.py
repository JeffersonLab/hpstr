#!/usr/bin/python3
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

#############################################################################################
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
###########################################################################################   

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

#Initialize Signal Processor
signalProcessor = simp_signal_2016.SignalProcessor(mpifpi=mpifpi, nsigma=nsigma)

#Load Data
data = ak.Array([])
if args.tenpct:
    outfilename = f'{outfilename}_10pct'
    #Load 10% data signal region
    inv_mass_range = (30,124)
    print('Loading 10% Data')
    branches = ["unc_vtx_mass","unc_vtx_psum", "unc_vtx_ele_track_z0", "unc_vtx_pos_track_z0", "unc_vtx_z", "unc_vtx_proj_sig"]
    infile = '/sdf/group/hps/user-data/alspellm/2016/data/full_hadd_blpass4c_ana.root'
    selection = 'vtxana_Tight_2016_simp_reach_SR'
    data = signalProcessor.load_data(infile,selection, expressions=branches)
    data['weight'] = 1.0

else:
    outfilename = f'{outfilename}_100pct'
    #Load 100% data
    print('Loading 100% Data')
    inv_mass_range = (30,200)
    branches = ["unc_vtx_mass","unc_vtx_psum", "unc_vtx_ele_track_z0", "unc_vtx_pos_track_z0", "unc_vtx_z", "unc_vtx_proj_sig"]
    indir = '/fs/ddn/sdf/group/hps/users/alspellm/data_storage/pass4kf/pass4kf_ana_20240513'
    #If high psum, can look at all masses
    if args.highPsum:
        selection = 'vtxana_Tight_2016_simp_reach_CR'
        mass_safety = 'unc_vtx_mass*1000. >= 0'
    else:
        selection = 'vtxana_Tight_2016_simp_reach_SR'
        #mass_safety = 'unc_vtx_mass*1000. > 135' #CANT LOOK BELOW THIS MASS UNTIL UNBLINDING!
        #inv_mass_range = (135,200)
        mass_safety = 'unc_vtx_mass*1000. > 0.0' #UNBLINDED!
        inv_mass_range = (30, 124)

    for filename in sorted(os.listdir(indir)):
        if not filename.endswith('.root'):
            continue
        run = filename.split('_')[4]
        print('Loading Run ', run)
        infile = os.path.join(indir,filename)
        data = ak.concatenate([data, signalProcessor.load_data(infile, selection, cut_expression=mass_safety, expressions=branches)])
    data['weight'] = 1.0


#Set the differential radiative trident rate lookup table used to scale expected signal
print('Load lookup table')
cr_data = '/sdf/group/hps/user-data/alspellm/2016/data/hadd_BLPass4c_1959files.root'
full_lumi_path = '/fs/ddn/sdf/group/hps/users/alspellm/data_storage/pass4kf/pass4kf_ana_20240513'
preselection = "vtxana_Tight_nocuts"
signal_mass_range = [x for x in range(20,130,1)]
signalProcessor.set_diff_prod_lut(cr_data, preselection, signal_mass_range, tenpct, full_lumi_path)

#Initialize the range of epsilon2
#masses = [x for x in range(50,56,2)]
masses = [x for x in range(inv_mass_range[0], inv_mass_range[-1]+2,2)]
#masses = [x for x in range(68,100, 2)]
ap_masses = [round(x*signalProcessor.mass_ratio_ap_to_vd,1) for x in masses]
eps2_range = np.logspace(-4.0,-8.0,num=1000)
logeps2_range = np.log10(eps2_range)
min_eps = min(np.log10(eps2_range))
max_eps = max(np.log10(eps2_range))
num_bins = len(eps2_range)

#make histos to store results
exclusion_conf_h = (
    hist.Hist.new
    .Reg(len(masses), np.min(masses),np.max(masses),label='v_{D} Invariant Mass [MeV]')
    .Reg(len(eps2_range), min_eps,max_eps,label=r'$log10(\epsilon^2)$')
    .Double()
)
exclusion_bestk_h = (
    hist.Hist.new
    .Reg(len(masses), np.min(masses),np.max(masses),label='v_{D} Invariant Mass [MeV]')
    .Reg(len(eps2_range), min_eps,max_eps,label=r'$log10(\epsilon^2)$')
    .Double()
)
total_yield_h = (
    hist.Hist.new
    .Reg(len(masses), np.min(masses),np.max(masses),label='v_{D} Invariant Mass [MeV]')
    .Reg(len(eps2_range), min_eps,max_eps,label=r'$log10(\epsilon^2)$')
    .Double()
)
excluded_signal_h = (
    hist.Hist.new
    .Reg(len(masses), np.min(masses),np.max(masses),label='v_{D} Invariant Mass [MeV]')
    .Reg(len(eps2_range), min_eps,max_eps,label=r'$log10(\epsilon^2)$')
    .Double()
)
sensitivity_h = (
    hist.Hist.new
    .Reg(len(masses), np.min(masses),np.max(masses),label='v_{D} Invariant Mass [MeV]')
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
    .Reg(len(masses), np.min(masses),np.max(masses),label='v_{D} Invariant Mass [MeV]')
    .Reg(len(eps2_range), min_eps,max_eps,label=r'$log10(\epsilon^2)$')
    .Double()
)
exclusion_conf_minus1_h = (
    hist.Hist.new
    .Reg(len(masses), np.min(masses),np.max(masses),label='v_{D} Invariant Mass [MeV]')
    .Reg(len(eps2_range), min_eps,max_eps,label=r'$log10(\epsilon^2)$')
    .Double()
)

#######################################################################################################################################

#Load lookup table
lookuptable_path = '/fs/ddn/sdf/group/hps/users/alspellm/mc_storage/opt_int_lookuptable_large.p'
lookuptable_path = '/sdf/home/a/alspellm/src/hpstr/plotUtils/simps/interval_ntrials_10000.p'
#lookuptable_path = '/fs/ddn/sdf/group/hps/users/alspellm/mc_storage/opt_int_lookuptable_max50_10ktoys.p'
lookuptable_path = '/fs/ddn/sdf/group/hps/users/alspellm/mc_storage/opt_int_lookuptable_max25_10ktoys_0.05steps_v2.p'
ntrials = 10000 #number of toy events thrown for each mu in lookup table
with open(lookuptable_path, 'rb') as f:
    # Load the object from the pickle file
    lookupTable = pickle.load(f)
    
#open output file
outfile = uproot.recreate(f'{outfilename}.root')

for signal_mass in masses:
    #Histograms for each mass
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

    #Set signal window
    mass_low = signal_mass - signalProcessor.mass_resolution(signal_mass)*nsigma
    mass_high = signal_mass + signalProcessor.mass_resolution(signal_mass)*nsigma

    #Build the selection for data
    zcut_sel = signalProcessor.zcut_sel(data)
    vprojsig_sel = signalProcessor.vprojsig_sel(data)
    minz0_sel = signalProcessor.minz0_sel(data)
    sameside_sel = signalProcessor.sameside_z0_cut(data)
    masswindow_sel = signalProcessor.mass_sel(data, signal_mass)
    #Set signal/control region
    if not args.tenpct and args.highPsum:
        psum_sel = signalProcessor.psum_sel(data, case='cr')
    elif args.tenpct and not args.highPsum:
        psum_sel = signalProcessor.psum_sel(data, case='sr')
    elif args.tenpct and args.highPsum:
        psum_sel = signalProcessor.psum_sel(data, case='cr')
    else:
        psum_sel = signalProcessor.psum_sel(data, case='sr')
        print('UNBLINDED!')
    tight_sel = np.logical_and.reduce([zcut_sel, vprojsig_sel, sameside_sel, psum_sel, minz0_sel, masswindow_sel])
    #tight_sel = np.logical_and.reduce([zcut_sel, vprojsig_sel, psum_sel, masswindow_sel])
    data_z = data[tight_sel].unc_vtx_z
    print(data_z)
        
    #Load MC Signal
    indir = '/sdf/group/hps/user-data/alspellm/2016/simp_mc/pass4b/beam/smeared'
    signal_pre_readout_path = lambda mass: f'/sdf/group/hps/user-data/alspellm/2016/simp_mc/pass4b/nobeam/mass_{mass}_simp_2pt3_slic_hadd_ana.root'
    signal_path = lambda mass: f'{indir}/mass_{mass}_hadd-simp-beam_ana_smeared_corr.root'
    signal_selection = 'vtxana_radMatchTight_2016_simp_SR_analysis'

    #Get the total signal yield as a function of eps2
    total_yield_per_epsilon2 = signalProcessor.total_signal_production_per_epsilon2(signal_mass)
    print('Total Yield Per eps2: ', total_yield_per_epsilon2)

    print('Load Signal ', signal_path(signal_mass))
    signal = signalProcessor.load_signal(signal_path(signal_mass), signal_pre_readout_path(signal_mass), signal_mass, signal_selection)

    #Build the selection for signal
    zcut_sel = signalProcessor.zcut_sel(signal)
    vprojsig_sel = signalProcessor.vprojsig_sel(signal)
    minz0_sel = signalProcessor.minz0_sel(signal)
    sameside_sel = signalProcessor.sameside_z0_cut(signal)
    masswindow_sel = signalProcessor.mass_sel(signal, signal_mass)
    #Set signal/control region
    if not args.tenpct and args.highPsum:
        psum_sel = signalProcessor.psum_sel(signal, case='cr')
    elif args.tenpct and not args.highPsum:
        psum_sel = signalProcessor.psum_sel(signal, case='sr')
    elif args.tenpct and args.highPsum:
        psum_sel = signalProcessor.psum_sel(signal, case='cr')
    else:
        psum_sel = signalProcessor.psum_sel(signal, case='sr')
        print('UNBLINDED!')
    tight_sel = np.logical_and.reduce([zcut_sel, vprojsig_sel, sameside_sel, psum_sel, minz0_sel, masswindow_sel])
    signal = signal[tight_sel]

    #Loop over eps2 values and reweight the signal
    print('Looping over eps2')
    for i, eps2 in enumerate(eps2_range): 
        signal = signalProcessor.get_exp_sig_eps2(signal_mass, signal, eps2)
        total_yield = ak.sum(signal['reweighted_accxEff'])*total_yield_per_epsilon2*eps2
        if i%20 == 0:
            print(f'eps2 = {eps2}')
            print(total_yield)
        
        #Make signal efficiency in recon z
        exp_sig_eff_z = (
            hist.Hist.new
            .Reg(140, -40.0,100.0,label=r'Recon z [mm]')
            .Double()
        )
        exp_sig_eff_z.fill(signal.unc_vtx_z, weight=signal.reweighted_accxEff*total_yield_per_epsilon2*eps2)
        
        #Convert the data to a uniform distribution in recon z, according to the expected signal distribution
        data_uniform_z = (
            hist.Hist.new
            .Reg(101, -0.005,1.005,label=r'Recon z [mm]')
            .Double()
        )
        
        dataArray = np.zeros(len(data_z)+2)
        dataArray[0] = 0.0
        for k in range (0, len(data_z)):
            thisX = data_z[k]
            dataArray[k+1] = total_yield -  exp_sig_eff_z[hist.loc(thisX)::sum]
            
        dataArray[len(data_z)+1] = total_yield
        dataArray = dataArray/total_yield
        dataArray = np.nan_to_num(dataArray, nan=1.0)
        dataArray[0] = 0.0
        dataArray.sort()
        data_uniform_z.fill(dataArray)
        
        kints = kLargestIntervals(dataArray)
        
        #Loop through lookup table to find confidence level 
        mu_90p = 99999.9
        k_90p = -1
        conf_90p = -1.0
        
        previous_mu = 999999.9
        previous_conf = -9.9
        for i,mu in enumerate(sorted(lookupTable.keys())):
            best_k = -1
            best_conf = -1.0
            for k in sorted(lookupTable[mu].keys()):
                if k > len(kints)-1:
                    break
                x = np.max(kints[k])
                conf = np.where(lookupTable[mu][k] < x)[0].size / (ntrials)
                if conf > best_conf:
                    best_k = k
                    best_conf = conf

            #debug histos 
            confidence_level_mass_h.fill(mu, np.log10(eps2), weight=best_conf)
            best_kvalue_mass_h.fill(mu, np.log10(eps2), weight=best_k)

            #if the confidence is >= 90%, this is the upper limit
            if best_conf >= 0.9:
                mu_90p = mu
                k_90p = best_k
                conf_90p = best_conf
                #print(f'90% confidence upper limit on mu={mu_90p}, when k={k_90p}')
                #print(f'Confidence level: ', conf_90p)

                #fill debug histo. Check excluded signal value right before upper limit
                excluded_signal_minus1_h.fill(signal_mass, np.log10(eps2), weight=previous_mu)
                exclusion_conf_minus1_h.fill(signal_mass, np.log10(eps2), weight=previous_conf)
                break

            #debug. Track values just before upper limit is reached
            previous_mu = mu
            previous_conf = best_conf

                
        #Fill histogram results
        exclusion_conf_h.fill(signal_mass, np.log10(eps2), weight=conf_90p)
        exclusion_bestk_h.fill(signal_mass, np.log10(eps2), weight=k_90p)
        total_yield_h.fill(signal_mass, np.log10(eps2), weight=total_yield)
        excluded_signal_h.fill(signal_mass, np.log10(eps2), weight=mu_90p)
        sensitivity_h.fill(signal_mass, np.log10(eps2), weight=(total_yield/mu_90p))

        total_yield_ap_h.fill(signalProcessor.mass_ratio_ap_to_vd*signal_mass, np.log10(eps2), weight=total_yield)
        excluded_signal_ap_h.fill(signalProcessor.mass_ratio_ap_to_vd*signal_mass, np.log10(eps2), weight=mu_90p)
        sensitivity_ap_h.fill(signalProcessor.mass_ratio_ap_to_vd*signal_mass, np.log10(eps2), weight=(total_yield/mu_90p))

    #save mass histograms
    outfile[f'masses/confidence_levels_{signal_mass}_h'] = confidence_level_mass_h
    outfile[f'masses/best_kvalues_{signal_mass}_h'] = best_kvalue_mass_h

outfile['total_yield_h'] = total_yield_h
outfile['excluded_signal_h'] = excluded_signal_h
outfile['sensitivity_h'] = sensitivity_h
outfile['confidence_level_h'] = exclusion_conf_h
outfile['best_exclusion_k_h'] = exclusion_bestk_h

outfile['total_yield_ap_h'] = total_yield_ap_h
outfile['excluded_signal_ap_h'] = excluded_signal_ap_h
outfile['sensitivity_ap_h'] = sensitivity_ap_h

#save debug plots
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





