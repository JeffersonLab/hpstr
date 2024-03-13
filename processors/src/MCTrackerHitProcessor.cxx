/**
 * @file MCTrackerHitProcessor.h
 * @brief Processor used to add simulated tracker hits to the event
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 */
#include "MCTrackerHitProcessor.h" 

MCTrackerHitProcessor::MCTrackerHitProcessor(const std::string& name, Process& process)
    : Processor(name, process) { 
    }

MCTrackerHitProcessor::~MCTrackerHitProcessor() { 
}

void MCTrackerHitProcessor::configure(const ParameterSet& parameters) {

    std::cout << "Configuring MCTrackerHitProcessor" << std::endl;
    try
    {
        debug_         = parameters.getInteger("debug", debug_ );
        hitCollLcio_   = parameters.getString("hitCollLcio", hitCollLcio_);
        hitCollRoot_   = parameters.getString("hitCollRoot", hitCollRoot_);
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }
}

void MCTrackerHitProcessor::initialize(TTree* tree) {

    tree->Branch(hitCollRoot_.c_str(), &trackerhits_);
}

bool MCTrackerHitProcessor::process(IEvent* ievent) {

    Event* event = static_cast<Event*>(ievent);
    // Get the collection of simulated tracker hits from the LCIO event.
    EVENT::LCCollection* lcio_trackerhits{nullptr};
    try
    {
        lcio_trackerhits = event->getLCCollection(hitCollLcio_.c_str());
    }
    catch (EVENT::DataNotAvailableException e) 
    {
        std::cout << e.what() << std::endl;
    }

    // Get decoders to read cellids
    UTIL::BitField64 decoder("system:0:6,barrel:6:3,layer:9:4,module:13:12,sensor:25:1,side:32:-2,strip:34:12");
    //decoder[field] returns the value

    // Loop over all of the raw SVT hits in the LCIO event and add them to the 
    // HPS event
    for(int i = 0; i < trackerhits_.size(); i++) delete trackerhits_.at(i);
    trackerhits_.clear();
    for (int ihit = 0; ihit < lcio_trackerhits->getNumberOfElements(); ++ihit) {

        // Get a 3D hit from the list of hits
        EVENT::SimTrackerHit* lcio_mcTracker_hit 
            = static_cast<EVENT::SimTrackerHit*>(lcio_trackerhits->getElementAt(ihit));
        //Decode the cellid
        EVENT::long64 value = EVENT::long64( lcio_mcTracker_hit->getCellID0() & 0xffffffff ) | 
            ( EVENT::long64( lcio_mcTracker_hit->getCellID1() ) << 32 ) ;
        decoder.setValue(value);

        // Add a raw tracker hit to the event
        MCTrackerHit* mc_tracker_hit = new MCTrackerHit();

        // Set sensitive detector identification
        mc_tracker_hit->setLayer(decoder["layer"]);
        mc_tracker_hit->setModule(decoder["module"]);

        // Set the position of the hit
        double hitPos[3];
        hitPos[0] = lcio_mcTracker_hit->getPosition()[0];
        hitPos[1] = lcio_mcTracker_hit->getPosition()[1];
        hitPos[2] = lcio_mcTracker_hit->getPosition()[2];
        mc_tracker_hit->setPosition(hitPos);

        // Set the momentum of the particle for this hit
        double hitMomentum[3];
        hitMomentum[0] = lcio_mcTracker_hit->getMomentum()[0];
        hitMomentum[1] = lcio_mcTracker_hit->getMomentum()[1];
        hitMomentum[2] = lcio_mcTracker_hit->getMomentum()[2];
        mc_tracker_hit->setMomentum(hitMomentum);

        // Set the energy deposit of the hit
        mc_tracker_hit->setEdep(lcio_mcTracker_hit->getEDep());

        // Set the pdg of particle generating the hit
        if(lcio_mcTracker_hit->getMCParticle()){
            mc_tracker_hit->setPDG(lcio_mcTracker_hit->getMCParticle()->getPDG());
            mc_tracker_hit->setPartID(lcio_mcTracker_hit->getMCParticle()->id());
        }
        
        // Set the time of the hit
        mc_tracker_hit->setTime(lcio_mcTracker_hit->getTime());

        //Push hit onto vector
        trackerhits_.push_back(mc_tracker_hit);

    }

    return true;
}

void MCTrackerHitProcessor::finalize() { 
}

DECLARE_PROCESSOR(MCTrackerHitProcessor); 
