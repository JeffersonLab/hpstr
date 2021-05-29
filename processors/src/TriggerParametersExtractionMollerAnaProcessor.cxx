/**
 *@file TriggerParametersExtractionAnaProcessor.cxx
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
}

bool TriggerParametersExtractionMollerAnaProcessor::process(IEvent* ievent) {
    double weight = 1.;

    // Apply tracks collection to filter events
    // Require available tracks with non-nan position at Ecal face and chi2/ndf less than threshold
    // Tracks are splits into four categories based on charge and position at Ecal face: pos at top, neg at top, pos at bot, neg at bot

	std::vector<Track> tracks_top;
	std::vector<Track> tracks_bot;

	tracks_top.clear();
	tracks_bot.clear();


/*

	for(int i = 0; i < n_tracks_top; i++){
		Track trackTop = tracks_top.at(i);
		std::vector<double> momTop= trackTop.getMomentum();
		TLorentzVector* lorentzVectorTop = new TLorentzVector();
		lorentzVectorTop->SetXYZM(momTop[0], momTop[1], momTop[2], ELECTRONMASS);
		double theta_top = lorentzVectorTop->Theta();
		double energy_top = lorentzVectorTop->Energy();

		for(int j = 0; j < n_tracks_bot; j++){
			Track trackBot = tracks_bot.at(i);
			std::vector<double> momBot= trackBot.getMomentum();
			TLorentzVector* lorentzVectorBot = new TLorentzVector();
			lorentzVectorBot->SetXYZM(momBot[0], momBot[1], momBot[2], ELECTRONMASS);
			double theta_bot = lorentzVectorBot->Theta();
			double energy_bot = lorentzVectorBot->Energy();

			histos->Fill2DHisto("energy_vs_theta_with_chi2_cut_hh",theta_top, energy_top, weight);
			histos->Fill2DHisto("energy_vs_theta_with_chi2_cut_hh",theta_bot, energy_bot, weight);

			histos->Fill2DHisto("thetaTop_vs_thetaBot_with_chi2_cut_hh",theta_bot, theta_top, weight);

			//std::cout << "IM: " << (*lorentzVectorTop + *lorentzVectorBot).M() << std::endl;


		}
	}
	*/

	int n_vtxs = vtxs_->size();

	histos->Fill1DHisto("n_vtxs_h", n_vtxs, weight);

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

		histos->Fill1DHisto("chi2ndf_tracks_top_h", trackTop.getChi2Ndf(), weight);
		histos->Fill1DHisto("chi2ndf_tracks_bot_h", trackBot.getChi2Ndf(), weight);

		std::vector<double> positionAtEcalTop = trackTop.getPositionAtEcal();
		std::vector<double> positionAtEcalBot = trackBot.getPositionAtEcal();

		histos->Fill2DHisto("xy_positionAtEcal_tracks_hh",positionAtEcalTop[0], positionAtEcalTop[1], weight);
		histos->Fill2DHisto("xy_positionAtEcal_tracks_hh",positionAtEcalBot[0], positionAtEcalBot[1], weight);

		std::vector<double> momTop;
		TLorentzVector* lorentzVectorTop = new TLorentzVector();
		double theta_top = -999;
		double energy_top = -999;

		std::vector<double> momBot;
		TLorentzVector* lorentzVectorBot = new TLorentzVector();
		double theta_bot = -999;
		double energy_bot = -999;

		std::cout << positionAtEcalTop[1] << "  " << positionAtEcalBot[1] << std::endl;

		std::cout << trackTop.getChi2Ndf() << "  " << trackBot.getChi2Ndf() << std::endl;

		if(!isnan(positionAtEcalTop[2]) && trackTop.getChi2Ndf() < CHI2NDFTHRESHOLD) {
			tracks_top.push_back(trackTop);

			momTop= trackTop.getMomentum();
			lorentzVectorTop = new TLorentzVector();
			lorentzVectorTop->SetXYZM(momTop[0], momTop[1], momTop[2], ELECTRONMASS);
			theta_top = lorentzVectorTop->Theta();
			energy_top = lorentzVectorTop->Energy();

			histos->Fill2DHisto("energy_vs_theta_with_chi2_cut_hh",theta_top, energy_top, weight);
		}

		if(!isnan(positionAtEcalBot[2]) && trackBot.getChi2Ndf() < CHI2NDFTHRESHOLD){
			tracks_bot.push_back(trackBot);

			momBot= trackBot.getMomentum();
			lorentzVectorBot = new TLorentzVector();
			lorentzVectorBot->SetXYZM(momBot[0], momBot[1], momBot[2], ELECTRONMASS);
			theta_bot = lorentzVectorBot->Theta();
			energy_bot = lorentzVectorBot->Energy();

			histos->Fill2DHisto("energy_vs_theta_with_chi2_cut_hh",theta_bot, energy_bot, weight);
		}

		histos->Fill2DHisto("thetaTop_vs_thetaBot_with_chi2_cut_hh",theta_bot, theta_top, weight);

		std::cout << "IM: " << (*lorentzVectorTop + *lorentzVectorBot).M() << std::endl;

		std::cout << momTop[0] << " " << momTop[1] << "  " << momTop[2] << std::endl;

		std::cout << momBot[0] << " " << momBot[1] << "  " << momBot[2] << std::endl;
    }

	int n_tracks_top = tracks_top.size();
	int n_tracks_bot = tracks_bot.size();

	histos->Fill1DHisto("n_tracks_top_h", n_tracks_top, weight);
	histos->Fill1DHisto("n_tracks_bot_h", n_tracks_bot, weight);


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
