#ifndef __SVTBLFIT_HISTOPROCESSOR_H__
#define __SVTBLFIT_HISTOPROCESSOR_H__
#include <iostream>

//ROOT
#include "Processor.h"
#include "TFile.h"
#include "TTree.h"

//HPSTR
#include "IEvent.h"
#include "anaUtils.h"
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
        std::vector<std::string> hybrid_{};
        int IterativeGaussFitCut_;
        //parameter defining json file
        std::string histCfgFilename_;
        int xmin_{};

        //Histogram handlers
        BlFitHistos* outputHistos_{nullptr};
        HistoManager* inputHistos_{nullptr};

        //binning for profileYIterativeGauss
        //int binning_{1};
        int rebin_{};
        int nPointsDer_{};
        int minStats_{};

        //Maps for sensor histograms and channels
        std::vector<std::string> histos2dk_{};

        //Folder where input histograms file is located
        std::string outDir_{"."};
        FlatTupleMaker* flat_tuple_{nullptr};


};

#endif
