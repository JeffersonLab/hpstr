#ifndef __SIM_PARTPROCESSOR_H__
#define __SIM_PARTPROCESSOR_H__

//HPSTR
#include "HpsEvent.h"
#include "Collections.h"
#include "MCParticle.h"
#include "MCTrackerHit.h"
#include "MCEcalHit.h"
#include "SimPartHistos.h"
#include "Track.h"
#include "TrackerHit.h"
#include "CalCluster.h"

//ROOT
#include "Processor.h"
#include "TClonesArray.h"
#include "TBranch.h"
#include "TTree.h"
#include "TFile.h"

class TTree;

/**
 * @brief Insert description here.
 * more details
 */
class SimPartProcessor : public Processor {

    public:
        /**
         * @brief Constructor
         * 
         * @param name 
         * @param process 
         */
        SimPartProcessor(const std::string& name, Process& process);

        ~SimPartProcessor();

        /**
         * @brief description
         * 
         * @param ievent 
         * @return true 
         * @return false 
         */
        virtual bool process(IEvent* ievent);

        /**
         * @brief description
         * 
         * @param tree 
         */
        virtual void initialize(TTree* tree);

        /**
         * @brief description
         * 
         */
        virtual void finalize();

        /**
         * @brief description
         * 
         * @param parameters 
         */
        virtual void configure(const ParameterSet& parameters);

    private:

        /** Containers to hold histogrammer info */
        SimPartHistos* histos{nullptr};
        std::string  histCfgFilename_; //!< description

        /** \todo Change this to be held from HPSEvent */
        TTree* tree_;
        TBranch* bMCParticles_{nullptr}; //!< description
        TBranch* bMCTrackerHits_{nullptr}; //!< description
        TBranch* bMCECalHits_{nullptr}; //!< description
        TBranch* bRecoTracks_{nullptr}; //!< description
        TBranch* bRecoTrackerClusters_{nullptr}; //!< description
        TBranch* bRecoEcalClusters_{nullptr}; //!< description

        std::vector<MCParticle*>   * MCParticles_{nullptr}; //!< description
        std::vector<MCTrackerHit*> * MCTrackerHits_{nullptr}; //!< description
        std::vector<MCEcalHit*>    * MCECalHits_{nullptr}; //!< description
        std::vector<Track*>   * RecoTracks_{nullptr}; //!< description
        std::vector<TrackerHit*> * RecoTrackerClusters_{nullptr}; //!< description
        std::vector<CalCluster*>    * RecoEcalClusters_{nullptr}; //!< description

        std::string anaName_{"SimPartAna"}; //!< description
        std::string MCParticleColl_{"MCParticle"}; //!< description
        std::string MCTrackerHitColl_{"TrackerHits"}; //!< description
        std::string MCEcalHitColl_{"EcalHits"}; //!< description
        std::string RecoTrackColl_{"KalmanFullTracks"}; //!< description
        std::string RecoTrackerClusterColl_{"SiClustersOnTrack"}; //!< description
        std::string RecoEcalClusterColl_{"RecoEcalClusters"}; //!< description

        std::string analysis_{"sim_part"}; //!< description

        int debug_{0}; //!< Debug Level

};


#endif
