/**
 * @file MCEcalHitProcessor.cxx
 * @brief Processor used to add simulated ecal hits to the event
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 */
#include "MCEcalHitProcessor.h" 

MCEcalHitProcessor::MCEcalHitProcessor(const std::string& name, Process& process)
    : Processor(name, process) { 
}

MCEcalHitProcessor::~MCEcalHitProcessor() { 
}

void MCEcalHitProcessor::initialize(TTree* tree) {

    ecalhits_   = new TClonesArray("MCEcalHit", 100000);  
    tree->Branch(Collections::MC_ECAL_HITS,&ecalhits_);
}

bool MCEcalHitProcessor::process(IEvent* ievent) {
  
    Event* event = static_cast<Event*>(ievent);
    // Get the collection of simulated ecal hits from the LCIO event.
    EVENT::LCCollection* lcio_ecalhits = event->getLCCollection(Collections::MC_ECAL_HITS);

    // Get decoders to read cellids
    UTIL::BitField64 decoder("system:0:6,layer:6:2,ix:8:-8,iy:16:-6");
    //decoder[field] returns the value

    // Loop over all of the raw SVT hits in the LCIO event and add them to the 
    // HPS event
    ecalhits_->Clear();
    for (int ihit = 0; ihit < lcio_ecalhits->getNumberOfElements(); ++ihit) {

      // Get a 3D hit from the list of hits
        EVENT::SimCalorimeterHit* lcio_mcEcal_hit 
            = static_cast<EVENT::SimCalorimeterHit*>(lcio_ecalhits->getElementAt(ihit));
        //Decode the cellid
        EVENT::long64 value = EVENT::long64( lcio_mcEcal_hit->getCellID0() & 0xffffffff ) | 
            ( EVENT::long64( lcio_mcEcal_hit->getCellID1() ) << 32 ) ;
        decoder.setValue(value);

        // Add a mc ecal hit to the event
        MCEcalHit* mc_ecal_hit = static_cast<MCEcalHit*>(ecalhits_->ConstructedAt(ihit));

        // Set sensitive detector identification
        mc_ecal_hit->setSystem(decoder["system"]);
        mc_ecal_hit->setLayer(decoder["layer"]);
        mc_ecal_hit->setIX(decoder["ix"]);
        mc_ecal_hit->setIY(decoder["iy"]);

        // Set the position of the hit, dealing with it being a float and not double
        const float hitPosF[3] = {lcio_mcEcal_hit->getPosition()[0], lcio_mcEcal_hit->getPosition()[1], lcio_mcEcal_hit->getPosition()[2]};
        double hitPosD[3] = {(double)hitPosF[0], (double)hitPosF[1], (double)hitPosF[2]};
        mc_ecal_hit->setPosition(hitPosD);

        // Set the energy deposit of the hit
        mc_ecal_hit->setEnergy(lcio_mcEcal_hit->getEnergy());

    }

    return true;
}

void MCEcalHitProcessor::finalize() { 
}

DECLARE_PROCESSOR(MCEcalHitProcessor); 
