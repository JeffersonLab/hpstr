/**
 *
 */

#ifndef __TRACKING_PROCESSOR_H__
#define __TRACKING_PROCESSOR_H__

//-----------------//
//   C++  StdLib   //
//-----------------//
#include <iostream>
#include <string>

//----------//
//   LCIO   //
//----------//
#include <EVENT/LCCollection.h>
#include <EVENT/Track.h>
#include <EVENT/TrackerHit.h>
#include <EVENT/TrackState.h>
#include <EVENT/TrackerRawData.h>
#include <IMPL/LCGenericObjectImpl.h>
#include <IMPL/TrackerHitImpl.h>
#include <UTIL/LCRelationNavigator.h>
#include <UTIL/BitField64.h>

//----------//
//   ROOT   //
//----------//
#include "TClonesArray.h"

//-----------//
//   hpstr   //
//-----------//
#include "Collections.h"
#include "Processor.h"
#include "Track.h"
#include "TrackerHit.h"
#include "Event.h"
#include "RawSvtHit.h"

// Forward declarations
class TTree; 

class TrackingProcessor : public Processor { 

    public: 

        /**
         * Class constructor. 
         *
         * @param name Name for this instance of the class.
         * @param process The Process class associated with Processor, provided
         *                by the processing framework.
         */
        TrackingProcessor(const std::string& name, Process& process); 

        /** Destructor */
        ~TrackingProcessor(); 

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

        /** Container to hold all TrackerHit objects. */
        std::vector<TrackerHit*> hits_{}; 

        /** Container to hold all Track objects. */
        std::vector<Track*> tracks_{};

        /** Container to hold all raw hits objecs. */
        std::vector<RawSvtHit*> rawhits_{};
        bool _debug{false};





}; // Tracking Processor

#endif // __TRACKING_PROCESSOR_H__
