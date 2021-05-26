/**
 *@file TriggerParametersExtractionAnaProcessor.cxx
 *@author Tongtong, UNH
 */

#include "CalHit.h"
#include "TriggerParametersExtractionAnaProcessor.h"

#include <iostream>

#include "TF1.h"
#include "math.h"

#define CLUSTERENERGYTHRESHOLD 0.1
#define CHI2NDFTHRESHOLD 20
#define PI 3.14159265358979

TriggerParametersExtractionAnaProcessor::TriggerParametersExtractionAnaProcessor(const std::string& name, Process& process) : Processor(name,process) {

}

TriggerParametersExtractionAnaProcessor::~TriggerParametersExtractionAnaProcessor(){}

void TriggerParametersExtractionAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring EcalTimingAnaProcessor" <<std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        anaName_         = parameters.getString("anaName");
        histCfgFilename_      = parameters.getString("histCfg",histCfgFilename_);
        trkColl_    = parameters.getString("trkColl");
        ecalClusColl_    = parameters.getString("ecalClusColl");
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
    tree_->SetBranchAddress(ecalClusColl_.c_str() , &ecalClusters_, &becalClusters_);\

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

	std::vector<Track> tracks_with_preselect;
	tracks_with_preselect.clear();

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
				std::vector<double> positionAtEcal = track->getPositionAtEcal();
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
	int n_cl = ecalClusters_->size();
	histos->Fill1DHisto("n_clusters_h", n_cl, weight);

	std::vector<CalCluster> clulsters_pos_top;
	std::vector<CalCluster> clulsters_neg_top;
	std::vector<CalCluster> clulsters_pos_bot;
	std::vector<CalCluster> clulsters_neg_bot;
	for(int i = 0; i < n_cl; i++){
		CalCluster* cluster = ecalClusters_->at(i);
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
	// In each categories: analyzable clusters pass through cut functions for detal_r (between position at Ecal face where tracks extrapolate and position of clusters) vs p of tracks
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

	// tuning singles trigger; events contain at least one cluster which energy is larger than threshold
	bool flag_singles = false;
	for(int i = 0; i < clulsters_pos_top_cut.size(); i++){
		if(clulsters_pos_top_cut.at(i).getEnergy() > CLUSTERENERGYTHRESHOLD) flag_singles = true;
	}

	for(int i = 0; i < clulsters_neg_top_cut.size(); i++){
		if(clulsters_neg_top_cut.at(i).getEnergy() > CLUSTERENERGYTHRESHOLD) flag_singles = true;
	}

	for(int i = 0; i < clulsters_pos_bot_cut.size(); i++){
		if(clulsters_pos_bot_cut.at(i).getEnergy() > CLUSTERENERGYTHRESHOLD) flag_singles = true;
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
		}
	}

	// tuning pairs trigger; events contain at least one pos_top + one neg_bot or one pos_bot + one nge_top clusters which energies are larger than threshold
	bool flag_pairs_pos_top = false;
	bool flag_pairs_neg_top = false;
	bool flag_pairs_pos_bot = false;
	bool flag_pairs_neg_bot = false;
	bool flag_pairs = false;
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
