#ifndef __RECOHIT_ANAPROCESSOR_H__
#define __RECOHIT_ANAPROCESSOR_H__

// HPSTR
#include "HpsEvent.h"
#include "Collections.h"
#include "Track.h"
#include "TrackerHit.h"
#include "RecoHitAnaHistos.h"


// ROOT
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
class RecoHitAnaProcessor : public Processor {

    public:
        /**
         * @brief Constructor
         * 
         * @param name 
         * @param process 
         */
        RecoHitAnaProcessor(const std::string& name, Process& process);

        ~RecoHitAnaProcessor();

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
        RecoHitAnaHistos* histos{nullptr};
        std::string  histCfgFilename_; //!< description

        /** \todo Change this to be held from HPSEvent */
        TTree* tree_; 
        TBranch* btrkrHits_{nullptr}; //!< description
        TBranch* btracks_{nullptr}; //!< description
        TBranch* becalHits_{nullptr}; //!< description
        TBranch* becalClusters_{nullptr}; //!< description

        std::vector<TrackerHit*> * trkrHits_{}; //!< description
        std::vector<Track*>      * tracks_{}; //!< description
        std::vector<CalHit*>     * ecalHits_{}; //!< description
        std::vector<CalCluster*> * ecalClusters_{}; //!< description

        std::string anaName_{"recoHitAna"}; //!< description
        std::string trkColl_{"GBLTracks"}; //!< description
        std::string trkrHitColl_{"RotatedHelicalTrackHits"}; //!< description
        std::string ecalHitColl_{"EcalCalHits"}; //!< description
        std::string ecalClusColl_{"EcalClustersCorr"}; //!< description

        int debug_{0}; //!< Debug Level

};


#endif
