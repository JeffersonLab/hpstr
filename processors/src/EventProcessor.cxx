/**
 * @file EventProcessor.cxx
 * @brief Processor used to write event info.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 */

#include "EventProcessor.h"


EventProcessor::EventProcessor(const std::string& name, Process& process)
    : Processor(name, process) { 
    }

EventProcessor::~EventProcessor() { 
}

void EventProcessor::configure(const ParameterSet& parameters) {

    std::cout << "Configuring EventProcessor" << std::endl;
    try
    {
        debug_         = parameters.getInteger("debug");
        headCollRoot_  = parameters.getString("headCollRoot");
        trigCollLcio_    = parameters.getString("trigCollLcio");
        rfCollLcio_    = parameters.getString("rfCollLcio");
        vtpCollLcio_   = parameters.getString("vtpCollLcio");
        vtpCollRoot_   = parameters.getString("vtpCollRoot");
        tsCollLcio_  = parameters.getString("tsCollLcio");
        tsCollRoot_  = parameters.getString("tsCollRoot");
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }
}

void EventProcessor::initialize(TTree* tree) {
    header_ = new EventHeader();
    vtpData = new VTPData();
    tsData = new TSData();
    tree->Branch(headCollRoot_.c_str(), &header_);
    tree->Branch(vtpCollRoot_.c_str(), &vtpData);
    tree->Branch(tsCollRoot_.c_str(),  &tsData);
}

bool EventProcessor::process(IEvent* ievent) {

    Event* event = static_cast<Event*> (ievent);
    *header_ = event->getEventHeaderMutable(); 

    EVENT::LCEvent* lc_event = event->getLCEvent(); 

    if (debug_) {
        std::cout<<"Event Number: "<<lc_event->getEventNumber()<<std::endl;
        std::cout<<"Run Number: "<<lc_event->getRunNumber()<<std::endl;
    }

    // Set the event number
    header_->setEventNumber(lc_event->getEventNumber());

    // Set the run number
    header_->setRunNumber(lc_event->getRunNumber());

    // Set the trigger timestamp 
    header_->setEventTime(lc_event->getTimeStamp()); 

    // Set the SVT bias state
    header_->setSvtBiasState(lc_event->getParameters().getIntVal("svt_bias_good")); 

    // Set the flag indicating whether the event was affected by SVT burst
    // mode noise 
    header_->setSvtBurstModeNoise(lc_event->getParameters().getIntVal("svt_burstmode_noise_good"));

    // Set the flag indicating whether the SVT latency was correct during an
    // event.
    header_->setSvtLatencyState(lc_event->getParameters().getIntVal("svt_latency_good")); 

    // Set the SVT position state
    header_->setSvtPositionState(lc_event->getParameters().getIntVal("svt_position_good"));

    // Set the SVT event header state
    header_->setSvtEventHeaderState(lc_event->getParameters().getIntVal("svt_event_header_good"));

    // First try to read "new/2019" trigger format, if not available assume it is "old/2016"
    try { 
        EVENT::LCCollection* vtp_data 
            = static_cast<EVENT::LCCollection*>(event->getLCCollection(vtpCollLcio_.c_str()));

        EVENT::LCGenericObject* vtp_datum 
            = static_cast<EVENT::LCGenericObject*>(vtp_data->getElementAt(0));

        EVENT::LCCollection* ts_data 
            = static_cast<EVENT::LCCollection*>(event->getLCCollection(tsCollLcio_.c_str()));

        EVENT::LCGenericObject* ts_datum 
            = static_cast<EVENT::LCGenericObject*>(ts_data->getElementAt(0));

        parseVTPData(vtp_datum);
        parseTSData(ts_datum);

    } 
    catch(EVENT::DataNotAvailableException e) 
    {
        // Get old version of trigger data
        EVENT::LCCollection* trigger_data 
            = static_cast<EVENT::LCCollection*>(event->getLCCollection(trigCollLcio_.c_str()));

        for (int itrigger = 0; itrigger < trigger_data->getNumberOfElements(); ++itrigger) { 

            EVENT::LCGenericObject* trigger_datum 
                = static_cast<EVENT::LCGenericObject*>(trigger_data->getElementAt(itrigger));

            if (trigger_datum->getIntVal(0) == 0xe10a) { 

                TriggerData* tdata = new TriggerData(trigger_datum); 
                header_->setSingle0Trigger(static_cast<int>(tdata->isSingle0Trigger()));
                header_->setSingle1Trigger(static_cast<int>(tdata->isSingle1Trigger()));
                header_->setPair0Trigger(static_cast<int>(tdata->isPair0Trigger()));
                header_->setPair1Trigger(static_cast<int>(tdata->isPair1Trigger()));
                header_->setPulserTrigger(static_cast<int>(tdata->isPulserTrigger()));

                delete tdata;
                break;
            }
        }
    }

    try { 
        // Get the LCIO GenericObject collection containing the RF times
        EVENT::LCCollection* rf_hits 
            = static_cast<EVENT::LCCollection*>(event->getLCCollection(rfCollLcio_.c_str()));

        // The collection should only have a single RFHit object per event
        if (rf_hits->getNumberOfElements() > 1) { 
            throw std::runtime_error("[ EventProcessor ]: The collection " 
                    + static_cast<std::string>(rfCollLcio_.c_str())
                    + " doesn't have the expected number of elements."); 
        }

        // Loop over all the RF hits in the event and write them to the DST
        for (int ihit = 0; ihit < rf_hits->getNumberOfElements(); ++ihit) { 

            // Get the RF hit from the event
            EVENT::LCGenericObject* rf_hit
                = static_cast<EVENT::LCGenericObject*>(rf_hits->getElementAt(ihit));

            // An RFHit GenericObject should only have two RF times
            if (rf_hit->getNDouble() != 2) { 
                throw std::runtime_error("[ EventProcessor ]: The collection "
                        + static_cast<std::string>(rfCollLcio_.c_str())
                        + " has the wrong structure."); 
            }

            // Write the RF times to the event
            for (int ichannel = 0; ichannel < rf_hit->getNDouble(); ++ichannel) { 
                header_->setRfTime(ichannel, rf_hit->getDoubleVal(ichannel));  
            }
        }
    } catch(EVENT::DataNotAvailableException e) {
        // It's fine if the event doesn't have an RF hits collection.
    }

    //vtpData->print();
    //event->add(Collections::EVENT_HEADERS, &header);

    return true;

}

void EventProcessor::parseVTPData(EVENT::LCGenericObject* vtp_data_lcio)
{ 
    // First Clear out all the old data.
    //std::cout << "New VTP Block of size " << vtp_data_lcio->getNInt() << std::endl;
    vtpData->Clear();
    for(int i=0; i<vtp_data_lcio->getNInt()/2; ++i)
    {
        int data = vtp_data_lcio->getIntVal(i);
        int secondWord = vtp_data_lcio->getIntVal(i+1);
        if(!(data & 1<<31)) continue;
        int type = (data>>27)&0x0F;
        int subtype;
        switch (type)
        {
            case 0:  // Block Header
                vtpData->blockHeader.blocklevel = (data      )&0x00FF;
                vtpData->blockHeader.blocknum   = (data >>  8)&0x03FF;
                vtpData->blockHeader.nothing    = (data >> 18)&0x00FF;
                vtpData->blockHeader.slotid     = (data >> 22)&0x001F;
                vtpData->blockHeader.type       = (data >> 27)&0x000F;
                vtpData->blockHeader.istype     = (data >> 31)&0x0001;
                //std::cout << i << " BlockHeader " << vtpData->blockHeader.type << std::endl;
                break;
            case 1: //  Block Tail
                vtpData->blockTail.nwords       = (data      )&0x03FFFFF;
                vtpData->blockTail.slotid       = (data >> 22)&0x000001F;
                vtpData->blockTail.type         = (data >> 27)&0x000000F;
                vtpData->blockTail.istype       = (data >> 31)&0x0000001;
                //std::cout << i << " BlockTail " << vtpData->blockTail.type << std::endl;
                break;
            case 2:  // Event Header
                vtpData->eventHeader.eventnum   = (data      )&0x07FFFFFF;
                vtpData->eventHeader.type       = (data >> 27)&0x0000000F;
                vtpData->eventHeader.istype     = (data >> 31)&0x00000001;
                //std::cout << i << " EventHeader " << vtpData->eventHeader.eventnum << std::endl;
                break;
            case 3:  // Trigger time
                vtpData->trigTime = (data & 0x00FFFFFF) + ((secondWord & 0x00FFFFFF )<<24);
                //std::cout << i << "&" << i+1 << " trigTime = " << vtpData->trigTime << std::endl;
                i++;
                break;
            case 12:  // Expansion type
                subtype = (data>>23)&0x0F;
                switch(subtype){
                    case 2: // HPS Cluster
                        VTPData::hpsCluster  clus;
                        clus.X        = (data      )&0x0003F;
                        clus.Y        = (data >>  6)&0x0000F;
                        clus.E        = (data >> 10)&0x01FFF;
                        clus.subtype  = (data >> 23)&0x0000F;
                        clus.type     = (data >> 27)&0x0000F;
                        clus.istype   = (data >> 31)&0x00001;
                        clus.T        = (secondWord      )&0x003FF;
                        clus.N        = (secondWord >> 10)&0x0000F;
                        clus.nothing  = (secondWord >> 14)&0x3FFFF;
                        vtpData->clusters.push_back(clus);
                        //std::cout << i << "&" << i+1 << " HPS Cluster " << clus.E << std::endl;
                        i++;
                        break;
                    case 3: // HPS Single Trigger
                        VTPData::hpsSingleTrig strig;
                        strig.T        = (data      )&0x003FF;
                        strig.emin     = (data >> 10)&0x00001;
                        strig.emax     = (data >> 11)&0x00001;
                        strig.nmin     = (data >> 12)&0x00001;
                        strig.xmin     = (data >> 13)&0x00001;
                        strig.pose     = (data >> 14)&0x00001;
                        strig.hodo1c   = (data >> 15)&0x00001;
                        strig.hodo2c   = (data >> 16)&0x00001;
                        strig.hodogeo  = (data >> 17)&0x00001;
                        strig.hodoecal = (data >> 18)&0x00001;
                        strig.topnbot  = (data >> 19)&0x00001;
                        strig.inst     = (data >> 20)&0x00007;
                        strig.subtype  = (data >> 23)&0x0000F;
                        strig.type     = (data >> 27)&0x0000F;
                        strig.istype   = (data >> 31)&0x00001;
                        //std::cout << i << " HPS Single Trigger " << strig.subtype << std::endl;
                        vtpData->singletrigs.push_back(strig);
                        break;
                    case 4: // HPS Pair Trigger
                        VTPData::hpsPairTrig ptrig;
                        ptrig.T          = (data      )&0x003FF;
                        ptrig.clusesum   = (data >> 10)&0x00001;
                        ptrig.clusedif   = (data >> 11)&0x00001;
                        ptrig.eslope     = (data >> 12)&0x00001;
                        ptrig.coplane    = (data >> 13)&0x00001;
                        ptrig.dummy      = (data >> 14)&0x0001F;
                        ptrig.topnbot    = (data >> 19)&0x00001;
                        ptrig.inst       = (data >> 20)&0x00007;
                        ptrig.subtype    = (data >> 23)&0x0000F;
                        ptrig.type       = (data >> 27)&0x0000F;
                        ptrig.istype     = (data >> 31)&0x00001;
                        //std::cout << i << " HPS Pair Trigger " << ptrig.subtype << std::endl;
                        vtpData->pairtrigs.push_back(ptrig);
                        break;
                    case 5: // HPS Calibration Trigger
                        VTPData::hpsCalibTrig ctrig;
                        ctrig.T          = (data      )&0x003FF;
                        ctrig.reserved   = (data >> 10)&0x001FF;
                        ctrig.cosmicTrig = (data >> 19)&0x00001;
                        ctrig.LEDTrig    = (data >> 20)&0x00001;
                        ctrig.hodoTrig   = (data >> 21)&0x00001;
                        ctrig.pulserTrig = (data >> 22)&0x00001;
                        ctrig.subtype    = (data >> 23)&0x0000F;
                        ctrig.type       = (data >> 27)&0x0000F;
                        ctrig.istype     = (data >> 31)&0x00001;
                        //std::cout << i << " HPS Cal Trigger " << ctrig.subtype << std::endl;
                        vtpData->calibtrigs.push_back(ctrig);
                        break;
                    case 6: // HPS Cluster Multiplicity Trigger
                        VTPData::hpsClusterMult clmul;
                        clmul.T          = (data      )&0x003FF;
                        clmul.multtop    = (data >> 10)&0x0000F;
                        clmul.multbot    = (data >> 14)&0x0000F;
                        clmul.multtot    = (data >> 18)&0x0000F;
                        clmul.bitinst    = (data >> 22)&0x00001;
                        clmul.subtype    = (data >> 23)&0x0000F;
                        clmul.type       = (data >> 27)&0x0000F;
                        clmul.istype     = (data >> 31)&0x00001;
                        //std::cout << i << " HPS Clus Mult Trigger " << clmul.subtype << std::endl;
                        vtpData->clustermult.push_back(clmul);
                        break;
                    case 7: // HPS FEE Trigger
                        VTPData::hpsFEETrig fee;
                        fee.T          = (data      )&0x003FF;
                        fee.region     = (data >> 10)&0x0007F;
                        fee.reserved   = (data >> 17)&0x0003F;
                        fee.subtype    = (data >> 23)&0x0000F;
                        fee.type       = (data >> 27)&0x0000F;
                        fee.istype     = (data >> 31)&0x00001;
                        //std::cout << i << " HPS FEE Trigger " << fee.subtype << std::endl;
                        vtpData->feetrigger.push_back(fee);
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
    //std::cout << "---------------------------------------" << std::endl;
    //std::cout << std::endl;
} //EventProcessor::parseVTPData(LCGenericObject* vtp_data_lcio)

void EventProcessor::parseTSData(EVENT::LCGenericObject* ts_data_lcio)
{ 
    // Parse out TS header
    unsigned int headerWord = ts_data_lcio->getIntVal(1);
    tsData->header.wordCount = (headerWord      )&0xFFFF; //  0-15 Word Count
    tsData->header.test      = (headerWord >> 16)&0x00FF; // 16-23 Test Word
    tsData->header.type      = (headerWord >> 24)&0x00FF; // 24-31 Trigger Type
    // Parse out trigger time and Event Number
    tsData->T = static_cast<unsigned long>(ts_data_lcio->getIntVal(3)) + ( (static_cast<unsigned long>(ts_data_lcio->getIntVal(4)&0xFFFF)<<32));
    tsData->EN = static_cast<unsigned long>(ts_data_lcio->getIntVal(2)) + ( (static_cast<unsigned long>(ts_data_lcio->getIntVal(4)&0xFFFF0000)<<16));
    // Parse out prescaled word
    tsData->prescaled.intval = ts_data_lcio->getIntVal(5); // Full word
    tsData->prescaled.Single_0_Top = (tsData->prescaled.intval      )&0x001; //  0 Low energy cluster
    tsData->prescaled.Single_1_Top = (tsData->prescaled.intval >>  1)&0x001; //  1 e+
    tsData->prescaled.Single_2_Top = (tsData->prescaled.intval >>  2)&0x001; //  2 e+ : Position dependent energy cut
    tsData->prescaled.Single_3_Top = (tsData->prescaled.intval >>  3)&0x001; //  3 e+ : HODO L1*L2  Match with cluster
    tsData->prescaled.Single_0_Bot = (tsData->prescaled.intval >>  4)&0x001; //  4 Low energy cluster
    tsData->prescaled.Single_1_Bot = (tsData->prescaled.intval >>  5)&0x001; //  5 e+
    tsData->prescaled.Single_2_Bot = (tsData->prescaled.intval >>  6)&0x001; //  6 e+ : Position dependent energy cut
    tsData->prescaled.Single_3_Bot = (tsData->prescaled.intval >>  7)&0x001; //  7 e+ : HODO L1*L2  Match with cluster
    tsData->prescaled.Pair_0       = (tsData->prescaled.intval >>  8)&0x001; //  8 A'
    tsData->prescaled.Pair_1       = (tsData->prescaled.intval >>  9)&0x001; //  9 Moller
    tsData->prescaled.Pair_2       = (tsData->prescaled.intval >> 10)&0x001; // 10 pi0
    tsData->prescaled.Pair_3       = (tsData->prescaled.intval >> 11)&0x001; // 11 -
    tsData->prescaled.LED          = (tsData->prescaled.intval >> 12)&0x001; // 12 LED
    tsData->prescaled.Cosmic       = (tsData->prescaled.intval >> 13)&0x001; // 13 Cosmic
    tsData->prescaled.Hodoscope    = (tsData->prescaled.intval >> 14)&0x001; // 14 Hodoscope
    tsData->prescaled.Pulser       = (tsData->prescaled.intval >> 15)&0x001; // 15 Pulser
    tsData->prescaled.Mult_0       = (tsData->prescaled.intval >> 16)&0x001; // 16 Multiplicity-0 2 Cluster Trigger
    tsData->prescaled.Mult_1       = (tsData->prescaled.intval >> 17)&0x001; // 17 Multiplicity-1 3 Cluster trigger
    tsData->prescaled.FEE_Top      = (tsData->prescaled.intval >> 18)&0x001; // 18 FEE Top       ( 2600-5200)
    tsData->prescaled.FEE_Bot      = (tsData->prescaled.intval >> 19)&0x001; // 19 FEE Bot       ( 2600-5200)
    tsData->prescaled.NA           = (tsData->prescaled.intval >> 20)&0xFFF; // 20-31 Not used
    // Parse out ext word
    tsData->ext.intval = ts_data_lcio->getIntVal(6); // Full word
    tsData->ext.Single_0_Top = (tsData->ext.intval      )&0x001; //  0 Low energy cluster
    tsData->ext.Single_1_Top = (tsData->ext.intval >>  1)&0x001; //  1 e+
    tsData->ext.Single_2_Top = (tsData->ext.intval >>  2)&0x001; //  2 e+ : Position dependent energy cut
    tsData->ext.Single_3_Top = (tsData->ext.intval >>  3)&0x001; //  3 e+ : HODO L1*L2  Match with cluster
    tsData->ext.Single_0_Bot = (tsData->ext.intval >>  4)&0x001; //  4 Low energy cluster
    tsData->ext.Single_1_Bot = (tsData->ext.intval >>  5)&0x001; //  5 e+
    tsData->ext.Single_2_Bot = (tsData->ext.intval >>  6)&0x001; //  6 e+ : Position dependent energy cut
    tsData->ext.Single_3_Bot = (tsData->ext.intval >>  7)&0x001; //  7 e+ : HODO L1*L2  Match with cluster
    tsData->ext.Pair_0       = (tsData->ext.intval >>  8)&0x001; //  8 A'
    tsData->ext.Pair_1       = (tsData->ext.intval >>  9)&0x001; //  9 Moller
    tsData->ext.Pair_2       = (tsData->ext.intval >> 10)&0x001; // 10 pi0
    tsData->ext.Pair_3       = (tsData->ext.intval >> 11)&0x001; // 11 -
    tsData->ext.LED          = (tsData->ext.intval >> 12)&0x001; // 12 LED
    tsData->ext.Cosmic       = (tsData->ext.intval >> 13)&0x001; // 13 Cosmic
    tsData->ext.Hodoscope    = (tsData->ext.intval >> 14)&0x001; // 14 Hodoscope
    tsData->ext.Pulser       = (tsData->ext.intval >> 15)&0x001; // 15 Pulser
    tsData->ext.Mult_0       = (tsData->ext.intval >> 16)&0x001; // 16 Multiplicity-0 2 Cluster Trigger
    tsData->ext.Mult_1       = (tsData->ext.intval >> 17)&0x001; // 17 Multiplicity-1 3 Cluster trigger
    tsData->ext.FEE_Top      = (tsData->ext.intval >> 18)&0x001; // 18 FEE Top       ( 2600-5200)
    tsData->ext.FEE_Bot      = (tsData->ext.intval >> 19)&0x001; // 19 FEE Bot       ( 2600-5200)
    tsData->ext.NA           = (tsData->ext.intval >> 20)&0xFFF; // 20-31 Not used
} //EventProcessor::parstsData->eTSData(LCGenericObject* ts_data_lcio)

void EventProcessor::finalize() { 
}

DECLARE_PROCESSOR(EventProcessor); 
