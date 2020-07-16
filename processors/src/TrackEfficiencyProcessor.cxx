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
        timeOffset_ = parameters.getDouble("CalTimeOffset");
        beamE_  = parameters.getDouble("beamE");
        isData  = parameters.getInteger("isData");

        //region definitions
        regionSelections_ = parameters.getVString("regionDefinitions");
        

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
    _trkeff_histos = std::make_shared<TrackEfficHistos>("clusterSelection");
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
       
       _reg_trkeff_histos[regname] = std::make_shared<TrackEfficHistos>(regname);
       _reg_trkeff_histos[regname]->loadHistoConfig(histoCfg_);
       _reg_trkeff_histos[regname]->DefineHistos();
       
       _regions.push_back(regname);
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
    std::vector<std::pair<CalCluster*,Track*> > goodEleSide;
    std::vector<std::pair<CalCluster*,Track*> > goodPosSide;


    //    std::cout<<"Number of clusters = "<<clus_->size()<<std::endl;
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
      _trkeff_histos->FillPreSelectionPlots(cluster,weight);
      
    }
    //cluster pair candidates
    std::vector<TridentCand> triPairs;

    //    std::cout<<"#Good Electrons = "<<goodEleSide.size()<<"; #Good Positrons = "<<goodPosSide.size()<<std::endl;
    for (int i_clu=0; i_clu<goodEleSide.size();i_clu++){
      CalCluster* cluEle=(goodEleSide.at(i_clu)).first;         
      //std::cout<<"Got electron cluster"<<std::endl;
      for (int j_clu=0; j_clu<goodPosSide.size();j_clu++){
	CalCluster* cluPos=(goodPosSide.at(j_clu)).first;
	double cluEleY=cluEle->getPosition().at(1);
	double cluPosY=cluPos->getPosition().at(1);
	double cluEleTime=cluEle->getTime();
	double cluPosTime=cluPos->getTime();
	//	if(cluEle->getPosition().at(0)<-265.0){
	//  std::cout<<"large -X cluster:  clX = "<<cluEle->getPosition().at(0)<<"  time = "<<cluEleTime<<"  diff = "<<abs(cluEleTime-cluPosTime)<<std::endl;
	//}
	if(cluEleY*cluPosY>0) { //just want top/bottom tracks
	  std::cout<<"Both top or bottom"<<std::endl;
	  continue;
	}
	_trkeff_histos->FillPairSelectionPlots(cluEle,cluPos,weight);

	if (!cluSelector->passCutLt("cluTimeDiff_lt",abs(cluEleTime-cluPosTime),weight)){
	  std::cout<<"Cluster time difference too big "<<abs(cluEleTime-cluPosTime)<<std::endl;
	  continue;
	}
	
	std::pair<CalCluster*,Track*> ele=goodEleSide.at(i_clu);
	std::pair<CalCluster*,Track*> pos=goodPosSide.at(j_clu);
	TridentCand epemPair={ele,pos};
	triPairs.push_back(epemPair);
	
      }
    }
    //    std::cout<<"#Good cluster pairs = "<<triPairs.size()<<std::endl;
    for (auto region : _regions ) {
      for (int i_pair=0; i_pair<triPairs.size();i_pair++){
	TridentCand epemPair(triPairs.at(i_pair));
	std::pair<CalCluster*,Track*> electron=epemPair.ele;
	std::pair<CalCluster*,Track*> positron=epemPair.pos;
	bool isFiducialElectron=_ah->IsECalFiducial(electron.first);
	bool isFiducialPositron=_ah->IsECalFiducial(positron.first);

      if (!_reg_trkeff_selectors[region]->passCutEq("cluFiducialElectron",isFiducialElectron,weight))
	continue;
      if (!_reg_trkeff_selectors[region]->passCutEq("cluFiducialPositron",isFiducialPositron,weight))
	continue;
	
      double coplan=_ah->GetClusterCoplanarity(electron.first, positron.first);
      
      if(!_reg_trkeff_selectors[region]->passCutLt("cluCoplan",abs(coplan-180.0),weight))
	continue;	

      _reg_trkeff_histos[region]->FillEffPlots(electron,positron,weight);
      
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
    
  outF_->Close();
        
}

DECLARE_PROCESSOR(TrackEfficiencyProcessor);
