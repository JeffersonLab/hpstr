/**
 * @file MCAnaProcessor.cxx
 * @brief AnaProcessor used fill histograms to compare simulations
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 */     
#include "MCAnaProcessor.h"
#include <iostream>
#include <utility> 
MCAnaProcessor::MCAnaProcessor(const std::string& name, Process& process) : Processor(name,process){}
//TODO CHECK THIS DESTRUCTOR
MCAnaProcessor::~MCAnaProcessor(){}


void MCAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring MCAnaProcessor" << std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        anaName_         = parameters.getString("anaName");
        partColl_        = parameters.getString("partColl");
        trkrHitColl_     = parameters.getString("trkrHitColl");
        ecalHitColl_     = parameters.getString("ecalHitColl");
        histCfgFilename_ = parameters.getString("histCfg");
	//region definitions
        regionSelections_ = parameters.getVString("regionDefinitions");        
	
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }
}

void MCAnaProcessor::initialize(TTree* tree) {
  _ah =  std::make_shared<AnaHelpers>();
  tree_= tree;
  // init histos
  histos = new MCAnaHistos(anaName_);
  histos->loadHistoConfig(histCfgFilename_);
  histos->DefineHistos();
    //histos->Define2DHistos();
  
    // init TTree
  tree_->SetBranchAddress(partColl_.c_str(), &mcParts_, &bmcParts_);
  tree_->SetBranchAddress(trkrHitColl_.c_str(), &mcTrkrHits_, &bmcTrkrHits_);
  tree_->SetBranchAddress(ecalHitColl_.c_str(), &mcEcalHits_, &bmcEcalHits_);
  
  for (unsigned int i_reg = 0; i_reg < regionSelections_.size(); i_reg++) {
    std::string regname = AnaHelpers::getFileName(regionSelections_[i_reg],false);
    std::cout<<"Setting up region:: " << regname <<std::endl;   
    _reg_mc_selectors[regname] = std::make_shared<BaseSelector>(regname, regionSelections_[i_reg]);
    _reg_mc_selectors[regname]->setDebug(debug_);
    _reg_mc_selectors[regname]->LoadSelection();
    
    _reg_mc_histos[regname] = std::make_shared<MCAnaHistos>(regname);
    _reg_mc_histos[regname]->loadHistoConfig(histCfgFilename_);
    _reg_mc_histos[regname]->DefineHistos();
    
    _regions.push_back(regname);
  }
  


}

bool MCAnaProcessor::process(IEvent* ievent) {
  double weight=1.0;
  histos->FillMCParticles(mcParts_);
  histos->FillMCTrackerHits(mcTrkrHits_);
  histos->FillMCEcalHits(mcEcalHits_);
  
  
  MCParticle* eleRad=NULL;
  MCParticle* eleRec=NULL;
  MCParticle* pos=NULL;
  int eleRadMCMom=-666;
  int eleRecMCMom=-666;
  int posMCMom=-666;
  // get the generated MCParticles...if this is tri-trig, eleRad is just the 
  // first electron (recoiling off W) in list
  for (int i=0; i<mcParts_->size(); i++){
    MCParticle* mcPart=mcParts_->at(i); 
    int pdgID=mcPart->getPDG();
    int momPdgID=mcPart->getMomPDG();
    if(pdgID==11){//electron
      if(momPdgID==623){
	eleRad=mcPart;
	eleRadMCMom=momPdgID;
      }
      if(momPdgID==622){
	if(eleRec==NULL){
	  eleRec=mcPart;
	  eleRecMCMom=momPdgID;
	}
	else {
	  eleRad=mcPart;	  
	  eleRadMCMom=momPdgID;
	}
      }	
    }
    if(pdgID==-11 && (momPdgID==622 || momPdgID==623)) {
      pos=mcPart; 
      posMCMom=momPdgID;
    }
  }
  
  if(!eleRad || !eleRec || !pos){
    std::cout<<"Didn't find one of electron/positron!!!"<<std::endl;
    for (int i=0; i<mcParts_->size(); i++){
      MCParticle* mcPart=mcParts_->at(i); 
      int pdgID=mcPart->getPDG();
      int momPdgID=mcPart->getMomPDG();
      std::cout<<"pdgID = "<<pdgID<<"; momPdgID = "<<momPdgID<<std::endl;
    }
    return true;
  }

  //if this is from trident event (no pdg=623) 
  //sort so that "eleRad" is highest energy
  if(eleRadMCMom!=623 && eleRec->getEnergy()>eleRad->getEnergy()){
    MCParticle* tmp=eleRad;
    eleRad=eleRec;
    eleRec=tmp;
  }


  std::vector<int> eleRadLayers=_ah->getMCParticleLayersHit(eleRad,*mcTrkrHits_);
  std::vector<int> eleRecLayers=_ah->getMCParticleLayersHit(eleRec,*mcTrkrHits_);
  std::vector<int> posLayers=_ah->getMCParticleLayersHit(pos,*mcTrkrHits_);
  

  for(auto region:_regions){
    _reg_mc_selectors[region]->getCutFlowHisto()->Fill(0.,weight);
    //require (any) ele and pos to have min module hits (3d)
    //    std::cout<<"eleRadPassModHits?"<<std::endl;
    bool eleRadPassModHits=_reg_mc_selectors[region]->passCutGt("minModHits_gt",countModuleHits(eleRadLayers),weight);
    //    std::cout<<eleRadPassModHits<<"     eleRecPassModHits?"<<std::endl;
    bool eleRecPassModHits=_reg_mc_selectors[region]->passCutGt("minModHits_gt",countModuleHits(eleRecLayers),weight);
    //std::cout<<eleRecPassModHits<<"   posPassModHits?"<<std::endl;
    bool posPassModHits=_reg_mc_selectors[region]->passCutGt("minModHits_gt",countModuleHits(posLayers),weight);
    //std::cout<<posPassModHits<<std::endl;

    if(!((eleRadPassModHits || eleRecPassModHits) && posPassModHits)) //require at least one electron + positron
      continue;

    std::cout<<"Passed cuts...filling plots"<<std::endl;
    _reg_mc_histos[region]->FillMCParticleHistos(eleRad,"eleRad",weight);
    _reg_mc_histos[region]->FillMCParticleHistos(eleRad,"ele",weight);
    _reg_mc_histos[region]->FillMCParticleHistos(eleRec,"eleRec",weight);
    _reg_mc_histos[region]->FillMCParticleHistos(eleRec,"ele",weight);
    _reg_mc_histos[region]->FillMCParticleHistos(pos,"pos",weight);

    if(posPassModHits && eleRecPassModHits)
      _reg_mc_histos[region]->FillMCPairHistos(eleRec,pos,"pair",weight);
    if(posPassModHits && eleRadPassModHits)
      _reg_mc_histos[region]->FillMCPairHistos(eleRad,pos,"pair",weight);

  }

  return true;
}

void MCAnaProcessor::finalize() {
  outF_->cd();
  histos->saveHistos(outF_, anaName_);
  delete histos;
  histos = nullptr;
  
  for (reg_it it = _reg_mc_histos.begin(); it!=_reg_mc_histos.end(); ++it) {
    (it->second)->saveHistos(outF_,it->first);
    outF_->cd((it->first).c_str());
    _reg_mc_selectors[it->first]->getCutFlowHisto()->Write();
  }
  outF_->Close();  
}

int MCAnaProcessor::countModuleHits(std::vector<int> hitmap){
  int cnt=0;
  for(int i=0;i<8;i++)
    if(hasHitsInModule(hitmap,i))
      cnt++;
  return cnt;
}

bool MCAnaProcessor::hasHitsInModule(std::vector<int> hitmap,int mod){
  //modules go 1-7 corresponding to hits in layers (mod*2-1, mod*2)
  bool hasFirstLayer=std::find(hitmap.begin(), hitmap.end(), 2*mod-1) != hitmap.end();
  bool hasSecondLayer=std::find(hitmap.begin(), hitmap.end(), 2*mod) != hitmap.end();
  if(hasFirstLayer && hasSecondLayer)
    return true;
  return false; 
}

DECLARE_PROCESSOR(MCAnaProcessor);
