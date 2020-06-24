#ifndef __BH_MASSRESSYSTEMATICSPROCESSOR_H__
#define __BH_MASSRESSYSTEMATICSPROCESSOR_H__

//HPSTR
#include "BumpHunter.h"
#include "FlatTupleMaker.h"
#include "HpsFitResult.h"

//ROOT
#include "Processor.h"
#include "TFile.h"
#include "TH1.h"
#include "TRandom.h"

class TTree;

class BhMassResSystematicsProcessor : public Processor {
    public:
        BhMassResSystematicsProcessor(const std::string& name, Process& process);

        ~BhMassResSystematicsProcessor();

        virtual void configure(const ParameterSet& parameters);

        virtual void initialize(std::string inFilename, std::string outFilename);

        virtual bool process();

        virtual void initialize(TTree* tree) {};

        virtual bool process(IEvent* event) { return true;};

        virtual void finalize();

    private:
        TFile* inF_{nullptr};

        // The file that contains the mass resolution error parameterization.
        TFile* function_file_{nullptr};

        // The name of the function object in the error file.
        std::string function_name_{""};

        // The bump hunter manager
        BumpHunter* bump_hunter_{nullptr};

        // The flat tuple manager
        FlatTupleMaker* flat_tuple_{nullptr};

        // The name of the mass spectrum to fit.
        std::string massSpectrum_{"testSpectrum_h"};

        // The mass spectrum to fit
        TH1* mass_spec_h{nullptr};

        // The signal hypothesis to use in the fit.
        double mass_hypo_{100.0};
		
		// The toy generator seed. This is always zero.
		double seed_ = 0.0;
        
        // Order of polynomial used to model the background.
        int poly_order_{3};

        // The factor that determines the size of the mass window as
        int win_factor_{10};
        
        // Whether to use the asymptotic upper limit or the power constrained. Defaults to asymptotic.
        bool asymptotic_limit_{true};

        // What background model type to use.
        int bkg_model_{3};
		
		// The width of the resolution Gaussian.
		double res_width_{0.05};
		
		// How many resolution variance runs to make.
		int res_runs_{1000};
		
		// How many resolution variance runs to make.
		int toy_res_runs_{100};
		
		// How many toys to generated.
		int nToys_{1000};

        //Debug Level
        int debug_{0};

};

#endif
