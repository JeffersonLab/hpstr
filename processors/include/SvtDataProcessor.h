#ifndef __SVT_DATA_PROCESSOR_H__
#define __SVT_DATA_PROCESSOR_H__

//-----------------//
//   C++  StdLib   //
//-----------------//
#include <iostream>
#include <string>
#include <vector>

//----------//
//   LCIO   //
//----------//
#include <EVENT/LCCollection.h>
#include <EVENT/Track.h>
#include <EVENT/TrackerHit.h>
#include <EVENT/TrackState.h>
#include <IMPL/LCGenericObjectImpl.h>
#include <IMPL/TrackerHitImpl.h>
#include <UTIL/LCRelationNavigator.h>

//-----------//
//   hpstr   //
//-----------//
#include "Collections.h"
#include "Processor.h"
#include "Track.h"
#include "TrackerHit.h"
#include "Event.h"

// Forward declarations
class TTree; 

/**
 * @brief Insert description here.
 * more details
 */
class SvtDataProcessor : public Processor { 

    public: 

        /**
         * @brief Class constructor. 
         *
         * @param name Name for this instance of the class.
         * @param process The Process class associated with Processor, provided
         *                by the processing framework.
         */
        SvtDataProcessor(const std::string& name, Process& process); 

        /** Destructor */
        ~SvtDataProcessor(); 

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

        /** Container to hold all TrackerHit objects. */
        std::vector<TrackerHit*> hits_; 

        /** Container to hold all Track objects. */
        std::vector<Track*> tracks_;


}; // SvtDataProcessor

#endif // __SVT_DATA_PROCESSOR_H__
