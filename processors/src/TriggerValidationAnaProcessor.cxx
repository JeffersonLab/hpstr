/**
 * Check trigger efficiency using Ecal clusters, hodoscope hits, and tracks from resonctruion and VTP bank from hardware.
 * Test how much percent that reconstructed events of interests with satisfactory of requirements are triggered.
 * Try to figure out what cause reconstructed events of interests are not triggered.
 *
 *@file TriggerValidationAnaProcessor.cxx
 *@author Tongtong, UNH
 */
#define ENERGYRATIOECALVTPCLUSTERS 1.0 // Ratio of energy between ecal and vtp clusters

/*
#define TIMEECALCLUSTERMINTOP 42 // ns
#define TIMEECALCLUSTERMAXTOP 55 // ns
#define TIMEECALCLUSTERMINBOT 39 // ns
#define TIMEECALCLUSTERMAXBOT 49 // ns

#define TIMEDIFFECALCLUSTERHODOHITMINTOP -60 // ns
#define TIMEDIFFECALCLUSTERHODOHITMAXTOP 4 // ns
#define TIMEDIFFECALCLUSTERHODOHITMINBOT -60 // ns
#define TIMEDIFFECALCLUSTERHODOHITMAXBOT 4 // ns

#define TIMEDIFFECALCLUSTERVTPSINGLETRIGGERMIN -1000 // ns
#define TIMEDIFFECALCLUSTERVTPSINGLETRIGGERMAX 1000 // ns
*/

#define TIMEECALCLUSTERMINTOP 30 // ns
#define TIMEECALCLUSTERMAXTOP 42 // ns
#define TIMEECALCLUSTERMINBOT 30 // ns
#define TIMEECALCLUSTERMAXBOT 42 // ns

#define TIMEDIFFECALCLUSTERHODOHITMINTOP -16 // ns
#define TIMEDIFFECALCLUSTERHODOHITMAXTOP 0 // ns
#define TIMEDIFFECALCLUSTERHODOHITMINBOT -20 // ns
#define TIMEDIFFECALCLUSTERHODOHITMAXBOT -4 // ns

#define TIMEDIFFECALCLUSTERVTPSINGLETRIGGERMIN -9 // ns
#define TIMEDIFFECALCLUSTERVTPSINGLETRIGGERMAX 4 // ns

#define GAINFACTOR 1.25/2 // Gain scaling factor for hits at two-hole tiles.
#define FADCHITTHRESHOLD 1 // Hodoscope FADC hit cut
#define HODOHITTHRESHOLD 200 // Hodoscope tilt/cluster hit cut

//parameters for trigger conditions
#define VTP_HPS_SINGLE_EMIN_SINGLE3 300 // MeV
#define VTP_HPS_SINGLE_EMAX 2500 // MeV
#define VTP_HPS_SINGLE_NMIN 2
#define VTP_HPS_SINGLE_XMIN 5

#define VTP_HPS_SINGLE_PDE_C0_SINGLE3 1322.79
#define VTP_HPS_SINGLE_PDE_C1_SINGLE3 -99.6022
#define VTP_HPS_SINGLE_PDE_C2_SINGLE3 2.89822
#define VTP_HPS_SINGLE_PDE_C3_SINGLE3 -0.0206909

#define VTP_HPS_SINGLE_EMIN_SINGLE2 400 // MeV
#define VTP_HPS_SINGLE_PDE_C0_SINGLE2 1782.58
#define VTP_HPS_SINGLE_PDE_C1_SINGLE2 -136.573
#define VTP_HPS_SINGLE_PDE_C2_SINGLE2 4.44288
#define VTP_HPS_SINGLE_PDE_C3_SINGLE2  -0.0472998

#define VTP_HPS_FEE_EMIN 2000
#define VTP_HPS_FEE_EMAX 4700
#define VTP_HPS_FEE_NMIN 3

#define CHI2NDFTHRESHOLD 15
#define TRACKMOMENTUMMIN 0.85

#include "TriggerValidationAnaProcessor.h"
#include "CalHit.h"
#include <iostream>


TriggerValidationAnaProcessor::TriggerValidationAnaProcessor(const std::string& name, Process& process) : Processor(name,process) {

}

//TODO Check this destructor

TriggerValidationAnaProcessor::~TriggerValidationAnaProcessor(){}

void TriggerValidationAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring TriggerValidationAnaProcessor" <<std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        anaName_         = parameters.getString("anaName");
        histCfgFilename_      = parameters.getString("histCfg",histCfgFilename_);
        ecalClusColl_    = parameters.getString("ecalClusColl");
        beamE_  = parameters.getDouble("beamE",beamE_);
		#ifdef __WITHSVT__
        	trkColl_    = parameters.getString("trkColl");
		#endif
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

}

void TriggerValidationAnaProcessor::initialize(TTree* tree) {
    tree_= tree;
    // init histos
    histos = new TriggerValidationAnaHistos(anaName_.c_str());
    histos->loadHistoConfig(histCfgFilename_);
    histos->DefineHistos();

    // init TTree
    tree_->SetBranchAddress(vtpColl_.c_str() , &vtpData_, &bvtpData_);
    tree_->SetBranchAddress(tsColl_.c_str(), &tsData_ , &btsData_);
    tree_->SetBranchAddress(ecalClusColl_.c_str() , &ecalClusters_, &becalClusters_);
    tree_->SetBranchAddress(hodoHitColl_.c_str() , &hodoHits_, &bhodoHits_);
    tree_->SetBranchAddress(hodoClusColl_.c_str() , &hodoClusters_, &bhodoClusters_);

	#ifdef __WITHSVT__
	tree_->SetBranchAddress(trkColl_.c_str() , &trks_, &btrks_);
	#endif

    //Cut functions for X
    func_pos_top_topCutX = new TF1("func_pos_top_topCutX", "pol1", 50, 390);
    func_pos_top_topCutX->SetParameters(pos_top_topCutX);
    func_pos_top_botCutX = new TF1("func_pos_top_botCutX", "pol1", 50, 390);
    func_pos_top_botCutX->SetParameters(pos_top_botCutX);

    func_neg_top_topCutX = new TF1("func_neg_top_topCutX", "pol1", -300, 40);
    func_neg_top_topCutX->SetParameters(neg_top_topCutX);
    func_neg_top_botCutX = new TF1("func_neg_top_botCutX", "pol1", -300, 40);
    func_neg_top_botCutX->SetParameters(neg_top_botCutX);

    func_pos_bot_topCutX = new TF1("func_pos_bot_topCutX", "pol1", 50, 390);
    func_pos_bot_topCutX->SetParameters(pos_bot_topCutX);
    func_pos_bot_botCutX = new TF1("func_pos_bot_botCutX", "pol1", 50, 390);
    func_pos_bot_botCutX->SetParameters(pos_bot_botCutX);

    func_neg_bot_topCutX = new TF1("func_neg_bot_topCutX", "pol1", -300, 40);
    func_neg_bot_topCutX->SetParameters(neg_bot_topCutX);
    func_neg_bot_botCutX = new TF1("func_neg_bot_botCutX", "pol1", -300, 40);
    func_neg_bot_botCutX->SetParameters(neg_bot_botCutX);

    //Cut functions for Y
    func_pos_top_topCutY = new TF1("func_pos_top_topCutY", "pol1", 30, 90);
    func_pos_top_topCutY->SetParameters(pos_top_topCutY);
    func_pos_top_botCutY = new TF1("func_pos_top_botCutY", "pol1", 30, 90);
    func_pos_top_botCutY->SetParameters(pos_top_botCutY);

    func_neg_top_topCutY = new TF1("func_neg_top_topCutY", "pol1", 30, 90);
    func_neg_top_topCutY->SetParameters(neg_top_topCutY);
    func_neg_top_botCutY = new TF1("func_neg_top_botCutY", "pol1", 30, 90);
    func_neg_top_botCutY->SetParameters(neg_top_botCutY);

    func_pos_bot_topCutY = new TF1("func_pos_bot_topCutY", "pol1", -90, -30);
    func_pos_bot_topCutY->SetParameters(pos_bot_topCutY);
    func_pos_bot_botCutY = new TF1("func_pos_bot_botCutY", "pol1", -90, -30);
    func_pos_bot_botCutY->SetParameters(pos_bot_botCutY);

    func_neg_bot_topCutY = new TF1("func_neg_bot_topCutY", "pol1", -90, -30);
    func_neg_bot_topCutY->SetParameters(neg_bot_topCutY);
    func_neg_bot_botCutY = new TF1("func_neg_bot_botCutY", "pol1", -90, -30);
    func_neg_bot_botCutY->SetParameters(neg_bot_botCutY);

}

bool TriggerValidationAnaProcessor::process(IEvent* ievent) {
    double weight = 1.;

    histos->FillTSData(tsData_);

    //////////// Check time of Ecal clusters, hodo hits, VTP single triggers and their time difference.
	for(int i = 0; i < ecalClusters_->size(); i++){
		CalCluster* ecalCluster = ecalClusters_->at(i);
		double energyEcalCluster = ecalCluster->getEnergy();
		double timeEcalCluster = ecalCluster->getTime();

		CalHit* seed = (CalHit*)ecalCluster->getSeed();

		int ix = seed -> getCrystalIndices()[0];
		int iy = seed -> getCrystalIndices()[1];
		if(ix < 0) ix++;

		if((tsData_->prescaled.Single_2_Top || tsData_->prescaled.Single_2_Bot) && seed->getEnergy() > 0.05){
			if(iy > 0) histos->Fill1DHisto("ecalClusterTime_top_single2_h", timeEcalCluster, weight);
			else histos->Fill1DHisto("ecalClusterTime_bot_single2_h", timeEcalCluster, weight);
		}

		if(ix >= 3){
			histos->Fill1DHisto("ecalClusterTime_h", timeEcalCluster, weight);
			if(iy > 0) histos->Fill1DHisto("ecalClusterTime_top_h", timeEcalCluster, weight);
			else histos->Fill1DHisto("ecalClusterTime_bot_h", timeEcalCluster, weight);


			histos->Fill2DHisto("energy_vs_time_ecalCluster_hh", timeEcalCluster, energyEcalCluster, weight);
		}

		for(int j = 0; j < hodoHits_->size(); j++){
			HodoHit* hodoHit = hodoHits_->at(j);
			double timeHodoHit = hodoHit->getTime();

			histos->Fill1DHisto("timeDiff_ecalCluster_hodoHit_h", timeEcalCluster - timeHodoHit, weight);

			if(iy > 0 && hodoHit->getIndices()[1] > 0) histos->Fill1DHisto("timeDiff_ecalCluster_hodoHit_top_h", timeEcalCluster - timeHodoHit, weight);
			else if (iy < 0 && hodoHit->getIndices()[1] < 0) histos->Fill1DHisto("timeDiff_ecalCluster_hodoHit_bot_h", timeEcalCluster - timeHodoHit, weight);
		}

		std::vector<VTPData::hpsSingleTrig> singleTrigs = vtpData_->singletrigs;
		for(int j = 0; j < singleTrigs.size(); j++){
			VTPData::hpsSingleTrig singleTrig = singleTrigs.at(j);

			if(singleTrig.inst == 2 || singleTrig.inst == 3){
				unsigned int timeSingleTrig = singleTrig.T;

				histos->Fill2DHisto("vtpSingleTrigTime_vs_ecalClusterTime_hh", timeEcalCluster, timeSingleTrig * 4, weight);

				double timeDiffEcalClusterVTPSingleTrigger = timeEcalCluster - timeSingleTrig * 4;
				histos->Fill1DHisto("timeDiff_ecalCluster_VTPSingleTrig_h", timeDiffEcalClusterVTPSingleTrigger, weight);
			}
		}

		std::vector<VTPData::hpsCluster> vtpClusters =  vtpData_->clusters;
		for(int j = 0; j < vtpClusters.size(); j++){
			VTPData::hpsCluster vtpCluster = vtpClusters.at(j);
			double energyDiffEcalClusterVTPCluster = energyEcalCluster - vtpCluster.E / 1000.;
			double energyRatioEcalClusterVTPCluster = energyEcalCluster * 1000. / vtpCluster.E;

			int xDiff = ix - vtpCluster.X;
			int yDiff = iy - vtpCluster.Y;

			histos->Fill1DHisto("xDiff_ecalCluster_VTPClusters_h", xDiff, weight);
			histos->Fill1DHisto("yDiff_ecalCluster_VTPClusters_h", yDiff, weight);

			histos->Fill2DHisto("xDiff_yDiff_ecalCluster_VTPClusters_hh", xDiff, yDiff, weight);

			if(xDiff == 0 && yDiff == 0){
				histos->Fill1DHisto("energyDiff_ecalCluster_VTPClusters_h", energyDiffEcalClusterVTPCluster, weight);
				histos->Fill2DHisto("energyDiff_ecalCluster_VTPClusters_vs_energy_ecalCluster_hh", energyEcalCluster, energyDiffEcalClusterVTPCluster, weight);

				histos->Fill1DHisto("energyRatio_ecalCluster_VTPClusters_h", energyRatioEcalClusterVTPCluster, weight);
				histos->Fill2DHisto("energyRatio_ecalCluster_VTPClusters_vs_energy_ecalCluster_hh", energyEcalCluster, energyRatioEcalClusterVTPCluster, weight);
			}
		}
	}

	for(int j = 0; j < hodoHits_->size(); j++){
		HodoHit* hodoHit = hodoHits_->at(j);
		double timeHodoHit = hodoHit->getTime();
		histos->Fill1DHisto("hodoHitTime_h", timeHodoHit, weight);

		if(hodoHit->getIndices()[1] > 0) histos->Fill1DHisto("hodoHitTime_top_h", timeHodoHit, weight);
		else histos->Fill1DHisto("hodoHitTime_bot_h", timeHodoHit, weight);

		if((tsData_->prescaled.Single_2_Top || tsData_->prescaled.Single_2_Bot) && hodoHit->getEnergy() > FADCHITTHRESHOLD){
			if(hodoHit->getIndices()[1] > 0) histos->Fill1DHisto("hodoHitTime_top_single2_h", timeHodoHit, weight);
			else histos->Fill1DHisto("hodoHitTime_bot_single2_h", timeHodoHit, weight);
		}

	}

	std::vector<VTPData::hpsSingleTrig> singleTrigs = vtpData_->singletrigs;
	for(int j = 0; j < singleTrigs.size(); j++){
		VTPData::hpsSingleTrig singleTrig = singleTrigs.at(j);

		if(singleTrig.inst == 2 || singleTrig.inst == 3){
			unsigned int timeSingleTrig = singleTrig.T;

			histos->Fill1DHisto("vtpSingleTrigTime_h", timeSingleTrig * 4, weight);

			if(singleTrig.topnbot) histos->Fill1DHisto("vtpSingleTrigTime_top_h", timeSingleTrig * 4, weight);
			else histos->Fill1DHisto("vtpSingleTrigTime_bot_h", timeSingleTrig * 4, weight);
		}
	}

    //////////// Do trigger validation
	// Loop ecal clusters
	for(int i = 0; i < ecalClusters_->size(); i++){
		CalCluster* ecalCluster = ecalClusters_->at(i);
		double energyEcalCluster = ecalCluster->getEnergy();
		double timeEcalCluster = ecalCluster->getTime();

		CalHit* seed = (CalHit*)ecalCluster->getSeed();
		int ix = seed->getCrystalIndices()[0];
		int iy = seed->getCrystalIndices()[1];
		if(ix < 0 ) ix++;

	    // With consideration of readout time windows and "edge effects", clusters are extracted with [8, 120] ns for the analysis
		// FADC250_W_WIDTH 192
		// FADC250_W_WIDTH 128
		if( (iy > 0 && timeEcalCluster >= TIMEECALCLUSTERMINTOP && timeEcalCluster <= TIMEECALCLUSTERMAXTOP)
				|| (iy < 0 && timeEcalCluster >= TIMEECALCLUSTERMINBOT && timeEcalCluster <= TIMEECALCLUSTERMAXBOT)){

			// For each cluster, select hodo hits within a time range around the cluster time
			std::vector<HodoHit*> hodoHitVect;
			hodoHitVect.clear();
			for(int j = 0; j < hodoHits_->size(); j++){
				HodoHit* hodoHit = hodoHits_->at(j);
				double timeHodoHit = hodoHit->getTime();
				double timeDiffEcalClusterHodoHit = timeEcalCluster - timeHodoHit;
				if(iy > 0 && timeDiffEcalClusterHodoHit >= TIMEDIFFECALCLUSTERHODOHITMINTOP && timeDiffEcalClusterHodoHit <= TIMEDIFFECALCLUSTERHODOHITMAXTOP)
					hodoHitVect.push_back(hodoHit);
				else if(iy < 0 && timeDiffEcalClusterHodoHit >= TIMEDIFFECALCLUSTERHODOHITMINBOT && timeDiffEcalClusterHodoHit <= TIMEDIFFECALCLUSTERHODOHITMAXBOT)
					hodoHitVect.push_back(hodoHit);
			}

			// Using selected hodo hits, build hodo patterns
			std::map<int, hodoPattern> hodopatternMap;
			hodopatternMap = buildHodoPatternMap(hodoHitVect);

			// Build trigger bit for single2
			singleTriggerTags single2TriTags = buildSingle2TriggerTags(ecalCluster);

			// Build trigger bit for single3
			singleTriggerTags single3TriTags = buildSingle3TriggerTags(ecalCluster, hodopatternMap);

			// Build trigger bit for FEE
			feeTriggerTags feeTriTags = buildFeeTriggerTags(ecalCluster);

			// If a simulated single2 trigger can be formed, check if there is a VTP single2 trigger, where time difference between simulated trigger and VTP trigger is within a time range
			bool flagSingle2Top = false;
			bool flagSingle2Bot = false;

			if(single2TriTags.singleTriggerSet == single2Bits){

				if(iy > 0){

					bool flagSingle3Top = false;

					std::vector<VTPData::hpsSingleTrig> singleTrigs = vtpData_->singletrigs;

					for(int j = 0; j < singleTrigs.size(); j++){
						VTPData::hpsSingleTrig singleTrig = singleTrigs.at(j);
						unsigned int timeSingleTrig = singleTrig.T;
						unsigned int instSingleTrig = singleTrig.inst;
						bool topnbot = singleTrig.topnbot;

						double timeDiffEcalClusterVTPSingleTrigger = timeEcalCluster - timeSingleTrig * 4;

						if(instSingleTrig == 2 && topnbot && timeDiffEcalClusterVTPSingleTrigger >= TIMEDIFFECALCLUSTERVTPSINGLETRIGGERMIN && timeDiffEcalClusterVTPSingleTrigger <= TIMEDIFFECALCLUSTERVTPSINGLETRIGGERMAX
								&& singleTrig.emin && singleTrig.emax && singleTrig.nmin && singleTrig.xmin && singleTrig.pose){
							flagSingle2Top = true;
							//break;
						}

						if(instSingleTrig == 3 && topnbot && timeDiffEcalClusterVTPSingleTrigger >= TIMEDIFFECALCLUSTERVTPSINGLETRIGGERMIN && timeDiffEcalClusterVTPSingleTrigger <= TIMEDIFFECALCLUSTERVTPSINGLETRIGGERMAX
								&& singleTrig.emin && singleTrig.emax && singleTrig.nmin && singleTrig.xmin && singleTrig.pose
								&& singleTrig.hodo1c && singleTrig.hodo2c && singleTrig.hodogeo && singleTrig.hodoecal){

							flagSingle3Top = true;
							//break;
						}
					}

					std::vector<VTPData::hpsCluster> vtpClusters =  vtpData_->clusters;
					std::vector<VTPData::hpsCluster> vtpClustersMatched;
					vtpClustersMatched.clear();

					for(int j = 0; j < vtpClusters.size(); j++){
						VTPData::hpsCluster vtpCluster = vtpClusters.at(j);

						int xDiff = ix - vtpCluster.X;
						int yDiff = iy - vtpCluster.Y;


						if(xDiff == 0 && yDiff == 0) vtpClustersMatched.push_back(vtpCluster);
					}

					double timeDiffMatched = 999;
					int indexMatched = 0;
					for(int j = 0; j < vtpClustersMatched.size(); j++){
						VTPData::hpsCluster vtpCluster = vtpClustersMatched.at(j);
						if(fabs(vtpCluster.T * 4 - timeEcalCluster) <  timeDiffMatched) {
							timeDiffMatched = fabs(vtpCluster.T * 4 - timeEcalCluster);
							indexMatched = j;
						}
					}

					double energyDiffEcalClusterVTPCluster = -999;
					double energyVTPCluster = -999;
					int ixVTPCluster = -999;
					int iyVTPCluster = -999;
					double timeVTPCluster = -999;

					if (vtpClustersMatched.size() >= 1) {
						VTPData::hpsCluster vtpCluster = vtpClustersMatched.at(indexMatched);

						energyDiffEcalClusterVTPCluster = energyEcalCluster - vtpCluster.E / 1000.;
						energyVTPCluster = vtpCluster.E / 1000.;
						ixVTPCluster = vtpCluster.X;
						iyVTPCluster = vtpCluster.Y;
						timeVTPCluster = vtpCluster.T * 4;
					}

					if(flagSingle2Top) {
						passSingle2Top++;

						histos->Fill2DHisto("ecal_energy_x_single2_pass_hh", ix, energyEcalCluster, weight);
					}
					else {
						failSingle2Top++;

						histos->Fill2DHisto("ecal_energy_x_single2_fail_hh", ix, energyEcalCluster, weight);

						histos->Fill2DHisto("ecal_energyDiff_x_single2_fail_hh", ix, energyDiffEcalClusterVTPCluster, weight);

						histos->Fill2DHisto("vtp_energy_x_single2_fail_hh", ixVTPCluster, energyVTPCluster, weight);

						if(energyDiffEcalClusterVTPCluster > 1.5){
							histos->Fill2DHisto("x_vs_y_single2_fail_energyDiffLargerthan1pt5_hh", ix, iy, weight);
							histos->Fill2DHisto("energyGTPCluster_vs_energyEcalCluster_single2_fail_energyDiffLargerthan1pt5_hh", energyEcalCluster, energyVTPCluster, weight);
							histos->Fill2DHisto("timeGTPCluster_vs_timeEcalCluster_single2_fail_energyDiffLargerthan1pt5_hh", timeEcalCluster, timeVTPCluster, weight);
						}

						if(energyDiffEcalClusterVTPCluster < 0){
							histos->Fill2DHisto("x_vs_y_single2_fail_energyDiffLessthan0_hh", ix, iy, weight);
							histos->Fill2DHisto("energyGTPCluster_vs_energyEcalCluster_single2_fail_energyDiffLessthan0_hh", energyEcalCluster, energyVTPCluster, weight);
							histos->Fill2DHisto("timeGTPCluster_vs_timeEcalCluster_single2_fail_energyDiffLessthan0_hh", timeEcalCluster, timeVTPCluster, weight);
						}

						if(flagSingle3Top) failSingle2PassSinge3Top++;
					}
				}
				else{
					bool flagSingle3Bot = false;

					std::vector<VTPData::hpsSingleTrig> singleTrigs = vtpData_->singletrigs;

					for(int j = 0; j < singleTrigs.size(); j++){
						VTPData::hpsSingleTrig singleTrig = singleTrigs.at(j);
						unsigned int timeSingleTrig = singleTrig.T;
						unsigned int instSingleTrig = singleTrig.inst;
						bool topnbot = singleTrig.topnbot;

						double timeDiffEcalClusterVTPSingleTrigger = timeEcalCluster - timeSingleTrig * 4;

						if(instSingleTrig == 2 && !topnbot && timeDiffEcalClusterVTPSingleTrigger >= TIMEDIFFECALCLUSTERVTPSINGLETRIGGERMIN && timeDiffEcalClusterVTPSingleTrigger <= TIMEDIFFECALCLUSTERVTPSINGLETRIGGERMAX
								&& singleTrig.emin && singleTrig.emax && singleTrig.nmin && singleTrig.xmin && singleTrig.pose){
							flagSingle2Bot = true;
							//break;
						}

						if(instSingleTrig == 3 && !topnbot && timeDiffEcalClusterVTPSingleTrigger >= TIMEDIFFECALCLUSTERVTPSINGLETRIGGERMIN && timeDiffEcalClusterVTPSingleTrigger <= TIMEDIFFECALCLUSTERVTPSINGLETRIGGERMAX
								&& singleTrig.emin && singleTrig.emax && singleTrig.nmin && singleTrig.xmin && singleTrig.pose
								&& singleTrig.hodo1c && singleTrig.hodo2c && singleTrig.hodogeo && singleTrig.hodoecal){
							flagSingle3Bot = true;
							//break;
						}
					}

					std::vector<VTPData::hpsCluster> vtpClusters =  vtpData_->clusters;
					std::vector<VTPData::hpsCluster> vtpClustersMatched;
					vtpClustersMatched.clear();

					for(int j = 0; j < vtpClusters.size(); j++){
						VTPData::hpsCluster vtpCluster = vtpClusters.at(j);

						int xDiff = ix - vtpCluster.X;
						int yDiff = iy - vtpCluster.Y;


						if(xDiff == 0 && yDiff == 0) vtpClustersMatched.push_back(vtpCluster);
					}

					double timeDiffMatched = 9999999;
					int indexMatched = 0;
					for(int j = 0; j < vtpClustersMatched.size(); j++){
						VTPData::hpsCluster vtpCluster = vtpClustersMatched.at(j);
						if(fabs(vtpCluster.T * 4 - timeEcalCluster) <  timeDiffMatched) {
							timeDiffMatched = fabs(vtpCluster.T * 4 - timeEcalCluster);
							indexMatched = j;
						}
					}

					double energyDiffEcalClusterVTPCluster = -999;
					double energyVTPCluster = -999;
					int ixVTPCluster = -999;
					int iyVTPCluster = -999;
					double timeVTPCluster = -999;

					if (vtpClustersMatched.size() >= 1) {
						VTPData::hpsCluster vtpCluster = vtpClustersMatched.at(indexMatched);

						energyDiffEcalClusterVTPCluster = energyEcalCluster - vtpCluster.E / 1000.;
						energyVTPCluster = vtpCluster.E / 1000.;
						ixVTPCluster = vtpCluster.X;
						iyVTPCluster = vtpCluster.Y;
						timeVTPCluster = vtpCluster.T * 4;
					}

					if(flagSingle2Bot) {
						passSingle2Bot++;

						histos->Fill2DHisto("ecal_energy_x_single2_pass_hh", ix, energyEcalCluster, weight);
					}
					else {
						failSingle2Bot++;

						histos->Fill2DHisto("ecal_energy_x_single2_fail_hh", ix, energyEcalCluster, weight);

						histos->Fill2DHisto("ecal_energyDiff_x_single2_fail_hh", ix, energyDiffEcalClusterVTPCluster, weight);

						histos->Fill2DHisto("vtp_energy_x_single2_fail_hh", ixVTPCluster, energyVTPCluster, weight);

						if(energyDiffEcalClusterVTPCluster > 1.5){
							histos->Fill2DHisto("x_vs_y_single2_fail_energyDiffLargerthan1pt5_hh", ix, iy, weight);
							histos->Fill2DHisto("energyGTPCluster_vs_energyEcalCluster_single2_fail_energyDiffLargerthan1pt5_hh", energyEcalCluster, energyVTPCluster, weight);
							histos->Fill2DHisto("timeGTPCluster_vs_timeEcalCluster_single2_fail_energyDiffLargerthan1pt5_hh", timeEcalCluster, timeVTPCluster, weight);
						}

						if(energyDiffEcalClusterVTPCluster < 0){
							histos->Fill2DHisto("x_vs_y_single2_fail_energyDiffLessthan0_hh", ix, iy, weight);
							histos->Fill2DHisto("energyGTPCluster_vs_energyEcalCluster_single2_fail_energyDiffLessthan0_hh", energyEcalCluster, energyVTPCluster, weight);
							histos->Fill2DHisto("timeGTPCluster_vs_timeEcalCluster_single2_fail_energyDiffLessthan0_hh", timeEcalCluster, timeVTPCluster, weight);
						}

						if(flagSingle3Bot) failSingle2PassSinge3Bot++;
					}
				}
			}

			// If a simulated single3 trigger can be formed, check if there is a VTP single3 trigger, where time difference between simulated trigger and VTP trigger is within a time range
			bool flagSingle3Top = false;
			bool flagSingle3Bot = false;
			if(single3TriTags.singleTriggerSet == single3Bits){
				if( iy > 0 ){

					std::vector<VTPData::hpsSingleTrig> singleTrigs = vtpData_->singletrigs;

					for(int j = 0; j < singleTrigs.size(); j++){
						VTPData::hpsSingleTrig singleTrig = singleTrigs.at(j);
						unsigned int timeSingleTrig = singleTrig.T;
						unsigned int instSingleTrig = singleTrig.inst;
						bool topnbot = singleTrig.topnbot;

						double timeDiffEcalClusterVTPSingleTrigger = timeEcalCluster - timeSingleTrig * 4;

						if(instSingleTrig == 3 && topnbot && timeDiffEcalClusterVTPSingleTrigger >= TIMEDIFFECALCLUSTERVTPSINGLETRIGGERMIN && timeDiffEcalClusterVTPSingleTrigger <= TIMEDIFFECALCLUSTERVTPSINGLETRIGGERMAX
								&& singleTrig.emin && singleTrig.emax && singleTrig.nmin && singleTrig.xmin && singleTrig.pose
								&& singleTrig.hodo1c && singleTrig.hodo2c && singleTrig.hodogeo && singleTrig.hodoecal){

							//std::cout << singleTrig.emin << "  " << singleTrig.emax << "  " << singleTrig.nmin << "  " << singleTrig.xmin << "  " << singleTrig.pose
							//									<< "  " << singleTrig.hodo1c << "  " << singleTrig.hodo2c << "  " << singleTrig.hodogeo << "  " << singleTrig.hodoecal << std::endl;

							flagSingle3Top = true;
							break;
						}
					}

					if(flagSingle3Top){
						passSingle3Top++;

						histos->Fill2DHisto("ecal_energy_x_single3_pass_hh", ix, energyEcalCluster, weight);
					}
					else{
						failSingle3Top++;

						histos->Fill2DHisto("ecal_energy_x_single3_fail_hh", ix, energyEcalCluster, weight);
					}
				}
				else{
					std::vector<VTPData::hpsSingleTrig> singleTrigs = vtpData_->singletrigs;

					for(int j = 0; j < singleTrigs.size(); j++){
						VTPData::hpsSingleTrig singleTrig = singleTrigs.at(j);
						unsigned int timeSingleTrig = singleTrig.T;
						unsigned int instSingleTrig = singleTrig.inst;
						bool topnbot = singleTrig.topnbot;

						double timeDiffEcalClusterVTPSingleTrigger = timeEcalCluster - timeSingleTrig * 4;

						if(instSingleTrig == 3 && !topnbot && timeDiffEcalClusterVTPSingleTrigger >= TIMEDIFFECALCLUSTERVTPSINGLETRIGGERMIN && timeDiffEcalClusterVTPSingleTrigger <= TIMEDIFFECALCLUSTERVTPSINGLETRIGGERMAX
								&& singleTrig.emin && singleTrig.emax && singleTrig.nmin && singleTrig.xmin && singleTrig.pose
								&& singleTrig.hodo1c && singleTrig.hodo2c && singleTrig.hodogeo && singleTrig.hodoecal){
							flagSingle3Bot = true;
							break;
						}
					}

					if(flagSingle3Bot){
						passSingle3Bot++;

						histos->Fill2DHisto("ecal_energy_x_single3_pass_hh", ix, energyEcalCluster, weight);
					}
					else{
						failSingle3Bot++;

						histos->Fill2DHisto("ecal_energy_x_single3_fail_hh", ix, energyEcalCluster, weight);
					}
				}
			}

			if(flagSingle2Top && flagSingle3Top) passSinge2Single3Top++;
			if(flagSingle2Bot && flagSingle3Bot) passSinge2Single3Bot++;

			if(flagSingle2Top && !flagSingle3Top)  histos->Fill2DHisto("ecal_energy_x_single2Pass_single3Fail_hh", ix, energyEcalCluster, weight);
			if(flagSingle2Bot && !flagSingle3Bot)  histos->Fill2DHisto("ecal_energy_x_single2Pass_single3Fail_hh", ix, energyEcalCluster, weight);

			if(!flagSingle2Top && flagSingle3Top)  histos->Fill2DHisto("ecal_energy_x_single2Fail_single3Pass_hh", ix, energyEcalCluster, weight);
			if(!flagSingle2Bot && flagSingle3Bot)  histos->Fill2DHisto("ecal_energy_x_single2Fail_single3Pass_hh", ix, energyEcalCluster, weight);

			// If a simulated FEE trigger can be formed, check if there is a VTP FEE trigger, where time difference between simulated trigger and VTP trigger is within a time range
			// Since prescales in regions for FEE trigger in hardware, just small part of FEE triggers with satisfactory of EMIN, EMAX and NHIT are stored in VTP bank.
			// Therefore, even if a simulated FEE trigger is formed, a corresponding FEE trigger is not necessary to exist in VTP bank.

			if(feeTriTags.feeTriggerSet == feeBits){
				bool flagFee = false;

				std::vector<VTPData::hpsFEETrig> feeTrigs = vtpData_->feetrigger;

				for(int j = 0; j < feeTrigs.size(); j++){
					VTPData::hpsFEETrig feeTrig = feeTrigs.at(j);
					unsigned int timeFeeTrig = feeTrig.T;

					double timeDiffEcalClusterVTPSingleTrigger = timeEcalCluster - timeFeeTrig * 4;

					if(timeDiffEcalClusterVTPSingleTrigger >= TIMEDIFFECALCLUSTERVTPSINGLETRIGGERMIN && timeDiffEcalClusterVTPSingleTrigger <= TIMEDIFFECALCLUSTERVTPSINGLETRIGGERMAX){
						flagFee = true;
						break;
					}
				}

				if(flagFee) passFee++;
				else failFee++;
			}
		}
	}

	#ifdef __WITHSVT__
	////// Track-cluster matching

	// Separate all tracks into four categories
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

		std::vector<double> positionAtEcal = track->getPositionAtEcal();
		if(!isnan(positionAtEcal[2])) {
			histos->Fill1DHisto("chi2ndf_tracks_h", track->getChi2Ndf(), weight);
			histos->Fill1DHisto("p_tracks_h", track->getP(), weight);

			// Cuts for tracks
			if(track->getChi2Ndf() < CHI2NDFTHRESHOLD){ // && track->getP() > TRACKMOMENTUMMIN

				if (charge == 1 && positionAtEcal[1] > 0) tracks_pos_top.push_back(*track);
				else if (charge == 1 && positionAtEcal[1] < 0) tracks_pos_bot.push_back(*track);
				else if (charge == -1 && positionAtEcal[1] > 0) tracks_neg_top.push_back(*track);
				else if (charge == -1 && positionAtEcal[1] < 0) tracks_neg_bot.push_back(*track);
			}
		}
	}

	histos->Fill1DHisto("num_pos_tracks_h", tracks_pos_top.size() + tracks_pos_bot.size(), weight);
	histos->Fill1DHisto("num_neg_tracks_h", tracks_neg_top.size() + tracks_neg_bot.size(), weight);


	// Study single2 and single3 with tracks
	bool tsBitSingle2 = tsData_->prescaled.Single_2_Top || tsData_->prescaled.Single_2_Bot;
	bool tsBitSingle3 = tsData_->prescaled.Single_3_Top || tsData_->prescaled.Single_3_Bot;

	if(tsBitSingle2) tsSl2T++;
	if(tsBitSingle3) tsSl3T++;
	if(tsBitSingle2 && tsBitSingle3) tsSl2TSl3T++;
	if(tsBitSingle2 && !tsBitSingle3) tsSl2TSl3F++;
	if(!tsBitSingle2 && tsBitSingle3) tsSl2FSl3T++;
	if(!tsBitSingle2 && !tsBitSingle3) tsSl2FSl3F++;

	if(tracks_pos_top.size() + tracks_pos_bot.size() > 0 ){
		if(tsBitSingle2) tsSl2T_P++;
		if(tsBitSingle3) tsSl3T_P++;
		if(tsBitSingle2 && tsBitSingle3) tsSl2TSl3T_P++;
		if(tsBitSingle2 && !tsBitSingle3) tsSl2TSl3F_P++;
		if(!tsBitSingle2 && tsBitSingle3) tsSl2FSl3T_P++;
		if(!tsBitSingle2 && !tsBitSingle3) tsSl2FSl3F_P++;
	}

	if( (tracks_pos_top.size() + tracks_pos_bot.size() > 0) && (tracks_neg_top.size() + tracks_neg_bot.size() > 0) ){
		if(tsBitSingle2) tsSl2T_PN++;
		if(tsBitSingle3) tsSl3T_PN++;
		if(tsBitSingle2 && tsBitSingle3) tsSl2TSl3T_PN++;
		if(tsBitSingle2 && !tsBitSingle3) tsSl2TSl3F_PN++;
		if(!tsBitSingle2 && tsBitSingle3) tsSl2FSl3T_PN++;
		if(!tsBitSingle2 && !tsBitSingle3) tsSl2FSl3F_PN++;
	}

	if( (tracks_pos_top.size() + tracks_pos_bot.size() == 1) && (tracks_neg_top.size() + tracks_neg_bot.size() == 1) ){
		if(tsBitSingle2) tsSl2T_1P1N++;
		if(tsBitSingle3) tsSl3T_1P1N++;
		if(tsBitSingle2 && tsBitSingle3) tsSl2TSl3T_1P1N++;
		if(tsBitSingle2 && !tsBitSingle3) tsSl2TSl3F_1P1N++;
		if(!tsBitSingle2 && tsBitSingle3) tsSl2FSl3T_1P1N++;
		if(!tsBitSingle2 && !tsBitSingle3) tsSl2FSl3F_1P1N++;
	}


	// Separate all clusters into four categories
	std::vector<CalCluster> clulsters_pos_top;
	std::vector<CalCluster> clulsters_neg_top;
	std::vector<CalCluster> clulsters_pos_bot;
	std::vector<CalCluster> clulsters_neg_bot;

	for(int i = 0; i < ecalClusters_->size(); i++){
		CalCluster* ecalCluster = ecalClusters_->at(i);
		std::vector<double> positionCluster = ecalCluster->getPosition();
		CalHit* seed = (CalHit*)ecalCluster->getSeed();

		int ix = seed -> getCrystalIndices()[0];
		int iy = seed -> getCrystalIndices()[1];

		if(iy > 0 ){
			if(ix > 0) clulsters_pos_top.push_back(*ecalCluster);
			else clulsters_neg_top.push_back(*ecalCluster);
		}
		else {
			if(ix > 0) clulsters_pos_bot.push_back(*ecalCluster);
			else clulsters_neg_bot.push_back(*ecalCluster);
		}
	}


	// Make plots for each category and find matched clusters
	std::vector<CalCluster> clulsters_pos_top_matched;
	std::vector<CalCluster> clulsters_neg_top_matched;
	std::vector<CalCluster> clulsters_pos_bot_matched;
	std::vector<CalCluster> clulsters_neg_bot_matched;

	std::vector<int> trackIndices_pos_top;
	std::vector<int> trackIndices_pos_bot;
	std::vector<int> trackIndices_neg_top;
	std::vector<int> trackIndices_neg_bot;

	// Positive top category
	for(int i = 0; i < clulsters_pos_top.size(); i++){
		CalCluster cluster = clulsters_pos_top.at(i);
		std::vector<double> positionCluster = cluster.getPosition();
		for(int j = 0; j < tracks_pos_top.size(); j++){
			Track track = tracks_pos_top.at(j);
			std::vector<double> positionAtEcal = track.getPositionAtEcal();

			histos->Fill2DHisto("trackX_vs_ClusterX_pos_top_hh", positionCluster[0], positionAtEcal[0], weight);
			histos->Fill2DHisto("trackY_vs_ClusterY_pos_top_hh", positionCluster[1], positionAtEcal[1], weight);

			if (positionAtEcal[0]< func_pos_top_topCutX->Eval(positionCluster[0])
					&& positionAtEcal[0] > func_pos_top_botCutX->Eval(positionCluster[0])
					&& positionAtEcal[1] < func_pos_top_topCutY->Eval(positionCluster[1])
					&& positionAtEcal[1] > func_pos_top_botCutY->Eval(positionCluster[1])) {
				clulsters_pos_top_matched.push_back(cluster);
				trackIndices_pos_top.push_back(j);
				break;
			}
		}
	}

	// Positive bottom category
	for(int i = 0; i < clulsters_pos_bot.size(); i++){
		CalCluster cluster = clulsters_pos_bot.at(i);
		std::vector<double> positionCluster = cluster.getPosition();
		for(int j = 0; j < tracks_pos_bot.size(); j++){
			Track track = tracks_pos_bot.at(j);
			std::vector<double> positionAtEcal = track.getPositionAtEcal();

			histos->Fill2DHisto("trackX_vs_ClusterX_pos_bot_hh", positionCluster[0], positionAtEcal[0], weight);
			histos->Fill2DHisto("trackY_vs_ClusterY_pos_bot_hh", positionCluster[1], positionAtEcal[1], weight);

			if (positionAtEcal[0]< func_pos_bot_topCutX->Eval(positionCluster[0])
					&& positionAtEcal[0] > func_pos_bot_botCutX->Eval(positionCluster[0])
					&& positionAtEcal[1] < func_pos_bot_topCutY->Eval(positionCluster[1])
					&& positionAtEcal[1] > func_pos_bot_botCutY->Eval(positionCluster[1])) {
				clulsters_pos_bot_matched.push_back(cluster);
				trackIndices_pos_bot.push_back(j);
				break;
			}
		}
	}

	// Negative top category
	for(int i = 0; i < clulsters_neg_top.size(); i++){
		CalCluster cluster = clulsters_neg_top.at(i);
		std::vector<double> positionCluster = cluster.getPosition();
		for(int j = 0; j < tracks_neg_top.size(); j++){
			Track track = tracks_neg_top.at(j);
			std::vector<double> positionAtEcal = track.getPositionAtEcal();

			histos->Fill2DHisto("trackX_vs_ClusterX_neg_top_hh", positionCluster[0], positionAtEcal[0], weight);
			histos->Fill2DHisto("trackY_vs_ClusterY_neg_top_hh", positionCluster[1], positionAtEcal[1], weight);

			if (positionAtEcal[0]< func_neg_top_topCutX->Eval(positionCluster[0])
					&& positionAtEcal[0] > func_neg_top_botCutX->Eval(positionCluster[0])
					&& positionAtEcal[1] < func_neg_top_topCutY->Eval(positionCluster[1])
					&& positionAtEcal[1] > func_neg_top_botCutY->Eval(positionCluster[1])) {
				clulsters_neg_top_matched.push_back(cluster);
				trackIndices_neg_top.push_back(j);
				break;
			}
		}
	}

	// Negative bottom category
	for(int i = 0; i < clulsters_neg_bot.size(); i++){
		CalCluster cluster = clulsters_neg_bot.at(i);
		std::vector<double> positionCluster = cluster.getPosition();
		for(int j = 0; j < tracks_neg_bot.size(); j++){
			Track track = tracks_neg_bot.at(j);
			std::vector<double> positionAtEcal = track.getPositionAtEcal();

			histos->Fill2DHisto("trackX_vs_ClusterX_neg_bot_hh", positionCluster[0], positionAtEcal[0], weight);
			histos->Fill2DHisto("trackY_vs_ClusterY_neg_bot_hh", positionCluster[1], positionAtEcal[1], weight);

			if (positionAtEcal[0]< func_neg_bot_topCutX->Eval(positionCluster[0])
					&& positionAtEcal[0] > func_neg_bot_botCutX->Eval(positionCluster[0])
					&& positionAtEcal[1] < func_neg_bot_topCutY->Eval(positionCluster[1])
					&& positionAtEcal[1] > func_neg_bot_botCutY->Eval(positionCluster[1])) {
				clulsters_neg_bot_matched.push_back(cluster);
				trackIndices_neg_bot.push_back(j);
				break;
			}
		}
	}

	// Find matched VTP clusters
	std::vector<VTPData::hpsCluster> vtpClusters =  vtpData_->clusters;
	std::vector<VTPData::hpsCluster> vtpClustersMatched_pos_top;
	std::vector<VTPData::hpsCluster> vtpClustersMatched_pos_bot;
	std::vector<VTPData::hpsCluster> vtpClustersMatched_neg_top;
	std::vector<VTPData::hpsCluster> vtpClustersMatched_neg_bot;

	std::vector<int> trackIndices_pos_top_vtpClustersMatched;
	std::vector<int> trackIndices_pos_bot_vtpClustersMatched;
	std::vector<int> trackIndices_neg_top_vtpClustersMatched;
	std::vector<int> trackIndices_neg_bot_vtpClustersMatched;

	for(int i = 0; i < vtpClusters.size(); i++){
		VTPData::hpsCluster vtpCluster = vtpClusters.at(i);

		int xVTP = vtpCluster.X;
		int yVTP = vtpCluster.Y;

		if(xVTP > 0 && yVTP > 0 ){
			for(int j = 0; j < clulsters_pos_top_matched.size(); j++){
				CalCluster cluster = clulsters_pos_top_matched.at(j);
				double energy = cluster.getEnergy();
				CalHit* seed = (CalHit*)cluster.getSeed();

				int ix = seed -> getCrystalIndices()[0];
				int iy = seed -> getCrystalIndices()[1];
				if(ix< 0) ix++;

				int xDiff = ix - xVTP;
				int yDiff = iy - yVTP;

				double timeDiff = cluster.getTime() - vtpCluster.T * 4;

				if(xDiff == 0 && yDiff == 0 && timeDiff > TIMEDIFFECALCLUSTERVTPSINGLETRIGGERMIN && timeDiff < TIMEDIFFECALCLUSTERVTPSINGLETRIGGERMAX ){
					vtpClustersMatched_pos_top.push_back(vtpCluster);
					trackIndices_pos_top_vtpClustersMatched.push_back(trackIndices_pos_top[j]);
				}
			}
		}

		if(xVTP > 0 && yVTP < 0 ){
			for(int j = 0; j < clulsters_pos_bot_matched.size(); j++){
				CalCluster cluster = clulsters_pos_bot_matched.at(j);
				double energy = cluster.getEnergy();
				CalHit* seed = (CalHit*)cluster.getSeed();

				int ix = seed -> getCrystalIndices()[0];
				int iy = seed -> getCrystalIndices()[1];
				if(ix< 0) ix++;

				int xDiff = ix - xVTP;
				int yDiff = iy - yVTP;

				double timeDiff = cluster.getTime() - vtpCluster.T * 4;

				if(xDiff == 0 && yDiff == 0 && timeDiff > TIMEDIFFECALCLUSTERVTPSINGLETRIGGERMIN && timeDiff < TIMEDIFFECALCLUSTERVTPSINGLETRIGGERMAX ){
					vtpClustersMatched_pos_bot.push_back(vtpCluster);
					trackIndices_pos_bot_vtpClustersMatched.push_back(trackIndices_pos_bot[j]);
				}
			}
		}

		if(xVTP <= 0 && yVTP > 0 ){
			for(int j = 0; j < clulsters_neg_top_matched.size(); j++){
				CalCluster cluster = clulsters_neg_top_matched.at(j);
				double energy = cluster.getEnergy();
				CalHit* seed = (CalHit*)cluster.getSeed();

				int ix = seed -> getCrystalIndices()[0];
				int iy = seed -> getCrystalIndices()[1];
				if(ix< 0) ix++;

				int xDiff = ix - xVTP;
				int yDiff = iy - yVTP;

				double timeDiff = cluster.getTime() - vtpCluster.T * 4;

				if(xDiff == 0 && yDiff == 0 && timeDiff > TIMEDIFFECALCLUSTERVTPSINGLETRIGGERMIN && timeDiff < TIMEDIFFECALCLUSTERVTPSINGLETRIGGERMAX ){
					vtpClustersMatched_neg_top.push_back(vtpCluster);
					trackIndices_neg_top_vtpClustersMatched.push_back(trackIndices_neg_top[j]);
				}
			}
		}

		if(xVTP <= 0 && yVTP < 0 ){
			for(int j = 0; j < clulsters_neg_bot_matched.size(); j++){
				CalCluster cluster = clulsters_neg_bot_matched.at(j);
				double energy = cluster.getEnergy();
				CalHit* seed = (CalHit*)cluster.getSeed();

				int ix = seed -> getCrystalIndices()[0];
				int iy = seed -> getCrystalIndices()[1];
				if(ix< 0) ix++;

				int xDiff = ix - xVTP;
				int yDiff = iy - yVTP;

				double timeDiff = cluster.getTime() - vtpCluster.T * 4;

				if(xDiff == 0 && yDiff == 0 && timeDiff > TIMEDIFFECALCLUSTERVTPSINGLETRIGGERMIN && timeDiff < TIMEDIFFECALCLUSTERVTPSINGLETRIGGERMAX ){
					vtpClustersMatched_neg_bot.push_back(vtpCluster);
					trackIndices_neg_bot_vtpClustersMatched.push_back(trackIndices_neg_bot[j]);
				}
			}
		}

	}

	////// Plot Energy and PDE distributions
	// Require that events have only one postive and one negative tracks

	// Ecal clusters matched with tracks
	// VTP clusters matched with Ecal clusters
	if(tracks_pos_top.size() == 1 && tracks_neg_bot.size() == 1){
		for(int i = 0; i < clulsters_pos_top_matched.size(); i++){
			CalCluster cluster = clulsters_pos_top_matched.at(i);
			double energy = cluster.getEnergy();
			CalHit* seed = (CalHit*)cluster.getSeed();

			int ix = seed -> getCrystalIndices()[0];
			int iy = seed -> getCrystalIndices()[1];
			if(ix< 0) ix++;

			histos->Fill1DHisto("ecalCluster_energy_events_with_1P1N_tracks_h", energy, weight);
			histos->Fill2DHisto("ecalCluster_energy_x_events_with_1P1N_tracks_hh", ix, energy, weight);

			if(tsBitSingle2) histos->Fill2DHisto("ecalCluster_energy_x_events_with_1P1N_tracks_single2_hh", ix, energy, weight);
			if(tsBitSingle3) histos->Fill2DHisto("ecalCluster_energy_x_events_with_1P1N_tracks_single3_hh", ix, energy, weight);
		}

		for(int i = 0; i < clulsters_neg_bot_matched.size(); i++){
			CalCluster cluster = clulsters_neg_bot_matched.at(i);
			double energy = cluster.getEnergy();
			CalHit* seed = (CalHit*)cluster.getSeed();

			int ix = seed -> getCrystalIndices()[0];
			int iy = seed -> getCrystalIndices()[1];
			if(ix< 0) ix++;

			histos->Fill1DHisto("ecalCluster_energy_events_with_1P1N_tracks_h", energy, weight);
			//histos->Fill2DHisto("ecalCluster_energy_x_events_with_1P1N_tracks_hh", ix, energy, weight);

			//if(tsBitSingle2) histos->Fill2DHisto("ecalCluster_energy_x_events_with_1P1N_tracks_single2_hh", ix, energy, weight);
			//if(tsBitSingle3) histos->Fill2DHisto("ecalCluster_energy_x_events_with_1P1N_tracks_single3_hh", ix, energy, weight);
		}

		for(int i = 0; i < vtpClustersMatched_pos_top.size(); i++){
			VTPData::hpsCluster vtpCluster = vtpClustersMatched_pos_top.at(i);
			double energy = vtpCluster.E / 1000.;

			int ix = vtpCluster.X;
			int iy = vtpCluster.Y;

			histos->Fill1DHisto("vtpCluster_energy_events_with_1P1N_tracks_h", energy, weight);
			histos->Fill2DHisto("vtpCluster_energy_x_events_with_1P1N_tracks_hh", ix, energy, weight);

			if(tsBitSingle2 && !tsBitSingle3){
				histos->Fill2DHisto("vtpCluster_energy_x_events_with_1P1N_tracks_Sl2TSl3F_hh", ix, energy, weight);

				Track track = tracks_pos_top.at(trackIndices_pos_top_vtpClustersMatched[i]);
				double p = track.getP();
				std::vector<double> mom = track.getMomentum();

				if(ix >= 19){
					histos->Fill1DHisto("p_right_corner_vtpCluster_energy_x_events_with_1P1N_tracks_Sl2TSl3F_h", p, weight);
					histos->Fill1DHisto("px_right_corner_vtpCluster_energy_x_events_with_1P1N_tracks_Sl2TSl3F_h", mom[0], weight);
					histos->Fill1DHisto("py_right_corner_vtpCluster_energy_x_events_with_1P1N_tracks_Sl2TSl3F_h", mom[1], weight);
					histos->Fill1DHisto("pz_right_corner_vtpCluster_energy_x_events_with_1P1N_tracks_Sl2TSl3F_h", mom[2], weight);

				}

				if(ix == 5){
					histos->Fill1DHisto("p_ix5_vtpCluster_energy_x_events_with_1P1N_tracks_Sl2TSl3F_h", p, weight);
					histos->Fill1DHisto("px_ix5_vtpCluster_energy_x_events_with_1P1N_tracks_Sl2TSl3F_h", mom[0], weight);
					histos->Fill1DHisto("py_ix5_vtpCluster_energy_x_events_with_1P1N_tracks_Sl2TSl3F_h", mom[1], weight);
					histos->Fill1DHisto("pz_ix5_vtpCluster_energy_x_events_with_1P1N_tracks_Sl2TSl3F_h", mom[2], weight);

				}
			}

			if(!tsBitSingle2 && tsBitSingle3)
				histos->Fill2DHisto("vtpCluster_energy_x_events_with_1P1N_tracks_Sl2FSl3T_hh", ix, energy, weight);

			if(tsBitSingle2) histos->Fill2DHisto("vtpCluster_energy_x_events_with_1P1N_tracks_single2_hh", ix, energy, weight);
			if(tsBitSingle3) histos->Fill2DHisto("vtpCluster_energy_x_events_with_1P1N_tracks_single3_hh", ix, energy, weight);
		}

		for(int i = 0; i < vtpClustersMatched_neg_bot.size(); i++){
			VTPData::hpsCluster vtpCluster = vtpClustersMatched_neg_bot.at(i);
			double energy = vtpCluster.E / 1000.;

			int ix = vtpCluster.X;
			int iy = vtpCluster.Y;

			histos->Fill1DHisto("vtpCluster_energy_events_with_1P1N_tracks_h", energy, weight);
			//histos->Fill2DHisto("vtpCluster_energy_x_events_with_1P1N_tracks_hh", ix, energy, weight);

			//if(tsBitSingle2 && !tsBitSingle3)
				//histos->Fill2DHisto("vtpCluster_energy_x_events_with_1P1N_tracks_Sl2TSl3F_hh", ix, energy, weight);


			//if(!tsBitSingle2 && tsBitSingle3)
				//histos->Fill2DHisto("vtpCluster_energy_x_events_with_1P1N_tracks_Sl2FSl3T_hh", ix, energy, weight);

			//if(tsBitSingle2) histos->Fill2DHisto("vtpCluster_energy_x_events_with_1P1N_tracks_single2_hh", ix, energy, weight);
			//if(tsBitSingle3) histos->Fill2DHisto("vtpCluster_energy_x_events_with_1P1N_tracks_single3_hh", ix, energy, weight);
		}
	}


	if(tracks_pos_bot.size() == 1 && tracks_neg_top.size() == 1){
		for(int i = 0; i < clulsters_pos_bot_matched.size(); i++){
			CalCluster cluster = clulsters_pos_bot_matched.at(i);
			double energy = cluster.getEnergy();
			CalHit* seed = (CalHit*)cluster.getSeed();

			int ix = seed -> getCrystalIndices()[0];
			int iy = seed -> getCrystalIndices()[1];
			if(ix< 0) ix++;

			histos->Fill1DHisto("ecalCluster_energy_events_with_1P1N_tracks_h", energy, weight);
			histos->Fill2DHisto("ecalCluster_energy_x_events_with_1P1N_tracks_hh", ix, energy, weight);

			if(tsBitSingle2) histos->Fill2DHisto("ecalCluster_energy_x_events_with_1P1N_tracks_single2_hh", ix, energy, weight);
			if(tsBitSingle3) histos->Fill2DHisto("ecalCluster_energy_x_events_with_1P1N_tracks_single3_hh", ix, energy, weight);
		}

		for(int i = 0; i < clulsters_neg_top_matched.size(); i++){
			CalCluster cluster = clulsters_neg_top_matched.at(i);
			double energy = cluster.getEnergy();
			CalHit* seed = (CalHit*)cluster.getSeed();

			int ix = seed -> getCrystalIndices()[0];
			int iy = seed -> getCrystalIndices()[1];
			if(ix< 0) ix++;

			histos->Fill1DHisto("ecalCluster_energy_events_with_1P1N_tracks_h", energy, weight);
			//histos->Fill2DHisto("ecalCluster_energy_x_events_with_1P1N_tracks_hh", ix, energy, weight);

			//if(tsBitSingle2) histos->Fill2DHisto("ecalCluster_energy_x_events_with_1P1N_tracks_single2_hh", ix, energy, weight);
			//if(tsBitSingle3) histos->Fill2DHisto("ecalCluster_energy_x_events_with_1P1N_tracks_single3_hh", ix, energy, weight);
		}

		for(int i = 0; i < vtpClustersMatched_pos_bot.size(); i++){
			VTPData::hpsCluster vtpCluster = vtpClustersMatched_pos_bot.at(i);
			double energy = vtpCluster.E / 1000.;

			int ix = vtpCluster.X;
			int iy = vtpCluster.Y;

			histos->Fill1DHisto("vtpCluster_energy_events_with_1P1N_tracks_h", energy, weight);
			histos->Fill2DHisto("vtpCluster_energy_x_events_with_1P1N_tracks_hh", ix, energy, weight);

			if(tsBitSingle2 && !tsBitSingle3){
				histos->Fill2DHisto("vtpCluster_energy_x_events_with_1P1N_tracks_Sl2TSl3F_hh", ix, energy, weight);

				Track track = tracks_pos_bot.at(trackIndices_pos_bot_vtpClustersMatched[i]);
				double p = track.getP();
				std::vector<double> mom = track.getMomentum();

				if(ix >= 19){
					histos->Fill1DHisto("p_right_corner_vtpCluster_energy_x_events_with_1P1N_tracks_Sl2TSl3F_h", p, weight);
					histos->Fill1DHisto("px_right_corner_vtpCluster_energy_x_events_with_1P1N_tracks_Sl2TSl3F_h", mom[0], weight);
					histos->Fill1DHisto("py_right_corner_vtpCluster_energy_x_events_with_1P1N_tracks_Sl2TSl3F_h", mom[1], weight);
					histos->Fill1DHisto("pz_right_corner_vtpCluster_energy_x_events_with_1P1N_tracks_Sl2TSl3F_h", mom[2], weight);
				}

				if(ix == 5){
					histos->Fill1DHisto("p_ix5_vtpCluster_energy_x_events_with_1P1N_tracks_Sl2TSl3F_h", p, weight);
					histos->Fill1DHisto("px_ix5_vtpCluster_energy_x_events_with_1P1N_tracks_Sl2TSl3F_h", mom[0], weight);
					histos->Fill1DHisto("py_ix5_vtpCluster_energy_x_events_with_1P1N_tracks_Sl2TSl3F_h", mom[1], weight);
					histos->Fill1DHisto("pz_ix5_vtpCluster_energy_x_events_with_1P1N_tracks_Sl2TSl3F_h", mom[2], weight);

				}
			}

			if(!tsBitSingle2 && tsBitSingle3)
				histos->Fill2DHisto("vtpCluster_energy_x_events_with_1P1N_tracks_Sl2FSl3T_hh", ix, energy, weight);

			if(tsBitSingle2) histos->Fill2DHisto("vtpCluster_energy_x_events_with_1P1N_tracks_single2_hh", ix, energy, weight);
			if(tsBitSingle3) histos->Fill2DHisto("vtpCluster_energy_x_events_with_1P1N_tracks_single3_hh", ix, energy, weight);
		}

		for(int i = 0; i < vtpClustersMatched_neg_top.size(); i++){
			VTPData::hpsCluster vtpCluster = vtpClustersMatched_neg_top.at(i);
			double energy = vtpCluster.E / 1000.;

			int ix = vtpCluster.X;
			int iy = vtpCluster.Y;

			histos->Fill1DHisto("vtpCluster_energy_events_with_1P1N_tracks_h", energy, weight);
			//histos->Fill2DHisto("vtpCluster_energy_x_events_with_1P1N_tracks_hh", ix, energy, weight);

			//if(tsBitSingle2 && !tsBitSingle3)
				//histos->Fill2DHisto("vtpCluster_energy_x_events_with_1P1N_tracks_Sl2TSl3F_hh", ix, energy, weight);

			//if(!tsBitSingle2 && tsBitSingle3)
				//histos->Fill2DHisto("vtpCluster_energy_x_events_with_1P1N_tracks_Sl2FSl3T_hh", ix, energy, weight);

			//if(tsBitSingle2) histos->Fill2DHisto("vtpCluster_energy_x_events_with_1P1N_tracks_single2_hh", ix, energy, weight);
			//if(tsBitSingle3) histos->Fill2DHisto("vtpCluster_energy_x_events_with_1P1N_tracks_single3_hh", ix, energy, weight);
		}
	}


	////// Validate geometry mapping between Ecal and hodo
	// Use events with single2 trigger
    if(tsData_->prescaled.Single_2_Top){
    	for(int i = 0; i < clulsters_pos_top_matched.size(); i++){
    		CalCluster ecalCluster = clulsters_pos_top_matched.at(i);
    		double energyEcalCluster = ecalCluster.getEnergy();
    		double timeEcalCluster = ecalCluster.getTime();

    		CalHit* seed = (CalHit*)ecalCluster.getSeed();
    		int ix = seed->getCrystalIndices()[0];
    		int iy = seed->getCrystalIndices()[1];
    		if(ix < 0 ) ix++;

    	    // With consideration of readout time windows and "edge effects", clusters are extracted with [8, 120] ns for the analysis
    		// FADC250_W_WIDTH 192
    		// FADC250_W_WIDTH 128
    		if( (iy > 0 && timeEcalCluster >= TIMEECALCLUSTERMINTOP && timeEcalCluster <= TIMEECALCLUSTERMAXTOP)
    				|| (iy < 0 && timeEcalCluster >= TIMEECALCLUSTERMINBOT && timeEcalCluster <= TIMEECALCLUSTERMAXBOT) ){

    			// For each cluster, select hodo hits within a time range around the cluster time
    			std::vector<HodoHit*> hodoHitVect;
    			hodoHitVect.clear();
    			for(int j = 0; j < hodoHits_->size(); j++){
    				HodoHit* hodoHit = hodoHits_->at(j);
    				double timeHodoHit = hodoHit->getTime();
    				double timeDiffEcalClusterHodoHit = timeEcalCluster - timeHodoHit;
    				if(iy > 0 && timeDiffEcalClusterHodoHit >= TIMEDIFFECALCLUSTERHODOHITMINTOP && timeDiffEcalClusterHodoHit <= TIMEDIFFECALCLUSTERHODOHITMAXTOP)
    					hodoHitVect.push_back(hodoHit);
    				else if(iy < 0 && timeDiffEcalClusterHodoHit >= TIMEDIFFECALCLUSTERHODOHITMINBOT && timeDiffEcalClusterHodoHit <= TIMEDIFFECALCLUSTERHODOHITMAXBOT)
    					hodoHitVect.push_back(hodoHit);
    			}

    			// Using selected hodo hits, build hodo patterns
    			std::map<int, hodoPattern> hodopatternMap;
    			hodopatternMap = buildHodoPatternMap(hodoHitVect);

    			// Build trigger bit for single3
    			singleTriggerTags single3TriTags = buildSingle3TriggerTags(&ecalCluster, hodopatternMap);

    			// If a simulated single3 trigger can be formed, check if there is a VTP single3 trigger, where time difference between simulated trigger and VTP trigger is within a time range
    			if(single3TriTags.singleTriggerSet == single3Bits){


					bool flagSingle3Top = false;

					std::vector<VTPData::hpsSingleTrig> singleTrigs = vtpData_->singletrigs;

					for(int j = 0; j < singleTrigs.size(); j++){
						VTPData::hpsSingleTrig singleTrig = singleTrigs.at(j);
						unsigned int timeSingleTrig = singleTrig.T;
						unsigned int instSingleTrig = singleTrig.inst;
						bool topnbot = singleTrig.topnbot;

						double timeDiffEcalClusterVTPSingleTrigger = timeEcalCluster - timeSingleTrig * 4;

						if(instSingleTrig == 3 && topnbot && timeDiffEcalClusterVTPSingleTrigger >= TIMEDIFFECALCLUSTERVTPSINGLETRIGGERMIN && timeDiffEcalClusterVTPSingleTrigger <= TIMEDIFFECALCLUSTERVTPSINGLETRIGGERMAX){
							flagSingle3Top = true;
							break;
						}
					}

					if(flagSingle3Top){
						passSingle3MatchedClusterTop++;

						histos->Fill2DHisto("ecalCluster_energy_x_single3_pass_matched_cluster_hh", ix, energyEcalCluster, weight);
					}
					else{
						failSingle3MatchedClusterTop++;

						histos->Fill2DHisto("ecalCluster_energy_x_single3_fail_matched_cluster_hh", ix, energyEcalCluster, weight);
					}
    			}
    		}
    	}
    }

    if(tsData_->prescaled.Single_2_Bot == true){
    	for(int i = 0; i < clulsters_pos_bot_matched.size(); i++){
    		CalCluster ecalCluster = clulsters_pos_bot_matched.at(i);
    		double energyEcalCluster = ecalCluster.getEnergy();
    		double timeEcalCluster = ecalCluster.getTime();

    		CalHit* seed = (CalHit*)ecalCluster.getSeed();
    		int ix = seed->getCrystalIndices()[0];
    		int iy = seed->getCrystalIndices()[1];
    		if(ix < 0 ) ix++;

    	    // With consideration of readout time windows and "edge effects", clusters are extracted with [8, 120] ns for the analysis
    		// FADC250_W_WIDTH 192
    		// FADC250_W_WIDTH 128
    		if( (iy > 0 && timeEcalCluster >= TIMEECALCLUSTERMINTOP && timeEcalCluster <= TIMEECALCLUSTERMAXTOP)
    				|| (iy < 0 && timeEcalCluster >= TIMEECALCLUSTERMINBOT && timeEcalCluster <= TIMEECALCLUSTERMAXBOT) ){

    			// For each cluster, select hodo hits within a time range around the cluster time
    			std::vector<HodoHit*> hodoHitVect;
    			hodoHitVect.clear();
    			for(int j = 0; j < hodoHits_->size(); j++){
    				HodoHit* hodoHit = hodoHits_->at(j);
    				double timeHodoHit = hodoHit->getTime();
    				double timeDiffEcalClusterHodoHit = timeEcalCluster - timeHodoHit;
    				if(iy > 0 && timeDiffEcalClusterHodoHit >= TIMEDIFFECALCLUSTERHODOHITMINTOP && timeDiffEcalClusterHodoHit <= TIMEDIFFECALCLUSTERHODOHITMAXTOP)
    					hodoHitVect.push_back(hodoHit);
    				else if(iy < 0 && timeDiffEcalClusterHodoHit >= TIMEDIFFECALCLUSTERHODOHITMINBOT && timeDiffEcalClusterHodoHit <= TIMEDIFFECALCLUSTERHODOHITMAXBOT)
    					hodoHitVect.push_back(hodoHit);
    			}

    			// Using selected hodo hits, build hodo patterns
    			std::map<int, hodoPattern> hodopatternMap;
    			hodopatternMap = buildHodoPatternMap(hodoHitVect);

    			// Build trigger bit for single3
    			singleTriggerTags single3TriTags = buildSingle3TriggerTags(&ecalCluster, hodopatternMap);

    			// If a simulated single3 trigger can be formed, check if there is a VTP single3 trigger, where time difference between simulated trigger and VTP trigger is within a time range
    			if(single3TriTags.singleTriggerSet == single3Bits){


					bool flagSingle3Bot = false;

					std::vector<VTPData::hpsSingleTrig> singleTrigs = vtpData_->singletrigs;

					for(int j = 0; j < singleTrigs.size(); j++){
						VTPData::hpsSingleTrig singleTrig = singleTrigs.at(j);
						unsigned int timeSingleTrig = singleTrig.T;
						unsigned int instSingleTrig = singleTrig.inst;
						bool topnbot = singleTrig.topnbot;

						double timeDiffEcalClusterVTPSingleTrigger = timeEcalCluster - timeSingleTrig * 4;

						if(instSingleTrig == 3 && !topnbot && timeDiffEcalClusterVTPSingleTrigger >= TIMEDIFFECALCLUSTERVTPSINGLETRIGGERMIN && timeDiffEcalClusterVTPSingleTrigger <= TIMEDIFFECALCLUSTERVTPSINGLETRIGGERMAX){
							flagSingle3Bot = true;
							break;
						}
					}

					if(flagSingle3Bot){
						passSingle3MatchedClusterBot++;

						histos->Fill2DHisto("ecalCluster_energy_x_single3_pass_matched_cluster_hh", ix, energyEcalCluster, weight);
					}
					else{
						failSingle3MatchedClusterBot++;

						histos->Fill2DHisto("ecalCluster_energy_x_single3_fail_matched_cluster_hh", ix, energyEcalCluster, weight);
					}
    			}
    		}
    	}

    }

#endif

    return true;
}

void TriggerValidationAnaProcessor::finalize() {

	std::cout <<"single2: " << std::endl;
	std::cout <<"	top:" << std::endl;
	std::cout << "		pass: " << passSingle2Top << ";  fail: " << failSingle2Top << ";  single2 fail, but single3 pass: " << failSingle2PassSinge3Top<< std::endl;
	std::cout << "		efficiency: " << (double)passSingle2Top/(passSingle2Top + failSingle2Top)*100 << "%" << std::endl;
	std::cout <<"	bot:" << std::endl;
	std::cout << "		pass: " << passSingle2Bot << ";  fail: " << failSingle2Bot << ";  single2 fail, but single3 pass: " << failSingle2PassSinge3Bot<< std::endl;
	std::cout << "		efficiency: " << (double)passSingle2Bot/(passSingle2Bot + failSingle2Bot)*100 << "%" << std::endl << std::endl;

	std::cout <<"single3: " << std::endl;
	std::cout <<"	top:" << std::endl;
	std::cout << "		pass: " << passSingle3Top << ";  fail: " << failSingle3Top << std::endl;
	std::cout << "		efficiency: " << (double)passSingle3Top/(passSingle3Top + failSingle3Top)*100 << "%" << std::endl;
	std::cout <<"	bot:" << std::endl;
	std::cout << "		pass: " << passSingle3Bot << ";  fail: " << failSingle3Bot << std::endl;
	std::cout << "		efficiency: " << (double)passSingle3Bot/(passSingle3Bot + failSingle3Bot)*100 << "%" << std::endl << std::endl;

	std::cout <<"both single2 and single3: " << std::endl;
	std::cout <<"	top:" << std::endl;
	std::cout << "		pass: " << passSinge2Single3Top << std::endl;
	std::cout <<"	bot:" << std::endl;
	std::cout << "		pass: " << passSinge2Single3Bot << std::endl << std::endl;

	//std::cout <<"fee: " << std::endl;
	//std::cout << "	pass: " << passFee << ";  fail: " << failFee << std::endl;
	//std::cout << "	efficiency: " << (double)passFee/(passFee + failFee)*100 << "%" << std::endl << std::endl;


	#ifdef __WITHSVT__
	std::cout <<"single3 for clusters matched with tracks: " << std::endl;
	std::cout <<"	top:" << std::endl;
	std::cout << "		pass: " << passSingle3MatchedClusterTop << ";  fail: " << failSingle3MatchedClusterTop << std::endl;
	std::cout << "		efficiency: " << (double)passSingle3MatchedClusterTop/(passSingle3MatchedClusterTop + failSingle3MatchedClusterTop)*100 << "%" << std::endl;
	std::cout <<"	bot:" << std::endl;
	std::cout << "		pass: " << passSingle3MatchedClusterBot << ";  fail: " << failSingle3MatchedClusterBot << std::endl;
	std::cout << "		efficiency: " << (double)passSingle3MatchedClusterBot/(passSingle3MatchedClusterBot + failSingle3MatchedClusterBot)*100 << "%" << std::endl << std::endl;

	std::cout << "Study TS bits single2 and single3 with tracks: " << std::endl << std::endl;

	std::cout << "	No track requirement: " << std::endl;
	std::cout << "		Number of events with single2 fired: " << tsSl2T << std::endl;
	std::cout << "		Number of events with single3 fired: " << tsSl3T << std::endl;
	std::cout << "		Number of events with both single2 and single3 fired: " << tsSl2TSl3T << std::endl;
	std::cout << "		Number of events with single2 fired, but single3 not fired: " << tsSl2TSl3F << std::endl;
	std::cout << "		Number of events with single3 fired, but single2 not fired: " << tsSl2FSl3T << std::endl;
	//std::cout << "		Number of events with both single2 and single3 not fired: " << tsSl2FSl3F << std::endl;
	std::cout << std::endl;

	std::cout << "	Require at least one positive track: " << std::endl;
	std::cout << "		Number of events with single2 fired: " << tsSl2T_P << std::endl;
	std::cout << "		Number of events with single3 fired: " << tsSl3T_P << std::endl;
	std::cout << "		Number of events with both single2 and single3 fired: " << tsSl2TSl3T_P << std::endl;
	std::cout << "		Number of events with single2 fired, but single3 not fired: " << tsSl2TSl3F_P << std::endl;
	std::cout << "		Number of events with single3 fired, but single2 not fired: " << tsSl2FSl3T_P << std::endl;
	//std::cout << "		Number of events with both single2 and single3 not fired: " << tsSl2FSl3F_P << std::endl;
	std::cout << std::endl;

	std::cout << "	Require at least one positive and at least one negative tracks: " << std::endl;
	std::cout << "		Number of events with single2 fired: " << tsSl2T_PN << std::endl;
	std::cout << "		Number of events with single3 fired: " << tsSl3T_PN << std::endl;
	std::cout << "		Number of events with both single2 and single3 fired: " << tsSl2TSl3T_PN << std::endl;
	std::cout << "		Number of events with single2 fired, but single3 not fired: " << tsSl2TSl3F_PN << std::endl;
	std::cout << "		Number of events with single3 fired, but single2 not fired: " << tsSl2FSl3T_PN << std::endl;
	//std::cout << "		Number of events with both single2 and single3 not fired: " << tsSl2FSl3F_PN << std::endl;
	std::cout << std::endl;

	std::cout << "	Require only one positive and one negative tracks: " << std::endl;
	std::cout << "		Number of events with single2 fired: " << tsSl2T_1P1N << std::endl;
	std::cout << "		Number of events with single3 fired: " << tsSl3T_1P1N << std::endl;
	std::cout << "		Number of events with both single2 and single3 fired: " << tsSl2TSl3T_1P1N << std::endl;
	std::cout << "		Number of events with single2 fired, but single3 not fired: " << tsSl2TSl3F_1P1N << std::endl;
	std::cout << "		Number of events with single3 fired, but single2 not fired: " << tsSl2FSl3T_1P1N << std::endl;
	//std::cout << "		Number of events with both single2 and single3 not fired: " << tsSl2FSl3F_1P1N << std::endl;
	std::cout << std::endl;

	#endif



    histos->saveHistos(outF_, anaName_.c_str());
    delete histos;
    histos = nullptr;

}

std::map<int, TriggerValidationAnaProcessor::hodoPattern> TriggerValidationAnaProcessor::buildHodoPatternMap(std::vector<HodoHit*> hodoHitVect){
    // All hits over FADCHITTHRESHOLD are saved for each hole of each layer
	std::map<int, std::map<std::pair<int, int>, std::vector<double>>> energyListMapForLayerMap;
	for(int i = botLayer2; i <= topLayer2; i++){
		std::map<std::pair<int, int>, std::vector<double>> energyListMap;
		for (int j = 0; j < 8; j++) {
			std::vector<double> vectEnergy;
			vectEnergy.clear();
			energyListMap[xHolePairs[j]] = vectEnergy;
		}
		energyListMapForLayerMap[i] = energyListMap;
	}

	for(int i = 0; i < hodoHitVect.size(); i++){
		HodoHit* hit = hodoHitVect.at(i);
		double energy = hit->getEnergy();

		if (energy > FADCHITTHRESHOLD) {
			int y = hit->getIndices()[1];
			int x = hit->getIndices()[0];
			int layer = hit->getLayer();
			int hole = hit->getHole();


			std::pair<int, int> xHolePair = std::pair<int, int>(x, hole);
			// Energy of hits is scaled except hits at tiles 0 and 4
			int layerIndex;
			if(y == -1 && layer == 1) layerIndex = botLayer2;
			else if(y == -1 && layer == 0) layerIndex = botLayer1;
			else if(y == 1 && layer == 0) layerIndex = topLayer1;
			else if(y == 1 && layer == 1) layerIndex = topLayer2;

			if(x == 0 || x == 4) energyListMapForLayerMap[layerIndex][xHolePair].push_back(energy);
			else energyListMapForLayerMap[layerIndex][xHolePair].push_back(energy * GAINFACTOR);
		}
	}

	//Get maximum of energy in lists for each hole of each layer
	std::map<int, std::map<std::pair<int,int>, double>> maxEnergyMapForLayerMap;
	for(int i = botLayer2; i <= topLayer2; i++){
		std::map<std::pair<int,int>, double> maxEnergyMap;
		for (int j = 0; j < 8; j++) {
			if(energyListMapForLayerMap[i][xHolePairs[j]].size() != 0)
				maxEnergyMap[xHolePairs[j]] = getMaxEnergyAtHodoHole(energyListMapForLayerMap[i][xHolePairs[j]]);
			else
				maxEnergyMap[xHolePairs[j]] = 0;

		}
		maxEnergyMapForLayerMap[i] = maxEnergyMap;
	}

    //Hodoscope patterns for all layers
    //Order of list: TopLayer1, TopLayer2, BotLayer1, BotLayer2
    std::map<int, TriggerValidationAnaProcessor::hodoPattern> hodoPatternMap;

	for(int i = botLayer2; i <= topLayer2; i++){
    	hodoPattern pattern;
        std::map<std::pair<int,int>, double> maxEnergyMap = maxEnergyMapForLayerMap[i];

        if (maxEnergyMap[xHolePairs[0]] > HODOHITTHRESHOLD) {
            pattern.patternSet[HODO_LX_1] = true;
        }
        if (maxEnergyMap[xHolePairs[1]] + maxEnergyMap[xHolePairs[2]] > HODOHITTHRESHOLD) {
            pattern.patternSet[HODO_LX_2] = true;
        }
        if (maxEnergyMap[xHolePairs[3]] + maxEnergyMap[xHolePairs[4]] > HODOHITTHRESHOLD) {
            pattern.patternSet[HODO_LX_3] = true;
        }
        if (maxEnergyMap[xHolePairs[5]] + maxEnergyMap[xHolePairs[6]] > HODOHITTHRESHOLD) {
        	pattern.patternSet[HODO_LX_4] = true;
        }
        if (maxEnergyMap[xHolePairs[7]] > HODOHITTHRESHOLD) {
            pattern.patternSet[HODO_LX_5] = true;
        }
        if (maxEnergyMap[xHolePairs[0]] + maxEnergyMap[xHolePairs[1]] + maxEnergyMap[xHolePairs[2]] > HODOHITTHRESHOLD
                && maxEnergyMap[xHolePairs[0]] != 0 && (maxEnergyMap[xHolePairs[1]] != 0 || maxEnergyMap[xHolePairs[2]] != 0)) {
            pattern.patternSet[HODO_LX_CL_12] = true;
        }
        if (maxEnergyMap[xHolePairs[1]] + maxEnergyMap[xHolePairs[2]] + maxEnergyMap[xHolePairs[3]] + maxEnergyMap[xHolePairs[4]] > HODOHITTHRESHOLD
                && (maxEnergyMap[xHolePairs[1]] != 0 || maxEnergyMap[xHolePairs[2]] != 0)
                && (maxEnergyMap[xHolePairs[3]] != 0 || maxEnergyMap[xHolePairs[4]] != 0)) {
        	pattern.patternSet[HODO_LX_CL_23] = true;
        }
        if (maxEnergyMap[xHolePairs[3]] + maxEnergyMap[xHolePairs[4]] + maxEnergyMap[xHolePairs[5]] + maxEnergyMap[xHolePairs[6]] > HODOHITTHRESHOLD
                && (maxEnergyMap[xHolePairs[3]] != 0 || maxEnergyMap[xHolePairs[4]] != 0)
                && (maxEnergyMap[xHolePairs[5]] != 0 || maxEnergyMap[xHolePairs[6]] != 0)) {
        	pattern.patternSet[HODO_LX_CL_34] = true;
        }
        if (maxEnergyMap[xHolePairs[5]] + maxEnergyMap[xHolePairs[6]] + maxEnergyMap[xHolePairs[7]] > HODOHITTHRESHOLD
                && (maxEnergyMap[xHolePairs[5]] != 0 || maxEnergyMap[xHolePairs[6]] != 0) && maxEnergyMap[xHolePairs[7]] != 0) {
        	pattern.patternSet[HODO_LX_CL_45] = true;
        }

        hodoPatternMap[i] = pattern;
    }

	return hodoPatternMap;
}

double TriggerValidationAnaProcessor::getMaxEnergyAtHodoHole(std::vector<double> vectEnergy){
	double maxEnergy = 0;
	for(int i = 0; i < vectEnergy.size(); i++)
		if(vectEnergy[i] > maxEnergy) maxEnergy = vectEnergy[i];

	return maxEnergy;
}

TriggerValidationAnaProcessor::singleTriggerTags TriggerValidationAnaProcessor::buildSingle2TriggerTags(CalCluster* ecalCluster){
	singleTriggerTags singleTrigTags;
	double energyEcalCluster = ecalCluster->getEnergy() * 1000; // MeV
	int nHitsEcalCluster = ecalCluster->getNHits();
	CalHit* seed = (CalHit*)ecalCluster->getSeed();
	int ix = seed->getCrystalIndices()[0];
	int iy = seed->getCrystalIndices()[1];

	singleTrigTags.singleTriggerSet[0] = energyEcalCluster >= VTP_HPS_SINGLE_EMIN_SINGLE2;

	singleTrigTags.singleTriggerSet[1] = energyEcalCluster <= VTP_HPS_SINGLE_EMAX;

	singleTrigTags.singleTriggerSet[2] = nHitsEcalCluster >= VTP_HPS_SINGLE_NMIN;

	singleTrigTags.singleTriggerSet[3] = ix >= VTP_HPS_SINGLE_XMIN;

	singleTrigTags.singleTriggerSet[4] = energyEcalCluster / ENERGYRATIOECALVTPCLUSTERS > calculatePDE(ix, single2);

	return singleTrigTags;
}

TriggerValidationAnaProcessor::singleTriggerTags TriggerValidationAnaProcessor::buildSingle3TriggerTags(CalCluster* ecalCluster, std::map<int, TriggerValidationAnaProcessor::hodoPattern> hodoPatternMap){
	singleTriggerTags singleTrigTags;
	double energyEcalCluster = ecalCluster->getEnergy() * 1000; // MeV
	int nHitsEcalCluster = ecalCluster->getNHits();
	CalHit* seed = (CalHit*)ecalCluster->getSeed();
	int ix = seed->getCrystalIndices()[0];
	int iy = seed->getCrystalIndices()[1];

	singleTrigTags.singleTriggerSet[0] = energyEcalCluster >= VTP_HPS_SINGLE_EMIN_SINGLE3;

	singleTrigTags.singleTriggerSet[1] = energyEcalCluster <= VTP_HPS_SINGLE_EMAX;

	singleTrigTags.singleTriggerSet[2] = nHitsEcalCluster >= VTP_HPS_SINGLE_NMIN;

	singleTrigTags.singleTriggerSet[3] = ix >= VTP_HPS_SINGLE_XMIN;

	singleTrigTags.singleTriggerSet[4] = energyEcalCluster / ENERGYRATIOECALVTPCLUSTERS > calculatePDE(ix, single3);

	hodoPattern patternLayer1, patternLayer2;
	if(iy > 0) {
		patternLayer1 = hodoPatternMap[topLayer1];
		patternLayer2 = hodoPatternMap[topLayer2];
	}
	else{
		patternLayer1 = hodoPatternMap[botLayer1];
		patternLayer2 = hodoPatternMap[botLayer2];
	}

	singleTrigTags.singleTriggerSet[5] = patternLayer1.patternSet.to_ulong() != 0;

	singleTrigTags.singleTriggerSet[6] = patternLayer2.patternSet.to_ulong() != 0 ;

	singleTrigTags.singleTriggerSet[7] = geometryHodoL1L2Matching(patternLayer1, patternLayer2);

	singleTrigTags.singleTriggerSet[8] = geometryEcalHodoMatching(ix, patternLayer1, patternLayer2);

	return singleTrigTags;
}

double TriggerValidationAnaProcessor::calculatePDE(int ix, TriggerValidationAnaProcessor::singleTriggerTypes type){
	if(type == single2)
		return VTP_HPS_SINGLE_PDE_C0_SINGLE2 + VTP_HPS_SINGLE_PDE_C1_SINGLE2 * ix + VTP_HPS_SINGLE_PDE_C2_SINGLE2 * ix * ix + VTP_HPS_SINGLE_PDE_C3_SINGLE2 * ix * ix * ix;
	else if(type == single3)
		return VTP_HPS_SINGLE_PDE_C0_SINGLE3 + VTP_HPS_SINGLE_PDE_C1_SINGLE3 * ix + VTP_HPS_SINGLE_PDE_C2_SINGLE3 * ix * ix + VTP_HPS_SINGLE_PDE_C3_SINGLE3 * ix * ix * ix;
	else
		return 0;
}

bool TriggerValidationAnaProcessor::geometryHodoL1L2Matching(hodoPattern layer1, hodoPattern layer2){
    // Single tile hits
    if (layer1.patternSet[HODO_LX_1] && layer2.patternSet[HODO_LX_1])
        return true;
    if (layer1.patternSet[HODO_LX_2] && (layer2.patternSet[HODO_LX_1] || layer2.patternSet[HODO_LX_2]))
        return true;
    if (layer1.patternSet[HODO_LX_3] && (layer2.patternSet[HODO_LX_2] || layer2.patternSet[HODO_LX_3]))
        return true;
    if (layer1.patternSet[HODO_LX_4] && (layer2.patternSet[HODO_LX_3] || layer2.patternSet[HODO_LX_4]))
        return true;
    if (layer1.patternSet[HODO_LX_5] && (layer2.patternSet[HODO_LX_4] || layer2.patternSet[HODO_LX_5]))
        return true;

    // Clusters tile hits L1
    if (layer1.patternSet[HODO_LX_CL_12] && layer2.patternSet[HODO_LX_1])
        return true;
    if (layer1.patternSet[HODO_LX_CL_23] && layer2.patternSet[HODO_LX_2])
        return true;
    if (layer1.patternSet[HODO_LX_CL_34] && layer2.patternSet[HODO_LX_3])
        return true;
    if (layer1.patternSet[HODO_LX_CL_45] && layer2.patternSet[HODO_LX_4])
        return true;

    // Clusters tile hits L2
    if (layer2.patternSet[HODO_LX_CL_12] && (layer1.patternSet[HODO_LX_2] || layer1.patternSet[HODO_LX_CL_12]))
        return true;
    if (layer2.patternSet[HODO_LX_CL_23] && layer1.patternSet[HODO_LX_3])
        return true;
    if (layer2.patternSet[HODO_LX_CL_34] && layer1.patternSet[HODO_LX_4])
        return true;
    if (layer2.patternSet[HODO_LX_CL_45] && layer1.patternSet[HODO_LX_5])
        return true;

    return false;

}

bool TriggerValidationAnaProcessor::geometryEcalHodoMatching(int x, hodoPattern layer1, hodoPattern layer2){
    bool flagLayer1 = false;
    bool flagLayer2 = false;

    // Cluster X <-> Layer 1 Matching
    if ((x >= 5) && (x <= 9) && (layer1.patternSet[HODO_LX_1] || layer1.patternSet[HODO_LX_CL_12]))
        flagLayer1 = true;
    if (flagLayer1 == false) {
        if ((x >= 6) && (x <= 12) && (layer1.patternSet[HODO_LX_CL_12] || layer1.patternSet[HODO_LX_2] || layer1.patternSet[HODO_LX_CL_23]))
            flagLayer1 = true;
        if (flagLayer1 == false) {
            if ((x >= 10) && (x <= 17) && (layer1.patternSet[HODO_LX_CL_23] | layer1.patternSet[HODO_LX_3] || layer1.patternSet[HODO_LX_CL_34]))
                flagLayer1 = true;
            if (flagLayer1 == false) {
                if ((x >= 15) && (x <= 21) && (layer1.patternSet[HODO_LX_CL_34] || layer1.patternSet[HODO_LX_4] || layer1.patternSet[HODO_LX_CL_45]))
                    flagLayer1 = true;
                if (flagLayer1 == false) {
                    if ((x >= 18) && (x <= 23) && (layer1.patternSet[HODO_LX_CL_45] || layer1.patternSet[HODO_LX_5]))
                        flagLayer1 = true;
                }
            }
        }
    }

    // Cluster X <-> Layer 2 Matching
    if ((x >= 5) && (x <= 9) && (layer2.patternSet[HODO_LX_1] || layer2.patternSet[HODO_LX_CL_12]))
        flagLayer2 = true;
    if (flagLayer2 == false) {
        if ((x >= 6) && (x <= 14) && (layer2.patternSet[HODO_LX_CL_12] || layer2.patternSet[HODO_LX_2] || layer2.patternSet[HODO_LX_CL_23]))
            flagLayer2 = true;
        if (flagLayer2 == false) {
            if ((x >= 12) && (x <= 18) && (layer2.patternSet[HODO_LX_CL_23] || layer2.patternSet[HODO_LX_3] || layer2.patternSet[HODO_LX_CL_34]))
                flagLayer2 = true;
            if (flagLayer2 == false) {
                if ((x >= 16) && (x <= 22) && (layer2.patternSet[HODO_LX_CL_34] || layer2.patternSet[HODO_LX_4] || layer2.patternSet[HODO_LX_CL_45]))
                    flagLayer2 = true;
                if (flagLayer2 == false) {
                    if ((x >= 20) && (x <= 23) && (layer2.patternSet[HODO_LX_CL_45] || layer2.patternSet[HODO_LX_5])) flagLayer2 = true;
                }
            }
        }
    }

    return flagLayer1 && flagLayer2;
}


TriggerValidationAnaProcessor::feeTriggerTags TriggerValidationAnaProcessor::buildFeeTriggerTags(CalCluster* ecalCluster){
	feeTriggerTags feeTrigTags;
	double energyEcalCluster = ecalCluster->getEnergy() * 1000; // MeV
	int nHitsEcalCluster = ecalCluster->getNHits();
	CalHit* seed = (CalHit*)ecalCluster->getSeed();
	int ix = seed->getCrystalIndices()[0];
	int iy = seed->getCrystalIndices()[1];

	feeTrigTags.feeTriggerSet[0] = energyEcalCluster >= VTP_HPS_FEE_EMIN;

	feeTrigTags.feeTriggerSet[1] = energyEcalCluster <= VTP_HPS_FEE_EMAX;

	feeTrigTags.feeTriggerSet[2] = nHitsEcalCluster >= VTP_HPS_FEE_NMIN;

	return feeTrigTags;
}

DECLARE_PROCESSOR(TriggerValidationAnaProcessor);
