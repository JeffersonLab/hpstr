/**
 *@file VtxAnaProcessor.cxx
 *@author Tongtong, UNH
 */

#include "VtxAnaProcessor.h"
#include "TMatrixD.h"
#include "TMath.h"

#include <iostream>
#include <stdio.h>

VtxAnaProcessor::VtxAnaProcessor(const std::string& name, Process& process) : Processor(name,process) {

}

//TODO Check this destructor

VtxAnaProcessor::~VtxAnaProcessor(){}

void VtxAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring VertexAnaProcessor" <<std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        anaName_         = parameters.getString("anaName");
        histCfgFilename_      = parameters.getString("histCfg",histCfgFilename_);
        vtxColl_ = parameters.getString("vtxColl",vtxColl_);
        tcvtxColl_ = parameters.getString("tcvtxColl",tcvtxColl_);
        beamE_  = parameters.getDouble("beamE",beamE_);
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

	_ah =  std::make_shared<AnaHelpers>();
}

void VtxAnaProcessor::initialize(TTree* tree) {
    tree_= tree;
    // init histos
    histos = new VtxAnaHistos(anaName_.c_str());
    histos->loadHistoConfig(histCfgFilename_);
    histos->DefineHistos();

    // init TTree
    tree_->SetBranchAddress(tsColl_.c_str(), &tsData_ , &btsData_);
    tree_->SetBranchAddress(vtxColl_.c_str(), &vtxs_ , &bvtxs_);
    tree_->SetBranchAddress(tcvtxColl_.c_str(), &tcvtxs_ , &btcvtxs_);
}

bool VtxAnaProcessor::process(IEvent* ievent) {
    histos->FillTSData(tsData_);

    if(tsData_->prescaled.Single_3_Top == true && tsData_->prescaled.Single_3_Bot == true){
    	std::cout << "Warning: Single3 trigger for both top and bot are registered." << std::endl;
    	std::cout << "Number of vertices: " << vtxs_->size() << std::endl;
    }

    if(tsData_->prescaled.Single_3_Top == true || tsData_->prescaled.Single_3_Bot == true){
    	histos->FillUnconstrainedV0s(vtxs_);
        for(int i = 0; i <vtxs_->size(); i++) {
        	vect_all_uc_v0s.push_back(*(vtxs_->at(i)));

    		Particle* ele = nullptr;
    		Particle* pos = nullptr;
    		bool foundParts = _ah->GetParticlesFromVtx(vtxs_->at(i), ele, pos);

    		if (!foundParts) {
    			std::cout
    					<< "VertexAnaProcessor::WARNING::Found vtx without ele/pos. Skip."
    					<< std::endl;
    		}

    		Track trackEle = ele->getTrack();
    		Track trackPos = pos->getTrack();

    		vect_all_track_ele.push_back(trackEle);
    		vect_all_track_pos.push_back(trackPos);

        }
    }



    return true;
}

void VtxAnaProcessor::finalize() {


    histos->saveHistos(outF_, anaName_.c_str());
    delete histos;
	histos = nullptr;



	int numAllUCVOs = vect_all_uc_v0s.size();
	TMatrixD vectX(numAllUCVOs, 1);
	TMatrixD vectY(numAllUCVOs, 1);
	TMatrixD vectZ(numAllUCVOs, 1);

	TMatrixD vectLogChi2PerNDFV0(numAllUCVOs, 1);
	TMatrixD vectLogChi2PerNDFTrackEle(numAllUCVOs, 1);
	TMatrixD vectLogChi2PerNDFTrackPos(numAllUCVOs, 1);

	for (int i = 0; i < numAllUCVOs; i++) {
		Vertex vtx = vect_all_uc_v0s[i];

		double x = vtx.getX();
		double y = vtx.getY();
		double z = vtx.getZ();

		vectX(i, 0) = x;
		vectY(i, 0) = y;
		vectZ(i, 0) = z;

		double chi2 = vtx.getChi2();
		double ndf = 1.; // NDF = 2n - 3 for unconstrained vertices; for vertices constructed by two tracks, n = 2


		Track trackEle = vect_all_track_ele[i];
		Track trackPos = vect_all_track_pos[i];

		double chi2Ele = trackEle.getChi2();
		double ndfEle = trackEle.getNdf();

		double chi2Pos = trackPos.getChi2();
		double ndfPos = trackPos.getNdf();

		vectLogChi2PerNDFV0(i, 0) = (float) TMath::Log(chi2 / ndf);
		vectLogChi2PerNDFTrackEle(i, 0) = (float) TMath::Log(chi2Ele / ndfEle);
		vectLogChi2PerNDFTrackPos(i, 0) = (float) TMath::Log(chi2Pos / ndfPos);

		if(std::isnan(vectLogChi2PerNDFV0(i, 0)) || std::isnan(vectLogChi2PerNDFTrackEle(i, 0)) || isnan(vectLogChi2PerNDFTrackPos(i, 0))){
			//std::cout << vectLogChi2PerNDFV0(i, 0) << " " << vectLogChi2PerNDFTrackEle(i, 0) << " " << vectLogChi2PerNDFTrackPos(i, 0) <<std::endl;
			std::cout<< chi2 << "  " << chi2Ele << "  " << chi2Pos << std::endl;
		}
	}

	//Mean and covariance for vertex
	std::cout << "Mean and covaiance for unconstrained vertex:" << std::endl;

	double meanX, meanY, meanZ;
	meanX = vectX.Sum() / numAllUCVOs;
	meanY = vectY.Sum() / numAllUCVOs;
	meanZ = vectZ.Sum() / numAllUCVOs;

	TMatrixD meanVect(3, 1);
	meanVect(0, 0) = meanX;
	meanVect(1, 0) = meanY;
	meanVect(2, 0) = meanZ;
	meanVect.Print();

	//vectX.Print();
	TMatrixD vectXT(1, numAllUCVOs);
	vectXT.Transpose(vectX);

	TMatrixD vectYT(1, numAllUCVOs);
	vectYT.Transpose(vectY);

	TMatrixD vectZT(1, numAllUCVOs);
	vectZT.Transpose(vectZ);

	TMatrixD varX = 1. / numAllUCVOs * (vectXT - meanX)
			* (vectX - meanX);
	TMatrixD varY = 1. / numAllUCVOs * (vectYT - meanY)
			* (vectY - meanY);
	TMatrixD varZ = 1. / numAllUCVOs* (vectZT - meanZ)
			* (vectZ - meanZ);
	TMatrixD covXY = 1. / numAllUCVOs * (vectXT - meanX)
			* (vectY - meanY);
	TMatrixD covYZ = 1. / numAllUCVOs * (vectYT - meanY)
			* (vectZ - meanZ);
	TMatrixD covZX = 1. / numAllUCVOs * (vectZT - meanZ)
			* (vectX - meanX);

	TMatrixD covVect(3, 3);

	covVect(0, 0) = varX(0, 0);
	covVect(1, 1) = varY(0, 0);
	covVect(2, 2) = varZ(0, 0);
	covVect(0, 1) = covXY(0, 0);
	covVect(1, 2) = covYZ(0, 0);
	covVect(0, 2) = covZX(0, 0);
	covVect(1, 0) = covVect(0, 1);
	covVect(2, 1) = covVect(1, 2);
	covVect(2, 0) = covVect(0, 2);

	covVect.Print();

	//Mean and covariance for log(Chi2/NDF)
	std::cout << "Mean and covaiance for log(Chi2/NDF):" << std::endl;

	double meanChi2PerNDFV0, meanChi2PerNDFEle, meanChi2PerNDFPos;
	meanChi2PerNDFV0 = vectLogChi2PerNDFV0.Sum() / numAllUCVOs;
	meanChi2PerNDFEle = vectLogChi2PerNDFTrackEle.Sum() / numAllUCVOs;
	meanChi2PerNDFPos = vectLogChi2PerNDFTrackPos.Sum() / numAllUCVOs;


	TMatrixD meanChi2PerNDF(3, 1);
	meanChi2PerNDF(0, 0) = meanChi2PerNDFV0;
	meanChi2PerNDF(1, 0) = meanChi2PerNDFEle;
	meanChi2PerNDF(2, 0) = meanChi2PerNDFPos;

	meanChi2PerNDF.Print();

	//Transpose
	TMatrixD vectLogChi2PerNDFV0T(1, numAllUCVOs);
	vectLogChi2PerNDFV0T.Transpose(vectLogChi2PerNDFV0);
	TMatrixD vectLogChi2PerNDFTrackEleT(1, numAllUCVOs);
	vectLogChi2PerNDFTrackEleT.Transpose(vectLogChi2PerNDFTrackEle);
	TMatrixD vectLogChi2PerNDFTrackPosT(1, numAllUCVOs);
	vectLogChi2PerNDFTrackPosT.Transpose(vectLogChi2PerNDFTrackPos);


	TMatrixD varLogChi2PerNDFTrackV0 = 1. / numAllUCVOs * (vectLogChi2PerNDFV0T - meanChi2PerNDFV0)
			* (vectLogChi2PerNDFV0 - meanChi2PerNDFV0);
	TMatrixD varLogChi2PerNDFTrackEle = 1. / numAllUCVOs * (vectLogChi2PerNDFTrackEleT - meanChi2PerNDFEle)
			* (vectLogChi2PerNDFTrackEle - meanChi2PerNDFEle);
	TMatrixD varLogChi2PerNDFTrackPos = 1. / numAllUCVOs * (vectLogChi2PerNDFTrackPosT - meanChi2PerNDFPos)
			* (vectLogChi2PerNDFTrackPos - meanChi2PerNDFPos);

	TMatrixD covLogChi2PerNDFTrackV0Ele = 1. / numAllUCVOs * (vectLogChi2PerNDFV0T - meanChi2PerNDFV0)
			* (vectLogChi2PerNDFTrackEle - meanChi2PerNDFEle);
	TMatrixD covLogChi2PerNDFTrackElePos = 1. / numAllUCVOs * (vectLogChi2PerNDFTrackEleT - meanChi2PerNDFEle)
			* (vectLogChi2PerNDFTrackPos - meanChi2PerNDFPos);
	TMatrixD covLogChi2PerNDFTrackPosV0 = 1. / numAllUCVOs * (vectLogChi2PerNDFTrackPosT - meanChi2PerNDFPos)
			* (vectLogChi2PerNDFV0 - meanChi2PerNDFV0);

	//(vectLogChi2PerNDFV0T - meanChi2PerNDFV0).Print();
	//(vectLogChi2PerNDFV0 - meanChi2PerNDFV0).Print();

	std::cout<<"meanChi2PerNDFV0:" << meanChi2PerNDFV0 <<std::endl;


	TMatrixD covLogChi2PerNDF(3, 3);

	covLogChi2PerNDF(0, 0) = varLogChi2PerNDFTrackV0(0, 0);
	covLogChi2PerNDF(1, 1) = varLogChi2PerNDFTrackEle(0, 0);
	covLogChi2PerNDF(2, 2) = varLogChi2PerNDFTrackPos(0, 0);
	covLogChi2PerNDF(0, 1) = covLogChi2PerNDFTrackV0Ele(0, 0);
	covLogChi2PerNDF(1, 2) = covLogChi2PerNDFTrackElePos(0, 0);
	covLogChi2PerNDF(0, 2) = covLogChi2PerNDFTrackPosV0(0, 0);
	covLogChi2PerNDF(1, 0) = covLogChi2PerNDF(0, 1);
	covLogChi2PerNDF(2, 1) = covLogChi2PerNDF(1, 2);
	covLogChi2PerNDF(2, 0) = covLogChi2PerNDF(0, 2);

	covLogChi2PerNDF.Print();


}

DECLARE_PROCESSOR(VtxAnaProcessor);
