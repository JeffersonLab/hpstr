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

        std::string baselineFits_{""};
        std::string baselineRun_{""};

        //TODO Change this to be held from HPSEvent
        TTree* tree_;
        std::vector<Track*> *tracks_{};
        TBranch*      btracks_{nullptr};
        std::vector<TrackerHit*> hits_{};
        TBranch*      bhits_{nullptr};
        TFile*        outF_{nullptr};

};


#endif
