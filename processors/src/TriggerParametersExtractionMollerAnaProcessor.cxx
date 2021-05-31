/**
 *@file TriggerParametersExtractionMollerAnaProcessor.cxx
 *@author Tongtong, UNH
 */

#include "TriggerParametersExtractionMollerAnaProcessor.h"

#include <iostream>

#include "TF1.h"
#include "math.h"

#define ELECTRONMASS 0.000510998950 // GeV
#define PI 3.14159265358979
#define CHI2NDFTHRESHOLD 20
#define CLUSTERENERGYTHRESHOLD 0.1 // threshold of cluster energy for analyzable events
#define CLUSTERENERGYMIN 0.3 // minimum of cluster energy
#define CLUSTERENERGYMAX 2.7 // maximum of cluster energy
#define CLUSTERNHTSMIN 2 // minimum for number of cluster's hits


TriggerParametersExtractionMollerAnaProcessor::TriggerParametersExtractionMollerAnaProcessor(const std::string& name, Process& process) : Processor(name,process) {

}

TriggerParametersExtractionMollerAnaProcessor::~TriggerParametersExtractionMollerAnaProcessor(){}

void TriggerParametersExtractionMollerAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring EcalTimingAnaProcessor" <<std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        anaName_         = parameters.getString("anaName");
        histCfgFilename_      = parameters.getString("histCfg",histCfgFilename_);
        trkColl_    = parameters.getString("trkColl");
        ecalClusColl_    = parameters.getString("ecalClusColl");
        mcColl_  = parameters.getString("mcColl",mcColl_);
        vtxColl_ = parameters.getString("vtxColl",vtxColl_);
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }
}

void TriggerParametersExtractionMollerAnaProcessor::initialize(TTree* tree) {
	_ah =  std::make_shared<AnaHelpers>();

    tree_= tree;
    // init histos
    histos = new TriggerParametersExtractionMollerAnaHistos(anaName_.c_str());
    histos->loadHistoConfig(histCfgFilename_);
    histos->DefineHistos();

    // init TTree
    tree_->SetBranchAddress(trkColl_.c_str() , &trks_, &btrks_);
    tree_->SetBranchAddress(ecalClusColl_.c_str() , &ecalClusters_, &becalClusters_);
    tree_->SetBranchAddress(mcColl_.c_str() , &mcParts_, &bmcParts_);
    tree_->SetBranchAddress(vtxColl_.c_str(), &vtxs_ , &bvtxs_);

    //Cut functions for X
    func_top_topCutX = new TF1("func_top_topCutX", "pol1", -300, 40);
    func_top_topCutX->SetParameters(top_topCutX);
    func_top_botCutX = new TF1("func_top_botCutX", "pol1", -300, 40);
    func_top_botCutX->SetParameters(top_botCutX);

    func_bot_topCutX = new TF1("func_bot_topCutX", "pol1", -300, 40);
    func_bot_topCutX->SetParameters(bot_topCutX);
    func_bot_botCutX = new TF1("func_bot_botCutX", "pol1", -300, 40);
    func_bot_botCutX->SetParameters(bot_botCutX);

    //Cut functions for Y
    func_top_topCutY = new TF1("func_top_topCutY", "pol1", 30, 90);
    func_top_topCutY->SetParameters(top_topCutY);
    func_top_botCutY = new TF1("func_top_botCutY", "pol1", 30, 90);
    func_top_botCutY->SetParameters(top_botCutY);

    func_bot_topCutY = new TF1("func_bot_topCutY", "pol1", -90, -30);
    func_bot_topCutY->SetParameters(bot_topCutY);
    func_bot_botCutY = new TF1("func_bot_botCutY", "pol1", -90, -30);
    func_bot_botCutY->SetParameters(bot_botCutY);
}

bool TriggerParametersExtractionMollerAnaProcessor::process(IEvent* ievent) {
    double weight = 1.;

	int n_tracks = trks_->size();
	histos->Fill1DHisto("n_tracks_h", n_tracks, weight);

	int n_cl = ecalClusters_->size();
	histos->Fill1DHisto("n_clusters_h", n_cl, weight);

	int n_vtxs = vtxs_->size();
	histos->Fill1DHisto("n_vtxs_h", n_vtxs, weight);

	histos->Fill2DHisto("n_clusters_vs_n_tracks_hh", n_tracks, n_cl, weight);
	histos->Fill2DHisto("n_clusters_vs_n_vtxs_hh", n_vtxs, n_cl, weight);
	histos->Fill2DHisto("n_tracks_vs_n_vtxs_hh", n_vtxs, n_tracks, weight);


    for(int i = 0; i < n_vtxs; i++){
        Vertex* vtx = vtxs_->at(i);

        int n_entries = vtx->getParticles()->GetEntries();
        if(n_entries != 2) {
        	std::cout << "Warning: entries of Moller vertex is not 2." << std::endl;
        	return false;
        }

        Particle* particleTop = (Particle*)vtx->getParticles()->At(0);
        Particle* particleBot = (Particle*)vtx->getParticles()->At(1);

        Track trackTop = particleTop->getTrack();
        Track trackBot = particleBot->getTrack();

        std::vector<double> positionAtEcalTop = trackTop.getPositionAtEcal();
        std::vector<double> positionAtEcalBot = trackBot.getPositionAtEcal();

        if(!isnan(positionAtEcalTop[2]) && !isnan(positionAtEcalBot[2]) ){
        	histos->Fill2DHisto("xy_positionAtEcal_vertices_hh", positionAtEcalTop[0], positionAtEcalTop[1], weight);
        	histos->Fill2DHisto("xy_positionAtEcal_vertices_hh", positionAtEcalBot[0], positionAtEcalBot[1], weight);
        }
    }


	std::vector<Track> tracks_top;
	std::vector<Track> tracks_bot;

	tracks_top.clear();
	tracks_bot.clear();

	for(int i = 0; i < n_tracks; i++){
		Track* track = trks_->at(i);

		int charge = track->getCharge();
		histos->Fill1DHisto("charge_tracks_h", charge, weight);

		std::vector<double> positionAtEcal = track->getPositionAtEcal();
		if(charge == -1 && !isnan(positionAtEcal[2])) {

			histos->Fill1DHisto("chi2ndf_tracks_h", track->getChi2Ndf(), weight);

			if(track->getChi2Ndf() < CHI2NDFTHRESHOLD){
				if (positionAtEcal[1] > 0) {
					tracks_top.push_back(*track);
				}
				else if (positionAtEcal[1] < 0) {
					tracks_bot.push_back(*track);
				}
			}
		}
	}

	int n_tracks_top = tracks_top.size();
	int n_tracks_bot = tracks_bot.size();

	histos->Fill1DHisto("n_tracks_top_with_chi2_cut_h", n_tracks_top, weight);
	histos->Fill1DHisto("n_tracks_bot_with_chi2_cut_h", n_tracks_bot, weight);

	for(int i = 0; i < n_tracks_top; i++) {
		Track trackTop = tracks_top.at(i);
		std::vector<double> positionAtEcalTop = trackTop.getPositionAtEcal();
		std::vector<double> momTop= trackTop.getMomentum();
		TLorentzVector* lorentzVectorTop = new TLorentzVector();
		lorentzVectorTop->SetXYZM(momTop[0], momTop[1], momTop[2], ELECTRONMASS);
		double theta_top = lorentzVectorTop->Theta();
		double energy_top = lorentzVectorTop->Energy();

		for(int j = 0; j < n_tracks_bot; j++) {
			Track trackBot = tracks_bot.at(i);
			std::vector<double> positionAtEcalBot = trackBot.getPositionAtEcal();
			std::vector<double> momBot= trackBot.getMomentum();
			TLorentzVector* lorentzVectorBot = new TLorentzVector();
			lorentzVectorBot->SetXYZM(momBot[0], momBot[1], momBot[2], ELECTRONMASS);
			double theta_bot = lorentzVectorBot->Theta();
			double energy_bot = lorentzVectorBot->Energy();

			histos->Fill2DHisto("xy_positionAtEcal_tracks_analyzable_events_hh",positionAtEcalTop[0], positionAtEcalTop[1], weight);
			histos->Fill2DHisto("xy_positionAtEcal_tracks_analyzable_events_hh",positionAtEcalBot[0], positionAtEcalBot[1], weight);

			histos->Fill2DHisto("energy_vs_theta_analyzable_events_hh",theta_top, energy_top, weight);
			histos->Fill2DHisto("energy_vs_theta_analyzable_events_hh",theta_bot, energy_bot, weight);

			histos->Fill2DHisto("thetaTop_vs_thetaBot_analyzable_events_hh",theta_bot, theta_top, weight);

			//double invariant_mass = (*lorentzVectorTop + *lorentzVectorBot).M();

		}
	}

	std::vector<CalCluster> clulsters_top;
	std::vector<CalCluster> clulsters_bot;

	for(int i = 0; i < n_cl; i++){
		CalCluster* cluster = ecalClusters_->at(i);
		std::vector<double> positionCluster = cluster->getPosition();
		histos->Fill2DHisto("xy_clusters_without_cut_hh",positionCluster[0], positionCluster[1], weight);

		CalHit* seed = (CalHit*)cluster->getSeed();
		histos->Fill1DHisto("seed_energy_cluster_without_cut_h", seed->getEnergy(), weight);
		histos->Fill1DHisto("energy_cluster_without_cut_h", cluster->getEnergy(), weight);
		histos->Fill1DHisto("n_hits_cluster_without_cut_h", cluster->getNHits(), weight);
		histos->Fill2DHisto("energy_vs_n_hits_cluster_without_cut_hh", cluster->getNHits(), cluster->getEnergy(), weight);

		int ix = seed -> getCrystalIndices()[0];
		int iy = seed -> getCrystalIndices()[1];

		if(ix < 0) {
			histos->Fill1DHisto("n_clusters_xAxis_without_cut_h", ix + 1, weight);
			histos->Fill2DHisto("xy_indices_clusters_without_cut_hh",ix + 1, iy, weight);
		}
		else {
			histos->Fill1DHisto("n_clusters_xAxis_without_cut_h", ix, weight);
			histos->Fill2DHisto("xy_indices_clusters_without_cut_hh",ix, iy, weight);
		}

		if(iy > 0 ){
			clulsters_top.push_back(*cluster);
		}
		else if(iy < 0) {
			clulsters_bot.push_back(*cluster);
		}
	}

	int n_clusters_top = clulsters_top.size();
	int n_clusters_bot = clulsters_bot.size();

	if( ( tracks_top.size() >= 1 && tracks_bot.size() >= 1 )){
		for(int i = 0; i < n_clusters_top; i++){
			CalCluster cluster = clulsters_top.at(i);
			std::vector<double> positionCluster = cluster.getPosition();

			for(int j = 0; j < tracks_top.size(); j++){
				Track track = tracks_top.at(j);
				std::vector<double> positionAtEcal = track.getPositionAtEcal();
				histos->Fill2DHisto("trackX_vs_ClusterX_top_hh", positionCluster[0], positionAtEcal[0], weight);
				histos->Fill2DHisto("trackY_vs_ClusterY_top_hh", positionCluster[1], positionAtEcal[1], weight);

				/*
				if (positionAtEcal[0]< func_top_topCutX->Eval(positionCluster[0])
						&& positionAtEcal[0] > func_top_botCutX->Eval(positionCluster[0])
						&& positionAtEcal[1] < func_top_topCutY->Eval(positionCluster[1])
						&& positionAtEcal[1] > func_top_botCutY->Eval(positionCluster[1])) {
					clulsters_top_cut.push_back(cluster);
					break;
				}
				*/
			}
		}

		for(int i = 0; i < n_clusters_bot; i++){
			CalCluster cluster = clulsters_bot.at(i);
			std::vector<double> positionCluster = cluster.getPosition();

			for(int j = 0; j < tracks_bot.size(); j++){
				Track track = tracks_bot.at(j);
				std::vector<double> positionAtEcal = track.getPositionAtEcal();
				histos->Fill2DHisto("trackX_vs_ClusterX_bot_hh", positionCluster[0], positionAtEcal[0], weight);
				histos->Fill2DHisto("trackY_vs_ClusterY_bot_hh", positionCluster[1], positionAtEcal[1], weight);

				/*
				if (positionAtEcal[0]< func_bot_topCutX->Eval(positionCluster[0])
						&& positionAtEcal[0] > func_bot_botCutX->Eval(positionCluster[0])
						&& positionAtEcal[1] < func_bot_topCutY->Eval(positionCluster[1])
						&& positionAtEcal[1] > func_bot_botCutY->Eval(positionCluster[1])) {
					clulsters_bot_cut.push_back(cluster);
					break;
				}
				*/
			}
		}
	}

	if( ( tracks_top.size() >= 1 && tracks_bot.size() >= 1 ) && (n_clusters_top >=1 || n_clusters_bot >= 1)){
		for(int i = 0; i < n_clusters_top; i++){
			CalCluster cluster = clulsters_top.at(i);

			std::vector<double> positionCluster = cluster.getPosition();
			histos->Fill2DHisto("xy_clusters_without_cut_hh",positionCluster[0], positionCluster[1], weight);

			CalHit* seed = (CalHit*)cluster.getSeed();
			histos->Fill1DHisto("seed_energy_cluster_analyzable_events_h", seed->getEnergy(), weight);
			histos->Fill1DHisto("energy_cluster_analyzable_events_h", cluster.getEnergy(), weight);
			histos->Fill1DHisto("n_hits_cluster_analyzable_events_h", cluster.getNHits(), weight);
			histos->Fill2DHisto("energy_vs_n_hits_cluster_analyzable_events_hh", cluster.getNHits(), cluster.getEnergy(), weight);

			int ix = seed -> getCrystalIndices()[0];
			int iy = seed -> getCrystalIndices()[1];

			if(ix < 0) {
				histos->Fill1DHisto("n_clusters_xAxis_analyzable_events_h", ix + 1, weight);
				histos->Fill2DHisto("xy_indices_clusters_analyzable_events_hh",ix + 1, iy, weight);
			}
			else {
				histos->Fill1DHisto("n_clusters_xAxis_analyzable_events_h", ix, weight);
				histos->Fill2DHisto("xy_indices_clusters_analyzable_events_hh",ix, iy, weight);
			}

		}

		for(int i = 0; i < n_clusters_bot; i++){
			CalCluster cluster = clulsters_bot.at(i);

			std::vector<double> positionCluster = cluster.getPosition();
			histos->Fill2DHisto("xy_clusters_without_cut_hh",positionCluster[0], positionCluster[1], weight);

			CalHit* seed = (CalHit*)cluster.getSeed();
			histos->Fill1DHisto("seed_energy_cluster_analyzable_events_h", seed->getEnergy(), weight);
			histos->Fill1DHisto("energy_cluster_analyzable_events_h", cluster.getEnergy(), weight);
			histos->Fill1DHisto("n_hits_cluster_analyzable_events_h", cluster.getNHits(), weight);
			histos->Fill2DHisto("energy_vs_n_hits_cluster_analyzable_events_hh", cluster.getNHits(), cluster.getEnergy(), weight);

			int ix = seed -> getCrystalIndices()[0];
			int iy = seed -> getCrystalIndices()[1];

			if(ix < 0) {
				histos->Fill1DHisto("n_clusters_xAxis_analyzable_events_h", ix + 1, weight);
				histos->Fill2DHisto("xy_indices_clusters_analyzable_events_hh",ix + 1, iy, weight);
			}
			else {
				histos->Fill1DHisto("n_clusters_xAxis_analyzable_events_h", ix, weight);
				histos->Fill2DHisto("xy_indices_clusters_analyzable_events_hh",ix, iy, weight);
			}
		}

	    for(int i = 0; i < n_vtxs; i++){
	        Vertex* vtx = vtxs_->at(i);
	        histos->Fill1DHisto("invariant_mass_vertex_analyzable_events_h", vtx->getInvMass(), weight);
	    }

	}




    return true;
}

void TriggerParametersExtractionMollerAnaProcessor::finalize() {
    histos->saveHistos(outF_, anaName_.c_str());
    delete histos;
    histos = nullptr;

}

std::vector<double> TriggerParametersExtractionMollerAnaProcessor::getCrystalPosition(CalCluster cluster){
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

double TriggerParametersExtractionMollerAnaProcessor::getValueCoplanarity(CalCluster clusterTop, CalCluster clusterBot) {
    // Get the variables used by the calculation.
	std::vector<double> positionTop = getCrystalPosition(clusterTop);
	std::vector<double> positionBot = getCrystalPosition(clusterBot);

	int angleTop = (int) std::round(atan(positionTop[0] / positionTop[1]) * 180.0 / PI);
	int angleBot = (int) std::round(atan(positionBot[0] / positionBot[1]) * 180.0 / PI);

	 // Calculate the coplanarity cut value.
	return abs(angleTop - angleBot);
}

std::vector<double> TriggerParametersExtractionMollerAnaProcessor::getVariablesForEnergySlopeCut(CalCluster clusterTop, CalCluster clusterBot){
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

DECLARE_PROCESSOR(TriggerParametersExtractionMollerAnaProcessor);
