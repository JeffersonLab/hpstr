/**
 *
 */

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
#include "Track.h"
#include "Event.h"
#include "TrackHistos.h"

// Forward declarations
class TTree; 

class TrackingAnaProcessor : public Processor { 

    public: 

        /**
         * Class constructor. 
         *
         * @param name Name for this instance of the class.
         * @param process The Process class associated with Processor, provided
         *                by the processing framework.
         */
        TrackingAnaProcessor(const std::string& name, Process& process); 

        /** Destructor */
        ~TrackingAnaProcessor(); 

        /**
         * Configure the Ana Processor
         * @param parameters The configuration parameters
         */
        virtual void configure(const ParameterSet& parameters);

        /**
         * Process the event and put new data products into it.
         * @param event The Event to process.
         */
        virtual bool process(IEvent* ievent);

        /**
         * Callback for the Processor to take any necessary
         * action when the processing of events starts.
         */
        virtual void initialize(TTree* tree);

        /**
         * Callback for the Processor to take any necessary
         * action when the processing of events finishes.
         */
        virtual void finalize();

    private: 

        /** Container to hold all Track objects. */
        std::vector<Track*> * tracks_{};
        TBranch*           btracks_{nullptr};

        // Track Collection name
        std::string trkCollName_;

        // Containers to hold histogrammer info
        std::string histCfgFilename_;
        std::string truthHistCfgFilename_;
        TrackHistos* trkHistos_{nullptr};
        TrackHistos* truthHistos_{nullptr};
        bool doTruth_{false};
        int debug_{0};



}; // TrackingAnaProcessor

#endif // __TRACKING_ANAPROCESSOR_
