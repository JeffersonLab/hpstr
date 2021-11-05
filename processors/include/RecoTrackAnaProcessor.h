#ifndef __RECOHODO_ANAPROCESSOR_H__
#define __RECOHODO_ANAPROCESSOR_H__

//HPSTR
#include "HpsEvent.h"
#include "Collections.h"
#include "RecoTrackAnaHistos.h"


//ROOT
#include "Processor.h"
#include "TClonesArray.h"
#include "TBranch.h"
#include "TTree.h"
#include "TFile.h"

class TTree;


class RecoTrackAnaProcessor : public Processor {

    public:

		RecoTrackAnaProcessor(const std::string& name, Process& process);

        ~RecoTrackAnaProcessor();

        virtual bool process(IEvent* ievent);

        virtual void initialize(TTree* tree);

        virtual void finalize();

        virtual void configure(const ParameterSet& parameters);

    private:

        //Containers to hold histogrammer info
        RecoHitAnaHistos* histos{nullptr};
        std::string  histCfgFilename_;

        //TODO Change this to be held from HPSEvent
        TTree* tree_;
        TBranch* btrkrHits_{nullptr};
        TBranch* btracks_{nullptr};
        TBranch* becalHits_{nullptr};
        TBranch* becalClusters_{nullptr};

        std::vector<TrackerHit*> * trkrHits_{};
        std::vector<Track*>      * tracks_{};

        std::string anaName_{"recoHitAna"};
        std::string trkColl_{"KalmanFullTracks"};
        std::string trkrHitColl_{"RotatedHelicalTrackHits"};

        //Debug Level
        int debug_{0};

};

#endif
