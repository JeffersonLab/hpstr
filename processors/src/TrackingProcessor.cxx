
#include "TrackingProcessor.h" 
#include "utilities.h"

TrackingProcessor::TrackingProcessor(const std::string& name, Process& process)
    : Processor(name, process) { 
    }

TrackingProcessor::~TrackingProcessor() { 
}

void TrackingProcessor::configure(const ParameterSet& parameters) {

    std::cout << "Configuring TrackingProcessor" << std::endl;
    try
    {
        debug_                   = parameters.getInteger("debug", debug_);
        trkCollLcio_             = parameters.getString("trkCollLcio", trkCollLcio_);
        trkCollRoot_             = parameters.getString("trkCollRoot", trkCollRoot_);
        kinkRelCollLcio_         = parameters.getString("kinkRelCollLcio", kinkRelCollLcio_);
        trkRelCollLcio_          = parameters.getString("trkRelCollLcio", trkRelCollLcio_);
        trkhitCollRoot_          = parameters.getString("trkhitCollRoot", trkhitCollRoot_);
        hitFitsCollLcio_         = parameters.getString("hitFitsCollLcio", hitFitsCollLcio_);
        rawhitCollRoot_          = parameters.getString("rawhitCollRoot", rawhitCollRoot_);
        truthTracksCollLcio_     = parameters.getString("truthTrackCollLcio",truthTracksCollLcio_);
        truthTracksCollRoot_     = parameters.getString("truthTrackCollRoot",truthTracksCollRoot_);
        bfield_                  = parameters.getDouble("bfield",bfield_);

        //Residual plotting is done in this processor for the moment.
        doResiduals_             = parameters.getInteger("doResiduals",doResiduals_);
        trackResDataLcio_        = parameters.getString("trackResDataLcio",trackResDataLcio_);
        resCfgFilename_          = parameters.getString("resPlots",resCfgFilename_);
        resoutname_              = parameters.getString("resoutname",resoutname_);

    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

}

void TrackingProcessor::initialize(TTree* tree) {
    tree->Branch(trkCollRoot_.c_str(), &tracks_);
    
    if (!trkhitCollRoot_.empty())
        tree->Branch(trkhitCollRoot_.c_str(), &hits_);
    
    if (!rawhitCollRoot_.empty())
        tree->Branch(rawhitCollRoot_.c_str(), &rawhits_);
    
    if (!truthTracksCollRoot_.empty())
        tree->Branch(truthTracksCollRoot_.c_str(),&truthTracks_);


    //Residual plotting
    if (doResiduals_) {
        trkResHistos_ = new TrackHistos(trkCollLcio_);
        trkResHistos_->debugMode(debug_);
        trkResHistos_->loadHistoConfig(resCfgFilename_);
        trkResHistos_->doTrackComparisonPlots(false);
        trkResHistos_->DefineHistos();
    }
    
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
    
    if (truthTracks_.size() > 0) {
        for (std::vector<Track *>::iterator it = truthTracks_.begin(); it != truthTracks_.end(); ++it) {
            delete *it;
        }
        truthTracks_.clear();
    }
    
    Event* event = static_cast<Event*> (ievent);
    // Get the collection of 3D hits from the LCIO event. If no such collection 
    // exist, a DataNotAvailableException is thrown
    
    // Get decoders to read cellids
    UTIL::BitField64 decoder("system:6,barrel:3,layer:4,module:12,sensor:1,side:32:-2,strip:12");

    UTIL::LCRelationNavigator* rawTracker_hit_fits_nav = nullptr;
    EVENT::LCCollection* raw_svt_hit_fits              = nullptr;
    //Check to see if fits are in the file
    auto evColls = event->getLCEvent()->getCollectionNames();
    auto it = std::find (evColls->begin(), evColls->end(), hitFitsCollLcio_.c_str());
    bool hasFits = true;
    if(it == evColls->end()) hasFits = false;
    if(hasFits) 
    {
        raw_svt_hit_fits = event->getLCCollection(hitFitsCollLcio_.c_str()); 
        // Heap an LCRelation navigator which will allow faster access 
        rawTracker_hit_fits_nav = new UTIL::LCRelationNavigator(raw_svt_hit_fits);     
    }

    EVENT::LCCollection* tracks{nullptr};
    try
    {
        // Get all track collections from the event
        tracks = event->getLCCollection(trkCollLcio_.c_str());
    }
    catch (EVENT::DataNotAvailableException e)
    {
        std::cout << e.what() << std::endl;
        return false;
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
    
    // Loop over all the LCIO Tracks and add them to the HPS event.
    for (int itrack = 0; itrack < tracks->getNumberOfElements(); ++itrack) {

        // Get a LCIO Track from the LCIO event
        EVENT::Track* lc_track = static_cast<EVENT::Track*>(tracks->getElementAt(itrack));

        // Get the collection of LCRelations between GBL kink data and track data variables 
        // and the corresponding track.
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
                std::cout<<"TrackingProcessor::Failed retrieving " << kinkRelCollLcio_ <<std::endl;
            if (!track_data)
                std::cout<<"TrackingProcessor::Failed retrieving " << trkRelCollLcio_ <<std::endl;
            
        }

        // Add a track to the event
        Track* track = utils::buildTrack(lc_track,gbl_kink_data,track_data);
        
        //Override the momentum of the track if the bfield_ > 0
        if (bfield_>0)
            track->setMomentum(bfield_);
	
        // Get the collection of hits associated with a LCIO Track
        EVENT::TrackerHitVec lc_tracker_hits = lc_track->getTrackerHits();

        //  Iterate through the collection of 3D hits (TrackerHit objects)
        //  associated with a track, find the corresponding hits in the HPS
        //  event and add references to the track
        bool rotateHits = true;
        int hitType = 0;
        if (track->isKalmanTrack())
            hitType=1; //SiClusters
        
        for (auto lc_tracker_hit : lc_tracker_hits) {
            
            TrackerHit* tracker_hit = utils::buildTrackerHit(static_cast<IMPL::TrackerHitImpl*>(lc_tracker_hit),rotateHits,hitType);
            
            std::vector<RawSvtHit*> rawSvthitsOn3d;
            utils::addRawInfoTo3dHit(tracker_hit,static_cast<IMPL::TrackerHitImpl*>(lc_tracker_hit),
                                     raw_svt_hit_fits,&rawSvthitsOn3d,hitType);
            
            for (auto rhit : rawSvthitsOn3d)
                rawhits_.push_back(rhit);

            rawSvthitsOn3d.clear();

            if (debug_)
                std::cout<<tracker_hit->getRawHits()->GetEntries()<<std::endl;
            // Add a reference to the hit
            track->addHit(tracker_hit);
            hits_.push_back(tracker_hit);
            
            //Get shared Hits information
            for (int jtrack = itrack+1; jtrack < tracks->getNumberOfElements(); ++jtrack) {
                
                EVENT::Track* j_lc_track = static_cast<EVENT::Track*>(tracks->getElementAt(jtrack));
                if (utils::isUsedByTrack(tracker_hit,j_lc_track)) {
                    //The hit is not already in the shared list
                    if (std::find(SharedHits[itrack].begin(), SharedHits[itrack].end(),tracker_hit->getID()) == SharedHits[itrack].end()) {
                        SharedHits[itrack].push_back(tracker_hit->getID());
                        if (tracker_hit->getLayer() == 0 )
                            SharedHitsLy0[itrack] = true;
                        if (tracker_hit->getLayer() == 1 ) 
                            SharedHitsLy1[itrack] = true;
                    }
                    if (std::find(SharedHits[jtrack].begin(), SharedHits[jtrack].end(),tracker_hit->getID()) == SharedHits[jtrack].end()) {
                        SharedHits[jtrack].push_back(tracker_hit->getID());
                        if (tracker_hit->getLayer() == 0 ) 
                            SharedHitsLy0[jtrack] = true;
                        if (tracker_hit->getLayer() == 1 ) 
                            SharedHitsLy1[jtrack] = true;
                    }
                } // found shared hit
            } // loop on j>i tracks
        }//tracker hits
        
        track->setNShared(SharedHits[itrack].size());
        track->setSharedLy0(SharedHitsLy0[itrack]);
        track->setSharedLy1(SharedHitsLy1[itrack]);
        

        //Get the truth tracks relations:
        
        // Get the collection of LCRelations between GBL kink data and track data variables 
        // and the corresponding track.
        EVENT::LCCollection* truth_tracks_rel{nullptr};
        
        try
        {
            if (!truthTracksCollLcio_.empty())
                truth_tracks_rel = static_cast<EVENT::LCCollection*>(event->getLCCollection(truthTracksCollLcio_.c_str()));
        }
        catch (EVENT::DataNotAvailableException e)
        {
            std::cout << e.what() << std::endl;
            if (!truth_tracks_rel)
                std::cout<<"Failed retrieving " << truthTracksCollLcio_ <<std::endl;
        }
        

        if (truth_tracks_rel) { 
            
            std::shared_ptr<UTIL::LCRelationNavigator> truth_tracks_nav = std::make_shared<UTIL::LCRelationNavigator>(truth_tracks_rel);
            //Get the truth_track associated with the lcio_track
            EVENT::LCObjectVec lc_truth_tracks = truth_tracks_nav->getRelatedToObjects(lc_track);
            if (lc_truth_tracks.size() < 1) {
                std::cout<<"Track with id "<<lc_track->id()<< " doesn't have a truth matched track "<<std::endl;
            }
            else {
                EVENT::Track* lc_truth_track = static_cast<EVENT::Track*> (lc_truth_tracks.at(0));
                Track* truth_track = utils::buildTrack(lc_truth_track,nullptr,nullptr);
                track->setTruthLink(truth_track);
                if (bfield_>0)
                    truth_track->setMomentum(bfield_);
                //truth tracks phi needs to be corrected
                if (truth_track->getPhi() > TMath::Pi())
                    truth_track->setPhi(truth_track->getPhi() - (TMath::Pi()) * 2.);
                
                truthTracks_.push_back(truth_track);
            }
            
        }
        tracks_.push_back(track);
        
        
        
        //Do the residual plots -- should be in another function
        if (doResiduals_)  {
            EVENT::LCCollection* trackRes_data_rel{nullptr};
            try {
                if (!trackResDataLcio_.empty())
                    trackRes_data_rel = static_cast<EVENT::LCCollection*>(event->getLCCollection(trackResDataLcio_.c_str()));
            }
            catch (EVENT::DataNotAvailableException e)
            {
                std::cout<<e.what()<<std::endl;
            }
            
            if (trackRes_data_rel) {
                std::shared_ptr<UTIL::LCRelationNavigator> trackRes_data_nav = std::make_shared<UTIL::LCRelationNavigator>(trackRes_data_rel);
                EVENT::LCObjectVec trackRes_data_vec = trackRes_data_nav->getRelatedFromObjects(lc_track);
                IMPL::LCGenericObjectImpl* trackRes_data = static_cast<IMPL::LCGenericObjectImpl*>(trackRes_data_vec.at(0)); 

                /*
                  //Some of the residuals do not get saved because sigma is negative. Will be fixed.
                if (track->getTrackerHitCount() != trackRes_data->getNDouble()) {
                    std::cout<<"WARNING-Different number of hit on track residuals wrt measurements"<<std::endl;
                    std::cout<<"Hits::"<<track->getTrackerHitCount()<<" Residuals:"<<trackRes_data->getNDouble()<<std::endl;
                }
                */
                
                //Last int is the volume
                for (int i_res = 0; i_res < trackRes_data->getNInt()-1;i_res++) {
                    //std::cout<<"Residual ly " << trackRes_data->getIntVal(i_res)<<" res="<< trackRes_data->getDoubleVal(i_res)<<" sigma="<<trackRes_data->getFloatVal(i_res)<<std::endl;
                    int ly = trackRes_data->getIntVal(i_res);
                    double res = trackRes_data->getDoubleVal(i_res);
                    double sigma = trackRes_data->getFloatVal(i_res);
                    trkResHistos_->FillResidualHistograms(track,ly,res,sigma);
                }
            }//trackResData exists
        }//doResiduals

        
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

    if (doResiduals_) {
        TFile* outfile = new TFile(resoutname_.c_str(),"RECREATE");
        trkResHistos_->saveHistos(outfile,trkCollLcio_);
        if (trkResHistos_) delete trkResHistos_;
        trkResHistos_=nullptr;
    }
}

DECLARE_PROCESSOR(TrackingProcessor); 
