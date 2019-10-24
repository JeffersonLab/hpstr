#include "RefittedTracksProcessor.h"
#include <iomanip>
#include "utilities.h"


RefittedTracksProcessor::RefittedTracksProcessor(const std::string& name, Process& process)
  : Processor(name, process) { 
  
  _OriginalTrkHistos       = new TrackHistos("trk");
  _RefitTrkHistos          = new TrackHistos("refit");
  _RefitTrkHistos_z0cut    = new TrackHistos("refit_z0");
  _RefitTrkHistos_chi2cut  = new TrackHistos("refit_chi2");
}

RefittedTracksProcessor::~RefittedTracksProcessor() { 
}

void RefittedTracksProcessor::initialize(TTree* tree) {
  
  tree->Branch("GBLRefittedTracks", &tracks_);
  tree->Branch("V0Vertices", &vertices_);
  tree->Branch("V0Vertices_refit", &vertices_refit_);
  
  
  //Original hists
  _OriginalTrkHistos->doTrackComparisonPlots(false);
  _OriginalTrkHistos->Define1DHistos();
  
  //Refit hists
  _RefitTrkHistos->doTrackComparisonPlots(true);
  _RefitTrkHistos->Define1DHistos();
  _RefitTrkHistos->Define2DHistos();

  //Refit hists with z0 closer to 0
  _RefitTrkHistos_z0cut->doTrackComparisonPlots(true);
  _RefitTrkHistos_z0cut->Define1DHistos();
  _RefitTrkHistos_z0cut->Define2DHistos();

  //Refit hists with z0 closer to 0
  _RefitTrkHistos_chi2cut->doTrackComparisonPlots(true);
  _RefitTrkHistos_chi2cut->Define1DHistos();
  _RefitTrkHistos_chi2cut->Define2DHistos();
  
 
}

bool RefittedTracksProcessor::process(IEvent* ievent) {

  tracks_.clear();
  refit_tracks_.clear();
  vertices_.clear();
  vertices_refit_.clear();
    
  Event* event = static_cast<Event*> (ievent);
  //Get all the tracks
  EVENT::LCCollection* tracks       = event->getLCCollection(Collections::GBL_TRACKS);

  //Get all the 3D hits 
  EVENT::LCCollection* trackerHits  = event->getLCCollection(Collections::TRACKER_HITS);
    
  //Get all the rawHits fits
  EVENT::LCCollection* raw_svt_hit_fits = event->getLCCollection(Collections::RAW_SVT_HIT_FITS);

  //Grab the vertices and the vtx candidates
  EVENT::LCCollection* u_vtx_candidates = nullptr;
  EVENT::LCCollection* u_vtxs = nullptr;
  if (utils::hasCollection(event->getLCEvent(),Collections::UC_V0CANDIDATES)) { 
    //Get the vertex candidates
    u_vtx_candidates  = event->getLCCollection(Collections::UC_V0CANDIDATES);
    //Get the vertices 
    u_vtxs = event->getLCCollection(Collections::UC_V0VERTICES);
  
    
    for (int ivtx = 0 ; ivtx < u_vtxs->getNumberOfElements(); ++ivtx) {
      Vertex* vtx = utils::buildVertex(static_cast<EVENT::Vertex*>(u_vtxs->getElementAt(ivtx)));
      vertices_.push_back(vtx);
      _OriginalTrkHistos->Fill1DHistograms(nullptr,vtx);
    }
    _OriginalTrkHistos->Fill1DHisto("n_vertices",u_vtxs->getNumberOfElements());
  }

  //Grab the vertices and the vtx candidates
  EVENT::LCCollection* u_vtx_candidates_r = nullptr;
  EVENT::LCCollection* u_vtxs_r = nullptr;
  if (utils::hasCollection(event->getLCEvent(),"UnconstrainedV0Candidates_refit")) { 
    //Get the vertex candidates
    u_vtx_candidates_r  = event->getLCCollection("UnconstrainedV0Candidates_refit");
    //Get the vertices 
    u_vtxs_r = event->getLCCollection("UnconstrainedV0Vertices_refit");
    
    for (int ivtx = 0 ; ivtx < u_vtxs_r->getNumberOfElements(); ++ivtx) {
      Vertex* vtx_r = utils::buildVertex(static_cast<EVENT::Vertex*>(u_vtxs_r->getElementAt(ivtx)));
      vertices_refit_.push_back(vtx_r);
      _RefitTrkHistos->Fill1DHistograms(nullptr,vtx_r);
    }
    _RefitTrkHistos->Fill1DHisto("n_vertices",u_vtxs_r->getNumberOfElements());
  }
    
    
    //Initialize map of shared hits
  std::map <int, std::vector<int> > SharedHits;
  //TODO: can we do better? (innermost)
  std::map <int, bool> SharedHitsLy0;
  std::map <int, bool> SharedHitsLy1;
  
  for (int itrack = 0; itrack < tracks->getNumberOfElements();++itrack) {
    SharedHits[itrack]   = {};
    SharedHitsLy0[itrack] = false;
    SharedHitsLy1[itrack] = false;
  }
  

  _OriginalTrkHistos->Fill1DHisto("n_tracks",tracks->getNumberOfElements());
  // Loop over all the LCIO Tracks and add them to the HPS event.
  for (int itrack = 0; itrack < tracks->getNumberOfElements(); ++itrack) {
    
    // Get a LCIO Track from the LCIO event
    EVENT::Track* lc_track = static_cast<EVENT::Track*>(tracks->getElementAt(itrack));
    
    // Get the GBL kink data
    EVENT::LCCollection* gbl_kink_data = 
      static_cast<EVENT::LCCollection*>(event->getLCCollection(Collections::KINK_DATA_REL));

    // Get the track data
    EVENT::LCCollection* track_data = static_cast<EVENT::LCCollection*>(event->getLCCollection(Collections::TRACK_DATA_REL));
    
    // Add a track to the event
    Track* track = utils::buildTrack(lc_track, gbl_kink_data, track_data);
        

    //Get the refitted tracks relations

    EVENT::LCCollection* refitted_tracks_rel = 
      static_cast<EVENT::LCCollection*>(event->getLCCollection("GBLTrackToGBLTrackRefitRelations"));

    //Build the navigator
    UTIL::LCRelationNavigator* refitted_tracks_nav = new UTIL::LCRelationNavigator(refitted_tracks_rel);
    

    //Get the list of data
    EVENT::LCObjectVec refitted_tracks_list = refitted_tracks_nav -> getRelatedToObjects(lc_track);

    //std::cout<<"========================================="<<std::endl;
    //std::cout<<"========================================="<<std::endl;
    //std::cout<<"Track params:           "<<std::endl;
    //track->Print();
    
    //Get the tracker hits
    EVENT::TrackerHitVec lc_tracker_hits = lc_track->getTrackerHits();
    

    //Build the vector of Tracker Hits on track, get the info and attach them to the track. 
    for (int ith = 0; ith<lc_tracker_hits.size();ith++) {
      IMPL::TrackerHitImpl* lc_th = static_cast<IMPL::TrackerHitImpl*>(lc_tracker_hits.at(ith));
      TrackerHit* th = utils::buildTrackerHit(lc_th);
      //TODO should check the status of this return
      utils::addRawInfoTo3dHit(th,lc_th,raw_svt_hit_fits);
      //TODO this should be under some sort of saving flag
      track->addHit(th);
      hits_.push_back(th);

      //Get shared Hits information
      for (int jtrack = itrack+1; jtrack < tracks->getNumberOfElements(); ++jtrack) {
	
	EVENT::Track* j_lc_track = static_cast<EVENT::Track*>(tracks->getElementAt(jtrack));
	if (utils::isUsedByTrack(th,j_lc_track)) {
	  //The hit is not already in the shared list
	  if (std::find(SharedHits[itrack].begin(), SharedHits[itrack].end(),th->getID()) == SharedHits[itrack].end()) {
	    SharedHits[itrack].push_back(th->getID());
	    if (th->getLayer() == 0 )
	      SharedHitsLy0[itrack] = true;
	    if (th->getLayer() == 1 ) 
	      SharedHitsLy1[itrack] = true;
	  }
	  if (std::find(SharedHits[jtrack].begin(), SharedHits[jtrack].end(),th->getID()) == SharedHits[jtrack].end()) {
	    SharedHits[jtrack].push_back(th->getID());
	    if (th->getLayer() == 0 ) 
	      SharedHitsLy0[jtrack] = true;
	    if (th->getLayer() == 1 ) 
	      SharedHitsLy1[jtrack] = true;
	  }
	} // found shared hit
      } // loop on j>i tracks
    } // loop on hits on track i

    //TODO:: bug prone?
    track->setNShared(SharedHits[itrack].size());
    track->setSharedLy0(SharedHitsLy0[itrack]);
    track->setSharedLy1(SharedHitsLy1[itrack]);
    
    //std::cout<<"Tracker hits time:";
    //for (auto lc_tracker_hit : lc_tracker_hits) { 
    //std::cout<<" "<<lc_tracker_hit->getTime();
    //}
    //std::cout<<std::endl;

    if (refitted_tracks_list.size() < 1)
      return false; 


    //Get only best X2 refit track
    int bestX2index = -999;
    float bestX2 = -999;
    for (int irtrk = 0; irtrk<refitted_tracks_list.size(); irtrk++) {
      EVENT::Track* lc_rfit_track = static_cast<EVENT::Track*>(refitted_tracks_list.at(irtrk));
      if (irtrk == 0) {
	bestX2 = (lc_rfit_track->getChi2() / lc_rfit_track->getNdf());
	bestX2index = 0;
      }
      else
	if ((lc_rfit_track->getChi2() / lc_rfit_track->getNdf()) < bestX2) {
	  bestX2 = (lc_rfit_track->getChi2() / lc_rfit_track->getNdf());
	  bestX2index = irtrk;
	}
    }

    _OriginalTrkHistos->Fill1DHistograms(track);
    
    _RefitTrkHistos->Fill1DHisto("n_tracks",refitted_tracks_list.size());
    
    for (int irtrk = 0; irtrk < refitted_tracks_list.size(); irtrk++) {
      
      if (irtrk != bestX2index)
	continue;
      
      EVENT::Track* lc_rfit_track = static_cast<EVENT::Track*>(refitted_tracks_list.at(irtrk));

      //Useless to get it every track
      // Get the GBL kink data
      EVENT::LCCollection* rfit_gbl_kink_data = 
	static_cast<EVENT::LCCollection*>(event->getLCCollection("GBLKinkDataRelations_refit"));
      // Get the track data
      EVENT::LCCollection* rfit_track_data = nullptr;
      Track* rfit_track = utils::buildTrack(lc_rfit_track,rfit_gbl_kink_data,rfit_track_data);
      EVENT::TrackerHitVec lc_rf_tracker_hits = lc_rfit_track->getTrackerHits();
      
      //TODO::move to utilities
      //recompute time if missing information
      float mean = 0.0;
      for (auto lc_rf_tracker_hit : lc_rf_tracker_hits) {
	mean += lc_rf_tracker_hit->getTime();
      }
      rfit_track->setTrackTime(mean / lc_rf_tracker_hits.size());
      
      //std::cout<<"Refitted tracks params: "<<std::endl;
      //rfit_track->Print();      
      
      //std::cout<<"Tracker hits time:";
      //for (auto lc_rf_tracker_hit : lc_rf_tracker_hits) { 
      //std::cout<<" "<<lc_rf_tracker_hit->getTime();
      //}
      //std::cout<<std::endl;
      
      _RefitTrkHistos->Fill1DHistograms(rfit_track);
      _RefitTrkHistos->FillTrackComparisonHistograms(track,rfit_track);
   
      if (fabs(rfit_track->getZ0()) < fabs(track->getZ0())) {
	_RefitTrkHistos_z0cut->Fill1DHistograms(rfit_track);
	_RefitTrkHistos_z0cut->FillTrackComparisonHistograms(track,rfit_track);
      }

      if (rfit_track->getChi2Ndf() < track->getChi2Ndf()) {
	_RefitTrkHistos_chi2cut->Fill1DHistograms(rfit_track);
	_RefitTrkHistos_chi2cut->FillTrackComparisonHistograms(track,rfit_track);
      }
    }//loop on refit tracks
  }//Loop on tracks
  
  
  
  return true;
}

void RefittedTracksProcessor::finalize() { 

  _OriginalTrkHistos      ->saveHistos();
  _RefitTrkHistos         ->saveHistos();
  _RefitTrkHistos_z0cut   ->saveHistos();
  _RefitTrkHistos_chi2cut ->saveHistos();
  
}

DECLARE_PROCESSOR(RefittedTracksProcessor); 
