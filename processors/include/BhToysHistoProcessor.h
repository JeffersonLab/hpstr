#ifndef __BHTOYS_HISTOPROCESSOR_H__
#define __BHTOYS_HISTOPROCESSOR_H__

//HPSTR
#include "HpsEvent.h"
#include "Collections.h"
#include "Track.h"
#include "TrackerHit.h"
#include "RecoHitAnaHistos.h"


//ROOT
#include "Processor.h"
#include "TClonesArray.h"
#include "TBranch.h"
#include "TTree.h"
#include "TFile.h"

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

        std::string massSpectrum_{"testSpectrum_h"};
        int polOrder_{1};
        double massHypo_{100.0};

        //Debug Level
        int debug_{0};

};


#endif
