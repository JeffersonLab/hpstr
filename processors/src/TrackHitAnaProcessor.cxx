#include "TrackHitAnaProcessor.h"
#include <iomanip>
#include "utilities.h"

TrackHitAnaProcessor::TrackHitAnaProcessor(const std::string& name, Process& process)
    : Processor(name, process) { 
    }

TrackHitAnaProcessor::~TrackHitAnaProcessor() { 
}

void TrackHitAnaProcessor::configure(const ParameterSet& parameters) {

    std::cout << "Configuring TrackHitAnaProcessor" << std::endl;
    try
    {
        debug_                = parameters.getInteger("debug",debug_);
        trkCollName_          = parameters.getString("trkCollName",trkCollName_);
        histCfgFilename_      = parameters.getString("histCfg",histCfgFilename_);
        doTruth_              = (bool) parameters.getInteger("doTruth",doTruth_);
        truthHistCfgFilename_ = parameters.getString("truthHistCfg",truthHistCfgFilename_);
        selectionCfg_         = parameters.getString("selectionjson",selectionCfg_); 
        regionSelections_     = parameters.getVString("regionDefinitions",regionSelections_);
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

}

void TrackHitAnaProcessor::initialize(TTree* tree) {

    //Init histos
    trkHistos_ = new TrackHistos("preselTrks");
    trkHistos_->loadHistoConfig(histCfgFilename_);
    trkHistos_->doTrackComparisonPlots(false);
    trkHistos_->DefineHistos();
    // Init tree
    tree->SetBranchAddress(trkCollName_.c_str(), &tracks_, &btracks_);
    
    if (!selectionCfg_.empty()) {
        trkSelector_ = std::make_shared<BaseSelector>(name_+"_trkSelector",selectionCfg_);
        trkSelector_->setDebug(debug_);
        trkSelector_->LoadSelection();
        std::cout << "Track Selection Loaded" << std::endl;
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

    //Setup regions
    for (unsigned int i_reg = 0; i_reg < regionSelections_.size(); i_reg++) 
    {
        std::string regname = AnaHelpers::getFileName(regionSelections_[i_reg],false);
        std::cout << "Setting up region:: " << regname << std::endl;
        reg_selectors_[regname] = std::make_shared<BaseSelector>(regname, regionSelections_[i_reg]);
        reg_selectors_[regname]->setDebug(debug_);
        reg_selectors_[regname]->LoadSelection();

        reg_histos_[regname] = std::make_shared<TrackHistos>(regname);
        reg_histos_[regname]->loadHistoConfig(histCfgFilename_);
        reg_histos_[regname]->doTrackComparisonPlots(false);
        reg_histos_[regname]->DefineTrkHitHistos();

        regions_.push_back(regname);

    }

}

bool TrackHitAnaProcessor::process(IEvent* ievent) {

    double weight = 1.;
    // Loop over all the LCIO Tracks and add them to the HPS event.
    int n_sel_tracks = 0;
    for (int itrack = 0; itrack < tracks_->size(); ++itrack) {
        
        if (trkSelector_) trkSelector_->getCutFlowHisto()->Fill(0.,weight);
        
        // Get a track
        Track* track = tracks_->at(itrack);
        bool isTop = (track->getTanLambda() > 0.0);
        bool isPos = (track->getOmega() < 0.0);
        bool isKF = track->isKalmanTrack();
        int trkType = (int)isTop*2 + (int)isPos;
        int n2dhits_onTrack = !track->isKalmanTrack() ? track->getTrackerHitCount() * 2 : track->getTrackerHitCount();
        
        //Track Selection
        if (trkSelector_ && !trkSelector_->passCutGt("n_hits_gt",n2dhits_onTrack,weight))
            continue;

        if (trkSelector_ && !trkSelector_->passCutLt("chi2ndf_lt",track->getChi2Ndf(),weight))
            continue;
        
        if (trkSelector_ && !trkSelector_->passCutGt("p_gt",fabs(track->getP()),weight))
            continue;

        if (trkSelector_ && !trkSelector_->passCutLt("p_lt",fabs(track->getP()),weight))
            continue;

        std::vector<int> hit_layers;
        int hitCode = 0;
        for (int ihit = 0; ihit<track->getSvtHits()->GetEntries(); ++ihit) {
            TrackerHit* hit = (TrackerHit*) track->getSvtHits()->At(ihit);
            int layer = hit->getLayer();
            if (isKF)
            {
                if (layer < 4) hitCode = hitCode | (0x1 << layer);
            }
            else
            {
                if (layer < 2) 
                {
                    hitCode = hitCode | (0x1 << (2*layer));
                    hitCode = hitCode | (0x1 << (2*layer+1));
                }
            }
        }
        trkHistos_->Fill1DHisto("hitCode_h", hitCode);
        trkHistos_->Fill2DHisto("hitCode_trkType_hh", hitCode, trkType);
        trkHistos_->Fill1DTrack(track, weight, "");

        n_sel_tracks++;

        for (auto region : regions_ ) 
        {

            reg_selectors_[region]->getCutFlowHisto()->Fill(0.,weight);
            if(debug_) std::cout<<"Check for region "<<region
                <<" hc "<<hitCode
                <<" lt:"<< !reg_selectors_[region]->passCutLt("hitCode_lt", ((double)hitCode)-0.5, weight)
                <<" gt:"<< !reg_selectors_[region]->passCutGt("hitCode_gt", ((double)hitCode)+0.5, weight)
                << std::endl;
            //Hit code req
            if ( !reg_selectors_[region]->passCutLt("hitCode_lt", ((double)hitCode)-0.5, weight) ) continue;

            if(debug_) std::cout<<"Pass Lt cut"<<std::endl;
            if ( !reg_selectors_[region]->passCutGt("hitCode_gt", ((double)hitCode)+0.5, weight) ) continue;

            if(debug_) std::cout<<"Pass Gt cut"<<std::endl;

            if(debug_) std::cout<<"Pass region "<<region<<std::endl;
            reg_histos_[region]->Fill1DHisto("hitCode_h", hitCode,weight);
            if(isTop&&isPos) reg_histos_[region]->Fill1DTrack(track, weight, "topPos_");
            if(isTop&&!isPos) reg_histos_[region]->Fill1DTrack(track, weight, "topEle_");
            if(!isTop&&isPos) reg_histos_[region]->Fill1DTrack(track, weight, "botPos_");
            if(!isTop&&!isPos) reg_histos_[region]->Fill1DTrack(track, weight, "botEle_");
        }
    }//Loop on tracks

    trkHistos_->Fill1DHisto("n_tracks_h",n_sel_tracks);



    return true;
}

void TrackHitAnaProcessor::finalize() { 

    outF_->cd();
    trkHistos_->saveHistos(outF_,"preselTrks");
    delete trkHistos_;
    trkHistos_ = nullptr;
    if (trkSelector_)
        trkSelector_->getCutFlowHisto()->Write();

    for (reg_it it = reg_histos_.begin(); it!=reg_histos_.end(); ++it) {
        std::string dirName = it->first;
        (it->second)->saveHistos(outF_,dirName);
        outF_->cd(dirName.c_str());
        reg_selectors_[it->first]->getCutFlowHisto()->Scale(0.5);
        reg_selectors_[it->first]->getCutFlowHisto()->Write();
    }


    if (truthHistos_) {
        truthHistos_->saveHistos(outF_,trkCollName_+"_truth");
        delete truthHistos_;
        truthHistos_ = nullptr;
    }
    //trkHistos_->Clear();
}

DECLARE_PROCESSOR(TrackHitAnaProcessor); 
