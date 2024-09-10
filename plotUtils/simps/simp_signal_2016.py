#!/usr/bin/python3
#=======================================================================================================================================
"""
SignalProcessor Class
---------------------
This script defines the 'SignalProcessor' class, which handles the expected signal calculations used in the 2016 SIMP L1L1 analysis.
This script also imports the necessary simp equations to perform the calculations.
This processor operates on the output of the hpstr vertex analysis processor, which is a flat tuple of events.

Modules:
    - os
    - awkward as ak
    - numpy as np
    - hist
    - uproot
    - ROOT as r
    - argparse
    - simp_theory_equations (imported as SimpEquations)
"""

import os
import awkward as ak
import numpy as np
import hist
from hist import Hist
import uproot
import math
import ROOT as r
import copy
import argparse
from simp_theory_equations import SimpEquations as simpeqs
#=======================================================================================================================================

class SignalProcessor:
    """
    A class for handling signal data processing and physics parameter calculations from SIMPs.

    Attributes:
        nsigma (float): Size of the invariant mass search window in terms of the mass resolution (default is 1.5).
        alpha_dark (float): The dark sector fine structure constant.
        mass_ratio_ap_to_vd (float): Ratio of dark photon mass to dark vector meson mass.
        mass_ratio_ap_to_pid (float): Ratio of dark photon mass to pion mass.
        mass_lepton (float): ele mass (in MeV).
        target_pos (float): Target position in mm (default is -4.3 mm)
        mpifpi (float): ratio of dark pion mass and dark pion decay constant, default value i 4*pi
        mass_resolution (function): Polynomial function representing mass resolution.
        radiative_fraction (function): Polynomial function for radiative fraction.
        radiative_acceptance (function): Polynomial function for radiative acceptance.
        psum_reweighting (function): Polynomial function for reweighting based on MC bkg and data Psum.
        minz0_cut_poly (function): Polynomial function for minimum z0 cut.
        cr_psum_low (float): Lower bound for CR psum.
        cr_psum_high (float): Upper bound for CR psum.
        sr_psum_low (float): Lower bound for SR psum.
        sr_psum_high (float): Upper bound for SR psum.
        trident_differential_production: Holds the differential trident production rate
    """

    def __init__(self, mpifpi=4*np.pi, nsigma=1.5):
        """
        Initializes the SignalProcessor with default SIMP model parameters and physics constants.

        Args:
            mpifpi (float, optional): benchmark values are 3 and 4pi.
            nsigma (float, optional): Size of the invariant mass search window in terms of the mass resolution (default is 1.5).
        """

        #search window size
        self.nsigma = nsigma 

        #SIMP parameters
        self.alpha_dark = 0.01
        self.mass_ratio_ap_to_vd = 1.66
        self.mass_ratio_ap_to_pid = 3.0
        self.mass_lepton = 0.511
        self.target_pos = -4.3
        self.mpifpi = mpifpi

        #fit functions calculated externally
        self.mass_resolution = self.polynomial(.75739851, 0.031621002, 5.2949672e-05)
        self.radiative_fraction = self.polynomial(0.10541434, -0.0011737697, 7.4487930e-06, -1.6766332e-08)
        self.radiative_acceptance = self.polynomial(-0.48922505, 0.073733061, -0.0043873158, 0.00013455495, -2.3630535e-06, 2.5402516e-08, -1.7090900e-10, 7.0355585e-13, -1.6215982e-15, 1.6032317e-18)
        self.psum_reweighting = self.polynomial(0.094272950, 0.87334446, -0.19641796) #GeV argument
        self.minz0_cut_poly = self.polynomial(1.07620094e+00 + 0.1, -7.44533811e-03, 1.58745903e-05)

        #signal and control region boundaries
        self.cr_psum_low = 1.9
        self.cr_psum_high = 2.4
        self.sr_psum_low = 1.0
        self.sr_psum_high = 1.9

        #scales the expected signal to data
        self.trident_differential_production = None


    def set_radiative_acceptance(self, *coefficients):
        """
        Sets the polynomial coefficients for the radiative acceptance function.
        Used for systematic studies where the radiative acceptance changes from nominal. 

        Args:
            *coefficients (float): Coefficients of the polynomial function.
        """
        self.radiative_acceptance = self.polynomial(*coefficients)

    @staticmethod
    def polynomial(*coefficients):
        def _implementation(x):
            return sum([
                coefficient * x**power
                for power, coefficient in enumerate(coefficients)
            ])
        return _implementation

    def load_data(self, filepath, selection, cut_expression=None, expressions=None):
        """
        Loads data from hpstr vertex ana processor output file using Uproot, applying selection and cuts if provided.

        Args:
            filepath (str): Path to the ROOT file.
            selection (str): The dataset selection (subdir representing hpstr lvl selection)
            cut_expression (str, optional): A cut expression to filter data.
            expressions (list, optional): List of specific expressions (branches) to load.

        Returns:
            awkward.Array: An array of selected data.
        """
        with uproot.open(filepath) as f:
            events = f[f'{selection}/{selection}_tree'].arrays(
                cut=cut_expression,
                expressions=expressions
            )
            try:
                events['unc_vtx_min_z0'] = np.minimum(abs(events.unc_vtx_ele_track_z0), abs(events.unc_vtx_pos_track_z0))
            except:
                pass
            return events

    @staticmethod
    def safe_divide(numerator, denominator, default=0.0):
        result = np.full(numerator.shape, default)
        result[denominator > 0] = numerator[denominator > 0] / denominator[denominator > 0]
        return result

    def load_pre_readout_signal_z_distribution(self, filepath):
        """
        Loads MC signal vertex z distribution output from the hpstr mcana processor

        Args:
            filepath (str): Path to ROOT file.
        """
        with uproot.open(filepath) as sig_f:
            return sig_f['mcAna/mcAna_mc625Z_h'].to_hist()

    def load_signal(self, filepath, pre_readout_filepath, mass_vd, selection, cut_expression=None, branches=None):
        """
        Loads MC signal from hpstr vertex ana processor output.
        
        Args:
            filepath (str): Path to ROOT file.
            pre_readout_filepath (str): mc ana file corresponding to signal mass.
            mass_vd (float): dark vector mass.
            selection (str): subdir where ttree is located in ROOT file.
            cut_expression (str, optional): A cut expression to filter data (Ex: "unc_vtx_psum < 1.9")
            branches (str, optional): List of branches to load (default ones necessary for analysis)

        """
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
        events['psum_reweight'] = self.psum_reweighting(events.unc_vtx_psum)
        events['psum_reweight'] = ak.where(events['psum_reweight'] > 1., 1., events['psum_reweight'])

        #load the mc truth signal vertex distribution. Used to calculate signal acceptance*eff as F(z)
        not_rebinned_pre_readout_z_h = self.load_pre_readout_signal_z_distribution(pre_readout_filepath)

        def sample_pre_readout_probability(z):
            """
            Calculates the signal acceptance*efficiency as a function of F(z)

            Args:
                z (float): truth vertex z bin.
            """
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
        """
        Loads lookup table that stores the differential radiative trident rate as a function of A' mass.
        This rate scales the expected signal to the data set.

        Args:
            background_file (str): hpstr vertex ana ROOT file containing reconstructed+selected background events in CR. 
            selection (str): subdir where ttree is located in ROOT file.
            signal_mass_range (tuple): range of dark vector masses that expected signal will be calculated for.
            mass_window_width (float): width of the mass window used to calculate the reconstructed bkg rate (it gets averaged)
            tenpct (bool, optional): If True, use 10% data to normalize signal. If False, use 100% data (must provide full lumi path).
            full_lumi_path (str, optional): If tenpct=False, provide path to 100% data. 

        Returns:
            The look-up table for radiative trident differential production rates as function of A' mass.

        """
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
        """
        Returns the radiative trident differential production rate.

        Args:
            mass_vd (float): dark vector mass

        Notes: 
            If the dark vector mass isn't found, could be the result of the mvd -> map conversion.
        """
        if int(mass_vd) in self.trident_differential_production.keys():
            return self.trident_differential_production[mass_vd]
        raise ValueError(f'The dark vector mass {mass_vd} is not found in the trident differential production look-up table.')

    def set_diff_prod_lut(self,infile, preselection, signal_mass_range, tenpct=True, full_lumi_path=None):
        """
        Initializes the trident differential production lookup table.

        Args:
            infile (str): hpstr vertex ana ROOT file containing reconstructed+selected background events in CR. 
            preselection (str): ROOT file subdir containing preselection ttree.
            signal_mass_range (tuple): dark vector meson mass range
            tenpct (bool, optional): If True use 10% data sample. If False, use 100% data (requires full_lumi_path).
            full_lumi_path (str, optional): If tenpct == False, provide path to 100% data.

        Notes:
            This was all developed using 10% data stored in a single ROOT file, hence the tenpct option. But you can pass any 
            single ROOT file with tenpct set to True. 
            The final analysis uses 100% data, which consists of multiple separate root files located at full_lumi_path.
        """

        #Initialize the lookup table to calculate the expected signal scale factor
        self.trident_differential_production = self._load_trident_differential_production_lut(infile, preselection, signal_mass_range, 2.0*self.nsigma, tenpct=tenpct, full_lumi_path=full_lumi_path)

    def total_signal_production_per_epsilon2(self, signal_mass):
        """
        Calculates the total dark vector meson production rate as a function of epsilon^2

        Args:
            signal_mass (float): dark vector meson mass.

        Notes:
            Notice that you pass the dark vector meson mass, and it gets converted to an A' mass.
            The radiative fraction, acceptance, and trident prod are all calculated using the A' mass, NOT the vector mass!
            The dark vector to A' mass ratio is set upon initializing the instance of this class. 
        """
        mass_ap = self.mass_ratio_ap_to_vd*signal_mass
        return (
            (3. * (137. / 2.) * np.pi)
            * mass_ap * self.radiative_fraction(mass_ap)
            * self.trident_differential_production[(int((mass_ap / self.mass_ratio_ap_to_vd)))]
            / self.radiative_acceptance(mass_ap)
        )

    def get_exp_sig_eps2(self, signal_mass, signal_array, eps2):
        """
        Calculates the neutral dark vector meson signal acceptance*efficiency*probability for each MC signal event for value eps^2.
        This function reweights the MC signal events by considering the dark vector (rho and phi) decay probabilities.

        Args:
            signal_mass (float): The generated dark vector meson mass.
            signal_array (awkward array): MC signal array loaded by load_signal. 
            eps2 (float): The square of the kineitc mixing strength parameter (affects decay probabilities).

        Returns:
            signal_array (awkward array): The input signal array with added columns for the reweighted signal 
            acceptance*efficiency*probability ('reweighted_accxEff'). 

        Notes:
            The vector decay probabilities depend on the A' mass, dark pion mass, and mpi/fpi, all of which are initialized
            with the instance of this class. 

        """

        #Define SIMP masses based on mass ratios and constants 
        mass_ap = self.mass_ratio_ap_to_vd*signal_mass # A' mass from Vd mass
        mass_pid = mass_ap / self.mass_ratio_ap_to_pid # Dark pion mass from A' mass
        fpid = mass_pid / self.mpifpi # Dark pion decay constant from ratio of dark pion mass to dark pion decay constant

        #Calculate the decay length in the lab frame for rho and phi
        rho_gctau = signal_array.vd_true_gamma * simpeqs.getCtau(mass_ap, mass_pid, signal_mass, np.sqrt(eps2), self.alpha_dark, fpid, self.mass_lepton, True)
        phi_gctau = signal_array.vd_true_gamma * simpeqs.getCtau(mass_ap, mass_pid, signal_mass, np.sqrt(eps2), self.alpha_dark, fpid, self.mass_lepton, False)

        #Calculate the decay weights for rho and phi based on the vertex z position and gammactau
        rho_decay_weight = (np.exp((self.target_pos - signal_array.vd_true_vtx_z) / rho_gctau) / rho_gctau)
        phi_decay_weight = (np.exp((self.target_pos - signal_array.vd_true_vtx_z) / phi_gctau) / phi_gctau)
        #signal_array['reweighted_accxEff_rho'] = simpeqs.br_Vrho_pi(mass_ap, mass_pid, signal_mass, self.alpha_dark, fpid)*rho_decay_weight*signal_array.event_weight_by_uniform_z*signal_array.psum_reweight
        #signal_array['reweighted_accxEff_phi'] = simpeqs.br_Vphi_pi(mass_ap, mass_pid, signal_mass, self.alpha_dark, fpid)*phi_decay_weight*signal_array.event_weight_by_uniform_z*signal_array.psum_reweight
        
        #Calculate the combined decay weight for both rho and phi mesons.
        #This result represenets the overall expected signal decay. 
        combined_decay_weight = (
            (rho_decay_weight * simpeqs.br_Vrho_pi(mass_ap, mass_pid, signal_mass, self.alpha_dark, fpid))
            + (phi_decay_weight * simpeqs.br_Vphi_pi(mass_ap, mass_pid, signal_mass, self.alpha_dark, fpid))
        )
        #The final reweighting includes both rho and phi, and includes the signal acceptance*efficiency as a function of z via 
        #signal_array.event_weight_by_uniform_z. Psum re-weighting (calculated externally, saved in this classs) is also included.
        signal_array['reweighted_accxEff'] = combined_decay_weight*signal_array.event_weight_by_uniform_z*signal_array.psum_reweight

        return signal_array

    @staticmethod
    def get_minz0_cut():
        """
        Defines minimum z0 cut polynomial coefficients. 
        These coefficients were determined by optimizing this cut using ZBi as a function of MC signal mass.
        The optimized cut values as a function of mass were then fit with a polynomial.
        The cut shape was then tightened by 0.1 mm.

        Returns:
            coeffs (list): minimum z0 cut function coefficients.


        """
        coeffs = [1.07620094e+00 + 0.1, -7.44533811e-03, 1.58745903e-05]
        return coeffs
    
    def minz0_sel(self,array):
        """
        Applies the minimum z0 cut. 

        Args:
            array (awkward array): data array.

        Returns:
            sel (awkward array): Boolean array representing events passing the minimum z0 cut. 
        """
        # Retrieve coefficients for the minimum z0 cut
        coeffs = self.get_minz0_cut()
        p0 = coeffs[0]
        p1 = coeffs[1]
        p2 = coeffs[2]

        # Get boolean mask of events that pass
        sel = (
            ( array.unc_vtx_min_z0 > (p0 + p1*array.unc_vtx_mass*1000 + (p2*np.square(array.unc_vtx_mass*1000.))) )
        )
        return sel

    def mass_sel(self,array, signal_mass):
        """
        Applies a mass window cut around the signal mass.
        The width of the window is determined by the mass resolution of the number of sigma.

        Args:
            array (awkward array): Data array.
            signal_mass (float): The dark vector meson search window mass center.

        Returns:
            sel (awkward array): Boolean array representing events inside the mass window.
        """
        # Calcualte the lower and upper bounds of the mass search window based on the mass resolution
        mass_low = signal_mass - self.nsigma*self.mass_resolution(signal_mass)
        mass_high = signal_mass + self.nsigma*self.mass_resolution(signal_mass)

        # Get the boolean mask of events that pass
        sel = (
            ( array.unc_vtx_mass*1000. >= {mass_low}) & (array.unc_vtx_mass*1000. <= {mass_high}) 
        )
        return sel

    @staticmethod
    def psum_sel(array, case='sr'):
        """
        Applies the Psum selection (signal region or control region).

        Args:
            array (awkward array): Data array.
            case (str, optional): Specify signal region ('sr') or control region ('cr').

        Returns:
            sel (awkward array): Boolean array representing events in Psum region.
        """

        if case == 'sr':
            # Select momentum sum for signal region (between 1.0 and 1.9 GeV)
            sel = (
                (array.unc_vtx_psum > 1.0) & (array.unc_vtx_psum < 1.9)
            )
        elif case == 'cr':
            # Select momentum sum for control region (between 1.9 and 2.4 GeV)
            sel = (
                (array.unc_vtx_psum > 1.9) & (array.unc_vtx_psum < 2.4)
            )
        else:
            # No selection if invalid case
            sel = ()
        return sel

    @staticmethod
    def vprojsig_sel(array):
        """
        Applies cut on the target projected vertex significance. 

        Args:
            array (awkward array): Data array. 

        Returns:
            sel (awkward array): Boolean array representing events that pass this cut.
        """
        # Select events with v0projsig < 2.0
        sel = (
            (array.unc_vtx_proj_sig < 2)
        )
        return sel

    @staticmethod
    def sameside_z0_cut(array):
        """
        Applies a cut to remove events where both tracks have z0 (aka y0) with the same sign.
        This cut doesn't seem to do much after the final cut, but could be useful earlier on in the analysis.

        Args:
            array (awkward array): Data array.

        Returns:
            sel (awkward array): Boolean array representing events that pass this cut.
        """
        sel = (
            (-1.*(array.unc_vtx_ele_track_z0*array.unc_vtx_pos_track_z0) > 0)
        )
        return sel
    
    @staticmethod
    def zcut_sel(array):
        """
        Applies a cut on the reconstructed z vertex position.

        Args:
            array (awkward array): Data array.

        Returns:
            sel (awkward array): Boolean array representing events passing this cut.
        """

        # Select events where the reconstructed vertex z position is greater than -4.3 mm (target location)
        sel = (
            (array.unc_vtx_z > -4.8)
        )
        return sel


    #Combine all of the selections into one function if you like. 
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
        """
        Quickly read a ROOT histogram from a root file, and make a deep copy.

        Args:
            infilename (str): Input ROOT file.
            histoname (str): Name of histogram being loaded. 

        Returns:
            histo (ROOT obj): Graph or histogram. 
        """
        #Open ROOT file
        infile = r.TFile(f'{infilename}',"READ")
        #Make copy of histogram
        histo = copy.deepcopy(infile.Get(f'{histoname}'))
        infile.Close()
        return histo


    @staticmethod
    def cnvHistosToROOT(histos=[], tempname='temporary_uproot'):
        """
        Converts hist histograms to ROOT histograms. Convenient because ROOT automatically calculates errors.
        Also nice if you want to present result using ROOT histogram formatting. 

        Args:
            histos (list[Hist]): List of hist histograms.
            tempname (str): Name of temporary ROOT file. Necessary to save Hist using uproot, and then read ROOT out.

        Returns:
            return_histos (list[ROOT histograms]): ROOT histograms.
        """
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

    @staticmethod
    def cnv_root_to_np(histo):
        """
        Extract values from ROOT histogram to enable analysis or plotting with other tools. 

        Args:
            hist (TH1): Input ROOT histogram.

        """
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

    @staticmethod
    def cnv_tgraph_to_np(tgraph):
        """
        Extract values from ROOT Tgraph.

        Args:
            tgraph (TGraph): Input ROOT Tgraph
        """
        # Number of points in the TGraph
        npoints = tgraph.GetN()
        
        # Retrieve X and Y values
        xvals = np.array([tgraph.GetX()[i] for i in range(npoints)])
        yvals = np.array([tgraph.GetY()[i] for i in range(npoints)])
        
        #Errors not available in standard TGraph, set to zero.
        errors = np.zeros(npoints)
        
        # Handle fit function if it exists
        x_fit = None
        y_fit = None
        if len(tgraph.GetListOfFunctions()) > 0:
            fitfunc = tgraph.GetListOfFunctions()[0]
            x_fit = np.linspace(fitfunc.GetXmin(), fitfunc.GetXmax(), 100)  # 100 points for the fit
            y_fit = np.array([fitfunc.Eval(x) for x in x_fit])
        
        return (xvals, yvals, errors), (x_fit, y_fit)


    @staticmethod
    def fit_plot_with_poly(plot, tgraph=False, specify_n=None, set_xrange=False, xrange=(0.0, 1.0)):
        """
        Fit TH1 or TGraph with polynomial fit function. Uses fstat test to guide choice of polynomial degree.

        Args:
            plot (TH1 or TGraph): Input plot to be fitted.
            tgraph (boolean, optional): If True, access npoints through TGraph method.
            specify_n (int, optional): If None, fit plot using all n degrees, calculate and print fstat for each n.
                                       If int, fit plot using n degree polynomial.
            set_xrange (boolean, optional): If True, set fit range according to xrange.
            xrange (float, float, optional): Define fit range.  

        Returns:
            params (list): List of fit function parameters.
            errors (list): List of fit parameter errors.

        Notes:
            If trying to determine what order polynomial to fit plot, specify_n=None. Function will print out fstat results.
            Once best order determined, set specifcy_n=<n> to get fit resulst.
        """
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
            for n in range(11):
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

                #Perform fstat test to see how much fit improves with additional order
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
            params = fit_result.Parameters()
            errors = fit_result.Errors()
            #return fit_result
            return params, errors



    def systematic_uncertainties(self):
        """
        This method contains all of the systematic uncertainties applied in Alic's 2016 SIMP L1L1 dissertation. 
        The detector misalignment systematic is not included as of 09/06/2024.
        These functions and values were calculated externally using a +- 1.5 sigma search window, with cuts frozen per dissertation.
        All polynomials are a function of A' mass. 
        """
        self.radacc_targetz_nominal = self.polynomial(0.24083419, -0.017612076, 0.00037553660, -1.0223921e-06, -3.8793240e-08,
                4.2199609e-10, -1.6641414e-12, 2.3433278e-15)
        self.radacc_targetz_Mpt5 = self.polynomial(0.22477846, -0.015984559, 0.00030943435, 3.6182165e-07, -5.4820194e-08,
                5.2531952e-10, -2.0102027e-12, 2.8109430e-15)
        self.radacc_targetz_Ppt5 = self.polynomial( 0.22779999, -0.016020742, 0.00029960205, 7.6823260e-07, -6.0956281e-08,
                5.6914810e-10, -2.1602258e-12, 3.0094146e-15)
        self.simp_targetz = self.polynomial(-1.38077250e+00, 8.00749424e-02, -9.78327706e-04, 5.13294008e-06, -9.77393492e-09)
        self.mass_unc = 0.043
        self.radfrac = 0.07

    def evaluate_polynomials(self, mass):
        """
        Returns the systematic uncertainties defined in systematic_uncertainties() as a function of mass.

        Args:
            mass (float): A' mass.

        Note:
            You need to understand each systematic in order to correctly combine these numbers. 
        """
        nominal_values = self.radacc_targetz_nominal(mass)
        Mpt5_values = self.radacc_targetz_Mpt5(mass)
        Ppt5_values = self.radacc_targetz_Ppt5(mass)
        simp_values = self.simp_targetz(mass)

        return nominal_values, Mpt5_values, Ppt5_values, simp_values, self.mass_unc, self.radfrac

    @staticmethod
    def inject_signal_mc(signal, data, nevents=100):
        """
        Randomly selects MC signal events and injects them into data array.

        Args:
            signal (awkward array): MC signal array loaded using load_signal().
            data (awwkard array): Data array loaded using load_data().
            nevents (int): Specify number of MC signal events to inject into data.

        Returns:
            injected_data (awkward array): Copy of data array with injected signal.
            thrown_events (int): Number of MC signal events that were thrown. Useful sanity check. 
        """
        # Identify the maximum signal event weight in the MC signal array
        max_weight = np.max(signal.expected_signal_weight)

        # Randomly sample the MC signal array until the specified number of events is thrown
        events_thrown = 0
        thrown_mask = []
        while events_thrown < nevents:

            # Randomly select a signal event from the array
            rint = np.random.randint(0,len(signal.expected_signal_weight)-1)
            random_event = signal[rint]

            # Randomly sample the uniform distribution between 0-maximum signal event weight
            # If the uniform sample weight is less than the randomly selected event weight, thrown the event
            rweight = np.random.uniform(0, max_weight)
            if rweight < random_event.expected_signal_weight:
                events_thrown += 1
                thrown_mask.append(rint)
        thrown_events = signal[thrown_mask]
        thrown_events['weight'] = 1.0

        # Inject the randomly selected signal events into the data by combining the awkward arrays
        injected_data = ak.concatenate([data, thrown_events])
        return injected_data, thrown_events

#=======================================================================================================================================
# MAIN: Calculate the expected signal
#=======================================================================================================================================

if __name__ == '__main__':

    #parse input arguments
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

    # Initialize signal processor
    processor = SignalProcessor(mpifpi=mpifpi, nsigma=nsigma)

    # Load either 10% data or 100% data, use the CR reconstructed bkg rate to scale the expected signal rate
    cr_data = '/sdf/group/hps/user-data/alspellm/2016/data/hadd_BLPass4c_1959files.root'
    full_lumi_path = '/fs/ddn/sdf/group/hps/users/alspellm/data_storage/pass4kf/pass4kf_ana_20240513'
    preselection = "vtxana_Tight_nocuts"
    signal_mass_range = [x for x in range(30,130,1)]
    processor.set_diff_prod_lut(cr_data, preselection, signal_mass_range, tenpct, full_lumi_path)

    # Initialize the mass and epsilon^2 range for the expected signal calculation
    mass_max = 124
    mass_min = 30
    mass_step = 2 # MC signal files were generated at 2 MeV increments 
    ap_step = round(mass_step*processor.mass_ratio_ap_to_vd,1)
    masses = np.array([x for x in range(mass_min, mass_max+mass_step, mass_step)])
    ap_masses = np.array([round(x*processor.mass_ratio_ap_to_vd,1) for x in masses])
    eps2_range = np.logspace(-4.0,-8.0,num=40)
    logeps2_range = np.log10(eps2_range)
    min_eps = min(np.log10(eps2_range))
    max_eps = max(np.log10(eps2_range))
    num_bins = len(eps2_range)

    # Initialize the histograms used to store the expected signal. One for Vd mass, one for Ap mass. 
    expected_signal_vd_h = (
        hist.Hist.new
        .Reg(len(masses), np.min(masses), np.max(masses)+mass_step, label='Vd Invariant Mass [MeV]')
        .Reg(num_bins, min_eps, max_eps,label=f'$log_{10}(\epsilon^2)$')
        .Double()
    )
    expected_signal_ap_h = (
        hist.Hist.new
        .Reg(len(ap_masses), np.min(ap_masses), np.max(ap_masses)+ap_step, label='A\' Invariant Mass [MeV]')
        .Reg(num_bins, min_eps, max_eps,label=f'$log_{10}(\epsilon^2)$')
        .Double()
    )

    # Calculate expected signal for each MC generated mass
    for signal_mass in masses:

        #Load MC Signal
        indir = '/sdf/group/hps/user-data/alspellm/2016/simp_mc/pass4b/beam/smeared_fixbeamspot'
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
        #sameside_sel = processor.sameside_z0_cut(signal)

        # Combine selections
        #tight_sel = np.logical_and.reduce([psum_sel,zcut_sel, vprojsig_sel, psum_sel, minz0_sel, masswindow_sel, sameside_sel])
        tight_sel = np.logical_and.reduce([psum_sel,zcut_sel, vprojsig_sel, psum_sel, minz0_sel, masswindow_sel])

        for l, eps2 in enumerate(eps2_range):
            signal = processor.get_exp_sig_eps2(signal_mass, signal, eps2)
            total_yield = signal_sf*ak.sum(signal['reweighted_accxEff'][tight_sel])*total_yield_per_epsilon2*eps2
            expected_signal_vd_h.fill(signal_mass, logeps2_range[l], weight=total_yield)
            expected_signal_ap_h.fill(signal_mass*processor.mass_ratio_ap_to_vd, logeps2_range[l], weight=total_yield)

    outfile = uproot.recreate(outfilename)
    outfile['expected_signal_vd_h'] = expected_signal_vd_h
    outfile['expected_signal_ap_h'] = expected_signal_ap_h










    




