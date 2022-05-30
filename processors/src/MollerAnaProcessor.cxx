/**
 *@file mollerAnaProcessor.cxx
 *@author Tongtong, UNH
 */

#include "../include/MollerAnaProcessor.h"

#include <iostream>

#include "TF1.h"
#include "math.h"

#define ELECTRONMASS 0.000510998950 // GeV
#define PI 3.14159265358979
#define ROTATIONANGLEAROUNDY 0.0305 // rad

MollerAnaProcessor::MollerAnaProcessor(const std::string& name, Process& process) : Processor(name,process) {

}

MollerAnaProcessor::~MollerAnaProcessor(){}

void MollerAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring mollerAnaProcessor" <<std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        anaName_         = parameters.getString("anaName");

        trkColl_    = parameters.getString("trkColl");
        vtxColl_ = parameters.getString("vtxColl",vtxColl_);

        trackSelectionCfg_   = parameters.getString("trackSelectionjson",trackSelectionCfg_);
        histTrackCfgFilename_      = parameters.getString("histTrackCfg",histTrackCfgFilename_);

        vertexSelectionCfg_   = parameters.getString("vertexSelectionjson",vertexSelectionCfg_);
        histVertexCfgFilename_      = parameters.getString("histVertexCfg",histVertexCfgFilename_);

        beamE_  = parameters.getDouble("beamE",beamE_);
        isData_  = parameters.getInteger("isData",isData_);
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

}

void MollerAnaProcessor::initialize(TTree* tree) {
	_ah =  std::make_shared<AnaHelpers>();

	trackSelector  = std::make_shared<BaseSelector>(anaName_+"_"+"track",trackSelectionCfg_);
	trackSelector->setDebug(debug_);
	trackSelector->LoadSelection();

	vtxSelector  = std::make_shared<BaseSelector>(anaName_+"_"+"vertex",vertexSelectionCfg_);
	vtxSelector->setDebug(debug_);
	vtxSelector->LoadSelection();

    // init histos
	trackHistos = new MollerAnaHistos((anaName_+"_"+"track").c_str());
	trackHistos->loadHistoConfig(histTrackCfgFilename_);
	trackHistos->DefineHistos();

	vertexHistos = new MollerAnaHistos((anaName_+"_"+"vertex").c_str());
	vertexHistos->loadHistoConfig(histVertexCfgFilename_);
	vertexHistos->DefineHistos();

    // init TTree
    tree_= tree;
    tree_->SetBranchAddress(tsColl_.c_str(), &tsData_ , &btsData_);
    tree_->SetBranchAddress(trkColl_.c_str() , &trks_, &btrks_);
    tree_->SetBranchAddress(vtxColl_.c_str(), &vtxs_ , &bvtxs_);

    // Kinematic equations
    // E vs theta
    func_E_vs_theta_after_roation = new TF1("func_E_vs_theta_after_roation", "[0]/(1 + 2*[0]/[1]*sin(x/2.)*sin(x/2.))", 0, 1);
    func_E_vs_theta_after_roation->SetParameter(0, beamE_);
    func_E_vs_theta_after_roation->SetParameter(1, ELECTRONMASS);

    // theta1 vs theta2
    func_theta1_vs_theta2_after_roation = new TF1("func_theta1_vs_theta2_after_roation", "2*asin([1]/2./[0] * 1/sin(x/2.))", 0, 1);
    func_theta1_vs_theta2_after_roation->SetParameter(0, beamE_);
    func_theta1_vs_theta2_after_roation->SetParameter(1, ELECTRONMASS);


    /*
	// save a tree for information of tracks from vertices
	_reg_tracks_from_vertices = std::make_shared<FlatTupleMaker>(anaName_ + "_tracks_from_vertices");
	_reg_tracks_from_vertices->addVector("momTop");
	_reg_tracks_from_vertices->addVector("momBot");


	_reg_tracks_from_vertices->addVariable("chi2NdfTop");
	_reg_tracks_from_vertices->addVariable("chi2NdfBot");

	_reg_tracks_from_vertices->addVariable("timeTop");
	_reg_tracks_from_vertices->addVariable("timeBot");
	*/
}

bool MollerAnaProcessor::process(IEvent* ievent) {
    double weight = 1.;

    //////////// Track analysis ////////////

	std::vector<Track> tracks_pos_top;
	std::vector<Track> tracks_pos_bot;
	std::vector<Track> tracks_neg_top;
	std::vector<Track> tracks_neg_bot;

	tracks_pos_top.clear();
	tracks_pos_bot.clear();
	tracks_neg_top.clear();
	tracks_neg_bot.clear();

	for(int i = 0; i < trks_->size(); i++){
		Track* track = trks_->at(i);

		int charge = track->getCharge();
		double py = track->getMomentum()[1];

		if (charge == 1 && py > 0){
			tracks_pos_top.push_back(*track);
			trackHistos->Fill1DHisto("chi2ndf_pos_top_h", track->getChi2Ndf(), weight);
			trackHistos->Fill1DHisto("nHits_pos_top_h", track->getTrackerHitCount(), weight);
			trackHistos->Fill1DHisto("p_pos_top_h", track->getP(), weight);
		}
		else if (charge == 1 && py < 0){
			tracks_pos_bot.push_back(*track);
			trackHistos->Fill1DHisto("chi2ndf_pos_bot_h", track->getChi2Ndf(), weight);
			trackHistos->Fill1DHisto("nHits_pos_bot_h", track->getTrackerHitCount(), weight);
			trackHistos->Fill1DHisto("p_pos_bot_h", track->getP(), weight);
		}
		else if (charge == -1 && py > 0){
			tracks_neg_top.push_back(*track);
			trackHistos->Fill1DHisto("chi2ndf_neg_top_h", track->getChi2Ndf(), weight);
			trackHistos->Fill1DHisto("nHits_neg_top_h", track->getTrackerHitCount(), weight);
			trackHistos->Fill1DHisto("p_neg_top_h", track->getP(), weight);
		}
		else if (charge == -1 && py < 0){
			tracks_neg_bot.push_back(*track);
			trackHistos->Fill1DHisto("chi2ndf_neg_bot_h", track->getChi2Ndf(), weight);
			trackHistos->Fill1DHisto("nHits_neg_bot_h", track->getTrackerHitCount(), weight);
			trackHistos->Fill1DHisto("p_neg_bot_h", track->getP(), weight);
		}
	}


	int num_tracks_pos_top = tracks_pos_top.size();
	int num_tracks_pos_bot = tracks_pos_bot.size();
	int num_tracks_neg_top = tracks_neg_top.size();
	int num_tracks_neg_bot = tracks_neg_bot.size();

	trackHistos->Fill1DHisto("num_tracks_pos_top_h", num_tracks_pos_top, weight);
	trackHistos->Fill1DHisto("num_tracks_pos_bot_h", num_tracks_pos_bot, weight);
	trackHistos->Fill1DHisto("num_tracks_neg_top_h", num_tracks_neg_top, weight);
	trackHistos->Fill1DHisto("num_tracks_neg_bot_h", num_tracks_neg_bot, weight);

	for(int i = 0; i< num_tracks_neg_top; i++){
		Track track_neg_top = tracks_neg_top[i];
	    std::vector<double> mom_neg_top = track_neg_top.getMomentum();
		for(int j = 0; j< num_tracks_neg_bot; j++){
			Track track_neg_bot = tracks_neg_bot[j];
		    std::vector<double> mom_neg_bot = track_neg_bot.getMomentum();

		    TLorentzVector* vect_neg_top = new TLorentzVector();
		    vect_neg_top->SetXYZM(mom_neg_top[0], mom_neg_top[1], mom_neg_top[2], ELECTRONMASS);
		    TLorentzVector* vect_neg_bot = new TLorentzVector();
		    vect_neg_bot->SetXYZM(mom_neg_bot[0], mom_neg_bot[1], mom_neg_bot[2], ELECTRONMASS);


		    double p_neg_top = vect_neg_top->P();
		    double p_neg_bot = vect_neg_bot->P();
		    double pSum = p_neg_top + p_neg_bot;
		    double pDiff = p_neg_top - p_neg_bot;
		    double im = (*vect_neg_top + *vect_neg_bot).M();


			trackHistos->Fill1DHisto("pSum_no_cuts_h", pSum, weight);
			trackHistos->Fill1DHisto("im_no_cuts_h", im, weight);
			trackHistos->Fill2DHisto("im_vs_pSum_no_cuts_hh", pSum, im, weight);
		}
	}

	trackSelector->getCutFlowHisto()->Fill(0.,weight);


    if (!trackSelector->passCutEq("num_tracks_pos_top_eq", num_tracks_pos_top,weight)){
    	trackSelector->clearSelector();
    	return true;
    }

    if (!trackSelector->passCutEq("num_tracks_pos_bot_eq", num_tracks_pos_bot,weight)){
    	trackSelector->clearSelector();
    	return true;
    }

    if (!trackSelector->passCutEq("num_tracks_neg_top_eq", num_tracks_neg_top,weight)){
    	trackSelector->clearSelector();
    	return true;
    }

    if (!trackSelector->passCutEq("num_tracks_neg_bot_eq", num_tracks_neg_bot,weight)){
    	trackSelector->clearSelector();
    	return true;
    }

    Track track_neg_top = tracks_neg_top[0];
    Track track_neg_bot = tracks_neg_bot[0];

    if (!trackSelector->passCutLt("chi2ndf_tracks_neg_top_lt", track_neg_top.getChi2Ndf(),weight)){
    	trackSelector->clearSelector();
    	return true;
    }

    if (!trackSelector->passCutLt("chi2ndf_tracks_neg_bot_lt", track_neg_bot.getChi2Ndf(),weight)){
    	trackSelector->clearSelector();
    	return true;
    }


    if (!trackSelector->passCutGt("num_hits_tracks_neg_top_gt", track_neg_top.getTrackerHitCount(),weight)){
    	trackSelector->clearSelector();
    	return true;
    }

    if (!trackSelector->passCutGt("num_hits_tracks_neg_bot_gt", track_neg_bot.getTrackerHitCount(),weight)){
    	trackSelector->clearSelector();
    	return true;
    }

    std::vector<double> mom_neg_top = track_neg_top.getMomentum();
    std::vector<double> mom_neg_bot = track_neg_bot.getMomentum();

    TLorentzVector* vect_neg_top = new TLorentzVector();
    vect_neg_top->SetXYZM(mom_neg_top[0], mom_neg_top[1], mom_neg_top[2], ELECTRONMASS);
    TLorentzVector* vect_neg_bot = new TLorentzVector();
    vect_neg_bot->SetXYZM(mom_neg_bot[0], mom_neg_bot[1], mom_neg_bot[2], ELECTRONMASS);


    double p_neg_top = vect_neg_top->P();
    double p_neg_bot = vect_neg_bot->P();
    double pSum = p_neg_top + p_neg_bot;
    double pDiff = p_neg_top - p_neg_bot;
    double pxSum = vect_neg_top->Px() + vect_neg_bot->Px();
    double pySum = vect_neg_top->Py() + vect_neg_bot->Py();
    double pzSum = vect_neg_top->Pz() + vect_neg_bot->Pz();
    double im = (*vect_neg_top + *vect_neg_bot).M();

	trackHistos->Fill1DHisto("pSum_with_numTracks_cuts_h", pSum, weight);
	trackHistos->Fill1DHisto("im_with_numTracks_cuts_h", im, weight);
	trackHistos->Fill2DHisto("im_vs_pSum_with_numTracks_cuts_hh", pSum, im, weight);


    double time_neg_top = track_neg_top.getTrackTime();
    double time_neg_bot = track_neg_bot.getTrackTime();

	trackHistos->Fill1DHisto("time_tracks_neg_top_with_numTracks_chi2ndf_numHits_cuts_h", time_neg_top, weight);
	trackHistos->Fill1DHisto("time_tracks_neg_bot_with_numTracks_chi2ndf_numHits_cuts_h", time_neg_bot, weight);
	trackHistos->Fill2DHisto("time_tracks_neg_top_vs_bot_with_numTracks_chi2ndf_numHits_cuts_hh", time_neg_top, time_neg_bot, weight);

    double time_diff = time_neg_top - time_neg_bot;
	trackHistos->Fill1DHisto("time_diff_with_numTracks_chi2ndf_numHits_cuts_h", time_diff, weight);

	if (!trackSelector->passCutLt("time_diff_lt", fabs(time_diff),weight)){
		trackSelector->clearSelector();
		return true;
	}

	trackHistos->Fill1DHisto("p_tracks_neg_top_with_numTracks_chi2ndf_numHits_timeDiff_cuts_h", p_neg_top, weight);
	trackHistos->Fill1DHisto("p_tracks_neg_bot_with_numTracks_chi2ndf_numHits_timeDiff_cuts_h", p_neg_bot, weight);
	trackHistos->Fill1DHisto("pDiff_with_numTracks_chi2ndf_numHits_timeDiff_cuts_h", pDiff, weight);

	trackHistos->Fill1DHisto("pSum_with_numTracks_chi2ndf_numHits_timeDiff_cuts_h", pSum, weight);
	trackHistos->Fill1DHisto("im_with_numTracks_chi2ndf_numHits_timeDiff_cuts_h", im, weight);
	trackHistos->Fill2DHisto("im_vs_pSum_with_numTracks_chi2ndf_numHits_timeDiff_cuts_hh", pSum, im, weight);

    if (!trackSelector->passCutLt("pTop_lt", p_neg_top, weight)){
    	trackSelector->clearSelector();
    	return true;
    }

    if (!trackSelector->passCutLt("pBot_lt", p_neg_bot, weight)){
    	trackSelector->clearSelector();
    	return true;
    }

    if (!trackSelector->passCutLt("pDiff_lt", fabs(pDiff), weight)){
    	trackSelector->clearSelector();
    	return true;
    }

	trackHistos->Fill1DHisto("pSum_with_numTracks_chi2ndf_numHits_timeDiff_p_pDiff_cuts_h", pSum, weight);
	trackHistos->Fill1DHisto("im_with_numTracks_chi2ndf_numHits_timeDiff_p_pDiff_cuts_h", im, weight);
	trackHistos->Fill2DHisto("im_vs_pSum_with_numTracks_chi2ndf_numHits_timeDiff_p_pDiff_cuts_hh", pSum, im, weight);

/*
	_reg_tracks_from_vertices->addToVector("momTop", mom_neg_top[0]);
	_reg_tracks_from_vertices->addToVector("momTop", mom_neg_top[1]);
	_reg_tracks_from_vertices->addToVector("momTop", mom_neg_top[2]);

	_reg_tracks_from_vertices->addToVector("momBot", mom_neg_bot[0]);
	_reg_tracks_from_vertices->addToVector("momBot", mom_neg_bot[1]);
	_reg_tracks_from_vertices->addToVector("momBot", mom_neg_bot[2]);

	_reg_tracks_from_vertices->setVariableValue("chi2NdfTop", track_neg_top.getChi2Ndf());
	_reg_tracks_from_vertices->setVariableValue("chi2NdfBot", track_neg_bot.getChi2Ndf());

    _reg_tracks_from_vertices->setVariableValue("timeTop", time_neg_top);
    _reg_tracks_from_vertices->setVariableValue("timeBot", time_neg_bot);

    _reg_tracks_from_vertices->fill();
*/
    if (!trackSelector->passCutGt("pSum_gt", pSum, weight)){
    	trackSelector->clearSelector();
    	return true;
    }

    if (!trackSelector->passCutLt("pSum_lt", pSum, weight)){
    	trackSelector->clearSelector();
    	return true;
    }

	trackHistos->Fill1DHisto("p_tracks_neg_top_with_all_cuts_h", p_neg_top, weight);
	trackHistos->Fill1DHisto("p_tracks_neg_bot_with_all_cuts_h", p_neg_bot, weight);

	trackHistos->Fill1DHisto("pxSum_with_all_cuts_h", pxSum, weight);
	trackHistos->Fill1DHisto("pySum_with_all_cuts_h", pySum, weight);
	trackHistos->Fill1DHisto("pzSum_with_all_cuts_h", pzSum, weight);

	trackHistos->Fill1DHisto("pSum_with_all_cuts_h", pSum, weight);
	trackHistos->Fill1DHisto("im_with_all_cuts_h", im, weight);
	trackHistos->Fill2DHisto("im_vs_pSum_with_all_cuts_hh", pSum, im, weight);


	double px_neg_top_after_beam_rotation = vect_neg_top->Px() * cos(ROTATIONANGLEAROUNDY) - vect_neg_top->Pz() * sin(ROTATIONANGLEAROUNDY);
	double pz_neg_top_after_beam_rotation = vect_neg_top->Px() * sin(ROTATIONANGLEAROUNDY) + vect_neg_top->Pz() * cos(ROTATIONANGLEAROUNDY);
	double py_neg_top_after_beam_rotation = vect_neg_top->Py();

	double px_neg_bot_after_beam_rotation = vect_neg_bot->Px() * cos(ROTATIONANGLEAROUNDY) - vect_neg_bot->Pz() * sin(ROTATIONANGLEAROUNDY);
	double pz_neg_bot_after_beam_rotation = vect_neg_bot->Px() * sin(ROTATIONANGLEAROUNDY) + vect_neg_bot->Pz() * cos(ROTATIONANGLEAROUNDY);
	double py_neg_bot_after_beam_rotation = vect_neg_bot->Py();

	TLorentzVector* vector_neg_top_beam_rotation = new TLorentzVector();
	vector_neg_top_beam_rotation->SetXYZM(px_neg_top_after_beam_rotation, py_neg_top_after_beam_rotation, pz_neg_top_after_beam_rotation, ELECTRONMASS);

	TLorentzVector* vector_neg_bot_beam_rotation = new TLorentzVector();
	vector_neg_bot_beam_rotation->SetXYZM(px_neg_bot_after_beam_rotation, py_neg_bot_after_beam_rotation, pz_neg_bot_after_beam_rotation, ELECTRONMASS);

	double energy_top = vector_neg_top_beam_rotation->E();
	double energy_bot = vector_neg_bot_beam_rotation->E();

	double theta_top_after_rotation = vector_neg_top_beam_rotation->Theta();
	double theta_bot_after_rotation = vector_neg_bot_beam_rotation->Theta();

	double energy_calcuated_top = func_E_vs_theta_after_roation->Eval(theta_top_after_rotation);
	double energy_calcuated_bot = func_E_vs_theta_after_roation->Eval(theta_bot_after_rotation);

	double theta_top_calculated_after_rotation = func_theta1_vs_theta2_after_roation->Eval(theta_bot_after_rotation);
	double theta_bot_calculated_after_rotation = func_theta1_vs_theta2_after_roation->Eval(theta_top_after_rotation);

	double energy_diff_top = energy_top - energy_calcuated_top;
	double energy_diff_bot = energy_bot - energy_calcuated_bot;
	double theta_diff_top = theta_top_after_rotation - theta_top_calculated_after_rotation;
	double theta_diff_bot = theta_bot_after_rotation - theta_bot_calculated_after_rotation;

	trackHistos->Fill2DHisto("energy_vs_theta_top_with_all_cuts_hh",theta_top_after_rotation, energy_top, weight);
	trackHistos->Fill2DHisto("energy_vs_theta_bot_with_all_cuts_hh",theta_bot_after_rotation, energy_bot, weight);

	trackHistos->Fill2DHisto("thetaTop_vs_thetaBot_with_all_cuts_hh",theta_bot_after_rotation, theta_top_after_rotation, weight);

	trackHistos->Fill1DHisto("diffE_top_with_all_cuts_h", energy_diff_top, weight);
	trackHistos->Fill1DHisto("diffE_bot_with_all_cuts_h", energy_diff_bot, weight);

	trackHistos->Fill1DHisto("diffTheta_top_with_all_cuts_h", theta_diff_top, weight);
	trackHistos->Fill1DHisto("diffTheta_bot_with_all_cuts_h", theta_diff_bot, weight);

	trackHistos->Fill2DHisto("pxSum_vs_pySum_with_all_cuts_hh", vector_neg_top_beam_rotation->Px() + vector_neg_bot_beam_rotation->Px(), vector_neg_top_beam_rotation->Py() + vector_neg_bot_beam_rotation->Py(), weight);

	trackHistos->Fill2DHisto("diffETop_vs_diffEBot_with_all_cuts_hh", energy_diff_top, energy_diff_bot, weight);

	trackHistos->Fill2DHisto("diffThetaTop_vs_diffThetaBot_with_all_cuts_hh", theta_diff_top, theta_diff_bot, weight);


	trackSelector->clearSelector();


    //////////// Vertex analysis ////////////
	int n_vtxs = vtxs_->size();
	//std::cout << n_vtxs << std::endl;

	// Loop over vertices in event and make selections
	for (int i_vtx = 0; i_vtx < n_vtxs; i_vtx++) {
		vtxSelector->getCutFlowHisto()->Fill(0., weight);

		Vertex* vtx = vtxs_->at(i_vtx);
		Particle* top_part = nullptr;
		Track* top_trk = nullptr;
		Particle* bot_part = nullptr;
		Track* bot_trk = nullptr;

		bool foundParts = _ah->GetParticlesFromVtx(vtx, top_part, bot_part);
		if (!foundParts) {
			if (debug_)
				std::cout
						<< "VertexAnaProcessor::WARNING::Found vtx without ele/pos. Skip."
						<< std::endl;
			continue;
		}

		if (!trkColl_.empty()) {
			bool foundTracks = _ah->MatchToGBLTracks((top_part->getTrack()).getID(),
					(bot_part->getTrack()).getID(), top_trk, bot_trk, *trks_);

			if (!foundTracks) {
				if (debug_)
					std::cout
							<< "VertexAnaProcessor::ERROR couldn't find ele/pos in the GBLTracks collection"
							<< std::endl;
				continue;
			}
		} else {
			top_trk = (Track*) top_part->getTrack().Clone();
			bot_trk = (Track*) bot_part->getTrack().Clone();
		}

		double top_E = top_part->getEnergy();
		double bot_E = bot_part->getEnergy();

		CalCluster top_clus = top_part->getCluster();
		CalCluster bot_clus = bot_part->getCluster();

		std::cout << top_clus.getPosition()[1] << "  " << bot_clus.getPosition()[1] << std::endl;
	}


    return true;
}

void MollerAnaProcessor::finalize() {
    outF_->cd();

    std::string dirNameTrack = anaName_+"_"+"track";
    trackHistos->saveHistos(outF_,dirNameTrack);
    outF_->cd(dirNameTrack.c_str());
    trackSelector->getCutFlowHisto()->Write();
    //_reg_tracks_from_vertices->writeTree();

    outF_->Close();
}

DECLARE_PROCESSOR(MollerAnaProcessor);
