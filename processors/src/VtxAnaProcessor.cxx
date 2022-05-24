/**
 *@file VtxAnaProcessor.cxx
 *@author Tongtong, UNH
 */

#include "VtxAnaProcessor.h"
#include "TMatrixD.h"
#include "TMath.h"

#include <iostream>
#include <stdio.h>

VtxAnaProcessor::VtxAnaProcessor(const std::string& name, Process& process) :
		Processor(name, process) {

}

//TODO Check this destructor

VtxAnaProcessor::~VtxAnaProcessor() {
}

void VtxAnaProcessor::configure(const ParameterSet& parameters) {
	std::cout << "Configuring VertexAnaProcessor" << std::endl;
	try {
		debug_ = parameters.getInteger("debug");
		anaName_ = parameters.getString("anaName");
		histCfgFilename_ = parameters.getString("histCfg", histCfgFilename_);
		vtxColl_ = parameters.getString("vtxColl", vtxColl_);
		tcvtxColl_ = parameters.getString("tcvtxColl", tcvtxColl_);
		beamE_ = parameters.getDouble("beamE", beamE_);
	} catch (std::runtime_error& error) {
		std::cout << error.what() << std::endl;
	}

    selectionCfg_   = parameters.getString("vtxSelectionjson",selectionCfg_);

	_ah = std::make_shared<AnaHelpers>();

    timeOffset_ = parameters.getDouble("CalTimeOffset",timeOffset_);
    beamE_  = parameters.getDouble("beamE",beamE_);
    isData_  = parameters.getInteger("isData",isData_);
}

void VtxAnaProcessor::initialize(TTree* tree) {
	tree_ = tree;
	// init histos
	histos = new VtxAnaHistos(anaName_.c_str());
	histos->loadHistoConfig(histCfgFilename_);
	histos->DefineHistos();

    // Vertex selector
    vtxSelector  = std::make_shared<BaseSelector>(anaName_+"_"+"vtxSelection",selectionCfg_);
    vtxSelector->setDebug(debug_);
    vtxSelector->LoadSelection();

	// init TTree
	tree_->SetBranchAddress(tsColl_.c_str(), &tsData_, &btsData_);
    tree_->SetBranchAddress(trkColl_.c_str(),&trks_, &btrks_);
	tree_->SetBranchAddress(vtxColl_.c_str(), &vtxs_, &bvtxs_);
	tree_->SetBranchAddress(tcvtxColl_.c_str(), &tcvtxs_, &btcvtxs_);

    //Save tuple variables for vertices
    treeVertex = std::make_shared<FlatTupleMaker>("tuple");

    treeVertex->addVariable("chi2NdfEle");
    treeVertex->addVariable("chi2NdfPos");
    treeVertex->addVariable("chi2UCVertex");
}

bool VtxAnaProcessor::process(IEvent* ievent) {
	histos->FillTSData(tsData_);
	histos->FillUnconstrainedV0s(vtxs_);

	double weight = 1.;

	int n_vtxs = vtxs_->size();

	// Loop over vertices in event and make selections
	for (int i_vtx = 0; i_vtx < n_vtxs; i_vtx++) {
		vtxSelector->getCutFlowHisto()->Fill(0., weight);

		Vertex* vtx = vtxs_->at(i_vtx);
		Particle* ele = nullptr;
		Track* ele_trk = nullptr;
		Particle* pos = nullptr;
		Track* pos_trk = nullptr;

		bool foundParts = _ah->GetParticlesFromVtx(vtx, ele, pos);
		if (!foundParts) {
			if (debug_)
				std::cout
						<< "VertexAnaProcessor::WARNING::Found vtx without ele/pos. Skip."
						<< std::endl;
			continue;
		}

		if (!trkColl_.empty()) {
			bool foundTracks = _ah->MatchToGBLTracks((ele->getTrack()).getID(),
					(pos->getTrack()).getID(), ele_trk, pos_trk, *trks_);

			if (!foundTracks) {
				if (debug_)
					std::cout
							<< "VertexAnaProcessor::ERROR couldn't find ele/pos in the GBLTracks collection"
							<< std::endl;
				continue;
			}
		} else {
			ele_trk = (Track*) ele->getTrack().Clone();
			pos_trk = (Track*) pos->getTrack().Clone();
		}

		double ele_E = ele->getEnergy();
		double pos_E = pos->getEnergy();

		CalCluster eleClus = ele->getCluster();
		CalCluster posClus = pos->getCluster();

		//Compute analysis variables here.
		TLorentzVector p_ele;
		p_ele.SetPxPyPzE(ele_trk->getMomentum()[0], ele_trk->getMomentum()[1],
				ele_trk->getMomentum()[2], ele->getEnergy());
		TLorentzVector p_pos;
		p_pos.SetPxPyPzE(pos_trk->getMomentum()[0], pos_trk->getMomentum()[1],
				pos_trk->getMomentum()[2], ele->getEnergy());

		//Tracks in opposite volumes - useless
		//if (!vtxSelector->passCutLt("eleposTanLambaProd_lt",ele_trk->getTanLambda() * pos_trk->getTanLambda(),weight))
		//  continue;

		//TS: single3 fired
		if (!vtxSelector->passCutEq("single3_eq",
				tsData_->prescaled.Single_3_Top
						|| tsData_->prescaled.Single_3_Bot, weight))
			continue;

		//Ele Track-cluster match
		if (!vtxSelector->passCutLt("eleTrkCluMatch_lt",
				ele->getGoodnessOfPID(), weight))
			continue;

		//Pos Track-cluster match
		if (!vtxSelector->passCutLt("posTrkCluMatch_lt",
				pos->getGoodnessOfPID(), weight))
			continue;

		//Require Positron Cluster exists
		if (!vtxSelector->passCutGt("posClusE_gt", posClus.getEnergy(), weight))
			continue;

		//Require Positron Cluster does NOT exists
		if (!vtxSelector->passCutLt("posClusE_lt", posClus.getEnergy(), weight))
			continue;

		double corr_eleClusterTime = ele->getCluster().getTime() - timeOffset_;
		double corr_posClusterTime = pos->getCluster().getTime() - timeOffset_;

		double botClusTime = 0.0;
		if (ele->getCluster().getPosition().at(1) < 0.0)
			botClusTime = ele->getCluster().getTime();
		else
			botClusTime = pos->getCluster().getTime();

		//Bottom Cluster Time
		if (!vtxSelector->passCutLt("botCluTime_lt", botClusTime, weight))
			continue;

		if (!vtxSelector->passCutGt("botCluTime_gt", botClusTime, weight))
			continue;

		//Ele Pos Cluster Time Difference
		if (!vtxSelector->passCutLt("eleposCluTimeDiff_lt",
				fabs(corr_eleClusterTime - corr_posClusterTime), weight))
			continue;

		//Ele Track-Cluster Time Difference
		if (!vtxSelector->passCutLt("eleTrkCluTimeDiff_lt",
				fabs(ele_trk->getTrackTime() - corr_eleClusterTime), weight))
			continue;

		//Pos Track-Cluster Time Difference
		if (!vtxSelector->passCutLt("posTrkCluTimeDiff_lt",
				fabs(pos_trk->getTrackTime() - corr_posClusterTime), weight))
			continue;

		TVector3 ele_mom;
		ele_mom.SetX(ele_trk->getMomentum()[0]);
		ele_mom.SetY(ele_trk->getMomentum()[1]);
		ele_mom.SetZ(ele_trk->getMomentum()[2]);

		TVector3 pos_mom;
		pos_mom.SetX(pos_trk->getMomentum()[0]);
		pos_mom.SetY(pos_trk->getMomentum()[1]);
		pos_mom.SetZ(pos_trk->getMomentum()[2]);

		//Beam Electron cut
		if (!vtxSelector->passCutLt("eleMom_lt", ele_mom.Mag(), weight))
			continue;

		//Ele min momentum cut
		if (!vtxSelector->passCutGt("eleMom_gt", ele_mom.Mag(), weight))
			continue;

		//Pos min momentum cut
		if (!vtxSelector->passCutGt("posMom_gt", pos_mom.Mag(), weight))
			continue;

		//Ele nHits
		int ele2dHits = ele_trk->getTrackerHitCount();
		if (!ele_trk->isKalmanTrack())
			ele2dHits *= 2;

		if (!vtxSelector->passCutGt("eleN2Dhits_gt", ele2dHits, weight)) {
			continue;
		}

		//Pos nHits
		int pos2dHits = pos_trk->getTrackerHitCount();
		if (!pos_trk->isKalmanTrack())
			pos2dHits *= 2;

		if (!vtxSelector->passCutGt("posN2Dhits_gt", pos2dHits, weight)) {
			continue;
		}

		//Less than 4 shared hits for ele/pos track
		if (!vtxSelector->passCutLt("eleNshared_lt", ele_trk->getNShared(),
				weight)) {
			continue;
		}

		if (!vtxSelector->passCutLt("posNshared_lt", pos_trk->getNShared(),
				weight)) {
			continue;
		}

		//Max vtx momentum
		if (!vtxSelector->passCutLt("maxVtxMom_lt", (ele_mom + pos_mom).Mag(),
				weight))
			continue;

		//Min vtx momentum

		if (!vtxSelector->passCutGt("minVtxMom_gt", (ele_mom + pos_mom).Mag(),
				weight))
			continue;

		vtxSelector->clearSelector();

		treeVertex->setVariableValue("chi2NdfEle", ele_trk->getChi2Ndf());
		treeVertex->setVariableValue("chi2NdfPos", pos_trk->getChi2Ndf());
		treeVertex->setVariableValue("chi2UCVertex", vtx->getChi2());

		treeVertex->fill();
	}

	return true;
}

void VtxAnaProcessor::finalize() {
	outF_->cd();
	histos->saveHistos(outF_, anaName_.c_str());
	vtxSelector->getCutFlowHisto()->Write();
	delete histos;
	histos = nullptr;

	TDirectory* dir = outF_->mkdir("treeVertex");
    dir->cd();
    treeVertex->writeTree();

    outF_->Close();

}

DECLARE_PROCESSOR(VtxAnaProcessor);
