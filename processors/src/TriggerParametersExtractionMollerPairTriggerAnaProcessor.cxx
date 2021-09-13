/**
 *@file TriggerParametersExtractionMoller1pt92AnaProcessor.cxx
 *@author Tongtong, UNH
 */

#include "../include/TriggerParametersExtractionMollerPairTriggerAnaProcessor.h"

#include <iostream>

#include "TF1.h"
#include "math.h"

#define ELECTRONMASS 0.000510998950 // GeV
#define PI 3.14159265358979
#define ROTATIONANGLEAROUNDY 0.0305 // rad
#define CHI2NDFTHRESHOLD 20
#define CLUSTERENERGYTHRESHOLD 0.05 // threshold of cluster energy for analyzable events
#define CLUSTERENERGYMIN 0.20 // minimum of cluster energy; 3sigma
#define CLUSTERENERGYMAX 0.86 // maximum of cluster energy; 3sigma
#define CLUSTERXMIN -13 // minimum of x index
#define CLUSTERXMAX -7 // maximum of x index
#define CLUSTERYMIN -3 // minimum of y index
#define CLUSTERYMAX 3 // maximum of y index
#define DIFFENERGYMIN -0.18 // minimum for difference between measured and calculated energy
#define DIFFENERGYMAX 0.17 // maximum for difference between measured and calculated energy
#define DIFFTHETAMIN -0.0046 // minimum for difference between measured and calculated theta before rotation
#define DIFFTHETAMAX 0.0054 // maximum for difference between measured and calculated theta before rotation

#define ENERGYSUMMIN 0.36
#define ENERGYSUMMAX 1.48
#define ENERGYDIFF 0.58
#define COPLANARITY 15

TriggerParametersExtractionMollerPairTriggerAnaProcessor::TriggerParametersExtractionMollerPairTriggerAnaProcessor(const std::string& name, Process& process) : Processor(name,process) {

}

TriggerParametersExtractionMollerPairTriggerAnaProcessor::~TriggerParametersExtractionMollerPairTriggerAnaProcessor(){}

void TriggerParametersExtractionMollerPairTriggerAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring TriggerParametersExtractionMollerPairTriggerAnaProcessor" <<std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        anaName_         = parameters.getString("anaName");
        histCfgFilename_      = parameters.getString("histCfg",histCfgFilename_);
        trkColl_    = parameters.getString("trkColl");
        gtpClusColl_    = parameters.getString("gtpClusColl");
        mcColl_  = parameters.getString("mcColl",mcColl_);
        vtxColl_ = parameters.getString("vtxColl",vtxColl_);
        beamE_  = parameters.getDouble("beamE",beamE_);
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }
}

void TriggerParametersExtractionMollerPairTriggerAnaProcessor::initialize(TTree* tree) {
	_ah =  std::make_shared<AnaHelpers>();

    tree_= tree;
    // init histos
    histos = new TriggerParametersExtractionMollerAnaHistos(anaName_.c_str());
    histos->loadHistoConfig(histCfgFilename_);
    histos->DefineHistos();

    // init TTree
    tree_->SetBranchAddress(trkColl_.c_str() , &trks_, &btrks_);
    tree_->SetBranchAddress(gtpClusColl_.c_str() , &gtpClusters_, &bgtpClusters_);
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

    //Upper limit for position dependent energy
    func_pde_moller = new TF1("func_pde_moller", "pol2", -22, 0);
    func_pde_moller->SetParameters(pars_pde_moller);

    // Kinematic equations
    // E vs theta
    func_E_vs_theta_before_roation = new TF1("func_E_vs_theta_before_roation", "[0]/(1 + 2*[0]/[1]*sin(x/2.)*sin(x/2.))", 0, 1);
    func_E_vs_theta_before_roation->SetParameter(0, beamE_);
    func_E_vs_theta_before_roation->SetParameter(1, ELECTRONMASS);

    // theta1 vs theta2
    func_theta1_vs_theta2_before_roation = new TF1("func_theta1_vs_theta2_before_roation", "2*asin([1]/2./[0] * 1/sin(x/2.))", 0, 1);
    func_theta1_vs_theta2_before_roation->SetParameter(0, beamE_);
    func_theta1_vs_theta2_before_roation->SetParameter(1, ELECTRONMASS);

    // save a tree for information of GTP cluster pairs
    _reg_gtp_cluster_pairs = std::make_shared<FlatTupleMaker>(anaName_ + "_gtp_cluster_pairs");
    _reg_gtp_cluster_pairs->addVariable("energyTop");
    _reg_gtp_cluster_pairs->addVariable("ixTop");
    _reg_gtp_cluster_pairs->addVariable("iyTop");
    _reg_gtp_cluster_pairs->addVariable("nHitsTop");
    _reg_gtp_cluster_pairs->addVariable("angleTop");
    _reg_gtp_cluster_pairs->addVariable("rTop");

    _reg_gtp_cluster_pairs->addVariable("energyBot");
    _reg_gtp_cluster_pairs->addVariable("ixBot");
    _reg_gtp_cluster_pairs->addVariable("iyBot");
    _reg_gtp_cluster_pairs->addVariable("nHitsBot");
    _reg_gtp_cluster_pairs->addVariable("angleBot");
    _reg_gtp_cluster_pairs->addVariable("rBot");

    _reg_gtp_cluster_pairs->addVariable("nVertices");

    _reg_gtp_cluster_pairs->addVector("momTrackTop");
    _reg_gtp_cluster_pairs->addVector("momTrackBot");


    // save a tree for information of tracks from vertices
    _reg_tracks_from_vertices = std::make_shared<FlatTupleMaker>(anaName_ + "_tracks_from_vertices");
    _reg_tracks_from_vertices->addVariable("momIDTop");
    _reg_tracks_from_vertices->addVariable("momIDBot");
    _reg_tracks_from_vertices->addVariable("momPDGTop");
    _reg_tracks_from_vertices->addVariable("momPDGBot");
    _reg_tracks_from_vertices->addVector("momTop");
    _reg_tracks_from_vertices->addVector("momBot");
    _reg_tracks_from_vertices->addVector("momMCPTop");
    _reg_tracks_from_vertices->addVector("momMCPBot");

    _reg_tracks_from_vertices->addVariable("timeTop");
    _reg_tracks_from_vertices->addVariable("timeBot");

    _reg_tracks_from_vertices->addVector("momVertex");
    _reg_tracks_from_vertices->addVariable("imVertex");

    _reg_tracks_from_vertices->addVariable("nClustersAssociatedTracksTop");
    _reg_tracks_from_vertices->addVariable("nClustersAssociatedTracksBot");

    _reg_tracks_from_vertices->addVariable("single_triggered_analyzable_flag");
    _reg_tracks_from_vertices->addVariable("triggered_analyzable_flag");
    _reg_tracks_from_vertices->addVariable("triggered_analyzable_and_kinematic_cuts_flag");
}

bool TriggerParametersExtractionMollerPairTriggerAnaProcessor::process(IEvent* ievent) {
    double weight = 1.;

    ////////////////////////////////// To extract analyzable events //////////////////////////////////
	int n_tracks = trks_->size();
	histos->Fill1DHisto("n_tracks_h", n_tracks, weight);

	int n_cl = gtpClusters_->size();
	histos->Fill1DHisto("n_clusters_h", n_cl, weight);

	int n_vtxs = vtxs_->size();
	histos->Fill1DHisto("n_vtxs_h", n_vtxs, weight);

	histos->Fill2DHisto("n_clusters_vs_n_tracks_hh", n_tracks, n_cl, weight);
	histos->Fill2DHisto("n_clusters_vs_n_vtxs_hh", n_vtxs, n_cl, weight);
	histos->Fill2DHisto("n_tracks_vs_n_vtxs_hh", n_vtxs, n_tracks, weight);

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

		for(int j = 0; j < n_tracks_bot; j++) {
			Track trackBot = tracks_bot.at(j);
			std::vector<double> positionAtEcalBot = trackBot.getPositionAtEcal();

			histos->Fill2DHisto("xy_positionAtEcal_track_pair_hh",positionAtEcalTop[0], positionAtEcalTop[1], weight);
			histos->Fill2DHisto("xy_positionAtEcal_track_pair_hh",positionAtEcalBot[0], positionAtEcalBot[1], weight);
		}
	}

	std::vector<CalCluster> clulsters_top;
	std::vector<CalCluster> clulsters_bot;

	for(int i = 0; i < n_cl; i++){
		CalCluster* cluster = gtpClusters_->at(i);
		std::vector<double> positionCluster = cluster->getPosition();
		histos->Fill2DHisto("xy_clusters_without_cut_hh",positionCluster[0], positionCluster[1], weight);

		CalHit* seed = (CalHit*)cluster->getSeed();
		histos->Fill1DHisto("seed_energy_cluster_without_cut_h", seed->getEnergy(), weight);
		histos->Fill1DHisto("energy_cluster_without_cut_h", cluster->getEnergy(), weight);
		histos->Fill1DHisto("n_hits_cluster_without_cut_h", cluster->getNHits(), weight);
		histos->Fill2DHisto("energy_vs_n_hits_cluster_without_cut_hh", cluster->getNHits(), cluster->getEnergy(), weight);

		int ix = seed -> getCrystalIndices()[0];
		if(ix < 0) ix++;
		int iy = seed -> getCrystalIndices()[1];

		histos->Fill1DHisto("n_clusters_xAxis_without_cut_h", ix, weight);
		histos->Fill2DHisto("xy_indices_clusters_without_cut_hh",ix, iy, weight);
		histos->Fill2DHisto("energy_vs_ix_clusters_without_cut_hh", ix, cluster->getEnergy(), weight);
		histos->Fill2DHisto("energy_vs_iy_clusters_without_cut_hh", iy, cluster->getEnergy(), weight);

		if(iy > 0 ){
			clulsters_top.push_back(*cluster);
		}
		else if(iy < 0) {
			clulsters_bot.push_back(*cluster);
		}
	}

	int n_clusters_top = clulsters_top.size();
	int n_clusters_bot = clulsters_bot.size();

	std::vector<CalCluster> clulsters_top_cut;
	std::vector<CalCluster> clulsters_bot_cut;

	std::vector<Track> tracks_matched_top_cut;
	std::vector<Track> tracks_matched_bot_cut;

	if( ( tracks_top.size() >= 1 && tracks_bot.size() >= 1 )){
		for(int i = 0; i < n_clusters_top; i++){
			CalCluster cluster = clulsters_top.at(i);
			std::vector<double> positionCluster = cluster.getPosition();

			for(int j = 0; j < tracks_top.size(); j++){
				Track track = tracks_top.at(j);
				std::vector<double> positionAtEcal = track.getPositionAtEcal();
				histos->Fill2DHisto("trackX_vs_ClusterX_top_hh", positionCluster[0], positionAtEcal[0], weight);
				histos->Fill2DHisto("trackY_vs_ClusterY_top_hh", positionCluster[1], positionAtEcal[1], weight);

				if (positionAtEcal[0]< func_top_topCutX->Eval(positionCluster[0])
						&& positionAtEcal[0] > func_top_botCutX->Eval(positionCluster[0])
						&& positionAtEcal[1] < func_top_topCutY->Eval(positionCluster[1])
						&& positionAtEcal[1] > func_top_botCutY->Eval(positionCluster[1])) {
					clulsters_top_cut.push_back(cluster);
					tracks_matched_top_cut.push_back(track);
					break;
				}
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

				if (positionAtEcal[0]< func_bot_topCutX->Eval(positionCluster[0])
						&& positionAtEcal[0] > func_bot_botCutX->Eval(positionCluster[0])
						&& positionAtEcal[1] < func_bot_topCutY->Eval(positionCluster[1])
						&& positionAtEcal[1] > func_bot_botCutY->Eval(positionCluster[1])) {
					clulsters_bot_cut.push_back(cluster);
					tracks_matched_bot_cut.push_back(track);
					break;
				}
			}
		}
	}

	int n_clusters_top_cut = clulsters_top_cut.size();
	int n_clusters_bot_cut = clulsters_bot_cut.size();

	int flag_top = false;
	int flag_bot = false;

	for(int i = 0; i < n_clusters_top_cut; i++){
		CalCluster cluster = clulsters_top_cut.at(i);
		if(cluster.getEnergy() >= CLUSTERENERGYTHRESHOLD) flag_top = true;
	}

	for(int i = 0; i < n_clusters_bot_cut; i++){
		CalCluster cluster = clulsters_bot_cut.at(i);
		if(cluster.getEnergy() >= CLUSTERENERGYTHRESHOLD) flag_bot = true;
	}

	int flag_analyzable_event = false;
	if( ( tracks_top.size() >= 1 && tracks_bot.size() >= 1 ) && flag_top == true && flag_bot == true )
		flag_analyzable_event = true;

	int flag_single_triggered_analyzable_event_top = false;
	int flag_single_triggered_analyzable_event_bot = false;

	std::vector<CalCluster> clulsters_top_pass_single_trigger;
	std::vector<CalCluster> clulsters_bot_pass_single_trigger;
	// To determine flags of analyzable events and triggered analyzable events
	if(flag_analyzable_event){

		for(int i = 0; i < n_clusters_top_cut; i++){
			CalCluster clusterTop = clulsters_top_cut.at(i);

			std::vector<double> positionCluster = clusterTop.getPosition();
			histos->Fill2DHisto("xy_clusters_analyzable_events_hh",positionCluster[0], positionCluster[1], weight);

			CalHit* seed = (CalHit*)clusterTop.getSeed();
			histos->Fill1DHisto("seed_energy_cluster_analyzable_events_h", seed->getEnergy(), weight);
			histos->Fill1DHisto("energy_cluster_analyzable_events_h", clusterTop.getEnergy(), weight);
			histos->Fill1DHisto("n_hits_cluster_analyzable_events_h", clusterTop.getNHits(), weight);
			histos->Fill2DHisto("energy_vs_n_hits_cluster_analyzable_events_hh", clusterTop.getNHits(), clusterTop.getEnergy(), weight);

			int ix = seed -> getCrystalIndices()[0];
			if(ix < 0) ix++;
			int iy = seed -> getCrystalIndices()[1];

			histos->Fill1DHisto("n_clusters_xAxis_analyzable_events_h", ix, weight);
			histos->Fill2DHisto("xy_indices_clusters_analyzable_events_hh",ix, iy, weight);
			histos->Fill2DHisto("energy_vs_ix_clusters_analyzable_events_hh",
					ix, clusterTop.getEnergy(), weight);
			histos->Fill2DHisto("energy_vs_iy_clusters_analyzable_events_hh",
					iy, clusterTop.getEnergy(), weight);

			if (clusterTop.getEnergy() <= CLUSTERENERGYMAX
					&& clusterTop.getEnergy() >= CLUSTERENERGYMIN
					&& ix >= CLUSTERXMIN && ix <= CLUSTERXMAX
					&& iy >= CLUSTERYMIN && iy <= CLUSTERYMAX
					&& clusterTop.getEnergy() <= func_pde_moller->Eval(ix)){
					flag_single_triggered_analyzable_event_top = true;
					clulsters_top_pass_single_trigger.push_back(clusterTop);
			}
		}

		for(int i = 0; i < n_clusters_bot_cut; i++){
			CalCluster clusterBot = clulsters_bot_cut.at(i);

			std::vector<double> positionCluster = clusterBot.getPosition();
			histos->Fill2DHisto("xy_clusters_analyzable_events_hh",positionCluster[0], positionCluster[1], weight);

			CalHit* seed = (CalHit*)clusterBot.getSeed();
			histos->Fill1DHisto("seed_energy_cluster_analyzable_events_h", seed->getEnergy(), weight);
			histos->Fill1DHisto("energy_cluster_analyzable_events_h", clusterBot.getEnergy(), weight);
			histos->Fill1DHisto("n_hits_cluster_analyzable_events_h", clusterBot.getNHits(), weight);
			histos->Fill2DHisto("energy_vs_n_hits_cluster_analyzable_events_hh", clusterBot.getNHits(), clusterBot.getEnergy(), weight);

			int ix = seed -> getCrystalIndices()[0];
			if(ix < 0) ix++;
			int iy = seed -> getCrystalIndices()[1];

			histos->Fill1DHisto("n_clusters_xAxis_analyzable_events_h", ix, weight);
			histos->Fill2DHisto("xy_indices_clusters_analyzable_events_hh",ix, iy, weight);
			histos->Fill2DHisto("energy_vs_ix_clusters_analyzable_events_hh", ix, clusterBot.getEnergy(), weight);
			histos->Fill2DHisto("energy_vs_iy_clusters_analyzable_events_hh", iy, clusterBot.getEnergy(), weight);

			if (clusterBot.getEnergy() <= CLUSTERENERGYMAX
					&& clusterBot.getEnergy() >= CLUSTERENERGYMIN
					&& ix >= CLUSTERXMIN && ix <= CLUSTERXMAX
					&& iy >= CLUSTERYMIN && iy <= CLUSTERYMAX
					&& clusterBot.getEnergy() <= func_pde_moller->Eval(ix)){
					flag_single_triggered_analyzable_event_bot = true;
					clulsters_bot_pass_single_trigger.push_back(clusterBot);
			}
		}

		for(int i = 0; i < n_tracks_top; i++) {
			Track trackTop = tracks_top.at(i);
			std::vector<double> positionAtEcalTop = trackTop.getPositionAtEcal();

			for(int j = 0; j < n_tracks_bot; j++) {
				Track trackBot = tracks_bot.at(j);
				std::vector<double> positionAtEcalBot = trackBot.getPositionAtEcal();

				histos->Fill2DHisto("xy_positionAtEcal_tracks_analyzable_events_hh",positionAtEcalTop[0], positionAtEcalTop[1], weight);
				histos->Fill2DHisto("xy_positionAtEcal_tracks_analyzable_events_hh",positionAtEcalBot[0], positionAtEcalBot[1], weight);

			}
		}
	}

	bool flag_single_triggered_analyzable_event = false;
	if(flag_single_triggered_analyzable_event_top && flag_single_triggered_analyzable_event_bot) flag_single_triggered_analyzable_event = true;

	////////////////////////////////// Save information for cluster pair //////////////////////////////////
	if(flag_analyzable_event){
		for(int i = 0; i < n_clusters_top_cut; i++){
			CalCluster clusterTop = clulsters_top_cut.at(i);
			CalHit* seedTop = (CalHit*)clusterTop.getSeed();
			int ixTop = seedTop -> getCrystalIndices()[0];
			if(ixTop < 0) ixTop++;
			int iyTop = seedTop -> getCrystalIndices()[1];

			Track trackTop = tracks_matched_top_cut.at(i);

			for(int j = 0; j < n_clusters_bot_cut; j++){
				CalCluster clusterBot = clulsters_bot_cut.at(j);
				CalHit* seedBot = (CalHit*)clusterBot.getSeed();
				int ixBot = seedBot -> getCrystalIndices()[0];
				if(ixBot < 0) ixBot++;
				int iyBot = seedBot -> getCrystalIndices()[1];

				Track trackBot = tracks_matched_bot_cut.at(j);

				histos->Fill1DHisto("energy_sum_cluster_h", clusterTop.getEnergy() + clusterBot.getEnergy(), weight);
				histos->Fill1DHisto("energy_difference_cluster_h", fabs(clusterTop.getEnergy() - clusterBot.getEnergy()), weight);
				histos->Fill1DHisto("coplanarity_cluster_h", getValueCoplanarity(clusterTop, clusterBot), weight);

				std::vector<double> variablesForEnergySlopeCut = getVariablesForEnergySlopeCut(clusterTop, clusterBot);
				histos->Fill2DHisto("energy_vs_r_lowerest_energy_cluster_hh", variablesForEnergySlopeCut[0], variablesForEnergySlopeCut[1], weight);

			    _reg_gtp_cluster_pairs->setVariableValue("energyTop", clusterTop.getEnergy());
			    _reg_gtp_cluster_pairs->setVariableValue("ixTop", ixTop);
			    _reg_gtp_cluster_pairs->setVariableValue("iyTop", iyTop);
			    _reg_gtp_cluster_pairs->setVariableValue("nHitsTop", clusterTop.getNHits());
			    _reg_gtp_cluster_pairs->setVariableValue("angleTop", getAngle(clusterTop));
			    _reg_gtp_cluster_pairs->setVariableValue("rTop", getR(clusterTop));

			    _reg_gtp_cluster_pairs->setVariableValue("energyBot", clusterBot.getEnergy());
			    _reg_gtp_cluster_pairs->setVariableValue("ixBot", ixBot);
			    _reg_gtp_cluster_pairs->setVariableValue("iyBot", iyBot);
			    _reg_gtp_cluster_pairs->setVariableValue("nHitsBot", clusterBot.getNHits());
			    _reg_gtp_cluster_pairs->setVariableValue("angleBot", getAngle(clusterBot));
			    _reg_gtp_cluster_pairs->setVariableValue("rBot", getR(clusterBot));

			    _reg_gtp_cluster_pairs->setVariableValue("nVertices", n_vtxs);

			    _reg_gtp_cluster_pairs->addToVector("momTrackTop", trackTop.getMomentum()[0]);
			    _reg_gtp_cluster_pairs->addToVector("momTrackTop", trackTop.getMomentum()[1]);
			    _reg_gtp_cluster_pairs->addToVector("momTrackTop", trackTop.getMomentum()[2]);

			    _reg_gtp_cluster_pairs->addToVector("momTrackBot", trackBot.getMomentum()[0]);
			    _reg_gtp_cluster_pairs->addToVector("momTrackBot", trackBot.getMomentum()[1]);
			    _reg_gtp_cluster_pairs->addToVector("momTrackBot", trackBot.getMomentum()[2]);

			    _reg_gtp_cluster_pairs->fill();
			}
		}
	}

	////////////////////////////////// To determine flag of triggered analyzable events //////////////////////////////////
	bool flag_triggered_analyzable_event = false; // pass both single and pair trigger conditions

	int n_clusters_top_pass_single_trigger = clulsters_top_pass_single_trigger.size();
	int n_clusters_bot_pass_single_trigger = clulsters_bot_pass_single_trigger.size();
	if(flag_single_triggered_analyzable_event){
		for(int i = 0; i < n_clusters_top_pass_single_trigger; i++){
			CalCluster clusterTop = clulsters_top_pass_single_trigger.at(i);
			for(int j = 0; j < n_clusters_bot_pass_single_trigger; j++){
				CalCluster clusterBot = clulsters_bot_pass_single_trigger.at(j);

				double energy_sum = clusterTop.getEnergy() + clusterBot.getEnergy();
				double energy_diff =  fabs(clusterTop.getEnergy() - clusterBot.getEnergy());
				double coplanarity = fabs(getValueCoplanarity(clusterTop, clusterBot));

				histos->Fill1DHisto("energy_sum_clusters_pass_single_trigger_h", energy_sum, weight);
				histos->Fill1DHisto("energy_difference_clusters_pass_single_trigger_h", energy_diff, weight);
				histos->Fill1DHisto("coplanarity_clusters_pass_single_trigger_h", coplanarity, weight);

				if(energy_sum >= ENERGYSUMMIN && energy_sum <= ENERGYSUMMAX && energy_diff <= ENERGYDIFF && coplanarity <= COPLANARITY)
					flag_triggered_analyzable_event = true;

			}
		}
	}

	////////////////////////////////// To determine flag of triggered analyzable events with kinematic cuts //////////////////////////////////
	bool flag_triggered_analyzable_event_and_pass_kinematic_cuts = false;

    for(int i = 0; i < n_vtxs; i++){
        Vertex* vtx = vtxs_->at(i);

        int n_entries = vtx->getParticles()->GetEntries();
        if(n_entries != 2) {
        	std::cout << "Warning: entries of Moller vertex is not 2." << std::endl;
        	return false;
        }

        double invariant_mass = vtx->getInvMass();
		histos->Fill1DHisto("invariant_mass_vertex_h", invariant_mass, weight);


        Particle* particleTop = (Particle*)vtx->getParticles()->At(0);
        Particle* particleBot = (Particle*)vtx->getParticles()->At(1);

        std::vector<double> momTop =  particleTop->getMomentum();
        std::vector<double> momBot =  particleBot->getMomentum();

        double pSum = sqrt(pow(momTop[0], 2) + pow(momTop[1], 2) + pow(momTop[2], 2)) + sqrt(pow(momBot[0], 2) + pow(momBot[1], 2) + pow(momBot[2], 2));

		histos->Fill1DHisto("pSum_vertex_h", pSum, weight);

		histos->Fill2DHisto("invariant_mass_vs_pSum_vertex_hh", pSum, invariant_mass, weight);

		histos->Fill2DHisto("px_vs_py_vertex_hh", momTop[0], momTop[1], weight);
		histos->Fill2DHisto("px_vs_py_vertex_hh", momBot[0], momBot[1], weight);

		double momTopX_before_beam_rotation = momTop[0] * cos(ROTATIONANGLEAROUNDY) - momTop[2] * sin(ROTATIONANGLEAROUNDY);
		double momTopZ_before_beam_rotation = momTop[0] * sin(ROTATIONANGLEAROUNDY) + momTop[2] * cos(ROTATIONANGLEAROUNDY);
		double momTopY_before_beam_rotation = momTop[1];

		double momBotX_before_beam_rotation = momBot[0] * cos(ROTATIONANGLEAROUNDY) - momBot[2] * sin(ROTATIONANGLEAROUNDY);
		double momBotZ_before_beam_rotation = momBot[0] * sin(ROTATIONANGLEAROUNDY) + momBot[2] * cos(ROTATIONANGLEAROUNDY);
		double momBotY_before_beam_rotation = momBot[1];

		TLorentzVector* lorentzVectorTop_beam_rotation = new TLorentzVector();
		lorentzVectorTop_beam_rotation->SetXYZM(momTopX_before_beam_rotation, momTopY_before_beam_rotation, momTopZ_before_beam_rotation, ELECTRONMASS);

		TLorentzVector* lorentzVectorBot_beam_rotation = new TLorentzVector();
		lorentzVectorBot_beam_rotation->SetXYZM(momBotX_before_beam_rotation, momBotY_before_beam_rotation, momBotZ_before_beam_rotation, ELECTRONMASS);

		double energy_top = lorentzVectorTop_beam_rotation->E();
		double energy_bot = lorentzVectorBot_beam_rotation->E();

		double thate_top_before_rotation = lorentzVectorTop_beam_rotation->Theta();
		double thate_bot_before_rotation = lorentzVectorBot_beam_rotation->Theta();

		double energy_calcuated_top = func_E_vs_theta_before_roation->Eval(thate_top_before_rotation);
		double energy_calcuated_bot = func_E_vs_theta_before_roation->Eval(thate_bot_before_rotation);

		double theta_top_calculated_before_rotation = func_theta1_vs_theta2_before_roation->Eval(thate_bot_before_rotation);
		double theta_bot_calculated_before_rotation = func_theta1_vs_theta2_before_roation->Eval(thate_top_before_rotation);

		double energy_diff_top = energy_top - energy_calcuated_top;
		double energy_diff_bot = energy_bot - energy_calcuated_bot;

		double theta_diff_top = thate_top_before_rotation - theta_top_calculated_before_rotation;
		double theta_diff_bot = thate_bot_before_rotation - theta_bot_calculated_before_rotation;

		histos->Fill2DHisto("energy_vs_theta_track_pair_from_vertex_before_rotation_hh",thate_top_before_rotation, energy_top, weight);
		histos->Fill2DHisto("energy_vs_theta_track_pair_from_vertex_before_rotation_hh",thate_bot_before_rotation, energy_bot, weight);

		histos->Fill2DHisto("thetaTop_vs_thetaBot_track_pair_from_vertex_before_rotation_hh",thate_bot_before_rotation, thate_top_before_rotation, weight);

		histos->Fill1DHisto("diff_E_vertex_before_rotation_h", energy_diff_top, weight);
		histos->Fill1DHisto("diff_E_vertex_before_rotation_h", energy_diff_bot, weight);

		histos->Fill1DHisto("diff_theta_vertex_before_rotation_h", theta_diff_top, weight);
		histos->Fill1DHisto("diff_theta_vertex_before_rotation_h", theta_diff_bot, weight);

        Track trackTop = particleTop->getTrack();
        Track trackBot = particleBot->getTrack();

        std::vector<double> positionAtEcalTop = trackTop.getPositionAtEcal();
        std::vector<double> positionAtEcalBot = trackBot.getPositionAtEcal();

        if(!isnan(positionAtEcalTop[2]) && !isnan(positionAtEcalBot[2]) ){
        	histos->Fill2DHisto("xy_positionAtEcal_vertices_hh", positionAtEcalTop[0], positionAtEcalTop[1], weight);
        	histos->Fill2DHisto("xy_positionAtEcal_vertices_hh", positionAtEcalBot[0], positionAtEcalBot[1], weight);
        }

        CalCluster clTop = particleTop->getCluster();
        CalCluster clBot = particleBot->getCluster();

    	histos->Fill1DHisto("diff_energy_between_recon_clulster_and_track_energy_vertex_h", clTop.getEnergy() - energy_top, weight);
    	histos->Fill1DHisto("diff_energy_between_recon_clulster_and_track_energy_vertex_h", clBot.getEnergy() - energy_bot, weight);

        if(flag_single_triggered_analyzable_event){
			histos->Fill1DHisto("invariant_mass_vertex_analyzable_events_h", invariant_mass, weight);

			histos->Fill1DHisto("pSum_vertex_analyzable_events_h", pSum, weight);

			histos->Fill2DHisto("invariant_mass_vs_pSum_vertex_analyzable_events_hh", pSum, invariant_mass, weight);

			histos->Fill2DHisto("px_vs_py_vertex_analyzable_events_hh", momTop[0], momTop[1], weight);
			histos->Fill2DHisto("px_vs_py_vertex_analyzable_events_hh", momBot[0], momBot[1], weight);

    		histos->Fill2DHisto("energy_vs_theta_analyzable_events_before_rotation_hh",thate_top_before_rotation, energy_top, weight);
    		histos->Fill2DHisto("energy_vs_theta_analyzable_events_before_rotation_hh",thate_bot_before_rotation, energy_bot, weight);

    		histos->Fill2DHisto("thetaTop_vs_thetaBot_analyzable_events_before_rotation_hh",thate_bot_before_rotation, thate_top_before_rotation, weight);

    		histos->Fill1DHisto("diff_E_analyzable_events_before_rotation_h", energy_diff_top, weight);
    		histos->Fill1DHisto("diff_E_analyzable_events_before_rotation_h", energy_diff_bot, weight);

    		histos->Fill1DHisto("diff_theta_analyzable_events_before_rotation_h", theta_diff_top, weight);
    		histos->Fill1DHisto("diff_theta_analyzable_events_before_rotation_h", theta_diff_bot, weight);

    		if(energy_diff_top > DIFFENERGYMIN && energy_diff_top < DIFFENERGYMAX && energy_diff_bot > DIFFENERGYMIN && energy_diff_bot < DIFFENERGYMAX){
        		histos->Fill2DHisto("thetaTop_vs_thetaBot_analyzable_events_before_rotation_with_diff_energy_cut_hh",thate_bot_before_rotation, thate_top_before_rotation, weight);
        		histos->Fill1DHisto("diff_theta_analyzable_events_before_rotation_with_diff_energy_cut_h", theta_diff_top, weight);
        		histos->Fill1DHisto("diff_theta_analyzable_events_before_rotation_with_diff_energy_cut_h", theta_diff_bot, weight);
    		}

        	histos->Fill1DHisto("diff_energy_between_recon_clulster_and_track_energy_analyzable_events_h", clTop.getEnergy() - energy_top, weight);
        	histos->Fill1DHisto("diff_energy_between_recon_clulster_and_track_energy_analyzable_events_h", clBot.getEnergy() - energy_bot, weight);

        	if( (energy_diff_top < DIFFENERGYMIN || energy_diff_top > DIFFENERGYMAX )
            		|| (energy_diff_bot < DIFFENERGYMIN || energy_diff_bot > DIFFENERGYMAX)
    				|| (theta_diff_top < DIFFTHETAMIN || theta_diff_top > DIFFTHETAMAX)
					|| (theta_diff_bot < DIFFTHETAMIN || theta_diff_bot > DIFFTHETAMAX))
        		histos->Fill1DHisto("invariant_mass_vertex_analyzable_events_out_of_kinematic_cuts_h", invariant_mass, weight);

				histos->Fill1DHisto("pSum_vertex_analyzable_events_out_of_kinematic_cuts_h", pSum, weight);

				histos->Fill2DHisto("invariant_mass_vs_pSum_vertex_analyzable_events_out_of_kinematic_cuts_hh", pSum, invariant_mass, weight);
        }

        if(flag_triggered_analyzable_event){
			histos->Fill1DHisto("invariant_mass_vertex_triggered_analyzable_events_h", invariant_mass, weight);

			histos->Fill1DHisto("pSum_vertex_triggered_analyzable_events_h", pSum, weight);

			histos->Fill2DHisto("invariant_mass_vs_pSum_vertex_triggered_analyzable_events_hh", pSum, invariant_mass, weight);

			histos->Fill2DHisto("px_vs_py_vertex_triggered_analyzable_events_hh", momTop[0], momTop[1], weight);
			histos->Fill2DHisto("px_vs_py_vertex_triggered_analyzable_events_hh", momBot[0], momBot[1], weight);

    		histos->Fill2DHisto("energy_vs_theta_triggered_analyzable_events_before_rotation_hh",thate_top_before_rotation, energy_top, weight);
    		histos->Fill2DHisto("energy_vs_theta_triggered_analyzable_events_before_rotation_hh",thate_bot_before_rotation, energy_bot, weight);

    		histos->Fill2DHisto("thetaTop_vs_thetaBot_triggered_analyzable_events_before_rotation_hh",thate_bot_before_rotation, thate_top_before_rotation, weight);

    		histos->Fill1DHisto("diff_E_triggered_analyzable_events_before_rotation_h", energy_diff_top, weight);
    		histos->Fill1DHisto("diff_E_triggered_analyzable_events_before_rotation_h", energy_diff_bot, weight);

    		histos->Fill1DHisto("diff_theta_triggered_analyzable_events_before_rotation_h", theta_diff_top, weight);
    		histos->Fill1DHisto("diff_theta_triggered_analyzable_events_before_rotation_h", theta_diff_bot, weight);

        	histos->Fill1DHisto("diff_energy_between_recon_clulster_and_track_energy_triggered_analyzable_events_h", clTop.getEnergy() - energy_top, weight);
        	histos->Fill1DHisto("diff_energy_between_recon_clulster_and_track_energy_triggered_analyzable_events_h", clBot.getEnergy() - energy_bot, weight);
        }

        if(flag_triggered_analyzable_event && energy_diff_top > DIFFENERGYMIN && energy_diff_top < DIFFENERGYMAX
        		&& energy_diff_bot > DIFFENERGYMIN && energy_diff_bot < DIFFENERGYMAX
				&& theta_diff_top > DIFFTHETAMIN && theta_diff_top < DIFFTHETAMAX
				&& theta_diff_bot > DIFFTHETAMIN && theta_diff_bot < DIFFTHETAMAX){
			histos->Fill1DHisto("invariant_mass_vertex_triggered_analyzable_events_with_kinematic_cuts_h", invariant_mass, weight);

			histos->Fill1DHisto("pSum_vertex_triggered_analyzable_events_with_kinematic_cuts_h", pSum, weight);

			histos->Fill2DHisto("invariant_mass_vs_pSum_vertex_triggered_analyzable_events_with_kinematic_cuts_hh", pSum, invariant_mass, weight);


			histos->Fill2DHisto("px_vs_py_vertex_triggered_analyzable_events_with_kinematic_cuts_hh", momTop[0], momTop[1], weight);
			histos->Fill2DHisto("px_vs_py_vertex_triggered_analyzable_events_with_kinematic_cuts_hh", momBot[0], momBot[1], weight);

			histos->Fill1DHisto("diff_energy_between_recon_clulster_and_track_energy_triggered_analyzable_events_with_kinematic_cuts_h", clTop.getEnergy() - energy_top, weight);
			histos->Fill1DHisto("diff_energy_between_recon_clulster_and_track_energy_triggered_analyzable_events_with_kinematic_cuts_h", clBot.getEnergy() - energy_bot, weight);

			flag_triggered_analyzable_event_and_pass_kinematic_cuts = true;

        }
    }

    if(flag_triggered_analyzable_event_and_pass_kinematic_cuts == true){
		for(int i = 0; i < n_clusters_top_cut; i++){
			CalCluster cluster = clulsters_top_cut.at(i);

			std::vector<double> positionCluster = cluster.getPosition();
			histos->Fill2DHisto("xy_clusters_triggered_analyzable_event_and_pass_kinematic_cuts_hh",positionCluster[0], positionCluster[1], weight);

			CalHit* seed = (CalHit*)cluster.getSeed();
			histos->Fill1DHisto("seed_energy_cluster_triggered_analyzable_event_and_pass_kinematic_cuts_h", seed->getEnergy(), weight);
			histos->Fill1DHisto("energy_cluster_triggered_analyzable_event_and_pass_kinematic_cuts_h", cluster.getEnergy(), weight);
			histos->Fill1DHisto("n_hits_cluster_triggered_analyzable_event_and_pass_kinematic_cuts_h", cluster.getNHits(), weight);
			histos->Fill2DHisto("energy_vs_n_hits_cluster_triggered_analyzable_event_and_pass_kinematic_cuts_hh", cluster.getNHits(), cluster.getEnergy(), weight);

			int ix = seed -> getCrystalIndices()[0];
			if(ix < 0) ix++;
			int iy = seed -> getCrystalIndices()[1];

			histos->Fill2DHisto("xy_indices_clusters_triggered_analyzable_event_and_pass_kinematic_cuts_hh",ix, iy, weight);
			histos->Fill2DHisto("energy_vs_ix_clusters_triggered_analyzable_event_and_pass_kinematic_cuts_hh", ix, cluster.getEnergy(), weight);
			histos->Fill2DHisto("energy_vs_iy_clusters_triggered_analyzable_event_and_pass_kinematic_cuts_hh", iy, cluster.getEnergy(), weight);

		}

		for(int i = 0; i < n_clusters_bot_cut; i++){
			CalCluster cluster = clulsters_bot_cut.at(i);

			std::vector<double> positionCluster = cluster.getPosition();
			histos->Fill2DHisto("xy_clusters_triggered_analyzable_event_and_pass_kinematic_cuts_hh",positionCluster[0], positionCluster[1], weight);

			CalHit* seed = (CalHit*)cluster.getSeed();
			histos->Fill1DHisto("seed_energy_cluster_triggered_analyzable_event_and_pass_kinematic_cuts_h", seed->getEnergy(), weight);
			histos->Fill1DHisto("energy_cluster_triggered_analyzable_event_and_pass_kinematic_cuts_h", cluster.getEnergy(), weight);
			histos->Fill1DHisto("n_hits_cluster_triggered_analyzable_event_and_pass_kinematic_cuts_h", cluster.getNHits(), weight);
			histos->Fill2DHisto("energy_vs_n_hits_cluster_triggered_analyzable_event_and_pass_kinematic_cuts_hh", cluster.getNHits(), cluster.getEnergy(), weight);

			int ix = seed -> getCrystalIndices()[0];
			if(ix < 0) ix++;
			int iy = seed -> getCrystalIndices()[1];

			histos->Fill2DHisto("xy_indices_clusters_triggered_analyzable_event_and_pass_kinematic_cuts_hh",ix, iy, weight);
			histos->Fill2DHisto("energy_vs_ix_clusters_triggered_analyzable_event_and_pass_kinematic_cuts_hh", ix, cluster.getEnergy(), weight);
			histos->Fill2DHisto("energy_vs_iy_clusters_triggered_analyzable_event_and_pass_kinematic_cuts_hh", iy, cluster.getEnergy(), weight);
		}
    }

    ////////////////////////////////// To determine flag of triggered events //////////////////////////////////
    bool flag_triggered = false;
	std::vector<CalCluster> cls_top;
	std::vector<CalCluster> cls_bot;
	for(int i = 0; i < n_cl; i++){
		CalCluster* cluster = gtpClusters_->at(i);
		double energy = cluster->getEnergy();
		CalHit* seed = (CalHit*)cluster->getSeed();
		int iy = seed -> getCrystalIndices()[1];
		if(iy > 0) cls_top.push_back(*cluster);
		else cls_bot.push_back(*cluster);
	}

	for(int i = 0; i < cls_top.size(); i++){
		CalCluster clusterTop = cls_top.at(i);
		double energyTop = clusterTop.getEnergy();
		CalHit* seedTop = (CalHit*)clusterTop.getSeed();
		int ixTop = seedTop -> getCrystalIndices()[0];
		if(ixTop < 0) ixTop++;
		int iyTop = seedTop -> getCrystalIndices()[1];

		if ( energyTop <= CLUSTERENERGYMAX
				&& energyTop >= CLUSTERENERGYMIN
				&& ixTop >= CLUSTERXMIN && ixTop <= CLUSTERXMAX
				&& iyTop >= CLUSTERYMIN && iyTop <= CLUSTERYMAX
				&& energyTop <= func_pde_moller->Eval(ixTop)){

			for(int j = 0; j < cls_bot.size(); j++){

				CalCluster clusterBot = cls_bot.at(j);
				double energyBot = clusterBot.getEnergy();
				CalHit* seedBot = (CalHit*)clusterBot.getSeed();
				int ixBot = seedBot -> getCrystalIndices()[0];
				if(ixBot < 0) ixBot++;
				int iyBot = seedBot -> getCrystalIndices()[1];
				if ( energyBot <= CLUSTERENERGYMAX
						&& energyBot >= CLUSTERENERGYMIN
						&& ixBot >= CLUSTERXMIN && ixBot <= CLUSTERXMAX
						&& iyBot >= CLUSTERYMIN && iyBot <= CLUSTERYMAX
						&& energyBot <= func_pde_moller->Eval(ixBot)){

					double energy_sum = energyTop + energyBot;
					double energy_diff =  fabs(energyTop - energyBot);
					double coplanarity = fabs(getValueCoplanarity(clusterTop, clusterBot));

					if(energy_sum >= ENERGYSUMMIN && energy_sum <= ENERGYSUMMAX && energy_diff <= ENERGYDIFF && coplanarity <= COPLANARITY)
						flag_triggered = true;
				}
			}
		}
	}

	if(flag_triggered){
		histos->Fill1DHisto("n_tracks_triggered_h", n_tracks, weight);
		histos->Fill1DHisto("n_clusters_triggered_h", n_cl, weight);
		histos->Fill1DHisto("n_vtxs_triggered_h", n_vtxs, weight);

		histos->Fill2DHisto("n_clusters_vs_n_tracks_triggered_hh", n_tracks, n_cl, weight);
		histos->Fill2DHisto("n_clusters_vs_n_vtxs_triggered_hh", n_vtxs, n_cl, weight);
		histos->Fill2DHisto("n_tracks_vs_n_vtxs_triggered_hh", n_vtxs, n_tracks, weight);
	}

	////////////////////////////////// To analyze truth information //////////////////////////////////
	int n_moller = 0;
	int n_wab = 0;
	int n_beam = 0;

	// Mom PDG 203: Moller
	// Mom PDG 622: wab
	// Mom PDG 204: beam
	if (mcParts_) {
		for (int j = 0; j < mcParts_->size(); j++) {
			MCParticle* mcParticle = mcParts_->at(j);
			int pdg = mcParticle->getPDG();

			if(pdg == 203) n_moller++;
			else if(pdg == 622) n_wab++;
			else if(pdg == 204) n_beam++;

		}
	}

	histos->Fill1DHisto("n_moller_h", n_moller, weight);
	histos->Fill1DHisto("n_wab_h", n_wab, weight);
	histos->Fill1DHisto("n_beam_h", n_beam, weight);

	bool flag_moller_truth = false;
	for (int i = 0; i < n_vtxs; i++) {
		Vertex* vtx = vtxs_->at(i);

		int n_entries = vtx->getParticles()->GetEntries();
		if (n_entries != 2) {
			std::cout << "Warning: entries of Moller vertex is not 2."
					<< std::endl;
			return false;
		}

		double invariant_mass = vtx->getInvMass();

		Particle* particleTop = (Particle*) vtx->getParticles()->At(0);
		Particle* particleBot = (Particle*) vtx->getParticles()->At(1);

		std::vector<double> momTop = particleTop->getMomentum();
		std::vector<double> momBot = particleBot->getMomentum();

		double pTop = sqrt(pow(momTop[0], 2) + pow(momTop[1], 2) + pow(momTop[2], 2));
		double pBot = sqrt(pow(momBot[0], 2) + pow(momBot[1], 2) + pow(momBot[2], 2));

        Track trackTop = particleTop->getTrack();
        Track trackBot = particleBot->getTrack();

		int indexTop = 0;
		int indexBot = 0;

		double diffTrackMomentumMCPEnergyTop = 10000;
		double diffTrackMomentumMCPEnergyBot = 10000;

		// Suppose that two tracks from vertex are from two outgoing electrons of Mollers, respectively.
		// A MCP from Moller in MCP collection is matched with a track from Moller-vertex in Moller vertex collection,
		// where sign of py are consistent between MCP and track
		// If a matched MCP from Moller could be found in MCP collections for a track,
		// then a MCP with closest energy and the same sign of py is supposed to be matched with the track.

		bool top_matching_flag = false;
		bool bot_matching_flag = false;
		if (mcParts_) {
			for (int j = 0; j < mcParts_->size(); j++) {
				MCParticle* mcParticle = mcParts_->at(j);

				if(mcParticle->getPDG() == 11){
					double mcpEnergy = mcParticle->getEnergy();
					std::vector<double> momMCP = mcParticle->getMomentum();

					if(mcParticle->getMomPDG() == 203){
						if (momMCP[1] > 0) {
							diffTrackMomentumMCPEnergyTop = pTop - mcpEnergy;
							indexTop = j;
							top_matching_flag = true;
						}

						if (momMCP[1] < 0) {
							diffTrackMomentumMCPEnergyBot = pBot - mcpEnergy;
							indexBot = j;
							bot_matching_flag = true;
						}
					}
					else{
						if (!top_matching_flag && momMCP[1] > 0 && fabs(pTop - mcpEnergy) < fabs(diffTrackMomentumMCPEnergyTop)) {
							diffTrackMomentumMCPEnergyTop = pTop - mcpEnergy;
							indexTop = j;
						}
						if (!bot_matching_flag && momMCP[1] < 0 && fabs(pBot - mcpEnergy) < fabs(diffTrackMomentumMCPEnergyBot)) {
							diffTrackMomentumMCPEnergyBot = pBot - mcpEnergy;
							indexBot = j;
						}
					}
				}
			}
		}

		MCParticle* mcParticleTop = mcParts_->at(indexTop);
		MCParticle* mcParticleBot = mcParts_->at(indexBot);

		std::vector<double> momMCPTop = mcParticleTop->getMomentum();
		std::vector<double> momMCPBot = mcParticleBot->getMomentum();

		double mcpEnergyTop = mcParticleTop->getEnergy();
		double mcpEnergyBot = mcParticleBot->getEnergy();

		int momIDTop = mcParticleTop->getMomID();
		int momIDBot = mcParticleBot->getMomID();

		int momPDGTop = mcParticleTop->getMomPDG();
		int momPDGBot = mcParticleBot->getMomPDG();

	    _reg_tracks_from_vertices->setVariableValue("momIDTop", momIDTop);
	    _reg_tracks_from_vertices->setVariableValue("momIDBot", momIDBot);

        _reg_tracks_from_vertices->setVariableValue("momPDGTop", momPDGTop);
        _reg_tracks_from_vertices->setVariableValue("momPDGBot", momPDGBot);

        _reg_tracks_from_vertices->addToVector("momTop", momTop[0]);
        _reg_tracks_from_vertices->addToVector("momTop", momTop[1]);
        _reg_tracks_from_vertices->addToVector("momTop", momTop[2]);

        _reg_tracks_from_vertices->addToVector("momBot", momBot[0]);
        _reg_tracks_from_vertices->addToVector("momBot", momBot[1]);
        _reg_tracks_from_vertices->addToVector("momBot", momBot[2]);

        _reg_tracks_from_vertices->addToVector("momMCPTop", momMCPTop[0]);
        _reg_tracks_from_vertices->addToVector("momMCPTop", momMCPTop[1]);
        _reg_tracks_from_vertices->addToVector("momMCPTop", momMCPTop[2]);

        _reg_tracks_from_vertices->addToVector("momMCPBot", momMCPBot[0]);
        _reg_tracks_from_vertices->addToVector("momMCPBot", momMCPBot[1]);
        _reg_tracks_from_vertices->addToVector("momMCPBot", momMCPBot[2]);

		double timeTop = trackTop.getTrackTime();
		double timeBot = trackBot.getTrackTime();

		for (int j = 0; j < n_tracks; j++) {
			Track* track = trks_->at(j);
			std::vector<double> momTrack = track->getMomentum();
			double timeTrack = track->getTrackTime();
			if (momTop[0] == momTrack[0] && momTop[1] == momTrack[1] && momTop[1] == momTrack[1])
				timeTop = timeTrack;
			if (momBot[0] == momTrack[0] && momBot[1] == momTrack[1] && momBot[1] == momTrack[1])
				timeBot = timeTrack;
		}

        _reg_tracks_from_vertices->setVariableValue("timeTop", trackTop.getTrackTime());
        _reg_tracks_from_vertices->setVariableValue("timeBot", trackBot.getTrackTime());

        _reg_tracks_from_vertices->setVariableValue("nClustersAssociatedTracksTop", n_clusters_top_cut);
        _reg_tracks_from_vertices->setVariableValue("nClustersAssociatedTracksBot", n_clusters_bot_cut);

        _reg_tracks_from_vertices->setVariableValue("imVertex", invariant_mass);
        _reg_tracks_from_vertices->addToVector("momVertex", vtx->getP().Px());
        _reg_tracks_from_vertices->addToVector("momVertex", vtx->getP().Py());
        _reg_tracks_from_vertices->addToVector("momVertex", vtx->getP().Pz());

        _reg_tracks_from_vertices->setVariableValue("single_triggered_analyzable_flag", flag_single_triggered_analyzable_event);
        _reg_tracks_from_vertices->setVariableValue("triggered_analyzable_flag", flag_triggered_analyzable_event);
        _reg_tracks_from_vertices->setVariableValue("triggered_analyzable_and_kinematic_cuts_flag", flag_triggered_analyzable_event_and_pass_kinematic_cuts);

        _reg_tracks_from_vertices->fill();
	}
    return true;
}

void TriggerParametersExtractionMollerPairTriggerAnaProcessor::finalize() {
    outF_->cd();
    histos->saveHistos(outF_, anaName_.c_str());
    delete histos;
    histos = nullptr;

    TDirectory* dir_gtp_cluster_pairs{nullptr};
    dir_gtp_cluster_pairs = outF_->mkdir((anaName_+"_gtp_cluster_pairs").c_str());
    dir_gtp_cluster_pairs->cd();
    _reg_gtp_cluster_pairs->writeTree();

    TDirectory* dir_tracks_from_vertices{nullptr};
    dir_tracks_from_vertices = outF_->mkdir((anaName_+"_tracks_from_vertices").c_str());
    dir_tracks_from_vertices->cd();
    _reg_tracks_from_vertices->writeTree();

    outF_->Close();

}

std::vector<double> TriggerParametersExtractionMollerPairTriggerAnaProcessor::getCrystalPosition(CalCluster cluster){
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

double TriggerParametersExtractionMollerPairTriggerAnaProcessor::getAngle(CalCluster cluster) {
    // Get the variables used by the calculation.
	std::vector<double> position = getCrystalPosition(cluster);
	double angle = std::round(atan(position[0] / position[1]) * 180.0 / PI);

	return angle;
}

double TriggerParametersExtractionMollerPairTriggerAnaProcessor::getValueCoplanarity(CalCluster clusterTop, CalCluster clusterBot) {
	 // Calculate the coplanarity cut value.
	return getAngle(clusterTop) + getAngle(clusterBot);
}

double TriggerParametersExtractionMollerPairTriggerAnaProcessor::getR(CalCluster cluster) {
	std::vector<double> position = getCrystalPosition(cluster);
	double slopeParamR = sqrt( pow(position[0],2) + pow( position[1], 2) );
	return slopeParamR;
}

std::vector<double> TriggerParametersExtractionMollerPairTriggerAnaProcessor::getVariablesForEnergySlopeCut(CalCluster clusterTop, CalCluster clusterBot){
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

DECLARE_PROCESSOR(TriggerParametersExtractionMollerPairTriggerAnaProcessor);
