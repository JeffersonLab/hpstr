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
#define MOMSCALE 1.761/2.09

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
        //vtxColl_ = parameters.getString("vtxColl",vtxColl_);

        selectionCfg_   = parameters.getString("vtxSelectionjson",selectionCfg_);
        histCfgFilename_      = parameters.getString("histCfg",histCfgFilename_);

        beamE_  = parameters.getDouble("beamE",beamE_);
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

}

void MollerAnaProcessor::initialize(TTree* tree) {
	_ah =  std::make_shared<AnaHelpers>();

    vtxSelector  = std::make_shared<BaseSelector>(anaName_+"_"+"vtxSelection",selectionCfg_);
    vtxSelector->setDebug(debug_);
    vtxSelector->LoadSelection();

    // init histos
    histos = new MollerAnaHistos(anaName_.c_str());
    histos->loadHistoConfig(histCfgFilename_);
    histos->DefineHistos();

    // init TTree
    tree_= tree;
    tree_->SetBranchAddress(tsColl_.c_str(), &tsData_ , &btsData_);
    tree_->SetBranchAddress(trkColl_.c_str() , &trks_, &btrks_);
    //tree_->SetBranchAddress(vtxColl_.c_str(), &vtxs_ , &bvtxs_);

    // Kinematic equations
    // E vs theta
    func_E_vs_theta_before_roation = new TF1("func_E_vs_theta_before_roation", "[0]/(1 + 2*[0]/[1]*sin(x/2.)*sin(x/2.))", 0, 1);
    func_E_vs_theta_before_roation->SetParameter(0, beamE_);
    func_E_vs_theta_before_roation->SetParameter(1, ELECTRONMASS);

    // theta1 vs theta2
    func_theta1_vs_theta2_before_roation = new TF1("func_theta1_vs_theta2_before_roation", "2*asin([1]/2./[0] * 1/sin(x/2.))", 0, 1);
    func_theta1_vs_theta2_before_roation->SetParameter(0, beamE_);
    func_theta1_vs_theta2_before_roation->SetParameter(1, ELECTRONMASS);
}

bool MollerAnaProcessor::process(IEvent* ievent) {
    double weight = 1.;


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
			histos->Fill1DHisto("chi2ndf_pos_top_h", track->getChi2Ndf(), weight);
			histos->Fill1DHisto("nHits_pos_top_h", track->getTrackerHitCount(), weight);
			histos->Fill1DHisto("p_pos_top_h", track->getP(), weight);
		}
		else if (charge == 1 && py < 0){
			tracks_pos_bot.push_back(*track);
			histos->Fill1DHisto("chi2ndf_pos_bot_h", track->getChi2Ndf(), weight);
			histos->Fill1DHisto("nHits_pos_bot_h", track->getTrackerHitCount(), weight);
			histos->Fill1DHisto("p_pos_bot_h", track->getP(), weight);
		}
		else if (charge == -1 && py > 0){
			tracks_neg_top.push_back(*track);
			histos->Fill1DHisto("chi2ndf_neg_top_h", track->getChi2Ndf(), weight);
			histos->Fill1DHisto("nHits_neg_top_h", track->getTrackerHitCount(), weight);
			histos->Fill1DHisto("p_neg_top_h", track->getP(), weight);
		}
		else if (charge == -1 && py < 0){
			tracks_neg_bot.push_back(*track);
			histos->Fill1DHisto("chi2ndf_neg_bot_h", track->getChi2Ndf(), weight);
			histos->Fill1DHisto("nHits_neg_bot_h", track->getTrackerHitCount(), weight);
			histos->Fill1DHisto("p_neg_bot_h", track->getP(), weight);
		}
	}


	int num_tracks_pos_top = tracks_pos_top.size();
	int num_tracks_pos_bot = tracks_pos_bot.size();
	int num_tracks_neg_top = tracks_neg_top.size();
	int num_tracks_neg_bot = tracks_neg_bot.size();


	histos->Fill1DHisto("num_tracks_pos_top_h", num_tracks_pos_top, weight);
	histos->Fill1DHisto("num_tracks_pos_bot_h", num_tracks_pos_bot, weight);
	histos->Fill1DHisto("num_tracks_neg_top_h", num_tracks_neg_top, weight);
	histos->Fill1DHisto("num_tracks_neg_bot_h", num_tracks_neg_bot, weight);

	vtxSelector->getCutFlowHisto()->Fill(0.,weight);


    if (!vtxSelector->passCutEq("num_tracks_pos_top_eq", num_tracks_pos_top,weight)){
        vtxSelector->clearSelector();
    	return true;
    }

    if (!vtxSelector->passCutEq("num_tracks_pos_bot_eq", num_tracks_pos_bot,weight)){
        vtxSelector->clearSelector();
    	return true;
    }

    if (!vtxSelector->passCutEq("num_tracks_neg_top_eq", num_tracks_neg_top,weight)){
        vtxSelector->clearSelector();
    	return true;
    }

    if (!vtxSelector->passCutEq("num_tracks_neg_bot_eq", num_tracks_neg_bot,weight)){
        vtxSelector->clearSelector();
    	return true;
    }


    Track track_neg_top = tracks_neg_top[0];
    Track track_neg_bot = tracks_neg_bot[0];

    if (!vtxSelector->passCutGt("num_hits_tracks_neg_top_gt", track_neg_top.getTrackerHitCount(),weight)){
        vtxSelector->clearSelector();
    	return true;
    }

    if (!vtxSelector->passCutGt("num_hits_tracks_neg_bot_gt", track_neg_bot.getTrackerHitCount(),weight)){
        vtxSelector->clearSelector();
    	return true;
    }


    double time_neg_top = track_neg_top.getTrackTime();
    double time_neg_bot = track_neg_bot.getTrackTime();
	histos->Fill1DHisto("time_tracks_neg_top_with_numTrakcs_numHits_cuts_h", time_neg_top, weight);
	histos->Fill1DHisto("time_tracks_neg_bot_with_numTrakcs_numHits_cuts_h", time_neg_bot, weight);
	histos->Fill2DHisto("time_tracks_neg_top_vs_bot_with_numTrakcs_numHits_cuts_hh", time_neg_top, time_neg_bot, weight);

    double time_diff = time_neg_top - time_neg_bot;
	histos->Fill1DHisto("time_diff_with_numTrakcs_numHits_cuts_h", time_diff, weight);

    if (!vtxSelector->passCutLt("time_diff_lt", fabs(time_diff),weight)){
        vtxSelector->clearSelector();
    	return true;
    }

    std::vector<double> mom_neg_top = track_neg_top.getMomentum();
    std::vector<double> mom_neg_bot = track_neg_bot.getMomentum();

    TLorentzVector* vect_neg_top = new TLorentzVector();
    vect_neg_top->SetXYZM(mom_neg_top[0], mom_neg_top[1], mom_neg_top[2], ELECTRONMASS);
    TLorentzVector* vect_neg_bot = new TLorentzVector();
    vect_neg_bot->SetXYZM(mom_neg_bot[0] * MOMSCALE, mom_neg_bot[1] * MOMSCALE, mom_neg_bot[2] * MOMSCALE, ELECTRONMASS);


    double p_neg_top = vect_neg_top->P();
    double p_neg_bot = vect_neg_bot->P();
    double pSum = p_neg_top + p_neg_bot;
    double pDiff = p_neg_top - p_neg_bot;
    double im = (*vect_neg_top + *vect_neg_bot).M();

	histos->Fill1DHisto("p_tracks_neg_top_with_numTrakcs_numHits_timeDiff_cuts_h", p_neg_top, weight);
	histos->Fill1DHisto("p_tracks_neg_bot_with_numTrakcs_numHits_timeDiff_cuts_h", p_neg_bot, weight);
	histos->Fill1DHisto("pDiff_with_numTrakcs_numHits_timeDiff_cuts_h", pDiff, weight);

	histos->Fill1DHisto("pSum_with_numTrakcs_numHits_timeDiff_cuts_h", pSum, weight);
	histos->Fill1DHisto("im_with_numTrakcs_numHits_timeDiff_cuts_h", im, weight);
	histos->Fill2DHisto("im_vs_pSum_with_numTrakcs_numHits_timeDiff_cuts_hh", pSum, im, weight);

    if (!vtxSelector->passCutLt("pTop_lt", p_neg_top, weight)){
        vtxSelector->clearSelector();
    	return true;
    }

    if (!vtxSelector->passCutLt("pBot_lt", p_neg_bot, weight)){
        vtxSelector->clearSelector();
    	return true;
    }

    if (!vtxSelector->passCutLt("pDiff_lt", fabs(pDiff), weight)){
        vtxSelector->clearSelector();
    	return true;
    }

	histos->Fill1DHisto("pSum_with_numTrakcs_numHits_timeDiff_p_pDiff_cuts_h", pSum, weight);
	histos->Fill1DHisto("im_with_numTrakcs_numHits_timeDiff_p_pDiff_cuts_h", im, weight);
	histos->Fill2DHisto("im_vs_pSum_with_numTrakcs_numHits_timeDiff_p_pDiff_cuts_hh", pSum, im, weight);

    if (!vtxSelector->passCutGt("pSum_gt", pSum, weight)){
        vtxSelector->clearSelector();
    	return true;
    }

    if (!vtxSelector->passCutLt("pSum_lt", pSum, weight)){
        vtxSelector->clearSelector();
    	return true;
    }

	histos->Fill1DHisto("im_with_numTrakcs_numHits_timeDiff_p_pDiff_pSum_cuts_h", im, weight);


	double px_neg_top_before_beam_rotation = vect_neg_top->Px() * cos(ROTATIONANGLEAROUNDY) - vect_neg_top->Pz() * sin(ROTATIONANGLEAROUNDY);
	double pz_neg_top_before_beam_rotation = vect_neg_top->Px() * sin(ROTATIONANGLEAROUNDY) + vect_neg_top->Pz() * cos(ROTATIONANGLEAROUNDY);
	double py_neg_top_before_beam_rotation = vect_neg_top->Py();

	double px_neg_bot_before_beam_rotation = vect_neg_bot->Px() * cos(ROTATIONANGLEAROUNDY) - vect_neg_bot->Pz() * sin(ROTATIONANGLEAROUNDY);
	double pz_neg_bot_before_beam_rotation = vect_neg_bot->Px() * sin(ROTATIONANGLEAROUNDY) + vect_neg_bot->Pz() * cos(ROTATIONANGLEAROUNDY);
	double py_neg_bot_before_beam_rotation = vect_neg_bot->Py();

	TLorentzVector* vector_neg_top_beam_rotation = new TLorentzVector();
	vector_neg_top_beam_rotation->SetXYZM(px_neg_top_before_beam_rotation, py_neg_top_before_beam_rotation, pz_neg_top_before_beam_rotation, ELECTRONMASS);

	TLorentzVector* vector_neg_bot_beam_rotation = new TLorentzVector();
	vector_neg_bot_beam_rotation->SetXYZM(px_neg_bot_before_beam_rotation, py_neg_bot_before_beam_rotation, pz_neg_bot_before_beam_rotation, ELECTRONMASS);

	double energy_top = vector_neg_top_beam_rotation->E();
	double energy_bot = vector_neg_top_beam_rotation->E();

	double theta_top_before_rotation = vector_neg_top_beam_rotation->Theta();
	double theta_bot_before_rotation = vector_neg_bot_beam_rotation->Theta();

	double energy_calcuated_top = func_E_vs_theta_before_roation->Eval(theta_top_before_rotation);
	double energy_calcuated_bot = func_E_vs_theta_before_roation->Eval(theta_bot_before_rotation);

	double theta_top_calculated_before_rotation = func_theta1_vs_theta2_before_roation->Eval(theta_bot_before_rotation);
	double theta_bot_calculated_before_rotation = func_theta1_vs_theta2_before_roation->Eval(theta_top_before_rotation);

	double energy_diff_top = energy_top - energy_calcuated_top;
	double energy_diff_bot = energy_bot - energy_calcuated_bot;
	double theta_diff_top = theta_top_before_rotation - theta_top_calculated_before_rotation;
	double theta_diff_bot = theta_bot_before_rotation - theta_bot_calculated_before_rotation;

	histos->Fill2DHisto("energy_vs_theta_top_with_numTrakcs_numHits_timeDiff_p_pDiff_pSum_cuts_hh",theta_top_before_rotation, energy_top, weight);
	histos->Fill2DHisto("energy_vs_theta_bot_with_numTrakcs_numHits_timeDiff_p_pDiff_pSum_cuts_hh",theta_bot_before_rotation, energy_bot, weight);

	histos->Fill2DHisto("thetaTop_vs_thetaBot_with_numTrakcs_numHits_timeDiff_p_pDiff_pSum_cuts_hh",theta_bot_before_rotation, theta_top_before_rotation, weight);

	histos->Fill1DHisto("diffE_top_with_numTrakcs_numHits_timeDiff_p_pDiff_pSum_cuts_h", energy_diff_top, weight);
	histos->Fill1DHisto("diffE_bot_with_numTrakcs_numHits_timeDiff_p_pDiff_pSum_cuts_h", energy_diff_bot, weight);

	histos->Fill1DHisto("diffTheta_top_with_numTrakcs_numHits_timeDiff_p_pDiff_pSum_cuts_h", theta_diff_top, weight);
	histos->Fill1DHisto("diffTheta_bot_with_numTrakcs_numHits_timeDiff_p_pDiff_pSum_cuts_h", theta_diff_bot, weight);

	histos->Fill2DHisto("pxSum_vs_pySum_with_numTrakcs_numHits_timeDiff_p_pDiff_pSum_cuts_hh", vector_neg_top_beam_rotation->Px() + vector_neg_bot_beam_rotation->Px(), vector_neg_top_beam_rotation->Py() + vector_neg_bot_beam_rotation->Py(), weight);

	histos->Fill2DHisto("diffETop_vs_diffEBot_with_numTrakcs_numHits_timeDiff_p_pDiff_pSum_cuts_hh", energy_diff_top, energy_diff_bot, weight);

	histos->Fill2DHisto("diffThetaTop_vs_diffThetaBot_with_numTrakcs_numHits_timeDiff_p_pDiff_pSum_cuts_hh", theta_diff_top, theta_diff_bot, weight);

    vtxSelector->clearSelector();
    return true;
}

void MollerAnaProcessor::finalize() {
    outF_->cd();
    histos->saveHistos(outF_, anaName_.c_str());
    delete histos;
    histos = nullptr;
    vtxSelector->getCutFlowHisto()->Write();

    outF_->Close();

}

DECLARE_PROCESSOR(MollerAnaProcessor);
