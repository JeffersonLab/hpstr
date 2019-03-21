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
#include <EVENT/TrackerRawData.h>
#include <IMPL/LCGenericObjectImpl.h>
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
#include "RawSvtHit.h"

class TTree; 

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
        virtual void initialize(TTree* tree);

        /**
         * Callback for the Processor to take any necessary
         * action when the processing of events finishes.
         */
        virtual void finalize();

    private: 

        /** Container to hold all TrackerHit objects. */
        TClonesArray* rawhits_{nullptr}; 

}; // SvtRawDataProcessor

#endif // __SVT_RAW_DATA_PROCESSOR_H__
