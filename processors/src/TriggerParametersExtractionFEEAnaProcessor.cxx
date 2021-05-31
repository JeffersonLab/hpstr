/**
 *@file TriggerParametersExtractionFEEAnaProcessor.cxx
 *@author Tongtong, UNH
 */

#include "TriggerParametersExtractionFEEAnaProcessor.h"

#include <iostream>

#include "TF1.h"
#include "math.h"

#define ELECTRONMASS 0.000510998950 // GeV
#define PI 3.14159265358979
#define CHI2NDFTHRESHOLD 20
#define TRACKPMIN 3.14 // 3 sigma
#define TRACKPMAX 4.23 // 3 sigma
#define CLUSTERENERGYTHRESHOLD 0.1 // threshold of cluster energy for analyzable events
#define CLUSTERENERGYMINNOCUT 2.46 // for no cut; minimum of cluster energy; 3 sigma for double gaussians
#define CLUSTERENERGYMAXNOCUT 3.60 // for no cut; maximum of cluster energy; 5 sigma for double gaussians
#define CLUSTERENERGYMINANALYZABLE 2.52 // for analyzable events, minimum of cluster energy; 3 sigma for double gaussians
#define CLUSTERENERGYMAXANALYZABLE 3.58 // for analyzable events, maximum of cluster energy; 5 sigma for double gaussians
#define CLUSTERNHTSMINNOCUT 3 // for no cut, minimum for number of cluster's hits
#define CLUSTERNHTSMINANALYZABLE  3 // for analyzable events,  minimum for number of cluster's hits


TriggerParametersExtractionFEEAnaProcessor::TriggerParametersExtractionFEEAnaProcessor(const std::string& name, Process& process) : Processor(name,process) {

}

TriggerParametersExtractionFEEAnaProcessor::~TriggerParametersExtractionFEEAnaProcessor(){}

void TriggerParametersExtractionFEEAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring EcalTimingAnaProcessor" <<std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        anaName_         = parameters.getString("anaName");
        histCfgFilename_      = parameters.getString("histCfg",histCfgFilename_);
        trkColl_    = parameters.getString("trkColl");
        ecalClusColl_    = parameters.getString("ecalClusColl");
        mcColl_  = parameters.getString("mcColl",mcColl_);
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }
}

void TriggerParametersExtractionFEEAnaProcessor::initialize(TTree* tree) {
    tree_= tree;
    // init histos
    histos = new TriggerParametersExtractionFEEAnaHistos(anaName_.c_str());
    histos->loadHistoConfig(histCfgFilename_);
    histos->DefineHistos();

    // init TTree
    tree_->SetBranchAddress(trkColl_.c_str() , &trks_, &btrks_);
    tree_->SetBranchAddress(ecalClusColl_.c_str() , &ecalClusters_, &becalClusters_);
    tree_->SetBranchAddress(mcColl_.c_str() , &mcParts_, &bmcParts_);

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

bool TriggerParametersExtractionFEEAnaProcessor::process(IEvent* ievent) {
    double weight = 1.;

    // Tracks
	int n_tracks = trks_->size();
	histos->Fill1DHisto("n_tracks_h", n_tracks, weight);

	std::vector<Track> tracks;
	for(int i = 0; i < n_tracks; i++){
		Track* track = trks_->at(i);

		int charge = track->getCharge();
		histos->Fill1DHisto("charge_tracks_h", charge, weight);


		double p = track->getP();
		if(charge == 1) histos->Fill1DHisto("p_pos_tracks_h", p, weight);
		else if(charge == -1) histos->Fill1DHisto("p_neg_tracks_h", p, weight);

		double chi2PerNDF = track->getChi2Ndf();
		histos->Fill1DHisto("chi2ndf_tracks_h", chi2PerNDF, weight);

		std::vector<double> positionAtEcal = track->getPositionAtEcal();

		if(!isnan(positionAtEcal[2]) && charge == -1 && chi2PerNDF < CHI2NDFTHRESHOLD && p > TRACKPMIN && p < TRACKPMAX) tracks.push_back(*track);
	}

	// Clusters
	int n_cl = ecalClusters_->size();
	histos->Fill1DHisto("n_clusters_h", n_cl, weight);

	std::vector<CalCluster> clulsters_cut;

	for(int i = 0; i < n_cl; i++){
		CalCluster* cluster = ecalClusters_->at(i);
		std::vector<double> positionCluster = cluster->getPosition();
		histos->Fill2DHisto("xy_clusters_without_cut_hh",positionCluster[0], positionCluster[1], weight);

		CalHit* seed = (CalHit*)cluster->getSeed();

		double energy = cluster->getEnergy();
		double nHits = cluster->getNHits();

		histos->Fill1DHisto("seed_energy_cluster_without_cut_h", seed->getEnergy(), weight);
		histos->Fill1DHisto("energy_cluster_without_cut_h", energy, weight);
		histos->Fill1DHisto("n_hits_cluster_without_cut_h", nHits, weight);
		histos->Fill2DHisto("energy_vs_n_hits_cluster_without_cut_hh", nHits, energy, weight);

		int ix = seed -> getCrystalIndices()[0];
		int iy = seed -> getCrystalIndices()[1];

		if(ix < 0) ix++;

		histos->Fill2DHisto("xy_indices_clusters_without_cut_hh", ix, iy, weight);

		if(energy >= CLUSTERENERGYMINNOCUT && energy <= CLUSTERENERGYMAXNOCUT && nHits >= CLUSTERNHTSMINNOCUT){
			histos->Fill2DHisto("xy_indices_clusters_without_track_association_hh", ix, iy, weight);
		}


		for(int j = 0; j < tracks.size(); j++){
			Track track = tracks.at(j);
			std::vector<double> positionAtEcal = track.getPositionAtEcal();

			if(iy > 0 && positionAtEcal[1] > 0){
				histos->Fill2DHisto("trackX_vs_ClusterX_top_hh", positionCluster[0], positionAtEcal[0], weight);
				histos->Fill2DHisto("trackY_vs_ClusterY_top_hh", positionCluster[1], positionAtEcal[1], weight);

				if (positionAtEcal[0]< func_top_topCutX->Eval(positionCluster[0])
						&& positionAtEcal[0] > func_top_botCutX->Eval(positionCluster[0])
						&& positionAtEcal[1] < func_top_topCutY->Eval(positionCluster[1])
						&& positionAtEcal[1] > func_top_botCutY->Eval(positionCluster[1])) {
					clulsters_cut.push_back(*cluster);
					break;
				}
			}

			else if(iy < 0 && positionAtEcal[1] < 0){
				histos->Fill2DHisto("trackX_vs_ClusterX_bot_hh", positionCluster[0], positionAtEcal[0], weight);
				histos->Fill2DHisto("trackY_vs_ClusterY_bot_hh", positionCluster[1], positionAtEcal[1], weight);

				if (positionAtEcal[0]< func_bot_topCutX->Eval(positionCluster[0])
						&& positionAtEcal[0] > func_bot_botCutX->Eval(positionCluster[0])
						&& positionAtEcal[1] < func_bot_topCutY->Eval(positionCluster[1])
						&& positionAtEcal[1] > func_bot_botCutY->Eval(positionCluster[1])) {
					clulsters_cut.push_back(*cluster);
					break;
				}
			}

		}
	}

	for(int i = 0; i < clulsters_cut.size(); i++){
		CalCluster cluster = clulsters_cut.at(i);
		std::vector<double> positionCluster = cluster.getPosition();

		histos->Fill2DHisto("xy_clusters_analyzable_hh",positionCluster[0], positionCluster[1], weight);

		CalHit* seed = (CalHit*)cluster.getSeed();

		double energy = cluster.getEnergy();
		double nHits = cluster.getNHits();

		histos->Fill1DHisto("seed_energy_cluster_analyzable_h", seed->getEnergy(), weight);
		histos->Fill1DHisto("energy_cluster_analyzable_h", energy, weight);
		histos->Fill1DHisto("n_hits_cluster_analyzable_h", nHits, weight);
		histos->Fill2DHisto("energy_vs_n_hits_cluster_analyzable_hh", nHits, energy, weight);

		int ix = seed -> getCrystalIndices()[0];
		int iy = seed -> getCrystalIndices()[1];

		if(ix < 0) ix++;

		histos->Fill2DHisto("xy_indices_clusters_analyzable_hh", ix, iy, weight);

		if(energy >= CLUSTERENERGYMINANALYZABLE && energy <= CLUSTERENERGYMAXANALYZABLE && nHits >= CLUSTERNHTSMINANALYZABLE){
			histos->Fill2DHisto("xy_indices_clusters_with_track_association_hh", ix, iy, weight);
		}
	}

    return true;
}

void TriggerParametersExtractionFEEAnaProcessor::finalize() {
    histos->saveHistos(outF_, anaName_.c_str());
    delete histos;
    histos = nullptr;

}

DECLARE_PROCESSOR(TriggerParametersExtractionFEEAnaProcessor);
