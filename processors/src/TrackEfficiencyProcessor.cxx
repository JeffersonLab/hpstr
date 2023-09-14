/** 
 *@file TrackEfficiencyProcessor.cxx
 *@brief Track Efficiency processor requiring two clusters, one with matched track and probing second 
 *cluster to see if it also has matched track (works best for tag-positrons, because WABs)
 *TODO:   
 *@author MattG, SLAC
 */
#include "TrackEfficiencyProcessor.h"
#include <iostream>
#include <utility> 
TrackEfficiencyProcessor::TrackEfficiencyProcessor(const std::string& name, Process& process) : Processor(name,process) {

}
   
//TODO Check this destructor

TrackEfficiencyProcessor::~TrackEfficiencyProcessor(){}

void TrackEfficiencyProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring TrackEfficiencyProcessor" <<std::endl;
    try 
    {
        debug_   = parameters.getInteger("debug");
        anaName_ = parameters.getString("anaName");
        cluColl_ = parameters.getString("cluColl");
        fspartColl_ = parameters.getString("fspartColl");
        trkColl_ = parameters.getString("trkColl");
	//	trkSelCfg_   = parameters.getString("trkSelectionjson");
        cluSelCfg_   = parameters.getString("cluSelectionjson");
        cluHistoCfg_ = parameters.getString("cluHistoCfg");
        histoCfg_ = parameters.getString("histoCfg");
        thrProngCfg_ = parameters.getString("thrProngCfg");
        timeOffset_ = parameters.getDouble("CalTimeOffset");
        beamE_  = parameters.getDouble("beamE");
        isData  = parameters.getInteger("isData");

        //region definitions
        regionSelections_ = parameters.getVString("regionDefinitions");
        threeProngSelections_ = parameters.getVString("threeProngDefinitions");
        

    }
    catch (std::runtime_error& error) 
    {
        std::cout<<error.what()<<std::endl;
    }
}

void TrackEfficiencyProcessor::initialize(TTree* tree) {
    tree_ = tree;
    _ah =  std::make_shared<AnaHelpers>();
    
    //    trkSelector  = std::make_shared<BaseSelector>("trkSelection",trkSelCfg_);
    //trkSelector->setDebug(debug_);
    // trkSelector->LoadSelection();
    std::cout<<"Setting up cluster selector and trkeff histos"<<std::endl;
    cluSelector  = std::make_shared<BaseSelector>("cluSelection",cluSelCfg_);
    cluSelector->setDebug(debug_);
    cluSelector->LoadSelection();
    std::cout<<"Setting up cluster selection"<<std::endl;
    _trkeff_histos = std::make_shared<TrackEfficHistos>("clusterSelection",_ah);
    _trkeff_histos->loadHistoConfig(cluHistoCfg_);
    _trkeff_histos->DefineHistos();
    
    
     // //For each region initialize plots
    std::cout<<"Number of regions = "<<regionSelections_.size()<<std::endl;
     for (unsigned int i_reg = 0; i_reg < regionSelections_.size(); i_reg++) {
       std::string regname = AnaHelpers::getFileName(regionSelections_[i_reg],false);
       std::cout<<"Setting up region:: " << regname <<std::endl;   
       _reg_trkeff_selectors[regname] = std::make_shared<BaseSelector>(regname, regionSelections_[i_reg]);
       _reg_trkeff_selectors[regname]->setDebug(debug_);
       _reg_trkeff_selectors[regname]->LoadSelection();
       
       _reg_trkeff_histos[regname] = std::make_shared<TrackEfficHistos>(regname,_ah);
       _reg_trkeff_histos[regname]->loadHistoConfig(histoCfg_);
       _reg_trkeff_histos[regname]->DefineHistos();
       
       _regions.push_back(regname);
     }
       // //For each three-prong region initialize plots
    std::cout<<"Number of regions = "<<threeProngSelections_.size()<<std::endl;
     for (unsigned int i_reg = 0; i_reg < threeProngSelections_.size(); i_reg++) {
       std::string regname = AnaHelpers::getFileName(threeProngSelections_[i_reg],false);
       std::cout<<"Setting up region:: " << regname <<std::endl;   
       _reg_three_prong_trkeff_selectors[regname] = std::make_shared<BaseSelector>(regname, threeProngSelections_[i_reg]);
       _reg_three_prong_trkeff_selectors[regname]->setDebug(debug_);
       _reg_three_prong_trkeff_selectors[regname]->LoadSelection();
       std::cout<<"Configuring histos with:: " << thrProngCfg_ <<std::endl;          
       _reg_three_prong_trkeff_histos[regname] = std::make_shared<ThreeProngHistos>(regname);
       _reg_three_prong_trkeff_histos[regname]->loadHistoConfig(thrProngCfg_);
       _reg_three_prong_trkeff_histos[regname]->DefineHistos();
       
       _three_prong_regions.push_back(regname);
     }
    
     
     //init Reading Tree
     tree_->SetBranchAddress(cluColl_.c_str(), &clus_ , &bclus_);
     tree_->SetBranchAddress(fspartColl_.c_str(), &fspart_ , &bfspart_);
     tree_->SetBranchAddress("EventHeader",&evth_ , &bevth_);
    
     //If track collection name is empty take the tracks from the particles. TODO:: change this
     if (!trkColl_.empty())
       tree_->SetBranchAddress(trkColl_.c_str(),&trks_, &btrks_);
}

bool TrackEfficiencyProcessor::process(IEvent* ievent) { 
    
    HpsEvent* hps_evt = (HpsEvent*) ievent;
    double weight = 1.;

    //Store processed number of events
    //std::vector<std::pair<CalCluster*,Track*> > goodEleSide;
    //std::vector<std::pair<CalCluster*,Track*> > goodPosSide;
    std::vector<Particle*> goodEleSide;
    std::vector<Particle*> goodPosSide;
    std::vector<Particle*> goodAll;
    int nClustersAll=clus_->size();
    _trkeff_histos->Fill1DHisto("nClusters_all_h",clus_->size());


    //    std::cout<<"Number of final state particles = "<<fspart_->size()<<std::endl;
    // std::cout<<"Number of clusters = "<<clus_->size()<<std::endl;
    int n_pass_trig_time=0;
    
    for(int i_part=0;i_part<fspart_->size();i_part++){
      Particle* part=fspart_->at(i_part);
      CalCluster cluster=part->getCluster();
      //std::cout<<"Number of hits in cluster = "<<cluster.getNHits()<<std::endl;
      //if(cluster.getNHits()==0)
      //   continue; //this means there was no cluster in this particle

      //std::cout<<"passed the cluster nhits...so there is a cluster with hits..."<<std::endl;
      //these cuts with cut out "clusters" that just have the initialization values...still not sure why we don't use pointers. 
      if (!cluSelector->passCutGt("cluEne_gt",cluster.getEnergy(),weight))
	continue;
      if (!cluSelector->passCutLt("cluTime_lt",cluster.getTime()-timeOffset_,weight))
	continue;
      if (!cluSelector->passCutGt("cluTime_gt",cluster.getTime()-timeOffset_,weight))
	continue;
      //found a good cluster...check if ele or pos side and match to particle if there is one
      //      if(cluster->getPosition().at(0)<-270.0){
      //std::cout<<"Cluster passed!  "<<std::endl;
      //std::cout<<"GOOD cluster:  clX = "<<cluster.getPosition().at(0)<<std::endl;
      //      }
      goodAll.push_back(part); 
      double clX=cluster.getPosition().at(0);
      if(clX>0){
	goodPosSide.push_back(part);
      } else {
	goodEleSide.push_back(part);
      }      
      n_pass_trig_time++;
      _trkeff_histos->FillPreSelectionPlots(&cluster,weight); 
      
    }

    /*
    for (int i_clu=0;i_clu<clus_->size(); i_clu++){
      CalCluster* cluster=clus_->at(i_clu);
      std::pair<CalCluster*, Track*> clTrkPair=_trkeff_histos->getClusterTrackPair(cluster,*trks_);
      //      if(cluster->getPosition().at(0)<-270.0){
      //	std::cout<<"large -X cluster:  clX = "<<cluster->getPosition().at(0)<<std::endl;
      //}
      if (!cluSelector->passCutGt("cluEne_gt",cluster->getEnergy(),weight))
	continue;
      if (!cluSelector->passCutLt("cluTime_lt",cluster->getTime()-timeOffset_,weight))
	continue;
      if (!cluSelector->passCutGt("cluTime_gt",cluster->getTime()-timeOffset_,weight))
	continue;
      //  found a good cluster...check if ele or pos side and match to particle if there is one
      //      if(cluster->getPosition().at(0)<-270.0){
      //std::cout<<"GOOD large -X cluster:  clX = "<<cluster->getPosition().at(0)<<std::endl;
      //      }
      double clX=cluster->getPosition().at(0);
      if(clX>0){
	goodPosSide.push_back(clTrkPair);
      } else {
	goodEleSide.push_back(clTrkPair);
      }      
      n_pass_trig_time++;
      _trkeff_histos->FillPreSelectionPlots(cluster,weight);      
    }
    */
    _trkeff_histos->Fill1DHisto("nClusters_pass_trig_time_h",n_pass_trig_time);

    //cluster pair candidates
    std::vector<TridentCand> triPairs;
    for (int i_part=0; i_part<goodEleSide.size();i_part++){
      Particle* partEle=goodEleSide.at(i_part);
      CalCluster cluEle=partEle->getCluster();//this already for sure exists
      //std::cout<<"Got electron cluster"<<std::endl;
      for (int j_part=0; j_part<goodPosSide.size();j_part++){
        Particle* partPos=goodPosSide.at(j_part);
        CalCluster cluPos=partPos->getCluster();//this already for sure exists
        double cluEleY=cluEle.getPosition().at(1);
	double cluPosY=cluPos.getPosition().at(1);
	double cluEleTime=cluEle.getTime();
	double cluPosTime=cluPos.getTime();
	//	if(cluEle->getPosition().at(0)<-265.0){
	//  std::cout<<"large -X cluster:  clX = "<<cluEle->getPosition().at(0)<<"  time = "<<cluEleTime<<"  diff = "<<abs(cluEleTime-cluPosTime)<<std::endl;
	//}
	if(cluEleY*cluPosY>0) { //just want top/bottom tracks
          //	  std::cout<<"Both top or bottom"<<std::endl;
	  continue;
	}

	if (!cluSelector->passCutLt("cluTimeDiff_lt",abs(cluEleTime-cluPosTime),weight)){
          //	  std::cout<<"Cluster time difference too big "<<abs(cluEleTime-cluPosTime)<<std::endl;
	  continue;
	}
	_trkeff_histos->FillPairSelectionPlots(&cluEle,&cluPos,weight);	
	TridentCand epemPair={partEle,partPos};
	triPairs.push_back(epemPair);

      }
    }
    _trkeff_histos->Fill1DHisto("nClusterPairs_pass_bunch_time_h",triPairs.size());
 

    //make a 3-prong candidate by looping over 2-prong (trident) pairs
    //and adding another particle from "goodAll" list
    //I am allowing the "recoil" to be on either electron or positron side...
    //I can require that recoil is in electron side in region cuts if I want to 
    std::vector<ThreeProngCand> threeProngers;
    //    std::cout<<"Looking for 3-prongers from triPairs n = "<<triPairs.size()<<std::endl;
    for (int i_pair=0; i_pair<triPairs.size();i_pair++){
      TridentCand epemPair(triPairs.at(i_pair));
      Particle* partEle=epemPair.ele;
      Particle* partPos=epemPair.pos;
      CalCluster cluEle=partEle->getCluster();
      CalCluster cluPos=partPos->getCluster();
      double cluEleTime=cluEle.getTime();
      double cluPosTime=cluPos.getTime();
      double cluEleEne=cluEle.getEnergy();
      double cluPosEne=cluPos.getEnergy();
      /*
      std::cout<<"Electron Cluster Time = "<<cluEleTime<<std::endl;
      std::cout<<"Positron Cluster Time = "<<cluPosTime<<std::endl;
      std::cout<<"looping over all good particles = "<<goodAll.size()<<std::endl;
      */
      for (int k_all=0; k_all<goodAll.size(); k_all++){
        Particle* partRec=goodAll.at(k_all);       
        CalCluster cluRec=partRec->getCluster();//this already for sure exists
	double cluRecTime=cluRec.getTime();
	double cluRecEne=cluRec.getEnergy();
        //        std::cout<<"Checking new particle with cluster time = "<<cluRecTime<<std::endl;
        if(partRec == partEle || partRec==partPos){
          //          std::cout<<"in 3-prong finder::Found an overlapping ele or pos...skip it"<<std::endl;
          continue;
        }
        //check if it's in the cluster dt of both ele and pos...if so, it's good for now
        if (!cluSelector->passCutLt("cluTimeDiff_lt",abs(cluEleTime-cluRecTime),weight))
	  continue;	
        if (!cluSelector->passCutLt("cluTimeDiff_lt",abs(cluPosTime-cluRecTime),weight))
	  continue;
        
        //ok...let's make sure we haven't added this one already in a different combination...
        //to do...make a function for this. 
        bool foundExistingPronger=false; 
        for(int j_3p=0;j_3p<threeProngers.size();j_3p++){
          ThreeProngCand check3Pronger=threeProngers.at(j_3p);
          Particle* checkPos=check3Pronger.pos;
          Particle* checkEle=check3Pronger.ele;
          Particle* checkRec=check3Pronger.recoil;
          bool eleMatches=false;
          bool recMatches=false;
          bool posMatches=false;
          if(checkEle==partEle ||checkEle==partRec) 
            eleMatches=true;
          if(checkPos==partPos ||checkPos==partRec) 
            posMatches=true; 
          if(checkRec==partRec ||checkEle==partRec ||checkPos==partRec) 
            recMatches=true;
          if(eleMatches && posMatches && recMatches)
            foundExistingPronger=true; 
          //          std::cout<<"found an exising pronger...."<<std::endl;
        }
        if(!foundExistingPronger){
          
          
          ThreeProngCand this3Pronger={partEle,partPos,partRec};
          if(cluRecEne>cluEleEne)//flip the order so highest energy "electron" is labeled "ele" and lowest is "recoil"
            this3Pronger={partRec,partPos,partEle};
          threeProngers.push_back(this3Pronger);  
        }
      }      
    }
    if(threeProngers.size()>0)
      std::cout<<"Number of 3-prongers found = "<<threeProngers.size()<<std::endl;

    //    std::cout<<"#Good cluster pairs = "<<triPairs.size()<<std::endl;
    for (auto region : _regions ) {
      for (int i_pair=0; i_pair<triPairs.size();i_pair++){
	TridentCand epemPair(triPairs.at(i_pair));
        //	std::pair<CalCluster*,Track*> electron=epemPair.ele;
	//  std::pair<CalCluster*,Track*> positron=epemPair.pos;
        Particle* partEle=epemPair.ele;
        Particle* partPos=epemPair.pos;
        CalCluster electron=partEle->getCluster();
        CalCluster positron=partPos->getCluster();
	bool isFiducialElectron=_ah->IsECalFiducial(&electron);
	bool isFiducialPositron=_ah->IsECalFiducial(&positron);
        double clusterESum=electron.getEnergy()+positron.getEnergy();        

        if (!_reg_trkeff_selectors[region]->passCutLt("nClustersCut",nClustersAll,weight))
          continue;
        if (!_reg_trkeff_selectors[region]->passCutLt("nClustersTrigTimeCut",n_pass_trig_time,weight))
          continue;
        if (!_reg_trkeff_selectors[region]->passCutGt("cluESum",clusterESum,weight))
          continue;
        if (!_reg_trkeff_selectors[region]->passCutLt("cluESumLt",clusterESum,weight))
          continue;
        if (!_reg_trkeff_selectors[region]->passCutEq("cluFiducialElectron",isFiducialElectron,weight))
          continue;
        if (!_reg_trkeff_selectors[region]->passCutEq("cluFiducialPositron",isFiducialPositron,weight))
          continue;
	
      //      double coplan=_ah->GetClusterCoplanarity(electron.first, positron.first);
      
      //if(!_reg_trkeff_selectors[region]->passCutLt("cluCoplan",abs(coplan-180.0),weight))
      //continue;	
        
        _reg_trkeff_histos[region]->FillEffPlots(partEle,partPos,weight);
      
      }
    }
    

    for (auto region : _three_prong_regions ) {
      for (int i_pair=0; i_pair<threeProngers.size();i_pair++){
	ThreeProngCand threeProng(threeProngers.at(i_pair));
        //	std::pair<CalCluster*,Track*> electron=epemPair.ele;
	//  std::pair<CalCluster*,Track*> positron=epemPair.pos;
        std::cout<<"analyzing a three pronger"<<std::endl;
        Particle* partEle=threeProng.ele;
        Particle* partRec=threeProng.recoil;
        Particle* partPos=threeProng.pos;
        CalCluster electron=partEle->getCluster();
        CalCluster positron=partPos->getCluster();
        CalCluster recoil=partRec->getCluster();
	bool isFiducialElectron=_ah->IsECalFiducial(&electron);
	bool isFiducialPositron=_ah->IsECalFiducial(&positron);
	bool isFiducialRecoil=_ah->IsECalFiducial(&recoil);
        double clusterESum=electron.getEnergy()+positron.getEnergy()+recoil.getEnergy();        
        std::cout<<"three pronger eSum = "<<clusterESum<<std::endl;

        if (!_reg_three_prong_trkeff_selectors[region]->passCutLt("nClustersCut",nClustersAll,weight))
          continue;
        if (!_reg_three_prong_trkeff_selectors[region]->passCutLt("nClustersTrigTimeCut",n_pass_trig_time,weight))
          continue;
        if (!_reg_three_prong_trkeff_selectors[region]->passCutGt("cluESum",clusterESum,weight))
          continue;
        if (!_reg_three_prong_trkeff_selectors[region]->passCutLt("cluESumLt",clusterESum,weight))
          continue;
        if (!_reg_three_prong_trkeff_selectors[region]->passCutEq("cluFiducialElectron",isFiducialElectron,weight))
          continue;
        if (!_reg_three_prong_trkeff_selectors[region]->passCutEq("cluFiducialPositron",isFiducialPositron,weight))
          continue;
	
      //      double coplan=_ah->GetClusterCoplanarity(electron.first, positron.first);
      
      //if(!_reg_three_prong_trkeff_selectors[region]->passCutLt("cluCoplan",abs(coplan-180.0),weight))
      //continue;	
        std::cout<<"Filling histos for three pronger"<<std::endl;

        _reg_three_prong_trkeff_histos[region]->FillThreeProngPlots(partEle,partPos,partRec,weight);
      
      }
    }

    return true;
}

void TrackEfficiencyProcessor::finalize() {
    
    //TODO clean this up a little.
  outF_->cd();
  _trkeff_histos->saveHistos(outF_,_trkeff_histos->getName());
  outF_->cd(_trkeff_histos->getName().c_str());
  //  trkSelector->getCutFlowHisto()->Write();
        
    
  for (reg_it it = _reg_trkeff_histos.begin(); it!=_reg_trkeff_histos.end(); ++it) {
    (it->second)->saveHistos(outF_,it->first);
    outF_->cd((it->first).c_str());
    _reg_trkeff_selectors[it->first]->getCutFlowHisto()->Write();
        //Save tuples
  //    _reg_tuples[it->first]->writeTree();
  }
  for (three_prong_reg_it it = _reg_three_prong_trkeff_histos.begin(); it!=_reg_three_prong_trkeff_histos.end(); ++it) {
    (it->second)->saveHistos(outF_,it->first);
    outF_->cd((it->first).c_str());
    _reg_three_prong_trkeff_selectors[it->first]->getCutFlowHisto()->Write();
        //Save tuples
  //    _reg_tuples[it->first]->writeTree();
  }
     
  outF_->Close();
        
}

DECLARE_PROCESSOR(TrackEfficiencyProcessor);
