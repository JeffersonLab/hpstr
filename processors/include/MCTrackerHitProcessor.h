/**
 *
 */

#ifndef __MCTRACKERHIT_PROCESSOR_H__
#define __MCTRACKERHIT_PROCESSOR_H__

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
#include <EVENT/SimTrackerHit.h>
#include <EVENT/MCParticle.h>
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
#include "MCTrackerHit.h"
#include "Event.h"

class MCTrackerHitProcessor : public Processor { 

    public: 

        /**
         * Class constructor. 
         *
         * @param name Name for this instance of the class.
         * @param process The Process class associated with Processor, provided
         *                by the processing framework.
         */
        MCTrackerHitProcessor(const std::string& name, Process& process); 

        /** Destructor */
        ~MCTrackerHitProcessor(); 

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

        /** Containers to hold all TrackerHit objects, and collection names. */
        std::vector<MCTrackerHit*> trackerhits_; 
        std::string   hitCollLcio_{"TrackerHits"};
        std::string   hitCollRoot_{"TrackerHits"};

        //Debug Level
        int debug_{0};

}; // MCTrackerHitProcessor

#endif // __MCTRACKERHIT_PROCESSOR_H__
