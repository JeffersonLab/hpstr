#include "RefittedTracksProcessor.h"

RefittedTracksProcessor::RefittedTracksProcessor(const std::string& name, Process& process)
  : Processor(name, process) { 
}

RefittedTracksProcessor::~RefittedTracksProcessor() { 
}

void RefittedTracksProcessor::initialize(TTree* tree) {
  tree->Branch(Collections::GBL_TRACKS,&tracks_);
  tree->Branch("GBLRefittedTracks", &refit_tracks_);
  
}

bool RefittedTracksProcessor::process(IEvent* ievent) {

  Event* event = static_cast<Event*> (ievent);
  //Get all ethe tracks
  EVENT::LCCollection* tracks       = event->getLCCollection(Collections::GBL_TRACKS);
    
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


    //Get the refitted tracks relations

    EVENT::LCCollection* refitted_tracks_rel = 
      static_cast<EVENT::LCCollection*>(event->getLCCollection("GBLTrackToGBLTrackRefitRelations"));

    //Build the navigator
    UTIL::LCRelationNavigator* refitted_tracks_nav = new UTIL::LCRelationNavigator(refitted_tracks_rel);

    //Get the list of data
    EVENT::LCObjectVec refitted_tracks_list = refitted_tracks_nav -> getRelatedFromObjects(lc_track);

    //print the size!
    //std::cout<<"For track: "<<itrack<<" refitted_tracks_list size:"<<refitted_tracks_list.size()<<std::endl;
					
    

  }//Loop on tracks
  
  
  
  return true;
}

void RefittedTracksProcessor::finalize() { 
}

DECLARE_PROCESSOR(RefittedTracksProcessor); 
