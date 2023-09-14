/** 
 *@file TridentWABAnaProcessor.cxx
 *@brief Main trident and WAB analysis processor
 *@author MattG, SLAC
 * copied from TridentAnaProcessorRedoParticles
 * in order to modernize for 2019/2021
 * based off of PF's VertexAnaProcessor
 */

#include "TridentWABAnaProcessor.h"
#include <iostream>
#include <utility> 
TridentWABAnaProcessor::TridentWABAnaProcessor(const std::string& name, Process& process) : Processor(name,process) {

}

//TODO Check this destructor

TridentWABAnaProcessor::~TridentWABAnaProcessor(){}

void TridentWABAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring TridentWABAnaProcessor" <<std::endl;
    try 
    {
        debug_   = parameters.getInteger("debug");
        anaName_ = parameters.getString("anaName");
	cluColl_ = parameters.getString("cluColl");
	hitColl_ = parameters.getString("hitColl",hitColl_);
	rawhitColl_ = parameters.getString("rawhitColl",rawhitColl_);
        vtxColl_ = parameters.getString("vtxColl");
        trkColl_ = parameters.getString("trkColl");
	mcColl_  = parameters.getString("mcColl",mcColl_);
	fspartColl_ = parameters.getString("fspartColl",mcColl_);
	trkSelCfg_   = parameters.getString("trkSelectionjson");
        selectionCfg_   = parameters.getString("vtxSelectionjson");
        histoCfg_ = parameters.getString("histoCfg");
        calTimeOffset_ = parameters.getDouble("CalTimeOffset");
        trkTimeOffset_ = parameters.getDouble("TrkTimeOffset");
        beamE_  = parameters.getDouble("beamE");
        isData  = parameters.getInteger("isData");

        //region definitions
        regionSelections_ = parameters.getVString("regionDefinitions");        
        regionWABSelections_ = parameters.getVString("regionWABDefinitions");
        

    }
    catch (std::runtime_error& error) 
    {
        std::cout<<error.what()<<std::endl;
    }
}

void TridentWABAnaProcessor::initialize(TTree* tree) {
    tree_ = tree;
    _ah =  std::make_shared<AnaHelpers>();
    
    trkSelector  = std::make_shared<BaseSelector>("trkSelection",trkSelCfg_);
    trkSelector->setDebug(debug_);
    trkSelector->LoadSelection();
        
    vtxSelector  = std::make_shared<BaseSelector>("vtxSelection",selectionCfg_);
    vtxSelector->setDebug(debug_);
    vtxSelector->LoadSelection();
    
    _vtx_histos = std::make_shared<TridentHistos>("vtxSelection");
    _vtx_histos->loadHistoConfig(histoCfg_);
    _vtx_histos->DefineHistos();
    
    
     // //For each region initialize plots
     
     for (unsigned int i_reg = 0; i_reg < regionSelections_.size(); i_reg++) {
       std::string regname = AnaHelpers::getFileName(regionSelections_[i_reg],false);
       std::cout<<"Setting up region:: " << regname <<std::endl;   
       _reg_vtx_selectors[regname] = std::make_shared<BaseSelector>(regname, regionSelections_[i_reg]);
       _reg_vtx_selectors[regname]->setDebug(debug_);
       _reg_vtx_selectors[regname]->LoadSelection();
       
       _reg_vtx_histos[regname] = std::make_shared<TridentHistos>(regname);
       _reg_vtx_histos[regname]->loadHistoConfig(histoCfg_);
       _reg_vtx_histos[regname]->DefineHistos();
       
       _reg_tuples[regname] = std::make_shared<FlatTupleMaker>(regname+"_tree");
       _reg_tuples[regname]->addVariable("unc_vtx_mass");
       _reg_tuples[regname]->addVariable("unc_vtx_z");
       
       _regions.push_back(regname);
     }
     
      
     for (unsigned int i_reg = 0; i_reg < regionWABSelections_.size(); i_reg++) {
       std::string regname = AnaHelpers::getFileName(regionWABSelections_[i_reg],false);
       std::cout<<"Setting up region:: " << regname <<std::endl;   
       _reg_WAB_selectors[regname] = std::make_shared<BaseSelector>(regname, regionWABSelections_[i_reg]);
       _reg_WAB_selectors[regname]->setDebug(debug_);
       _reg_WAB_selectors[regname]->LoadSelection();
       
       _reg_WAB_histos[regname] = std::make_shared<TridentHistos>(regname);
       _reg_WAB_histos[regname]->loadHistoConfig(histoCfg_);
       _reg_WAB_histos[regname]->DefineHistos();
       
       _regionsWAB.push_back(regname);
     }

     //init Reading Tree
     tree_->SetBranchAddress(vtxColl_.c_str(), &vtxs_ , &bvtxs_);
     tree_->SetBranchAddress(fspartColl_.c_str(), &fspart_ , &bfspart_);
     tree_->SetBranchAddress("EventHeader",&evth_ , &bevth_);
     tree_->SetBranchAddress(cluColl_.c_str(), &clus_ , &bclus_);
     tree_->SetBranchAddress(hitColl_.c_str(), &hits_   , &bhits_);
     tree_->SetBranchAddress(rawhitColl_.c_str(), &rawhits_   , &brawhits_);
     tree_->SetBranchAddress("TSBank", &tsdata_   , &btsdata_);
     //If track collection name is empty take the tracks from the particles. TODO:: change this
     if (!trkColl_.empty())
       tree_->SetBranchAddress(trkColl_.c_str(),&trks_, &btrks_);
     if(!isData && !mcColl_.empty()) tree_->SetBranchAddress(mcColl_.c_str() , &mcParts_, &bmcParts_);
}

bool TridentWABAnaProcessor::process(IEvent* ievent) { 
    
    HpsEvent* hps_evt = (HpsEvent*) ievent;
    double weight = 1.;
    //Get "true" mass
    double apMass = -0.9;
    double apZ = -0.9;

    if (mcParts_) {
      std::cout<<"Number of MCParticles for this events = "<<mcParts_->size()<<std::endl;
      for(int i = 0; i < mcParts_->size(); i++){
            if(mcParts_->at(i)->getPDG() == 622) {
                apMass = mcParts_->at(i)->getMass();
                apZ = mcParts_->at(i)->getVertexPosition().at(2);
            }
        }
    }
  

    int nElectrons=0;
    int nPositrons=0;
    if(trks_->size()>0){
      for(int i_trk; i_trk<trks_->size();i_trk++){
        Track* trk=trks_->at(i_trk);
        int ch=trk->getCharge();
        if(ch==-1){
          nElectrons++;
        }else{
          nPositrons++;
        }	      
      }
    }
    
    int nEleSideClusters=0;
    int nPosSideClusters=0;

    for(int i_clu=0; i_clu<clus_->size(); i_clu++){
      CalCluster* clu=clus_->at(i_clu);
      double clX=clu->getPosition().at(0);
      if(clX>100.)
        nPosSideClusters++; 
      if(clX<0)
        nEleSideClusters++;
    }

    /*
    _vtx_histos->Fill1DHisto("n_electrons_h",goodElectrons.size(),weight);
    _vtx_histos->Fill1DHisto("n_positrons_h",goodPositrons.size(),weight);
    
    std::vector<CalCluster*> goodPhotons=getUnmatchedClusters(*clus_,goodElectrons, goodPositrons);

    if(debug_)
      std::cout<<"Electrons = "<<goodElectrons.size()<<"; Positrons = "<<goodPositrons.size()<<"; Photons = "<<goodPhotons.size()<<std::endl;
    */
    std::vector<Vertex*> selected_tri;
    /*
    if(goodElectrons.size()==0 || goodPositrons.size()==0){
      std::cout<<"No good ele or pos...skipping event"<<std::endl;
      ///this skips the WAB part, but whatever for now...I shouldn't have to do this...
      return(0);
    }
    
    for (auto elecluPair : goodElectrons){
      for (auto poscluPair : goodPositrons){
	std::cout<<"trying to match ele+pos to vertex"<<std::endl;
	Vertex* matchedVtx=matchPairToVertex(elecluPair.second,poscluPair.second,*vtxs_);
	if(matchedVtx==NULL) { //if we didn't find a match, continue
          std::cout<<"No vertex match"<<std::endl;
	  continue;
	}
        std::cout<<"found a match!"<<std::endl;
	TridentCand cand={matchedVtx,elecluPair.first,poscluPair.first};
	matched_tri.push_back(cand);
      }
    }
    */
    /*

    for (int ipart=0;ipart<fspart_->size(); ipart++){
      Particle* part=fspart_->at(ipart);
      Track trk=part->getTrack(); 
      if(&trk==NULL)
        continue;
      std::cout<<"From FSP List::Number of hits on track = "<<trk.getTrackerHitCount()<<std::endl;
      for (int ihit=0; ihit<trk.getTrackerHitCount();++ihit) {
        std::cout<<"From FSP List::svt hit list size = "<<trk.getSvtHits().GetEntries()<<std::endl; 
        TrackerHit* hit = (TrackerHit*) trk.getSvtHits().At(ihit);
        std::cout<<"From FSP List::got Tracker hit "<<hit<<std::endl; 
        std::cout<<"From FSP List::layer hit = "<<hit->getLayer()<<std::endl;
      }
    }
    */

    //    std::cout<<"Found "<<vtxs_->size()<<" Unconstrained Vertices"<<std::endl;
    //    std::cout<<"Found "<<matched_tri.size()<<" Matched Vertices"<<std::endl;
    for ( int i_vtx = 0; i_vtx < vtxs_->size(); i_vtx++ ) {
        
        vtxSelector->getCutFlowHisto()->Fill(0.,weight);
        //        TridentCand cand=matched_tri.at(i_vtx);
        Vertex* vtx = vtxs_->at(i_vtx);
        Particle* ele = nullptr;       
        Particle* pos = nullptr;
        //	CalCluster* ele_clu=nullptr; 
	//CalCluster* pos_clu=nullptr;                
        //        bool foundParts = _ah->GetParticlesFromVtx(vtx,ele,pos);
        bool foundParts = _ah->GetParticlesFromVtxAndParticleList(*fspart_,vtx,ele,pos);

	if(debug_)	std::cout<<"Checking Particles"<<std::endl;
        if (!foundParts) {
	  std::cout<<"TridentWABAnaProcessor::WARNING::Found vtx without ele/pos. Skip."<<std::endl;
	  continue;
        }
        
        CalCluster ele_clu=ele->getCluster();
        CalCluster pos_clu=pos->getCluster();
   	//        Trigger requirement - *really hate* having to do it here for each vertex.
	if(debug_)  std::cout<<"Checking Trigger"<<std::endl;
	if (isData) {
	  if (!vtxSelector->passCutEq("Singles2or3_eq",((int)tsdata_->prescaled.Single_2_Top)||((int)tsdata_->prescaled.Single_2_Bot),weight)){
            if(debug_)std::cout<<"events didn't pass singles2, check single3"<<std::endl;          
            if (!vtxSelector->passCutEq("Singles2or3_eq",((int)tsdata_->prescaled.Single_3_Top)||((int)tsdata_->prescaled.Single_3_Bot),weight)){
              if(debug_)std::cout<<"Skipping because event didn't pass singles2 or 3"<<std::endl;
              continue;
            }
          }
	}
       
       
        //Add the momenta to the tracks
        //        ele_trk->setMomentum(ele->getMomentum()[0],ele->getMomentum()[1],ele->getMomentum()[2]);
        //  pos_trk->setMomentum(pos->getMomentum()[0],pos->getMomentum()[1],pos->getMomentum()[2]);
        
        Track ele_trk = ele->getTrack();  //wish these were still pointers to the track...why did this change? 
        Track pos_trk = pos->getTrack();
        int n_hits_ele=ele_trk.getTrackerHitCount();
        int n_hits_pos=pos_trk.getTrackerHitCount();

        //bool foundL1ele = false;
        //	bool foundL2ele = false;
        //        _ah->InnermostLayerCheck(ele_trk_from_trkList, foundL1ele, foundL2ele);   
        //_ah->InnermostLayerCheck(&ele_trk, foundL1ele, foundL2ele);   
        //std::cout<<"main vertex loop:  found L1? "<<foundL1ele<<std::endl;


        TVector3 ele_mom;
        ele_mom.SetX(ele->getMomentum()[0]);
        ele_mom.SetY(ele->getMomentum()[1]);
        ele_mom.SetZ(ele->getMomentum()[2]);

        TVector3 pos_mom;
        pos_mom.SetX(pos->getMomentum()[0]);
        pos_mom.SetY(pos->getMomentum()[1]);
        pos_mom.SetZ(pos->getMomentum()[2]);
        
	if(debug_)std::cout<<" Going through cuts "<<std::endl;
        if (!vtxSelector->passCutGt("eleNHits_gt",n_hits_ele,weight))
          continue;
        if (!vtxSelector->passCutGt("posNHits_gt",n_hits_pos,weight))
            continue;

        if (!vtxSelector->passCutLt("eleMom_lt",ele_mom.Mag(),weight))
            continue;
	//	std::cout<<" Pass beam electron "<<std::endl;

        //Ele Track Quality
        if (!vtxSelector->passCutLt("eleTrkChi2_lt",ele_trk.getChi2Ndf(),weight))
            continue;
        
        //Pos Track Quality
        if (!vtxSelector->passCutLt("posTrkChi2_lt",pos_trk.getChi2Ndf(),weight))
          continue;
	//std::cout<<"track qual "<<std::endl;

        //Vertex Quality
        if (!vtxSelector->passCutLt("chi2unc_lt",vtx->getChi2(),weight))
            continue;
        

        //Ele min momentum cut
        if (!vtxSelector->passCutGt("eleMom_gt",ele_mom.Mag(),weight))
            continue;

        //Pos min momentum cut
        if (!vtxSelector->passCutGt("posMom_gt",pos_mom.Mag(),weight))
            continue;

        //Max vtx momentum
        
        if (!vtxSelector->passCutLt("maxVtxMom_lt",(ele_mom+pos_mom).Mag(),weight))
            continue;
		
        if (!vtxSelector->passCutGt("minVtxMom_gt",(ele_mom+pos_mom).Mag(),weight))
            continue;


	double corr_eleClusterTime=666;
	double corr_posClusterTime=666;
	
        corr_eleClusterTime = ele_clu.getTime() - calTimeOffset_;
        corr_posClusterTime = pos_clu.getTime() - calTimeOffset_;

        double corr_eleTrackTime=-666;
        double corr_posTrackTime=-666;
        
        corr_eleTrackTime=ele_trk.getTrackTime() - trkTimeOffset_;
        corr_posTrackTime=pos_trk.getTrackTime() - trkTimeOffset_;

        bool hasEleCluster=corr_eleClusterTime>-300; //this is kludgy but the default time is set at -9999 ns...anything < ~ -100 ns means it was not filled...
        bool hasPosCluster=corr_posClusterTime>-300;       
	
	//	std::cout<<"Checking ClusterTimeDiff"<<std::endl;
        //Ele Track Time        
        if (!vtxSelector->passCutLt("eleTrkTime_lt",fabs(corr_eleTrackTime),weight))
          continue;
        //Pos Track Time        
        if (!vtxSelector->passCutLt("posTrkTime_lt",fabs(corr_posTrackTime),weight))
          continue;

	
	//Ele Track-Cluster Time Difference
	if(hasEleCluster)
	  if (!vtxSelector->passCutLt("eleTrkCluTimeDiff_lt",fabs(corr_eleTrackTime - corr_eleClusterTime),weight))
	    continue;
	
	//Pos Track-Cluster Time Difference
	if(hasPosCluster)
	  if (!vtxSelector->passCutLt("posTrkCluTimeDiff_lt",fabs(corr_posTrackTime - corr_posClusterTime),weight))
	    continue;


        //Ele Pos Cluster Tme Difference
	if(hasEleCluster && hasPosCluster)
	  if (!vtxSelector->passCutLt("eleposCluTimeDiff_lt",fabs(corr_eleClusterTime - corr_posClusterTime),weight))
	    continue;

        _vtx_histos->Fill1DVertex(vtx,
                                  ele,
                                  pos,
                                  &ele_trk,
                                  &pos_trk,
                                  trkTimeOffset_,
                                  weight);
        
        _vtx_histos->Fill2DHistograms(vtx,weight);
        _vtx_histos->Fill2DTrack(&ele_trk,weight,"ele_");
        _vtx_histos->Fill2DTrack(&pos_trk,weight,"pos_");
        
        selected_tri.push_back(vtx);       
        vtxSelector->clearSelector();
    }
    
    //    std::cout<<"Found "<<selected_tri.size()<<" Selected Vertices"<<std::endl;
    //    _vtx_histos->Fill1DHisto("n_vertices_nodups_h",matched_tri.size(),weight);
    _vtx_histos->Fill1DHisto("n_vertices_h",selected_tri.size()); 
    if (trks_){
      _vtx_histos->Fill1DHisto("n_tracks_h",trks_->size(),weight); 
      _vtx_histos->Fill1DHisto("n_positrons_h",nPositrons,weight);
      _vtx_histos->Fill1DHisto("n_electrons_h",nElectrons,weight);
      //      _vtx_histos->Fill1DHisto("n_tracks_nodups_h",noDups.size(),weight);
    }
      
    _vtx_histos->Fill1DHisto("n_pos_clusters_h",nPosSideClusters,weight);
    _vtx_histos->Fill1DHisto("n_ele_clusters_h",nEleSideClusters,weight);
    
    //Make Plots for each region: loop on each region. Check if the region has the cut and apply it

    for (auto region : _regions ) {
      int nvertPass=0;
      std::vector<Vertex*> final_tri;
      for ( auto cand : selected_tri) {
	//No cuts.
	_reg_vtx_selectors[region]->getCutFlowHisto()->Fill(0.,weight);
	// get objects we need for this candidate
	Vertex* vtx = cand;
	Particle* ele = nullptr;
	Particle* pos = nullptr;        
        //        bool foundParts = _ah->GetParticlesFromVtx(vtx,ele,pos);

        bool foundParts = _ah->GetParticlesFromVtxAndParticleList(*fspart_,vtx,ele,pos);
	if(debug_)	std::cout<<"Checking Particles"<<std::endl;
        if (!foundParts) {
	  std::cout<<"TridentWABAnaProcessor::WARNING::Found vtx without ele/pos. Skip."<<std::endl;
	  continue;
        }        
	Track ele_trk =  ele->getTrack();
	Track pos_trk = pos->getTrack();
        /*
        Track trk=part->getTrack(); 
        if(&trk==NULL)
          continue;
        std::cout<<"From FSP List::Number of hits on track = "<<trk.getTrackerHitCount()<<std::endl;
        for (int ihit=0; ihit<trk.getTrackerHitCount();++ihit) {
          std::cout<<"From FSP List::svt hit list size = "<<trk.getSvtHits().GetEntries()<<std::endl; 
          TrackerHit* hit = (TrackerHit*) trk.getSvtHits().At(ihit);
          std::cout<<"From FSP List::got Tracker hit "<<hit<<std::endl; 
          std::cout<<"From FSP List::layer hit = "<<hit->getLayer()<<std::endl;
      }
        */

        //        Track* ele_trk_from_trkList = _ah->GetTrackFromParticle(*trks_,ele);
        //Track* pos_trk_from_trkList = _ah->GetTrackFromParticle(*trks_,pos);

        CalCluster ele_clu=ele->getCluster(); 
        CalCluster pos_clu=pos->getCluster();
	//Compute analysis variables here.        
	
	double ele_E = ele->getEnergy();
	double pos_E = pos->getEnergy();
	
	TVector3 ele_mom;
	ele_mom.SetX(ele->getMomentum()[0]);
	ele_mom.SetY(ele->getMomentum()[1]);
	ele_mom.SetZ(ele->getMomentum()[2]);
	
	TVector3 pos_mom;
	pos_mom.SetX(pos->getMomentum()[0]);
	pos_mom.SetY(pos->getMomentum()[1]);
	pos_mom.SetZ(pos->getMomentum()[2]);
	
	if (!_reg_vtx_selectors[region]->passCutGt("radMom_gt",(ele_mom+pos_mom).Mag()/beamE_,weight))
	  continue;

        if (!_reg_vtx_selectors[region]->passCutLt("nPosTracks_lt",nPositrons,weight))
          continue ;
        if (!_reg_vtx_selectors[region]->passCutLt("nEleTracks_lt",nElectrons,weight))
          continue; 

        if (!_reg_vtx_selectors[region]->passCutEq("nPosSideClusters_eq",nPosSideClusters,weight))
          continue; 
        if (!_reg_vtx_selectors[region]->passCutEq("nEleSideClusters_eq",nEleSideClusters,weight))
          continue; 
	
	if(!isData){
	  std::pair<Track*,MCParticle*> eleMCMatch=matchToMCParticle(&ele_trk,*mcParts_);
	  std::pair<Track*,MCParticle*> posMCMatch=matchToMCParticle(&pos_trk,*mcParts_);
	  /*	
		if(eleMCMatch.second!=NULL)
		std::cout<<"found Electron match!!"<<std::endl;
		if(posMCMatch.second!=NULL)
		std::cout<<"found Positron match!!"<<std::endl; 
	  */
	  if(!_reg_vtx_selectors[region]->passCutEq("eleMCMatched",eleMCMatch.second!=NULL,weight))
	    continue;
	  if(!_reg_vtx_selectors[region]->passCutEq("posMCMatched",posMCMatch.second!=NULL,weight))
	    continue;
	  //check if MC matched particles are from gamma*	
	  bool isRadEle=false;
	  bool isRadPos=false;
	  if(eleMCMatch.second!=NULL){
	    isRadEle=eleMCMatch.second->getMomPDG()==622;
	    if(isRadEle)
	      std::cout<<"Found radiative electron"<<std::endl;
	  }
	  if(posMCMatch.second!=NULL){
	    isRadPos=posMCMatch.second->getMomPDG()==622;
	    if(isRadEle)
	      std::cout<<"Found radiative positron"<<std::endl;
	  }
	  if(!_reg_vtx_selectors[region]->passCutEq("eleMCGammaSt",isRadEle,weight))
	    continue;
	  if(!_reg_vtx_selectors[region]->passCutEq("posMCGammaSt",isRadPos,weight))
	    continue;
	}
	
	double corr_eleClusterTime=666;
	double corr_posClusterTime=666;
	
        corr_eleClusterTime = ele_clu.getTime() - calTimeOffset_;
        corr_posClusterTime = pos_clu.getTime() - calTimeOffset_;

        double corr_eleTrackTime=-666;
        double corr_posTrackTime=-666;
        
        corr_eleTrackTime=ele_trk.getTrackTime() - trkTimeOffset_;
        corr_posTrackTime=pos_trk.getTrackTime() - trkTimeOffset_;

        bool hasEleCluster=corr_eleClusterTime>-300; //this is kludgy but the default time is set at -9999 ns...anything < ~ -100 ns means it was not filled...
        bool hasPosCluster=corr_posClusterTime>-300;
        //        std::cout<<"eleCluster Time = "<<corr_eleClusterTime<<"; posCluster Time = "<<corr_posClusterTime<<std::endl;
	if(!_reg_vtx_selectors[region]->passCutEq("eleClusterMatched",hasEleCluster,weight))
	  continue;
	if(!_reg_vtx_selectors[region]->passCutEq("posClusterMatched",hasPosCluster,weight))
	  continue;
	        
	bool foundL1ele = false;
	bool foundL2ele = false;
        //_ah->InnermostLayerCheck(ele_trk_from_trkList, foundL1ele, foundL2ele);   
        _ah->InnermostLayerCheck(&ele_trk, foundL1ele, foundL2ele);   
        
	bool foundL1pos = false;
	bool foundL2pos = false;
        //_ah->InnermostLayerCheck(pos_trk_from_trkList, foundL1pos, foundL2pos);  
       	_ah->InnermostLayerCheck(&pos_trk, foundL1pos, foundL2pos);  
        
	int layerCombo=-1;
	if (foundL1pos&&foundL1ele) //L1L1
	  layerCombo=1;
	if(!foundL1pos&&foundL2pos&&foundL1ele)//L2L1
	  layerCombo=2;
	if(foundL1pos&&!foundL1ele&&foundL2ele)//L1L2 
	  layerCombo=3;
	if(!foundL1pos&&foundL2pos&&!foundL1ele&&foundL2ele)//L2L2
	  layerCombo=4;
        
        
	
	//	    std::cout<<"checking layer cut   "<<layerCombo<<std::endl;            
	if (!_reg_vtx_selectors[region]->passCutEq("LayerRequirement",layerCombo,weight))
	  continue;
	if(debug_)std::cout<<"passed layer cut layer combo = "<<layerCombo<<std::endl;     
        if(!_reg_vtx_selectors[region]->passCutEq("ele_FoundL2",foundL2ele,weight))
          continue;
        if(!_reg_vtx_selectors[region]->passCutEq("pos_FoundL2",foundL2pos,weight))
          continue;
	nvertPass++;  //this vertex passed all cuts (expect nVtx cut...)
	//N selected vertices - this is quite a silly cut to make at the end. But okay. that's how we decided atm.
        // actually, pick random vertex belore
	final_tri.push_back(vtx);

      }// preselected vertices
      _reg_vtx_histos[region]->Fill1DHisto("n_vertices_h",selected_tri.size(),weight);
      _reg_vtx_histos[region]->Fill1DHisto("n_vertices_passallcuts_h",nvertPass,weight);
      
      if(final_tri.size()==0)
	continue;
      // pick a random good vertex: 
      Vertex* vtx = *select_randomly(final_tri.begin(), final_tri.end());
      // get objects we need for this candidate....again...this is lame.  I should encapsulate this or something...
      Particle* ele = nullptr;
      Particle* pos = nullptr;        
      _ah->GetParticlesFromVtxAndParticleList(*fspart_,vtx,ele,pos);
      Track ele_trk = ele->getTrack();
      Track pos_trk = pos->getTrack();
      CalCluster ele_clu=ele->getCluster(); 
      CalCluster pos_clu=pos->getCluster();

      ///////////////////////////////////////////////////////////////////////////////////////////////

      double corr_eleClusterTime = ele_clu.getTime() - calTimeOffset_;
      double corr_posClusterTime = pos_clu.getTime() - calTimeOffset_;
      
      double corr_eleTrackTime=ele_trk.getTrackTime() - trkTimeOffset_;
      double corr_posTrackTime=pos_trk.getTrackTime() - trkTimeOffset_;
      if(debug_){
        std::cout<<"***********************************"<<std::endl;
        std::cout<<"nPositrons in event = "<<nPositrons<<std::endl;
        std::cout<<"positron position at ecal:  x = "<<pos_trk.getPositionAtEcal().at(0)
                 <<"; y = "<<pos_trk.getPositionAtEcal().at(1)<<"; p = "<<pos_trk.getMomentum().at(2)<<"  trk time = "<<corr_posTrackTime<<std::endl;
        std::cout<<"positron Cluster:  x = "<<pos_clu.getPosition().at(0)
                 <<"; y = "<<pos_clu.getPosition().at(1)<<"; E = "<<pos_clu.getEnergy()<<std::endl;
        std::cout<<"clus_->size() = "<<clus_->size()<<std::endl;
        for(int k_clu=0; k_clu<clus_->size(); k_clu++){
          CalCluster* clu=clus_->at(k_clu);
          double corr_ClusterTime = clu->getTime() - calTimeOffset_;
          std::cout<<"CalCluster k = "<<k_clu<<" x = "<<clu->getPosition().at(0)<<" y = "<<clu->getPosition().at(1)<<"  cluster time = "<< corr_ClusterTime<<"; E = "<<clu->getEnergy()<<std::endl;
        }
        
        std::cout<<"fspart_->size() = "<<fspart_->size()<<std::endl;
        for(int j_part=0; j_part<fspart_->size();j_part++){
          Particle* part=fspart_->at(j_part);
          Track trk=part->getTrack();
          CalCluster clu=part->getCluster();
          double corr_ClusterTime = clu.getTime() - calTimeOffset_;
          double corr_TrackTime=trk.getTrackTime() - trkTimeOffset_;
          std::cout<<"FSParticle track::  charge = "<<trk.getCharge()<<";  nHits = "<<trk.getTrackerHitCount()<<"; trk time = "<<corr_TrackTime<<std::endl;
          std::cout<<"FSParticle track::   x = "<<trk.getPositionAtEcal().at(0)
                   <<"; y = "<<trk.getPositionAtEcal().at(1)<<"; p = "<<trk.getMomentum().at(2)<<std::endl;
          std::cout<<"FSParticle cluster::  x = "<<clu.getPosition().at(0)<<" y = "<<clu.getPosition().at(1)<<"; E = "<<clu.getEnergy()<<"; clu time = "<<corr_ClusterTime<<std::endl;
        }
        std::cout<<"***********************************"<<std::endl;
        //      _reg_vtx_histos[region]->Fill1DHisto("ele_nHits_2d_h",ele_trk.getTrackerHitCount(),weight);
        // _reg_vtx_histos[region]->Fill1DHisto("pos_nHits_2d_h",pos_trk.getTrackerHitCount(),weight);
      }
      //	continue;    
      _reg_vtx_histos[region]->Fill2DHistograms(vtx,weight);
      _reg_vtx_histos[region]->Fill1DVertex(vtx,
                                            ele,
                                            pos,
                                            &ele_trk,
                                            &pos_trk,
                                            trkTimeOffset_,
                                            weight);
      
      _reg_vtx_histos[region]->Fill2DTrack(&ele_trk,weight,"ele_");
      _reg_vtx_histos[region]->Fill2DTrack(&pos_trk,weight,"pos_");
      _reg_vtx_histos[region]->FillTrackClusterHistos(std::pair<CalCluster,Track>(ele_clu,ele_trk),std::pair<CalCluster,Track>(pos_clu,pos_trk),calTimeOffset_,trkTimeOffset_,clus_,weight);
      
      
      if (trks_){
	_reg_vtx_histos[region]->Fill1DHisto("n_tracks_h",trks_->size(),weight);
        _reg_vtx_histos[region]->Fill1DHisto("n_positrons_h",nPositrons,weight);
        _reg_vtx_histos[region]->Fill1DHisto("n_electrons_h",nElectrons,weight);
      }
      _reg_vtx_histos[region]->Fill1DHisto("n_pos_clusters_h",nPosSideClusters,weight);
      _reg_vtx_histos[region]->Fill1DHisto("n_ele_clusters_h",nEleSideClusters,weight);

      //Just for the selected vertex
      _reg_tuples[region]->setVariableValue("unc_vtx_mass", vtx->getInvMass());
      
      //TODO put this in the Vertex!
      TVector3 vtxPosSvt;
      vtxPosSvt.SetX(vtx->getX());
      vtxPosSvt.SetY(vtx->getY());
      vtxPosSvt.SetZ(vtx->getZ());
      vtxPosSvt.RotateY(-0.0305);
      
      _reg_tuples[region]->setVariableValue("unc_vtx_z"   , vtxPosSvt.Z());
      _reg_tuples[region]->fill();
    } // regions
    
    
    /*   Ok...now do the WAB analysis  */ 
    
    //    std::vector<WABCand> selected_wab;
    /*
      if(debug_)  std::cout<<"Number of electrons = "<<goodElectrons.size()<<"; number of photons = "<<goodPhotons.size()<<std::endl;

    for(auto ele: goodElectrons){      
      Track* ele_trk=ele.second;
      CalCluster* ele_clu=ele.first;
      for(auto gamma:  goodPhotons){
	std::pair<CalCluster*,Track*> photPair(gamma,NULL);  //make this dumb pair for histo filling
	for(auto region : _regionsWAB){
	   //No cuts.
	  _reg_WAB_selectors[region]->getCutFlowHisto()->Fill(0.,weight);
	  //fill some histos....
	  bool foundL1ele = false;
	  bool foundL2ele = false;
          //_ah->InnermostLayerCheck(ele_trk, foundL1ele, foundL2ele);   
	  int layerCombo=3;//== no L1 or L2 hit
	  if(foundL1ele)
	    layerCombo=1;
	  else if(foundL2ele)
	    layerCombo=2;	  
	  if (!_reg_WAB_selectors[region]->passCutEq("LayerRequirement",layerCombo,weight))
	    continue;

	  if(!_reg_WAB_selectors[region]->passCutEq("eleClusterMatched",ele.first!=NULL,weight))
	    continue;
	  bool isFiducialElectron=_ah->IsECalFiducial(ele.first);
	  bool isFiducialGamma=_ah->IsECalFiducial(gamma);

	  if (!_reg_WAB_selectors[region]->passCutEq("cluFiducialElectron",isFiducialElectron,weight))
	    continue;
	  if (!_reg_WAB_selectors[region]->passCutEq("cluFiducialGamma",isFiducialGamma,weight))
	    continue;

	  double corr_eleClusterTime=-666;
	  if(ele_clu !=NULL) corr_eleClusterTime = ele_clu->getTime() - timeOffset_;
	  double corr_gamClusterTime = gamma->getTime() - timeOffset_;

	  //gamma cluster time
	   if (!_reg_WAB_selectors[region]->passCutLt("gamCluTime_lt",fabs(corr_gamClusterTime),weight))
	      continue;
	  //Ele Pos Cluster Tme Difference
	  if(ele_clu!=NULL)
	    if (!_reg_WAB_selectors[region]->passCutLt("elegamCluTimeDiff_lt",fabs(corr_eleClusterTime - corr_gamClusterTime),weight))
	      continue;
	  
	  //Ele Track-Cluster Time Difference
	  if(ele_clu!=NULL)
	    if (!_reg_WAB_selectors[region]->passCutLt("eleTrkCluTimeDiff_lt",fabs(ele_trk->getTrackTime() - corr_eleClusterTime),weight))
	      continue;	   
	  
	  _reg_WAB_histos[region]->Fill1DTrack(ele.second,weight,"ele_");
	  _reg_WAB_histos[region]->FillWABHistos(ele,gamma,weight);
	  _reg_WAB_histos[region]->FillTrackClusterHistos(ele,photPair,timeOffset_,weight);	  
          std::cout<<"Filled WAB Histos"<<std::endl;
	}
      }      
      }
    */
    return true;
}

void TridentWABAnaProcessor::finalize() {
    
    //TODO clean this up a little.
    outF_->cd();
    _vtx_histos->saveHistos(outF_,_vtx_histos->getName());
    outF_->cd(_vtx_histos->getName().c_str());
    vtxSelector->getCutFlowHisto()->Write();
        
    
    for (reg_it it = _reg_vtx_histos.begin(); it!=_reg_vtx_histos.end(); ++it) {
        (it->second)->saveHistos(outF_,it->first);
        outF_->cd((it->first).c_str());
        _reg_vtx_selectors[it->first]->getCutFlowHisto()->Write();
        //Save tuples
        _reg_tuples[it->first]->writeTree();
    }
      for (reg_it it = _reg_WAB_histos.begin(); it!=_reg_WAB_histos.end(); ++it) {
        (it->second)->saveHistos(outF_,it->first);
        outF_->cd((it->first).c_str());
        _reg_WAB_selectors[it->first]->getCutFlowHisto()->Write();
    }
    outF_->Close();
        
}

//get the v0 from the tracks after unduplicating and matching to clusters
Vertex* TridentWABAnaProcessor::matchPairToVertex(Track* eleTrk,Track* posTrk, std::vector<Vertex*>& verts){
  Vertex* matchedVert=NULL; 
  for (auto vtx: verts){
    Particle* ele=NULL; 
    Particle* pos=NULL; 
    bool foundParts = _ah->GetParticlesFromVtx(vtx,ele,pos);
    Track eleVtx=ele->getTrack();
    Track posVtx=pos->getTrack();   
    //    std::cout<<eleVtx.getID()<<"  "<<eleTrk->getID()<<"   "<<posVtx.getID()<<"   "<<posTrk->getID()<<std::endl;
    if (eleVtx.getID()==eleTrk->getID() && posVtx.getID()==posTrk->getID())
      matchedVert=vtx;
  }
  return matchedVert; 
}

std::pair<Track*,MCParticle*> TridentWABAnaProcessor::matchToMCParticle(Track* trk, std::vector<MCParticle*>& mcParts){
  int minHitsForMatch=3; 
  MCParticle* matchedMCPart=NULL; 
  TRefArray trk_hits = trk->getSvtHits();
  std::map<int, std::vector<int> > trueHitIDs;

  for(int i = 0; i < hits_->size(); i++) {
    TrackerHit* hit = hits_->at(i);
    trueHitIDs[hit->getID()] = hit->getMCPartIDs();
  }
  std::map<int, int> nHits4part;
  for(int i = 0; i < trk_hits.GetEntries(); i++){
    TrackerHit* eleHit = (TrackerHit*)trk_hits.At(i);
    for(int idI = 0; idI < trueHitIDs[eleHit->getID()].size(); idI++ ){
	int partID = trueHitIDs[eleHit->getID()].at(idI);
	if ( nHits4part.find(partID) == nHits4part.end() ) {
	  // not found
	  nHits4part[partID] = 1;
	} 
	else {
	  // found
	  nHits4part[partID]++;
	}
    }
  }
  
  //Determine the MC part with the most hits on the track
  int maxNHits = 0;
  int maxID = 0;
  for (std::map<int,int>::iterator it=nHits4part.begin(); it!=nHits4part.end(); ++it){
    if(it->second > maxNHits){
      maxNHits = it->second;
      maxID = it->first;
    }
  }
  //  std::cout<<"Got maxID and nHits = "<<maxNHits<<std::endl;

  if(maxNHits>=minHitsForMatch){
    for(int i = 0; i < mcParts.size(); i++){
      if(mcParts.at(i)->getID() != maxID) continue;
      //      std::cout<<"Found Match"<<std::endl;
      matchedMCPart=mcParts.at(i);
    }  
  }

  return std::pair<Track*,MCParticle*>(trk,matchedMCPart);
}

std::vector<CalCluster*> TridentWABAnaProcessor::getUnmatchedClusters(std::vector<CalCluster*>& allClusters,std::vector<std::pair<CalCluster*,Track*> > electrons, std::vector<std::pair<CalCluster*,Track*> > positrons){
  std::vector<CalCluster*> unmatchedClusters; 

  for(auto clu: allClusters){
    bool foundMatch=false;
    for (auto ele: electrons){
      CalCluster* eleCl=ele.first;
      if(clu == eleCl)
	foundMatch=true; 
    }
    for (auto pos: positrons){
      CalCluster* posCl=pos.first;
      if(clu == posCl)
	foundMatch=true; 
    }
    if(!foundMatch)
      unmatchedClusters.push_back(clu); 
  }
  return unmatchedClusters; 
}

DECLARE_PROCESSOR(TridentWABAnaProcessor);
