#include "TrackingAnaProcessor.h"
#include <iomanip>
#include "utilities.h"
#include "AnaHelpers.h"

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
        isData_               = parameters.getInteger("isData",isData_);
        ecalCollName_         = parameters.getString("ecalCollName",ecalCollName_);
        regionSelections_     = parameters.getVString("regionDefinitions",regionSelections_);
        
        //Momentum smearing closure test
        pSmearingFile_            = parameters.getString("pSmearingFile",pSmearingFile_);
        
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

    if (!isData_)
      time_offset_ = 5.;
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

    // Setup track selections plots
    for (unsigned int i_reg = 0;
         i_reg < regionSelections_.size(); 
         i_reg++) {
      std::string regname = AnaHelpers::getFileName(regionSelections_[i_reg],false);
      std::cout<< "Setting up region "<< regname<<std::endl;
      
      reg_selectors_[regname] = std::make_shared<BaseSelector>(regname, regionSelections_[i_reg]);
      reg_selectors_[regname]->setDebug(false);
      reg_selectors_[regname]->LoadSelection();
      
      reg_histos_[regname] = std::make_shared<TrackHistos>(regname);
      reg_histos_[regname]->loadHistoConfig(histCfgFilename_);
      reg_histos_[regname]->doTrackComparisonPlots(false);
      reg_histos_[regname]->DefineTrkHitHistos();
      
      regions_.push_back(regname);
      
    }
      

    
    
    //Get event header information for trigger
    tree->SetBranchAddress("EventHeader", &evth_ , &bevth_);
    
    //Get cluster information for FEEs
    if (!ecalCollName_.empty()) 
      tree->SetBranchAddress(ecalCollName_.c_str(),&ecal_, &becal_);
    

    //Momentum smearing closure test
    if (!pSmearingFile_.empty()) {
      smearingTool_ =   std::make_shared<TrackSmearingTool>(pSmearingFile_);
      
      psmear_h_     =   new TH1D("psmear_h",
                                 "psmear_h",200,0,4);
      
      psmear_vs_nHits_hh_ =  new TH2D("psmear_vs_nHits_hh",
                                      "psmear_vs_nHits_hh",
                                      5,8,13,
                                      200,0,4);
      psmear_vs_nHits_top_hh_ =  new TH2D("psmear_vs_nHits_top_hh",
                                          "psmear_vs_nHits_top_hh",
                                          5,8,13,
                                          200,0,4);
      psmear_vs_nHits_bot_hh_ =  new TH2D("psmear_vs_nHits_bot_hh",
                                          "psmear_vs_nHits_bot_hh",
                                          5,8,13,
                                          200,0,4);
      
    }
      
    
}

bool TrackingAnaProcessor::process(IEvent* ievent) {
  
    double weight = 1.;
    // Loop over all the LCIO Tracks and add them to the HPS event.
    int n_sel_tracks = 0;
    
    
    //Trigger requirements - Singles 0 and 1. 
    //TODO use cutFlow 
    if (isData_ && (!evth_->isSingle0Trigger() && !evth_->isSingle1Trigger()))
      return true; //true is correct?

    //Ask for 1 cluster p > 1.2 GeV with time [40,70]
    //TODO Use Cutflow
    
    double minTime = 40;
    double maxTime = 70;
    
    if (!isData_) {
      minTime = 30;
      maxTime = 50;
    }
        
    if (ecal_->size() <= 2)
      return true;
    
    bool foundFeeCluster = false;
    
    for (unsigned int iclu = 0; iclu < ecal_->size(); iclu++) {
      if (ecal_->at(iclu)->getEnergy() > 1.5)
        foundFeeCluster = true;
      break;
    }
    
    if (!foundFeeCluster)
      return true;
    
    bool clusterInTime = true;
    
    for (unsigned int iclu = 0; iclu < ecal_->size(); iclu++) { 
      if (ecal_->at(iclu)->getTime() < 40 || ecal_->at(iclu)->getTime() > 70)
        clusterInTime = false;
    }

    if (!clusterInTime)
      return true;
        
    for (int itrack = 0; itrack < tracks_->size(); ++itrack) {
        
        if (trkSelector_) trkSelector_->getCutFlowHisto()->Fill(0.,weight);
        
        // Get a track
        Track* track = tracks_->at(itrack);
        int n2dhits_onTrack = !track->isKalmanTrack() ? track->getTrackerHitCount() * 2 : track->getTrackerHitCount();

        TVector3 trk_mom;
        trk_mom.SetX(track->getMomentum()[0]);
        trk_mom.SetY(track->getMomentum()[1]);
        trk_mom.SetZ(track->getMomentum()[2]);

        
        //Track Selection
        if (trkSelector_ && !trkSelector_->passCutGt("n_hits_gt",n2dhits_onTrack,weight))
            continue;

        if (trkSelector_ && !trkSelector_->passCutLt("chi2ndf_lt",track->getChi2Ndf(),weight))
          continue;
        
        if (trkSelector_ && !trkSelector_->passCutGt("p_gt",trk_mom.Mag(),weight))
          continue;

        if (trkSelector_ && !trkSelector_->passCutLt("p_lt",trk_mom.Mag(),weight))
          continue;
        
        if (trkSelector_ && !trkSelector_->passCutLt("trk_ecal_lt",track->getPositionAtEcal()[0],weight))
          continue;
        
        
        if (trkSelector_ && !trkSelector_->passCutGt("trk_ecal_gt",track->getPositionAtEcal()[0],weight))
          continue;
        
        if (trkSelector_ && !trkSelector_->passCutGt("trk_time_gt",track->getTrackTime()-time_offset_,weight))
          continue;
        
        if (trkSelector_ && !trkSelector_->passCutLt("trk_time_lt",track->getTrackTime()-time_offset_,weight))
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

        

        //Fill histograms for FEE smearing analysis
        trkHistos_->Fill2DHisto("xypos_at_ecal_hh",
                                track->getPositionAtEcal()[0],
                                track->getPositionAtEcal()[1]);
        
        trkHistos_->Fill3DHisto("p_vs_TanLambda_Phi_hhh",
                                track->getPhi(),
                                track->getTanLambda(),
                                track->getP());
        
        trkHistos_->Fill2DHisto("p_vs_nHits_hh",
                                track->getTrackerHitCount(),
                                track->getP());
        
        if (track->getTanLambda() > 0 )
          trkHistos_->Fill2DHisto("p_vs_nHits_top_hh",
                                  track->getTrackerHitCount(),
                                  track->getP());
        else
          trkHistos_->Fill2DHisto("p_vs_nHits_bot_hh",
                                  track->getTrackerHitCount(),
                                  track->getP());
        
        trkHistos_->Fill3DHisto("p_vs_TanLambda_nHits_hhh",
                                track->getTanLambda(),
                                track->getTrackerHitCount(),
                                track->getP());
        
        
        //pSmearing closure Test
        if (!isData_ && !pSmearingFile_.empty()) {
          double psmear = smearingTool_->smearTrackP(*track);
          double nhits  = track->getTrackerHitCount();
          double isTop  = track->getTanLambda() > 0 ? true : false;
          
          psmear_h_->Fill(psmear);
          psmear_vs_nHits_hh_->Fill(nhits,psmear);
          
          if (isTop) {
            psmear_vs_nHits_top_hh_->Fill(nhits,psmear);
          }
          else {
            psmear_vs_nHits_bot_hh_->Fill(nhits,psmear);
          }
        } // closer test
        
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

    for (reg_it it = reg_histos_.begin(); it!=reg_histos_.end(); ++it) {
      std::string dirName = it->first;
      (it->second)->saveHistos(outF_,dirName);
      outF_->cd(dirName.c_str());
      reg_selectors_[it->first]->getCutFlowHisto()->Write();
    }
    
    if (!pSmearingFile_.empty()) {
      outF_->cd(trkCollName_.c_str());
      psmear_h_->Write();
      psmear_vs_nHits_hh_->Write();
      psmear_vs_nHits_top_hh_->Write();
      psmear_vs_nHits_bot_hh_->Write();
      delete psmear_h_;
      delete psmear_vs_nHits_hh_;
      delete psmear_vs_nHits_top_hh_;
      delete psmear_vs_nHits_bot_hh_;
    }
    
    
    
    
    //trkHistos_->Clear();
}

DECLARE_PROCESSOR(TrackingAnaProcessor); 
