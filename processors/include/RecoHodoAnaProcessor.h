#ifndef __RECOHODO_ANAPROCESSOR_H__
#define __RECOHODO_ANAPROCESSOR_H__

//HPSTR
#include "HpsEvent.h"
#include "Collections.h"
#include "RecoHodoAnaHistos.h"


//ROOT
#include "Processor.h"
#include "TClonesArray.h"
#include "TBranch.h"
#include "TTree.h"
#include "TFile.h"

class TTree;


class RecoHodoAnaProcessor : public Processor {

    public:

		RecoHodoAnaProcessor(const std::string& name, Process& process);

        ~RecoHodoAnaProcessor();

        virtual bool process(IEvent* ievent);

        virtual void initialize(TTree* tree);

        virtual void finalize();

        virtual void configure(const ParameterSet& parameters);

    private:

        //Containers to hold histogrammer info
        RecoHodoAnaHistos* histos{nullptr};
        std::string  histCfgFilename_;

        //TODO Change this to be held from HPSEvent
        TTree* tree_;
        TBranch* bhodoHits_{nullptr};
        TBranch* bhodoClusters_{nullptr};

        std::vector<HodoHit*>     * hodoHits_{};
        std::vector<HodoCluster*> * hodoClusters_{};

        std::string anaName_{"recoHodoAna"};
        std::string hodoHitColl_{"RecoHodoHits"};
        std::string hodoClusColl_{"RecoHodoClusters"};

        //Debug Level
        int debug_{0};

};

#endif
