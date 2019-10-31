
#include "SvtDataProcessor.h" 

SvtDataProcessor::SvtDataProcessor(const std::string& name, Process& process)
    : Processor(name, process) { 
}

SvtDataProcessor::~SvtDataProcessor() { 
}

void SvtDataProcessor::initialize(TTree* tree) {
    // Add branches to tree
    tree->Branch(Collections::GBL_TRACKS, &tracks_);
    tree->Branch(Collections::TRACKER_HITS, &hits_);
}

bool SvtDataProcessor::process(IEvent* ievent) {

    for(int i = 0; i < tracks_.size(); i++) delete tracks_.at(i);
    for(int i = 0; i < hits_.size(); i++) delete hits_.at(i);
    tracks_.clear();
    hits_.clear();

    Event* event = static_cast<Event*> (ievent);

    // Get the collection of 3D hits from the LCIO event. If no such collection 
    // exist, a DataNotAvailableException is thrown
    EVENT::LCCollection* tracker_hits = event->getLCCollection(Collections::TRACKER_HITS); 

    // Create a map from an LCIO TrackerHit to a SvtHit. This will be used when
    // assigning references to a track
    // TODO: Use an unordered map for faster access
    std::map<EVENT::TrackerHit*, TrackerHit*> hit_map;

    // Loop over all of the 3D hits in the LCIO event and add them to the 
    // HPS event
    for (int ihit = 0; ihit < tracker_hits->getNumberOfElements(); ++ihit) { 
        
        // Get a 3D hit from the list of hits
        IMPL::TrackerHitImpl* lc_tracker_hit = static_cast<IMPL::TrackerHitImpl*>(tracker_hits->getElementAt(ihit));
    
        // Add a tracker hit to the event
        TrackerHit* tracker_hit = new TrackerHit();

        // Rotate the position of the LCIO TrackerHit and set the position of 
        // the TrackerHit
        double hit_position[3] = { 
            lc_tracker_hit->getPosition()[1], 
            lc_tracker_hit->getPosition()[2], 
            lc_tracker_hit->getPosition()[0]
        };
        tracker_hit->setPosition(hit_position);
    
        // Set the covariance matrix of the SvtHit
        tracker_hit->setCovarianceMatrix(lc_tracker_hit->getCovMatrix());
                
        // Set the time of the SvtHit
        tracker_hit->setTime(lc_tracker_hit->getTime());

        // Set the charge of the SvtHit
        tracker_hit->setCharge(lc_tracker_hit->getEDep());

        // Map the TrackerHit object to the corresponding SvtHit object. This
        // will be used later when setting references for hits on tracks.
        hit_map[lc_tracker_hit] = tracker_hit; 
        hits_.push_back(tracker_hit);
        
    }

    // Add the hit collection to the event
    //event->addCollection(Collections::TRACKER_HITS, hits_); 
   
    // Get all track collections from the event
    EVENT::LCCollection* tracks = event->getLCCollection(Collections::GBL_TRACKS);

     
    // Loop over all the LCIO Tracks and add them to the HPS event.
    for (int itrack = 0; itrack < tracks->getNumberOfElements(); ++itrack) {
          
        // Get a LCIO Track from the LCIO event
        EVENT::Track* lc_track = static_cast<EVENT::Track*>(tracks->getElementAt(itrack));

        // Add a track to the event
        Track* track = new Track();
         
        // Set the track parameters
        track->setTrackParameters(lc_track->getD0(), 
                                  lc_track->getPhi(), 
                                  lc_track->getOmega(), 
                                  lc_track->getTanLambda(), 
                                  lc_track->getZ0());

        // Set the track type
        track->setType(lc_track->getType()); 

        // Set the track fit chi^2
        track->setChi2(lc_track->getChi2());

        // Set the position of the extrapolated track at the ECal face.  The
        // extrapolation uses the full 3D field map.
        const EVENT::TrackState* track_state 
            = lc_track->getTrackState(EVENT::TrackState::AtCalorimeter); 
        double position_at_ecal[3] = { 
            track_state->getReferencePoint()[1],  
            track_state->getReferencePoint()[2],  
            track_state->getReferencePoint()[0]
        };
        track->setPositionAtEcal(position_at_ecal); 

        // Get the collection of LCRelations between GBL kink data variables 
        // (GBLKinkData) and the corresponding track.
        EVENT::LCCollection* gbl_kink_data = 
            static_cast<EVENT::LCCollection*>(event->getLCCollection(Collections::KINK_DATA_REL));

        // Instantiate an LCRelation navigator which will allow faster access 
        // to GBLKinkData object
        UTIL::LCRelationNavigator* gbl_kink_data_nav 
            = new UTIL::LCRelationNavigator(gbl_kink_data);

        // Get the list of GBLKinkData associated with the LCIO Track
        EVENT::LCObjectVec gbl_kink_data_list 
            = gbl_kink_data_nav->getRelatedFromObjects(lc_track);

        // The container of GBLKinkData objects should only contain a 
        // single object. If not, throw an exception
        if (gbl_kink_data_list.size() != 1) { 
            throw std::runtime_error("[ SvtDataProcessor ]: The collection " 
                    + std::string(Collections::TRACK_DATA_REL)
                    + " has the wrong data structure."); 
        }

        // Get the list GBLKinkData GenericObject associated with the LCIO Track
        IMPL::LCGenericObjectImpl* gbl_kink_datum 
            = static_cast<IMPL::LCGenericObjectImpl*>(gbl_kink_data_list.at(0));

        // Set the lambda and phi kink values
        for (int ikink = 0; ikink < gbl_kink_datum->getNDouble(); ++ikink) { 
            track->setLambdaKink(ikink, gbl_kink_datum->getFloatVal(ikink));
            track->setPhiKink(ikink, gbl_kink_datum->getDoubleVal(ikink));
        }

        delete gbl_kink_data_nav; 

        // Get the collection of LCRelations between track data variables 
        // (TrackData) and the corresponding track.
        EVENT::LCCollection* track_data = static_cast<EVENT::LCCollection*>(
                event->getLCCollection(Collections::TRACK_DATA_REL));

        // Instantiate an LCRelation navigator which will allow faster access
        // to TrackData objects  
        UTIL::LCRelationNavigator* track_data_nav 
            = new UTIL::LCRelationNavigator(track_data);

        // Get the list of TrackData associated with the LCIO Track
        EVENT::LCObjectVec track_data_list = track_data_nav->getRelatedFromObjects(lc_track);

        // The container of TrackData objects should only contain a single
        //  object.  If not, throw an exception.
        if (track_data_list.size() == 1) { 
    
            // Get the TrackData GenericObject associated with the LCIO Track
            IMPL::LCGenericObjectImpl* track_datum = static_cast<IMPL::LCGenericObjectImpl*>(track_data_list.at(0));
                
            // Check that the TrackData data structure is correct.  If it's
            // not, throw a runtime exception.   
            if (track_datum->getNDouble() < 12 || track_datum->getNFloat() != 1 
                    || track_datum->getNInt() != 1) {
                throw std::runtime_error("[ SvtDataProcessor ]: The collection " 
                        + std::string(Collections::TRACK_DATA)
                        + " has the wrong structure.");
            }
    
            // Set the SvtTrack isolation values
            for (int iso_index = 0; iso_index < track_datum->getNDouble(); ++iso_index) { 
                track->setIsolation(iso_index, track_datum->getDoubleVal(iso_index));
            }
                
            // Set the SvtTrack time
            track->setTrackTime(track_datum->getFloatVal(0));

            // Set the volume (top/bottom) in which the SvtTrack resides
            track->setTrackVolume(track_datum->getIntVal(0));
        }

        delete track_data_nav; 
    
        // Get the collection of 3D hits associated with a LCIO Track
        EVENT::TrackerHitVec lc_tracker_hits = lc_track->getTrackerHits();

        //  Iterate through the collection of 3D hits (TrackerHit objects)
        //  associated with a track, find the corresponding hits in the HPS
        //  event and add references to the track
        for (auto hit : lc_tracker_hits) { 
                
            // Add a reference to the hit
            track->addHit(hit_map[hit]);  
        }
        tracks_.push_back(track);
    }

    //event->addCollection(Collections::GBL_TRACKS, tracks_); 

    return true;
}

void SvtDataProcessor::finalize() { 
}

DECLARE_PROCESSOR(SvtDataProcessor); 
