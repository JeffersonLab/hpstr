#ifndef __CLUSTERONTRACK_ANAPROCESSOR_H__
#define __CLUSTERONTRACK_ANAPROCESSOR_H__

//HPSTR
#include "HpsEvent.h"
#include "Track.h"
#include "TrackerHit.h"


//ROOT
#include "ClusterHistos.h"
#include "Processor.h"
#include "TClonesArray.h"
#include "TFile.h"

class TTree;


class ClusterOnTrackAnaProcessor : public Processor {

    public:

        ClusterOnTrackAnaProcessor(const std::string& name, Process& process);

        ~ClusterOnTrackAnaProcessor();

        virtual bool process(IEvent* ievent);

        virtual void initialize(TTree* tree);

        virtual void finalize();

        virtual void configure(const ParameterSet& parameters);

        void setBaselineFits(const std::string& baselineFits,const std::string& baselineRun){
            baselineFits_ = baselineFits;
            baselineRun_  = baselineRun;
        };


    private:

        ClusterHistos* clusterHistos{nullptr};

        //TODO Change this to be held from HPSEvent
        TTree* tree_;

        //Containers for adding to the TTree
        std::vector<Track*> *tracks_{};
        TBranch*      btracks_{nullptr};
        std::vector<TrackerHit*> hits_{};
        TBranch*      bhits_{nullptr};

        std::string anaName_{"hitsOnTrack_2D"};
        std::string trkColl_{"GBLTracks"};
        std::string baselineFits_{""};
        std::string baselineRun_{""};

        //Debug Level
        int debug_{1};

};

#endif
