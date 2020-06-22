/**
 * @file BumpHunter.h
 * @brief
 * @author Omar Moreno <omoreno1@ucsc.edu>
 *         Santa Cruz Institute for Particle Physics
 *         University of California, Santa Cruz
 * @date January 14, 2015
 */

#ifndef __BUMP_HUNTER_H__
#define __BUMP_HUNTER_H__

//----------------//   
//   C++ StdLib   //
//----------------//  
#include <cmath>
#include <cstdio> 
#include <exception>
#include <fstream>
#include <map>
#include <vector>

//----------//
//   ROOT   //
//----------// 
#include <TF1.h> 
#include <TFitResult.h>
#include <TFitResultPtr.h>
#include <TH1.h>
#include <TMath.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TRandom.h>
#include <TString.h>

#include <Math/ProbFunc.h>
#include <Math/MinimizerOptions.h>

//---//
#include "HpsFitResult.h"
#include "ChebyshevFitFunction.h"
#include "LegendreFitFunction.h"

class BumpHunter {
    public:
        /** Default Constructor */
        BumpHunter(FitFunction::BkgModel model, int poly_order, int toy_poly_order, int res_factor, double res_scale = 1.00, bool asymptotic_limit = true);
        
        /** Destructor */
        ~BumpHunter();
        
        /** 
         * Perform a search for a resonance at the given mass hypothesis.
         *
         * @param histogram Histogram containing the mass spectrum that will be
         *                  used to search for a resonance.
         * @param mass_hypothesis The mass of interest.
         */
        HpsFitResult* performSearch(TH1* histogram, double mass_hypothesis, bool skip_bkg_fit, bool skip_ul);
        
        /** 
         * Given the mass of interest, setup the window parameters and 
         * initialize the fit parameters.  This includes setting the size of the
         * window and the window edges as well as estimating the initial value
         * of some fit parameters.
         *
         * @param histogram Histogram containing the mass spectrum that will be
         *                  used to search for a resonance.
         * @param mass_hypothesis The mass of interest.
         */
        void initialize(TH1* histogram, double &mass_hypothesis);
        
        /**
         *
         */
        void calculatePValue(HpsFitResult* result);
        
        /** Fit using a background only model. */
        void fitBkgOnly();
        
        /** Set the histogram bounds. */
        void setBounds(double low_bound, double high_bound);
        
        /** Enable/disable debug */
        void enableDebug(bool debug = true) { this->debug = debug; };
        
        /** Enable batch running. */
        void runBatchMode(bool batch = true) { _batch = batch; };
        
        /** Write the fit results to a text file */
        void writeResults(bool write_results = true) { _write_results = write_results; };
        
        /** Get the signal upper limit. */
        void getUpperLimit(TH1* histogram, HpsFitResult* result);
        void getUpperLimitAsymptotic(TH1* histogram, HpsFitResult* result);
        void getUpperLimitAsymCLs(TH1* histogram, HpsFitResult* result);
        void getUpperLimitPower(TH1* histogram, HpsFitResult* result);
        
        /** Set the resolution after instantiation. */
        void setResolutionScale(double res_scale) { res_scale_ = res_scale; }

        /** Sets whether the window size is scaled according to the resolution scaling factor. */
        void setWindowSizeUsesResScale(bool window_use_res_scale) { window_use_res_scale_ = window_use_res_scale; }
        
        /** */
        std::vector<TH1*> generateToys(TH1* histogram, double n_toys, int seed, int toy_sig_samples, int bkg_mult = 1, TH1* signal_hist = nullptr);

        /**
         * Get the HPS mass resolution at the given mass.  The functional form
         * of the mass resolution was determined using MC.
         *
         * @param mass The mass of interest.
         * @return The mass resolution at the given mass.
         */
        double getMassResolution(double mass) {
            // Omar's 2015 mass resolution.
            //return res_scale_ * ((-6.2 * mass * mass * mass) + (0.91 * mass * mass) - (0.00297 * mass) + 0.000579);
            //return 0.0389938364847*mass - 0.0000713783511061; // ideal
            //return 0.0501460737193*mass - 0.0000917925595224; // scaled to moller mass from data
            //return 0.0532190838657*mass - 0.0000922283032152; // scaled to moller mass + sys
            //return res_scale_*(0.000955 - 0.004198 * mass + 0.2367 * mass * mass - 0.7009 * mass * mass * mass);

            // Rafo's 2016 mass resolution.
            //return res_scale_ * (0.000379509 + (0.0416842 * mass) - (0.271364 * mass * mass) + (3.49537 * mass * mass * mass) - (11.1153 * mass * mass * mass * mass));
            return getMassResolution(mass, res_scale_);
        };
        
    private:
        inline double getMassResolution(double mass, double res_scale) {
            return res_scale * (0.000379509 + (0.0416842 * mass) - (0.271364 * mass * mass) + (3.49537 * mass * mass * mass) - (11.1153 * mass * mass * mass * mass));
        }
        
        /**
         *
         */
        double correctMass(double mass);
        
        /** 
         * Print debug statement.
         *
         * @param message Debug statement to print.
         */
        void printDebug(std::string message);
         
        /**
         *
         */
        void getChi2Prob(double min_nll_null, double min_nll, double &q0, double &p_value);

        /** Background only fit result. */
        HpsFitResult* bkg_only_result_{nullptr};
        
        /** Output file stream */
        std::ofstream* ofs;
        
        //
        // Variable definitions
        //
        /** The mass after the mass scale correction. */
        double corr_mass_{0};
        
        /** The lower bound of the histogram. */
        double lower_bound_{0.016};
        
        /** The upper bound of the histogram. */
        double upper_bound_{0.115};
        
        /** The total number of events within the fit window. */
        double integral_{0};
        
        /** The background fit function model to use. */
        FitFunction::BkgModel bkg_model_{FitFunction::BkgModel::EXP_CHEBYSHEV};

        /**
         * Flag to specify whether the power constrained or asymptotic upper
         * limit should be employed.
         */
        bool asymptotic_limit_{true};
        
        /** 
         * Resolution multiplicative factor used in determining the fit window 
         * size.
         */
        double res_factor_{13};
        
        /** Size of the background window that will be used to fit. */
        double window_size_{0};
        
        /** The total number of bins */
        int bins_{0};
        
        /** The bin width */
        double bin_width_{0.0};
        
        /** Polynomial order used to model the background. */
        int poly_order_{0};

        /** Polynomial order used to model the toy fit. */
        int toy_poly_order_{0};

        /** The scaling factor for the mass resolution. Was 1.56 for Sebouh's, should 1.00 for Rafo's. */
        //double res_scale_{1.56};
        double res_scale_{1.00};
        
        /** 
         * Flag denoting if application should run in batch mode.  If set to 
         * true, plots aren't generated and fit results aren't logged.
         */
        bool _batch{false};
        
        /** Debug flag */
        bool debug{false};
        
        /** Write the results to a file. */
        bool _write_results{false};
        
        double window_start_{0};
        
        double window_end_{0};
        
        double mass_hypothesis_{0};
        
        double mass_resolution_{0};

        bool window_use_res_scale_{true};
};

#endif // __BUMP_HUNTER_H__
