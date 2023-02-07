/**
 * @file StdhepMCParticleProcessor.h
 * @brief Processor used to translate StdHep MCParticles to ROOT 
 *        MCParticle objects.
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

/**
 * @brief Processor used to translate StdHep MCParticles to ROOT MCParticle objects.
 * more details
 */
class StdhepMCParticleProcessor : public Processor { 

    public: 

        /** 
         * @brief Class constructor.
         *
         * @param name Name for this instance of the class.
         * @param process The Process class associated with the Processor,
         *                provided by the processing framework.
         */
        StdhepMCParticleProcessor(const std::string& name, Process& process); 

        /** Destructor */
        ~StdhepMCParticleProcessor(); 

        /**
         * @brief Process the event and put new data products into it.
         * 
         * @param event The Event to process.
         */
        virtual bool process();
        virtual bool process(IEvent* event) {};

        /**
         * @brief Configure the Processor
         * 
         * @param parameters The configuration parameters
         */
        virtual void configure(const ParameterSet& parameters);

        /**
         * @brief Callback for the Processor to take any necessary
         *        action when the processing of events starts.
         * 
         * @param inFilename 
         * @param outFilename 
         */
        virtual void initialize(std::string inFilename, std::string outFilename);

        /**
         * @brief Callback for the Processor to take any necessary
         *        action when the processing of events starts.
         * 
         * @param tree 
         */
        virtual void initialize(TTree* tree) {};

        /**
         * @brief Callback for the Processor to take any necessary
         *        action when the processing of events finishes.
         */
        virtual void finalize();


    private: 
        std::string inFilename_; //!< stdhep input file
        std::string mcPartCollStdhep_{"MCParticle"}; //!< name temporary lcio collection
        int maxEvent_{-1}; //!< max stdhep event number to convert
        int skipEvent_{0};//!< skipped event numbers to convet
        TFile* outF_{nullptr}; //!< root tuple outfile
        std::string mcPartCollRoot_{"MCParticle"}; //!< name root collection
        std::vector<MCParticle*> mc_particles_{}; //!< list of converted MCParticles
        TTree* tree_{nullptr}; //!< TTree holds converted MCParticles for each event

}; // Tracking Processor

#endif // __STDHEP_ROOT_CONVERTER_H__
