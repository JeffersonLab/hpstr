/**
 * @file SimPartProcessor.cxx
 * @brief SimPartProcessor used fill histograms to check acceptance of simulated particle source
 * @author Abhisek Datta, University of California, Los Angeles
 */     
#include "SimPartProcessor.h"
#include <iostream>
#include <algorithm>

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
        MCTrackerHitECalColl_     = parameters.getString("MCTrackerHitECalColl");
        MCEcalHitColl_     = parameters.getString("MCEcalHitColl");
        RecoTrackColl_     = parameters.getString("RecoTrackColl");
        RecoTrackColl_atlasthit_     = parameters.getString("RecoTrackColl_AtLastHit");
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
    tuples->addVector("sim_pdgid");
    tuples->addVector("sim_px");
    tuples->addVector("sim_py");
    tuples->addVector("sim_pz");
    tuples->addVector("sim_p");
    tuples->addVector("sim_energy");
    tuples->addVector("sim_pxpz");
    tuples->addVector("sim_pypz");
    tuples->addVector("track_n_hits");
    tuples->addVector("track_px");
    tuples->addVector("track_py");
    tuples->addVector("track_pz");
    tuples->addVector("track_p");
    tuples->addVector("track_phi0");
    tuples->addVector("track_tanlambda");
    tuples->addVector("track_d0");
    tuples->addVector("track_z0");
    tuples->addVector("track_omega");
    //tuples->addVector("track_x");
    //tuples->addVector("track_y");
    tuples->addVector("track_ecal_x");
    tuples->addVector("track_ecal_y");
    tuples->addVector("track_hit_layer");
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

        //reg_tuples_[regname] = std::make_shared<FlatTupleMaker>(anaName_+"_"+regname+"_tree");
        /*
        reg_tuples_[regname] = new FlatTupleMaker(anaName_+"_"+regname+"_tree");
        reg_tuples_[regname]->addVariable("numMCparts");
        reg_tuples_[regname]->addVariable("numRecoTracks");
        reg_tuples_[regname]->addVariable("numSimTrackerHits");
        reg_tuples_[regname]->addVariable("numRecoTrackerClusters");
        reg_tuples_[regname]->addVariable("numSimEcalHits");
        reg_tuples_[regname]->addVariable("numRecoEcalClusters");
        reg_tuples_[regname]->addVector("sim_pdgid");
        reg_tuples_[regname]->addVector("sim_px");
        reg_tuples_[regname]->addVector("sim_py");
        reg_tuples_[regname]->addVector("sim_pz");
        reg_tuples_[regname]->addVector("sim_p");
        reg_tuples_[regname]->addVector("sim_energy");
        reg_tuples_[regname]->addVector("sim_pxpz");
        reg_tuples_[regname]->addVector("sim_pypz");
        reg_tuples_[regname]->addVector("track_n_hits");
        reg_tuples_[regname]->addVector("track_px");
        reg_tuples_[regname]->addVector("track_py");
        reg_tuples_[regname]->addVector("track_pz");
        reg_tuples_[regname]->addVector("track_p");
        reg_tuples_[regname]->addVector("track_phi0");
        reg_tuples_[regname]->addVector("track_tanlambda");
        reg_tuples_[regname]->addVector("track_d0");
        reg_tuples_[regname]->addVector("track_z0");
        reg_tuples_[regname]->addVector("track_omega");
        //reg_tuples_[regname]->addVector("track_x");
        //reg_tuples_[regname]->addVector("track_y");
        reg_tuples_[regname]->addVector("track_ecal_x");
        reg_tuples_[regname]->addVector("track_ecal_y");
        reg_tuples_[regname]->addVector("track_hit_layrer");
        reg_tuples_[regname]->addVector("ecal_n_hits");
        reg_tuples_[regname]->addVector("ecal_energy");
        reg_tuples_[regname]->addVector("ecal_x");
        reg_tuples_[regname]->addVector("ecal_y");
        */

        regions_.push_back(regname);
    }

    // init TTree
    tree_->SetBranchAddress(MCParticleColl_.c_str(), &MCParticles_, &bMCParticles_);

    if (tree_->FindBranch(MCTrackerHitColl_.c_str()))
        tree_->SetBranchAddress(MCTrackerHitColl_.c_str(), &MCTrackerHits_, &bMCTrackerHits_);
    else
        std::cout<<"WARNING: No MC tracker hit collection"<<std::endl;

    if (tree_->FindBranch(MCTrackerHitECalColl_.c_str()))
        tree_->SetBranchAddress(MCTrackerHitECalColl_.c_str(), &MCTrackerHitsECal_, &bMCTrackerHitsECal_);
    else
        std::cout<<"WARNING: No MC tracker hit collection"<<std::endl;

    if ( tree_->FindBranch(MCEcalHitColl_.c_str()))
        tree_->SetBranchAddress(MCEcalHitColl_.c_str(), &MCEcalHits_, &bMCEcalHits_);
    else
        std::cout<<"WARNING: No MC Ecal hit collection"<<std::endl;

    if (tree_->FindBranch(RecoTrackColl_.c_str()))
        tree_->SetBranchAddress(RecoTrackColl_.c_str(), &RecoTracks_, &bRecoTracks_);
    else
        std::cout<<"WARNING: No Reco track collection"<<std::endl;

    if (tree_->FindBranch(RecoTrackColl_atlasthit_.c_str()))
        tree_->SetBranchAddress(RecoTrackColl_atlasthit_.c_str(), &RecoTracks_atlasthit_, &bRecoTracks_atlasthit_);
    else
        std::cout<<"WARNING: No Reco track at last hit collection"<<std::endl;

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
    int nSim_Tracker_hits_Ecal = MCTrackerHitsECal_->size();
    int nSim_Ecal_hits = MCEcalHits_->size(); 
    int nReco_Tracks = RecoTracks_->size();
    int nReco_Tracks_atlasthit = RecoTracks_atlasthit_->size();
    int nReco_Tracker_clusters = RecoTrackerClusters_->size();
    int nReco_Ecal_clusters = RecoEcalClusters_->size(); 

    // Event Selection
    if (EventSelector_ && !EventSelector_->passCutGt("n_simpart_gt", nParts, weight))
        return true;
    //if (EventSelector_ && !EventSelector_->passCutLt("n_simpart_lt", nParts, weight))
    //    return true;

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

    double sim_max_p = -99999;
    double sim_pxpz = -99999;
    double sim_pypz = -99999;
    std::vector<double> sim_p_list;
    std::vector<double> sim_pypz_list;
    sim_p_list.clear();
    sim_pypz_list.clear();
    for (int i=0; i<nParts; i++) {
        MCParticle *part = MCParticles_->at(i);
        int gen = part->getGenStatus();
        if (gen != 1)
            continue;
        histos->FillMCParticle(part, tuples);
        std::vector<double> momentum_V = part->getMomentum();
        double px = momentum_V.at(0);
        double py = momentum_V.at(1);
        double pz = momentum_V.at(2);
        double p = sqrt(px*px + py*py + pz*pz);
        sim_p_list.push_back(p);
        sim_pypz_list.push_back(py/pz);
        if (p > sim_max_p){
            sim_max_p = p;
            sim_pxpz = px/pz;
            sim_pypz = py/pz;
        }
    }

    int min_n_Track_hits = 99999;
    double track_omega = -99999;
    double track_phi0 = -99999;
    double track_tanlamda = -99999;
    double track_max_p_ecal_x = -99999;
    double track_max_p = -99999;
    std::vector<double> track_p_list;
    track_p_list.clear();
    for (int i=0; i<nReco_Tracks; i++) {
        Track* track = RecoTracks_->at(i);
        int n_hits = track->getTrackerHitCount();
        if (n_hits < min_n_Track_hits)
            min_n_Track_hits = n_hits;
        histos->FillRecoTrack(track, tuples);
        double p = track->getP();
        track_p_list.push_back(p);
        if (p > track_max_p){
            track_max_p = p;
            track_max_p_ecal_x = track->getPositionAtEcal().at(0);
            track_omega = track->getOmega();
            track_phi0 = track->getPhi();
            track_tanlamda = track->getTanLambda();
        }
    }

    double ecal_max_energy = -99999;
    double ecal_max_p_x = -99999;
    for (int i=0; i<nReco_Ecal_clusters; i++) {
        CalCluster *ecal_cluster = RecoEcalClusters_->at(i); 
        histos->FillRecoEcalCuster(ecal_cluster, tuples);
        double energy = ecal_cluster->getEnergy();
        if (energy > ecal_max_energy){
            ecal_max_energy = energy;
            ecal_max_p_x = ecal_cluster->getPosition().at(0);
        }
    }

    double mc_tracker_hit_ecal_max_p = -99999;
    double mc_tracker_hit_ecal_max_p_x = -99999;
    for (int i=0; i<nSim_Tracker_hits_Ecal; i++){
        MCTrackerHit *mc_tracker_hit_ecal = MCTrackerHitsECal_->at(i);
        int track_id = mc_tracker_hit_ecal->getPartID();
        double sim_p = -99999;
        for (int j=0; j<nParts; j++) {
            MCParticle *part = MCParticles_->at(j);
            int gen = part->getGenStatus();
            if (gen != 1)
                continue;
            int sim_id = part->getID();
            std::vector<double> momentum_V = part->getMomentum();
            double px = momentum_V.at(0);
            double py = momentum_V.at(1);
            double pz = momentum_V.at(2);
            if (sim_id == track_id){
                sim_p = sqrt(px*px + py*py + pz*pz);
                break;
            }
        }
        if (sim_p > mc_tracker_hit_ecal_max_p){
            mc_tracker_hit_ecal_max_p = sim_p;
            mc_tracker_hit_ecal_max_p_x = mc_tracker_hit_ecal->getPosition().at(0);
        }
    }

    double track_atlasthit_max_p_x = -99999;
    double track_atlasthit_max_p_y = -99999;
    double track_atlasthit_max_p_z = -99999;
    double track_atlasthit_max_p = -99999;
    double track_atlasthit_max_p_px = -99999;
    double track_atlasthit_max_p_py = -99999;
    double track_atlasthit_max_p_pz = -99999;
    int track_atlasthit_last_layer = -99999;
    std::vector<double> track_atlasthit_pypz_list;
    track_atlasthit_pypz_list.clear();
    for (int i=0; i<nReco_Tracks_atlasthit; i++) {
        Track* track_atlasthit = RecoTracks_atlasthit_->at(i);
        double track_p = track_atlasthit->getP();
        track_atlasthit_pypz_list.push_back(track_atlasthit->getMomentum().at(1)/track_atlasthit->getMomentum().at(2));
        if (track_p > track_atlasthit_max_p){
            track_atlasthit_max_p = track_p;
            track_atlasthit_max_p_x = track_atlasthit->getPosition().at(0);
            track_atlasthit_max_p_y = track_atlasthit->getPosition().at(1);
            track_atlasthit_max_p_z = track_atlasthit->getPosition().at(2);
            track_atlasthit_max_p_px = track_atlasthit->getMomentum().at(2);
            track_atlasthit_max_p_py = track_atlasthit->getMomentum().at(0);
            track_atlasthit_max_p_pz = track_atlasthit->getMomentum().at(1);
            std::vector<int> layer_list = track_atlasthit->getHitLayers();
            track_atlasthit_last_layer = *(std::max_element(layer_list.begin(), layer_list.end()));
        }
    }

    double mc_tracker_hit_atlasthit_max_p_x = -99999;
    double mc_tracker_hit_atlasthit_max_p_y = -99999;
    double mc_tracker_hit_atlasthit_max_p_z = -99999;
    double mc_tracker_hit_atlasthit_max_p = -99999;
    double mc_tracker_hit_atlasthit_max_p_px = -99999;
    double mc_tracker_hit_atlasthit_max_p_py = -99999;
    double mc_tracker_hit_atlasthit_max_p_pz = -99999;
    for (int i=0; i<nSim_Tracker_hits; i++){
        MCTrackerHit *mc_tracker_hit_atlasthit = MCTrackerHits_->at(i);
        int track_layer = mc_tracker_hit_atlasthit->getLayer();
        int track_id = mc_tracker_hit_atlasthit->getPartID();
        double mc_track_p = mc_tracker_hit_atlasthit->getP();
        if (track_layer != (track_atlasthit_last_layer+1))
            continue;
        if (mc_track_p > mc_tracker_hit_atlasthit_max_p){
            mc_tracker_hit_atlasthit_max_p = mc_track_p;
            mc_tracker_hit_atlasthit_max_p_x = mc_tracker_hit_atlasthit->getPosition().at(0);
            mc_tracker_hit_atlasthit_max_p_y = mc_tracker_hit_atlasthit->getPosition().at(1);
            mc_tracker_hit_atlasthit_max_p_z = mc_tracker_hit_atlasthit->getPosition().at(2);
            mc_tracker_hit_atlasthit_max_p_px = mc_tracker_hit_atlasthit->getMomentum().at(0);
            mc_tracker_hit_atlasthit_max_p_py = mc_tracker_hit_atlasthit->getMomentum().at(1);
            mc_tracker_hit_atlasthit_max_p_pz = mc_tracker_hit_atlasthit->getMomentum().at(2);
        }
    }

    if (track_max_p != -99999 && sim_max_p != -99999)
        histos->Fill2DHisto("track_sim_p_sim_p_hh", track_max_p/sim_max_p, sim_max_p, weight);

    if (sim_pxpz != -99999 && track_phi0 != -99999){
        histos->Fill1DHisto("track_phi0_simpxpz_h", (track_phi0/sim_pxpz), weight);
        histos->Fill2DHisto("track_phi0_simpxpz_sim_p_hh", (track_phi0/sim_pxpz), sim_max_p, weight);
    }
    if (sim_pypz != -99999 && track_tanlamda != -99999){
        histos->Fill1DHisto("track_tanlambda_simpypz_h", (track_tanlamda/sim_pypz), weight);
        histos->Fill2DHisto("track_tanlambda_simpypz_sim_p_hh", (track_tanlamda/sim_pypz), sim_max_p, weight);
    }
    if (sim_pxpz != -99999 && track_phi0 != -99999 && sim_pypz != -99999 && track_tanlamda != -99999)
        histos->Fill2DHisto("track_phi0_simpxpz_tanlambda_simpypz_hh", (track_phi0/sim_pxpz), (track_tanlamda/sim_pypz), weight);
    if (track_max_p_ecal_x != -99999 && ecal_max_p_x != 99999){
        histos->Fill1DHisto("track_ecal_x_diff_h", (track_max_p_ecal_x-ecal_max_p_x), weight);
        histos->Fill2DHisto("track_ecal_x_track_p_hh", (track_max_p_ecal_x-ecal_max_p_x), track_max_p, weight);
        histos->Fill2DHisto("track_ecal_x_sim_p_hh", (track_max_p_ecal_x-ecal_max_p_x), sim_max_p, weight);
        histos->Fill2DHisto("track_ecal_x_ecal_energy_hh", (track_max_p_ecal_x-ecal_max_p_x), ecal_max_energy, weight);
        histos->Fill2DHisto("track_ecal_x_ecal_x_hh", ecal_max_p_x, (track_max_p_ecal_x-ecal_max_p_x), weight);
    }
    if (mc_tracker_hit_ecal_max_p_x != -99999 && track_max_p_ecal_x != 99999){
        histos->Fill1DHisto("sim_track_x_diff_h", (track_max_p_ecal_x-mc_tracker_hit_ecal_max_p_x), weight);
        histos->Fill2DHisto("sim_track_x_track_p_hh", (track_max_p_ecal_x-mc_tracker_hit_ecal_max_p_x), track_max_p, weight);
        histos->Fill2DHisto("sim_track_x_sim_p_hh", (track_max_p_ecal_x-mc_tracker_hit_ecal_max_p_x), sim_max_p, weight);
        histos->Fill2DHisto("sim_track_x_ecal_energy_hh", (track_max_p_ecal_x-mc_tracker_hit_ecal_max_p_x), ecal_max_energy, weight);
    }
    if (mc_tracker_hit_ecal_max_p_x != -99999 && ecal_max_p_x != 99999){
        histos->Fill1DHisto("sim_ecal_x_diff_h", (ecal_max_p_x-mc_tracker_hit_ecal_max_p_x), weight);
        histos->Fill2DHisto("sim_ecal_x_track_p_hh", (ecal_max_p_x-mc_tracker_hit_ecal_max_p_x), track_max_p, weight);
        histos->Fill2DHisto("sim_ecal_x_sim_p_hh", (ecal_max_p_x-mc_tracker_hit_ecal_max_p_x), sim_max_p, weight);
        histos->Fill2DHisto("sim_ecal_x_ecal_energy_hh", (-ecal_max_p_x-mc_tracker_hit_ecal_max_p_x), ecal_max_energy, weight);
    }
    if (mc_tracker_hit_atlasthit_max_p_x != -99999 && track_atlasthit_max_p_x != -99999){
        histos->Fill1DHisto("sim_track_atlasthit_x_diff_h", (track_atlasthit_max_p_x-mc_tracker_hit_atlasthit_max_p_x), weight);
        histos->Fill1DHisto("sim_track_atlasthit_y_diff_h", (track_atlasthit_max_p_y-mc_tracker_hit_atlasthit_max_p_y), weight);
        histos->Fill1DHisto("sim_track_atlasthit_z_diff_h", (track_atlasthit_max_p_z-mc_tracker_hit_atlasthit_max_p_z), weight);

        histos->Fill1DHisto("sim_track_atlasthit_p_diff_h", (track_atlasthit_max_p-mc_tracker_hit_atlasthit_max_p), weight);
        histos->Fill1DHisto("sim_track_atlasthit_px_diff_h", (track_atlasthit_max_p_px-mc_tracker_hit_atlasthit_max_p_px), weight);
        histos->Fill1DHisto("sim_track_atlasthit_py_diff_h", (track_atlasthit_max_p_py-mc_tracker_hit_atlasthit_max_p_py), weight);
        histos->Fill1DHisto("sim_track_atlasthit_pz_diff_h", (track_atlasthit_max_p_pz-mc_tracker_hit_atlasthit_max_p_pz), weight);

        histos->Fill2DHisto("sim_track_atlasthit_x_track_p_hh", (track_atlasthit_max_p_x-mc_tracker_hit_atlasthit_max_p_x), track_atlasthit_max_p, weight);
        histos->Fill2DHisto("sim_track_atlasthit_x_sim_p_hh", (track_atlasthit_max_p_x-mc_tracker_hit_atlasthit_max_p_x), mc_tracker_hit_atlasthit_max_p, weight);
        histos->Fill2DHisto("sim_track_atlasthit_x_ecal_energy_hh", (track_atlasthit_max_p_x-mc_tracker_hit_atlasthit_max_p_x), ecal_max_energy, weight);
    }

    tuples->fill();

    // Regions
    for (auto region : regions_ ) {
        reg_selectors_[region]->getCutFlowHisto()->Fill(0.,weight);
        if(debug_) std::cout<<"Check for region "<<region<<" Nr. of sim particles: "<<nParts<<" Nr. of tracks: "<<nReco_Tracks<<" Nr. of ECal Clusters: "<<nReco_Ecal_clusters<<" Min. Nr. of Track Hits: "<<min_n_Track_hits<<" Track Omega: "<<track_omega<<"Track Last Layer of Hit: "<<track_atlasthit_last_layer<<std::endl;

        // Cuts
        if ( !reg_selectors_[region]->passCutEq("n_sim_eq", nParts, weight) ) continue;
        if(debug_) std::cout<<"Pass Nr. of Sim Particles Eq cut"<<std::endl;

        if ( !reg_selectors_[region]->passCutEq("n_track_eq", nReco_Tracks, weight) ) continue;
        if(debug_) std::cout<<"Pass Nr. of Tracks Eq cut"<<std::endl;

        if ( !reg_selectors_[region]->passCutGt("n_track_gt", nReco_Tracks, weight) ) continue;
        if(debug_) std::cout<<"Pass Nr. of Tracks Gt cut"<<std::endl;

        if ( !reg_selectors_[region]->passCutLt("n_track_lt", nReco_Tracks, weight) ) continue;
        if(debug_) std::cout<<"Pass Nr. of Tracks Lt cut"<<std::endl;

        if ( !reg_selectors_[region]->passCutEq("n_ecal_cluster_eq", nReco_Ecal_clusters, weight) ) continue;
        if(debug_) std::cout<<"Pass Nr. of ECal Clusters Eq cut"<<std::endl;

        if ( !reg_selectors_[region]->passCutGt("n_ecal_cluster_gt", nReco_Ecal_clusters, weight) ) continue;
        if(debug_) std::cout<<"Pass Nr. of ECal Clusters Gt cut"<<std::endl;

        if ( !reg_selectors_[region]->passCutLt("n_ecal_cluster_lt", nReco_Ecal_clusters, weight) ) continue;
        if(debug_) std::cout<<"Pass Nr. of ECal Clusters Lt cut"<<std::endl;

        if ( !reg_selectors_[region]->passCutGt("track_omega_gt", track_omega, weight) ) continue;
        if(debug_) std::cout<<"Pass Track Omega Gt cut"<<std::endl;

        if ( !reg_selectors_[region]->passCutLt("track_omega_lt", track_omega, weight) ) continue;
        if(debug_) std::cout<<"Pass Track Omega Lt cut"<<std::endl;

        if ( !reg_selectors_[region]->passCutGt("n_track_hits_gt", min_n_Track_hits, weight) ) continue;
        if(debug_) std::cout<<"Pass Nr. of Track Hits Gt cut"<<std::endl;

        if ( !reg_selectors_[region]->passCutEq("track_last_layer_eq", track_atlasthit_last_layer, weight) ) continue;
        if(debug_) std::cout<<"Pass Track Last Layer of Hit Eq cut"<<std::endl;

        int track_p_fail = 0;
        for (int i=0; i<track_p_list.size(); i++){
            if ( !reg_selectors_[region]->passCutGt("track_p_gt", track_p_list[i], weight) ){
                track_p_fail = 1;
                break;
            }
        }
        if (track_p_fail) continue;
        if(debug_) std::cout<<"Pass Track pT Gt cut"<<std::endl;

        int sim_p_fail = 0;
        for (int i=0; i<sim_p_list.size(); i++){
            if ( !reg_selectors_[region]->passCutGt("sim_p_gt", sim_p_list[i], weight) ){
                sim_p_fail = 1;
                break;
            }
        }
        if (sim_p_fail) continue;
        if(debug_) std::cout<<"Pass Sim pT Gt cut"<<std::endl;

        sim_p_fail = 0;
        for (int i=0; i<sim_p_list.size(); i++){
            if ( !reg_selectors_[region]->passCutLt("sim_p_lt", sim_p_list[i], weight) ){
                sim_p_fail = 1;
                break;
            }
        }
        if (sim_p_fail) continue;
        if(debug_) std::cout<<"Pass Sim pT Lt cut"<<std::endl;

        int sim_pypz_fail = 0;
        for (int i=0; i<sim_pypz_list.size(); i++){
            if ( !reg_selectors_[region]->passCutGt("sim_pypz_gt", sim_pypz_list[i], weight) ){
                sim_pypz_fail = 1;
                break;
            }
        }
        if (sim_pypz_fail) continue;
        if(debug_) std::cout<<"Pass Sim py/pz Gt cut"<<std::endl;

        sim_pypz_fail = 0;
        for (int i=0; i<sim_pypz_list.size(); i++){
            if ( !reg_selectors_[region]->passCutLt("sim_pypz_lt", sim_pypz_list[i], weight) ){
                sim_pypz_fail = 1;
                break;
            }
        }
        if (sim_pypz_fail) continue;
        if(debug_) std::cout<<"Pass Sim py/pz Lt cut"<<std::endl;

        int track_atlasthit_pypz_fail = 0;
        for (int i=0; i<track_atlasthit_pypz_list.size(); i++){
            if ( !reg_selectors_[region]->passCutGt("track_pypz_gt", track_atlasthit_pypz_list[i], weight) ){
                track_atlasthit_pypz_fail = 1;
                break;
            }
        }
        if (track_atlasthit_pypz_fail) continue;
        if(debug_) std::cout<<"Pass Track at Last Hit py/pz Gt cut"<<std::endl;

        track_atlasthit_pypz_fail = 0;
        for (int i=0; i<track_atlasthit_pypz_list.size(); i++){
            if ( !reg_selectors_[region]->passCutLt("track_pypz_lt", track_atlasthit_pypz_list[i], weight) ){
                track_atlasthit_pypz_fail = 1;
                break;
            }
        }
        if (track_atlasthit_pypz_fail) continue;
        if(debug_) std::cout<<"Pass Track at Last Hit py/pz Lt cut"<<std::endl;


        if(debug_) std::cout<<"Pass region "<<region<<std::endl;

        reg_histos_[region]->Fill1DHisto("numMCparts_h", (float)nParts, weight);
        reg_histos_[region]->Fill1DHisto("numRecoTracks_h", (float)nReco_Tracks, weight);
        reg_histos_[region]->Fill1DHisto("numSimTrackerHits_h", (float)nSim_Tracker_hits, weight);
        reg_histos_[region]->Fill1DHisto("numSimEcalHits_h", (float)nSim_Ecal_hits, weight);
        reg_histos_[region]->Fill1DHisto("numRecoTrackerClusters_h", (float)nReco_Tracker_clusters, weight);
        reg_histos_[region]->Fill1DHisto("numRecoEcalClusters_h", (float)nReco_Ecal_clusters, weight);
        //reg_tuples_[region]->setVariableValue("numMCparts", (float)nParts);
        //reg_tuples_[region]->setVariableValue("numRecoTracks", (float)nReco_Tracks);
        //reg_tuples_[region]->setVariableValue("numSimTrackerHits", (float)nSim_Tracker_hits);
        //reg_tuples_[region]->setVariableValue("numSimEcalHits", (float)nSim_Ecal_hits);
        //reg_tuples_[region]->setVariableValue("numRecoTrackerClusters", (float)nReco_Tracker_clusters);
        //reg_tuples_[region]->setVariableValue("numRecoEcalClusters", (float)nReco_Ecal_clusters);

        for (int i=0; i<nParts; i++) {
            MCParticle *part = MCParticles_->at(i);
            int gen = part->getGenStatus();
            if (gen != 1)
                continue;
            //reg_histos_[region]->FillMCParticle(part, reg_tuples_[region]);
            reg_histos_[region]->FillMCParticle(part);
        }

        for (int i=0; i<nReco_Tracks; i++) {
            Track* track = RecoTracks_->at(i);
            reg_histos_[region]->FillRecoTrack(track);
        }

        for (int i=0; i<nReco_Ecal_clusters; i++) {
            CalCluster *ecal_cluster = RecoEcalClusters_->at(i); 
            //reg_histos_[region]->FillRecoEcalCuster(ecal_cluster, reg_tuples_[region]);
            reg_histos_[region]->FillRecoEcalCuster(ecal_cluster);
        }

        if (track_max_p != -99999 && sim_max_p != -99999)
            reg_histos_[region]->Fill2DHisto("track_sim_p_sim_p_hh", track_max_p/sim_max_p, sim_max_p, weight);

        if (sim_pxpz != -99999 && track_phi0 != -99999){
            reg_histos_[region]->Fill1DHisto("track_phi0_simpxpz_h", (track_phi0/sim_pxpz), weight);
            reg_histos_[region]->Fill2DHisto("track_phi0_simpxpz_sim_p_hh", (track_phi0/sim_pxpz), sim_max_p, weight);
        }
        if (sim_pypz != -99999 && track_tanlamda != -99999){
            reg_histos_[region]->Fill1DHisto("track_tanlambda_simpypz_h", (track_tanlamda/sim_pypz), weight);
            reg_histos_[region]->Fill2DHisto("track_tanlambda_simpypz_sim_p_hh", (track_tanlamda/sim_pypz), sim_max_p, weight);
        }
        if (sim_pxpz != -99999 && track_phi0 != -99999 && sim_pypz != -99999 && track_tanlamda != -99999)
            reg_histos_[region]->Fill2DHisto("track_phi0_simpxpz_tanlambda_simpypz_hh", (track_phi0/sim_pxpz), (track_tanlamda/sim_pypz), weight);
        if (track_max_p_ecal_x != -99999 && ecal_max_p_x != 99999){
            reg_histos_[region]->Fill1DHisto("track_ecal_x_diff_h", (track_max_p_ecal_x-ecal_max_p_x), weight);
            reg_histos_[region]->Fill2DHisto("track_ecal_x_track_p_hh", (track_max_p_ecal_x-ecal_max_p_x), track_max_p, weight);
            reg_histos_[region]->Fill2DHisto("track_ecal_x_sim_p_hh", (track_max_p_ecal_x-ecal_max_p_x), sim_max_p, weight);
            reg_histos_[region]->Fill2DHisto("track_ecal_x_ecal_energy_hh", (track_max_p_ecal_x-ecal_max_p_x), ecal_max_energy, weight);
            reg_histos_[region]->Fill2DHisto("track_ecal_x_ecal_x_hh", ecal_max_p_x, (track_max_p_ecal_x-ecal_max_p_x), weight);
        }
        if (mc_tracker_hit_ecal_max_p_x != -99999 && track_max_p_ecal_x != 99999){
            reg_histos_[region]->Fill1DHisto("sim_track_x_diff_h", (track_max_p_ecal_x-mc_tracker_hit_ecal_max_p_x), weight);
            reg_histos_[region]->Fill2DHisto("sim_track_x_track_p_hh", (track_max_p_ecal_x-mc_tracker_hit_ecal_max_p_x), track_max_p, weight);
            reg_histos_[region]->Fill2DHisto("sim_track_x_sim_p_hh", (track_max_p_ecal_x-mc_tracker_hit_ecal_max_p_x), sim_max_p, weight);
            reg_histos_[region]->Fill2DHisto("sim_track_x_ecal_energy_hh", (track_max_p_ecal_x-mc_tracker_hit_ecal_max_p_x), ecal_max_energy, weight);
        }
        if (mc_tracker_hit_ecal_max_p_x != -99999 && ecal_max_p_x != 99999){
            reg_histos_[region]->Fill1DHisto("sim_ecal_x_diff_h", (ecal_max_p_x-mc_tracker_hit_ecal_max_p_x), weight);
            reg_histos_[region]->Fill2DHisto("sim_ecal_x_track_p_hh", (ecal_max_p_x-mc_tracker_hit_ecal_max_p_x), track_max_p, weight);
            reg_histos_[region]->Fill2DHisto("sim_ecal_x_sim_p_hh", (ecal_max_p_x-mc_tracker_hit_ecal_max_p_x), sim_max_p, weight);
            reg_histos_[region]->Fill2DHisto("sim_ecal_x_ecal_energy_hh", (-ecal_max_p_x-mc_tracker_hit_ecal_max_p_x), ecal_max_energy, weight);
        }
        if (mc_tracker_hit_atlasthit_max_p_x != -99999 && track_atlasthit_max_p_x != -99999){
            reg_histos_[region]->Fill1DHisto("sim_track_atlasthit_x_diff_h", (track_atlasthit_max_p_x-mc_tracker_hit_atlasthit_max_p_x), weight);
            reg_histos_[region]->Fill1DHisto("sim_track_atlasthit_y_diff_h", (track_atlasthit_max_p_y-mc_tracker_hit_atlasthit_max_p_y), weight);
            reg_histos_[region]->Fill1DHisto("sim_track_atlasthit_z_diff_h", (track_atlasthit_max_p_z-mc_tracker_hit_atlasthit_max_p_z), weight);

            reg_histos_[region]->Fill1DHisto("sim_track_atlasthit_p_diff_h", (track_atlasthit_max_p-mc_tracker_hit_atlasthit_max_p), weight);
            reg_histos_[region]->Fill1DHisto("sim_track_atlasthit_px_diff_h", (track_atlasthit_max_p_px-mc_tracker_hit_atlasthit_max_p_px), weight);
            reg_histos_[region]->Fill1DHisto("sim_track_atlasthit_py_diff_h", (track_atlasthit_max_p_py-mc_tracker_hit_atlasthit_max_p_py), weight);
            reg_histos_[region]->Fill1DHisto("sim_track_atlasthit_pz_diff_h", (track_atlasthit_max_p_pz-mc_tracker_hit_atlasthit_max_p_pz), weight);

            reg_histos_[region]->Fill2DHisto("sim_track_atlasthit_x_track_p_hh", (track_atlasthit_max_p_x-mc_tracker_hit_atlasthit_max_p_x), track_atlasthit_max_p, weight);
            reg_histos_[region]->Fill2DHisto("sim_track_atlasthit_x_sim_p_hh", (track_atlasthit_max_p_x-mc_tracker_hit_atlasthit_max_p_x), mc_tracker_hit_atlasthit_max_p, weight);
            reg_histos_[region]->Fill2DHisto("sim_track_atlasthit_x_ecal_energy_hh", (track_atlasthit_max_p_x-mc_tracker_hit_atlasthit_max_p_x), ecal_max_energy, weight);
    }
        
        //reg_tuples_[region]->fill();
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
        //reg_tuples_[it->first]->writeTree();
        outF_->cd();
    }
    outF_->Close();
}

DECLARE_PROCESSOR(SimPartProcessor);
