#ifndef __MC_ANAPROCESSOR_H__
#define __MC_ANAPROCESSOR_H__

//HPSTR
#include "HpsEvent.h"
#include "Collections.h"
#include "MCParticle.h"
#include "MCTrackerHit.h"
#include "MCEcalHit.h"
#include "MCAnaHistos.h"


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
class MCAnaProcessor : public Processor {

    public:
        /**
         * @brief Constructor
         * 
         * @param name 
         * @param process 
         */
        MCAnaProcessor(const std::string& name, Process& process);

        ~MCAnaProcessor();

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
        MCAnaHistos* histos{nullptr};
        std::string  histCfgFilename_; //!< description

        /** \todo Change this to be held from HPSEvent */
        TTree* tree_;
        TBranch* bmcParts_{nullptr}; //!< description
        TBranch* bmcTrkrHits_{nullptr}; //!< description
        TBranch* bmcEcalHits_{nullptr}; //!< description

        std::vector<MCParticle*>   * mcParts_{nullptr}; //!< description
        std::vector<MCTrackerHit*> * mcTrkrHits_{nullptr}; //!< description
        std::vector<MCEcalHit*>    * mcEcalHits_{nullptr}; //!< description

        std::string anaName_{"recoHitAna"}; //!< description
        std::string partColl_{"MCParticle"}; //!< description
        std::string trkrHitColl_{"TrackerHits"}; //!< description
        std::string ecalHitColl_{"EcalHits"}; //!< description
        std::string analysis_{"vertex"}; //!< description

        int debug_{0}; //!< Debug Level

};


#endif
