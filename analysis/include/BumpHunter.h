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

#include <Math/ProbFunc.h>

//---//
#include <HpsFitResult.h>
#include <ExpChebyshevFitFunction.h>

class BumpHunter {

    public:

        /** Enum constants used to denote the different background models. */
        enum BkgModel { 
            POLY     = 0,
            EXP_POLY = 1,
            EXP_POLY_X_POLY = 2,
        };

        /** Default Constructor */
        BumpHunter(BkgModel model, int poly_order, int res_factor);

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

        /** */
        std::vector<TH1*> generateToys(TH1* histogram, double n_toys, int seed, int toy_sig_samples);

    private:

        /**
         * Get the HPS mass resolution at the given mass.  The functional form 
         * of the mass resolution was determined using MC.
         *
         * @param mass The mass of interest.
         * @return The mass resolution at the given mass.
         */
        inline double getMassResolution(double mass) { 
            //return -6.2*mass*mass*mass + 0.91*mass*mass - 0.00297*mass + 0.000579; 
            //return 0.0389938364847*mass - 0.0000713783511061; // ideal
            //return 0.0501460737193*mass - 0.0000917925595224; // scaled to moller mass from data
            //return 0.0532190838657*mass - 0.0000922283032152; // scaled to moller mass + sys
            return 1.56*(0.000955 - 0.004198 * mass + 0.2367 * mass * mass - 0.7009 * mass * mass * mass);
        };

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
};

//
// TODO: Move the classes externally 
//

class ExpPol1BkgFunction { 
    
    public: 

        /** Constructor */
        ExpPol1BkgFunction(double mass_hypothesis, double window_size, double bin_width); 

        double operator() (double* x, double* par); 

    private: 

        /** Mass hypothesis */
        double mass_hypothesis_{0}; 

        /** Size of the search window. */
        double window_size_{0};

        /** Bin width of histogram */
        double bin_width_{0};

         
};

class ExpPol1FullFunction { 
    
    public: 

        /** Constructor */
        ExpPol1FullFunction(double mass_hypothesis, double window_size, double bin_width); 

        double operator() (double* x, double* par); 

    private: 

        double mass_hypothesis_{0}; 

        double window_size_{0}; 

        /** Bin width of histogram */
        double bin_width_{0};
};

class ExpPol3BkgFunction { 
    
    public: 

        /** Constructor */
        ExpPol3BkgFunction(double mass_hypothesis, double window_size, double bin_width); 

        double operator() (double* x, double* par); 

    private: 

        /** Mass hypothesis */
        double mass_hypothesis_{0}; 

        /** Size of the search window. */
        double window_size_{0};

        /** Bin width of histogram */
        double bin_width_{0};

         
};

class ExpPol3FullFunction { 
    
    public: 

        /** Constructor */
        ExpPol3FullFunction(double mass_hypothesis, double window_size, double bin_width); 

        double operator() (double* x, double* par); 

    private: 

        double mass_hypothesis_{0}; 

        double window_size_{0}; 

        /** Bin width of histogram */
        double bin_width_{0};
};

class ExpPol5BkgFunction { 
    
    public: 

        /** Constructor */
        ExpPol5BkgFunction(double mass_hypothesis, double window_size, double bin_width); 

        double operator() (double* x, double* par); 

    private: 

        /** Mass hypothesis */
        double mass_hypothesis_{0}; 

        /** Size of the search window. */
        double window_size_{0};

        /** Bin width of histogram */
        double bin_width_{0};

         
};


class ExpPol5FullFunction { 
    
    public: 

        /** Constructor */
        ExpPol5FullFunction(double mass_hypothesis, double window_size, double bin_width); 

        double operator() (double* x, double* par); 

    private: 

        double mass_hypothesis_{0}; 

        double window_size_{0}; 

        /** Bin width of histogram */
        double bin_width_{0};
};

class ExpPol7BkgFunction { 
    
    public: 

        /** Constructor */
        ExpPol7BkgFunction(double mass_hypothesis, double window_size, double bin_width); 

        double operator() (double* x, double* par); 

    private: 

        /** Mass hypothesis */
        double mass_hypothesis_{0}; 

        /** Size of the search window. */
        double window_size_{0};

        /** Bin width of histogram */
        double bin_width_{0};

         
};

#endif // __BUMP_HUNTER_H__
