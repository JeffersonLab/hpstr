/**
 *@file TriggerParametersExtractionAnaProcessor.cxx
 *@author Tongtong, UNH
 */

#include "TriggerParametersExtractionAnaProcessor.h"

#include <iostream>

#include "TF1.h"
#include "math.h"

#define ELECTRONMASS 0.000510998950 // GeV
#define PI 3.14159265358979

TriggerParametersExtractionAnaProcessor::TriggerParametersExtractionAnaProcessor(const std::string& name, Process& process) : Processor(name,process) {

}

TriggerParametersExtractionAnaProcessor::~TriggerParametersExtractionAnaProcessor(){}

void TriggerParametersExtractionAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring TriggerParametersExtractionAnaProcessor" <<std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        anaName_         = parameters.getString("anaName");
        histCfgFilename_      = parameters.getString("histCfg",histCfgFilename_);
        trkColl_    = parameters.getString("trkColl");
        gtpClusColl_    = parameters.getString("gtpClusColl");
        mcColl_  = parameters.getString("mcColl",mcColl_);
        beamE_  = parameters.getDouble("beamE",beamE_);
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }
}

void TriggerParametersExtractionAnaProcessor::initialize(TTree* tree) {
    tree_= tree;
    // init histos
    histos = new TriggerParametersExtractionAnaHistos(anaName_.c_str());
    histos->loadHistoConfig(histCfgFilename_);
    histos->DefineHistos();

    // init TTree
    tree_->SetBranchAddress(trkColl_.c_str() , &trks_, &btrks_);
    tree_->SetBranchAddress(gtpClusColl_.c_str() , &gtpClusters_, &bgtpClusters_);
    tree_->SetBranchAddress(mcColl_.c_str() , &mcParts_, &bmcParts_);

    // Parameters for beam of 1.92 GeV
	if(beamE_ == 1.92){
		CLUSTERENERGYTHRESHOLD = 0.05;
        CLUSTERENERGYMIN = 0.15; // minimum of cluster energy
        CLUSTERENERGYMAX = 1.5; // maximum of cluster energy
        CLUSTERNHTSMIN = 2; // minimum for number of cluster's hits
        CLUSTERXMIN = 4; // x min of clusters

        //Parameters of cut functions for X
        pos_top_topCutX[0] = 20.6863;
        pos_top_topCutX[1] = 0.90564;
        pos_top_botCutX[0] = -11.0944;
        pos_top_botCutX[1] = 0.824505;

        neg_top_topCutX[0] = 21.8343;
        neg_top_topCutX[1] = 0.856248;
        neg_top_botCutX[0] = -20.3702;
        neg_top_botCutX[1] = 0.914624;

        pos_bot_topCutX[0] = 20.77;
        pos_bot_topCutX[1] = 0.905562;
        pos_bot_botCutX[0] = -11.715;
        pos_bot_botCutX[1] = 0.826122;

        neg_bot_topCutX[0] = 23.7274;
        neg_bot_topCutX[1] = 0.859316;
        neg_bot_botCutX[0] = -21.9968;
        neg_bot_botCutX[1] = 0.911893;

        //Parameters of cut functions for Y

        pos_top_topCutY[0] = 9.01733;
        pos_top_topCutY[1] = 0.946667;
        pos_top_botCutY[0] = -7.95948;
        pos_top_botCutY[1] = 0.859626;

        neg_top_topCutY[0] = 9.93097;
        neg_top_topCutY[1] = 0.892269;
        neg_top_botCutY[0] = -7.77353;
        neg_top_botCutY[1] = 0.900972;

        pos_bot_topCutY[0] = 6.50821;
        pos_bot_topCutY[1] = 0.836669;
        pos_bot_botCutY[0] = -8.03496;
        pos_bot_botCutY[1] = 0.956376;

        neg_bot_topCutY[0] = 6.74298;
        neg_bot_topCutY[1] = 0.888922;
        neg_bot_botCutY[0] = -8.77968;
        neg_bot_botCutY[1] = 0.908499;

        //Parameters of cut function for PDE; 99% based on rad sample
        pars_pde[0] = 0.225874;
        pars_pde[1] = -0.0180655;
        pars_pde[2] = 0.00101429;
        pars_pde[3] = -2.30553e-05;

	}

    //Cut functions for X
    func_pos_top_topCutX = new TF1("func_pos_top_topCutX", "pol1", 50, 390);
    func_pos_top_topCutX->SetParameters(pos_top_topCutX);
    func_pos_top_botCutX = new TF1("func_pos_top_botCutX", "pol1", 50, 390);
    func_pos_top_botCutX->SetParameters(pos_top_botCutX);

    func_neg_top_topCutX = new TF1("func_neg_top_topCutX", "pol1", -300, 40);
    func_neg_top_topCutX->SetParameters(neg_top_topCutX);
    func_neg_top_botCutX = new TF1("func_neg_top_botCutX", "pol1", -300, 40);
    func_neg_top_botCutX->SetParameters(neg_top_botCutX);

    func_pos_bot_topCutX = new TF1("func_pos_bot_topCutX", "pol1", 50, 390);
    func_pos_bot_topCutX->SetParameters(pos_bot_topCutX);
    func_pos_bot_botCutX = new TF1("func_pos_bot_botCutX", "pol1", 50, 390);
    func_pos_bot_botCutX->SetParameters(pos_bot_botCutX);

    func_neg_bot_topCutX = new TF1("func_neg_bot_topCutX", "pol1", -300, 40);
    func_neg_bot_topCutX->SetParameters(neg_bot_topCutX);
    func_neg_bot_botCutX = new TF1("func_neg_bot_botCutX", "pol1", -300, 40);
    func_neg_bot_botCutX->SetParameters(neg_bot_botCutX);

    //Cut functions for Y
    func_pos_top_topCutY = new TF1("func_pos_top_topCutY", "pol1", 30, 90);
    func_pos_top_topCutY->SetParameters(pos_top_topCutY);
    func_pos_top_botCutY = new TF1("func_pos_top_botCutY", "pol1", 30, 90);
    func_pos_top_botCutY->SetParameters(pos_top_botCutY);

    func_neg_top_topCutY = new TF1("func_neg_top_topCutY", "pol1", 30, 90);
    func_neg_top_topCutY->SetParameters(neg_top_topCutY);
    func_neg_top_botCutY = new TF1("func_neg_top_botCutY", "pol1", 30, 90);
    func_neg_top_botCutY->SetParameters(neg_top_botCutY);

    func_pos_bot_topCutY = new TF1("func_pos_bot_topCutY", "pol1", -90, -30);
    func_pos_bot_topCutY->SetParameters(pos_bot_topCutY);
    func_pos_bot_botCutY = new TF1("func_pos_bot_botCutY", "pol1", -90, -30);
    func_pos_bot_botCutY->SetParameters(pos_bot_botCutY);

    func_neg_bot_topCutY = new TF1("func_neg_bot_topCutY", "pol1", -90, -30);
    func_neg_bot_topCutY->SetParameters(neg_bot_topCutY);
    func_neg_bot_botCutY = new TF1("func_neg_bot_botCutY", "pol1", -90, -30);
    func_neg_bot_botCutY->SetParameters(neg_bot_botCutY);

    //Cut function for PDE
    func_pde = new TF1("func_pde", "pol3", 0, 23);
    func_pde->SetParameters(pars_pde);

    //Cut function for energy slope
    func_energy_slope = new TF1("func_pde", "pol1", 0, 400);
    func_energy_slope->SetParameters(pars_energy_slope);
}

bool TriggerParametersExtractionAnaProcessor::process(IEvent* ievent) {
    double weight = 1.;

    // Apply tracks collection to filter events
    // Require available tracks with non-nan position at Ecal face and chi2/ndf less than threshold
    // Tracks are splits into four categories based on charge and position at Ecal face: pos at top, neg at top, pos at bot, neg at bot
	int n_tracks = trks_->size();
	histos->Fill1DHisto("n_tracks_h", n_tracks, weight);

	std::vector<Track> tracks_pos_top;
	std::vector<Track> tracks_pos_bot;
	std::vector<Track> tracks_neg_top;
	std::vector<Track> tracks_neg_bot;

	tracks_pos_top.clear();
	tracks_pos_bot.clear();
	tracks_neg_top.clear();
	tracks_neg_bot.clear();

	for(int i = 0; i < n_tracks; i++){
		Track* track = trks_->at(i);

		int charge = track->getCharge();
		histos->Fill1DHisto("charge_tracks_h", charge, weight);

		std::vector<double> positionAtEcal = track->getPositionAtEcal();
		if(!isnan(positionAtEcal[2])) {
			double p = track->getP();
			if(charge == 1) histos->Fill1DHisto("p_pos_tracks_h", p, weight);
			else if(charge == -1) histos->Fill1DHisto("p_neg_tracks_h", p, weight);

			histos->Fill1DHisto("chi2ndf_tracks_h", track->getChi2Ndf(), weight);

			if(track->getChi2Ndf() < CHI2NDFTHRESHOLD){
				histos->Fill2DHisto("xy_positionAtEcal_tracks_hh",positionAtEcal[0], positionAtEcal[1], weight);

				if (charge == 1 && positionAtEcal[1] > 0) tracks_pos_top.push_back(*track);
				else if (charge == 1 && positionAtEcal[1] < 0) tracks_pos_bot.push_back(*track);
				else if (charge == -1 && positionAtEcal[1] > 0) tracks_neg_top.push_back(*track);
				else if (charge == -1 && positionAtEcal[1] < 0) tracks_neg_bot.push_back(*track);
			}
		}
	}

    // Apply GTP clusters collection to tune triggers
    // Clusters are splits into four categories based on xy indices of seed: pos at top, neg at top, pos at bot, neg at bot
	int n_cl = gtpClusters_->size();
	histos->Fill1DHisto("n_clusters_h", n_cl, weight);

	std::vector<CalCluster> clulsters_pos_top;
	std::vector<CalCluster> clulsters_neg_top;
	std::vector<CalCluster> clulsters_pos_bot;
	std::vector<CalCluster> clulsters_neg_bot;
	for(int i = 0; i < n_cl; i++){
		CalCluster* cluster = gtpClusters_->at(i);
		std::vector<double> positionCluster = cluster->getPosition();
		histos->Fill2DHisto("xy_clusters_hh",positionCluster[0], positionCluster[1], weight);

		CalHit* seed = (CalHit*)cluster->getSeed();
		histos->Fill1DHisto("seed_energy_cluster_without_cut_h", seed->getEnergy(), weight);
		histos->Fill1DHisto("energy_cluster_without_cut_h", cluster->getEnergy(), weight);
		histos->Fill1DHisto("n_hits_cluster_without_cut_h", cluster->getNHits(), weight);
		histos->Fill2DHisto("energy_vs_n_hits_cluster_without_cut_hh", cluster->getNHits(), cluster->getEnergy(), weight);

		int ix = seed -> getCrystalIndices()[0];
		int iy = seed -> getCrystalIndices()[1];

		if(ix < 0) histos->Fill1DHisto("n_clusters_xAxis_without_cut_h", ix + 1, weight);
		else histos->Fill1DHisto("n_clusters_xAxis_without_cut_h", ix, weight);

		if(iy > 0 ){
			if(ix > 0) clulsters_pos_top.push_back(*cluster);
			else clulsters_neg_top.push_back(*cluster);
		}
		else {
			if(ix > 0) clulsters_pos_bot.push_back(*cluster);
			else clulsters_neg_bot.push_back(*cluster);
		}
	}

	// Extract analyzable GTP clusters
	// Require events with at least one pos_top + one neg_bot or one pos_bot + one neg_top tracks
	// In each categories: analyzable clusters pass through cut functions for X/Y of track extrapolation at Ecal face vs. GTP cluster
	std::vector<CalCluster> clulsters_pos_top_cut;
	std::vector<CalCluster> clulsters_neg_top_cut;
	std::vector<CalCluster> clulsters_pos_bot_cut;
	std::vector<CalCluster> clulsters_neg_bot_cut;

	if( ( tracks_pos_top.size() >= 1 && tracks_neg_bot.size() >= 1 ) || ( tracks_pos_bot.size() >= 1 && tracks_neg_top.size() >= 1)  ){
		for(int i = 0; i < clulsters_pos_top.size(); i++){
			CalCluster cluster = clulsters_pos_top.at(i);
			std::vector<double> positionCluster = cluster.getPosition();

			for(int j = 0; j < tracks_pos_top.size(); j++){
				Track track = tracks_pos_top.at(j);
				std::vector<double> positionAtEcal = track.getPositionAtEcal();
				double delta_r = sqrt(pow(positionCluster[0] - positionAtEcal[0],2) + pow(positionCluster[1] - positionAtEcal[1],2));
				double p = track.getP();
				histos->Fill2DHisto("deltaR_vs_p_pos_top_hh", p, delta_r, weight);
				histos->Fill2DHisto("trackX_vs_ClusterX_pos_top_hh", positionCluster[0], positionAtEcal[0], weight);
				histos->Fill2DHisto("trackY_vs_ClusterY_pos_top_hh", positionCluster[1], positionAtEcal[1], weight);

				if (positionAtEcal[0]< func_pos_top_topCutX->Eval(positionCluster[0])
						&& positionAtEcal[0] > func_pos_top_botCutX->Eval(positionCluster[0])
						&& positionAtEcal[1] < func_pos_top_topCutY->Eval(positionCluster[1])
						&& positionAtEcal[1] > func_pos_top_botCutY->Eval(positionCluster[1])) {
					clulsters_pos_top_cut.push_back(cluster);
					break;
				}
			}
		}

		for(int i = 0; i < clulsters_neg_top.size(); i++){
			CalCluster cluster = clulsters_neg_top.at(i);
			std::vector<double> positionCluster = cluster.getPosition();
			for(int j = 0; j < tracks_neg_top.size(); j++){
				Track track = tracks_neg_top.at(j);
				std::vector<double> positionAtEcal = track.getPositionAtEcal();
				double delta_r = sqrt(pow(positionCluster[0] - positionAtEcal[0],2) + pow(positionCluster[1] - positionAtEcal[1],2));
				double p = track.getP();
				histos->Fill2DHisto("deltaR_vs_p_neg_top_hh", p, delta_r, weight);
				histos->Fill2DHisto("trackX_vs_ClusterX_neg_top_hh", positionCluster[0], positionAtEcal[0], weight);
				histos->Fill2DHisto("trackY_vs_ClusterY_neg_top_hh", positionCluster[1], positionAtEcal[1], weight);

				if (positionAtEcal[0]< func_neg_top_topCutX->Eval(positionCluster[0])
						&& positionAtEcal[0] > func_neg_top_botCutX->Eval(positionCluster[0])
						&& positionAtEcal[1] < func_neg_top_topCutY->Eval(positionCluster[1])
						&& positionAtEcal[1] > func_neg_top_botCutY->Eval(positionCluster[1])) {
					clulsters_neg_top_cut.push_back(cluster);
					break;
				}
			}
		}

		for(int i = 0; i < clulsters_pos_bot.size(); i++){
			CalCluster cluster = clulsters_pos_bot.at(i);
			std::vector<double> positionCluster = cluster.getPosition();

			for(int j = 0; j < tracks_pos_bot.size(); j++){
				Track track = tracks_pos_bot.at(j);
				std::vector<double> positionAtEcal = track.getPositionAtEcal();
				double delta_r = sqrt(pow(positionCluster[0] - positionAtEcal[0],2) + pow(positionCluster[1] - positionAtEcal[1],2));
				double p = track.getP();
				histos->Fill2DHisto("deltaR_vs_p_pos_bot_hh", p, delta_r, weight);
				histos->Fill2DHisto("trackX_vs_ClusterX_pos_bot_hh", positionCluster[0], positionAtEcal[0], weight);
				histos->Fill2DHisto("trackY_vs_ClusterY_pos_bot_hh", positionCluster[1], positionAtEcal[1], weight);

				if (positionAtEcal[0]< func_pos_bot_topCutX->Eval(positionCluster[0])
						&& positionAtEcal[0] > func_pos_bot_botCutX->Eval(positionCluster[0])
						&& positionAtEcal[1] < func_pos_bot_topCutY->Eval(positionCluster[1])
						&& positionAtEcal[1] > func_pos_bot_botCutY->Eval(positionCluster[1])) {
					clulsters_pos_bot_cut.push_back(cluster);
					break;
				}
			}
		}

		for(int i = 0; i < clulsters_neg_bot.size(); i++){
			CalCluster cluster = clulsters_neg_bot.at(i);
			std::vector<double> positionCluster = cluster.getPosition();
			for(int j = 0; j < tracks_neg_bot.size(); j++){
				Track track = tracks_neg_bot.at(j);
				std::vector<double> positionAtEcal = track.getPositionAtEcal();
				double delta_r = sqrt(pow(positionCluster[0] - positionAtEcal[0],2) + pow(positionCluster[1] - positionAtEcal[1],2));
				double p = track.getP();
				histos->Fill2DHisto("deltaR_vs_p_neg_bot_hh", p, delta_r, weight);
				histos->Fill2DHisto("trackX_vs_ClusterX_neg_bot_hh", positionCluster[0], positionAtEcal[0], weight);
				histos->Fill2DHisto("trackY_vs_ClusterY_neg_bot_hh", positionCluster[1], positionAtEcal[1], weight);

				if (positionAtEcal[0]< func_neg_bot_topCutX->Eval(positionCluster[0])
						&& positionAtEcal[0] > func_neg_bot_botCutX->Eval(positionCluster[0])
						&& positionAtEcal[1] < func_neg_bot_topCutY->Eval(positionCluster[1])
						&& positionAtEcal[1] > func_neg_bot_botCutY->Eval(positionCluster[1])) {
					clulsters_neg_bot_cut.push_back(cluster);
					break;
				}
			}
		}
	}

	// Tuning singles trigger; events contain at least one cluster which energy is larger than threshold
	bool flag_singles = false;
	// Further, events contain at least one cluster in positive region which passes PDE cut
	bool flag_pde = false;
	// Futher, the cluster pass through x min cut
	bool flag_xmin = false;
	// Further, the cluster passes through energy and nhits cuts
	bool flag_energy_nhits = false;

	for(int i = 0; i < clulsters_pos_top_cut.size(); i++){
		CalCluster cluster = clulsters_pos_top_cut.at(i);

		if(cluster.getEnergy() > CLUSTERENERGYTHRESHOLD) flag_singles = true;

		CalHit* seed = (CalHit*) cluster.getSeed();
		int ix = seed->getCrystalIndices()[0];
		if (flag_singles && cluster.getEnergy() >= func_pde->Eval(ix)) flag_pde = true;

		if(flag_singles && flag_pde && ix >= CLUSTERXMIN) flag_xmin = true;

		if(flag_singles && flag_pde && flag_xmin && cluster.getEnergy() >= CLUSTERENERGYMIN && cluster.getEnergy() <= CLUSTERENERGYMAX && cluster.getNHits() >= CLUSTERNHTSMIN)
			flag_energy_nhits = true;
	}

	for(int i = 0; i < clulsters_neg_top_cut.size(); i++){
		if(clulsters_neg_top_cut.at(i).getEnergy() > CLUSTERENERGYTHRESHOLD) flag_singles = true;
	}

	for(int i = 0; i < clulsters_pos_bot_cut.size(); i++){
		CalCluster cluster = clulsters_pos_bot_cut.at(i);

		if(cluster.getEnergy() > CLUSTERENERGYTHRESHOLD) flag_singles = true;

		CalHit* seed = (CalHit*)cluster.getSeed();
		int ix = seed -> getCrystalIndices()[0];
		if(cluster.getEnergy() > func_pde->Eval(ix)) flag_pde = true;

		if(flag_singles && flag_pde && ix >= CLUSTERXMIN) flag_xmin = true;

		if(flag_singles && flag_pde && ix >= CLUSTERXMIN && cluster.getEnergy() >= CLUSTERENERGYMIN && cluster.getEnergy() <= CLUSTERENERGYMAX && cluster.getNHits() >= CLUSTERNHTSMIN)
			flag_energy_nhits = true;
	}

	for(int i = 0; i < clulsters_neg_bot_cut.size(); i++){
		if(clulsters_neg_bot_cut.at(i).getEnergy() > CLUSTERENERGYTHRESHOLD) flag_singles = true;
	}

	if(flag_singles){
		for(int i = 0; i < clulsters_pos_top_cut.size(); i++){
			CalCluster cluster = clulsters_pos_top_cut.at(i);
			CalHit* seed = (CalHit*)cluster.getSeed();

			histos->Fill1DHisto("seed_energy_cluster_h", seed->getEnergy(), weight);

			histos->Fill1DHisto("energy_cluster_h", cluster.getEnergy(), weight);

			histos->Fill1DHisto("n_hits_cluster_h", cluster.getNHits(), weight);

			histos->Fill2DHisto("energy_vs_n_hits_cluster_hh", cluster.getNHits(), cluster.getEnergy(), weight);

			int ix = seed -> getCrystalIndices()[0];
			histos->Fill2DHisto("energy_vs_x_index_hh", ix, cluster.getEnergy(), weight);
			if(ix > 0) histos->Fill2DHisto("energy_vs_x_index_pos_hh", ix, cluster.getEnergy(), weight);

			if(flag_pde){
				histos->Fill1DHisto("seed_energy_cluster_with_pde_h", seed->getEnergy(), weight);

				histos->Fill1DHisto("energy_cluster_with_pde_h", cluster.getEnergy(), weight);

				histos->Fill1DHisto("n_hits_cluster_with_pde_h", cluster.getNHits(), weight);

				histos->Fill2DHisto("energy_vs_n_hits_cluster_with_pde_hh", cluster.getNHits(), cluster.getEnergy(), weight);
			}
		}

		for(int i = 0; i < clulsters_neg_top_cut.size(); i++){
			CalCluster cluster = clulsters_neg_top_cut.at(i);
			CalHit* seed = (CalHit*)cluster.getSeed();

			histos->Fill1DHisto("seed_energy_cluster_h", seed->getEnergy(), weight);

			histos->Fill1DHisto("energy_cluster_h", cluster.getEnergy(), weight);

			histos->Fill1DHisto("n_hits_cluster_h", cluster.getNHits(), weight);

			histos->Fill2DHisto("energy_vs_n_hits_cluster_hh", cluster.getNHits(), cluster.getEnergy(), weight);

			int ix = seed -> getCrystalIndices()[0];
			histos->Fill2DHisto("energy_vs_x_index_hh", ix, cluster.getEnergy(), weight);
			if(ix > 0) histos->Fill2DHisto("energy_vs_x_index_pos_hh", ix, cluster.getEnergy(), weight);

			if(flag_pde){
				histos->Fill1DHisto("seed_energy_cluster_with_pde_h", seed->getEnergy(), weight);

				histos->Fill1DHisto("energy_cluster_with_pde_h", cluster.getEnergy(), weight);

				histos->Fill1DHisto("n_hits_cluster_with_pde_h", cluster.getNHits(), weight);

				histos->Fill2DHisto("energy_vs_n_hits_cluster_with_pde_hh", cluster.getNHits(), cluster.getEnergy(), weight);
			}
		}

		for(int i = 0; i < clulsters_pos_bot_cut.size(); i++){
			CalCluster cluster = clulsters_pos_bot_cut.at(i);
			CalHit* seed = (CalHit*)cluster.getSeed();

			histos->Fill1DHisto("seed_energy_cluster_h", seed->getEnergy(), weight);

			histos->Fill1DHisto("energy_cluster_h", cluster.getEnergy(), weight);

			histos->Fill1DHisto("n_hits_cluster_h", cluster.getNHits(), weight);

			histos->Fill2DHisto("energy_vs_n_hits_cluster_hh", cluster.getNHits(), cluster.getEnergy(), weight);

			int ix = seed -> getCrystalIndices()[0];
			histos->Fill2DHisto("energy_vs_x_index_hh", ix, cluster.getEnergy(), weight);
			if(ix > 0) histos->Fill2DHisto("energy_vs_x_index_pos_hh", ix, cluster.getEnergy(), weight);

			if(flag_pde){
				histos->Fill1DHisto("seed_energy_cluster_with_pde_h", seed->getEnergy(), weight);

				histos->Fill1DHisto("energy_cluster_with_pde_h", cluster.getEnergy(), weight);

				histos->Fill1DHisto("n_hits_cluster_with_pde_h", cluster.getNHits(), weight);

				histos->Fill2DHisto("energy_vs_n_hits_cluster_with_pde_hh", cluster.getNHits(), cluster.getEnergy(), weight);
			}
		}

		for(int i = 0; i < clulsters_neg_bot_cut.size(); i++){
			CalCluster cluster = clulsters_neg_bot_cut.at(i);
			CalHit* seed = (CalHit*)cluster.getSeed();

			histos->Fill1DHisto("seed_energy_cluster_h", seed->getEnergy(), weight);

			histos->Fill1DHisto("energy_cluster_h", cluster.getEnergy(), weight);

			histos->Fill1DHisto("n_hits_cluster_h", cluster.getNHits(), weight);

			histos->Fill2DHisto("energy_vs_n_hits_cluster_hh", cluster.getNHits(), cluster.getEnergy(), weight);

			int ix = seed -> getCrystalIndices()[0];
			histos->Fill2DHisto("energy_vs_x_index_hh", ix, cluster.getEnergy(), weight);
			if(ix > 0) histos->Fill2DHisto("energy_vs_x_index_pos_hh", ix, cluster.getEnergy(), weight);

			if(flag_pde){
				histos->Fill1DHisto("seed_energy_cluster_with_pde_h", seed->getEnergy(), weight);

				histos->Fill1DHisto("energy_cluster_with_pde_h", cluster.getEnergy(), weight);

				histos->Fill1DHisto("n_hits_cluster_with_pde_h", cluster.getNHits(), weight);

				histos->Fill2DHisto("energy_vs_n_hits_cluster_with_pde_hh", cluster.getNHits(), cluster.getEnergy(), weight);
			}
		}
	}

	double truthMass = -999;
	double truthEnergy = -999;
	TLorentzVector *lorentzVectorTruth = new TLorentzVector();

	for (int i = 0; i < mcParts_->size(); i++) {
		MCParticle* mc_particle = mcParts_->at(i);

		if (mc_particle->getPDG() == 622) {
			truthMass = mc_particle->getMass();
			truthEnergy = mc_particle->getEnergy();
			std::vector<double> mom = mc_particle->getMomentum();
			lorentzVectorTruth->SetXYZM(mom[0], mom[1], mom[2], ELECTRONMASS);
		}
	}

	for (int i = 0; i < tracks_pos_top.size(); i++) {
		Track trackPos = tracks_pos_top.at(i);
		std::vector<double> momPos = trackPos.getMomentum();
		TLorentzVector *lorentzVectorPos = new TLorentzVector();
		lorentzVectorPos->SetXYZM(momPos[0], momPos[1], momPos[2], ELECTRONMASS);

		for(int j = 0; j < tracks_neg_bot.size(); j++){
			histos->Fill1DHisto("truth_mass_track_chi2_cut_h", truthMass, weight);
			if (flag_singles) histos->Fill1DHisto("truth_mass_singles_analyzable_h", truthMass, weight);
			if (flag_singles && flag_pde && flag_energy_nhits) histos->Fill1DHisto("truth_mass_singles_triggered_h", truthMass, weight);

			Track trackNeg = tracks_neg_bot.at(j);
			std::vector<double> momNeg = trackNeg.getMomentum();
			TLorentzVector *lorentzVectorNeg = new TLorentzVector();
			lorentzVectorNeg->SetXYZM(momNeg[0], momNeg[1], momNeg[2], ELECTRONMASS);

			double invariant_mass = (*lorentzVectorPos + *lorentzVectorNeg).M();
			histos->Fill1DHisto("invariant_mass_track_chi2_cut_h", invariant_mass, weight);
			if(flag_singles) histos->Fill1DHisto("invariant_mass_singles_analyzable_h", invariant_mass, weight);
			if(flag_singles && flag_pde && flag_xmin && flag_energy_nhits) histos->Fill1DHisto("invariant_mass_singles_triggered_h", invariant_mass, weight);

			histos->Fill2DHisto("truth_vs_invariant_mass_track_chi2_cut_hh", truthMass, invariant_mass, weight);

			double pSum = trackPos.getP() + trackNeg.getP();
			histos->Fill1DHisto("p_sum_track_chi2_cut_h", pSum, weight);

			TLorentzVector *lorentzVectorBeam = new TLorentzVector();
			lorentzVectorBeam->SetXYZM(0, 0, sqrt(pow(beamE_, 2) - pow(ELECTRONMASS, 2)), ELECTRONMASS);
			TLorentzVector *lorentzVectorRecoiledElectron = new TLorentzVector();
			*lorentzVectorRecoiledElectron = *lorentzVectorBeam - *lorentzVectorTruth;
			histos->Fill2DHisto("theta_recoiled_electron_vs_true_mass_track_chi2_cut_hh", truthMass, lorentzVectorRecoiledElectron->Theta(), weight);
			histos->Fill2DHisto("p_recoiled_electron_vs_true_mass_track_chi2_cut_hh", truthMass, lorentzVectorRecoiledElectron->P(), weight);
		}
	}

	for(int i = 0; i < tracks_pos_bot.size(); i++){
		Track trackPos = tracks_pos_bot.at(i);
		std::vector<double> momPos = trackPos.getMomentum();
		TLorentzVector *lorentzVectorPos = new TLorentzVector();
		lorentzVectorPos->SetXYZM(momPos[0], momPos[1], momPos[2], ELECTRONMASS);

		for(int j = 0; j < tracks_neg_top.size(); j++){
			histos->Fill1DHisto("truth_mass_track_chi2_cut_h", truthMass, weight);
			if (flag_singles) histos->Fill1DHisto("truth_mass_singles_analyzable_h", truthMass, weight);
			if (flag_singles && flag_pde && flag_energy_nhits) histos->Fill1DHisto("truth_mass_singles_triggered_h", truthMass, weight);

			Track trackNeg = tracks_neg_top.at(j);
			std::vector<double> momNeg = trackNeg.getMomentum();
			TLorentzVector *lorentzVectorNeg = new TLorentzVector();
			lorentzVectorNeg->SetXYZM(momNeg[0], momNeg[1], momNeg[2], ELECTRONMASS);

			double invariant_mass = (*lorentzVectorPos + *lorentzVectorNeg).M();
			histos->Fill1DHisto("invariant_mass_track_chi2_cut_h", invariant_mass, weight);
			if(flag_singles) histos->Fill1DHisto("invariant_mass_singles_analyzable_h", invariant_mass, weight);
			if(flag_singles && flag_pde && flag_energy_nhits) histos->Fill1DHisto("invariant_mass_singles_triggered_h", invariant_mass, weight);

			histos->Fill2DHisto("truth_vs_invariant_mass_track_chi2_cut_hh", truthMass, invariant_mass, weight);

			double pSum = trackPos.getP() + trackNeg.getP();
			histos->Fill1DHisto("p_sum_track_chi2_cut_h", pSum, weight);

			TLorentzVector *lorentzVectorBeam = new TLorentzVector();
			lorentzVectorBeam->SetXYZM(0, 0, sqrt(pow(beamE_, 2) - pow(ELECTRONMASS, 2)), ELECTRONMASS);
			TLorentzVector *lorentzVectorRecoiledElectron = new TLorentzVector();
			*lorentzVectorRecoiledElectron = *lorentzVectorBeam - *lorentzVectorTruth;
			histos->Fill2DHisto("theta_recoiled_electron_vs_true_mass_track_chi2_cut_hh", truthMass, lorentzVectorRecoiledElectron->Theta(), weight);
			histos->Fill2DHisto("p_recoiled_electron_vs_true_mass_track_chi2_cut_hh", truthMass, lorentzVectorRecoiledElectron->P(), weight);
		}
	}

	// tuning pairs trigger; events contain at least one pos_top + one neg_bot or one pos_bot + one nge_top clusters which energies are larger than threshold
	bool flag_pairs_pos_top = false;
	bool flag_pairs_neg_top = false;
	bool flag_pairs_pos_bot = false;
	bool flag_pairs_neg_bot = false;
	bool flag_pairs = false;
	// Further, passes through energy slope for cluster with lowest energy
	bool flag_energy_slope = false;
	for(int i = 0; i < clulsters_pos_top_cut.size(); i++){
		if(clulsters_pos_top_cut.at(i).getEnergy() > CLUSTERENERGYTHRESHOLD) flag_pairs_pos_top = true;
	}

	for(int i = 0; i < clulsters_neg_top_cut.size(); i++){
		if(clulsters_neg_top_cut.at(i).getEnergy() > CLUSTERENERGYTHRESHOLD) flag_pairs_neg_top = true;
	}

	for(int i = 0; i < clulsters_pos_bot_cut.size(); i++){
		if(clulsters_pos_bot_cut.at(i).getEnergy() > CLUSTERENERGYTHRESHOLD) flag_pairs_pos_bot = true;
	}

	for(int i = 0; i < clulsters_neg_bot_cut.size(); i++){
		if(clulsters_neg_bot_cut.at(i).getEnergy() > CLUSTERENERGYTHRESHOLD) flag_pairs_neg_bot = true;
	}

	if( ( flag_pairs_pos_top && flag_pairs_neg_bot) || (flag_pairs_neg_top && flag_pairs_pos_bot)) flag_pairs = true;

	if(flag_pairs){
		for(int i = 0; i < clulsters_pos_top_cut.size(); i++){
			CalCluster clusterTop = clulsters_pos_top_cut.at(i);

			for(int j = 0; j < clulsters_neg_bot_cut.size(); j++){
				CalCluster clusterBot = clulsters_neg_bot_cut.at(j);

				histos->Fill1DHisto("energy_sum_cluster_h", clusterTop.getEnergy() + clusterBot.getEnergy(), weight);
				histos->Fill1DHisto("energy_difference_cluster_h", fabs(clusterTop.getEnergy() - clusterBot.getEnergy()), weight);
				histos->Fill1DHisto("coplanarity_cluster_h", getValueCoplanarity(clusterTop, clusterBot), weight);

				std::vector<double> variablesForEnergySlopeCut = getVariablesForEnergySlopeCut(clusterTop, clusterBot);
				histos->Fill2DHisto("energy_vs_r_lowerest_energy_cluster_hh", variablesForEnergySlopeCut[0], variablesForEnergySlopeCut[1], weight);

				if(variablesForEnergySlopeCut[1] > func_energy_slope->Eval(variablesForEnergySlopeCut[0])){
					histos->Fill1DHisto("energy_sum_cluster_with_energy_slope_cut_h", clusterTop.getEnergy() + clusterBot.getEnergy(), weight);
					histos->Fill1DHisto("energy_difference_cluster_with_energy_slope_cut_h", fabs(clusterTop.getEnergy() - clusterBot.getEnergy()), weight);
					histos->Fill1DHisto("coplanarity_cluster_with_energy_slope_cut_h", getValueCoplanarity(clusterTop, clusterBot), weight);
				}
			}
		}

		for(int i = 0; i < clulsters_neg_top_cut.size(); i++){
			CalCluster clusterTop = clulsters_neg_top_cut.at(i);

			for(int j = 0; j < clulsters_pos_bot_cut.size(); j++){
				CalCluster clusterBot = clulsters_pos_bot_cut.at(j);

				histos->Fill1DHisto("energy_sum_cluster_h", clusterTop.getEnergy() + clusterBot.getEnergy(), weight);
				histos->Fill1DHisto("energy_difference_cluster_h", fabs(clusterTop.getEnergy() - clusterBot.getEnergy()), weight);
				histos->Fill1DHisto("coplanarity_cluster_h", getValueCoplanarity(clusterTop, clusterBot), weight);

				std::vector<double> variablesForEnergySlopeCut = getVariablesForEnergySlopeCut(clusterTop, clusterBot);
				histos->Fill2DHisto("energy_vs_r_lowerest_energy_cluster_hh", variablesForEnergySlopeCut[0], variablesForEnergySlopeCut[1], weight);

				if(variablesForEnergySlopeCut[1] > func_energy_slope->Eval(variablesForEnergySlopeCut[0])){
					histos->Fill1DHisto("energy_sum_cluster_with_energy_slope_cut_h", clusterTop.getEnergy() + clusterBot.getEnergy(), weight);
					histos->Fill1DHisto("energy_difference_cluster_with_energy_slope_cut_h", fabs(clusterTop.getEnergy() - clusterBot.getEnergy()), weight);
					histos->Fill1DHisto("coplanarity_cluster_with_energy_slope_cut_h", getValueCoplanarity(clusterTop, clusterBot), weight);
				}
			}
		}
	}


    return true;
}

void TriggerParametersExtractionAnaProcessor::finalize() {
    histos->saveHistos(outF_, anaName_.c_str());
    delete histos;
    histos = nullptr;

}

std::vector<double> TriggerParametersExtractionAnaProcessor::getCrystalPosition(CalCluster cluster){
	CalHit* seed = (CalHit*)cluster.getSeed();
	int ix = seed->getCrystalIndices()[0];
	int iy = seed->getCrystalIndices()[1];

    // Get the position map.
	std::vector<double> position;
    if (ix < 1) {
    	position.push_back(positionMap[5 - iy][22 - ix][0]);
    	position.push_back(positionMap[5 - iy][22 - ix][2]);
    	position.push_back(positionMap[5 - iy][22 - ix][1]);
    } else {
    	position.push_back(positionMap[5 - iy][23 - ix][0]);
    	position.push_back(positionMap[5 - iy][23 - ix][2]);
    	position.push_back(positionMap[5 - iy][23 - ix][1]);
    }

    // Return the corrected mapped position.
    return position;
}

double TriggerParametersExtractionAnaProcessor::getValueCoplanarity(CalCluster clusterTop, CalCluster clusterBot) {
    // Get the variables used by the calculation.
	std::vector<double> positionTop = getCrystalPosition(clusterTop);
	std::vector<double> positionBot = getCrystalPosition(clusterBot);

	int angleTop = (int) std::round(atan(positionTop[0] / positionTop[1]) * 180.0 / PI);
	int angleBot = (int) std::round(atan(positionBot[0] / positionBot[1]) * 180.0 / PI);

	 // Calculate the coplanarity cut value.
	return abs(angleTop - angleBot);
}

std::vector<double> TriggerParametersExtractionAnaProcessor::getVariablesForEnergySlopeCut(CalCluster clusterTop, CalCluster clusterBot){
	double energyTop = clusterTop.getEnergy();
	double energyBot = clusterBot.getEnergy();

	std::vector<double> variables;

	if(energyTop < energyBot){
		// Get the low energy cluster radial distance
		std::vector<double> positionTop = getCrystalPosition(clusterTop);
		double slopeParamR = sqrt( pow(positionTop[0],2) + pow( positionTop[1], 2) );

		variables.push_back(slopeParamR);
		variables.push_back(energyTop);

		return variables;
	}

	else{
		// Get the low energy cluster radial distance
		std::vector<double> positionBot = getCrystalPosition(clusterBot);
		double slopeParamR = sqrt( pow(positionBot[0],2) + pow( positionBot[1], 2) );

		variables.push_back(slopeParamR);
		variables.push_back(energyBot);

		return variables;
	}

}

DECLARE_PROCESSOR(TriggerParametersExtractionAnaProcessor);
