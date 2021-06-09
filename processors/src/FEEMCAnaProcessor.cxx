/**
 *@file FEEMCAnaProcessor.cxx
 *@author Tongtong, UNH
 */

#include "CalHit.h"
#include "FEEMCAnaProcessor.h"

#include <iostream>

#define CHI2NDFTHRESHOLD 10
#define XMIN -80
#define XMAX 40
#define TRACKPMIN 4.03 // 3 sigma
#define TRACKPMAX 5.04 // 3 sigma

#define DIFFIX 0 // Limit for ix difference between Ecal and VTP clusters
#define DIFFIY 0 // Limit for iy difference between Ecal and VTP clusters

FEEMCAnaProcessor::FEEMCAnaProcessor(const std::string& name, Process& process) : Processor(name,process) {

}

//TODO Check this destructor

FEEMCAnaProcessor::~FEEMCAnaProcessor(){}

void FEEMCAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring EcalTimingAnaProcessor" <<std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        anaName_         = parameters.getString("anaName");
        histCfgFilename_      = parameters.getString("histCfg",histCfgFilename_);
        ecalClusColl_    = parameters.getString("ecalClusColl");
        gtpClusColl_    = parameters.getString("gtpClusColl");
        beamE_  = parameters.getDouble("beamE",beamE_);
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

}

void FEEMCAnaProcessor::initialize(TTree* tree) {
    tree_= tree;
    // init histos
    histos = new FEEMCAnaHistos(anaName_.c_str());
    histos->loadHistoConfig(histCfgFilename_);
    histos->DefineHistos();

    // init TTree
    tree_->SetBranchAddress(gtpClusColl_.c_str() , &gtpClusters_, &bgtpClusters_);
    tree_->SetBranchAddress(trkColl_.c_str() , &trks_, &btrks_);
    tree_->SetBranchAddress(ecalClusColl_.c_str() , &ecalClusters_, &becalClusters_);

    //Cut functions for X
    func_top_topCutX = new TF1("func_top_topCutX", "pol1", -100, 100);
    func_top_topCutX->SetParameters(top_topCutX);
    func_top_botCutX = new TF1("func_top_botCutX", "pol1", -100, 100);
    func_top_botCutX->SetParameters(top_botCutX);

    func_bot_topCutX = new TF1("func_bot_topCutX", "pol1", -100, 100);
    func_bot_topCutX->SetParameters(bot_topCutX);
    func_bot_botCutX = new TF1("func_bot_botCutX", "pol1", -100, 100);
    func_bot_botCutX->SetParameters(bot_botCutX);

    //Cut functions for Y
    func_top_topCutY = new TF1("func_top_topCutY", "pol1", 0, 100);
    func_top_topCutY->SetParameters(top_topCutY);
    func_top_botCutY = new TF1("func_top_botCutY", "pol1", 0, 100);
    func_top_botCutY->SetParameters(top_botCutY);

    func_bot_topCutY = new TF1("func_bot_topCutY", "pol1", -100, 0);
    func_bot_topCutY->SetParameters(bot_topCutY);
    func_bot_botCutY = new TF1("func_bot_botCutY", "pol1", -100, 0);
    func_bot_botCutY->SetParameters(bot_botCutY);

}

bool FEEMCAnaProcessor::process(IEvent* ievent) {
    double weight = 1.;

	histos->FillEcalClusters(ecalClusters_);
	histos->FillGTPClusters(gtpClusters_);
	histos->FillTracks(trks_);

	int nPos = 0, nNeg = 0;
	for(int i=0; i < trks_->size(); i++){
		Track* tr = trks_->at(i);
		int charge = tr->getCharge();
		if (charge == 1) {
			nPos++;
		}
		else if (charge == -1) {
			nNeg++;
		}
	}

	//Event selection
	bool flag_event_selction = false;
	if(nPos == 0 && nNeg == 1){
		Track* trk = trks_->at(0);
		double chi2NDF = trk->getChi2Ndf();
		std::vector<double> positionAtEcal = trk->getPositionAtEcal();

		histos->Fill2DHisto("xy_positionAtEcal_tracks_hh", positionAtEcal[0], positionAtEcal[1], weight);

		if(chi2NDF < CHI2NDFTHRESHOLD && positionAtEcal[0] > XMIN && positionAtEcal[0] < XMAX) {

			std::vector<double> mom = trk->getMomentum();
			double p = sqrt(pow(mom[0], 2) + pow(mom[1], 2) + pow(mom[2], 2));
			histos->Fill1DHisto("p_negative_tracks_with_event_selction_h", p, weight);

			if(p > TRACKPMIN && p < TRACKPMAX) flag_event_selction = true;
		}
	}

	std::vector<CalCluster*> ecalClulsters_cut;

	if(flag_event_selction == true){

		int nClusters = ecalClusters_->size();
		for (int i=0; i < nClusters; i++){
			CalCluster *cluster = ecalClusters_->at(i);
			std::vector<double> positionCluster = cluster->getPosition();
			histos->Fill1DHisto("ecalClusterEnergy_with_event_selction_h", cluster->getEnergy(), weight);

			CalHit* seed = (CalHit*)cluster->getSeed();

			int ix = seed -> getCrystalIndices()[0];
			int iy = seed -> getCrystalIndices()[1];

			if(ix < 0) ix++;

			histos->Fill2DHisto("xy_indices_clusters_with_event_selction_hh", ix, iy, weight);

			for(int j = 0; j < trks_->size(); j++){
				Track* track = trks_->at(j);
				std::vector<double> positionAtEcal = track->getPositionAtEcal();

				if(iy > 0 && positionAtEcal[1] > 0){
					histos->Fill2DHisto("trackX_vs_ClusterX_top_with_event_selction_hh", positionCluster[0], positionAtEcal[0], weight);
					histos->Fill2DHisto("trackY_vs_ClusterY_top_with_event_selction_hh", positionCluster[1], positionAtEcal[1], weight);

					if (positionAtEcal[0]< func_top_topCutX->Eval(positionCluster[0])
							&& positionAtEcal[0] > func_top_botCutX->Eval(positionCluster[0])
							&& positionAtEcal[1] < func_top_topCutY->Eval(positionCluster[1])
							&& positionAtEcal[1] > func_top_botCutY->Eval(positionCluster[1])) {
						ecalClulsters_cut.push_back(cluster);
						break;
					}
				}

				else if(iy < 0 && positionAtEcal[1] < 0){
					histos->Fill2DHisto("trackX_vs_ClusterX_bot_with_event_selction_hh", positionCluster[0], positionAtEcal[0], weight);
					histos->Fill2DHisto("trackY_vs_ClusterY_bot_with_event_selction_hh", positionCluster[1], positionAtEcal[1], weight);

					if (positionAtEcal[0]< func_bot_topCutX->Eval(positionCluster[0])
							&& positionAtEcal[0] > func_bot_botCutX->Eval(positionCluster[0])
							&& positionAtEcal[1] < func_bot_topCutY->Eval(positionCluster[1])
							&& positionAtEcal[1] > func_bot_botCutY->Eval(positionCluster[1])) {
						ecalClulsters_cut.push_back(cluster);
						break;
					}
				}
			}
		}

	}

	std::vector<CalCluster*> vtpClulsters_cut;

	for(int i = 0; i < ecalClulsters_cut.size(); i++){
		CalCluster *ecalCluster = ecalClulsters_cut.at(i);

		double energyEcalCluster = ecalCluster->getEnergy();
        double timeEcalCluster = ecalCluster->getTime();

		histos->Fill1DHisto("ecalClusterEnergy_with_event_selction_and_track_cluster_matching_h", ecalCluster->getEnergy(), weight);

		CalHit* seedEcal = (CalHit*)ecalCluster->getSeed();

		int ixEcal = seedEcal -> getCrystalIndices()[0];
		if(ixEcal < 0) ixEcal++;
		int iyEcal = seedEcal -> getCrystalIndices()[1];

		for (int j = 0; j < gtpClusters_->size(); j++){
			CalCluster* vtpCluster = gtpClusters_->at(j);
			double energyVTPCluster = vtpCluster->getEnergy();
            double timeVTPCluster = vtpCluster->getTime();

			CalHit* seed = (CalHit*)vtpCluster->getSeed();
			int ixVTP = seed -> getCrystalIndices()[0];
			int iyVTP = seed -> getCrystalIndices()[1];
			if(ixVTP < 0) ixVTP++;

            double timeDiff = timeEcalCluster - timeVTPCluster;

			int ixDiff = ixEcal - ixVTP;
			int iyDiff = iyEcal - iyVTP;

			histos->Fill1DHisto("diff_ix_between_EcalCluster_VTPCluster_with_event_selction_and_track_cluster_matching_h", ixDiff, weight);
			histos->Fill1DHisto("diff_iy_between_EcalCluster_VTPCluster_with_event_selction_and_track_cluster_matching_h", iyDiff, weight);

			histos->Fill2DHisto("diff_ix_vs_diff_iy_between_EcalCluster_VTPCluster_with_event_selction_and_track_cluster_matching_hh", ixDiff, iyDiff, weight);


			if(ixDiff == DIFFIX && iyDiff == DIFFIX){
				vtpClulsters_cut.push_back(vtpCluster);
			}
		}
	}

	for(int i = 0; i < vtpClulsters_cut.size(); i++){
		CalCluster* vtpCluster = vtpClulsters_cut.at(i);
		histos->Fill1DHisto("VTPClusterEnergy_with_Ecal_VTP_cluster_matching_h", vtpCluster->getEnergy(), weight);
	}

    return true;
}

void FEEMCAnaProcessor::finalize() {


    histos->saveHistos(outF_, anaName_.c_str());
    delete histos;
    histos = nullptr;

}

DECLARE_PROCESSOR(FEEMCAnaProcessor);
