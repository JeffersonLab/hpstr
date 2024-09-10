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
#=======================================================================================================================================
# Initialize
#=======================================================================================================================================
# --outfilename: Specify output file name.
# --trials: Number of toy mc trials used to calculate data significance (typically millions+).
# --highPsum: If set, use high psum selection (control region).
# --tenpct: If True search for signal in 10% data. If False search for signal in 100% data.

import argparse
parser = argparse.ArgumentParser(description='Process some inputs.')
parser.add_argument('--outfilename', type=str, default='expected_background_test')
parser.add_argument('--trials', type=int, default=100000)
parser.add_argument('--highPsum', type=int, default=0)
parser.add_argument('--tenpct', type=int, default=1)

args = parser.parse_args()
outfilename = args.outfilename
t0_trials = args.trials #number of toy mc trials used to calculat p-value
nsigma=1.5 #+-1.5*mass_res (3sigma wide in total)
left_nsigma=nsigma+4.0 #mass sideband edge (sideband is 4sigma wide)
right_nsigma=nsigma+4.0
z0_floor_threshold=1000 #Defines the sideband size in min_z0. 1000 is arbitrary choice that performs well (no signal contam)
print(f'Search Window Size: +-', nsigma)

#=======================================================================================================================================
# Read data
#=======================================================================================================================================

#Import all definitions from simp_signal_2016, so that these results are consistent with the expected signal calculations
signalProcessor = simp_signal_2016.SignalProcessor(mpifpi=4.*np.pi, nsigma=1.5)

data = ak.Array([])

if args.tenpct:
    # Search for signal in 10% data
    print('Loading 10% Data')
    outfilename = f'{outfilename}_10pct'
    inv_mass_range = (26,200)
    branches = ["unc_vtx_mass","unc_vtx_psum", "unc_vtx_ele_track_z0", "unc_vtx_pos_track_z0", "unc_vtx_z", "unc_vtx_proj_sig"]
    infile = '/sdf/group/hps/user-data/alspellm/2016/data/full_hadd_blpass4c_ana.root'
    selection = 'vtxana_Tight_2016_simp_reach_SR'
    data = signalProcessor.load_data(infile,selection, expressions=branches)
    data['weight'] = 1.0

else:
    # Search for signal in 100% data
    print('Loading 100% Data')
    outfilename = f'{outfilename}_100pct'
    inv_mass_range = (30,124)
    branches = ["unc_vtx_mass","unc_vtx_psum", "unc_vtx_ele_track_z0", "unc_vtx_pos_track_z0", "unc_vtx_z", "unc_vtx_proj_sig"]
    indir = '/fs/ddn/sdf/group/hps/users/alspellm/data_storage/pass4kf/pass4kf_ana_20240513'
    mass_safety = 'unc_vtx_mass*1000. >= 0' #choose to blind certain masses if necessary

    #If high psum, can look at all masses
    if args.highPsum:
        selection = 'vtxana_Tight_2016_simp_reach_CR'
    else:
        selection = 'vtxana_Tight_2016_simp_reach_SR'

    #Loop over all 100% data vertex ana tuples
    for filename in sorted(os.listdir(indir)):
        if not filename.endswith('.root'):
            continue
        run = filename.split('_')[4]
        print('Loading Run ', run)
        infile = os.path.join(indir,filename)

        #build awkward array for 100% data
        data = ak.concatenate([data, signalProcessor.load_data(infile, selection, cut_expression=mass_safety, expressions=branches)])
    data['weight'] = 1.0


#######################################################################################################################################
#=======================================================================================================================================
# Useful Functions
#=======================================================================================================================================
poisson_low_err = lambda n : np.sqrt(n - 0.25) if n >= 0.25 else 0.0
poisson_up_err = lambda n : np.sqrt(n+0.75) + 1
def get_abcd_error(A, B, C, D, E):
    """
    Calculate the estimated background error bars using the ABCD method.

    Args:
        A,B,C,D,E (int): Number of events in each region. 

    Returns:
        sigma_F_up, sigma_F_low (float, float): Error bars on the estimated background in region F (signal region).
    """

    #Calculate poisson errors on regions A and E combined.
    sigma_AE_up = poisson_up_err(A+E)
    sigma_AE_low = poisson_low_err(A+E)

    #Calculate Gaussian errors on regions B+D and region C.
    sigma_BD = math.sqrt(B+D)
    sigma_C = math.sqrt(C)

    # Calculate the partial derivatives
    partial_AE = C / (B + D)
    partial_BD = -((A + E) * C) / ((B + D) ** 2)
    partial_C = (A + E) / (B + D)

    # Calculate the propagated uncertainty
    sigma_F_up = math.sqrt(
        (partial_AE * sigma_AE_up) ** 2 +
        (partial_BD * sigma_BD) ** 2 +
        (partial_C * sigma_C) ** 2
    )

    sigma_F_low = math.sqrt(
        (partial_AE * sigma_AE_low) ** 2 +
        (partial_BD * sigma_BD) ** 2 +
        (partial_C * sigma_C) ** 2
    )
    return sigma_F_up, sigma_F_low

def calc_exp_bkg(A, B, C, D, E):
    """
    Calculates the expected background using the counts in regions A, B, C, D, and E.

    Args:
        A, B, C, D, E (int): Number of events in each region. 

    Returns:
        exp_bkg (float): Expected background using ABCD estimation.
        counts (list[int]): List of integers corresponding to counts in each region. 

    Notes:
        If the counts in A+E < 1, set A+E = 0.4.
            0.4 is the Poisson mean where 0 is observed ~68% of the time. 
    """
    # Set the minimum count rate in A+E
    if A+E < 0.4:
        A = 0.4
        E = 0.0

    #Expected background ABCD calculation
    exp_bkg = C*((A+E)/(B+D))
    counts = [A, B, C, D, E]
    return exp_bkg, counts

def run_abcd_method(data, signal_mass):
    """
    Calculate the expected background, with errors, in the invariant mass window signal region by defining the regions ABCDEF.
    Also return the number of events observed in the signal region. 

    Args:
        data (awkward array): Input data array.
        signal_mass (float): Center of the invariant mass search window in dark vector meson mass.

    Returns:
        expected_bkg (float): Expected background in the search window signal region estimated using ABCD method.
        nobs (float): Number of events observed in the search window signal region.
        bkg_error ([float,float]): [upper,lower] error bars on the expected background.
        counts (list[float]): Number of events in each region except search window signal region [A, B, C, D, E].
    """
    
        # Define the boundaries of the invariant mass search window
        mass_low = signal_mass - signalProcessor.mass_resolution(signal_mass)*nsigma
        mass_high = signal_mass + signalProcessor.mass_resolution(signal_mass)*nsigma
        # Define the upper and lower boundaries of the mass sidebands.
        left_xlow = signal_mass - signalProcessor.mass_resolution(signal_mass)*left_nsigma
        right_xhigh = signal_mass + signalProcessor.mass_resolution(signal_mass)*right_nsigma
        print(f'Left:{left_xlow}-{mass_low} | Search:{mass_low}-{mass_high} | Right:{mass_high}-{right_xhigh}')
        
        # Apply the invariant mass search window mass selection on the data, as well as the sidebands. 
        mass_sel = {}
        mass_sel[f'search_window'] = (data.unc_vtx_mass * 1000. <= mass_high) & (data.unc_vtx_mass * 1000. >= mass_low)
        mass_sel[f'left_sideband'] = (data.unc_vtx_mass * 1000. <= mass_low) & (data.unc_vtx_mass * 1000. >= left_xlow)
        mass_sel[f'right_sideband'] = (data.unc_vtx_mass * 1000. <= right_xhigh) & (data.unc_vtx_mass * 1000. >= mass_high)
        
        # ABCD method extends the sidebands and search window into the minimum z0 (aka y0) parameter distribution.
        # Define and fill the z0 (aka y0) distributions for each region.
        min_z0_h = (
            hist.Hist.new
            .StrCategory(list(mass_sel.keys()), name='mass selection')
            .Reg(400,-0.005,2.995,label='Vtx Min Z0 [mm]')
            .Double()
        )
        min_z0_h.fill(f'search_window', data[mass_sel[f'search_window']].unc_vtx_min_z0,weight=data[mass_sel[f'search_window']].weight )
        min_z0_h.fill(f'left_sideband', data[mass_sel[f'left_sideband']].unc_vtx_min_z0, weight=data[mass_sel[f'left_sideband']].weight)
        min_z0_h.fill(f'right_sideband', data[mass_sel[f'right_sideband']].unc_vtx_min_z0, weight=data[mass_sel[f'right_sideband']].weight)
        # Regions A E and F are defined based on the value of the minimum z0 cut in region F
        minz0_coeffs = signalProcessor.get_minz0_cut()
        min_z0_cut = signalProcessor.polynomial(minz0_coeffs[0],minz0_coeffs[1],minz0_coeffs[2])(signal_mass)

        # Define the minimum z0 floor used to count events in regions B, C, and D.
        # This floor is defined so that the background estimate is weighted towards the tails of the minimum z0 distributions rather
        # than the core of the distribution. However, the tails cannot be so small that signal contamination is an issue in C. 
        xwidth = min_z0_h[f'search_window',:].axes[0].widths[0]
        xmax = min_z0_cut - 2*xwidth
        threshold = z0_floor_threshold
        while xmax > 0.0:
            sig_int = min_z0_h[f'search_window',:][hist.loc(xmax):hist.loc(min_z0_cut):sum]
            if sig_int < threshold:
                xmax = xmax - xwidth
            else:
                break
        z0_floor = round(xmax,2)
        
        # Count the events in each region
        B = min_z0_h[f'left_sideband',:][hist.loc(z0_floor):hist.loc(min_z0_cut):sum]
        A = min_z0_h[f'left_sideband',:][hist.loc(min_z0_cut)+1::sum]
        D = min_z0_h[f'right_sideband',:][hist.loc(z0_floor):hist.loc(min_z0_cut):sum]
        E = min_z0_h[f'right_sideband',:][hist.loc(min_z0_cut)+1::sum]
        C = min_z0_h[f'search_window',:][hist.loc(z0_floor):hist.loc(min_z0_cut):sum]
        counts = [A, B, C, D, E]

        #Calculate the error on the background estimate according to the counts
        bkg_error = get_abcd_error(A, B, C, D, E)
        #Calculate the expected background
        expected_bkg, counts = calc_exp_bkg(A, B, C, D, E)

        # Determine the observed number of events after applying the last selection criteria, mass and minimum z0.
        minz0_sel = signalProcessor.minz0_sel(data)
        masswindow_sel = signalProcessor.mass_sel(data, signal_mass)
        final_sel = np.logical_and.reduce([masswindow_sel, minz0_sel])
        nobs = ak.sum(data[final_sel].weight)

        min_z0_h.reset()
        return expected_bkg, nobs, bkg_error, counts

def get_t0(A, B, C, D, E, ntrials=100000):
    """
    Constructs the background-only test-statistic distribution used to calculate the significance of the data in each search window.
    This test-statistic distribution has the statistical uncertainty on the background baked into it. 

    Args:
        A, B, C, D, E (int): Counts in each region.
        ntrials (int, optional): Specify the number of toy MC trials used to construct the test-stat distribution.
                                 This should be large enough to achieve a statistically significant right tail. 

    Returns:
        t0_distribution (hist): Hist histogram of the background-only test-statistic distribution.
        Performing a one-sided tail integral of this histogram starting from the number of observed events 
            gives the local p-value of the data in the search window signal region.
    """

    # Define the background-only test-statistic distribution
    t0_distribution = (
        hist.Hist.new
        .Reg(500, 0.0, 500.0, label='Expected Background Toy MC Trials')
        .Double()
    )

    # Sample the parent distributions of each region used to estimate the background rate
    A_E_s = np.random.poisson(lam=(A+E), size=ntrials)
    B_D_s = np.random.normal(loc=(B+D), scale=np.sqrt(B+D), size=ntrials)
    C_s = np.random.normal(loc=C, scale=np.sqrt(C), size=ntrials)

    # Calculate the expected background (b) and t0 for all trials
    b = (A_E_s / B_D_s) * C_s

    #Use the expected background b as the true mean of the background parent distribution (which is an approximation)
    t0 = np.random.poisson(lam=b)

    # Fill background-only test-statistic distribution
    t0_distribution.fill(t0)

    return t0_distribution

def get_pvalue(test_stat_h, nobs):
    """ 
    Calculate the local p-value of the data in the search window signal region.

    Args:
        test_stat_h (hist histogram): Background-only test-statistic distribution returned by get_t0().
        nobs (float0: Number of events observed in the search window signal region.

    Returns:
        mean (float): Local p-value mean.
        low_err (float): Lower error bar on local p-value mean.
        up_err (float): Upper error bar on local p-value mean.
    """

    # Count the number of events in the background-only test-statistic distribution greater than the number of events observed.
    # This will be normalized to the total distribution.
    try:
        nover = test_stat_h[hist.loc(nobs)::sum]
    except:
        nover = 0.0

    # Define hist histograms to store the total number of events in the background-only test-stat distribution, and nover.
    numer_h = (
        hist.Hist.new
        .Reg(1, 0.0, 1.1, label='Events past nobs')
        .Double()
    )
    numer_h.fill(np.ones(int(nover)))

    denom_h = (
        hist.Hist.new
        .Reg(1, 0.0, 1.1, label='Total events thrown')
        .Double()
    )
    denom_h.fill(np.ones(int(test_stat_h[::sum])))
    test_stat_h.reset()

    # Convert the hist histograms to ROOT histograms so that they can be divided and have the correct error bars. 
    # Seems messy, but filling the hist histograms and then converting to ROOT is *way* faster when you have 100 million events.
    # The reason we use histograms is that we get the Clopper Pearson errors when we divide two ROOT histograms.
    histos = [numer_h, denom_h]
    uproot_file = uproot.recreate(f'tmp_cnv_histos.root')
    for i, histo in enumerate(histos):
        uproot_file[f'histo_{i}'] = histo
    uproot_file.close()
    infile = r.TFile(f'tmp_cnv_histos.root',"READ")
    for i, histo in enumerate(histos):
        histos[i] = copy.deepcopy(infile.Get(f'histo_{i}'))
    infile.Close()

    # The local p-value of the search window signal region is calculated by taking the ratio of nover to the total number of toys
    histos[0].SetBinErrorOption(1)
    histos[1].SetBinErrorOption(1)
    result_g = r.TGraphAsymmErrors()
    result_g.Divide(histos[0], histos[1], opt="cp") # Specify clopper pearson errors for correct error bars

    # Reset the histograms to clear mem
    numer_h.reset()
    denom_h.reset()

    mean = result_g.GetY()[0]
    up_err = result_g.GetErrorYhigh(0)
    low_err = result_g.GetErrorYlow(0)

    return mean, low_err, up_err

#=======================================================================================================================================
# SEARCH FOR SIGNAL: Calculates the local p-value of each invariant mass search window signal region.
#=======================================================================================================================================

#=======================================================================================================================================
# INITIALIZE
#=======================================================================================================================================

# Define the invariant mass range to perform the search for signal
inv_masses = np.array([x for x in range(inv_mass_range[0], inv_mass_range[-1])])

# Save the results to these arrays
exp_bkg_mev=[]
nobs_mev=[]
bkg_uperror_mev = []
bkg_lowerror_mev = []
abcd_counts_mev = []
pvalue_mev = []
pvalue_uperr_mev = []
pvalue_lowerr_mev = []

# Select the data set (10% or 100%) and the Psum region to search for signal
if not args.tenpct and args.highPsum:
    psum_sel = signalProcessor.psum_sel(data, case='cr')
elif args.tenpct and not args.highPsum:
    psum_sel = signalProcessor.psum_sel(data, case='sr')
elif args.tenpct and args.highPsum:
    psum_sel = signalProcessor.psum_sel(data, case='cr')
else:
    psum_sel = signalProcessor.psum_sel(data, case='sr')

# Initialize subset of the pre-defined selections. These do not yet include mass selection or minimum z0 (aka y0) cut.
# Those cuts can't be applied before the background estimation is completed. 
zcut_sel = signalProcessor.zcut_sel(data) # Cut on reconstructed vertex z < -4.3 mm
vprojsig_sel = signalProcessor.vprojsig_sel(data) # Cut events with target projected vertex significance > 2.0
sameside_sel = signalProcessor.sameside_z0_cut(data) # Cut events where both tracks have same-side z0 (degenerate cut...)
initial_sel = np.logical_and.reduce([zcut_sel, vprojsig_sel, psum_sel, sameside_sel]) # Combine initial selections

#=======================================================================================================================================
# RUN SEARCH FOR SIGNAL
#=======================================================================================================================================

# Loop over invariant mass range
for m,mass in enumerate(inv_masses):
    print(f'Running Signal Mass Window Center {mass}')

    # Estimate the background in the search window signal region using ABCD method
    exp_bkg, nobs, bkg_error, counts = run_abcd_method(data[initial_sel], mass)
    bkg_lowerror_mev.append(bkg_error[1])
    bkg_uperror_mev.append(bkg_error[0])
    exp_bkg_mev.append(exp_bkg)
    nobs_mev.append(nobs)
    abcd_counts_mev.append(counts)
    print(f'background estimate: {exp_bkg} | nobs: {nobs} | counts: {counts} | bkg error: {bkg_error}')

    #Calculate the p-value by building the test statistic distribution t0
    # Calculate the significance of the data (local p-value) by constructing the background-only test-statistic distribution
    # and then performing a one-tailed integral starting at the observed number of events in the search window signal region
    t0_distr_h = get_t0(counts[0], counts[1], counts[2], counts[3], counts[4], ntrials=t0_trials)
    pmean, plow_err, pup_err = get_pvalue(t0_distr_h, nobs)
    pvalue_uperr_mev.append(pup_err)
    pvalue_lowerr_mev.append(plow_err)
    pvalue_mev.append(pmean)
    print('local pvalue: ', pmean)

# Convert the search results into numpy arrays
inv_masses = np.array(inv_masses, dtype=float)
exp_bkg_mev = np.array(exp_bkg_mev, dtype=float)
nobs_mev = np.array(nobs_mev, dtype=float)
bkg_uperror_mev = np.array(bkg_uperror_mev, dtype=float)
bkg_lowerror_mev = np.array(bkg_lowerror_mev, dtype=float)
pvalue_mev = np.array(pvalue_mev , dtype=float)
pvalue_uperr_mev = np.array(pvalue_uperr_mev, dtype=float)
pvalue_lowerr_mev = np.array(pvalue_lowerr_mev, dtype=float)


# Save the expected background with errors in a ROOT TGraphAsymmErrors
expected_bkg_g = r.TGraphAsymmErrors(len(inv_masses), inv_masses, exp_bkg_mev, np.zeros(len(inv_masses)), np.zeros(len(inv_masses)), bkg_lowerror_mev, bkg_uperror_mev)
expected_bkg_g.SetName('expected_background')
expected_bkg_g.SetTitle('Expected Background;Vd Invariant Mass [MeV]; Events')

# Save the number of observed events in each search window signal region as a ROOT Tgraph
nobs_g = r.TGraph(len(inv_masses), inv_masses, nobs_mev)
nobs_g.SetName('Nobs')
nobs_g.SetTitle('Observed;Vd Invariant Mass [MeV]; Events')

# Save the local p-value with errors for each search window signal region as a ROOT TGraphAsymmErrors
pvalue_g = r.TGraphAsymmErrors(len(inv_masses), inv_masses, pvalue_mev, np.zeros(len(inv_masses)), np.zeros(len(inv_masses)), pvalue_lowerr_mev, pvalue_uperr_mev)
pvalue_g.SetName('local_pvalue')
pvalue_g.SetTitle('Local P-Value;Vd Invariant Mass [MeV]; local p-value')

# Calcualte the global p-value corrected for the "Look Elsewhere Effect" (LEE)
avg_resolution = np.average(np.array([signalProcessor.mass_resolution(x) for x in inv_masses]))
look_elsewhere = np.array((inv_masses[-1] - inv_masses[0])/avg_resolution)
print(f'Average mass resolution: {avg_resolution}')
print(f'Look elsewhere effect: {look_elsewhere}')

# Global p-value with LEE correction
pvalue_global_g = r.TGraphAsymmErrors(len(inv_masses), inv_masses, pvalue_mev*look_elsewhere, np.zeros(len(inv_masses)), np.zeros(len(inv_masses)), pvalue_lowerr_mev*look_elsewhere, pvalue_uperr_mev*look_elsewhere)
pvalue_global_g.SetName('global_pvalue')
pvalue_global_g.SetTitle('Global P-Value;Vd Invariant Mass [MeV]; global p-value')

# Save the search results
outfile = r.TFile(f'{outfilename}.root', "RECREATE")
outfile.cd()
expected_bkg_g.Write()
nobs_g.Write()
pvalue_g.Write()
pvalue_global_g.Write()
outfile.Close()

# Print the local and global thresholds for reference/recording
thresholds = []
thresholds_lew = []
from scipy.stats import norm
for nsigma in [1,2,3,4,5]:
    gaus_cdf = norm.cdf(nsigma)
    threshold = (1.0 - gaus_cdf)/look_elsewhere
    thresholds_lew.append(threshold)
    thresholds.append((1.0 - gaus_cdf))

print('Local Nsigma thresholds: ', thresholds)
print('Global LE thresholds: ', thresholds_lew)


