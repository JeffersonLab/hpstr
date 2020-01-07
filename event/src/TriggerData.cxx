/**
 * @file TriggerData.h
 * @brief Class used to decode trigger information.
 * @author: Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "TriggerData.h"

TriggerData::TriggerData(EVENT::LCGenericObject* trigger_data) {
    this->parseTriggerData(trigger_data); 
}

void TriggerData::parseTriggerData(EVENT::LCGenericObject* trigger_data) { 

    int trigger_data_int = trigger_data->getIntVal(1);
    single0_ = ((trigger_data_int >> 24) & 1) == 1;
    single1_ = ((trigger_data_int >> 25) & 1) == 1;
    pair0_ = ((trigger_data_int >> 26) & 1) == 1;
    pair1_ = ((trigger_data_int >> 27) & 1) == 1;
    pulser_ = ((trigger_data_int >> 29) & 1) == 1;

    trigger_data_int = trigger_data->getIntVal(3);
    long w1 = trigger_data_int & 0xffffffffL;
    trigger_data_int = trigger_data->getIntVal(4);
    long w2 = trigger_data_int & 0xffffffffL;

    long timelo = w1;
    long timehi = (w2 & 0xffff) << 32;

    time_stamp_ = 4 * (timelo + timehi); 
}
