/** 
 * @file FinalStateParticleProcessor.h
 * @brief Class used to convert LCIO Vertex collections into ROOT collections.
 * @author Cameron Bravo, SLAC
 */
#include "FinalStateParticleProcessor.h" 
#include "utilities.h"

FinalStateParticleProcessor::FinalStateParticleProcessor(const std::string& name, Process& process)
    : Processor(name, process) { 
    }

FinalStateParticleProcessor::~FinalStateParticleProcessor() { 
}

void FinalStateParticleProcessor::configure(const ParameterSet& parameters) {

    std::cout << "Configuring FinalStateParticleProcessor" << std::endl;
    try
    {
        debug_             = parameters.getInteger("debug", debug_);
        fspCollLcio_       = parameters.getString("fspCollLcio", fspCollLcio_);
        fspCollRoot_       = parameters.getString("fspCollRoot", fspCollRoot_);
        kinkRelCollLcio_   = parameters.getString("kinkRelCollLcio", kinkRelCollLcio_);
        trkRelCollLcio_    = parameters.getString("trkRelCollLcio", trkRelCollLcio_);
        hitFitsCollLcio_   = parameters.getString("hitFitsCollLcio", hitFitsCollLcio_);    
        trkhitCollRoot_    = parameters.getString("trkhitCollRoot",trkhitCollRoot_);
        rawhitCollRoot_    = parameters.getString("rawhitCollRoot",rawhitCollRoot_);
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }
}

void FinalStateParticleProcessor::initialize(TTree* tree) {
    // Add branches to tree
   
    if (!trkhitCollRoot_.empty())
        tree->Branch(trkhitCollRoot_.c_str(), &hits_);
    
    if (!rawhitCollRoot_.empty())
        tree->Branch(rawhitCollRoot_.c_str(), &rawhits_); 
    tree->Branch(fspCollRoot_.c_str(),  &fsps_);
}

bool FinalStateParticleProcessor::process(IEvent* ievent) {

    if (debug_ > 0) std::cout << "FinalStateParticleProcessor: Clear output vector" << std::endl;
    
    //Clean up
    if (hits_.size() > 0) {
        for (std::vector<TrackerHit *>::iterator it = hits_.begin(); it != hits_.end(); ++it) {
            delete *it;
        }
        hits_.clear();
    }
    
    if (rawhits_.size() > 0) {
        for (std::vector<RawSvtHit *>::iterator it = rawhits_.begin(); it != rawhits_.end(); ++it) {
            delete *it;
        }
        rawhits_.clear();
    } 
    
    for(int i = 0; i < fsps_.size(); i++) delete fsps_.at(i);
    fsps_.clear();

    Event* event = static_cast<Event*> (ievent);

    // Get the collection of vertices from the LCIO event. If no such collection 
    // exist, a DataNotAvailableException is thrown
    if (debug_ > 0) std::cout << "FinalStateParticleProcessor: Get LCIO Collection " << fspCollLcio_ << std::endl;
    EVENT::LCCollection* lc_fsps= nullptr;
    try
    {
        lc_fsps = event->getLCCollection(fspCollLcio_.c_str()); 
    }
    catch (EVENT::DataNotAvailableException e) 
    {
        std::cout << e.what() << std::endl;
        return false;
    }

    // Get the collection of LCRelations between GBL tracks and kink data and track data variables.
    EVENT::LCCollection* gbl_kink_data{nullptr};
    EVENT::LCCollection* track_data{nullptr};
        
    try
    {
        if (!kinkRelCollLcio_.empty())
            gbl_kink_data = static_cast<EVENT::LCCollection*>(event->getLCCollection(kinkRelCollLcio_.c_str()));
        if (!trkRelCollLcio_.empty())
            track_data = static_cast<EVENT::LCCollection*>(event->getLCCollection(trkRelCollLcio_.c_str()));
    }
    catch (EVENT::DataNotAvailableException e)
    {
        std::cout << e.what() << std::endl;
        if (!gbl_kink_data)
            std::cout<<"Failed retrieving " << kinkRelCollLcio_ <<std::endl;
        if (!track_data)
            std::cout<<"Failed retrieving " << trkRelCollLcio_ <<std::endl;
        
    }
    
    
    if (debug_ > 0) std::cout << "FinalStateParticleProcessor: Converting"<< std::endl;

    bool rotateHits = true;
    int hitType = 0;
    EVENT::LCCollection* raw_svt_hit_fits = nullptr;

    auto evColls = event->getLCEvent()->getCollectionNames();
    auto it = std::find (evColls->begin(), evColls->end(), hitFitsCollLcio_.c_str());
    bool hasFits = true;
    if(it == evColls->end()) hasFits = false; 
    if(hasFits)
    {
        raw_svt_hit_fits = event->getLCCollection(hitFitsCollLcio_.c_str()); 
    }
    for (int ifsp = 0 ; ifsp < lc_fsps->getNumberOfElements(); ++ifsp) 
    {
        if (debug_ > 0) std::cout << "FinalStateParticleProcessor: Converting FinalStateParticle " << ifsp << std::endl;
        EVENT::ReconstructedParticle* lc_fsp{nullptr};
        lc_fsp = static_cast<EVENT::ReconstructedParticle*>(lc_fsps->getElementAt(ifsp));
        if (debug_ > 0) std::cout << "FinalStateParticleProcessor: Build Particle" << std::endl;
        
        Particle * fsp = utils::buildParticle(lc_fsp, gbl_kink_data, track_data);
        if (lc_fsp->getTracks().size()>0){
            EVENT::Track* lc_track = static_cast<EVENT::Track*>(lc_fsp->getTracks()[0]);
            Track* track = utils::buildTrack(lc_track,gbl_kink_data,track_data);
            EVENT::TrackerHitVec lc_tracker_hits = lc_track->getTrackerHits(); 
            for (auto lc_tracker_hit : lc_tracker_hits) {
                TrackerHit* tracker_hit = utils::buildTrackerHit(static_cast<IMPL::TrackerHitImpl*>(lc_tracker_hit),rotateHits,hitType);
                std::vector<RawSvtHit*> rawSvthitsOn3d;
                utils::addRawInfoTo3dHit(tracker_hit,static_cast<IMPL::TrackerHitImpl*>(lc_tracker_hit),
                                         raw_svt_hit_fits,&rawSvthitsOn3d,hitType);
                for (auto rhit : rawSvthitsOn3d)
                    rawhits_.push_back(rhit);
                    //rawhits_->addHit(rhit); 

                track->addHit(tracker_hit);
                hits_.push_back(tracker_hit);
                rawSvthitsOn3d.clear();
                // loop on j>i tracks
            }
            fsp->setTrack(track);
        }   
         
        if (debug_ > 0) std::cout << "FinalStateParticleProcessor: Add Particle" << std::endl;
        fsps_.push_back(fsp);
    }

    if (debug_ > 0) std::cout << "FinalStateParticleProcessor: End process" << std::endl;
    return true;
}

void FinalStateParticleProcessor::finalize() { 
}

DECLARE_PROCESSOR(FinalStateParticleProcessor); 
