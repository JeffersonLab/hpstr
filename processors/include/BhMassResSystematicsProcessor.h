#ifndef __BH_MASSRESSYSTEMATICSPROCESSOR_H__
#define __BH_MASSRESSYSTEMATICSPROCESSOR_H__

// HPSTR
#include "BumpHunter.h"
#include "FlatTupleMaker.h"
#include "HpsFitResult.h"

// ROOT
#include "Processor.h"
#include "TFile.h"
#include "TH1.h"
#include "TRandom.h"

class TTree;

/**
 * @brief Insert description here.
 * more details
 */
class BhMassResSystematicsProcessor : public Processor {
    public:
        /**
         * @brief Constructor
         * 
         * @param name 
         * @param process 
         */
        BhMassResSystematicsProcessor(const std::string& name, Process& process);

        ~BhMassResSystematicsProcessor();

        /**
         * @brief Configure using given parameters.
         * 
         * @param parameters The parameters used for configuration.
         */
        virtual void configure(const ParameterSet& parameters);

        /**
         * @brief Initialize processor.
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
        TFile* function_file_{nullptr}; //!< The file that contains the mass resolution error parameterization.
        std::string function_name_{""}; //!< The name of the function object in the error file.
        BumpHunter* bump_hunter_{nullptr}; //!< The bump hunter manager
        FlatTupleMaker* flat_tuple_{nullptr}; //!< The flat tuple manager
        std::string massSpectrum_{"testSpectrum_h"}; //!< The name of the mass spectrum to fit.
        TH1* mass_spec_h{nullptr}; //!< The mass spectrum to fit
        double mass_hypo_{100.0}; //!< The signal hypothesis to use in the fit.
        double seed_ = 0.0; //!< The toy generator seed. This is always zero.
        int poly_order_{3}; //!< Order of polynomial used to model the background.
        int win_factor_{10}; //!< The factor that determines the size of the mass window as
        bool asymptotic_limit_{true}; //!< Whether to use the asymptotic upper limit or the power constrained. Defaults to asymptotic.
        int bkg_model_{3}; //!< What background model type to use.
		double res_width_{0.05}; //!< The width of the resolution Gaussian.
        int res_runs_{1000}; //!< How many resolution variance runs to make.
        int toy_res_runs_{100}; //!< How many resolution variance runs to make.
        int nToys_{1000}; //!< How many toys to generated.
        int debug_{0}; //!< Debug Level
};

#endif
