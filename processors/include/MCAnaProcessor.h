#ifndef __MC_ANAPROCESSOR_H__
#define __MC_ANAPROCESSOR_H__

//HPSTR
#include "HpsEvent.h"
#include "Collections.h"
#include "MCParticle.h"
#include "MCTrackerHit.h"
#include "MCEcalHit.h"
#include "MCAnaHistos.h"


//ROOT
#include "Processor.h"
#include "TClonesArray.h"
#include "TBranch.h"
#include "TTree.h"
#include "TFile.h"

class TTree;


class MCAnaProcessor : public Processor {

    public:

        MCAnaProcessor(const std::string& name, Process& process);

        ~MCAnaProcessor();

        virtual bool process(IEvent* ievent);

        virtual void initialize(TTree* tree);

        virtual void finalize();

        virtual void configure(const ParameterSet& parameters);

    private:

        MCAnaHistos* histos{nullptr};

        //TODO Change this to be held from HPSEvent
        TTree* tree_;
        TBranch* bmcParts_{nullptr};
        TBranch* bmcTrkrHits_{nullptr};
        TBranch* bmcEcalHits_{nullptr};
        TClonesArray* mcParts_{nullptr};
        TClonesArray* mcTrkrHits_{nullptr};
        TClonesArray* mcEcalHits_{nullptr};

};


#endif
