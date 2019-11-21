/**
 *
 */

#ifndef __TRACKER3DHIT_PROCESSOR_H__
#define __TRACKER3DHIT_PROCESSOR_H__

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
#include <EVENT/TrackerHit.h>
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

class Tracker3DHitProcessor : public Processor { 

    public: 

        /**
         * Class constructor. 
         *
         * @param name Name for this instance of the class.
         * @param process The Process class associated with Processor, provided
         *                by the processing framework.
         */
        Tracker3DHitProcessor(const std::string& name, Process& process); 

        /** Destructor */
        ~Tracker3DHitProcessor(); 

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
        std::vector<TrackerHit*> hits_; 


}; // Tracker3DHitProcessor

#endif // __TRACKER3DHIT_PROCESSOR_H__
