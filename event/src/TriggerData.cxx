/**
 * @file TriggerData.h
 * @brief Class used to decode trigger information.
 * @author: Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "TriggerData.h"

TriggerData::TriggerData(EVENT::LCGenericObject* vtp_data, EVENT::LCGenericObject* ts_data) {
    this->parseTriggerData(vtp_data, ts_data); 
}

void TriggerData::parseTriggerData(EVENT::LCGenericObject* vtp_data, EVENT::LCGenericObject* ts_data) 
{ 
    vtpData = new VTPData(vtp_data);
}
