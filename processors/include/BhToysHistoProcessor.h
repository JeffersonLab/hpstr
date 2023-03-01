#ifndef __BHTOYS_HISTOPROCESSOR_H__
#define __BHTOYS_HISTOPROCESSOR_H__

// HPSTR
#include "BumpHunter.h"
#include "FlatTupleMaker.h"
#include "HpsFitResult.h"

// ROOT
#include "Processor.h"
#include "TFile.h"
#include "TH1.h"

class TTree;

/**
 * @brief Insert description here.
 * more details
 */
class BhToysHistoProcessor : public Processor {

    public:
        /**
         * @brief Constructor
         * 
         * @param name 
         * @param process 
         */
        BhToysHistoProcessor(const std::string& name, Process& process);

        ~BhToysHistoProcessor();

        /**
         * @brief Configure using given parameters.
         * 
         * @param parameters The parameters used for configuration.
         */
        virtual void configure(const ParameterSet& parameters);

        /**
         * @brief description
         * 
         * @param inFilename 
         * @param outFilename 
         */
        virtual void initialize(std::string inFilename, std::string outFilename);

        /**
         * @brief description
         * 
         * @return true 
         * @return false 
         */
        virtual bool process();

        /**
         * @brief description
         * 
         * @param tree 
         */
        virtual void initialize(TTree* tree) {};

        /**
         * @brief description
         * 
         * @param event 
         * @return true 
         * @return false 
         */
        virtual bool process(IEvent* event) { return true;};

        /**
         * @brief description
         * 
         */
        virtual void finalize();

    private:

        TFile* inF_{nullptr}; //!< description

        BumpHunter* bump_hunter_{nullptr}; //!< The bump hunter manager

        FlatTupleMaker* flat_tuple_{nullptr}; //!< The flat tuple manager

        std::string massSpectrum_{"testSpectrum_h"}; //!< The name of the mass spectrum to fit.

        TH1* mass_spec_h{nullptr}; //!< The mass spectrum to fit

        //std::string* signal_shape_h_name_{"bhTight/bhTight_vtx_InvM_h"};
        std::string signal_shape_h_name_{""}; //!< The signal shape histogram name to use, if defined.
        //std::string* signal_shape_h_file_{"/volatile/hallb/hps/mccarty/anaBhAp100.root"};
        std::string signal_shape_h_file_{""}; //!< The signal shpae histogram file path, if defined.

        TH1* signal_shape_h_{nullptr}; //!< The signal shape histogram to use.

        double mass_hypo_{100.0}; //!< The signal hypothesis to use in the fit.
        int poly_order_{3}; //!< Order of polynomial used to model the background.
        int toy_poly_order_{-1}; //!< Order of polynomial used to create the toy generator function.

        /**
         * The factor that determines the size of the mass window as
         * window_size = (mass_resolution*win_factor)
         */
        int win_factor_{10};

        /**
         * The seed used in generating random numbers.
         * The default of 0 causes the generator to use the system time.
         */
        int seed_{10};

        int nToys_{50}; //!< Number of toys to throw and fit

        /**
         * Number of samples for signal to employ in toy model generation.
         * Defaults to zero.
         */
        int toy_sig_samples_{0};

        /**
         * Toy background events are by default equal in number to the integral
         * of the invariant mass distribution. The number of events may be modified
         * by a multiplicative factor.
         */
        int bkg_mult_{1};

        double res_scale_{1.00}; //!< The factor by which to scale the mass resolution function.
        bool asymptotic_limit_{true}; //!< Whether to use the asymptotic upper limit or the power constrained. Defaults to asymptotic.
        int bkg_model_{1}; //!< What background model type to use.
        int debug_{0}; //!< Debug Level
};

#endif
