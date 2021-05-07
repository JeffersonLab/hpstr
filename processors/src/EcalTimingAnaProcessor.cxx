/**
 *@file EcalTimingAnaProcessor.cxx
 *@author Tongtong, UNH
 */

#include "CalHit.h"
#include "EcalTimingAnaProcessor.h"

#include <iostream>

#include "TF1.h"

EcalTimingAnaProcessor::EcalTimingAnaProcessor(const std::string& name, Process& process) : Processor(name,process) {

}

//TODO Check this destructor

EcalTimingAnaProcessor::~EcalTimingAnaProcessor(){}

void EcalTimingAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring EcalTimingAnaProcessor" <<std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        anaName_         = parameters.getString("anaName");
        histCfgFilename_      = parameters.getString("histCfg",histCfgFilename_);
        ecalClusColl_    = parameters.getString("ecalClusColl");
        vtxColl_ = parameters.getString("vtxColl",vtxColl_);
        beamE_  = parameters.getDouble("beamE",beamE_);
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

}

void EcalTimingAnaProcessor::initialize(TTree* tree) {
    tree_= tree;
    // init histos
    histos = new EcalTimingAnaHistos(anaName_.c_str());
    histos->loadHistoConfig(histCfgFilename_);
    histos->DefineHistos();

    // init TTree
    tree_->SetBranchAddress(vtpColl_.c_str() , &vtpData_, &bvtpData_);
    tree_->SetBranchAddress(tsColl_.c_str(), &tsData_ , &btsData_);
    tree_->SetBranchAddress(trkColl_.c_str() , &trks_, &btrks_);
    tree_->SetBranchAddress(ecalClusColl_.c_str() , &ecalClusters_, &becalClusters_);
    tree_->SetBranchAddress(vtxColl_.c_str(), &vtxs_ , &bvtxs_);

    // init histogram
    histDT = new TH1D("Time difference", "Time difference; Time difference (ns); Counts", 300, -15, 15);
    histDTESumMinCut = new TH1D("Time difference with Esum min cut", "Time difference with Esum min cut; Time difference (ns); Counts", 300, -15, 15);
    histDTOnePosNegTrack = new TH1D("Time difference with only one pos and one neg tracks", "Time difference; Time difference (ns); Counts", 300, -15, 15);

    histDTCorr = new TH1D("Time difference after time correction", "Time difference after time correction; Time difference (ns); Counts", 300, -15, 15);
    histDTESumMinCutCorr = new TH1D("Time difference with Esum min cut after time correction", "Time difference with Esum min cut after time correction; Time difference (ns); Counts", 300, -15, 15);
    histDTOnePosNegTrackCorr = new TH1D("Time difference with only one pos and one neg tracks after time correction", "Time difference with only one pos and one neg tracks after time correction; Time difference (ns); Counts", 300, -15, 15);

    histDTESumMinCutCorrGlobal = new TH1D("Time difference with Esum min cut after time correction and global correction", "Time difference with Esum min cut after time correction and global correction; Time difference (ns); Counts", 300, -15, 15);

    histDTMean = new TH2D("Mean of time difference", "Mean of time difference; X; Y", 47, -23.5, 23.5, 11, -5.5, 5.5);
    histDTCount = new TH2D("Count for bin of histDTMean", "Count for bin of histDTMean; X; Y", 47, -23.5, 23.5, 11, -5.5, 5.5);

    histDTMeanCorr = new TH2D("Mean of time difference after time correction", "Mean of time difference after time correction; X; Y", 47, -23.5, 23.5, 11, -5.5, 5.5);
    histDTCountCorr = new TH2D("Count for bin of histDTMean after time correction", "Count for bin of histDTMean after time correction; X; Y", 47, -23.5, 23.5, 11, -5.5, 5.5);

    histEcalClusterTimeCorection = new TH2D("Mean of DT", "Mean of DT", 47, -23.5, 23.5, 11, -5.5, 5.5);

    for (int ix = -23; ix <= 23; ix++) {
        for (int iy = -5; iy <= 5; iy++) {
        	histMapDTMean[std::pair<int, int>(ix, iy)] = new TH1D(Form("DT_Mean_%d_%d", ix, iy), "", 200, -5., 5.);
        	dtCountMap[std::pair<int, int>(ix, iy)] = 0;

        	histMapDTMeanCorr[std::pair<int, int>(ix, iy)] = new TH1D(Form("DT_corr_Mean_%d_%d", ix, iy), "", 200, -5., 5.);
        	dtCountMapCorr[std::pair<int, int>(ix, iy)] = 0;

        	histEcalClusterTimeCorection->Fill(ix, iy, ecalClusterTimeCorrectionArray[ix + 23][iy+5]);
        }
    }

    // init cut functions
    ecalClusterTimeCutTop = new TF1("funcTop", "40.72 + 0.002095 * x - 0.000000387 * x * x", 0, 5000);
    ecalClusterTimeCutBot = new TF1("funcTop", "30.31 + 0.003204 * x - 0.0000007116 * x * x", 0, 5000);
}

bool EcalTimingAnaProcessor::process(IEvent* ievent) {
    histos->FillTSData(tsData_);

    if(tsData_->prescaled.Single_3_Top == true && tsData_->prescaled.Single_3_Bot == true){
    	std::cout << "Warning: Single3 trigger for both top and bot are registered." << std::endl;
    	std::cout << "Number of vertices: " << vtxs_->size() << std::endl;
    }

    if(tsData_->prescaled.Single_3_Top == true || tsData_->prescaled.Single_3_Bot == true){
    	histos->FillEcalClusters(ecalClusters_);
    	histos->FillTargetConstrainedV0s(vtxs_);

    	int nPos = 0, nNeg = 0;
        for(int i=0; i < trks_->size(); i++){
        	Track* tr = trks_->at(i);
        	int charge = tr->getCharge();
            if (charge == 1) {
            	nPos++;
            } else if (charge == -1) {
            	nNeg++;
            }

        }

    	std::vector<int> clIndexTop, clIndexBot;

        for (int i=0; i < ecalClusters_->size(); i++){
        	CalCluster* cl = ecalClusters_->at(i);
        	double time = cl->getTime();
        	double energy = cl->getEnergy() * 1000.0; // Energy scaled to MeV
        	double timeCutMax = ecalClusterTimeCutTop->Eval(energy);
        	double timeCutMin = ecalClusterTimeCutBot->Eval(energy);

        	if(time > timeCutMin && time < timeCutMax){
        		if(ecalClusters_->at(i)->getPosition()[1] > 0) clIndexTop.push_back(i);
        		else clIndexBot.push_back(i);
        	}
        }

        for (int iTopCl = 0; iTopCl < clIndexTop.size(); iTopCl++) {
            for (int iBotCl = 0; iBotCl < clIndexBot.size(); iBotCl++) {
            	CalCluster* clTop = ecalClusters_->at(clIndexTop[iTopCl]);
            	CalCluster* clBot = ecalClusters_->at(clIndexBot[iBotCl]);

            	CalHit* clSeedTop= (CalHit*)clTop->getSeed();
            	CalHit* clSeedBot= (CalHit*)clBot->getSeed();

                int ix_top = clSeedTop->getCrystalIndices()[0];
                int iy_top = clSeedTop->getCrystalIndices()[1];
                int ix_bot = clSeedBot->getCrystalIndices()[0];
                int iy_bot = clSeedBot->getCrystalIndices()[1];

                double clTimeTop = clTop->getTime();
                double clTimeBot = clBot->getTime();
                double clTimeCorrTop = clTimeTop - histEcalClusterTimeCorection->GetBinContent(histEcalClusterTimeCorection->FindBin(ix_top, iy_top));
                double clTimeCorrBot = clTimeBot - histEcalClusterTimeCorection->GetBinContent(histEcalClusterTimeCorection->FindBin(ix_bot, iy_bot));

                double dt = clTimeTop - clTimeBot;
                double dtCorr = clTimeCorrTop - clTimeCorrBot;
                double Esum = clTop->getEnergy() + clBot->getEnergy();

                histDT->Fill(dt);
                histDTCorr->Fill(dtCorr);

                if(nPos == 1 && nNeg == 1) {
                	histDTOnePosNegTrack->Fill(dt);
                	histDTOnePosNegTrackCorr->Fill(dtCorr);
                }


                if (Esum > 0.75 * beamE_) {
                	histDTESumMinCut->Fill(dt);
                 	histDTESumMinCutCorr->Fill(dtCorr);
                 	histDTESumMinCutCorrGlobal->Fill(dtCorr + 0.474);

                	dtCountMap[std::pair<int, int>(ix_top, iy_top)]++;
                	histMapDTMean[std::pair<int, int>(ix_top, iy_top)]->Fill(dt);
                   	dtCountMap[std::pair<int, int>(ix_bot, iy_bot)]++;
                	histMapDTMean[std::pair<int, int>(ix_bot, iy_bot)]->Fill(-dt);

                	dtCountMapCorr[std::pair<int, int>(ix_top, iy_top)]++;
                	histMapDTMeanCorr[std::pair<int, int>(ix_top, iy_top)]->Fill(dtCorr);
                   	dtCountMapCorr[std::pair<int, int>(ix_bot, iy_bot)]++;
                	histMapDTMeanCorr[std::pair<int, int>(ix_bot, iy_bot)]->Fill(-dtCorr);
                }
            }
        }
    }



    return true;
}

void EcalTimingAnaProcessor::finalize() {


    histos->saveHistos(outF_, anaName_.c_str());
    delete histos;
    histos = nullptr;

    for (int ix = -23; ix <= 23; ix++) {
        for (int iy = -5; iy <= 5; iy++) {

            double dtMean = histMapDTMean[std::pair<int, int>(ix, iy)]->GetMean();
            histDTMean->Fill(ix, iy, dtMean);
            histDTCount->Fill(ix, iy, dtCountMap[std::pair<int, int>(ix, iy)]);

            double dtMeanCorr = histMapDTMeanCorr[std::pair<int, int>(ix, iy)]->GetMean();
            histDTMeanCorr->Fill(ix, iy, dtMeanCorr);
            histDTCountCorr->Fill(ix, iy, dtCountMapCorr[std::pair<int, int>(ix, iy)]);
        }
    }

    outF_->cd();
    TDirectory* dir = outF_->mkdir("extra");
    dir->cd();

    histDT->Write();
    histDTESumMinCut->Write();
    histDTOnePosNegTrack->Write();

    histDTCorr->Write();
    histDTESumMinCutCorr->Write();
    histDTOnePosNegTrackCorr->Write();

    histDTESumMinCutCorrGlobal->Write();

    histDTMean->Write();
    histDTCount->Write();

    histDTMeanCorr->Write();
    histDTCountCorr->Write();

    outF_->Close();
}

DECLARE_PROCESSOR(EcalTimingAnaProcessor);
