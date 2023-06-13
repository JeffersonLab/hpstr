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
        //beamspot positions
        run_number_ = parameters.getInteger("run_number",run_number_);
        beamPosCfg_ = parameters.getString("beamPosCfg",beamPosCfg_);
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

    //Run Dependent Corrections
    //Beam Position 
    if(!beamPosCfg_.empty() && run_number_ != -999){
        std::cout << "Loading beam positions" << std::endl;
        std::cout << "Looking for run closest to " << run_number_ << std::endl;
        std::ifstream bpc_file(beamPosCfg_);
        bpc_file >> bpc_configs_;
        bpc_file.close();
        int closest_run;
        for(auto run : bpc_configs_.items()){
            int check_run = std::stoi(run.key());
            if(check_run > run_number_)
                break;
            else{
                closest_run = check_run;
            }
        }
        std::cout << "Found beam position corrections from run " << closest_run << std::endl;
        beamPosCorrections_ = {bpc_configs_[std::to_string(closest_run)]["beamspot_x"], 
            bpc_configs_[std::to_string(closest_run)]["beamspot_y"],
            bpc_configs_[std::to_string(closest_run)]["beamspot_z"]};
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
        //Plot beampos corrected values
        if(!bpc_configs_.empty()){
            trkHistos_->Fill1DHisto("Z0_beampos_corr_h",track->getZ0()-beamPosCorrections_.at(1));
            trkHistos_->Fill1DHisto("Z0_at_target_beampos_corr_h",track->getTargetZ0()-beamPosCorrections_.at(1));
            if (track->getTanLambda() > 0.0){
                if (track->getCharge() < 0){
                    trkHistos_->Fill1DHisto("top_ele_Z0_beampos_corr_h",track->getZ0()-beamPosCorrections_.at(1));
                    trkHistos_->Fill1DHisto("top_ele_Z0_at_target_beampos_corr_h",track->getTargetZ0()-beamPosCorrections_.at(1));
                }
                else{
                    trkHistos_->Fill1DHisto("top_pos_Z0_beampos_corr_h",track->getZ0()-beamPosCorrections_.at(1));
                    trkHistos_->Fill1DHisto("top_pos_Z0_at_target_beampos_corr_h",track->getTargetZ0()-beamPosCorrections_.at(1));
                }
            }
            else{
                if (track->getCharge() < 0){
                    trkHistos_->Fill1DHisto("bot_ele_Z0_beampos_corr_h",track->getZ0()-beamPosCorrections_.at(1));
                    trkHistos_->Fill1DHisto("bot_ele_Z0_at_target_beampos_corr_h",track->getTargetZ0()-beamPosCorrections_.at(1));
                }
                else{
                    trkHistos_->Fill1DHisto("bot_pos_Z0_beampos_corr_h",track->getZ0()-beamPosCorrections_.at(1));
                    trkHistos_->Fill1DHisto("bot_pos_Z0_at_target_beampos_corr_h",track->getTargetZ0()-beamPosCorrections_.at(1));
                }
            }   
        }
        
        if (truthHistos_) {
            truthHistos_->Fill1DHistograms(truth_track);
            truthHistos_->Fill2DTrack(track);
            truthHistos_->Fill1DTrackTruth(track, truth_track);
        }
        
        n_sel_tracks++;
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
