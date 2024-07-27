import os
import awkward as ak
import numpy as np
import hist
from hist import Hist
import uproot
import math
import ROOT as r
import matplotlib as mpl
import copy
import mplhep
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
mpl.style.use(mplhep.style.ROOT)
import argparse
from simp_theory_equations import SimpEquations as simpeqs

class SignalProcessor:
    def __init__(self, mpifpi=4*np.pi, nsigma=1.5):
        self.nsigma = nsigma

        #SIMP parameters
        self.alpha_dark = 0.01
        self.mass_ratio_ap_to_vd = 1.66
        self.mass_ratio_ap_to_pid = 3.0
        self.mass_lepton = 0.511
        self.target_pos = -4.3
        self.mpifpi = mpifpi
        ###2016 KF MC with hit killing and momentum smearing
        self.mass_resolution = self.polynomial(.75739851, 0.031621002, 5.2949672e-05)
        self.radiative_fraction = self.polynomial(0.10541434, -0.0011737697, 7.4487930e-06, -1.6766332e-08)
        self.radiative_acceptance = self.polynomial(-0.48922505, 0.073733061, -0.0043873158, 0.00013455495, -2.3630535e-06, 2.5402516e-08, -1.7090900e-10, 7.0355585e-13, -1.6215982e-15, 1.6032317e-18)
        self.cr_psum_low = 1.9
        self.cr_psum_high = 2.4
        self.sr_psum_low = 1.0
        self.sr_psum_high = 1.9
        ###
        self.trident_differential_production = None


    @staticmethod
    def polynomial(*coefficients):
        def _implementation(x):
            return sum([
                coefficient * x**power
                for power, coefficient in enumerate(coefficients)
            ])
        return _implementation

    def load_data(self, filepath, selection, cut_expression=None, expressions=None):
        with uproot.open(filepath) as f:
            events = f[f'{selection}/{selection}_tree'].arrays(
                cut=cut_expression,
                expressions=expressions
            )
            events['unc_vtx_min_z0'] = np.minimum(abs(events.unc_vtx_ele_track_z0), abs(events.unc_vtx_pos_track_z0))
            return events

    @staticmethod
    def safe_divide(numerator, denominator, default=0.0):
        result = np.full(numerator.shape, default)
        result[denominator > 0] = numerator[denominator > 0] / denominator[denominator > 0]
        return result

    def load_pre_readout_signal_z_distribution(self, filepath):
        with uproot.open(filepath) as sig_f:
            return sig_f['mcAna/mcAna_mc625Z_h'].to_hist()

    def load_signal(self, filepath, pre_readout_filepath, mass_vd, selection, cut_expression=None, branches=None):
        with uproot.open(filepath) as sig_f:
            if branches:
                events = sig_f[f'{selection}/{selection}_tree'].arrays(
                    expressions=branches,
                    cut=cut_expression
                )
            else:
                events = sig_f[f'{selection}/{selection}_tree'].arrays(
                    expressions=[
                        'vd_true_vtx_z', 'unc_vtx_z', 'unc_vtx_ele_track_z0',
                        'unc_vtx_pos_track_z0', 'unc_vtx_mass', 'unc_vtx_deltaZ',
                        'unc_vtx_proj_sig', 'vd_true_vtx_energy', 'unc_vtx_psum'
                    ],
                    cut=cut_expression
                )
        events['vd_true_gamma'] = events.vd_true_vtx_energy * 1000 / mass_vd
        events['unc_vtx_min_z0'] = np.minimum(abs(events.unc_vtx_ele_track_z0), abs(events.unc_vtx_pos_track_z0))

        not_rebinned_pre_readout_z_h = self.load_pre_readout_signal_z_distribution(pre_readout_filepath)

        def sample_pre_readout_probability(z):
            if z < not_rebinned_pre_readout_z_h.axes[0].edges[0]:
                return 0.
            if z > not_rebinned_pre_readout_z_h.axes[0].edges[-1]:
                return 0.
            index = not_rebinned_pre_readout_z_h.axes.index(z)
            return not_rebinned_pre_readout_z_h.axes.widths[0][index] / not_rebinned_pre_readout_z_h[index].value

        events['event_weight_by_uniform_z'] = [
            sample_pre_readout_probability(z)
            for z in events.vd_true_vtx_z
        ]
        return events

    def _load_trident_differential_production_lut(self, background_file, selection, signal_mass_range, mass_window_width, tenpct=True, full_lumi_path=None):
        dNdm_by_mass_vd = {}
        bkgd_CR = ak.Array([])

        if tenpct:
            with uproot.open(background_file) as bkgd_f:
                bkgd_CR = bkgd_f[f'{selection}/{selection}_tree'].arrays(
                    cut=f'( (unc_vtx_psum > {self.cr_psum_low}) & (unc_vtx_psum < {self.cr_psum_high}) )',
                    expressions=['unc_vtx_mass', 'unc_vtx_z'],
                )
        else:
            for filename in sorted(os.listdir(full_lumi_path)):
                if not filename.endswith('.root'):
                    continue
                run = filename.split('_')[4]
                print('Loading Run ', run)


                background_file = os.path.join(full_lumi_path,filename)
                with uproot.open(background_file) as bkgd_f:
                    bkgd_CR_per_run = bkgd_f[f'{selection}/{selection}_tree'].arrays(
                        cut=f'( (unc_vtx_psum > {self.cr_psum_low}) & (unc_vtx_psum < {self.cr_psum_high}) )',
                        expressions=['unc_vtx_mass', 'unc_vtx_z'],
                    )
                    bkgd_CR = ak.concatenate([bkgd_CR, bkgd_CR_per_run])

        for mass_vd in signal_mass_range:
            window_half_width = mass_window_width * self.mass_resolution(mass_vd) / 2.
            dNdm_by_mass_vd[mass_vd] = ak.sum(
                (bkgd_CR.unc_vtx_mass * 1000 > self.mass_ratio_ap_to_vd * (mass_vd - window_half_width)) &
                (bkgd_CR.unc_vtx_mass * 1000 < self.mass_ratio_ap_to_vd * (mass_vd + window_half_width))
            ) / (2 * window_half_width * self.mass_ratio_ap_to_vd)
        return dNdm_by_mass_vd

    def trident_differential_production(self, mass_vd):
        if int(mass_vd) in self.trident_differential_production.keys():
            return self.trident_differential_production[mass_vd]
        raise ValueError(f'The dark vector mass {mass_vd} is not found in the trident differential production look-up table.')

    #Use the reconstructed data in the high psum region to scale the differential radiative trident production rate
    #This scales the A' production rate, therefore the expected signal
    def set_diff_prod_lut(self,infile, preselection, signal_mass_range, tenpct=True, full_lumi_path=None):
        #Initialize the lookup table to calculate the expected signal scale factor
        self.trident_differential_production = self._load_trident_differential_production_lut(infile, preselection, signal_mass_range, 2.0*self.nsigma, tenpct=tenpct, full_lumi_path=full_lumi_path)

    def total_signal_production_per_epsilon2(self, signal_mass):
        mass_ap = self.mass_ratio_ap_to_vd*signal_mass
        return (
            (3. * (137. / 2.) * np.pi)
            * mass_ap * self.radiative_fraction(mass_ap)
            * self.trident_differential_production[(int((mass_ap / self.mass_ratio_ap_to_vd)))]
            / self.radiative_acceptance(mass_ap)
        )

    def get_exp_sig_eps2(self, signal_mass, signal_array, eps2):

        #Define simp masses
        mass_ap = self.mass_ratio_ap_to_vd*signal_mass
        mass_pid = mass_ap / self.mass_ratio_ap_to_pid
        fpid = mass_pid / self.mpifpi

        rho_gctau = signal_array.vd_true_gamma * simpeqs.getCtau(mass_ap, mass_pid, signal_mass, np.sqrt(eps2), self.alpha_dark, fpid, self.mass_lepton, True)
        phi_gctau = signal_array.vd_true_gamma * simpeqs.getCtau(mass_ap, mass_pid, signal_mass, np.sqrt(eps2), self.alpha_dark, fpid, self.mass_lepton, False)

        rho_decay_weight = (np.exp((self.target_pos - signal_array.vd_true_vtx_z) / rho_gctau) / rho_gctau)
        phi_decay_weight = (np.exp((self.target_pos - signal_array.vd_true_vtx_z) / phi_gctau) / phi_gctau)
        
        combined_decay_weight = (
            rho_decay_weight * simpeqs.br_Vrho_pi(mass_ap, mass_pid, signal_mass, self.alpha_dark, fpid)
            + phi_decay_weight * simpeqs.br_Vphi_pi(mass_ap, mass_pid, signal_mass, self.alpha_dark, fpid)
        )
        # the weight for a single event is the chance of that decay (z and gamma from either Vd)
        # multiplied by probability the event was from that z-bin in the original sample
        signal_array['reweighted_accxEff'] = combined_decay_weight*signal_array.event_weight_by_uniform_z

        return signal_array

    @staticmethod
    def get_minz0_cut():
        coeffs = [1.07620094e+00 + 0.1, -7.44533811e-03, 1.58745903e-05]
        return coeffs
    
    def minz0_sel(self,array):
        coeffs = self.get_minz0_cut()
        p0 = coeffs[0]
        p1 = coeffs[1]
        p2 = coeffs[2]
        sel = (
            ( array.unc_vtx_min_z0 > (p0 + p1*array.unc_vtx_mass*1000 + (p2*np.square(array.unc_vtx_mass*1000.))) )
        )
        return sel

    def mass_sel(self,array, signal_mass):
        mass_low = signal_mass - self.nsigma*self.mass_resolution(signal_mass)
        mass_high = signal_mass + self.nsigma*self.mass_resolution(signal_mass)
        sel = (
            ( array.unc_vtx_mass*1000. >= {mass_low}) & (array.unc_vtx_mass*1000. <= {mass_high}) 
        )
        return sel

    @staticmethod
    def psum_sel(array, case='sr'):
        if case == 'sr':
            sel = (
                (array.unc_vtx_psum > 1.0) & (array.unc_vtx_psum < 1.9)
            )
        elif case == 'cr':
            sel = (
                (array.unc_vtx_psum > 1.9) & (array.unc_vtx_psum < 2.4)
            )
        else:
            sel = ()
        return sel

    @staticmethod
    def vprojsig_sel(array):
        sel = (
            (array.unc_vtx_proj_sig < 2)
        )
        return sel

    @staticmethod
    def sameside_z0_cut(array):
        sel = (
            (-1.*(array.unc_vtx_ele_track_z0*array.unc_vtx_pos_track_z0) > 0)
        )
        return sel
    
    @staticmethod
    def zcut_sel(array):
        sel = (
            (array.unc_vtx_z > -4.3)
        )
        return sel


    def tight_selection(self, array, signal_mass, case=1):
        coeffs = self.get_minz0_cut()
        p0 = coeffs[0]
        p1 = coeffs[1]
        p2 = coeffs[2]
        mass_low = signal_mass - self.nsigma*self.mass_resolution(signal_mass)
        mass_high = signal_mass + self.nsigma*self.mass_resolution(signal_mass)

        if case == 1: #full tight analysis selection
            sel = (
                ( array.unc_vtx_min_z0 > (p0 + p1*array.unc_vtx_mass*1000 + (p2*np.square(array.unc_vtx_mass*1000.))) ) &
                ( array.unc_vtx_mass*1000. >= {mass_low}) & (array.unc_vtx_mass*1000. <= {mass_high}) &
                (array.unc_vtx_proj_sig < 2) & (array.unc_vtx_z > -4.3) & (array.unc_vtx_psum > 1.0) & (array.unc_vtx_psum < 1.9)
            )
        if case == 2: #tight selection without minz0 cut
            sel = (
                ( array.unc_vtx_mass*1000. >= {mass_low}) & (array.unc_vtx_mass*1000. <= {mass_high}) &
                (array.unc_vtx_proj_sig < 2) & (array.unc_vtx_z > -4.3) & (array.unc_vtx_psum > 1.0) & (array.unc_vtx_psum < 1.9)
            )

        if case == 3: #tight selection without mass and minz0 cut
            sel = (
                (array.unc_vtx_proj_sig < 2) & (array.unc_vtx_z > -4.3) & (array.unc_vtx_psum > 1.0) & (array.unc_vtx_psum < 1.9)
            )

        return sel

    @staticmethod
    def readROOTHisto(infilename, histoname):
        infile = r.TFile(f'{infilename}',"READ")
        histo = copy.deepcopy(infile.Get(f'{histoname}'))
        infile.Close()
        return histo


    @staticmethod
    def cnvHistosToROOT(histos=[], tempname='temporary_uproot'):
        return_histos = []
        uproot_file = uproot.recreate(f'trash_{tempname}.root')
        for i, histo in enumerate(histos):
            uproot_file[f'histo_{i}'] = histo
        uproot_file.close()
        infile = r.TFile(f'trash_{tempname}.root',"READ")
        for i, histo in enumerate(histos):
            return_histos.append(copy.deepcopy(infile.Get(f'histo_{i}')))
        infile.Close()
        return return_histos

    @staticmethod
    def cnvHistoToROOT(histo, tempname='temporary_uproot'):
        uproot_file = uproot.recreate(f'trash_{tempname}.root')
        uproot_file['histogram'] = histo
        uproot_file.close()
        infile = r.TFile(f'trash_{tempname}.root',"READ")
        root_hist = (copy.deepcopy(infile.Get(f'histogram')))
        infile.Close()
        return root_hist


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Process some inputs.')
    parser.add_argument('--outfilename', type=str, default='expected_signal_output.root')
    parser.add_argument('--mpifpi', type=float, default=4*np.pi)
    parser.add_argument('--signal_sf', type=float, default=1.0)
    parser.add_argument('--nsigma', type=float, default=1.5)
    parser.add_argument('--tenpct', type=int, default=0)
    args = parser.parse_args()

    mpifpi = args.mpifpi
    nsigma = args.nsigma
    signal_sf = args.signal_sf
    outfilename = args.outfilename
    tenpct = args.tenpct



    #Create MC signal analysis tuple processor
    print('Initialize signal processor')
    processor = SignalProcessor(mpifpi=mpifpi, nsigma=nsigma)

    #Set the differential radiative trident rate lookup table used to scale expected signal
    print('Load lookup table')
    cr_data = '/sdf/group/hps/user-data/alspellm/2016/data/hadd_BLPass4c_1959files.root'
    full_lumi_path = '/fs/ddn/sdf/group/hps/users/alspellm/data_storage/pass4kf/pass4kf_ana_20240513'
    preselection = "vtxana_Tight_nocuts"
    signal_mass_range = [x for x in range(30,130,1)]
    processor.set_diff_prod_lut(cr_data, preselection, signal_mass_range, tenpct, full_lumi_path)

    #Initialize the range of epsilon2
    masses = [x for x in range(30,124,2)]
    masses = [x for x in range(50,70,2)]
    ap_masses = [round(x*processor.mass_ratio_ap_to_vd,1) for x in masses]
    eps2_range = np.logspace(-4.0,-8.0,num=40)
    logeps2_range = np.log10(eps2_range)
    min_eps = min(np.log10(eps2_range))
    max_eps = max(np.log10(eps2_range))
    num_bins = len(eps2_range)

    #Define all histograms
    expected_signal_vd_h = (
        hist.Hist.new
        .Reg(len(masses), np.min(masses), np.max(masses), label='Vd Invariant Mass [MeV]')
        .Reg(num_bins, min_eps, max_eps,label=f'$log_{10}(\epsilon^2)$')
        .Double()
    )
    expected_signal_ap_h = (
        hist.Hist.new
        .Reg(len(ap_masses), np.min(ap_masses), np.max(ap_masses), label='A\' Invariant Mass [MeV]')
        .Reg(num_bins, min_eps, max_eps,label=f'$log_{10}(\epsilon^2)$')
        .Double()
    )


    for signal_mass in masses:

        #Load MC Signal
        indir = '/sdf/group/hps/user-data/alspellm/2016/simp_mc/pass4b/beam/smeared'
        signal_pre_readout_path = lambda mass: f'/sdf/group/hps/user-data/alspellm/2016/simp_mc/pass4b/nobeam/mass_{mass}_simp_2pt3_slic_hadd_ana.root'
        signal_path = lambda mass: f'{indir}/mass_{mass}_hadd-simp-beam_ana_smeared_corr.root'
        signal_selection = 'vtxana_radMatchTight_2016_simp_SR_analysis'

        #Get the total signal yield as a function of eps2
        total_yield_per_epsilon2 = processor.total_signal_production_per_epsilon2(signal_mass)
        print('Total Yield Per eps2: ', total_yield_per_epsilon2)

        print('Load Signal ', signal_path(signal_mass))
        signal = processor.load_signal(signal_path(signal_mass), signal_pre_readout_path(signal_mass), signal_mass, signal_selection)

        #Get Tight selection
        psum_sel = processor.psum_sel(signal, case='sr')
        zcut_sel = processor.zcut_sel(signal)
        vprojsig_sel = processor.vprojsig_sel(signal)
        minz0_sel = processor.minz0_sel(signal)
        masswindow_sel = processor.mass_sel(signal, signal_mass)
        sameside_sel = processor.sameside_z0_cut(signal)
        tight_sel = np.logical_and.reduce([psum_sel,zcut_sel, vprojsig_sel, psum_sel, minz0_sel, masswindow_sel, sameside_sel])
        #tight_sel = processor.tight_selection(signal, signal_mass)

        for l, eps2 in enumerate(eps2_range):
            signal = processor.get_exp_sig_eps2(signal_mass, signal, eps2)
            total_yield = signal_sf*ak.sum(signal['reweighted_accxEff'][tight_sel])*total_yield_per_epsilon2*eps2
            #print('Total Yield: ', total_yield)
            expected_signal_vd_h.fill(signal_mass, logeps2_range[l], weight=total_yield)
            expected_signal_ap_h.fill(signal_mass*processor.mass_ratio_ap_to_vd, logeps2_range[l], weight=total_yield)

    outfile = uproot.recreate(outfilename)
    outfile['expected_signal_vd_h'] = expected_signal_vd_h
    outfile['expected_signal_ap_h'] = expected_signal_ap_h










    




