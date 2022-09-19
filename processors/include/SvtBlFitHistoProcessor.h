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


/**
 * @brief Insert description here.
 * more details
 */
class SvtBlFitHistoProcessor : public Processor {

    public: 
        /**
         * @brief Constructor
         * 
         * @param name 
         * @param process 
         */
        SvtBlFitHistoProcessor(const std::string& name, Process& process);
        
        ~SvtBlFitHistoProcessor();

        /**
         * @brief description
         * 
         * @param parameters 
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
        virtual bool process(IEvent* event) {};

        /**
         * @brief description
         * 
         */
        virtual void finalize();

    private:

        TFile* inF_{nullptr}; //!< description
        TFile* outF_chi2{nullptr}; //!< description

        std::string layer_{""}; //!< Select which layer to fit baselines. Default is all.
        std::string histCfgFilename_; //!< json file for histo config
        std::string rawhitsHistCfgFilename_; //!< json file for reading in rawsvthit histograms
        std::string thresholdsFileIn_; //!< Load apv channel thresholds in

        BlFitHistos* fitHistos_{nullptr}; //!< Histogram handlers

        // Configurable parameters for fitting. All have default settings.
        int rebin_{}; //!< description
        int minStats_{}; //!< description
        int deadRMS_{}; //!< description
        int debug_{0}; //!< description

        std::string simpleGausFit_; //!< description

        FlatTupleMaker* flat_tuple_{nullptr}; //!< description


};

#endif
