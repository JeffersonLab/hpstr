/**
 *
 */

#ifndef __SVT_RAW_DATA_PROCESSOR_H__
#define __SVT_RAW_DATA_PROCESSOR_H__

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
#include <IMPL/LCGenericObjectImpl.h>
#include <UTIL/LCRelationNavigator.h>

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

class SvtRawDataProcessor : public Processor { 

    public: 

        /**
         * Class constructor. 
         *
         * @param name Name for this instance of the class.
         * @param process The Process class associated with Processor, provided
         *                by the processing framework.
         */
        SvtRawDataProcessor(const std::string& name, Process& process); 

        /** Destructor */
        ~SvtRawDataProcessor(); 

        /**
         * Process the event and put new data products into it.
         * @param event The Event to process.
         */
        virtual void process(Event* event);

        /**
         * Callback for the Processor to take any necessary
         * action when the processing of events starts.
         */
        virtual void initialize();

        /**
         * Callback for the Processor to take any necessary
         * action when the processing of events finishes.
         */
        virtual void finalize();

    private: 

        /** Container to hold all TrackerHit objects. */
        TClonesArray* hits_{nullptr}; 

        /** Container to hold all Track objects. */
        TClonesArray* tracks_{nullptr};


}; // SvtRawDataProcessor

#endif // __SVT_RAW_DATA_PROCESSOR_H__
