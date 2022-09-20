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

        int  year_=2019;

        //Select which layer to fit baselines. Default is all.
        std::string layer_{""};

        //json file for histo config
        std::string histCfgFilename_;

        //json file for reading in rawsvthit histograms
        std::string rawhitsHistCfgFilename_;

        //Histogram handlers
        BlFitHistos* fitHistos_{nullptr};

        //Load apv channel thresholds in
        std::string thresholdsFileIn_;

        //configurable parameters for fitting. All have default settings.
        int rebin_{};
        int minStats_{};
        int deadRMS_{};
        int debug_{0};

        std::string simpleGausFit_;

        FlatTupleMaker* flat_tuple_{nullptr};


};

#endif
