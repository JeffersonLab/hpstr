/**
 *@file TriggerParametersExtractionAnaProcessor.cxx
 *@author Tongtong, UNH
 */

#include "CalHit.h"
#include "TriggerParametersExtractionAnaProcessor.h"

#include <iostream>

#include "TF1.h"
#include "math.h"

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
    tree_->SetBranchAddress(ecalClusColl_.c_str() , &ecalClusters_, &becalClusters_);
}

bool TriggerParametersExtractionAnaProcessor::process(IEvent* ievent) {
    double weight = 1.;

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

			if(track->getChi2Ndf() < 20){
				std::vector<double> positionAtEcal = track->getPositionAtEcal();
				histos->Fill2DHisto("xy_positionAtEcal_tracks_hh",positionAtEcal[0], positionAtEcal[1], weight);

				if (charge == 1 && positionAtEcal[1] > 0) tracks_pos_top.push_back(*track);
				else if (charge == 1 && positionAtEcal[1] < 0) tracks_pos_bot.push_back(*track);
				else if (charge == -1 && positionAtEcal[1] > 0) tracks_neg_top.push_back(*track);
				else if (charge == -1 && positionAtEcal[1] < 0) tracks_neg_bot.push_back(*track);
			}
		}
	}

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
		int x_index = seed -> getCrystalIndices()[0];
		int y_index = seed -> getCrystalIndices()[1];
		if(y_index > 0 ){
			if(x_index > 0) clulsters_pos_top.push_back(*cluster);
			else clulsters_neg_top.push_back(*cluster);
		}
		else {
			if(x_index > 0) clulsters_pos_bot.push_back(*cluster);
			else clulsters_neg_bot.push_back(*cluster);
		}
	}

	if( ( tracks_pos_top.size() >= 1 && tracks_neg_bot.size() >= 1 ) || ( tracks_pos_bot.size() >= 1 && tracks_neg_top.size() >= 1)  ){
		for(int i = 0; i < clulsters_pos_top.size(); i++){
			CalCluster cluster = clulsters_pos_top.at(i);
			std::vector<double> positionCluster = cluster.getPosition();

			for(int j = 0; j < tracks_pos_top.size(); j++){
				Track track = tracks_pos_top.at(j);
				std::vector<double> positionAtEcal = track.getPositionAtEcal();
				double delta_r = sqrt(pow(positionCluster[0] - positionAtEcal[0],2) + pow(positionCluster[1] - positionAtEcal[1],2));
				histos->Fill2DHisto("deltaR_vs_p_pos_top_hh", track.getP(), delta_r, weight);
			}
		}

		for(int i = 0; i < clulsters_neg_top.size(); i++){
			CalCluster cluster = clulsters_neg_top.at(i);
			std::vector<double> positionCluster = cluster.getPosition();
			for(int j = 0; j < tracks_neg_top.size(); j++){
				Track track = tracks_neg_top.at(j);
				std::vector<double> positionAtEcal = track.getPositionAtEcal();
				double delta_r = sqrt(pow(positionCluster[0] - positionAtEcal[0],2) + pow(positionCluster[1] - positionAtEcal[1],2));
				histos->Fill2DHisto("deltaR_vs_p_neg_top_hh", track.getP(), delta_r, weight);
			}
		}

		for(int i = 0; i < clulsters_pos_bot.size(); i++){
			CalCluster cluster = clulsters_pos_bot.at(i);
			std::vector<double> positionCluster = cluster.getPosition();

			for(int j = 0; j < tracks_pos_bot.size(); j++){
				Track track = tracks_pos_bot.at(j);
				std::vector<double> positionAtEcal = track.getPositionAtEcal();
				double delta_r = sqrt(pow(positionCluster[0] - positionAtEcal[0],2) + pow(positionCluster[1] - positionAtEcal[1],2));
				histos->Fill2DHisto("deltaR_vs_p_pos_bot_hh", track.getP(), delta_r, weight);
			}
		}

		for(int i = 0; i < clulsters_neg_bot.size(); i++){
			CalCluster cluster = clulsters_neg_bot.at(i);
			std::vector<double> positionCluster = cluster.getPosition();
			for(int j = 0; j < tracks_neg_bot.size(); j++){
				Track track = tracks_neg_bot.at(j);
				std::vector<double> positionAtEcal = track.getPositionAtEcal();
				double delta_r = sqrt(pow(positionCluster[0] - positionAtEcal[0],2) + pow(positionCluster[1] - positionAtEcal[1],2));
				histos->Fill2DHisto("deltaR_vs_p_neg_bot_hh", track.getP(), delta_r, weight);
			}
		}
	}

	for(int i = 0; i < n_cl; i++){
		CalCluster* cluster = ecalClusters_->at(i);
		CalHit* seed = (CalHit*)cluster->getSeed();

		histos->Fill1DHisto("seed_energy_cluster_h", seed->getEnergy(), weight);

		histos->Fill1DHisto("energy_cluster_h", cluster->getEnergy(), weight);

		histos->Fill1DHisto("n_hits_cluster_h", cluster->getNHits(), weight);

		histos->Fill2DHisto("energy_vs_n_hits_cluster_hh", cluster->getNHits(), cluster->getEnergy(), weight);

		int x_index = seed -> getCrystalIndices()[0];
		histos->Fill2DHisto("energy_vs_x_index_hh", x_index, cluster->getEnergy(), weight);
		if(x_index > 0) histos->Fill2DHisto("energy_vs_x_index_pos_hh", x_index, cluster->getEnergy(), weight);

	}

    return true;
}

void TriggerParametersExtractionAnaProcessor::finalize() {
    histos->saveHistos(outF_, anaName_.c_str());
    delete histos;
    histos = nullptr;

}

DECLARE_PROCESSOR(TriggerParametersExtractionAnaProcessor);
