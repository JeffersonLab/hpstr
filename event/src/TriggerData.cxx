/**
 * @file TriggerData.h
 * @brief Class used to decode trigger information.
 * @author: Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "TriggerData.h"

TriggerData::TriggerData(EVENT::LCGenericObject* trigger_data) {
    this->parseTriggerData(trigger_data); 
}

void TriggerData::parseTriggerData(EVENT::LCGenericObject* trigger_data) 
{ 
    //std::cout << trigger_data->getNInt() << " VTP words" << std::endl;
    for(int i = 0; i < trigger_data->getNInt(); i++)
    {
        int trigger_data_int = trigger_data->getIntVal(i);
        if(!(trigger_data_int & 1<<31)) continue;
        int type = (trigger_data_int>>27)&0x0F;
        int subtype = -1;
        switch (type)
        {
            case 0:  // Block Header
                //std::cout << i << "VTP Block Header Word" << std::endl;
                break;
            case 1: //  Block Tail
                //std::cout << i << "VTP Block Tail Word" << std::endl;
                break;
            case 2:  // Event Header
                //std::cout << i << "VTP Event Header Word" << std::endl;
                break;
            case 3:  // Trigger time
                time_stamp_ = (trigger_data_int & 0x00FFFFFF) + ((trigger_data->getIntVal(i+1)& 0x00FFFFFF )<<24);
                i++;
                break;
            case 12:  // Expansion type
                subtype = (trigger_data_int>>23)&0x0F;
                switch(subtype){
                    case 2: // HPS Cluster
                        //std::cout << i << "VTP Cluster Word" << std::endl;
                        break;
                    case 3: // HPS Single Cluster
                        //std::cout << i << "VTP Single Cluster Word" << std::endl;
                        break;
                    case 4: // HPS Pair Trigger
                        //std::cout << i << "VTP Pair Trigger Word" << std::endl;
                        break;
                    case 5: // HPS Calibration Trigger
                        //std::cout << i << "VTP Calibration Trigger Word: " << trigger_data_int << std::endl;
                        break;
                    case 6: // HPS Cluster Multiplicity Trigger
                        //std::cout << i << "VTP Cluster Multiplicity Trigger Word" << std::endl;
                        break;
                    case 7: // HPS FEE Trigger
                        //std::cout << i << "VTP FEE Trigger Word" << std::endl;
                        break;
                    default:
                        //std::cout << "At " << i << " invalid HPS type: " << type << " subtype: " << subtype << std::endl;
                        break;
                }

                break;
            case 14:
                //std::cout << i << "VTP data type not valid: " << type << std::endl;
                break;
            default:
                //std::cout << i << "I was not expecting a VTP data type of " << type << std::endl;
                break;
        }
        single0_ = 0;
        single1_ = 0;
        pair0_ = 0;
        pair1_ = 0;
        pulser_ = 0;
    }
}
