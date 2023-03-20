#include "Tracker2DHitProcessor.h" 
#include "utilities.h"

Tracker2DHitProcessor::Tracker2DHitProcessor(const std::string& name, Process& process)
    : Processor(name, process) { 
}

Tracker2DHitProcessor::~Tracker2DHitProcessor() { 
}

void Tracker2DHitProcessor::configure(const ParameterSet& parameters) {

    std::cout << "Configuring Tracker2DHitProcessor" << std::endl;
    try
    {
        debug_           = parameters.getInteger("debug", debug_);
        hitCollLcio_     = parameters.getString("hitCollLcio", hitCollLcio_);
        hitCollRoot_     = parameters.getString("hitCollRoot", hitCollRoot_);
        hitFitsCollLcio_   = parameters.getString("hitFitsCollLcio", hitFitsCollLcio_);
        rawhitCollRoot_  = parameters.getString("rawhitCollRoot", hitCollRoot_);
        mcPartRelLcio_ = parameters.getString("mcPartRelLcio", mcPartRelLcio_);
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }
}

void Tracker2DHitProcessor::initialize(TTree* tree) {
    // Add branches to tree
    tree->Branch(hitCollRoot_.c_str(), &hits_);
    if (!rawhitCollRoot_.empty())
        tree->Branch(rawhitCollRoot_.c_str(), &rawhits_);
}

bool Tracker2DHitProcessor::process(IEvent* ievent) {

    for(int i = 0; i < hits_.size(); i++) delete hits_.at(i);
    hits_.clear();

    if (rawhits_.size() > 0) {
        for (std::vector<RawSvtHit *>::iterator it = rawhits_.begin(); it != rawhits_.end(); ++it) {
            delete *it;
        }
        rawhits_.clear();
    }

    Event* event = static_cast<Event*> (ievent);
    UTIL::LCRelationNavigator* mcPartRel_nav;

    // Get the collection of 2D hits from the LCIO event. If no such collection 
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
    auto mcit = std::find (evColls->begin(), evColls->end(), mcPartRelLcio_.c_str());
    bool hasMCParts = true;
    EVENT::LCCollection* mcPartRel;
    if(mcit == evColls->end()) hasMCParts = false;
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

    EVENT::LCCollection* raw_svt_hit_fits = nullptr;
    auto fitit = std::find (evColls->begin(), evColls->end(), hitFitsCollLcio_.c_str());
    bool hasFits = true;
    if(fitit == evColls->end()) hasFits = false;
    if(hasFits)
    {
        raw_svt_hit_fits = event->getLCCollection(hitFitsCollLcio_.c_str());
    }

    // Loop over all of the 2D hits in the LCIO event and add them to the 
    // HPS event
    for (int ihit = 0; ihit < tracker_hits->getNumberOfElements(); ++ihit) { 

        // Get a 2D hit from the list of hits
        IMPL::TrackerHitImpl* lc_tracker_hit = static_cast<IMPL::TrackerHitImpl*>(tracker_hits->getElementAt(ihit));

        if(debug_ > 0)
            std::cout << "tracker hit lcio id: " << lc_tracker_hit->id() << std::endl;

        // Build a TrackerHit
        TrackerHit* tracker_hit = utils::buildTrackerHit(lc_tracker_hit,true,0);
        std::vector<RawSvtHit*> rawSvthits;
        utils::addRawInfoTo3dHit(tracker_hit, lc_tracker_hit, raw_svt_hit_fits,&rawSvthits,0);
        for (auto rhit : rawSvthits)
            rawhits_.push_back(rhit);


        if(hasMCParts)
        {
            //Get the SvtRawTrackerHits that make up the 2D hit
            EVENT::LCObjectVec rawHits = lc_tracker_hit->getRawHits(); 
            for(int irawhit = 0; irawhit < rawHits.size(); ++irawhit)
            {
                IMPL::TrackerHitImpl* rawhit = static_cast<IMPL::TrackerHitImpl*>(rawHits.at(irawhit));
                if(debug_ > 0)
                    std::cout << "rawhit on track has lcio id: " << rawhit->id() << std::endl;

                // Get the list of fit params associated with the raw tracker hit
                EVENT::LCObjectVec lc_simtrackerhits = mcPartRel_nav->getRelatedToObjects(rawhit);

                //Loop over SimTrackerHits to get MCParticles
                for(int isimhit = 0; isimhit < lc_simtrackerhits.size(); isimhit++){
                    IMPL::SimTrackerHitImpl* lc_simhit = static_cast<IMPL::SimTrackerHitImpl*>(lc_simtrackerhits.at(isimhit));
                    IMPL::MCParticleImpl* lc_mcp = static_cast<IMPL::MCParticleImpl*>(lc_simhit->getMCParticle());
                    tracker_hit->addMCPartID(lc_mcp->id());
                    if(debug_ > 0) {
                        std::cout << "simtrackerhit lcio id: " << lc_simhit->id() << std::endl;
                        std::cout << "mcp lcio id: " << lc_mcp->id() << std::endl;
                    }
                }

                /*
                // Get all the MC Particle IDs associated to the hit
                for(int ipart = 0; ipart < mcPart_list.size(); ipart++)
                {
                IMPL::MCParticleImpl* lc_particle
                = static_cast<IMPL::MCParticleImpl*>(mcPart_list.at(ipart));
                tracker_hit->addMCPartID(lc_particle->id());
                if(debug_ > 0) std::cout << "Has Related MC Particle with ID " << lc_particle->id() << std::endl;
                }
                */
            }
        }

        // Map the TrackerHit object to the corresponding SvtHit object. This
        // will be used later when setting references for hits on tracks.
        //hit_map[lc_tracker_hit] = tracker_hit; 
        hits_.push_back(tracker_hit);

    }

    if(hasMCParts) delete mcPartRel_nav;

    return true;
}

void Tracker2DHitProcessor::finalize() { 
}

DECLARE_PROCESSOR(Tracker2DHitProcessor); 
