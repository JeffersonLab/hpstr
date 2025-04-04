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
#include <algorithm>
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

/**
 * @brief Processor used to write event info.
 * more details
 */
class EventProcessor : public Processor { 

    public: 

        /**
         * @brief Class constructor. 
         *
         * @param name Name for this instance of the class.
         * @param process The Process class associated with Processor, provided
         *                by the processing framework.
         */
        EventProcessor(const std::string& name, Process& process); 

        /** Destructor */
        ~EventProcessor(); 

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

        /** Containers for event header */
        EventHeader* header_{nullptr};
        std::string  headCollRoot_{"EventHeader"}; //!< description
        std::string  rfCollLcio_{"RFHits"}; //!< description
        std::string  trigCollLcio_{"TriggerBank"}; //!< description

        /** Containers for vtp data */
        VTPData* vtpData{nullptr};
        std::string vtpCollLcio_{""}; //!< description
        std::string vtpCollRoot_{""}; //!< description

        /** Containers for ts data */
        TSData* tsData{nullptr};
        std::string tsCollLcio_{"TSBank"}; //!< description
        std::string tsCollRoot_{"TSBank"}; //!< description

        /** Parsing method */
        void parseVTPData(EVENT::LCGenericObject* vtp_data_lcio);
        /** Parsing method */
        void parseTSData(EVENT::LCGenericObject* ts_data_lcio);

        /** single events checks */
        std::string run_evt_list_{""};
        std::map<int,std::vector<int >> run_evts_map_; //!< description

        int debug_{0}; //!< Debug Level
	int year_{2021};

}; // EventProcessor

#endif // _EVENT_HEADER_PROCESSOR_H__
