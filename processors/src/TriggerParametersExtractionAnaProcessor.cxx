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

	std::vector<CalCluster> clulsters_top;
	std::vector<CalCluster> clulsters_bot;
	for(int i = 0; i < n_cl; i++){
		CalCluster* cluster = ecalClusters_->at(i);
		std::vector<double> positionCluster = cluster->getPosition();
		histos->Fill2DHisto("xy_clusters_hh",positionCluster[0], positionCluster[1], weight);
		if(positionCluster[1] > 0 ) clulsters_top.push_back(*cluster);
		else if(positionCluster[1] < 0 ) clulsters_bot.push_back(*cluster);
	}

	if( ( tracks_pos_top.size() >= 1 && tracks_neg_bot.size() >= 1 ) || ( tracks_pos_bot.size() >= 1 && tracks_neg_top.size() >= 1)  ){
		for(int i = 0; i < clulsters_top.size(); i++){
			CalCluster cluster = clulsters_top.at(i);
			std::vector<double> positionCluster = cluster.getPosition();

			for(int j = 0; j < tracks_pos_top.size(); j++){
				Track track = tracks_pos_top.at(j);
				std::vector<double> positionAtEcal = track.getPositionAtEcal();
				double delta_r = sqrt(pow(positionCluster[0] - positionAtEcal[0],2) + pow(positionCluster[1] - positionAtEcal[1],2));
				histos->Fill2DHisto("deltaR_vs_p_pos_top_hh", track.getP(), delta_r, weight);
			}

			for(int j = 0; j < tracks_neg_top.size(); j++){
				Track track = tracks_neg_top.at(j);
				std::vector<double> positionAtEcal = track.getPositionAtEcal();
				double delta_r = sqrt(pow(positionCluster[0] - positionAtEcal[0],2) + pow(positionCluster[1] - positionAtEcal[1],2));
				histos->Fill2DHisto("deltaR_vs_p_neg_top_hh", track.getP(), delta_r, weight);
			}
		}

		for(int i = 0; i < clulsters_bot.size(); i++){
			CalCluster cluster = clulsters_bot.at(i);
			std::vector<double> positionCluster = cluster.getPosition();

			for(int j = 0; j < tracks_pos_bot.size(); j++){
				Track track = tracks_pos_bot.at(j);
				std::vector<double> positionAtEcal = track.getPositionAtEcal();
				double delta_r = sqrt(pow(positionCluster[0] - positionAtEcal[0],2) + pow(positionCluster[1] - positionAtEcal[1],2));
				histos->Fill2DHisto("deltaR_vs_p_pos_bot_hh", track.getP(), delta_r, weight);
			}

			for(int j = 0; j < tracks_neg_bot.size(); j++){
				Track track = tracks_neg_bot.at(j);
				std::vector<double> positionAtEcal = track.getPositionAtEcal();
				double delta_r = sqrt(pow(positionCluster[0] - positionAtEcal[0],2) + pow(positionCluster[1] - positionAtEcal[1],2));
				histos->Fill2DHisto("deltaR_vs_p_neg_bot_hh", track.getP(), delta_r, weight);
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

DECLARE_PROCESSOR(TriggerParametersExtractionAnaProcessor);
