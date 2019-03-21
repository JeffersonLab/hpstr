/**
 * @file EventProcessor.cxx
 * @brief Processor used to write event info.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "EventProcessor.h"

EventProcessor::EventProcessor(const std::string& name, Process& process)
    : Processor(name, process) { 
}

EventProcessor::~EventProcessor() { 
}

void EventProcessor::initialize(TTree* tree) {
}

void EventProcessor::process(Event* event) {

    /*EventHeader* header 
        = static_cast<EventHeader*>(header_->ConstructedAt(0));*/
    EventHeader& header = event->getEventHeaderMutable(); 

    EVENT::LCEvent* lc_event = event->getLCEvent(); 

    // Set the event number
    header.setEventNumber(lc_event->getEventNumber());

    // Set the run number
    header.setRunNumber(lc_event->getRunNumber());

    // Set the trigger timestamp 
    header.setEventTime(lc_event->getTimeStamp()); 

    // Set the SVT bias state
    header.setSvtBiasState(lc_event->getParameters().getIntVal("svt_bias_good")); 
    
    // Set the flag indicating whether the event was affected by SVT burst
    // mode noise 
    header.setSvtBurstModeNoise(lc_event->getParameters().getIntVal("svt_burstmode_noise_good"));

    // Set the flag indicating whether the SVT latency was correct during an
    // event.
    header.setSvtLatencyState(lc_event->getParameters().getIntVal("svt_latency_good")); 

    // Set the SVT position state
    header.setSvtPositionState(lc_event->getParameters().getIntVal("svt_position_good"));

    // Set the SVT event header state
    header.setSvtEventHeaderState(lc_event->getParameters().getIntVal("svt_event_header_good"));

    try { 
        EVENT::LCCollection* trigger_data 
            = static_cast<EVENT::LCCollection*>(event->getLCCollection(Collections::TRIGGER_BANK));

        for (int itrigger = 0; itrigger < trigger_data->getNumberOfElements(); ++itrigger) { 
       
            EVENT::LCGenericObject* trigger_datum 
                = static_cast<EVENT::LCGenericObject*>(trigger_data->getElementAt(itrigger));

            if (trigger_datum->getIntVal(0) == 0xe10a) { 
          
                TriggerData* tdata = new TriggerData(trigger_datum); 
                header.setSingle0Trigger(static_cast<int>(tdata->isSingle0Trigger()));
                header.setSingle1Trigger(static_cast<int>(tdata->isSingle1Trigger()));
                header.setPair0Trigger(static_cast<int>(tdata->isPair0Trigger()));
                header.setPair1Trigger(static_cast<int>(tdata->isPair1Trigger()));
                header.setPulserTrigger(static_cast<int>(tdata->isPulserTrigger()));

                delete tdata;
                break;
            }
        }
    } catch(EVENT::DataNotAvailableException e) {
        // It's fine if the event doesn't have a trigger bank.
    }

    try { 
        // Get the LCIO GenericObject collection containing the RF times
        EVENT::LCCollection* rf_hits 
            = static_cast<EVENT::LCCollection*>(event->getLCCollection(Collections::RF_HITS));

        // The collection should only have a single RFHit object per event
        if (rf_hits->getNumberOfElements() > 1) { 
            throw std::runtime_error("[ EventProcessor ]: The collection " 
                    + static_cast<std::string>(Collections::RF_HITS)
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
                    + static_cast<std::string>(Collections::RF_HITS)
                    + " has the wrong structure."); 
            }
    
            // Write the RF times to the event
            for (int ichannel = 0; ichannel < rf_hit->getNDouble(); ++ichannel) { 
                header.setRfTime(ichannel, rf_hit->getDoubleVal(ichannel));  
            }
        }
    } catch(EVENT::DataNotAvailableException e) {
        // It's fine if the event doesn't have an RF hits collection.
    }

    event->add(Collections::EVENT_HEADERS, &header);
}

void EventProcessor::finalize() { 
}

DECLARE_PROCESSOR(EventProcessor); 
