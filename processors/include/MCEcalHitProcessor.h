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
         * Callback for the Processor to configure itself from the given set of parameters.
         * @param parameters ParameterSet for configuration.
         */
        virtual void configure(const ParameterSet& parameters);

        /**
         * Callback for the Processor to take any necessary
         * action when the processing of events starts.
         */
        virtual void initialize(TTree* tree);

        /**
         * Process the event and put new data products into it.
         * @param event The Event to process.
         */
        virtual bool process(IEvent* ievent);

        /**
         * Callback for the Processor to take any necessary
         * action when the processing of events finishes.
         */
        virtual void finalize();

    private: 

        /** Container to hold all MCEcalHit objects. */
        std::vector<MCEcalHit*> ecalhits_; 
        std::string   hitCollLcio_{"EcalHits"};
        std::string   hitCollRoot_{"EcalHits"};

        //Debug Level
        int debug_{0};

}; // MCEcalHitProcessor

#endif // __MCECALHIT_PROCESSOR_H__
