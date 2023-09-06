/**
 *
 */

#ifndef __TRACKER2DHIT_PROCESSOR_H__
#define __TRACKER2DHIT_PROCESSOR_H__

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
#include <IMPL/MCParticleImpl.h>
#include <IMPL/SimTrackerHitImpl.h>
#include <UTIL/LCRelationNavigator.h>

//-----------//
//   hpstr   //
//-----------//
#include "Collections.h"
#include "Processor.h"
#include "Track.h"
#include "RawSvtHit.h"
#include "TrackerHit.h"
#include "Event.h"

// Forward declarations
class TTree; 

class Tracker2DHitProcessor : public Processor { 

    public: 

        /**
         * Class constructor. 
         *
         * @param name Name for this instance of the class.
         * @param process The Process class associated with Processor, provided
         *                by the processing framework.
         */
        Tracker2DHitProcessor(const std::string& name, Process& process); 

        /** Destructor */
        ~Tracker2DHitProcessor(); 

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

        /** Container to hold all TrackerHit objects. */
        std::vector<TrackerHit*> hits_; 
        std::string hitCollLcio_{"RotatedHelicalTrackHits"};
        std::string hitCollRoot_{"RotatedHelicalTrackHits"};

        std::vector<RawSvtHit*> rawhits_{};
        std::string rawhitCollRoot_{"fspOnTrackRawHits"};
        std::string hitFitsCollLcio_{"SVTFittedRawTrackerHits"};

        std::string mcPartRelLcio_{"RotatedHelicalTrackMCRelations"};


        //Debug Level
        int debug_{0};

}; // Tracker2DHitProcessor

#endif // __TRACKER2DHIT_PROCESSOR_H__
