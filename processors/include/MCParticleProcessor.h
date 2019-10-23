/**
 * @file MCParticleProcessor.h
 * @brief Processor used to translate LCIO MCParticles to DST 
 *        MCParticle objects.
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 */

#ifndef _MCPARTICLE_PROCESSOR_H_
#define _MCPARTICLE_PROCESSOR_H_

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
#include <EVENT/LCIO.h>
#include <EVENT/LCCollection.h>
#include <IMPL/MCParticleImpl.h>
//#include <Exceptions.h>

//----------//
//   ROOT   //
//----------//
#include "TClonesArray.h"
#include "TTree.h"

//-----------//
//   hpstr   //
//-----------//
#include "CalCluster.h"
#include "Collections.h"
#include "MCParticle.h"
#include "Processor.h"
#include "Track.h"
#include "Event.h"

class MCParticleProcessor : public Processor { 

    public: 
        
        /**
         * Class constructor. 
         *
         * @param name Name for this instance of the class.
         * @param process The Process class associated with Processor, provided
         *                by the processing framework.
         */
        MCParticleProcessor(const std::string& name, Process& process); 

        /** Destructor */
        ~MCParticleProcessor();

        /**
         * Callback for the EventProcessor to configure itself from the given set of parameters.
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

        /** Map to hold all particle collections. */
        TClonesArray* mc_particles_; 

}; // MCParticleProcessor

#endif // _MCPARTICLE_PROCESSOR_H_
