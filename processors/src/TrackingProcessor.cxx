
#include "TrackingProcessor.h" 

TrackingProcessor::TrackingProcessor(const std::string& name, Process& process)
  : Processor(name, process) { 
}

TrackingProcessor::~TrackingProcessor() { 
}

void TrackingProcessor::initialize(TTree* tree) {
  tree->Branch(Collections::GBL_TRACKS,&tracks_);
  tree->Branch(Collections::TRACKER_HITS, &hits_);
  tree->Branch(Collections::RAW_SVT_HITS, &rawhits_);
  
}

bool TrackingProcessor::process(IEvent* ievent) {


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
      throw std::runtime_error("[ TrackingProcessor ]: The collection " 
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
      if (track_datum->getNDouble() > 14 || track_datum->getNFloat() != 1 
	  || track_datum->getNInt() != 1) {
	throw std::runtime_error("[ TrackingProcessor ]: The collection " 
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
    for (auto lc_tracker_hit : lc_tracker_hits) {


      TrackerHit* tracker_hit = new TrackerHit();
      // Get the position of the LCIO TrackerHit and set the position of 
      // the TrackerHit
      double hit_position[3] = { 
				lc_tracker_hit->getPosition()[0], 
				lc_tracker_hit->getPosition()[1], 
				lc_tracker_hit->getPosition()[2]
      };
      tracker_hit->setPosition(hit_position, true);
      
      // Set the covariance matrix of the SvtHit
      tracker_hit->setCovarianceMatrix(lc_tracker_hit->getCovMatrix());
      
      // Set the time of the SvtHit
      tracker_hit->setTime(lc_tracker_hit->getTime());
      
      // Set the charge of the SvtHit
      tracker_hit->setCharge(lc_tracker_hit->getEDep());
      
      
      //Get the Raw content of the tracker hits
      EVENT::LCObjectVec rawHits             = lc_tracker_hit->getRawHits();
      
      //Return 0
      //EVENT::LCObjectVec rawHits_navigated   = tracker_hits_to_raw_nav->getRelatedToObjects(lc_tracker_hit);

      if (_debug)
	std::cout<<"Raw Hit Size::"<< rawHits.size()<<std::endl;

      
      
      //TODO Change with iterator
      for (unsigned int irh =0; irh<rawHits.size(); ++irh) {
	
	EVENT::TrackerRawData* rawTracker_hit = static_cast<EVENT::TrackerRawData*>(rawHits.at(irh));
	
	EVENT::long64 value =
	  EVENT::long64(rawTracker_hit->getCellID0() & 0xffffffff) |
	( EVENT::long64(rawTracker_hit->getCellID1() ) << 32       );
	decoder.setValue(value);
	
	
	RawSvtHit* rawHit = new RawSvtHit();
	rawHit->setSystem(decoder["system"]);
	rawHit->setBarrel(decoder["barrel"]);
	rawHit->setLayer(decoder["layer"]);
	rawHit->setModule(decoder["module"]);
	rawHit->setSensor(decoder["sensor"]);
	rawHit->setSide(decoder["side"]);
	rawHit->setStrip(decoder["strip"]);
	
	// Extract ADC values for this hit
	int hit_adcs[6] = { 
			   (int)rawTracker_hit->getADCValues().at(0), 
			   (int)rawTracker_hit->getADCValues().at(1), 
			   (int)rawTracker_hit->getADCValues().at(2), 
			   (int)rawTracker_hit->getADCValues().at(3), 
			   (int)rawTracker_hit->getADCValues().at(4), 
			   (int)rawTracker_hit->getADCValues().at(5)};
	
	rawHit->setADCs(hit_adcs);

	if (hasFits)
        {
            // Get the list of fit params associated with the raw tracker hit
            EVENT::LCObjectVec rawTracker_hit_fits_list
                = rawTracker_hit_fits_nav->getRelatedToObjects(rawTracker_hit);

            // Get the list SVTFittedRawTrackerHit GenericObject associated with the SVTRawTrackerHit
            IMPL::LCGenericObjectImpl* hit_fit_param
                = static_cast<IMPL::LCGenericObjectImpl*>(rawTracker_hit_fits_list.at(0));

            double fit_params[5] = { 
                (double)hit_fit_param->getDoubleVal(0), 
                (double)hit_fit_param->getDoubleVal(1), 
                (double)hit_fit_param->getDoubleVal(2), 
                (double)hit_fit_param->getDoubleVal(3), 
                (double)hit_fit_param->getDoubleVal(4)
            };

            rawHit->setFit(fit_params);
        }
	
	tracker_hit->addRawHit(rawHit);
	rawhits_.push_back(rawHit);
	if (_debug)
	  std::cout<<"RawHit ID:"<<rawTracker_hit->id()<<std::endl;
      } //raw hits
      
      if (_debug)
	std::cout<<tracker_hit->getRawHits()->GetEntries()<<std::endl;
      // Add a reference to the hit
      track->addHit(tracker_hit);
      hits_.push_back(tracker_hit);
    }//tracker hits
    tracks_.push_back(track);
  }// tracks
  

  //event->addCollection("TracksInfo",   &tracks_);
  //event->addCollection("TrackerHitsInfo", &hits_); 
  //event->addCollection("TrackerHitsRawInfo",     &rawhits_);
  

  return true;
}

void TrackingProcessor::finalize() { 
}

DECLARE_PROCESSOR(TrackingProcessor); 
