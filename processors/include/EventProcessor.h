/**
 * @file EventProcessor.h
 * @brief Processor used to write event info.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 * @author PF, SLAC National Accelerator Laboratory
 */

#ifndef _EVENT_HEADER_PROCESSOR_H__
#define _EVENT_HEADER_PROCESSOR_H__

//-----------------//
//   C++  StdLib   //
//-----------------//
#include <string>
#include <iostream>
#include <fstream>

//----------//
//   LCIO   //
//----------//
#include <EVENT/LCGenericObject.h>
#include <EVENT/LCCollection.h>

//----------//
//   ROOT   //
//----------//
#include "TClonesArray.h"

//-----------//
//   hpstr   //
//-----------//
#include "Collections.h"
#include "EventHeader.h"
#include "Processor.h"
#include "VTPData.h"
#include "TSData.h"
#include "TriggerData.h"
#include "Event.h"

// Forward declarations
class TTree; 

class EventProcessor : public Processor { 

    public: 

        /**
         * Class constructor. 
         *
         * @param name Name for this instance of the class.
         * @param process The Process class associated with Processor, provided
         *                by the processing framework.
         */
        EventProcessor(const std::string& name, Process& process); 

        /** Destructor */
        ~EventProcessor(); 

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

        //Containers for event header
        EventHeader* header_{nullptr};
        std::string  headCollRoot_{"EventHeader"};
        std::string  rfCollLcio_{"RFHits"};
        std::string  trigCollLcio_{"TriggerBank"};

        //Containers for vtp data
        VTPData* vtpData{nullptr};
        std::string vtpCollLcio_{"VTPBank"};
        std::string vtpCollRoot_{"VTPBank"};

        //Containers for ts data
        TSData* tsData{nullptr};
        std::string tsCollLcio_{"TSBank"};
        std::string tsCollRoot_{"TSBank"};

        //Parsing methods
        void parseVTPData(EVENT::LCGenericObject* vtp_data_lcio);
        void parseTSData(EVENT::LCGenericObject* ts_data_lcio);

        //single events checks
        std::string run_evt_list_{""};
        std::map<int,std::vector<int > > run_evts_map_;
        
        //Debug Level
        int debug_{0};

}; // EventProcessor

#endif // _EVENT_HEADER_PROCESSOR_H__
