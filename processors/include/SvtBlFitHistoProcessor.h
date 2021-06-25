#ifndef __SVTBLFIT_HISTOPROCESSOR_H__
#define __SVTBLFIT_HISTOPROCESSOR_H__
#include <iostream>

//ROOT
#include "Processor.h"
#include "TFile.h"
#include "TTree.h"

//HPSTR
#include "IEvent.h"
#include "BlFitHistos.h"
#include "HistoManager.h"
#include "BlFitFunction.h"
#include "FlatTupleMaker.h"

class SvtBlFitHistoProcessor : public Processor {

    public: 

        SvtBlFitHistoProcessor(const std::string& name, Process& process);
        
        ~SvtBlFitHistoProcessor();

        virtual void configure(const ParameterSet& parameters);

        virtual void initialize(std::string inFilename, std::string outFilename);

        virtual bool process();

        virtual void initialize(TTree* tree) {};

        virtual bool process(IEvent* event) {};

        virtual void finalize();

    private:

        TFile* inF_{nullptr};
        TFile* outF_chi2{nullptr};

        //Vector that holds hybrid names
        std::vector<std::string> hybrid_{};
        //json file for histo config
        std::string histCfgFilename_;

        //Histogram handlers
        BlFitHistos* fitHistos_{nullptr};

        //configurable parameters for fitting. All have default settings.
        int rebin_{};
        int nPointsDer_{};
        int minStats_{};
        int xmin_{};
        int deadRMS_{};
        //Set simpleGausFit_ to true in config file if fitting online baseline
        std::string simpleGausFit_;

        //Define RMS threshold for noisy channels
        int noisyRMS_{};

        //Maps for sensor histograms and channels
        std::vector<std::string> histos2dk_{};

        //Folder where input histograms file is located
        FlatTupleMaker* flat_tuple_{nullptr};


};

#endif
