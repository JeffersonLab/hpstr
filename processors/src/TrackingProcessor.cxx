
#include "TrackingProcessor.h" 
#include "utilities.h"

TrackingProcessor::TrackingProcessor(const std::string& name, Process& process)
    : Processor(name, process) { 
    }

TrackingProcessor::~TrackingProcessor() { 
}

void TrackingProcessor::initialize(TTree* tree) {
    tree->Branch(Collections::GBL_TRACKS, &tracks_);
    tree->Branch(Collections::TRACKER_HITS_ON_TRACK, &hits_);
    tree->Branch(Collections::RAW_SVT_HITS_ON_TRACK, &rawhits_);

}

bool TrackingProcessor::process(IEvent* ievent) {


    //Clean up
    if (tracks_.size() > 0 ) {
        for (std::vector<Track *>::iterator it = tracks_.begin(); it != tracks_.end(); ++it) {
            delete *it;
        }
        tracks_.clear();
    }

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


    tracks_.clear();
    hits_.clear();
    rawhits_.clear();

    Event* event = static_cast<Event*> (ievent);
    // Get the collection of 3D hits from the LCIO event. If no such collection 
    // exist, a DataNotAvailableException is thrown

    // Get decoders to read cellids
    UTIL::BitField64 decoder("system:6,barrel:3,layer:4,module:12,sensor:1,side:32:-2,strip:12");

    UTIL::LCRelationNavigator* rawTracker_hit_fits_nav = nullptr;
    EVENT::LCCollection* raw_svt_hit_fits              = nullptr;
    //Check to see if fits are in the file
    auto evColls = event->getLCEvent()->getCollectionNames();
    auto it = std::find (evColls->begin(), evColls->end(), Collections::RAW_SVT_HIT_FITS);
    bool hasFits = true;
    if(it == evColls->end()) hasFits = false;
    if(hasFits) 
    {
        raw_svt_hit_fits = event->getLCCollection(Collections::RAW_SVT_HIT_FITS); 
        // Heap an LCRelation navigator which will allow faster access 
        rawTracker_hit_fits_nav = new UTIL::LCRelationNavigator(raw_svt_hit_fits);     
    }

    // Get all track collections from the event
    EVENT::LCCollection* tracks = event->getLCCollection(Collections::GBL_TRACKS);


    // Loop over all the LCIO Tracks and add them to the HPS event.
    for (int itrack = 0; itrack < tracks->getNumberOfElements(); ++itrack) {

        // Get a LCIO Track from the LCIO event
        EVENT::Track* lc_track = static_cast<EVENT::Track*>(tracks->getElementAt(itrack));

        // Get the collection of LCRelations between GBL kink data variables 
        // (GBLKinkData) and the corresponding track.
        EVENT::LCCollection* gbl_kink_data = 
            static_cast<EVENT::LCCollection*>(event->getLCCollection(Collections::KINK_DATA_REL));

        // Get the collection of LCRelations between track data variables 
        // (TrackData) and the corresponding track.
        EVENT::LCCollection* track_data = static_cast<EVENT::LCCollection*>(
                event->getLCCollection(Collections::TRACK_DATA_REL));


        // Add a track to the event
        Track* track = utils::buildTrack(lc_track,gbl_kink_data,track_data);


        // Get the collection of 3D hits associated with a LCIO Track
        EVENT::TrackerHitVec lc_tracker_hits = lc_track->getTrackerHits();

        //  Iterate through the collection of 3D hits (TrackerHit objects)
        //  associated with a track, find the corresponding hits in the HPS
        //  event and add references to the track


        for (auto lc_tracker_hit : lc_tracker_hits) {

            TrackerHit* tracker_hit = utils::buildTrackerHit(static_cast<IMPL::TrackerHitImpl*>(lc_tracker_hit));

            std::vector<RawSvtHit*> rawSvthitsOn3d;
            utils::addRawInfoTo3dHit(tracker_hit,static_cast<IMPL::TrackerHitImpl*>(lc_tracker_hit),
                    raw_svt_hit_fits,&rawSvthitsOn3d);


            for (auto rhit : rawSvthitsOn3d)
                rawhits_.push_back(rhit);

            rawSvthitsOn3d.clear();

            if (_debug)
                std::cout<<tracker_hit->getRawHits()->GetEntries()<<std::endl;
            // Add a reference to the hit
            track->addHit(tracker_hit);
            hits_.push_back(tracker_hit);
        }//tracker hits
        tracks_.push_back(track);
    }// tracks

    //delete
    if (rawTracker_hit_fits_nav) {
        delete rawTracker_hit_fits_nav; rawTracker_hit_fits_nav = nullptr;}

    //event->addCollection("TracksInfo",   &tracks_);
    //event->addCollection("TrackerHitsInfo", &hits_); 
    //event->addCollection("TrackerHitsRawInfo",     &rawhits_);

    return true;
}

void TrackingProcessor::finalize() { 
}

DECLARE_PROCESSOR(TrackingProcessor); 
