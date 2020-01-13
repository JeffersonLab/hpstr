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
#include "HistogramHelpers.h"


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

        TFile* inFile{nullptr};
        
        int binning = 1;
        //Maps for sensor histograms and channels
        std::vector<std::string> histos2dk{};
        std::map<std::string, TH2F*> histos2d{};
        std::map<std::string, TH1D*> singleChannel_h{};

        //1D histogram names for each fit parameter for each sensor
        std::string graphname_m{""};
        std::string graphname_w{""};
        std::string graphname_n{""};
        std::string graphname_l{""};
        std::string graphname_u{""};

        //histograms that hold fit parameters for each channel
        std::map<std::string, TH1D*> histoMean{}; 
        std::map<std::string, TH1D*> histoWidth{};
        std::map<std::string, TH1D*> histoNorm{};
        std::map<std::string, TH1D*> histoFitRangeLower{};
        std::map<std::string, TH1D*> histoFitRangeUpper{};

        /*TH1D* histoMean{nullptr};
        TH1D* histoWidth{nullptr};
        TH1D* histoNorm{nullptr};
        TH1D* histoFitRangeLower{nullptr};
        TH1D* histoFitRangeUpper{nullptr};
        */

        //Folder where input histograms file is located
        std::string folder{"."};


};

#endif
