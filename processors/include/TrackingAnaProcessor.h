#ifndef __TRACKING_ANAPROCESSOR_H__
#define __TRACKING_ANAPROCESSOR_H__

//-----------------//
//   C++  StdLib   //
//-----------------//
#include <iostream>
#include <string>

//----------//
//   ROOT   //
//----------//
#include "TClonesArray.h"

//-----------//
//   hpstr   //
//-----------//
#include "Processor.h"
#include "BaseSelector.h"
#include "Track.h"
#include "Event.h"
#include "CalCluster.h"
#include "EventHeader.h"
#include "TrackHistos.h"

// Forward declarations
class TTree; 

/**
 * @brief Insert description here.
 * more details
 */
class TrackingAnaProcessor : public Processor { 

    public:
        /**
         * @brief Class constructor. 
         *
         * @param name Name for this instance of the class.
         * @param process The Process class associated with Processor, provided
         *                by the processing framework.
         */
        TrackingAnaProcessor(const std::string& name, Process& process); 

        /** Destructor */
        ~TrackingAnaProcessor(); 

        /**
         * @brief Configure the Ana Processor
         * 
         * @param parameters The configuration parameters
         */
        virtual void configure(const ParameterSet& parameters);

        /**
         * @brief Process the event and put new data products into it.
         * 
         * @param ievent The Event to process.
         */
        virtual bool process(IEvent* ievent);

        /**
         * @brief Callback for the Processor to take any necessary
         *        action when the processing of events starts.
         * 
         * @param tree
         */
        virtual void initialize(TTree* tree);

        /**
         * @brief Callback for the Processor to take any necessary
         *        action when the processing of events finishes.
         */
        virtual void finalize();

    private: 

        /** Container to hold all Track objects. */
        std::vector<Track*>* tracks_{};
        TBranch* btracks_{nullptr}; //!< description
        
        /** Event header branch. */
        TBranch* bevth_{nullptr}; //!
        
        /** Clusters */
        TBranch* becal_{nullptr}; //!
        
        // Event Header
        EventHeader* evth_{nullptr}; //!
        std::vector<CalCluster*>* ecal_{}; //!< 
        
        std::string trkCollName_; //!< Track Collection name
        std::string ecalCollName_{"RecoEcalClusters"}; //!< Cluster Collection name

        // Track Selector configuration
        std::string selectionCfg_;
        std::shared_ptr<BaseSelector> trkSelector_; //!< description
        std::vector<std::string> regionSelections_; //!< track selections
        std::map<std::string, std::shared_ptr<BaseSelector>> reg_selectors_; //!< description
        std::map<std::string, std::shared_ptr<TrackHistos>> reg_histos_; //!< description
        typedef std::map<std::string, std::shared_ptr<TrackHistos>>::iterator reg_it; //!< description
        
        // Containers to hold histogrammer info
        std::string histCfgFilename_; //!< description
        std::string truthHistCfgFilename_; //!< description
        TrackHistos* trkHistos_{nullptr}; //!< description
        TrackHistos* truthHistos_{nullptr}; //!< description
        
        std::vector<std::string> regions_; //!
        
        bool doTruth_{false}; //!< description
        int isData_{1}; //! is data
        int debug_{0}; //!< debug level
        float time_offset_{0}; //! time offset


}; // TrackingAnaProcessor

#endif // __TRACKING_ANAPROCESSOR_
