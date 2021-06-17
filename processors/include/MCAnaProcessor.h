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

        //Containers to hold histogrammer info
        MCAnaHistos* histos{nullptr};
        std::string  histCfgFilename_;

        //TODO Change this to be held from HPSEvent
        TTree* tree_;
        TBranch* bmcParts_{nullptr};
        TBranch* bmcTrkrHits_{nullptr};
        TBranch* bmcEcalHits_{nullptr};

        std::vector<MCParticle*>   * mcParts_{nullptr};
        std::vector<MCTrackerHit*> * mcTrkrHits_{nullptr};
        std::vector<MCEcalHit*>    * mcEcalHits_{nullptr};

        std::string anaName_{"recoHitAna"};
        std::string partColl_{"MCParticle"};
        std::string trkrHitColl_{"TrackerHits"};
        std::string ecalHitColl_{"EcalHits"};
	std::string analysis_{"vertex"};


        //Debug Level
        int debug_{0};

};


#endif
