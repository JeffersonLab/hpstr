#ifndef __APV25ROXTALK_ANAPROCESSOR_H__
#define __APV25ROXTALK_ANAPROCESSOR_H__

//HPSTR
#include "HpsEvent.h"
#include "Collections.h"
#include "EventHeader.h"
#include "RawSvtHit.h"
// #include "Apv25XtalkAnaHistos.h"

//ROOT
#include "Processor.h"
#include "TClonesArray.h"
#include "TBranch.h"
#include "TTree.h"
#include "TFile.h"

class TTree;


class Apv25RoXtalkAnaProcessor : public Processor {

    public:

        Apv25RoXtalkAnaProcessor(const std::string& name, Process& process);

        ~Apv25RoXtalkAnaProcessor();

        virtual bool process(IEvent* ievent);

        virtual void initialize(TTree* tree);

        virtual void finalize();

        virtual void configure(const ParameterSet& parameters);

    private:

        //Containers to hold histogrammer info
        //RecoHitAnaHistos* histos{nullptr};
        std::string  histCfgFilename_;

        //TODO Change this to be held from HPSEvent
        TTree* tree_;
        TBranch* bevth_{nullptr};
        TBranch* brawHits_{nullptr};

        EventHeader* evth_{nullptr};
        std::vector<RawSvtHit*> * rawHits_{};

        std::string anaName_{"apvRoXtalkAna"};
        std::string rawHitColl_{"SVTRawTrackerHits"};
        int initSyncPhase_{224};
        int trigDel_{6696};

        std::vector<int> eventTimes;
        std::vector<int>  hitMultis;
        std::vector<int>  lFEBMultis;
        std::vector<int>  hFEBMultis;

        //Debug Level
        int debug_{0};

};


#endif
