/**
 * @file VTPData.cxx
 * @brief Class used to decode VTP words.
 * @author: Cameron Bravo, SLAC National Accelerator Laboratory
 */

#include "VTPData.h"

VTPData::VTPData(EVENT::LCGenericObject* vtp_data) {
    this->parseVTPData(vtp_data); 
}

void VTPData::parseVTPData(EVENT::LCGenericObject* vtp_data)
{ 
    // First Clear out all the old data.
    Clear();
    for(int i=0; i<vtp_data->getNInt(); ++i)
    {
        int data = vtp_data->getIntVal(i);
        if(!(data & 1<<31)) continue;
        int type = (data>>27)&0x0F;
        int subtype;
        switch (type)
        {
            case 0:  // Block Header
                ((unsigned int *)&blockHeader)[0] = data;
                break;
            case 1: //  Block Tail
                ((unsigned int *)&blockTail)[0] = data;
                break;
            case 2:  // Event Header
                ((unsigned int *)&eventHeader)[0] = data;
                break;
            case 3:  // Trigger time
                trigTime = (data & 0x00FFFFFF) + ((vtp_data->getIntVal(i+1)& 0x00FFFFFF )<<24);
                i++;
                break;
            case 12:  // Expansion type
                subtype = (data>>23)&0x0F;
                switch(subtype){
                    case 2: // HPS Cluster
                        hpsCluster  clus;
                        ((unsigned long *)&clus)[0] = ((long)data + (((long)vtp_data->getIntVal(i+1))<<32) ); 
                        clusters.push_back(clus);
                        break;
                    case 3: // HPS Single Cluster
                        hpsSingleTrig strig;
                        ((unsigned long *)&strig)[0] = data;
                        singletrigs.push_back(strig);
                        break;
                    case 4: // HPS Pair Trigger
                        hpsPairTrig ptrig;
                        ((unsigned long *)&ptrig)[0] = data;
                        pairtrigs.push_back(ptrig);
                        break;
                    case 5: // HPS Calibration Trigger
                        hpsCalibTrig ctrig;
                        ((unsigned long *)&ctrig)[0] = data;
                        calibtrigs.push_back(ctrig);
                        break;
                    case 6: // HPS Cluster Multiplicity Trigger
                        hpsClusterMult clmul;
                        ((unsigned long *)&clmul)[0] = data;
                        clustermult.push_back(clmul);
                        break;
                    case 7: // HPS FEE Trigger
                        hpsFEETrig fee;
                        ((unsigned long *)&fee)[0] = data;
                        feetrigger.push_back(fee);
                        break;
                    default:
                        std::cout << "At " << i << " invalid HPS type: " << type << " subtype: " << subtype << std::endl;
                        break;
                }

                break;
            case 14:
                std::cout << i << "VTP data type not valid: " << type << std::endl;
                break;
            default:
                std::cout << i << "I was not expecting a VTP data type of " << type << std::endl;
                break;
        }
    }
}
