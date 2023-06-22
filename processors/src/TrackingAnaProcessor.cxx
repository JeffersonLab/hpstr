#include "TrackingAnaProcessor.h"
#include <iomanip>
#include "utilities.h"

TrackingAnaProcessor::TrackingAnaProcessor(const std::string& name, Process& process)
    : Processor(name, process) { 
    }

TrackingAnaProcessor::~TrackingAnaProcessor() { 
}

void TrackingAnaProcessor::configure(const ParameterSet& parameters) {

    std::cout << "Configuring TrackingAnaProcessor" << std::endl;
    try
    {
        debug_                = parameters.getInteger("debug",debug_);
        trkCollName_          = parameters.getString("trkCollName",trkCollName_);
        histCfgFilename_      = parameters.getString("histCfg",histCfgFilename_);
        doTruth_              = (bool) parameters.getInteger("doTruth",doTruth_);
        truthHistCfgFilename_ = parameters.getString("truthHistCfg",truthHistCfgFilename_);
        selectionCfg_         = parameters.getString("selectionjson",selectionCfg_); 
        hitColl_ = parameters.getString("hitColl",hitColl_);
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

}

void TrackingAnaProcessor::initialize(TTree* tree) {

    //Init histos
    trkHistos_ = new TrackHistos(trkCollName_);
    trkHistos_->loadHistoConfig(histCfgFilename_);
    trkHistos_->doTrackComparisonPlots(false);
    trkHistos_->DefineHistos();
    // Init tree
    tree->SetBranchAddress(trkCollName_.c_str(), &tracks_, &btracks_);
    if (!hitColl_.empty()){
        std::cout << "READ BRANCH" << std::endl;
        tree->SetBranchAddress(hitColl_.c_str(), &hits_   , &bhits_);
    }
    
    if (!selectionCfg_.empty()) {
        trkSelector_ = std::make_shared<BaseSelector>(name_+"_trkSelector",selectionCfg_);
        trkSelector_->setDebug(debug_);
        trkSelector_->LoadSelection();
    }
    
    if (doTruth_) {
        truthHistos_ = new TrackHistos(trkCollName_+"_truthComparison");
        truthHistos_->loadHistoConfig(histCfgFilename_);
        truthHistos_->DefineHistos();
        truthHistos_->loadHistoConfig(truthHistCfgFilename_);
        truthHistos_->DefineHistos();
        truthHistos_->doTrackComparisonPlots(false);
        
        //tree->SetBranchAddress(truthCollName_.c_str(),&truth_tracks_,&btruth_tracks_);
    }

}

bool TrackingAnaProcessor::process(IEvent* ievent) {

    double weight = 1.;
    // Loop over all the LCIO Tracks and add them to the HPS event.
    int n_sel_tracks = 0;
    for (int itrack = 0; itrack < tracks_->size(); ++itrack) {
        
        if (trkSelector_) trkSelector_->getCutFlowHisto()->Fill(0.,weight);
        
        // Get a track
        Track* track = tracks_->at(itrack);
        int n2dhits_onTrack = !track->isKalmanTrack() ? track->getTrackerHitCount() * 2 : track->getTrackerHitCount();
        
        //Track Selection
        if (trkSelector_ && !trkSelector_->passCutGt("n_hits_gt",n2dhits_onTrack,weight))
            continue;
        
        if (trkSelector_ && !trkSelector_->passCutGt("pt_gt",fabs(track->getPt()),weight))
            continue;

        Track* truth_track = nullptr;

        //Get the truth track
        if (doTruth_) { 
            truth_track = (Track*) track->getTruthLink().GetObject();
            if (!truth_track)
                std::cout<<"Warnings::TrackingAnaProcessor::Requested Truth track but couldn't find it in the ntuple"<<std::endl;
        }
        
        if(debug_ > 0)
        {
            std::cout<<"========================================="<<std::endl;
            std::cout<<"========================================="<<std::endl;
            std::cout<<"Track params:           "<<std::endl;
            track->Print();
        }
        
        trkHistos_->Fill1DHistograms(track);
        trkHistos_->Fill2DTrack(track);
        
        if (truthHistos_) {
            truthHistos_->Fill1DHistograms(truth_track);
            truthHistos_->Fill2DTrack(track);
            truthHistos_->Fill1DTrackTruth(track, truth_track);
        }
        
        n_sel_tracks++;

        double track_time = track->getTrackTime();
        if(track->getTrackerHitCount() < 10)
            continue;
        if(track->getTrackTime() > 10.0 || track->getTrackTime() < -10.0)
            continue;
        if (!hitColl_.empty()){
            //Investigating Track Isolation
            //std::cout << "Track type: " << track->getType() << std::endl;
            for (int i = 0; i < track->getSvtHits().GetEntries(); i++){

                TrackerHit* track_hit = (TrackerHit*)track->getSvtHits().At(i);
                std::vector<int> trackhit_rawhits = track_hit->getRawHitStripNumbers();
                int trackhit_maxstrip = *max_element(trackhit_rawhits.begin(), trackhit_rawhits.end());
                int trackhit_minstrip = *min_element(trackhit_rawhits.begin(), trackhit_rawhits.end());

                int trackhit_id = track_hit->getID();
                int trackhit_layer = track_hit->getLayer();
                int trackhit_volume = track_hit->getVolume();

                std::string vol = "T";
                if(trackhit_volume == 1)
                    vol = "B";

                bool isAxial = false;
                //std::cout << "Hit Layer: " << trackhit_layer << std::endl;
                //std::cout << "Hit Z: " << track_hit->getGlobalZ() << std::endl;
                //std::cout << "Hit volume: " << trackhit_volume << std::endl;
                //std::cout << "Vol: " << vol << std::endl;
                if(trackhit_volume == 1){
                    if(trackhit_layer%2 == 1)
                        isAxial = true;
                }
                if(trackhit_volume == 0){
                    if(trackhit_layer%2 == 0)
                        isAxial = true;
                }
                //std::cout << "Is Axial: " << isAxial << std::endl;
                if(!isAxial)
                    continue;
                if(trackhit_layer > 1)
                    continue;

                double trackhit_y = track_hit->getGlobalY();
                double trackhit_charge = track_hit->getRawCharge();
                double trackhit_time = track_hit->getTime();
                trkHistos_->Fill1DHisto("track_hitsOnTrack_L"+std::to_string(trackhit_layer)+vol+"_dt_h", track_time - trackhit_time);
                trkHistos_->Fill1DHisto("hitsOnTrack_L"+std::to_string(trackhit_layer)+vol+"_t_h",trackhit_time);
                trkHistos_->Fill1DHisto("hitsOnTrack_L"+std::to_string(trackhit_layer)+vol+"_charge_h",trackhit_charge);

                //Loop over all SiClusters in event
                double closest_distance = 999999.9;
                double charge_diff = 99999.9;
                double time_diff = 999999.9;
                double isohit_y = 999999.9;
                TrackerHit* closestStrip = nullptr;
                for (int j = 0; j < hits_->size(); j++){
                    TrackerHit* altStripCluster = hits_->at(j);
                    int althit_id = altStripCluster->getID();
                    int althit_volume = altStripCluster->getVolume();
                    std::string althit_vol = "T";
                    if(althit_volume == 1)
                        althit_vol = "B";
                    int althit_layer = altStripCluster->getLayer();
                    double althit_y = altStripCluster->getGlobalY();
                    double althit_charge = altStripCluster->getRawCharge();
                    double althit_time = altStripCluster->getTime();

                    //plots for all SiClusters
                    trkHistos_->Fill1DHisto("SiClusters_L"+std::to_string(althit_layer)+althit_vol+"_t_h",althit_time);
                    trkHistos_->Fill1DHisto("SiClusters_L"+std::to_string(althit_layer)+althit_vol+"_charge_h",althit_charge);

                    //Skip if SiCluster not on same layer as track hit
                    if (althit_layer != trackhit_layer) 
                        continue;
                    
                    if(althit_volume != trackhit_volume)
                        continue;

                    //Skip same cluster
                    if (althit_id == trackhit_id)
                        continue;

                    //Only look at hits that are further from beam-axis in Global Y
                    if ( (trackhit_volume == 0 && althit_y < trackhit_y) || 
                            (trackhit_volume == 1 && althit_y > trackhit_y))
                        continue;

                    //Skip adjacent rawhits
                    std::vector<int> althit_rawhits = altStripCluster->getRawHitStripNumbers();
                    int althit_maxstrip = *max_element(althit_rawhits.begin(), althit_rawhits.end());
                    int althit_minstrip = *min_element(althit_rawhits.begin(), althit_rawhits.end());
                    if(trackhit_minstrip - althit_maxstrip <= 1 && althit_minstrip - trackhit_maxstrip <= 1){
                        trkHistos_->Fill1DHisto("track_hit_adjacent_cluster_L"+std::to_string(trackhit_layer)+vol+"_dt_h", trackhit_time - althit_time);
                        trkHistos_->Fill1DHisto("track_hit_adjacent_cluster_L"+std::to_string(trackhit_layer)+vol+"_charge_h", althit_charge);
                        trkHistos_->Fill1DHisto("track_hit_plus_adjacent_cluster_L"+std::to_string(trackhit_layer)+vol+"_charge_h", althit_charge + trackhit_charge);
                        continue;
                    }

                    //Alternative SiCluster candidates
                    trkHistos_->Fill1DHisto("track_altstrips_L"+std::to_string(trackhit_layer)+vol+"_dt_h", trackhit_time - althit_time);
                    trkHistos_->Fill1DHisto("track_altstrips_L"+std::to_string(trackhit_layer)+vol+"_dy_h", trackhit_y - althit_y);
                    trkHistos_->Fill2DHisto("track_altstrips_L"+std::to_string(trackhit_layer)+vol+"_dt_v_dY_hh", trackhit_y-althit_y, trackhit_time - althit_time);
                    if(std::abs(trackhit_time-althit_time) <= 10.0)
                        trkHistos_->Fill2DHisto("track_stripy_v_intime_altstripy_L"+std::to_string(trackhit_layer)+vol+"_hh", trackhit_y, althit_y);

                    if (std::abs(trackhit_y - althit_y) < closest_distance){
                        closest_distance = std::abs(trackhit_y-althit_y);
                        closestStrip = altStripCluster;
                        charge_diff = trackhit_charge - althit_charge;
                        time_diff = trackhit_time - althit_time;
                        isohit_y = althit_y;
                    }
                }
                trkHistos_->Fill1DHisto("track_isostrip_L"+std::to_string(trackhit_layer)+vol+"_dt_h", time_diff);
                trkHistos_->Fill1DHisto("track_isostrip_L"+std::to_string(trackhit_layer)+vol+"_dy_h", closest_distance);
                trkHistos_->Fill2DHisto("track_isostrip_L"+std::to_string(trackhit_layer)+vol+"_dt_v_dY_hh", closest_distance, time_diff);
                if(std::abs(time_diff) <= 10.0)
                    trkHistos_->Fill2DHisto("track_stripy_v_intime_isostripy_L"+std::to_string(trackhit_layer)+vol+"_hh", trackhit_y, isohit_y);
            }
        }
    }//Loop on tracks

    trkHistos_->Fill1DHisto("n_tracks_h",n_sel_tracks);


    return true;
}

void TrackingAnaProcessor::finalize() { 

    trkHistos_->saveHistos(outF_,trkCollName_);
    delete trkHistos_;
    trkHistos_ = nullptr;
    if (trkSelector_)
        trkSelector_->getCutFlowHisto()->Write();

    if (truthHistos_) {
        truthHistos_->saveHistos(outF_,trkCollName_+"_truth");
        delete truthHistos_;
        truthHistos_ = nullptr;
    }
    //trkHistos_->Clear();
}

DECLARE_PROCESSOR(TrackingAnaProcessor); 
