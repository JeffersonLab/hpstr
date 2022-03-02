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

    //Save tuple variables in a tree
    treeTuple = std::make_shared<FlatTupleMaker>("tuple");

    treeTuple->addVariable("posN2DHits");
    treeTuple->addVariable("ele1N2DHits");
    treeTuple->addVariable("ele2N2DHits");

    treeTuple->addVariable("posTime");
    treeTuple->addVariable("ele1Time");
    treeTuple->addVariable("ele2Time");

    treeTuple->addVariable("energySum");
    treeTuple->addVariable("pxSum");
    treeTuple->addVariable("pySum");
    treeTuple->addVariable("pzSum");
    treeTuple->addVariable("invariantMass");

    treeTuple->addVariable("chi2Vertex");
    treeTuple->addVariable("pxVertex");
    treeTuple->addVariable("pyVertex");
    treeTuple->addVariable("pzVertex");
    treeTuple->addVariable("imVertex");
    treeTuple->addVariable("xVertex");
    treeTuple->addVariable("yVertex");
    treeTuple->addVariable("zVertex");
    treeTuple->addVariable("chi2ndfEleVertex");
    treeTuple->addVariable("pxEleVertex");
    treeTuple->addVariable("pyEleVertex");
    treeTuple->addVariable("pzEleVertex");
    treeTuple->addVariable("chi2ndfPosVertex");
    treeTuple->addVariable("pxPosVertex");
    treeTuple->addVariable("pyPosVertex");
    treeTuple->addVariable("pzPosVertex");

}

bool BeamRotationTargetOffsettingAnaProcessor::process(IEvent* ievent) {
	double weight = 1.;

	int n_vtxs = vtxs_->size();
	histosVertex->Fill1DHisto("n_vertices_h",n_vtxs);

	for (int iVertex = 0; iVertex < n_vtxs; iVertex ++){
		Vertex* vertex = vtxs_->at(iVertex);
		histosVertex->Fill1DVertex(vertex);
		histosVertex->Fill2DHistograms(vertex);
	}

	int n_ecalClusts = ecalClusters_->size();

	// Single3 is fired
	// Have and only have 3 Ecal clusters
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



								treeTuple->setVariableValue("posN2DHits", trk_pos_match.getTrackerHitCount());
								treeTuple->setVariableValue("ele1N2DHits", trk_ele1_match.getTrackerHitCount());
								treeTuple->setVariableValue("ele2N2DHits", trk_ele2_match.getTrackerHitCount());

								double posTime = trk_pos_match.getTrackTime();
								double ele1Time = trk_ele1_match.getTrackTime();
								double ele2Time = trk_ele2_match.getTrackTime();

								treeTuple->setVariableValue("posTime", posTime);
								treeTuple->setVariableValue("ele1Time", ele1Time);
								treeTuple->setVariableValue("ele2Time", ele2Time);


								treeTuple->setVariableValue("energySum", energy_sum);
								treeTuple->setVariableValue("pxSum", lzVect_sum->Px());
								treeTuple->setVariableValue("pySum", lzVect_sum->Py());
								treeTuple->setVariableValue("pzSum", lzVect_sum->Pz());
								treeTuple->setVariableValue("invariantMass", lzVect_sum->M());


							    if(trk_pos_match.getTrackerHitCount() >= NHITSCUT
							    		&& trk_ele1_match.getTrackerHitCount() >= NHITSCUT
										&& trk_ele2_match.getTrackerHitCount() >= NHITSCUT
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

								treeTuple->setVariableValue("chi2Vertex", chi2Vertex);
								treeTuple->setVariableValue("pxVertex", pxVertex);
								treeTuple->setVariableValue("pyVertex", pyVertex);
								treeTuple->setVariableValue("pzVertex", pzVertex);
								treeTuple->setVariableValue("imVertex", imVertex);

								treeTuple->setVariableValue("xVertex", xVertex);
								treeTuple->setVariableValue("yVertex", yVertex);
								treeTuple->setVariableValue("zVertex", zVertex);

								treeTuple->setVariableValue("chi2ndfEleVertex", chi2ndfEleVertex);
								treeTuple->setVariableValue("pxEleVertex", pxEleVertex);
								treeTuple->setVariableValue("pyEleVertex", pyEleVertex);
								treeTuple->setVariableValue("pzEleVertex", pzEleVertex);
								treeTuple->setVariableValue("chi2ndfPosVertex", chi2ndfPosVertex);
								treeTuple->setVariableValue("pxPosVertex", pxPosVertex);
								treeTuple->setVariableValue("pyPosVertex", pyPosVertex);
								treeTuple->setVariableValue("pzPosVertex", pzPosVertex);


								treeTuple->fill();
							}
						}
					}

				}
			}
    	}
    }

    return true;
}

void BeamRotationTargetOffsettingAnaProcessor::finalize() {
	outF_->cd();

    histos->saveHistos(outF_, anaName_.c_str());
    delete histos;
    histos = nullptr;


    histosVertex->saveHistos(outF_, histosVertex->getName());
    delete histosVertex;
    histosVertex = nullptr;

    histosParticle->saveHistos(outF_, histosParticle->getName());
    delete histosParticle;
    histosParticle = nullptr;

    TDirectory* dir{nullptr};
    dir = outF_->mkdir("treeTuple");
    dir->cd();
    treeTuple->writeTree();

    outF_->Close();

}

DECLARE_PROCESSOR(BeamRotationTargetOffsettingAnaProcessor);
