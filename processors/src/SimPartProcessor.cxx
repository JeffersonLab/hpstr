/**
 * @file SimPartProcessor.cxx
 * @brief SimPartProcessor used fill histograms to check acceptance of simulated particle source
 * @author Abhisek Datta, University of California, Los Angeles
 */     
#include "SimPartProcessor.h"
#include <iostream>

SimPartProcessor::SimPartProcessor(const std::string& name, Process& process) : Processor(name,process){}
//TODO CHECK THIS DESTRUCTOR
SimPartProcessor::~SimPartProcessor(){}


void SimPartProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring SimPartProcessor" << std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        anaName_         = parameters.getString("anaName");
        MCParticleColl_        = parameters.getString("MCParticleColl");
        MCTrackerHitColl_     = parameters.getString("MCTrackerHitColl");
        MCEcalHitColl_     = parameters.getString("MCEcalHitColl");
        RecoTrackColl_     = parameters.getString("RecoTrackColl");
        RecoTrackerClusterColl_     = parameters.getString("RecoTrackerClusterColl");
        RecoEcalClusterColl_     = parameters.getString("RecoEcalClusterColl");
        histCfgFilename_ = parameters.getString("histCfg");
        analysis_        = parameters.getString("analysis");
        selectionCfg_         = parameters.getString("selectionjson",selectionCfg_); 
        regionSelections_     = parameters.getVString("regionDefinitions",regionSelections_);
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }
}

void SimPartProcessor::initialize(TTree* tree) {
    tree_= tree;
    // init histos
    histos = new SimPartHistos(anaName_);
    histos->loadHistoConfig(histCfgFilename_);
    histos->DefineHistos();
    tuples = new FlatTupleMaker(anaName_+"_tree");
    tuples->addVariable("numMCparts");
    tuples->addVariable("numRecoTracks");
    tuples->addVariable("numSimTrackerHits");
    tuples->addVariable("numRecoTrackerClusters");
    tuples->addVariable("numSimEcalHits");
    tuples->addVariable("numRecoEcalClusters");
    tuples->addVector("particle_pdgid");
    tuples->addVector("ele_px");
    tuples->addVector("ele_py");
    tuples->addVector("ele_pz");
    tuples->addVector("ele_p");
    tuples->addVector("ele_energy");
    tuples->addVector("ele_pxpy");
    tuples->addVector("ele_pypz");
    tuples->addVector("track_n_hits");
    tuples->addVector("track_px");
    tuples->addVector("track_py");
    tuples->addVector("track_pz");
    tuples->addVector("track_p");
    tuples->addVector("track_phi0");
    tuples->addVector("track_tanlambda");
    tuples->addVector("track_x");
    tuples->addVector("track_y");
    tuples->addVector("track_ecal_x");
    tuples->addVector("track_ecal_y");
    tuples->addVector("ecal_n_hits");
    tuples->addVector("ecal_energy");
    tuples->addVector("ecal_x");
    tuples->addVector("ecal_y");

    if (!selectionCfg_.empty()) {
        EventSelector_ = std::make_shared<BaseSelector>(name_+"_EventSelector",selectionCfg_);
        EventSelector_->setDebug(debug_);
        EventSelector_->LoadSelection();
        std::cout << "Event Selection Loaded" << std::endl;
    }
    for (unsigned int i_reg = 0; i_reg < regionSelections_.size(); i_reg++) {
        std::string regname = AnaHelpers::getFileName(regionSelections_[i_reg],false);
        std::cout << "Setting up region:: " << regname << std::endl;
        reg_selectors_[regname] = std::make_shared<BaseSelector>(regname, regionSelections_[i_reg]);
        reg_selectors_[regname]->setDebug(debug_);
        reg_selectors_[regname]->LoadSelection();

        reg_histos_[regname] = std::make_shared<SimPartHistos>(regname);
        reg_histos_[regname]->loadHistoConfig(histCfgFilename_);
        reg_histos_[regname]->DefineHistos();

        reg_tuples_[regname] = std::make_shared<FlatTupleMaker>(anaName_+"_"+regname+"_tree");
        reg_histos_[regname]->addVariable("numMCparts");
        reg_histos_[regname]->addVariable("numRecoTracks");
        reg_histos_[regname]->addVariable("numSimTrackerHits");
        reg_histos_[regname]->addVariable("numRecoTrackerClusters");
        reg_histos_[regname]->addVariable("numSimEcalHits");
        reg_histos_[regname]->addVariable("numRecoEcalClusters");
        reg_histos_[regname]->addVector("particle_pdgid");
        reg_histos_[regname]->addVector("ele_px");
        reg_histos_[regname]->addVector("ele_py");
        reg_histos_[regname]->addVector("ele_pz");
        reg_histos_[regname]->addVector("ele_p");
        reg_histos_[regname]->addVector("ele_energy");
        reg_histos_[regname]->addVector("ele_pxpy");
        reg_histos_[regname]->addVector("ele_pypz");
        reg_histos_[regname]->addVector("track_n_hits");
        reg_histos_[regname]->addVector("track_px");
        reg_histos_[regname]->addVector("track_py");
        reg_histos_[regname]->addVector("track_pz");
        reg_histos_[regname]->addVector("track_p");
        reg_histos_[regname]->addVector("track_phi0");
        reg_histos_[regname]->addVector("track_tanlambda");
        reg_histos_[regname]->addVector("track_x");
        reg_histos_[regname]->addVector("track_y");
        reg_histos_[regname]->addVector("track_ecal_x");
        reg_histos_[regname]->addVector("track_ecal_y");
        reg_histos_[regname]->addVector("ecal_n_hits");
        reg_histos_[regname]->addVector("ecal_energy");
        reg_histos_[regname]->addVector("ecal_x");
        reg_histos_[regname]->addVector("ecal_y");

        regions_.push_back(regname);
    }

    // init TTree
    tree_->SetBranchAddress(MCParticleColl_.c_str(), &MCParticles_, &bMCParticles_);

    if (tree_->FindBranch(MCTrackerHitColl_.c_str()))
        tree_->SetBranchAddress(MCTrackerHitColl_.c_str(), &MCTrackerHits_, &bMCTrackerHits_);
    else
        std::cout<<"WARNING: No MC tracker hit collection"<<std::endl;
    if ( tree_->FindBranch(MCEcalHitColl_.c_str()))
        tree_->SetBranchAddress(MCEcalHitColl_.c_str(), &MCECalHits_, &bMCECalHits_);
    else
        std::cout<<"WARNING: No MC Ecal hit collection"<<std::endl;

    if (tree_->FindBranch(RecoTrackColl_.c_str()))
        tree_->SetBranchAddress(RecoTrackColl_.c_str(), &RecoTracks_, &bRecoTracks_);
    else
        std::cout<<"WARNING: No Reco track collection"<<std::endl;

    if (tree_->FindBranch(RecoTrackerClusterColl_.c_str()))
        tree_->SetBranchAddress(RecoTrackerClusterColl_.c_str(), &RecoTrackerClusters_, &bRecoTrackerClusters_);
    else
        std::cout<<"WARNING: No Reco tracker hit collection"<<std::endl;

    if (tree_->FindBranch(RecoEcalClusterColl_.c_str()))
        tree_->SetBranchAddress(RecoEcalClusterColl_.c_str(), &RecoEcalClusters_, &bRecoEcalClusters_);
    else
        std::cout<<"WARNING: No Reco Ecal hit collection"<<std::endl;
}

bool SimPartProcessor::process(IEvent* ievent) {

    double weight = 1.0;
    if (EventSelector_) EventSelector_->getCutFlowHisto()->Fill(0.,weight);

    int nParts = MCParticles_->size();
    int nSim_Tracker_hits = MCTrackerHits_->size();
    int nSim_Ecal_hits = MCEcalHits_->size(); 
    int nReco_Tracks = RecoTracks_->size();
    int nReco_Tracker_clusters = RecoTrackerClusters_->size();
    int nReco_Ecal_clusters = RecoEcalClusters_->size(); 

    // Event Selection
    if (EventSelector_ && !EventSelector_->passCutGt("n_simpart_gt", nParts, weight))
        continue;

    histos->Fill1DHisto("numMCparts_h", (float)nParts, weight);
    histos->Fill1DHisto("numRecoTracks_h", (float)nReco_Tracks, weight);
    histos->Fill1DHisto("numSimTrackerHits_h", (float)nSim_Tracker_hits, weight);
    histos->Fill1DHisto("numSimEcalHits_h", (float)nSim_Ecal_hits, weight);
    histos->Fill1DHisto("numRecoTrackerClusters_h", (float)nReco_Tracker_clusters, weight);
    histos->Fill1DHisto("numRecoEcalClusters_h", (float)nReco_Ecal_clusters, weight);
    tuples->setVariableValue("numMCparts", (float)nParts);
    tuples->setVariableValue("numRecoTracks", (float)nReco_Tracks);
    tuples->setVariableValue("numSimTrackerHits", (float)nSim_Tracker_hits);
    tuples->setVariableValue("numSimEcalHits", (float)nSim_Ecal_hits);
    tuples->setVariableValue("numRecoTrackerClusters", (float)nReco_Tracker_clusters);
    tuples->setVariableValue("numRecoEcalClusters", (float)nReco_Ecal_clusters);

    for (int i=0; i<nParts; i++) {
        MCParticle *part = MCParticles_->at(i);
        int gen = part->getGenStatus();
        if (gen != 1)
            continue;
        histos->FillMCParticle(part, tuples);
    }

    int min_n_Track_hits = 99999;
    for (int i=0; i<nReco_Tracks; i++) {
        Track* track = RecoTracks_->at(i);
        int n_hits = track->getTrackerHitCount();
        if (n_hits < min_n_Track_hits)
            min_n_Track_hits = n_hits;
        histos->FillRecoTrack(track, tuples);
    }

    for (int i=0; i<nReco_Ecal_clusters; i++) {
        CalCluster *ecal_cluster = RecoEcalClusters_->at(i); 
        histos->FillRecoEcalCuster(ecal_cluster, tuples);
    }

    tuple->fill();

    // Regions
    for (auto region : regions_ ) {
        reg_selectors_[region]->getCutFlowHisto()->Fill(0.,weight);
        if(debug_) std::cout<<"Check for region "<<region<<" Nr. of tracks: "<<nReco_Tracks<<" Nr. of ECal Clusters: "<<nReco_Ecal_clusters<<" Min. Nr. of Track Hits: "<<min_n_Track_hits<<std::endl;

        // Cuts
        if ( !reg_selectors_[region]->passCutGt("n_track_gt", nReco_Tracks, weight) ) continue;
        if(debug_) std::cout<<"Pass Nr. of Tracks Gt cut"<<std::endl;

        if ( !reg_selectors_[region]->passCutLt("n_track_lt", nReco_Tracks, weight) ) continue;
        if(debug_) std::cout<<"Pass Nr. of Tracks Lt cut"<<std::endl;

        if ( !reg_selectors_[region]->passCutGt("n_ecal_cluster_gt", nReco_Ecal_clusters, weight) ) continue;
        if(debug_) std::cout<<"Pass Nr. of ECal Clusters Gt cut"<<std::endl;

        if ( !reg_selectors_[region]->passCutLt("n_ecal_cluster_lt", nReco_Ecal_clusters, weight) ) continue;
        if(debug_) std::cout<<"Pass Nr. of ECal Clusters Lt cut"<<std::endl;

        if ( !reg_selectors_[region]->passCutGt("n_track_hits_gt", min_n_Track_hits, weight) ) continue;
        if(debug_) std::cout<<"Pass Nr. of Track Hits Gt cut"<<std::endl;

        if(debug_) std::cout<<"Pass region "<<region<<std::endl;

        reg_histos_[region]->Fill1DHisto("numMCparts_h", (float)nParts, weight);
        reg_histos_[region]->Fill1DHisto("numRecoTracks_h", (float)nReco_Tracks, weight);
        reg_histos_[region]->Fill1DHisto("numSimTrackerHits_h", (float)nSim_Tracker_hits, weight);
        reg_histos_[region]->Fill1DHisto("numSimEcalHits_h", (float)nSim_Ecal_hits, weight);
        reg_histos_[region]->Fill1DHisto("numRecoTrackerClusters_h", (float)nReco_Tracker_clusters, weight);
        reg_histos_[region]->Fill1DHisto("numRecoEcalClusters_h", (float)nReco_Ecal_clusters, weight);
        reg_tuples_[region]->setVariableValue("numMCparts", (float)nParts);
        reg_tuples_[region]->setVariableValue("numRecoTracks", (float)nReco_Tracks);
        reg_tuples_[region]->setVariableValue("numSimTrackerHits", (float)nSim_Tracker_hits);
        reg_tuples_[region]->setVariableValue("numSimEcalHits", (float)nSim_Ecal_hits);
        reg_tuples_[region]->setVariableValue("numRecoTrackerClusters", (float)nReco_Tracker_clusters);
        reg_tuples_[region]->setVariableValue("numRecoEcalClusters", (float)nReco_Ecal_clusters);

        for (int i=0; i<nParts; i++) {
            MCParticle *part = MCParticles_->at(i);
            int gen = part->getGenStatus();
            if (gen != 1)
                continue;
            reg_histos_[region]->FillMCParticle(part, reg_tuples_[region]);
        }

        for (int i=0; i<nReco_Tracks; i++) {
            Track* track = RecoTracks_->at(i);
            int n_hits = track->getTrackerHitCount();
            reg_histos_[region]->FillRecoTrack(track, reg_tuples_[region]);
        }

        for (int i=0; i<nReco_Ecal_clusters; i++) {
            CalCluster *ecal_cluster = RecoEcalClusters_->at(i); 
            reg_histos_[region]->FillRecoEcalCuster(ecal_cluster, reg_tuples_[region]);
        }
        reg_tuples_[region]->fill();
    }
    
    return true;
}

void SimPartProcessor::finalize() {

    outF_->cd();
    histos->saveHistos(outF_, "presel");
    outF_->cd("presel");
    if (EventSelector_)
        EventSelector_->getCutFlowHisto()->Write();
    tuples->writeTree();
    outF_->cd();

    for (reg_it it = reg_histos_.begin(); it!=reg_histos_.end(); ++it) {
        std::string dirName = it->first;
        (it->second)->saveHistos(outF_,dirName);
        outF_->cd(dirName.c_str());
        reg_selectors_[it->first]->getCutFlowHisto()->Scale(0.5);
        reg_selectors_[it->first]->getCutFlowHisto()->Write();
        reg_tuples_[region]->writeTree();
        outF_->cd();
    }
    outF_->Close();
}

DECLARE_PROCESSOR(SimPartProcessor);
