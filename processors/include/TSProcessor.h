#ifndef _TS_PROCESSOR_H__
#define _TS_PROCESSOR_H__

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
#include "Processor.h"
#include "TSData.h"
#include "Event.h"

// Forward declarations
class TTree;

class TSProcessor : public Processor {

    public:

        /**
         * Class constructor.
         *
         * @param name Name for this instance of the class.
         * @param process The Process class associated with Processor, provided
         *                by the processing framework.
         */
		TSProcessor(const std::string& name, Process& process);

        /** Destructor */
        ~TSProcessor();

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
        //Containers for ts data
        TSData* tsData{nullptr};
        std::string tsCollLcio_{"TSBank"};
        std::string tsCollRoot_{"TSBank"};

        //Parsing methods
        void parseTSData(EVENT::LCGenericObject* ts_data_lcio);


        //Debug Level
        int debug_{0};

}; // TSProcessor

#endif // _TS_PROCESSOR_H__
