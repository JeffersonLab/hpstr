/**
 * @file EventFile.h
 * @brief Class for managing io files.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef __EVENT_FILE_H__
#define __EVENT_FILE_H__

//----------//
//   ROOT   //
//----------//
#include <TFile.h>
#include <TTree.h>

//----------//
//   LCIO   //
//----------//
#include <IO/LCReader.h>
#include <IOIMPL/LCFactory.h>
#include <EVENT/LCEvent.h>

//-----------//
//   hpstr   //
//-----------//
#include "Event.h"
#include "IEventFile.h"

class EventFile : public IEventFile {

    public: 

        /** Constructor */
        EventFile(const std::string ifilename, const std::string& ofilename);

        //TODO CHECK THIS
        /** Destructor */
        virtual ~EventFile();

        /**
         * Load the next event in the file. 
         *
         * @return true if an event was loaded successfully, false otherwise 
         */
        virtual bool nextEvent();

        /**
         * Persists the event
         *
         */
        virtual void FillEvent();

        /**
         * Setup the event object that will be used by this file.
         *
         * @param event The Event container.
         */
        void setupEvent(IEvent* ievent);

        /** 
         * Close the file, writing the tree to disk if creating an output file.
         */
        void close();

  /** 
   * Get output file directory
   */
  void resetOutputFileDir();

    private: 

        /** The ROOT file to which event data will be written to. */
        TFile* ofile_{nullptr}; 

        /** Additional output files, i.e. for histogramming */
        //std::vector<TFile* > outputs_f;
     
        /** Object used to build the HPS event model. */
        Event* event_{nullptr};

        /** Object used to load all of current LCIO event information. */
        EVENT::LCEvent* lc_event_{nullptr}; 
        
        /** LCIO reader */
        IO::LCReader* lc_reader_{IOIMPL::LCFactory::getInstance()->createLCReader()}; 

        int entry_{0};  

}; // EventFile

#endif // __EVENT_FILE_H__
