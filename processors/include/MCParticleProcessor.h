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

/**
 * @brief Processor used to translate LCIO MCParticles to DST MCParticle objects.
 * more details
 */
class MCParticleProcessor : public Processor { 

    public: 
        
        /**
         * @brief Class constructor. 
         *
         * @param name Name for this instance of the class.
         * @param process The Process class associated with Processor, provided
         *                by the processing framework.
         */
        MCParticleProcessor(const std::string& name, Process& process); 

        /** Destructor */
        ~MCParticleProcessor();

        /**
         * @brief Callback for the Processor to configure itself from the given set of parameters.
         * 
         * @param parameters ParameterSet for configuration.
         */
        virtual void configure(const ParameterSet& parameters);
         
        /**
         * @brief Callback for the Processor to take any necessary
         *        action when the processing of events starts.
         * 
         * @param tree
         */
        virtual void initialize(TTree* tree);

        /**
         * @brief Process the event and put new data products into it.
         * 
         * @param event The Event to process.
         */
        virtual bool process(IEvent* ievent);

        /**
         * @brief Callback for the Processor to take any necessary
         *        action when the processing of events finishes.
         */
        virtual void finalize();

    private:

        /** Map to hold all particle collections. */
        std::vector<MCParticle*> mc_particles_{}; 
        std::string mcPartCollLcio_{"MCParticle"}; //!< description
        std::string mcPartCollRoot_{"MCParticle"}; //!< description

        int debug_{0}; //!< Debug level


}; // MCParticleProcessor

#endif // _MCPARTICLE_PROCESSOR_H_
