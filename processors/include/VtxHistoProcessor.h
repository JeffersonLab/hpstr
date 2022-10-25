#ifndef __VTX_HISTOPROCESSOR_H__
#define __VTX_HISTOPROCESSOR_H__

// HPSTR
#include "Processor.h"
#include "HistogramHelpers.h"

// ROOT
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TAxis.h"

// CPLUSPLUS
#include <map>
#include <vector>
#include <memory>

/**
 * @brief Insert description here.
 * more details
 */
class VtxHistoProcessor : public Processor {

    public:
        /**
         * @brief Constructor
         * 
         * @param name 
         * @param process 
         */
        VtxHistoProcessor(const std::string& name, Process& process);

        ~VtxHistoProcessor();

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
        virtual bool process(IEvent* event) { return true;};

        /**
         * @brief description
         * 
         */
        virtual void finalize();

    private:

        TFile* inF_{nullptr}; //!< description

        int debug_{0}; //!< Debug Level
        int rebin_{1}; //!< Rebin factor

        std::vector<std::string> selections_{}; //!< Selection folder
        std::vector<std::string> projections_; //!< 2D histos to project
        std::map<std::string,TH2F*> _histos2d; //!< Map storing the 2D histograms
        typedef std::map<std::string,TH2F*>::iterator it2d_;

        std::map<std::string,TH1F* > _histos1d; //!< Map storing the 1D biases and resolutions
        typedef std::map<std::string,TH1F*>::iterator it1d_;

};


#endif
