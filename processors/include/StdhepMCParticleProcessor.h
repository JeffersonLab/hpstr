/**
 * @file StdhepMCParticleProcessor.h
 * @brief Processor used to translate StdHep MCParticles to ROOT 
 *      MCParticle objects.
 */

#ifndef __STDHEP_ROOT_CONVERTER_H__
#define __STDHEP_ROOT_CONVERTER_H__

//-----------------//
//   C++  StdLib   //
//-----------------//
#include <iostream>
#include <string>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <memory>
#include <vector>

//----------//
//   LCIO   //
//----------//
#include <EVENT/LCIO.h>
#include <EVENT/LCCollection.h>
#include <IMPL/LCEventImpl.h>
#include <IMPL/MCParticleImpl.h>

#include <UTIL/LCStdHepRdr.h>


//----------//
//   ROOT   //
//----------//
#include "TFile.h"
#include "TTree.h"
//-----------//
//   hpstr   //
//-----------//
#include "Processor.h"
#include "MCParticle.h"

class StdhepMCParticleProcessor : public Processor { 

    public: 

        /** 
         * Class constructor.
         *
         * @param name Name for this instance of the class.
         * @param process the Process class associated with the Processor,
         *      provided by the processing framework.
         */
        StdhepMCParticleProcessor(const std::string& name, Process& process); 

        /** Destructor */
        ~StdhepMCParticleProcessor(); 

        /**
         * Process the event and put new data products into it.
         * @param event The Event to process.
         */
        virtual bool process();
        virtual bool process(IEvent* event) {};

        /**
         * Configure the Processor
         * @param parameters The configuration parameters
         */
        virtual void configure(const ParameterSet& parameters);

        /**
         * Callback for the Processor to take any necessary
         * action when the processing of events starts.
         */
        virtual void initialize(std::string inFilename, std::string outFilename);
        virtual void initialize(TTree* tree) {};

        /**
         * Callback for the Processor to take any necessary
         * action when the processing of events finishes.
         */
        virtual void finalize();


    private: 
        //stdhep input file
        std::string inFilename_;
        //name temporary lcio collection
        std::string mcPartCollStdhep_{"MCParticle"};
        //max stdhep event number to convert
        int maxEvent_{-1};
        //root tuple outfile
        TFile* outF_{nullptr};
        //name root collection
        std::string mcPartCollRoot_{"MCParticle"};
        //list of converted MCParticles
        std::vector<MCParticle*> mc_particles_{};
        //TTree holds converted MCParticles for each event
        TTree* tree_{nullptr};

}; // Tracking Processor

#endif // __STDHEP_ROOT_CONVERTER_H__
