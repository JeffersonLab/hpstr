/**
 * @file TriggerData.h
 * @brief Class used to decode TI information.
 * @author: Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef _TRIGGER_DATA_H_
#define _TRIGGER_DATA_H_

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
        TriggerData(EVENT::LCGenericObject* trigger_data);

        /** @return The trigger time. */
        double getTime() const { return time_stamp_; };

        /** @return True if the event registered a single0 trigger. */
        bool isSingle0Trigger() const { return single0_; };

        /** @return True if the event registered a single1 trigger. */
        bool isSingle1Trigger() const { return single1_; };

        /** @return True if the event registered a pair0 trigger. */
        bool isPair0Trigger() const { return pair0_; };
        
        /** @return True if the event registered a pair1 trigger. */
        bool isPair1Trigger() const { return pair1_; };

        /** @return True if the event registered a pulser trigger. */
        bool isPulserTrigger() const { return pulser_; };

    private:

        /** Private method used to decode all trigger information. */
        void parseTriggerData(EVENT::LCGenericObject* trigger_data);  

        /** Trigger time stamp. */
        long time_stamp_{-9999}; 

        /** Flag indicating whether a single0 trigger was registered. */
        bool single0_{0};
        
        /** Flag indicating whether a single1 trigger was registered. */
        bool single1_{0};
        
        /** Flag indicating whether a pair0 trigger was registered. */
        bool pair0_{0};
        
        /** Flag indicating whether a pair1 trigger was registered. */
        bool pair1_{0}; 
        
        /** Flag indicating whether a pulser trigger was registered. */
        bool pulser_{0}; 
};

#endif // __TRIGGER_DATA_H__
