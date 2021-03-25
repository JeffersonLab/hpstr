#ifndef __APV25ROXTALK_ANAPROCESSOR_H__
#define __APV25ROXTALK_ANAPROCESSOR_H__

//HPSTR
#include "Processor.h"
#include "HpsEvent.h"
#include "Collections.h"
#include "EventHeader.h"
#include "RawSvtHit.h"
#include "ModuleMapper.h"
// #include "Apv25XtalkAnaHistos.h"

//ROOT
#include "TClonesArray.h"
#include "TBranch.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TTree.h"
#include "TFile.h"
#include "TString.h"

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

        void emulateApv25Buff(int buffIter);

        //Containers to hold histogrammer info
        //RecoHitAnaHistos* histos{nullptr};
        std::string  histCfgFilename_;
        ModuleMapper * modMap_{nullptr};

        //TODO Change this to be held from HPSEvent
        TTree* tree_;
        TBranch* bevth_{nullptr};
        TBranch* brawHits_{nullptr};

        EventHeader* evth_{nullptr};
        std::vector<RawSvtHit*> * rawHits_{};

        std::string anaName_{"apvRoXtalkAna"};
        std::string rawHitColl_{"SVTRawTrackerHits"};
        int syncPhase_{224};
        int trigPhase_{8};
        int trigDel_{6696};

        std::vector<long> reads;
        std::vector<long> readEvs;
        std::vector<long> eventTimes;
        std::vector<int>  hitMultis;
        std::vector<int>  lFEBMultis;
        std::vector<int>  hFEBMultis;

        //Debug Level
        int debug_{0};

};


#endif
