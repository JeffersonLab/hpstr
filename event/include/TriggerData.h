/**
 * @file TriggerData.h
 * @brief Class used to decode TI information.
 * @author: Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef _TRIGGER_DATA_H_
#define _TRIGGER_DATA_H_

//----------//
//  event   //
//----------//
#include "VTPData.h"

//----------//
//   LCIO   //
//----------//
#include <EVENT/LCGenericObject.h>

class TriggerData { 
    
    public: 

        /**
         * Constructor
         *
         * @param trigger_data : The LCGenericObeject that is being used to 
         *                       store the data from the TI
         */
        TriggerData(){};
        TriggerData(EVENT::LCGenericObject* vtp_data, EVENT::LCGenericObject* ts_data);

        /** @return The parsed VTP data. */
        VTPData* getVTPData() const { return vtpData; };

    private:

        /** Private method used to decode all trigger information. */
        void parseTriggerData(EVENT::LCGenericObject* vtp_data, EVENT::LCGenericObject* ts_data);  

        /** Trigger time stamp. */
        long time_stamp_{-9999}; 

        /** VTP data parser. */
        VTPData * vtpData{nullptr}; 

};

#endif // __TRIGGER_DATA_H__
