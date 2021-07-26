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
#define CLUSTERENERGYMIN 0.72 // minimum of cluster energy
#define CLUSTERENERGYMAX 1.52 // maximum of cluster energy
#define CLUSTERXMIN -13 // minimum of x index
#define CLUSTERXMAX -10 // maximum of x index
#define CLUSTERYMIN -1 // minimum of y index
#define CLUSTERYMAX 1 // maximum of y index
#define ROTATIONANGLEAROUNDY 0.0305 // rad
#define DIFFENERGYMIN -0.34 // minimum for difference between measured and calculated energy
#define DIFFENERGYMAX 0.33 // maximum for difference between measured and calculated energy
#define DIFFTHETAMIN -0.0030 // minimum for difference between measured and calculated theta before rotation
#define DIFFTHETAMAX 0.0046 // maximum for difference between measured and calculated theta before rotation

//#define DIFFTHETAMIN -0.1 // minimum for difference between measured and calculated theta before rotation
//#define DIFFTHETAMAX 0.02 // maximum for difference between measured and calculated theta before rotation

#define DIFFTRACKMOMENTUMMCPENERGY 0.13 // GeV maxium for differece between track's momentum and mcp's energy

TriggerParametersExtractionMollerAnaProcessor::TriggerParametersExtractionMollerAnaProcessor(const std::string& name, Process& process) : Processor(name,process) {

}

TriggerParametersExtractionMollerAnaProcessor::~TriggerParametersExtractionMollerAnaProcessor(){}

void TriggerParametersExtractionMollerAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring TriggerParametersExtractionMollerAnaProcessor" <<std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        anaName_         = parameters.getString("anaName");
        histCfgFilename_      = parameters.getString("histCfg",histCfgFilename_);
        trkColl_    = parameters.getString("trkColl");
        gtpClusColl_    = parameters.getString("gtpClusColl");
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

    //NHits dependence energy
    func_nhde = new TF1("func_nhde", "pol1", 0, 20);
    func_nhde->SetParameters(pars_nhde);

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

    _reg_tuple = std::make_shared<FlatTupleMaker>(anaName_ + "_tree");
    _reg_tuple->addVariable("momPDGTop");
    _reg_tuple->addVariable("momPDGBot");
    _reg_tuple->addVector("momTop");
    _reg_tuple->addVector("momBot");
    _reg_tuple->addVector("momMCPTop");
    _reg_tuple->addVector("momMCPBot");

    _reg_tuple->addVariable("timeTop");
    _reg_tuple->addVariable("timeBot");

    _reg_tuple->addVector("momVertex");
    _reg_tuple->addVariable("imVertex");

    _reg_tuple->addVariable("analyzable_flag");
    _reg_tuple->addVariable("triggered_analyzable_flag");
    _reg_tuple->addVariable("triggered_analyzable_and_kinematic_cuts_flag");
}

bool TriggerParametersExtractionMollerAnaProcessor::process(IEvent* ievent) {
    double weight = 1.;

    // To extract analyzable events
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
					break;
				}
			}
		}
	}

	int n_clusters_top_cut = clulsters_top_cut.size();
	int n_clusters_bot_cut = clulsters_bot_cut.size();

	int flag = false;

	for(int i = 0; i < n_clusters_top_cut; i++){
		CalCluster cluster = clulsters_top_cut.at(i);
		if(cluster.getEnergy() >= CLUSTERENERGYTHRESHOLD) flag = true;
	}

	for(int i = 0; i < n_clusters_bot_cut; i++){
		CalCluster cluster = clulsters_bot_cut.at(i);
		if(cluster.getEnergy() >= CLUSTERENERGYTHRESHOLD) flag = true;
	}

	int flag_analyzable_event = false;
	int flag_triggered_analyzable_event = false;

	// To determine flags of analyzable events and triggered analyzable events
	if( ( tracks_top.size() >= 1 && tracks_bot.size() >= 1 ) && (n_clusters_top_cut >=1 || n_clusters_bot_cut >= 1) && flag){
		flag_analyzable_event = true;

		for(int i = 0; i < n_clusters_top_cut; i++){
			CalCluster cluster = clulsters_top_cut.at(i);

			std::vector<double> positionCluster = cluster.getPosition();
			histos->Fill2DHisto("xy_clusters_analyzable_events_hh",positionCluster[0], positionCluster[1], weight);

			CalHit* seed = (CalHit*)cluster.getSeed();
			histos->Fill1DHisto("seed_energy_cluster_analyzable_events_h", seed->getEnergy(), weight);
			histos->Fill1DHisto("energy_cluster_analyzable_events_h", cluster.getEnergy(), weight);
			histos->Fill1DHisto("n_hits_cluster_analyzable_events_h", cluster.getNHits(), weight);
			histos->Fill2DHisto("energy_vs_n_hits_cluster_analyzable_events_hh", cluster.getNHits(), cluster.getEnergy(), weight);

			int ix = seed -> getCrystalIndices()[0];
			if(ix < 0) ix++;
			int iy = seed -> getCrystalIndices()[1];

			histos->Fill1DHisto("n_clusters_xAxis_analyzable_events_h", ix, weight);
			histos->Fill2DHisto("xy_indices_clusters_analyzable_events_hh",ix, iy, weight);
			histos->Fill2DHisto("energy_vs_ix_clusters_analyzable_events_hh",
					ix, cluster.getEnergy(), weight);
			histos->Fill2DHisto("energy_vs_iy_clusters_analyzable_events_hh",
					iy, cluster.getEnergy(), weight);

			if (cluster.getEnergy() <= CLUSTERENERGYMAX
					&& cluster.getEnergy() >= CLUSTERENERGYMIN
					&& ix >= CLUSTERXMIN && ix <= CLUSTERXMAX
					&& iy >= CLUSTERYMIN && iy <= CLUSTERYMAX
					&& cluster.getEnergy() <= func_pde_moller->Eval(ix))
				flag_triggered_analyzable_event = true;
		}

		for(int i = 0; i < n_clusters_bot_cut; i++){
			CalCluster cluster = clulsters_bot_cut.at(i);

			std::vector<double> positionCluster = cluster.getPosition();
			histos->Fill2DHisto("xy_clusters_analyzable_events_hh",positionCluster[0], positionCluster[1], weight);

			CalHit* seed = (CalHit*)cluster.getSeed();
			histos->Fill1DHisto("seed_energy_cluster_analyzable_events_h", seed->getEnergy(), weight);
			histos->Fill1DHisto("energy_cluster_analyzable_events_h", cluster.getEnergy(), weight);
			histos->Fill1DHisto("n_hits_cluster_analyzable_events_h", cluster.getNHits(), weight);
			histos->Fill2DHisto("energy_vs_n_hits_cluster_analyzable_events_hh", cluster.getNHits(), cluster.getEnergy(), weight);

			int ix = seed -> getCrystalIndices()[0];
			if(ix < 0) ix++;
			int iy = seed -> getCrystalIndices()[1];

			histos->Fill1DHisto("n_clusters_xAxis_analyzable_events_h", ix, weight);
			histos->Fill2DHisto("xy_indices_clusters_analyzable_events_hh",ix, iy, weight);
			histos->Fill2DHisto("energy_vs_ix_clusters_analyzable_events_hh", ix, cluster.getEnergy(), weight);
			histos->Fill2DHisto("energy_vs_iy_clusters_analyzable_events_hh", iy, cluster.getEnergy(), weight);

			if(cluster.getEnergy() < CLUSTERENERGYMAX && cluster.getEnergy() > CLUSTERENERGYMIN) flag_triggered_analyzable_event = true;
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

	//To determine flag of triggered analyzable events with kinematic cuts
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

		double theta_bot_calculated_before_rotation = func_theta1_vs_theta2_before_roation->Eval(thate_top_before_rotation);

		double energy_diff_top = energy_top - energy_calcuated_top;
		double energy_diff_bot = energy_bot - energy_calcuated_bot;
		double theta_diff = thate_bot_before_rotation - theta_bot_calculated_before_rotation;

		histos->Fill2DHisto("energy_vs_theta_track_pair_from_vertex_before_rotation_hh",thate_top_before_rotation, energy_top, weight);
		histos->Fill2DHisto("energy_vs_theta_track_pair_from_vertex_before_rotation_hh",thate_bot_before_rotation, energy_bot, weight);

		histos->Fill2DHisto("thetaTop_vs_thetaBot_track_pair_from_vertex_before_rotation_hh",thate_bot_before_rotation, thate_top_before_rotation, weight);

		histos->Fill1DHisto("diff_E_vertex_before_rotation_h", energy_diff_top, weight);
		histos->Fill1DHisto("diff_E_vertex_before_rotation_h", energy_diff_bot, weight);

		histos->Fill1DHisto("diff_theta_vertex_before_rotation_h", theta_diff, weight);

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

        if(flag_analyzable_event){
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

    		histos->Fill1DHisto("diff_theta_analyzable_events_before_rotation_h", theta_diff, weight);

    		if(energy_diff_top > DIFFENERGYMIN && energy_diff_top < DIFFENERGYMAX && energy_diff_bot > DIFFENERGYMIN && energy_diff_bot < DIFFENERGYMAX){
        		histos->Fill2DHisto("thetaTop_vs_thetaBot_analyzable_events_before_rotation_with_diff_energy_cut_hh",thate_bot_before_rotation, thate_top_before_rotation, weight);
        		histos->Fill1DHisto("diff_theta_analyzable_events_before_rotation_with_diff_energy_cut_h", theta_diff, weight);
    		}

        	histos->Fill1DHisto("diff_energy_between_recon_clulster_and_track_energy_analyzable_events_h", clTop.getEnergy() - energy_top, weight);
        	histos->Fill1DHisto("diff_energy_between_recon_clulster_and_track_energy_analyzable_events_h", clBot.getEnergy() - energy_bot, weight);

        	if( (energy_diff_top < DIFFENERGYMIN || energy_diff_top > DIFFENERGYMAX )
            		|| (energy_diff_bot < DIFFENERGYMIN || energy_diff_bot > DIFFENERGYMAX)
    				|| (theta_diff < DIFFTHETAMIN || theta_diff > DIFFTHETAMAX))
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

    		histos->Fill1DHisto("diff_theta_triggered_analyzable_events_before_rotation_h", theta_diff, weight);

        	histos->Fill1DHisto("diff_energy_between_recon_clulster_and_track_energy_triggered_analyzable_events_h", clTop.getEnergy() - energy_top, weight);
        	histos->Fill1DHisto("diff_energy_between_recon_clulster_and_track_energy_triggered_analyzable_events_h", clBot.getEnergy() - energy_bot, weight);
        }

        if(flag_triggered_analyzable_event && energy_diff_top > DIFFENERGYMIN && energy_diff_top < DIFFENERGYMAX
        		&& energy_diff_bot > DIFFENERGYMIN && energy_diff_bot < DIFFENERGYMAX
				&& theta_diff > DIFFTHETAMIN && theta_diff < DIFFTHETAMAX){
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

    // To determine flag of triggered events
    bool flag_triggered = false;
	for(int i = 0; i < n_cl; i++){
		CalCluster* cluster = gtpClusters_->at(i);
		double energy = cluster->getEnergy();

		CalHit* seed = (CalHit*)cluster->getSeed();

		int ix = seed -> getCrystalIndices()[0];
		if(ix < 0) ix++;
		int iy = seed -> getCrystalIndices()[1];

		if (energy <= CLUSTERENERGYMAX
				&& energy >= CLUSTERENERGYMIN
				&& ix >= CLUSTERXMIN && ix <= CLUSTERXMAX
				&& iy >= CLUSTERYMIN && iy <= CLUSTERYMAX
				&& energy <= func_pde_moller->Eval(ix))
			flag_triggered = true;
	}

	if(flag_triggered){
		histos->Fill1DHisto("n_tracks_triggered_h", n_tracks, weight);
		histos->Fill1DHisto("n_clusters_triggered_h", n_cl, weight);
		histos->Fill1DHisto("n_vtxs_triggered_h", n_vtxs, weight);

		histos->Fill2DHisto("n_clusters_vs_n_tracks_triggered_hh", n_tracks, n_cl, weight);
		histos->Fill2DHisto("n_clusters_vs_n_vtxs_triggered_hh", n_vtxs, n_cl, weight);
		histos->Fill2DHisto("n_tracks_vs_n_vtxs_triggered_hh", n_vtxs, n_tracks, weight);
	}

	// To analyze truth information

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

			if(pdg == 203){
				histos->Fill1DHisto("n_daughters_moller_h", mcParticle->getNumDaughters(), weight);
			}

		}
	}

	histos->Fill1DHisto("n_moller_h", n_moller, weight);
	histos->Fill1DHisto("n_wab_h", n_wab, weight);
	histos->Fill1DHisto("n_beam_h", n_beam, weight);

	/*
	if(n_vtxs >=1 ){
		std::cout << "****************************"<<std::endl;
		for(int i = 0; i < n_tracks; i++){
			Track* track = trks_->at(i);
			std::vector<double> mom = track->getMomentum();
			std::cout << "track " << i <<": " << mom[0] << "  " << mom[1] << "  " << mom[2] << "  " << track->getP()  << std::endl;
		}

		if (mcParts_) {
			for (int i = 0; i < mcParts_->size(); i++) {
				MCParticle* mcParticle = mcParts_->at(i);
				int pdg = mcParticle->getPDG();

				std::vector<double> momMCP = mcParticle->getMomentum();
				std::cout <<"MPC " << i << ": "<< momMCP[0] << "  " << momMCP[1] << "  " << momMCP[2] << "  " << sqrt(pow(momMCP[0], 2) + pow(momMCP[1], 2) + pow(momMCP[2], 2)) << std::endl;
			}
		}
	}
	 */

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

		double theta_bot_calculated_before_rotation = func_theta1_vs_theta2_before_roation->Eval(thate_top_before_rotation);

		double energy_diff_top = energy_top - energy_calcuated_top;
		double energy_diff_bot = energy_bot - energy_calcuated_bot;
		double theta_diff = thate_bot_before_rotation - theta_bot_calculated_before_rotation;

		double pTop = sqrt(pow(momTop[0], 2) + pow(momTop[1], 2) + pow(momTop[2], 2));
		double pBot = sqrt(pow(momBot[0], 2) + pow(momBot[1], 2) + pow(momBot[2], 2));
		double pSum = pTop + pBot;

        Track trackTop = particleTop->getTrack();
        Track trackBot = particleBot->getTrack();

        std::vector<double> positionAtEcalTop = trackTop.getPositionAtEcal();
        std::vector<double> positionAtEcalBot = trackBot.getPositionAtEcal();

        CalCluster clTop = particleTop->getCluster();
        CalCluster clBot = particleBot->getCluster();


		int indexTop = 0;
		int indexBot = 0;

		double diffTrackMomentumMCPEnergyTop = 10000;
		double diffTrackMomentumMCPEnergyBot = 10000;

		// Find matched MCP for top and bot tracks, separately.
		// Energy of matched MCP is closest to tracks's momentum.
		if (mcParts_) {
			for (int j = 0; j < mcParts_->size(); j++) {
				MCParticle* mcParticle = mcParts_->at(j);

				if(mcParticle->getMomPDG() == 203 && mcParticle->getPDG() == 11){
					double mcpEnergy = mcParticle->getEnergy();
					std::vector<double> momMCP = mcParticle->getMomentum();

					if (momMCP[1] > 0 && fabs(pTop - mcpEnergy) < fabs(diffTrackMomentumMCPEnergyTop)) {
						diffTrackMomentumMCPEnergyTop = pTop - mcpEnergy;
						indexTop = j;
					}

					if (momMCP[1] < 0 && fabs(pBot - mcpEnergy) < fabs(diffTrackMomentumMCPEnergyBot)) {
						diffTrackMomentumMCPEnergyBot = pBot - mcpEnergy;
						indexBot = j;
					}
				}
			}
		}

		histos->Fill1DHisto("diff_track_momentum_and_mcp_energy_top_no_cuts_h", diffTrackMomentumMCPEnergyTop, weight);
		histos->Fill1DHisto("diff_track_momentum_and_mcp_energy_bot_no_cuts_h", diffTrackMomentumMCPEnergyBot, weight);


		MCParticle* mcParticleTop = mcParts_->at(indexTop);
		MCParticle* mcParticleBot = mcParts_->at(indexBot);

		std::vector<double> momMCPTop = mcParticleTop->getMomentum();
		std::vector<double> momMCPBot = mcParticleBot->getMomentum();

		double mcpEnergyTop = mcParticleTop->getEnergy();
		double mcpEnergyBot = mcParticleBot->getEnergy();

		int momPDGTop = mcParticleTop->getMomPDG();
		int momPDGBot = mcParticleBot->getMomPDG();


		histos->Fill2DHisto("track_momentum_vs_mcp_energy_top_no_cuts_hh", mcpEnergyTop, pTop, weight);
		histos->Fill2DHisto("track_momentum_vs_mcp_energy_bot_no_cuts_hh", mcpEnergyBot, pBot, weight);

		if(mcpEnergyTop < 2) histos->Fill1DHisto("diff_track_momentum_and_mcp_energy_top_less_than_2_h", diffTrackMomentumMCPEnergyTop, weight);
		if(mcpEnergyBot < 2) histos->Fill1DHisto("diff_track_momentum_and_mcp_energy_bot_less_than_2_h", diffTrackMomentumMCPEnergyBot, weight);

        _reg_tuple->setVariableValue("momPDGTop", momPDGTop);
        _reg_tuple->setVariableValue("momPDGBot", momPDGBot);

        _reg_tuple->addToVector("momTop", momTop[0]);
        _reg_tuple->addToVector("momTop", momTop[1]);
        _reg_tuple->addToVector("momTop", momTop[2]);

        _reg_tuple->addToVector("momBot", momBot[0]);
        _reg_tuple->addToVector("momBot", momBot[1]);
        _reg_tuple->addToVector("momBot", momBot[2]);

        _reg_tuple->addToVector("momMCPTop", momMCPTop[0]);
        _reg_tuple->addToVector("momMCPTop", momMCPTop[1]);
        _reg_tuple->addToVector("momMCPTop", momMCPTop[2]);

        _reg_tuple->addToVector("momMCPBot", momMCPBot[0]);
        _reg_tuple->addToVector("momMCPBot", momMCPBot[1]);
        _reg_tuple->addToVector("momMCPBot", momMCPBot[2]);

        _reg_tuple->setVariableValue("timeTop", trackTop.getTrackTime());
        _reg_tuple->setVariableValue("timeBot", trackBot.getTrackTime());

        _reg_tuple->setVariableValue("imVertex", invariant_mass);
        _reg_tuple->addToVector("momVertex", vtx->getP().Px());
        _reg_tuple->addToVector("momVertex", vtx->getP().Py());
        _reg_tuple->addToVector("momVertex", vtx->getP().Pz());

        _reg_tuple->setVariableValue("analyzable_flag", flag_analyzable_event);
        _reg_tuple->setVariableValue("triggered_analyzable_flag", flag_triggered_analyzable_event);
        _reg_tuple->setVariableValue("triggered_analyzable_and_kinematic_cuts_flag", flag_triggered_analyzable_event_and_pass_kinematic_cuts);

        _reg_tuple->fill();

		if(fabs(diffTrackMomentumMCPEnergyTop) > DIFFTRACKMOMENTUMMCPENERGY || fabs(diffTrackMomentumMCPEnergyBot) > DIFFTRACKMOMENTUMMCPENERGY) break;

		histos->Fill2DHisto("diff_px_track_mcp_after_momentum_match_hh", momMCPTop[0] - momTop[0], momMCPBot[0] - momBot[0], weight);
		histos->Fill2DHisto("diff_py_track_mcp_after_momentum_match_hh", momMCPTop[1] - momTop[1], momMCPBot[1] - momBot[1], weight);
		histos->Fill2DHisto("diff_pz_track_mcp_after_momentum_match_hh", momMCPTop[2] - momTop[2], momMCPBot[2] - momBot[2], weight);

		/*
		std::cout <<"MPC top: " << momMCPTop[0] << "  " << momMCPTop[1] << "  " << momMCPTop[2] << "  " << mcpEnergyTop  << std::endl;
		std::cout <<"MPC bot: " << momMCPBot[0] << "  " << momMCPBot[1] << "  " << momMCPBot[2] << "  " << mcpEnergyBot << std::endl;

		std::cout <<"Par top: " << momTop[0] << "  " << momTop[1] << "  " << momTop[2] << "  " << pTop << std::endl;
		std::cout <<"Par bot: " << momBot[0] << "  " << momBot[1] << "  " << momBot[2] << "  " << pBot << std::endl;
		 */

		int momPDGIDTop = 4;
		if(momPDGTop == 203) momPDGIDTop = 1;
		else if(momPDGTop == 622) momPDGIDTop = 2;
		else if(momPDGTop == 204) momPDGIDTop = 3;
		else momPDGIDTop = 4;

		int momPDGIDBot = 4;
		if(momPDGBot == 203) momPDGIDBot = 1;
		else if(momPDGBot == 622) momPDGIDBot = 2;
		else if(momPDGBot == 204) momPDGIDBot = 3;
		else momPDGIDBot = 4;

		int idMomTop = mcParticleTop->getMomID();
		int idMomBot = mcParticleBot->getMomID();

		if(idMomTop == idMomBot)
			histos->Fill1DHisto("mom_id_match_no_cuts_h", 1, weight);
		else
			histos->Fill1DHisto("mom_id_match_no_cuts_h", 0, weight);

		if (flag_analyzable_event) {
			histos->Fill2DHisto("track_momentum_vs_mcp_energy_top_analyzable_event_hh", mcpEnergyTop, pTop, weight);
			histos->Fill2DHisto("track_momentum_vs_mcp_energy_bot_analyzable_event_hh", mcpEnergyBot, pBot, weight);

			histos->Fill1DHisto("motherPDG_top_analyzable_event_h", momPDGIDTop, weight);
			histos->Fill1DHisto("motherPDG_bot_analyzable_event_h", momPDGIDBot, weight);
			histos->Fill2DHisto("motherPDGTop_vs_motherPDGBot_analyzable_event_hh", momPDGIDBot, momPDGIDTop, weight);

			if(momPDGIDTop == 1 && momPDGIDBot == 1){
				if(idMomTop == idMomBot)
					histos->Fill1DHisto("mom_id_match_analyzable_events_both_tracks_from_moller_h", 1, weight);
				else
					histos->Fill1DHisto("mom_id_match_analyzable_events_both_tracks_from_moller_h", 0, weight);

				histos->Fill1DHisto("diff_track_time_vertex_analyzable_events_both_tracks_from_moller_h", trackTop.getTrackTime() - trackBot.getTrackTime(), weight);

				histos->Fill1DHisto("invariant_mass_vertex_analyzable_events_both_tracks_from_moller_h", invariant_mass, weight);

				histos->Fill1DHisto("pSum_vertex_analyzable_events_both_tracks_from_moller_h", pSum, weight);

				histos->Fill2DHisto("invariant_mass_vs_pSum_vertex_analyzable_events_both_tracks_from_moller_hh", pSum, invariant_mass, weight);

				histos->Fill2DHisto("px_vs_py_vertex_analyzable_events_both_tracks_from_moller_hh", momTop[0], momTop[1], weight);
				histos->Fill2DHisto("px_vs_py_vertex_analyzable_events_both_tracks_from_moller_hh", momBot[0], momBot[1], weight);

				histos->Fill2DHisto("energy_vs_theta_analyzable_events_before_rotation_both_tracks_from_moller_hh",thate_top_before_rotation, energy_top, weight);
				histos->Fill2DHisto("energy_vs_theta_analyzable_events_before_rotation_both_tracks_from_moller_hh",thate_bot_before_rotation, energy_bot, weight);

				histos->Fill2DHisto("thetaTop_vs_thetaBot_analyzable_events_before_rotation_both_tracks_from_moller_hh",thate_bot_before_rotation, thate_top_before_rotation, weight);

				histos->Fill1DHisto("diff_E_analyzable_events_before_rotation_both_tracks_from_moller_h", energy_diff_top, weight);
				histos->Fill1DHisto("diff_E_analyzable_events_before_rotation_both_tracks_from_moller_h", energy_diff_bot, weight);

				histos->Fill1DHisto("diff_theta_analyzable_events_before_rotation_both_tracks_from_moller_h", theta_diff, weight);

				if(idMomTop == idMomBot){

					histos->Fill1DHisto("diff_track_time_vertex_analyzable_events_both_tracks_from_moller_with_the_same_id_h", trackTop.getTrackTime() - trackBot.getTrackTime(), weight);

					histos->Fill1DHisto("invariant_mass_vertex_analyzable_events_both_tracks_from_moller_with_the_same_id_h", invariant_mass, weight);

					histos->Fill1DHisto("pSum_vertex_analyzable_events_both_tracks_from_moller_with_the_same_id_h", pSum, weight);

					histos->Fill2DHisto("invariant_mass_vs_pSum_vertex_analyzable_events_both_tracks_from_moller_with_the_same_id_hh", pSum, invariant_mass, weight);

					histos->Fill2DHisto("px_vs_py_vertex_analyzable_events_both_tracks_from_moller_with_the_same_id_hh", momTop[0], momTop[1], weight);
					histos->Fill2DHisto("px_vs_py_vertex_analyzable_events_both_tracks_from_moller_with_the_same_id_hh", momBot[0], momBot[1], weight);

					histos->Fill2DHisto("energy_vs_theta_analyzable_events_before_rotation_both_tracks_from_moller_with_the_same_id_hh",thate_top_before_rotation, energy_top, weight);
					histos->Fill2DHisto("energy_vs_theta_analyzable_events_before_rotation_both_tracks_from_moller_with_the_same_id_hh",thate_bot_before_rotation, energy_bot, weight);

					histos->Fill2DHisto("thetaTop_vs_thetaBot_analyzable_events_before_rotation_both_tracks_from_moller_with_the_same_id_hh",thate_bot_before_rotation, thate_top_before_rotation, weight);

					histos->Fill1DHisto("diff_E_analyzable_events_before_rotation_both_tracks_from_moller_with_the_same_id_h", energy_diff_top, weight);
					histos->Fill1DHisto("diff_E_analyzable_events_before_rotation_both_tracks_from_moller_with_the_same_id_h", energy_diff_bot, weight);

					histos->Fill1DHisto("diff_theta_analyzable_events_before_rotation_both_tracks_from_moller_with_the_same_id_h", theta_diff, weight);

					histos->Fill2DHisto("diff_px_track_mcp_vertex_analyzable_events_both_tracks_from_moller_with_the_same_id_hh", momMCPTop[0] - momTop[0], momMCPBot[0] - momBot[0], weight);
					histos->Fill2DHisto("diff_py_track_mcp_vertex_analyzable_events_both_tracks_from_moller_with_the_same_id_hh", momMCPTop[1] - momTop[1], momMCPBot[1] - momBot[1], weight);
					histos->Fill2DHisto("diff_pz_track_mcp_vertex_analyzable_events_both_tracks_from_moller_with_the_same_id_hh", momMCPTop[2] - momTop[2], momMCPBot[2] - momBot[2], weight);
				}

				if(energy_diff_top > DIFFENERGYMIN && energy_diff_top < DIFFENERGYMAX && energy_diff_bot > DIFFENERGYMIN && energy_diff_bot < DIFFENERGYMAX){
					histos->Fill2DHisto("thetaTop_vs_thetaBot_analyzable_events_before_rotation_with_diff_energy_cut_both_tracks_from_moller_hh",thate_bot_before_rotation, thate_top_before_rotation, weight);
					histos->Fill1DHisto("diff_theta_analyzable_events_before_rotation_with_diff_energy_cut_both_tracks_from_moller_h", theta_diff, weight);
				}

			}

		}

		if (flag_triggered_analyzable_event) {
	    	histos->Fill1DHisto("diff_track_time_vertex_triggered_analyzable_event_h", trackTop.getTrackTime() - trackBot.getTrackTime(), weight);
	    	if(idMomTop == idMomBot)
	    		histos->Fill1DHisto("mom_id_match_triggered_analyzable_event_h", 1, weight);
	    	else
	    		histos->Fill1DHisto("mom_id_match_triggered_analyzable_event_h", 0, weight);

			if(momPDGIDTop == 1 && momPDGIDBot == 1){
		    	histos->Fill1DHisto("diff_track_time_vertex_triggered_analyzable_event_both_tracks_from_moller_h", trackTop.getTrackTime() - trackBot.getTrackTime(), weight);
		    	if(idMomTop == idMomBot)
		    		histos->Fill1DHisto("mom_id_match_triggered_analyzable_event_both_tracks_from_moller_h", 1, weight);
		    	else
		    		histos->Fill1DHisto("mom_id_match_triggered_analyzable_event_both_tracks_from_moller_h", 0, weight);
			}

			histos->Fill2DHisto("track_momentum_vs_mcp_energy_top_triggered_analyzable_event_hh", mcpEnergyTop, pTop, weight);
			histos->Fill2DHisto("track_momentum_vs_mcp_energy_bot_triggered_analyzable_event_hh", mcpEnergyBot, pBot, weight);

			histos->Fill1DHisto("motherPDG_top_triggered_analyzable_event_h", momPDGIDTop, weight);
			histos->Fill1DHisto("motherPDG_bot_triggered_analyzable_event_h", momPDGIDBot, weight);
			histos->Fill2DHisto("motherPDGTop_vs_motherPDGBot_triggered_analyzable_event_hh", momPDGIDBot, momPDGIDTop, weight);

		}

	    if(flag_triggered_analyzable_event_and_pass_kinematic_cuts){
	    	histos->Fill1DHisto("diff_track_time_vertex_triggered_analyzable_event_and_pass_kinematic_cuts_h", trackTop.getTrackTime() - trackBot.getTrackTime(), weight);
	    	if(idMomTop == idMomBot)
	    		histos->Fill1DHisto("mom_id_match_triggered_analyzable_event_and_pass_kinematic_cuts_h", 1, weight);
	    	else
	    		histos->Fill1DHisto("mom_id_match_triggered_analyzable_event_and_pass_kinematic_cuts_h", 0, weight);

			if(momPDGIDTop == 1 && momPDGIDBot == 1){
		    	histos->Fill1DHisto("diff_track_time_vertex_triggered_analyzable_event_and_pass_kinematic_cuts_both_tracks_from_moller_h", trackTop.getTrackTime() - trackBot.getTrackTime(), weight);
		    	if(idMomTop == idMomBot)
		    		histos->Fill1DHisto("mom_id_match_triggered_analyzable_event_and_pass_kinematic_cuts_both_tracks_from_moller_h", 1, weight);
		    	else
		    		histos->Fill1DHisto("mom_id_match_triggered_analyzable_event_and_pass_kinematic_cuts_both_tracks_from_moller_h", 0, weight);

		    	if(idMomTop == idMomBot){
		    		histos->Fill2DHisto("diff_px_track_mcp_vertex_triggered_analyzable_event_and_pass_kinematic_cuts_both_tracks_from_moller_with_the_same_id_hh", momMCPTop[0] - momTop[0], momMCPBot[0] - momBot[0], weight);
		    		histos->Fill2DHisto("diff_py_track_mcp_vertex_triggered_analyzable_event_and_pass_kinematic_cuts_both_tracks_from_moller_with_the_same_id_hh", momMCPTop[1] - momTop[1], momMCPBot[1] - momBot[1], weight);
		    		histos->Fill2DHisto("diff_pz_track_mcp_vertex_triggered_analyzable_event_and_pass_kinematic_cuts_both_tracks_from_moller_with_the_same_id_hh", momMCPTop[2] - momTop[2], momMCPBot[2] - momBot[2], weight);
		    	}
			}

			histos->Fill2DHisto("track_momentum_vs_mcp_energy_top_triggered_analyzable_event_and_pass_kinematic_cuts_hh", mcpEnergyTop, pTop, weight);
			histos->Fill2DHisto("track_momentum_vs_mcp_energy_bot_triggered_analyzable_event_and_pass_kinematic_cuts_hh", mcpEnergyBot, pBot, weight);

			histos->Fill1DHisto("motherPDG_top_triggered_analyzable_event_and_pass_kinematic_cuts_h", momPDGIDTop, weight);
			histos->Fill1DHisto("motherPDG_bot_triggered_analyzable_event_and_pass_kinematic_cuts_h", momPDGIDBot, weight);
			histos->Fill2DHisto("motherPDGTop_vs_motherPDGBot_triggered_analyzable_event_and_pass_kinematic_cuts_hh", momPDGIDBot, momPDGIDTop, weight);
	    }

		if (flag_triggered) {
			histos->Fill2DHisto("track_momentum_vs_mcp_energy_top_triggered_event_hh", mcpEnergyTop, pTop, weight);
			histos->Fill2DHisto("track_momentum_vs_mcp_energy_bot_triggered_event_hh", mcpEnergyBot, pBot, weight);

			histos->Fill1DHisto("motherPDG_top_triggered_event_h", momPDGIDTop, weight);
			histos->Fill1DHisto("motherPDG_bot_triggered_event_h", momPDGIDBot, weight);
			histos->Fill2DHisto("motherPDGTop_vs_motherPDGBot_triggered_event_hh", momPDGIDBot, momPDGIDTop, weight);
		}

		if(momPDGIDTop == 1 && momPDGIDBot == 1) flag_moller_truth = true;
	}

	if(flag_analyzable_event){
		if(flag_moller_truth && flag_triggered_analyzable_event_and_pass_kinematic_cuts)
			histos->Fill2DHisto("cut_flag_vs_truth_flag_hh", 1, 1, weight);
		else if(!flag_moller_truth && flag_triggered_analyzable_event_and_pass_kinematic_cuts)
			histos->Fill2DHisto("cut_flag_vs_truth_flag_hh", 0, 1, weight);
		else if(flag_moller_truth && !flag_triggered_analyzable_event_and_pass_kinematic_cuts)
			histos->Fill2DHisto("cut_flag_vs_truth_flag_hh", 1, 0, weight);
		else if(!flag_moller_truth && !flag_triggered_analyzable_event_and_pass_kinematic_cuts)
			histos->Fill2DHisto("cut_flag_vs_truth_flag_hh", 0, 0, weight);
	}


    if(flag_moller_truth && flag_triggered_analyzable_event_and_pass_kinematic_cuts){
		for(int i = 0; i < n_clusters_top_cut; i++){
			CalCluster cluster = clulsters_top_cut.at(i);

			std::vector<double> positionCluster = cluster.getPosition();
			histos->Fill2DHisto("xy_clusters_triggered_analyzable_event_and_pass_kinematic_cuts_and_true_Moller_hh",positionCluster[0], positionCluster[1], weight);

			CalHit* seed = (CalHit*)cluster.getSeed();
			histos->Fill1DHisto("seed_energy_cluster_triggered_analyzable_event_and_pass_kinematic_cuts_and_true_Moller_h", seed->getEnergy(), weight);
			histos->Fill1DHisto("energy_cluster_triggered_analyzable_event_and_pass_kinematic_cuts_and_true_Moller_h", cluster.getEnergy(), weight);
			histos->Fill1DHisto("n_hits_cluster_triggered_analyzable_event_and_pass_kinematic_cuts_and_true_Moller_h", cluster.getNHits(), weight);
			histos->Fill2DHisto("energy_vs_n_hits_cluster_triggered_analyzable_event_and_pass_kinematic_cuts_and_true_Moller_hh", cluster.getNHits(), cluster.getEnergy(), weight);

			int ix = seed -> getCrystalIndices()[0];
			if(ix < 0) ix++;
			int iy = seed -> getCrystalIndices()[1];

			histos->Fill2DHisto("xy_indices_clusters_triggered_analyzable_event_and_pass_kinematic_cuts_and_true_Moller_hh",ix, iy, weight);
			histos->Fill2DHisto("energy_vs_ix_clusters_triggered_analyzable_event_and_pass_kinematic_cuts_and_true_Moller_hh", ix, cluster.getEnergy(), weight);
			histos->Fill2DHisto("energy_vs_iy_clusters_triggered_analyzable_event_and_pass_kinematic_cuts_and_true_Moller_hh", iy, cluster.getEnergy(), weight);

		}

		for(int i = 0; i < n_clusters_bot_cut; i++){
			CalCluster cluster = clulsters_bot_cut.at(i);

			std::vector<double> positionCluster = cluster.getPosition();
			histos->Fill2DHisto("xy_clusters_triggered_analyzable_event_and_pass_kinematic_cuts_and_true_Moller_hh",positionCluster[0], positionCluster[1], weight);

			CalHit* seed = (CalHit*)cluster.getSeed();
			histos->Fill1DHisto("seed_energy_cluster_triggered_analyzable_event_and_pass_kinematic_cuts_and_true_Moller_h", seed->getEnergy(), weight);
			histos->Fill1DHisto("energy_cluster_triggered_analyzable_event_and_pass_kinematic_cuts_and_true_Moller_h", cluster.getEnergy(), weight);
			histos->Fill1DHisto("n_hits_cluster_triggered_analyzable_event_and_pass_kinematic_cuts_and_true_Moller_h", cluster.getNHits(), weight);
			histos->Fill2DHisto("energy_vs_n_hits_cluster_triggered_analyzable_event_and_pass_kinematic_cuts_and_true_Moller_hh", cluster.getNHits(), cluster.getEnergy(), weight);

			int ix = seed -> getCrystalIndices()[0];
			if(ix < 0) ix++;
			int iy = seed -> getCrystalIndices()[1];

			histos->Fill2DHisto("xy_indices_clusters_triggered_analyzable_event_and_pass_kinematic_cuts_and_true_Moller_hh",ix, iy, weight);
			histos->Fill2DHisto("energy_vs_ix_clusters_triggered_analyzable_event_and_pass_kinematic_cuts_and_true_Moller_hh", ix, cluster.getEnergy(), weight);
			histos->Fill2DHisto("energy_vs_iy_clusters_triggered_analyzable_event_and_pass_kinematic_cuts_and_true_Moller_hh", iy, cluster.getEnergy(), weight);
		}

	    for(int i = 0; i < n_vtxs; i++){
	        Vertex* vtx = vtxs_->at(i);

	        int n_entries = vtx->getParticles()->GetEntries();
	        if(n_entries != 2) {
	        	std::cout << "Warning: entries of Moller vertex is not 2." << std::endl;
	        	return false;
	        }

	        double invariant_mass = vtx->getInvMass();

	        Particle* particleTop = (Particle*)vtx->getParticles()->At(0);
	        Particle* particleBot = (Particle*)vtx->getParticles()->At(1);

	        std::vector<double> momTop =  particleTop->getMomentum();
	        std::vector<double> momBot =  particleBot->getMomentum();

	        double pSum = sqrt(pow(momTop[0], 2) + pow(momTop[1], 2) + pow(momTop[2], 2)) + sqrt(pow(momBot[0], 2) + pow(momBot[1], 2) + pow(momBot[2], 2));


	        histos->Fill1DHisto("invariant_mass_vertex_triggered_analyzable_events_with_kinematic_cuts_and_true_Moller_h", invariant_mass, weight);

	        histos->Fill1DHisto("pSum_vertex_triggered_analyzable_events_with_kinematic_cuts_and_true_Moller_h", pSum, weight);

	        histos->Fill2DHisto("invariant_mass_vs_pSum_vertex_triggered_analyzable_events_with_kinematic_cuts_and_true_Moller_hh", pSum, invariant_mass, weight);
	    }
    }
    else if(!flag_moller_truth && flag_triggered_analyzable_event_and_pass_kinematic_cuts){
	    for(int i = 0; i < n_vtxs; i++){
	        Vertex* vtx = vtxs_->at(i);

	        int n_entries = vtx->getParticles()->GetEntries();
	        if(n_entries != 2) {
	        	std::cout << "Warning: entries of Moller vertex is not 2." << std::endl;
	        	return false;
	        }

	        double invariant_mass = vtx->getInvMass();

	        Particle* particleTop = (Particle*)vtx->getParticles()->At(0);
	        Particle* particleBot = (Particle*)vtx->getParticles()->At(1);

	        std::vector<double> momTop =  particleTop->getMomentum();
	        std::vector<double> momBot =  particleBot->getMomentum();

	        double pTop = sqrt(pow(momTop[0], 2) + pow(momTop[1], 2) + pow(momTop[2], 2));
	        double pBot = sqrt(pow(momBot[0], 2) + pow(momBot[1], 2) + pow(momBot[2], 2));

	        double pSum = pTop + pBot;


	        histos->Fill1DHisto("invariant_mass_vertex_non_triggered_analyzable_events_with_kinematic_cuts_and_true_Moller_h", invariant_mass, weight);

	        histos->Fill1DHisto("pSum_vertex_non_triggered_analyzable_events_with_kinematic_cuts_and_true_Moller_h", pSum, weight);

	        histos->Fill2DHisto("invariant_mass_vs_pSum_vertex_non_triggered_analyzable_events_with_kinematic_cuts_and_true_Moller_hh", pSum, invariant_mass, weight);

	        histos->Fill2DHisto("pTop_vs_pBot_vertex_non_triggered_analyzable_events_with_kinematic_cuts_and_true_Moller_hh", pBot, pTop, weight);
	    }
    }

    return true;
}

void TriggerParametersExtractionMollerAnaProcessor::finalize() {
    outF_->cd();
    histos->saveHistos(outF_, anaName_.c_str());
    delete histos;
    histos = nullptr;

    TDirectory* dir{nullptr};
    dir = outF_->mkdir((anaName_+"_tuple").c_str());
    dir->cd();
    _reg_tuple->writeTree();

    outF_->Close();

}

DECLARE_PROCESSOR(TriggerParametersExtractionMollerAnaProcessor);
