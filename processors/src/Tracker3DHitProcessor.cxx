#include "Tracker3DHitProcessor.h" 
#include "utilities.h"

Tracker3DHitProcessor::Tracker3DHitProcessor(const std::string& name, Process& process)
    : Processor(name, process) { 
}

Tracker3DHitProcessor::~Tracker3DHitProcessor() { 
}

void Tracker3DHitProcessor::configure(const ParameterSet& parameters) {

    std::cout << "Configuring Tracker3DHitProcessor" << std::endl;
    try
    {
        debug_          = parameters.getInteger("debug", debug_);
        hitCollLcio_    = parameters.getString("hitCollLcio", hitCollLcio_);
        hitCollRoot_    = parameters.getString("hitCollRoot", hitCollRoot_);
        mcPartRelLcio_ = parameters.getString("mcPartRelLcio", mcPartRelLcio_);
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }
}

void Tracker3DHitProcessor::initialize(TTree* tree) {
    // Add branches to tree
    tree->Branch(hitCollRoot_.c_str(), &hits_);
}

bool Tracker3DHitProcessor::process(IEvent* ievent) {

    for(int i = 0; i < hits_.size(); i++) delete hits_.at(i);
    hits_.clear();

    Event* event = static_cast<Event*> (ievent);
    UTIL::LCRelationNavigator* mcPartRel_nav;

    // Get the collection of 3D hits from the LCIO event. If no such collection 
    // exist, a DataNotAvailableException is thrown
    EVENT::LCCollection* tracker_hits{nullptr};
    try
    {
        tracker_hits = event->getLCCollection(hitCollLcio_.c_str()); 
    }
    catch (EVENT::DataNotAvailableException e) 
    {
        std::cout << e.what() << std::endl;
    }

    //Check to see if MC Particles are in the file
    auto evColls = event->getLCEvent()->getCollectionNames();
    auto it = std::find (evColls->begin(), evColls->end(), mcPartRelLcio_.c_str());
    bool hasMCParts = true;
    EVENT::LCCollection* mcPartRel;
    if(it == evColls->end()) hasMCParts = false;
    if(hasMCParts)
    {
        mcPartRel = event->getLCCollection(mcPartRelLcio_.c_str());
        // Heap an LCRelation navigator which will allow faster access 
        mcPartRel_nav = new UTIL::LCRelationNavigator(mcPartRel);

    }

    // Create a map from an LCIO TrackerHit to a SvtHit. This will be used when
    // assigning references to a track
    // TODO: Use an unordered map for faster access
    std::map<EVENT::TrackerHit*, TrackerHit*> hit_map;

    // Loop over all of the 3D hits in the LCIO event and add them to the 
    // HPS event
    for (int ihit = 0; ihit < tracker_hits->getNumberOfElements(); ++ihit) { 

        // Get a 3D hit from the list of hits
        IMPL::TrackerHitImpl* lc_tracker_hit = static_cast<IMPL::TrackerHitImpl*>(tracker_hits->getElementAt(ihit));

        // Build a TrackerHit
        TrackerHit* tracker_hit = utils::buildTrackerHit(lc_tracker_hit);

        if(hasMCParts)
        {
            // Get the list of fit params associated with the raw tracker hit
            EVENT::LCObjectVec mcPart_list
                = mcPartRel_nav->getRelatedToObjects(lc_tracker_hit);

            if(debug_ > 0) std::cout << "Has " << mcPart_list.size() << " Related MC Particles" << std::endl;
            // Get all the MC Particle IDs associated to the hit
            for(int ipart = 0; ipart < mcPart_list.size(); ipart++)
            {
                IMPL::MCParticleImpl* lc_particle
                    = static_cast<IMPL::MCParticleImpl*>(mcPart_list.at(ipart));
                tracker_hit->addMCPartID(lc_particle->id());
                if(debug_ > 0) std::cout << "Has Related MC Particle with ID " << lc_particle->id() << std::endl;
            }
        }

        // Map the TrackerHit object to the corresponding SvtHit object. This
        // will be used later when setting references for hits on tracks.
        hit_map[lc_tracker_hit] = tracker_hit; 
        hits_.push_back(tracker_hit);

    }

    if(hasMCParts) delete mcPartRel_nav;

    return true;
}

void Tracker3DHitProcessor::finalize() { 
}

DECLARE_PROCESSOR(Tracker3DHitProcessor); 
