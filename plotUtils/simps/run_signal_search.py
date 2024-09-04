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

#############################################################################################
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
z0_floor_threshold=1000 #Defines the sideband size in min_z0. 1000 is arbitrary choice, that performs well (no signal contam)
print(f'Search Window Size: +-', nsigma)


#Import all definitions from simp_signal_2016, so that these results are consistent with the expected signal calculations
signalProcessor = simp_signal_2016.SignalProcessor(mpifpi=4.*np.pi, nsigma=1.5)

data = ak.Array([])
if args.tenpct:
    outfilename = f'{outfilename}_10pct'
    #Load 10% data signal region
    inv_mass_range = (26,200)
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
    inv_mass_range = (30,124)
    branches = ["unc_vtx_mass","unc_vtx_psum", "unc_vtx_ele_track_z0", "unc_vtx_pos_track_z0", "unc_vtx_z", "unc_vtx_proj_sig"]
    indir = '/fs/ddn/sdf/group/hps/users/alspellm/data_storage/pass4kf/pass4kf_ana_20240513'
    mass_safety = 'unc_vtx_mass*1000. >= 0'
    #If high psum, can look at all masses
    if args.highPsum:
        selection = 'vtxana_Tight_2016_simp_reach_CR'
    else:
        selection = 'vtxana_Tight_2016_simp_reach_SR'
        #inv_mass_range = (135,200)
        #mass_safety = 'unc_vtx_mass*1000. > 135' #CANT LOOK BELOW THIS MASS UNTIL UNBLINDING!

    for filename in sorted(os.listdir(indir)):
        if not filename.endswith('.root'):
            continue
        run = filename.split('_')[4]
        print('Loading Run ', run)
        infile = os.path.join(indir,filename)
        data = ak.concatenate([data, signalProcessor.load_data(infile, selection, cut_expression=mass_safety, expressions=branches)])
    data['weight'] = 1.0


#######################################################################################################################################
poisson_low_err = lambda n : np.sqrt(n - 0.25) if n >= 0.25 else 0.0
poisson_up_err = lambda n : np.sqrt(n+0.75) + 1
def get_abcd_error(A, B, C, D, E):

    sigma_AE_up = poisson_up_err(A+E)
    sigma_AE_low = poisson_low_err(A+E)

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
    #If A+E are 0, set A+E = value, based on nsigma errors.
    #A poisson with mean=0.4 will throw 0 64% of the time
    if A+E < 0.4:
        A = 0.4
        E = 0.0
    exp_bkg = C*((A+E)/(B+D))
    counts = [A, B, C, D, E]
    return exp_bkg, counts

def run_abcd_method(data, signal_mass):
    
        #Get the search window
        mass_low = signal_mass - signalProcessor.mass_resolution(signal_mass)*nsigma
        mass_high = signal_mass + signalProcessor.mass_resolution(signal_mass)*nsigma
        left_xlow = signal_mass - signalProcessor.mass_resolution(signal_mass)*left_nsigma
        right_xhigh = signal_mass + signalProcessor.mass_resolution(signal_mass)*right_nsigma
        print(f'Left:{left_xlow}-{mass_low} | Search:{mass_low}-{mass_high} | Right:{mass_high}-{right_xhigh}')
        
        #Signal mass window
        mass_sel = {}
        mass_sel[f'search_window'] = (data.unc_vtx_mass * 1000. <= mass_high) & (data.unc_vtx_mass * 1000. >= mass_low)
        mass_sel[f'left_sideband'] = (data.unc_vtx_mass * 1000. <= mass_low) & (data.unc_vtx_mass * 1000. >= left_xlow)
        mass_sel[f'right_sideband'] = (data.unc_vtx_mass * 1000. <= right_xhigh) & (data.unc_vtx_mass * 1000. >= mass_high)
        
        #Fill z0 distributions for each region
        min_z0_h = (
            hist.Hist.new
            .StrCategory(list(mass_sel.keys()), name='mass selection')
            .Reg(400,-0.005,2.995,label='Vtx Min Z0 [mm]')
            .Double()
        )
        min_z0_h.fill(f'search_window', data[mass_sel[f'search_window']].unc_vtx_min_z0,weight=data[mass_sel[f'search_window']].weight )
        min_z0_h.fill(f'left_sideband', data[mass_sel[f'left_sideband']].unc_vtx_min_z0, weight=data[mass_sel[f'left_sideband']].weight)
        min_z0_h.fill(f'right_sideband', data[mass_sel[f'right_sideband']].unc_vtx_min_z0, weight=data[mass_sel[f'right_sideband']].weight)
        minz0_coeffs = signalProcessor.get_minz0_cut()
        min_z0_cut = signalProcessor.polynomial(minz0_coeffs[0],minz0_coeffs[1],minz0_coeffs[2])(signal_mass)
        print('min_z0_cut: ', min_z0_cut)
        #Determine the min z0 cut floor. The ratio of potential signal to background in region C should be so small
        #as to be negligible, or else the expected background in region F will be overestimated due to signal contamination in C, 
        #and our ability to make a discovery will be dramatically reduced.
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
        
        B = min_z0_h[f'left_sideband',:][hist.loc(z0_floor):hist.loc(min_z0_cut):sum]
        A = min_z0_h[f'left_sideband',:][hist.loc(min_z0_cut)+1::sum]
        D = min_z0_h[f'right_sideband',:][hist.loc(z0_floor):hist.loc(min_z0_cut):sum]
        E = min_z0_h[f'right_sideband',:][hist.loc(min_z0_cut)+1::sum]
        C = min_z0_h[f'search_window',:][hist.loc(z0_floor):hist.loc(min_z0_cut):sum]
        counts = [A, B, C, D, E]
        bkg_error = get_abcd_error(A, B, C, D, E)
        expected_bkg, counts = calc_exp_bkg(A, B, C, D, E)

        #Get number of observed events
        #tight_sel = signalProcessor.tight_selection(data, signal_mass, case=1)

        print('final sel')
        minz0_sel = signalProcessor.minz0_sel(data)
        masswindow_sel = signalProcessor.mass_sel(data, signal_mass)
        final_sel = np.logical_and.reduce([masswindow_sel, minz0_sel])
        nobs = ak.sum(data[final_sel].weight)

        min_z0_h.reset()
        return expected_bkg, nobs, bkg_error, counts

####################################### P-Value Functions ###########################################################################
def get_t0(A, B, C, D, E, ntrials=100000):

    # Include background estimate statistical uncertainty in p-value calculation
    # Sample three parent distributions
        # Gaussian for (B+D)
        # Gaussian for C
        # Poisson for (A+E)
    # Calculate expected background F. This is the mean of the expected background parent distribution, Poisson with mean F
    # Sample the F parent distribution to measure the test statistic t0
    # Build test statistic distribution

    t0_distribution = (
        hist.Hist.new
        .Reg(500, 0.0, 500.0, label='Expected Background Toy MC Trials')
        .Double()
    )

    # Vectorized sampling
    A_E_s = np.random.poisson(lam=(A+E), size=ntrials)
    B_D_s = np.random.normal(loc=(B+D), scale=np.sqrt(B+D), size=ntrials)
    C_s = np.random.normal(loc=C, scale=np.sqrt(C), size=ntrials)

    # Calculate F and t0 for all trials
    F = (A_E_s / B_D_s) * C_s
    t0 = np.random.poisson(lam=F)

    # Fill histogram
    t0_distribution.fill(t0)

    return t0_distribution

def get_pvalue(test_stat_h, nobs):

    #Get the number of events >= nobs
    try:
        nover = test_stat_h[hist.loc(nobs)::sum]
    except:
        nover = 0.0
    print('LOOK. NOVER = ', nover)

    #make a numerator and denominator histogram, convert to ROOT to get correct Poisson errors, then divide into TGraphAsymm
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

    #convert to ROOT to get exact poisson errors
    histos = [numer_h, denom_h]
    uproot_file = uproot.recreate(f'tmp_cnv_histos.root')
    for i, histo in enumerate(histos):
        uproot_file[f'histo_{i}'] = histo
    uproot_file.close()
    infile = r.TFile(f'tmp_cnv_histos.root',"READ")
    for i, histo in enumerate(histos):
        histos[i] = copy.deepcopy(infile.Get(f'histo_{i}'))
    infile.Close()

    #Divide number past versus number generated to get clopper pearson errors
    histos[0].SetBinErrorOption(1)
    histos[1].SetBinErrorOption(1)
    result_g = r.TGraphAsymmErrors()
    result_g.Divide(histos[0], histos[1], opt="cp")

    numer_h.reset()
    denom_h.reset()

    mean = result_g.GetY()[0]
    up_err = result_g.GetErrorYhigh(0)
    low_err = result_g.GetErrorYlow(0)

    return mean, low_err, up_err

####################################################################################################################################
#Loop over mass and signal
inv_masses = np.array([x for x in range(inv_mass_range[0], inv_mass_range[-1])])

exp_bkg_mev=[]
nobs_mev=[]
bkg_uperror_mev = []
bkg_lowerror_mev = []
abcd_counts_mev = []
pvalue_mev = []
pvalue_uperr_mev = []
pvalue_lowerr_mev = []

#Set signal/control region
if not args.tenpct and args.highPsum:
    psum_sel = signalProcessor.psum_sel(data, case='cr')
elif args.tenpct and not args.highPsum:
    psum_sel = signalProcessor.psum_sel(data, case='sr')
elif args.tenpct and args.highPsum:
    psum_sel = signalProcessor.psum_sel(data, case='cr')
else:
    psum_sel = signalProcessor.psum_sel(data, case='sr')

#get the Tight selection, without mass and minz0 cuts
#init_sel = signalProcessor.tight_selection(data, 0.0, case=3)
zcut_sel = signalProcessor.zcut_sel(data)
vprojsig_sel = signalProcessor.vprojsig_sel(data)
sameside_sel = signalProcessor.sameside_z0_cut(data)
initial_sel = np.logical_and.reduce([zcut_sel, vprojsig_sel, psum_sel, sameside_sel])
print(initial_sel)
print(np.max(data[initial_sel].unc_vtx_min_z0))

#Loop over invariant mass range
for m,mass in enumerate(inv_masses):
    print(f'Running Signal Mass Window Center {mass}')
    sel = zcut_sel & vprojsig_sel & psum_sel
    print('sel')
    exp_bkg, nobs, bkg_error, counts = run_abcd_method(data[initial_sel], mass)
    bkg_lowerror_mev.append(bkg_error[1])
    bkg_uperror_mev.append(bkg_error[0])
    exp_bkg_mev.append(exp_bkg)
    nobs_mev.append(nobs)
    abcd_counts_mev.append(counts)
    print(f'background estimate: {exp_bkg} | nobs: {nobs} | counts: {counts} | bkg error: {bkg_error}')

    #Calculate the p-value by building the test statistic distribution t0
    t0_distr_h = get_t0(counts[0], counts[1], counts[2], counts[3], counts[4], ntrials=t0_trials)
    pmean, plow_err, pup_err = get_pvalue(t0_distr_h, nobs)
    print('pvalue: ', pmean)
    pvalue_uperr_mev.append(pup_err)
    pvalue_lowerr_mev.append(plow_err)
    pvalue_mev.append(pmean)

#cnv results to numpy floats
inv_masses = np.array(inv_masses, dtype=float)
exp_bkg_mev = np.array(exp_bkg_mev, dtype=float)
nobs_mev = np.array(nobs_mev, dtype=float)
bkg_uperror_mev = np.array(bkg_uperror_mev, dtype=float)
bkg_lowerror_mev = np.array(bkg_lowerror_mev, dtype=float)
pvalue_mev = np.array(pvalue_mev , dtype=float)
pvalue_uperr_mev = np.array(pvalue_uperr_mev, dtype=float)
pvalue_lowerr_mev = np.array(pvalue_lowerr_mev, dtype=float)


expected_bkg_g = r.TGraphAsymmErrors(len(inv_masses), inv_masses, exp_bkg_mev, np.zeros(len(inv_masses)), np.zeros(len(inv_masses)), bkg_lowerror_mev, bkg_uperror_mev)
expected_bkg_g.SetName('expected_background')
expected_bkg_g.SetTitle('Expected Background;Vd Invariant Mass [MeV]; Events')

nobs_g = r.TGraph(len(inv_masses), inv_masses, nobs_mev)
nobs_g.SetName('Nobs')
nobs_g.SetTitle('Observed;Vd Invariant Mass [MeV]; Events')

pvalue_g = r.TGraphAsymmErrors(len(inv_masses), inv_masses, pvalue_mev, np.zeros(len(inv_masses)), np.zeros(len(inv_masses)), pvalue_lowerr_mev, pvalue_uperr_mev)
pvalue_g.SetName('local_pvalue')
pvalue_g.SetTitle('Local P-Value;Vd Invariant Mass [MeV]; local p-value')

#look elsewhere effect
avg_resolution = np.average(np.array([signalProcessor.mass_resolution(x) for x in inv_masses]))
look_elsewhere = np.array((inv_masses[-1] - inv_masses[0])/avg_resolution)
print(f'Average mass resolution: {avg_resolution}')
print(f'Look elsewhere effect: {look_elsewhere}')

pvalue_global_g = r.TGraphAsymmErrors(len(inv_masses), inv_masses, pvalue_mev*look_elsewhere, np.zeros(len(inv_masses)), np.zeros(len(inv_masses)), pvalue_lowerr_mev*look_elsewhere, pvalue_uperr_mev*look_elsewhere)
pvalue_global_g.SetName('global_pvalue')
pvalue_global_g.SetTitle('Global P-Value;Vd Invariant Mass [MeV]; global p-value')


outfile = r.TFile(f'{outfilename}.root', "RECREATE")
outfile.cd()
expected_bkg_g.Write()
nobs_g.Write()
pvalue_g.Write()
pvalue_global_g.Write()

outfile.Close()

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


