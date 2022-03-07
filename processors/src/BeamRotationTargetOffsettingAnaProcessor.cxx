/**
 * @author Tongtong Cao, UNH
 */
#include "BeamRotationTargetOffsettingAnaProcessor.h"
#include <iostream>

#include "TLorentzVector.h"

#define ELECTRONMASS 0.000510998950 // GeV
#define GOODNESSCUT 10
#define TRACKCHI2NDFCUT 6
#define NHITSCUT 10
#define TIMEDIFFCUT 4 // ns
#define TRACKMOMMAXCUT 4.55*0.75 // GeV

BeamRotationTargetOffsettingAnaProcessor::BeamRotationTargetOffsettingAnaProcessor(const std::string& name, Process& process) : Processor(name,process){}
//TODO CHECK THIS DESTRUCTOR
BeamRotationTargetOffsettingAnaProcessor::~BeamRotationTargetOffsettingAnaProcessor(){}

void BeamRotationTargetOffsettingAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring BeamRotationTargetOffsettingAnaProcessor" << std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        anaName_         = parameters.getString("anaName");

        tsColl_     = parameters.getString("TSColl");

        histCfgFilename_ = parameters.getString("histCfg");

        fspCollRoot_    = parameters.getString("fspCollRoot");
        histCfgFilenameParticle_ = parameters.getString("histCfgParticle");

        trkColl_ = parameters.getString("trkColl",trkColl_);
        vtxColl_     = parameters.getString("vtxColl");
        histCfgFilenameVertex_ = parameters.getString("histCfgVertex");

        ecalClusColl_    = parameters.getString("ecalClusColl");

        selectionCfg_   = parameters.getString("vtxSelectionjson",selectionCfg_);

        timeOffset_ = parameters.getDouble("CalTimeOffset",timeOffset_);
        beamE_  = parameters.getDouble("beamE",beamE_);
        isData_  = parameters.getInteger("isData",isData_);

    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

}

void BeamRotationTargetOffsettingAnaProcessor::initialize(TTree* tree) {
    _ah =  std::make_shared<AnaHelpers>();

    tree_= tree;
    // init TTree for TS
    tree_->SetBranchAddress(tsColl_.c_str(), &tsData_ , &btsData_);
    tree_->SetBranchAddress(fspCollRoot_.c_str() , &fsps_, &bfsps_);
    tree_->SetBranchAddress(trkColl_.c_str(),&trks_, &btrks_);
    tree_->SetBranchAddress(vtxColl_.c_str()  , &vtxs_    , &bvtxs_    );
    tree_->SetBranchAddress(ecalClusColl_.c_str() , &ecalClusters_, &becalClusters_);


    // init histos
    histos = new RecoParticleAnaHistos(anaName_.c_str());
    histos->loadHistoConfig(histCfgFilename_);
    histos->DefineHistos();

    // init histos for final-state particle
    histosParticle = new RecoParticleAnaHistos("recoParticleAna");
    histosParticle->loadHistoConfig(histCfgFilenameParticle_);
    histosParticle->DefineHistos();

    // init histos for track and vertex
    histosVertex = new RecoTrackVertexAnaHistos("recoVertexAna");
    histosVertex->loadHistoConfig(histCfgFilenameVertex_);
    histosVertex->doTrackComparisonPlots(false);
    histosVertex->DefineHistos();

    //Save tuple variables for events with three final-state particles
    treeThreeFSPs = std::make_shared<FlatTupleMaker>("tuple");

    treeThreeFSPs->addVariable("posN2DHits");
    treeThreeFSPs->addVariable("ele1N2DHits");
    treeThreeFSPs->addVariable("ele2N2DHits");

    treeThreeFSPs->addVariable("posTime");
    treeThreeFSPs->addVariable("ele1Time");
    treeThreeFSPs->addVariable("ele2Time");

    treeThreeFSPs->addVariable("energySum");
    treeThreeFSPs->addVariable("pxSum");
    treeThreeFSPs->addVariable("pySum");
    treeThreeFSPs->addVariable("pzSum");
    treeThreeFSPs->addVariable("invariantMass");

    treeThreeFSPs->addVariable("chi2Vertex");
    treeThreeFSPs->addVariable("pxVertex");
    treeThreeFSPs->addVariable("pyVertex");
    treeThreeFSPs->addVariable("pzVertex");
    treeThreeFSPs->addVariable("imVertex");
    treeThreeFSPs->addVariable("xVertex");
    treeThreeFSPs->addVariable("yVertex");
    treeThreeFSPs->addVariable("zVertex");
    treeThreeFSPs->addVariable("chi2ndfEleVertex");
    treeThreeFSPs->addVariable("pxEleVertex");
    treeThreeFSPs->addVariable("pyEleVertex");
    treeThreeFSPs->addVariable("pzEleVertex");
    treeThreeFSPs->addVariable("chi2ndfPosVertex");
    treeThreeFSPs->addVariable("pxPosVertex");
    treeThreeFSPs->addVariable("pyPosVertex");
    treeThreeFSPs->addVariable("pzPosVertex");

    //Save tuple variables for vertices
    treeVertex = std::make_shared<FlatTupleMaker>("tuple");

    treeVertex->addVariable("xVertex");
    treeVertex->addVariable("yVertex");
    treeVertex->addVariable("zVertex");

    // Vertex selector
    vtxSelector  = std::make_shared<BaseSelector>(anaName_+"_"+"vtxSelection",selectionCfg_);
    vtxSelector->setDebug(debug_);
    vtxSelector->LoadSelection();

}

bool BeamRotationTargetOffsettingAnaProcessor::process(IEvent* ievent) {
	double weight = 1.;

	/****** Beam momentum rotation study by events with three final-state particles ******/
	// Single3 is fired
	// Have and only have 3 Ecal clusters
	int n_ecalClusts = ecalClusters_->size();
    if((tsData_->prescaled.Single_3_Top == true || tsData_->prescaled.Single_3_Bot == true) && n_ecalClusts == 3){
    	CalCluster* ecalClus1 = ecalClusters_->at(0);
    	CalCluster* ecalClus2 = ecalClusters_->at(1);
    	CalCluster* ecalClus3 = ecalClusters_->at(2);

    	CalHit* seed1 = (CalHit*)ecalClus1->getSeed();
    	CalHit* seed2 = (CalHit*)ecalClus2->getSeed();
    	CalHit* seed3 = (CalHit*)ecalClus3->getSeed();


    	std::vector<int> ind1 = seed1->getCrystalIndices();
    	std::vector<int> ind2 = seed2->getCrystalIndices();
    	std::vector<int> ind3 = seed3->getCrystalIndices();

		// For three ecal clusters, they locate at two sides in y, and two of them locate at negative side and one locates at positive side in x
		if (!((ind1[1] > 0 && ind2[1] > 0 && ind3[1] > 0)
				|| (ind1[1] < 0 && ind2[1] < 0 && ind3[1] < 0))) {
			if ((ind1[0] < 0 && ind2[0] < 0 && ind3[0] > 0)
					|| (ind1[0] < 0 && ind2[0] > 0 && ind3[0] < 0)
					|| (ind1[0] > 0 && ind2[0] < 0 && ind3[0] < 0)) {

				int n_FSPs = fsps_->size();
				if (n_FSPs == 3) {
					histosParticle->FillFSPs(fsps_);

					Particle* part1 = fsps_->at(0);
					Particle* part2 = fsps_->at(1);
					Particle* part3 = fsps_->at(2);

					int charge_part1 = part1->getCharge();
					int charge_part2 = part2->getCharge();
					int charge_part3 = part3->getCharge();

					Particle *pos, *ele1, *ele2;

					// Require two electrons and one positron
					if ((charge_part1 == 1 && charge_part2 == -1
							&& charge_part3 == -1)
							|| (charge_part1 == -1 && charge_part2 == 1
									&& charge_part3 == -1)
							|| (charge_part1 == -1 && charge_part2 == -1
									&& charge_part3 == 1)) {

						// Reorder particles: 1. postrion; 2. electron; 3. electron
						if (charge_part1 == 1 && charge_part2 == -1
								&& charge_part3 == -1) {
							pos = part1;
							ele1 = part2;
							ele2 = part3;
						} else if (charge_part1 == -1 && charge_part2 == 1
								&& charge_part3 == -1) {
							pos = part2;
							ele1 = part1;
							ele2 = part3;
						} else if (charge_part1 == -1 && charge_part2 == -1
								&& charge_part3 == 1) {
							pos = part3;
							ele1 = part1;
							ele2 = part2;
						}

						double goodness_pos = pos->getGoodnessOfPID();
						double goodness_ele1 = ele1->getGoodnessOfPID();
						double goodness_ele2 = ele2->getGoodnessOfPID();

						histos->Fill1DHisto("goodness_pos_h", goodness_pos,
								weight);
						histos->Fill1DHisto("goodness_ele1_h", goodness_ele1,
								weight);
						histos->Fill1DHisto("goodness_ele2_h", goodness_ele2,
								weight);

						// Goodness cut for track-ecal match
						if (goodness_pos < GOODNESSCUT
								&& goodness_ele1 < GOODNESSCUT
								&& goodness_ele2 < GOODNESSCUT) {

							Track trk_pos = pos->getTrack();
							Track trk_ele1 = ele1->getTrack();
							Track trk_ele2 = ele2->getTrack();

							CalCluster clus_pos = pos->getCluster();
							CalCluster clus_ele1 = ele1->getCluster();
							CalCluster clus_ele2 = ele2->getCluster();

							std::vector<double> position_clus_pos =
									clus_pos.getPosition();
							std::vector<double> position_clus_ele1 =
									clus_ele1.getPosition();
							std::vector<double> position_clus_ele2 =
									clus_ele2.getPosition();

							double energy_pos = pos->getEnergy();
							double energy_ele1 = ele1->getEnergy();
							double energy_ele2 = ele2->getEnergy();

							std::vector<double> mom_pos =
									pos->getCorrMomentum();
							std::vector<double> mom_ele1 =
									ele1->getCorrMomentum();
							std::vector<double> mom_ele2 =
									ele2->getCorrMomentum();

							TLorentzVector *lzVect_pos = new TLorentzVector();
							TLorentzVector *lzVect_ele1 = new TLorentzVector();
							TLorentzVector *lzVect_ele2 = new TLorentzVector();
							lzVect_pos->SetXYZM(mom_pos[0], mom_pos[1],
									mom_pos[2], ELECTRONMASS);
							lzVect_ele1->SetXYZM(mom_ele1[0], mom_ele1[1],
									mom_ele1[2], ELECTRONMASS);
							lzVect_ele2->SetXYZM(mom_ele2[0], mom_ele2[1],
									mom_ele2[2], ELECTRONMASS);

							TLorentzVector *lzVect_sum = new TLorentzVector();
							*lzVect_sum = *lzVect_pos + *lzVect_ele1
									+ *lzVect_ele2;

							double energy_sum = energy_pos + energy_ele1
									+ energy_ele2;

							Track trk_pos_match, trk_ele1_match, trk_ele2_match;

							for(int iTrk; iTrk < trks_->size(); iTrk++){
								if(trk_pos.getMomentum()[2] == trks_->at(iTrk)->getMomentum()[2]) trk_pos_match = *trks_->at(iTrk);
								if(trk_ele1.getMomentum()[2] == trks_->at(iTrk)->getMomentum()[2]) trk_ele1_match = *trks_->at(iTrk);
								if(trk_ele2.getMomentum()[2] == trks_->at(iTrk)->getMomentum()[2]) trk_ele2_match = *trks_->at(iTrk);
							}

							// Track cut for chi2/NDF
							// Track momentum maximum cut to remove FEE (less than 75%EBeam)
							if (trk_pos.getChi2Ndf() < TRACKCHI2NDFCUT
									&& trk_ele1.getChi2Ndf() < TRACKCHI2NDFCUT
									&& trk_ele2.getChi2Ndf() < TRACKCHI2NDFCUT
									&& lzVect_pos->P() < TRACKMOMMAXCUT
									&& lzVect_ele1->P() < TRACKMOMMAXCUT
									&& lzVect_ele2->P() < TRACKMOMMAXCUT) {


								int num2DHits_pos = trk_pos_match.getTrackerHitCount();
								int num2DHits_ele1 = trk_ele1_match.getTrackerHitCount();
								int num2DHits_ele2 = trk_ele2_match.getTrackerHitCount();

								if (!trk_ele1_match.isKalmanTrack()){
									num2DHits_pos *= 2;
									num2DHits_ele1 *= 2;
									num2DHits_ele2 *= 2;

								}

								treeThreeFSPs->setVariableValue("posN2DHits", num2DHits_pos);
								treeThreeFSPs->setVariableValue("ele1N2DHits", num2DHits_ele1);
								treeThreeFSPs->setVariableValue("ele2N2DHits", num2DHits_ele2);

								double posTime = trk_pos_match.getTrackTime();
								double ele1Time = trk_ele1_match.getTrackTime();
								double ele2Time = trk_ele2_match.getTrackTime();

								treeThreeFSPs->setVariableValue("posTime", posTime);
								treeThreeFSPs->setVariableValue("ele1Time", ele1Time);
								treeThreeFSPs->setVariableValue("ele2Time", ele2Time);


								treeThreeFSPs->setVariableValue("energySum", energy_sum);
								treeThreeFSPs->setVariableValue("pxSum", lzVect_sum->Px());
								treeThreeFSPs->setVariableValue("pySum", lzVect_sum->Py());
								treeThreeFSPs->setVariableValue("pzSum", lzVect_sum->Pz());
								treeThreeFSPs->setVariableValue("invariantMass", lzVect_sum->M());


							    if(num2DHits_pos >= NHITSCUT
							    		&& num2DHits_ele1 >= NHITSCUT
										&& num2DHits_ele2 >= NHITSCUT
							    		&& fabs(posTime - ele1Time) < TIMEDIFFCUT
										&& fabs(posTime - ele2Time) < TIMEDIFFCUT
										&& fabs(ele1Time - ele2Time) < TIMEDIFFCUT){
									histos->Fill1DHisto("energySum_h", energy_sum,
											weight);

									histos->Fill1DHisto("pxSum_h", lzVect_sum->Px(),
											weight);
									histos->Fill1DHisto("pySum_h", lzVect_sum->Py(),
											weight);
									histos->Fill1DHisto("pzSum_h", lzVect_sum->Pz(),
											weight);

									histos->Fill1DHisto("invariantMass_h",
											lzVect_sum->M(), weight);
							    }

							    double chi2Vertex = -999;
							    double pxVertex = -999;
							    double pyVertex = -999;
							    double pzVertex = -999;
							    double imVertex = -999;
							    double xVertex = -999;
							    double yVertex = -999;
							    double zVertex = -999;
							    double chi2ndfEleVertex = -999;
							    double pxEleVertex = -999;
							    double pyEleVertex = -999;
							    double pzEleVertex = -999;
							    double chi2ndfPosVertex = -999;
							    double pxPosVertex = -999;
							    double pyPosVertex = -999;
							    double pzPosVertex = -999;

								int n_vtxs = vtxs_->size();

								if(n_vtxs == 1){
									Vertex* vtx = vtxs_->at(0);

									chi2Vertex = vtx->getChi2();
									pxVertex = vtx->getP().X();
									pyVertex = vtx->getP().Y();
									pzVertex = vtx->getP().Z();

									xVertex = vtx->getX();
									yVertex = vtx->getY();
									zVertex = vtx->getZ();

									imVertex = vtx->getInvMass();


							        Particle* ele_part = nullptr;
							        Particle* pos_part = nullptr;
							        bool foundParts = _ah->GetParticlesFromVtx(vtx, ele_part, pos_part);
									Track ele_trk = ele_part->getTrack();
									Track pos_trk = pos_part->getTrack();

									chi2ndfEleVertex = ele_trk.getChi2Ndf();
									pxEleVertex = ele_trk.getMomentum()[0];
									pyEleVertex = ele_trk.getMomentum()[1];
									pzEleVertex = ele_trk.getMomentum()[2];

									chi2ndfPosVertex = pos_trk.getChi2Ndf();
									pxPosVertex = pos_trk.getMomentum()[0];
									pyPosVertex = pos_trk.getMomentum()[1];
									pzPosVertex = pos_trk.getMomentum()[2];

								}

								treeThreeFSPs->setVariableValue("chi2Vertex", chi2Vertex);
								treeThreeFSPs->setVariableValue("pxVertex", pxVertex);
								treeThreeFSPs->setVariableValue("pyVertex", pyVertex);
								treeThreeFSPs->setVariableValue("pzVertex", pzVertex);
								treeThreeFSPs->setVariableValue("imVertex", imVertex);

								treeThreeFSPs->setVariableValue("xVertex", xVertex);
								treeThreeFSPs->setVariableValue("yVertex", yVertex);
								treeThreeFSPs->setVariableValue("zVertex", zVertex);

								treeThreeFSPs->setVariableValue("chi2ndfEleVertex", chi2ndfEleVertex);
								treeThreeFSPs->setVariableValue("pxEleVertex", pxEleVertex);
								treeThreeFSPs->setVariableValue("pyEleVertex", pyEleVertex);
								treeThreeFSPs->setVariableValue("pzEleVertex", pzEleVertex);
								treeThreeFSPs->setVariableValue("chi2ndfPosVertex", chi2ndfPosVertex);
								treeThreeFSPs->setVariableValue("pxPosVertex", pxPosVertex);
								treeThreeFSPs->setVariableValue("pyPosVertex", pyPosVertex);
								treeThreeFSPs->setVariableValue("pzPosVertex", pzPosVertex);


								treeThreeFSPs->fill();
							}
						}
					}

				}
			}
    	}
    }

	/****** Target offsetting study ******/
    if(tsData_->prescaled.Single_3_Top == true || tsData_->prescaled.Single_3_Bot == true) {

        int n_vtxs = vtxs_->size();
        histosVertex->Fill1DHisto("n_vertices_h",n_vtxs);

        std::vector<Vertex*> selected_vtxs;
        selected_vtxs.clear();

		// Loop over vertices in event and make selections
		for ( int i_vtx = 0; i_vtx <  n_vtxs; i_vtx++ ) {
			vtxSelector->getCutFlowHisto()->Fill(0.,weight);

			Vertex* vtx = vtxs_->at(i_vtx);
			Particle* ele = nullptr;
			Track* ele_trk = nullptr;
			Particle* pos = nullptr;
			Track* pos_trk = nullptr;

			bool foundParts = _ah->GetParticlesFromVtx(vtx,ele,pos);
			if (!foundParts) {
				if(debug_) std::cout<<"VertexAnaProcessor::WARNING::Found vtx without ele/pos. Skip."<<std::endl;
				continue;
			}

			if (!trkColl_.empty()) {
				bool foundTracks = _ah->MatchToGBLTracks((ele->getTrack()).getID(),(pos->getTrack()).getID(),
						ele_trk, pos_trk, *trks_);

				if (!foundTracks) {
					if(debug_) std::cout<<"VertexAnaProcessor::ERROR couldn't find ele/pos in the GBLTracks collection"<<std::endl;
					continue;
				}
			}
			else {
				ele_trk = (Track*)ele->getTrack().Clone();
				pos_trk = (Track*)pos->getTrack().Clone();
			}

			double ele_E = ele->getEnergy();
			double pos_E = pos->getEnergy();

			CalCluster eleClus = ele->getCluster();
			CalCluster posClus = pos->getCluster();


			//Compute analysis variables here.
			TLorentzVector p_ele;
			p_ele.SetPxPyPzE(ele_trk->getMomentum()[0],ele_trk->getMomentum()[1],ele_trk->getMomentum()[2],ele->getEnergy());
			TLorentzVector p_pos;
			p_pos.SetPxPyPzE(pos_trk->getMomentum()[0],pos_trk->getMomentum()[1],pos_trk->getMomentum()[2],ele->getEnergy());

			//Tracks in opposite volumes - useless
			//if (!vtxSelector->passCutLt("eleposTanLambaProd_lt",ele_trk->getTanLambda() * pos_trk->getTanLambda(),weight))
			//  continue;

			//Ele Track-cluster match
			if (!vtxSelector->passCutLt("eleTrkCluMatch_lt",ele->getGoodnessOfPID(),weight))
				continue;

			//Pos Track-cluster match
			if (!vtxSelector->passCutLt("posTrkCluMatch_lt",pos->getGoodnessOfPID(),weight))
				continue;

			//Require Positron Cluster exists
			if (!vtxSelector->passCutGt("posClusE_gt",posClus.getEnergy(),weight))
				continue;

			//Require Positron Cluster does NOT exists
			if (!vtxSelector->passCutLt("posClusE_lt",posClus.getEnergy(),weight))
				continue;

			double corr_eleClusterTime = ele->getCluster().getTime() - timeOffset_;
			double corr_posClusterTime = pos->getCluster().getTime() - timeOffset_;

			double botClusTime = 0.0;
			if(ele->getCluster().getPosition().at(1) < 0.0) botClusTime = ele->getCluster().getTime();
			else botClusTime = pos->getCluster().getTime();

			//Bottom Cluster Time
			if (!vtxSelector->passCutLt("botCluTime_lt", botClusTime, weight))
				continue;

			if (!vtxSelector->passCutGt("botCluTime_gt", botClusTime, weight))
				continue;

			//Ele Pos Cluster Time Difference
			if (!vtxSelector->passCutLt("eleposCluTimeDiff_lt",fabs(corr_eleClusterTime - corr_posClusterTime),weight))
				continue;

			//Ele Track-Cluster Time Difference
			if (!vtxSelector->passCutLt("eleTrkCluTimeDiff_lt",fabs(ele_trk->getTrackTime() - corr_eleClusterTime),weight))
				continue;

			//Pos Track-Cluster Time Difference
			if (!vtxSelector->passCutLt("posTrkCluTimeDiff_lt",fabs(pos_trk->getTrackTime() - corr_posClusterTime),weight))
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
			if (!vtxSelector->passCutLt("eleMom_lt",ele_mom.Mag(),weight))
				continue;

			//Ele Track Quality - Chi2
			if (!vtxSelector->passCutLt("eleTrkChi2_lt",ele_trk->getChi2(),weight))
				continue;

			//Pos Track Quality - Chi2
			if (!vtxSelector->passCutLt("posTrkChi2_lt",pos_trk->getChi2(),weight))
				continue;

			//Ele Track Quality - Chi2Ndf
			if (!vtxSelector->passCutLt("eleTrkChi2Ndf_lt",ele_trk->getChi2Ndf(),weight))
				continue;

			//Pos Track Quality - Chi2Ndf
			if (!vtxSelector->passCutLt("posTrkChi2Ndf_lt",pos_trk->getChi2Ndf(),weight))
				continue;

			//Ele min momentum cut
			if (!vtxSelector->passCutGt("eleMom_gt",ele_mom.Mag(),weight))
				continue;

			//Pos min momentum cut
			if (!vtxSelector->passCutGt("posMom_gt",pos_mom.Mag(),weight))
				continue;

			//Ele nHits
			int ele2dHits = ele_trk->getTrackerHitCount();
			if (!ele_trk->isKalmanTrack())
				ele2dHits*=2;

			if (!vtxSelector->passCutGt("eleN2Dhits_gt",ele2dHits,weight))  {
				continue;
			}

			//Pos nHits
			int pos2dHits = pos_trk->getTrackerHitCount();
			if (!pos_trk->isKalmanTrack())
				pos2dHits*=2;

			if (!vtxSelector->passCutGt("posN2Dhits_gt",pos2dHits,weight))  {
				continue;
			}

			//Less than 4 shared hits for ele/pos track
			if (!vtxSelector->passCutLt("eleNshared_lt",ele_trk->getNShared(),weight)) {
				continue;
			}

			if (!vtxSelector->passCutLt("posNshared_lt",pos_trk->getNShared(),weight)) {
				continue;
			}


			//Vertex Quality
			if (!vtxSelector->passCutLt("chi2unc_lt",vtx->getChi2(),weight))
				continue;

			//Max vtx momentum
			if (!vtxSelector->passCutLt("maxVtxMom_lt",(ele_mom+pos_mom).Mag(),weight))
				continue;

			//Min vtx momentum

			if (!vtxSelector->passCutGt("minVtxMom_gt",(ele_mom+pos_mom).Mag(),weight))
				continue;

			histosVertex->Fill1DVertex(vtx,
					ele,
					pos,
					ele_trk,
					pos_trk,
					weight);

			histosVertex->Fill1DHisto("vtx_Psum_h", p_ele.P()+p_pos.P(), weight);
			histosVertex->Fill1DHisto("vtx_Esum_h", ele_E + pos_E, weight);
			histosVertex->Fill1DHisto("ele_pos_clusTimeDiff_h", fabs(corr_eleClusterTime - corr_posClusterTime), weight);
			histosVertex->Fill2DHisto("ele_vtxZ_iso_hh", TMath::Min(ele_trk->getIsolation(0), ele_trk->getIsolation(1)), vtx->getZ(), weight);
			histosVertex->Fill2DHisto("pos_vtxZ_iso_hh", TMath::Min(pos_trk->getIsolation(0), pos_trk->getIsolation(1)), vtx->getZ(), weight);
			histosVertex->Fill2DHistograms(vtx,weight);
			histosVertex->Fill2DTrack(ele_trk,weight,"ele_");
			histosVertex->Fill2DTrack(pos_trk,weight,"pos_");


			histos->Fill1DHisto("eleposCluTimeDiff_h", ele->getCluster().getTime() - pos->getCluster().getTime() , weight);

			histos->Fill1DHisto("eleTrkCluTimeDiff_h", ele_trk->getTrackTime() - ele->getCluster().getTime() , weight);

			histos->Fill1DHisto("posTrkCluTimeDiff_h", pos_trk->getTrackTime() - pos->getCluster().getTime() , weight);

			selected_vtxs.push_back(vtx);

			vtxSelector->clearSelector();
		}

		if(selected_vtxs.size() == 1){
			treeVertex->setVariableValue("xVertex", selected_vtxs[0]->getX());
			treeVertex->setVariableValue("yVertex", selected_vtxs[0]->getY());
			treeVertex->setVariableValue("zVertex", selected_vtxs[0]->getZ());

			treeVertex->fill();
		}

    } // requires that single3 trigger is fired


    return true;
}

void BeamRotationTargetOffsettingAnaProcessor::finalize() {
	outF_->cd();

    histos->saveHistos(outF_, anaName_.c_str());
    delete histos;
    histos = nullptr;


    histosVertex->saveHistos(outF_, histosVertex->getName());
    outF_->cd(histosVertex->getName().c_str());
    vtxSelector->getCutFlowHisto()->Write();
    delete histosVertex;
    histosVertex = nullptr;

    histosParticle->saveHistos(outF_, histosParticle->getName());
    delete histosParticle;
    histosParticle = nullptr;

    TDirectory* dir{nullptr};
    dir = outF_->mkdir("treeThreeFSPs");
    dir->cd();
    treeThreeFSPs->writeTree();

    dir = outF_->mkdir("treeVertex");
    dir->cd();
    treeVertex->writeTree();

    outF_->Close();

}

DECLARE_PROCESSOR(BeamRotationTargetOffsettingAnaProcessor);
