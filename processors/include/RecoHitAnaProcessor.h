#ifndef __RECOHIT_ANAPROCESSOR_H__
#define __RECOHIT_ANAPROCESSOR_H__

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


class RecoHitAnaProcessor : public Processor {

    public:

        RecoHitAnaProcessor(const std::string& name, Process& process);

        ~RecoHitAnaProcessor();

        virtual bool process(IEvent* ievent);

        virtual void initialize(TTree* tree);

        virtual void finalize();

        virtual void configure(const ParameterSet& parameters);

    private:

        RecoHitAnaHistos* histos{nullptr};

        //TODO Change this to be held from HPSEvent
        TTree* tree_;
        TBranch* btrkrHits_{nullptr};
        TBranch* btracks_{nullptr};
        TBranch* becalHits_{nullptr};
        TBranch* becalClusters_{nullptr};

        std::vector<TrackerHit*> * trkrHits_{};
        std::vector<Track*>      * tracks_{};
        std::vector<CalHit*>     * ecalHits_{};
        std::vector<CalCluster*> * ecalClusters_{};

};


#endif
