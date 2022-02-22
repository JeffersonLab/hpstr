#ifndef __RECOCOMBO_ANAPROCESSOR_H__
#define __RECOCOMBO_ANAPROCESSOR_H__

//HPSTR
#include "HpsEvent.h"
#include "Collections.h"
#include "RecoEcalAnaHistos.h"
#include "RecoHodoAnaHistos.h"
#include "RecoTrackAnaHistos.h"
#include "RecoParticleAnaHistos.h"

//ROOT
#include "Processor.h"
#include "TClonesArray.h"
#include "TBranch.h"
#include "TTree.h"
#include "TFile.h"

class TTree;


class RecoComboAnaProcessor : public Processor {

    public:

		RecoComboAnaProcessor(const std::string& name, Process& process);

        ~RecoComboAnaProcessor();

        virtual bool process(IEvent* ievent);

        virtual void initialize(TTree* tree);

        virtual void finalize();

        virtual void configure(const ParameterSet& parameters);

    private:

        //Containers to hold histogrammer info
        RecoEcalAnaHistos* histosEcal{nullptr};
        std::string  histCfgFilenameEcal_;

        RecoHodoAnaHistos* histosHodo{nullptr};
        std::string  histCfgFilenameHodo_;

        RecoTrackAnaHistos* histosTrack{nullptr};
        std::string  histCfgFilenameTrack_;

        RecoParticleAnaHistos* histosParticle{nullptr};
        std::string  histCfgFilenameParticle_;



        //TODO Change this to be held from HPSEvent
        TTree* tree_;
        TBranch* becalHits_{nullptr};
        TBranch* becalClusters_{nullptr};
        TBranch* bhodoHits_{nullptr};
        TBranch* bhodoClusters_{nullptr};
        TBranch* bvtxs_{nullptr};
        TBranch* btracks_{nullptr};
        TBranch* bfsps_{nullptr};

        std::vector<CalHit*>     * ecalHits_{};
        std::vector<CalCluster*> * ecalClusters_{};
        std::vector<HodoHit*>     * hodoHits_{};
        std::vector<HodoCluster*> * hodoClusters_{};
        std::vector<Vertex*> * vtxs_{};
        std::vector<Track*>      * tracks_{};
        std::vector<Particle*> * fsps_{};

        std::string anaName_{"recoComboAna"};
        std::string ecalHitColl_{"EcalCalHits"};
        std::string ecalClusColl_{"EcalClustersCorr"};
        std::string hodoHitColl_{"RecoHodoHits"};
        std::string hodoClusColl_{"RecoHodoClusters"};
        std::string vtxColl_{"UnconstrainedV0Vertices_KF"};
        std::string trkColl_{"KalmanFullTracks"};
        std::string fspCollRoot_{"FinalStateParticles_KF"};

        //Debug Level
        int debug_{0};

};

#endif
