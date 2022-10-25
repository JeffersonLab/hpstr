#ifndef __TRACKHIT_ANAPROCESSOR_H__
#define __TRACKHIT_ANAPROCESSOR_H__

//-----------------//
//   C++  StdLib   //
//-----------------//
#include <iostream>
#include <string>
#include <vector>

//----------//
//   ROOT   //
//----------//
#include "TClonesArray.h"
#include "TRefArray.h"

//-----------//
//   hpstr   //
//-----------//
#include "Processor.h"
#include "BaseSelector.h"
#include "Track.h"
#include "TrackerHit.h"
#include "Event.h"
#include "TrackHistos.h"
#include "AnaHelpers.h"

// Forward declarations
class TTree; 

/**
 * @brief Insert description here.
 * more details
 */
class TrackHitAnaProcessor : public Processor { 

    public: 
        /**
         * @brief Class constructor. 
         *
         * @param name Name for this instance of the class.
         * @param process The Process class associated with Processor, provided
         *                by the processing framework.
         */
        TrackHitAnaProcessor(const std::string& name, Process& process); 

        /** Destructor */
        ~TrackHitAnaProcessor(); 

        /**
         * @brief Configure the Ana Processor
         * 
         * @param parameters The configuration parameters
         */
        virtual void configure(const ParameterSet& parameters);

        /**
         * @brief Process the event and put new data products into it.
         * 
         * @param event The Event to process.
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

        std::string trkCollName_; //!< Track Collection name

        // Track Selector configuration
        std::string selectionCfg_;
        std::shared_ptr<BaseSelector> trkSelector_; //!< description
        std::vector<std::string> regionSelections_; //!< description

        std::map<std::string, std::shared_ptr<BaseSelector>> reg_selectors_; //!< description

        std::map<std::string, std::shared_ptr<TrackHistos>> reg_histos_; //!< description
        typedef std::map<std::string, std::shared_ptr<TrackHistos>>::iterator reg_it; //!< description

        std::vector<std::string> regions_; //!< description

        // Containers to hold histogrammer info
        std::string histCfgFilename_; //!< description
        std::string truthHistCfgFilename_; //!< description
        TrackHistos* trkHistos_{nullptr}; //!< description
        TrackHistos* truthHistos_{nullptr}; //!< description
        bool doTruth_{false}; //!< description
        int debug_{0}; //!< debug level

}; // TrackHitAnaProcessor

#endif // __TRACKHIT_ANAPROCESSOR_
