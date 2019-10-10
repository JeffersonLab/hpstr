/**
 * @file TriggerData.h
 * @brief Class used to decode VTP and TS information.
 * @author: Omar Moreno, SLAC National Accelerator Laboratory
 * @author: Cameron Bravo, SLAC National Accelerator Laboratory
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

//----------//
//   ROOT   //
//----------//
#include "TObject.h"

class TriggerData : public TObject { 
    
    public: 

        // Constructor
        TriggerData();

        /**
         * Constructor
         *
         * @param trigger_data : The LCGenericObeject that is being used to 
         *                       store the data from the TI
         */
        TriggerData(EVENT::LCGenericObject* vtp_data, EVENT::LCGenericObject* ts_data);

        // Destructor
        ~TriggerData();

        //Reset the trigger object
        void Clear();

        /** VTP data parser. */
        VTPData * vtpData{nullptr}; 


        ClassDef(TriggerData, 1);

    private:

        /** Private method used to decode all trigger information. */
        void parseTriggerData(EVENT::LCGenericObject* vtp_data, EVENT::LCGenericObject* ts_data);  

        /** Trigger time stamp. */
        long time_stamp_{-9999}; 

};

#endif // __TRIGGER_DATA_H__
