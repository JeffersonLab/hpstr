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

void MCEcalHitProcessor::configure(const ParameterSet& parameters) {

    std::cout << "Configuring MCEcalHitProcessor" << std::endl;
    try
    {
        debug_         = parameters.getInteger("debug", debug_);
        hitCollLcio_   = parameters.getString("hitCollLcio", hitCollLcio_);
        hitCollRoot_   = parameters.getString("hitCollRoot", hitCollRoot_);
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }
}

void MCEcalHitProcessor::initialize(TTree* tree) {

    tree->Branch(hitCollRoot_.c_str(), &ecalhits_);
}

bool MCEcalHitProcessor::process(IEvent* ievent) {

    Event* event = static_cast<Event*>(ievent);
    // Get the collection of simulated ecal hits from the LCIO event.
    EVENT::LCCollection* lcio_ecalhits{nullptr};
    try
    {
        lcio_ecalhits = event->getLCCollection(hitCollLcio_.c_str());
    }
    catch (EVENT::DataNotAvailableException e) 
    {
        std::cout << e.what() << std::endl;
    }


    // Get decoders to read cellids
    UTIL::BitField64 decoder("system:0:6,layer:6:2,ix:8:-8,iy:16:-6");
    //decoder[field] returns the value

    // Loop over all of the raw SVT hits in the LCIO event and add them to the 
    // HPS event
    for(int i = 0; i < ecalhits_.size(); i++) delete ecalhits_.at(i);
    ecalhits_.clear();
    for (int ihit = 0; ihit < lcio_ecalhits->getNumberOfElements(); ++ihit) {

        // Get a 3D hit from the list of hits
        EVENT::SimCalorimeterHit* lcio_mcEcal_hit 
            = static_cast<EVENT::SimCalorimeterHit*>(lcio_ecalhits->getElementAt(ihit));
        //Decode the cellid
        EVENT::long64 value = EVENT::long64( lcio_mcEcal_hit->getCellID0() & 0xffffffff ) | 
            ( EVENT::long64( lcio_mcEcal_hit->getCellID1() ) << 32 ) ;
        decoder.setValue(value);

        // Add a mc ecal hit to the event
        MCEcalHit* mc_ecal_hit = new MCEcalHit();

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

        //Push onto vector of hits
        ecalhits_.push_back(mc_ecal_hit);

    }

    return true;
}

void MCEcalHitProcessor::finalize() { 
}

DECLARE_PROCESSOR(MCEcalHitProcessor); 
