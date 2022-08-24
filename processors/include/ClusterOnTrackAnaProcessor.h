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

/**
 * @brief Insert description here.
 * more details
 */
class ClusterOnTrackAnaProcessor : public Processor {

    public:
        /**
         * @brief Constructor
         * 
         * @param name 
         * @param process 
         */
        ClusterOnTrackAnaProcessor(const std::string& name, Process& process);

        ~ClusterOnTrackAnaProcessor();

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
         * @brief Configure using given parameters.
         * 
         * @param parameters The parameters used for configuration.
         */
        virtual void configure(const ParameterSet& parameters);

        /**
         * @brief Set baseline fits
         * 
         * @param baselineFits 
         * @param baselineRun 
         */
        void setBaselineFits(const std::string& baselineFits, const std::string& baselineRun){
            baselineFits_ = baselineFits;
            baselineRun_  = baselineRun;
        };


    private:

        ClusterHistos* clusterHistos{nullptr}; //!< description
        
        /** \todo Change this to be held from HPSEvent */
        TTree* tree_;

        std::vector<Track*> *tracks_{}; //!< Containers for adding to the TTree
        TBranch*      btracks_{nullptr}; //!< description
        std::vector<TrackerHit*> hits_{}; //!< description
        TBranch*      bhits_{nullptr}; //!< description

        std::string anaName_{"hitsOnTrack_2D"}; //!< description
        std::string trkColl_{"GBLTracks"}; //!< description
        std::string baselineFits_{""}; //!< description
        std::string baselineRun_{""}; //!< description

        int debug_{0}; //!< Debug Level

};

#endif
