/**
 * @file ParticleProcessor.h
 * @brief Processor used to translate LCIO ReconstructedParticles to DST 
 *        Particle objects.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef _PARTICLE_PROCESSOR_H_
#define _PARTICLE_PROCESSOR_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <iostream>
#include <map>
#include <vector>
#include <string>

//----------//
//   LCIO   //
//----------//
#include <EVENT/LCCollection.h>
#include <EVENT/ReconstructedParticle.h>
#include <EVENT/Vertex.h>
//#include <Exceptions.h>

//----------//
//   ROOT   //
//----------//
#include "TClonesArray.h"

//-----------//
//   hpstr   //
//-----------//
#include "Collections.h"
#include "Particle.h"
#include "Processor.h"
#include "Track.h"

class ParticleProcessor : public Processor { 

    public: 
        
        /**
         * Class constructor. 
         *
         * @param name Name for this instance of the class.
         * @param process The Process class associated with Processor, provided
         *                by the processing framework.
         */
        ParticleProcessor(const std::string& name, Process& process); 

        /** Destructor */
        ~ParticleProcessor();

        /**
         * Callback for the EventProcessor to configure itself from the given set of parameters.
         * @param parameters ParameterSet for configuration.
         */
        virtual void configure(const ParameterSet& parameters);
         
        /**
         * Callback for the Processor to take any necessary
         * action when the processing of events starts.
         */
        virtual void initialize();

        /**
         * Process the event and put new data products into it.
         * @param event The Event to process.
         */
        virtual void process(Event* event);

        /**
         * Callback for the Processor to take any necessary
         * action when the processing of events finishes.
         */
        virtual void finalize();

    private:

        /** Map to hold all particle collections. */
        std::map<std::string, TClonesArray*> collections_; 

}; // ParticleProcessor

#endif // _PARTICLE_PROCESSOR_H_
