/**
 *
 */

#ifndef __FINALSTATEPARTICLE_PROCESSOR_H__
#define __FINALSTATEPARTICLE_PROCESSOR_H__

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
#include <EVENT/ReconstructedParticle.h>
#include <IMPL/LCGenericObjectImpl.h>
#include <UTIL/LCRelationNavigator.h>

//-----------//
//   hpstr   //
//-----------//
#include "Processor.h"
#include "Particle.h"
#include "Event.h"

// Forward declarations
class TTree; 

class FinalStateParticleProcessor : public Processor { 

    public: 

        /**
         * Class constructor. 
         *
         * @param name Name for this instance of the class.
         * @param process The Process class associated with Processor, provided
         *                by the processing framework.
         */
        FinalStateParticleProcessor(const std::string& name, Process& process); 

        /** Destructor */
        ~FinalStateParticleProcessor(); 

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

        /** Containers to hold all TrackerHit objects. */
        std::vector<Particle*>   fsps_{}; 
        std::string fspCollLcio_{"FinalStateParticles"};
        std::string fspCollRoot_{"FinalStateParticles"};
        std::string kinkRelCollLcio_{"GBLKinkDataRelations"};
        std::string trkRelCollLcio_{"TrackDataRelations"};

        //Debug Level
        int debug_{0};

}; // FinalStateParticleProcessor

#endif // __FINALSTATEPARTICLE_PROCESSOR_H__
