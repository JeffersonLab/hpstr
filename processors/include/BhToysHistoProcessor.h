#ifndef __BHTOYS_HISTOPROCESSOR_H__
#define __BHTOYS_HISTOPROCESSOR_H__

//HPSTR
#include "BumpHunter.h"
#include "FlatTupleMaker.h"
#include "HpsFitResult.h"

//ROOT
#include "Processor.h"
#include "TFile.h"
#include "TH1.h"

class TTree;


class BhToysHistoProcessor : public Processor {

    public:

        BhToysHistoProcessor(const std::string& name, Process& process);

        ~BhToysHistoProcessor();

        virtual void configure(const ParameterSet& parameters);

        virtual void initialize(std::string inFilename, std::string outFilename);

        virtual bool process();

        virtual void initialize(TTree* tree) {};

        virtual bool process(IEvent* event) {};

        virtual void finalize();

    private:

        TFile* inF_{nullptr};

        // The bump hunter manager
        BumpHunter* bump_hunter_{nullptr};

        // The bkg model
        BumpHunter::BkgModel bkg_model_{BumpHunter::BkgModel::EXP_POLY};

        // The flat tuple manager
        FlatTupleMaker* flat_tuple_{nullptr};

        // The name of the mass spectrum to fit.
        std::string massSpectrum_{"testSpectrum_h"};

        // The mass spectrum to fit
        TH1* mass_spec_h{nullptr};

        // The signal hypothesis to use in the fit.
        double mass_hypo_{100.0};
        
        // Order of polynomial used to model the background.
        int poly_order_{3};

        // The factor that determines the size of the mass window as
        //      window_size = (mass_resolution*win_factor)
        int win_factor_{10};

        // The seed used in generating random numbers.  The default of 0 causes
        // the generator to use the system time.
        int seed_{10};

        // Number of toys to throw and fit
        int nToys_{50};

        //Debug Level
        int debug_{0};

};


#endif
