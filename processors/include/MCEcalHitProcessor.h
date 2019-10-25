/**
 * @file MCEcalHitProcessor.h
 * @brief Processor used to add simulated ecal hits to the event
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 */

#ifndef __MCECALHIT_PROCESSOR_H__
#define __MCECALHIT_PROCESSOR_H__

//-----------------//
//   C++  StdLib   //
//-----------------//
#include <iostream>
#include <algorithm>
#include <string>

//----------//
//   LCIO   //
//----------//
#include <EVENT/LCCollection.h>
#include <EVENT/SimCalorimeterHit.h>
#include <UTIL/BitField64.h>

//----------//
//   ROOT   //
//----------//
#include "TClonesArray.h"
#include "TTree.h"

//-----------//
//   hpstr   //
//-----------//
#include "Collections.h"
#include "Processor.h"
#include "MCEcalHit.h"
#include "Event.h"

class MCEcalHitProcessor : public Processor { 

    public: 

        /**
         * Class constructor. 
         *
         * @param name Name for this instance of the class.
         * @param process The Process class associated with Processor, provided
         *                by the processing framework.
         */
        MCEcalHitProcessor(const std::string& name, Process& process); 

        /** Destructor */
        ~MCEcalHitProcessor(); 

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

        /** Container to hold all MCEcalHit objects. */
        TClonesArray* ecalhits_{nullptr}; 

}; // MCEcalHitProcessor

#endif // __MCECALHIT_PROCESSOR_H__
