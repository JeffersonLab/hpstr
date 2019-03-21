/**
 * @file EventProcessor.h
 * @brief Processor used to write event info.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef _EVENT_HEADER_PROCESSOR_H__
#define _EVENT_HEADER_PROCESSOR_H__

//-----------------//
//   C++  StdLib   //
//-----------------//
#include <string>
#include <iostream>

//----------//
//   LCIO   //
//----------//
#include <EVENT/LCGenericObject.h>
#include <EVENT/LCCollection.h>

//----------//
//   ROOT   //
//----------//
#include "TClonesArray.h"

//-----------//
//   hpstr   //
//-----------//
#include "Collections.h"
#include "EventHeader.h"
#include "Processor.h"
#include "TriggerData.h"

// Forward declarations
class TTree; 

class EventProcessor : public Processor { 

    public: 

        /**
         * Class constructor. 
         *
         * @param name Name for this instance of the class.
         * @param process The Process class associated with Processor, provided
         *                by the processing framework.
         */
        EventProcessor(const std::string& name, Process& process); 

        /** Destructor */
        ~EventProcessor(); 

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

       TClonesArray* header_{nullptr};


}; // EventProcessor

#endif // _EVENT_HEADER_PROCESSOR_H__
