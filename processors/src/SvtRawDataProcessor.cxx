/**
 * @file SvtRawDataProcessor.h
 * @brief Processor used to add Raw SVT data to tree
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 */
#include "SvtRawDataProcessor.h" 

SvtRawDataProcessor::SvtRawDataProcessor(const std::string& name, Process& process)
    : Processor(name, process) { 
}

SvtRawDataProcessor::~SvtRawDataProcessor() { 
}

void SvtRawDataProcessor::initialize(TTree* tree) {

    rawhits_   = new TClonesArray("RawSvtHit", 100000);  
}

void SvtRawDataProcessor::process(Event* event) {
    // Get the collection of 3D hits from the LCIO event. If no such collection 
    // exist, a DataNotAvailableException is thrown
    EVENT::LCCollection* raw_svt_hits = event->getLCCollection(Collections::RAW_SVT_HITS); 

    // Get decoders to read cellids
    UTIL::BitField64 decoder("system:6,barrel:3,layer:4,module:12,sensor:1,side:32:-2,strip:12");
    //decoder[field] returns the value

    // Loop over all of the raw SVT hits in the LCIO event and add them to the 
    // HPS event
    for (int ihit = 0; ihit < raw_svt_hits->getNumberOfElements(); ++ihit) { 
        
        // Get a 3D hit from the list of hits
        EVENT::TrackerRawData* rawTracker_hit = static_cast<EVENT::TrackerRawData*>(raw_svt_hits->getElementAt(ihit));
        //Decode the cellid
        EVENT::long64 value = EVENT::long64( rawTracker_hit->getCellID0() & 0xffffffff ) | ( EVENT::long64( rawTracker_hit->getCellID1() ) << 32 ) ;
        decoder.setValue(value);
    
        // Add a raw tracker hit to the event
        RawSvtHit* rawHit = static_cast<RawSvtHit*>(rawhits_->ConstructedAt(ihit));

        rawHit->setLayer(decoder["layer"]);
        rawHit->setModule(decoder["module"]);
        rawHit->setSensor(decoder["sensor"]);
        rawHit->setSide(decoder["side"]);
        rawHit->setStrip(decoder["strip"]);

        // Rotate the position of the LCIO TrackerHit and set the position of 
        // the TrackerHit
        int hit_adcs[6] = { 
            (int)rawTracker_hit->getADCValues().at(0), 
            (int)rawTracker_hit->getADCValues().at(1), 
            (int)rawTracker_hit->getADCValues().at(2), 
            (int)rawTracker_hit->getADCValues().at(3), 
            (int)rawTracker_hit->getADCValues().at(4), 
            (int)rawTracker_hit->getADCValues().at(5)
        };
        rawHit->setADCs(hit_adcs);
        
    }

    // Add the raw hit collection to the event
    event->addCollection(Collections::RAW_SVT_HITS, rawhits_); 

}

void SvtRawDataProcessor::finalize() { 
}

DECLARE_PROCESSOR(SvtRawDataProcessor); 
