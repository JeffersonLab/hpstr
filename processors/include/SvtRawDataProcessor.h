#ifndef __SVT_RAW_DATA_PROCESSOR_H__
#define __SVT_RAW_DATA_PROCESSOR_H__

//-----------------//
//   C++  StdLib   //
//-----------------//
#include <iostream>
#include <algorithm>
#include <string>

//----------//
//   LCIO   //
//----------//
#include <EVENT/LCCollection.h>
#include <EVENT/TrackerRawData.h>
#include <IMPL/LCGenericObjectImpl.h>
#include <UTIL/BitField64.h>
#include <IMPL/LCGenericObjectImpl.h>
#include <UTIL/LCRelationNavigator.h>

//----------//
//   ROOT   //
//----------//
#include "TClonesArray.h"

//-----------//
//   hpstr   //
//-----------//
#include "Collections.h"
#include "Processor.h"
#include "RawSvtHit.h"
#include "Event.h"

class TTree; 

class SvtRawDataProcessor : public Processor { 

    public:
        /**
         * @brief Class constructor. 
         *
         * @param name Name for this instance of the class.
         * @param process The Process class associated with Processor, provided
         *                by the processing framework.
         */
        SvtRawDataProcessor(const std::string& name, Process& process); 

        /** Destructor */
        ~SvtRawDataProcessor(); 

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
        std::vector<RawSvtHit*> rawhits_; //!< Container to hold all TrackerHit objects.
        std::string hitCollLcio_{"SVTRawTrackerHits"}; //!< collection name
        std::string hitfitCollLcio_{"SVTFittedRawTrackerHits"}; //!< collection name
        std::string hitCollRoot_{"SVTRawTrackerHits"}; //!< collection name
        
        int debug_{0}; //!< Debug Level

}; // SvtRawDataProcessor

#endif // __SVT_RAW_DATA_PROCESSOR_H__
