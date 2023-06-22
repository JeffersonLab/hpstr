#ifndef __TRACKING_ANAPROCESSOR_H__
#define __TRACKING_ANAPROCESSOR_H__

//-----------------//
//   C++  StdLib   //
//-----------------//
#include <iostream>
#include <string>
#include <iostream>
#include <fstream>

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
#include "TrackHistos.h"
#include "TrackerHit.h"

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

        std::string trkCollName_; //!< Track Collection name

        /** Container to hold strip clusters. */
        std::string hitColl_{""};
        std::vector<TrackerHit*>* hits_{};
        TBranch* bhits_{nullptr};

        // Track Selector configuration
        std::string selectionCfg_;
        std::shared_ptr<BaseSelector> trkSelector_; //!< description

        // Beampos corrections
        std::string beamPosCfg_{""}; //!< json containing run dep beamspot positions
        json bpc_configs_; //!< json object
        std::vector<double> beamPosCorrections_ = {0.0,0.0,0.0}; //!< holds beam position corrections
        int run_number_ = -999;

        // Containers to hold histogrammer info
        std::string histCfgFilename_; //!< description
        std::string truthHistCfgFilename_; //!< description
        TrackHistos* trkHistos_{nullptr}; //!< description
        TrackHistos* truthHistos_{nullptr}; //!< description
        bool doTruth_{false}; //!< description

        int debug_{0}; //!< debug level

}; // TrackingAnaProcessor

#endif // __TRACKING_ANAPROCESSOR_
