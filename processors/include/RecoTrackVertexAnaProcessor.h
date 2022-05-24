#ifndef __RECOTRACKVERTEX_ANAPROCESSOR_H__
#define __RECOTRACKVERTEX_ANAPROCESSOR_H__

//HPSTR
#include "../../analysis/include/RecoTrackVertexAnaHistos.h"
#include "HpsEvent.h"
#include "Collections.h"
#include "Processor.h"
#include "TClonesArray.h"
#include "TBranch.h"
#include "TTree.h"
#include "TFile.h"

class TTree;


class RecoTrackVertexAnaProcessor : public Processor {

    public:

		RecoTrackVertexAnaProcessor(const std::string& name, Process& process);

        ~RecoTrackVertexAnaProcessor();

        virtual bool process(IEvent* ievent);

        virtual void initialize(TTree* tree);

        virtual void finalize();

        virtual void configure(const ParameterSet& parameters);

    private:

        //Containers to hold histogrammer info
        RecoTrackVertexAnaHistos* histos{nullptr};
        std::string  histCfgFilename_;

        //TODO Change this to be held from HPSEvent
        TTree* tree_;
        TBranch* bvtxs_{nullptr};
        TBranch* btracks_{nullptr};

        std::vector<Vertex*> * vtxs_{};
        std::vector<Track*>      * tracks_{};

        std::string anaName_{"recoTrackAna"};
        std::string vtxColl_{"UnconstrainedV0Vertices_KF"};
        std::string trkColl_{"KalmanFullTracks"};


        //Debug Level
        int debug_{0};

};

#endif
