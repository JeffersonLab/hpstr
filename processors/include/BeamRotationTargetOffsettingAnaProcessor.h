#ifndef __BEAMROTATIONTARGETOFFSETTING_ANAPROCESSOR_H__
#define __BEAMROTATIONTARGETOFFSETTING_ANAPROCESSOR_H__

//HPSTR
#include "HpsEvent.h"
#include "Collections.h"

//ROOT
#include "Processor.h"
#include "TClonesArray.h"
#include "TBranch.h"
#include "TTree.h"
#include "TFile.h"

#include "TSData.h"
#include "Vertex.h"
#include "Particle.h"
#include "CalCluster.h"
#include "CalHit.h"

#include "RecoParticleAnaHistos.h"
#include "RecoTrackVertexAnaHistos.h"

#include "FlatTupleMaker.h"
#include "AnaHelpers.h"
#include "BaseSelector.h"

class TTree;


class BeamRotationTargetOffsettingAnaProcessor : public Processor {

    public:

		BeamRotationTargetOffsettingAnaProcessor(const std::string& name, Process& process);

        ~BeamRotationTargetOffsettingAnaProcessor();

        virtual bool process(IEvent* ievent);

        virtual void initialize(TTree* tree);

        virtual void finalize();

        virtual void configure(const ParameterSet& parameters);

    private:
        std::shared_ptr<AnaHelpers> _ah;

        std::shared_ptr<BaseSelector> vtxSelector;
        std::string selectionCfg_;

        //Containers to hold histogrammer info
        RecoParticleAnaHistos* histos{nullptr};
        std::string  histCfgFilename_;

        RecoParticleAnaHistos* histosParticle{nullptr};
        std::string  histCfgFilenameParticle_;

        RecoTrackVertexAnaHistos* histosVertex{nullptr};
        std::string  histCfgFilenameVertex_;

        //TODO Change this to be held from HPSEvent
        TTree* tree_;
        TBranch* btsData_{nullptr};
        TBranch* becalClusters_{nullptr};
        TBranch* btrks_{nullptr};
        TBranch* bvtxs_{nullptr};
        TBranch* bfsps_{nullptr};

        TSData* tsData_{};
        std::vector<CalCluster*> * ecalClusters_{};
        std::vector<Track*>      * trks_{};
        std::vector<Vertex*> * vtxs_{};
        std::vector<Particle*> * fsps_{};

        std::string anaName_{"beamRotationTargetOffsettingAnaProcessor"};
        std::string tsColl_{"TSBank"};
        std::string ecalClusColl_{"RecoEcalClusters"};
        std::string trkColl_{"KalmanFullTracks"};
        std::string vtxColl_{"UnconstrainedV0Vertices_KF"};
        std::string fspCollRoot_{"FinalStateParticles_KF"};

        //Debug Level
        int debug_{0};
        double timeOffset_{-999};
        double beamE_{4.55};
        int isData_{1};

        // save variables for events with three final-state particles
        std::shared_ptr<FlatTupleMaker> treeThreeFSPs;

        // save variables for vertices
        std::shared_ptr<FlatTupleMaker> treeVertex;

};

#endif
