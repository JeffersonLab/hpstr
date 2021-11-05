#ifndef __RECOECAL_ANAPROCESSOR_H__
#define __RECOECAL_ANAPROCESSOR_H__

//HPSTR
#include "HpsEvent.h"
#include "Collections.h"
#include "RecoEcalAnaHistos.h"


//ROOT
#include "Processor.h"
#include "TClonesArray.h"
#include "TBranch.h"
#include "TTree.h"
#include "TFile.h"

class TTree;


class RecoEcalAnaProcessor : public Processor {

    public:

		RecoEcalAnaProcessor(const std::string& name, Process& process);

        ~RecoEcalAnaProcessor();

        virtual bool process(IEvent* ievent);

        virtual void initialize(TTree* tree);

        virtual void finalize();

        virtual void configure(const ParameterSet& parameters);

    private:

        //Containers to hold histogrammer info
        RecoEcalAnaHistos* histos{nullptr};
        std::string  histCfgFilename_;

        //TODO Change this to be held from HPSEvent
        TTree* tree_;
        TBranch* becalHits_{nullptr};
        TBranch* becalClusters_{nullptr};

        std::vector<CalHit*>     * ecalHits_{};
        std::vector<CalCluster*> * ecalClusters_{};

        std::string anaName_{"recoEcalAna"};
        std::string ecalHitColl_{"EcalCalHits"};
        std::string ecalClusColl_{"EcalClustersCorr"};

        //Debug Level
        int debug_{0};

};

#endif
