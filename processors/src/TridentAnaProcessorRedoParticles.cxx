/** 
 *@file TridentAnaProcessorRedoParticles.cxx
 *@brief Main trident and WAB analysis processor
 *@author MattG, SLAC
 * based off of PF's VertexAnaProcessor
 */

#include "TridentAnaProcessorRedoParticles.h"
#include <iostream>
#include <utility> 
TridentAnaProcessorRedoParticles::TridentAnaProcessorRedoParticles(const std::string& name, Process& process) : Processor(name,process) {

}

//TODO Check this destructor

TridentAnaProcessorRedoParticles::~TridentAnaProcessorRedoParticles(){}

void TridentAnaProcessorRedoParticles::configure(const ParameterSet& parameters) {
    std::cout << "Configuring TridentAnaProcessorRedoParticles" <<std::endl;
    try 
    {
        debug_   = parameters.getInteger("debug");
        anaName_ = parameters.getString("anaName");
	cluColl_ = parameters.getString("cluColl");
        vtxColl_ = parameters.getString("vtxColl");
        trkColl_ = parameters.getString("trkColl");
	//        trkSelCfg_   = parameters.getString("trkSelectionjson");
        selectionCfg_   = parameters.getString("vtxSelectionjson");
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

void TridentAnaProcessorRedoParticles::initialize(TTree* tree) {
    tree_ = tree;
    _ah =  std::make_shared<AnaHelpers>();
    
    // trkSelector  = std::make_shared<BaseSelector>("trkSelection",trkSelCfg_);
    //    trkSelector->setDebug(debug_);
    //trkSelector->LoadSelection();
        
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
     
     
     //init Reading Tree
     tree_->SetBranchAddress(vtxColl_.c_str(), &vtxs_ , &bvtxs_);
     tree_->SetBranchAddress(fspartColl_.c_str(), &fspart_ , &bfspart_);
     tree_->SetBranchAddress("EventHeader",&evth_ , &bevth_);
     tree_->SetBranchAddress(cluColl_.c_str(), &clus_ , &bclus_);
     //If track collection name is empty take the tracks from the particles. TODO:: change this
     if (!trkColl_.empty())
       tree_->SetBranchAddress(trkColl_.c_str(),&trks_, &btrks_);
}

bool TridentAnaProcessorRedoParticles::process(IEvent* ievent) { 
    
    HpsEvent* hps_evt = (HpsEvent*) ievent;
    double weight = 1.;

    //Store processed number of events
    
    std::vector<Track*> noDups;

    // remove similar tracks == more than 1 shared hit; choose "best" track
    for (int i_trk;i_trk<trks_->size();i_trk++){
      Track* trk=trks_->at(i_trk);
      if(_ah->IsBestTrack(trk,*trks_))
	noDups.push_back(trk);      
    }

    //
    
    if(trks_->size()!=noDups.size())
      std::cout<<"with dups = "<<trks_->size()<<";  no dups = "<<noDups.size()<<std::endl;
    
    std::vector<std::pair<CalCluster*,Track*> > goodElectrons;
    std::vector<std::pair<CalCluster*,Track*> > goodPositrons;
    std::vector<CalCluster*> goodPhotons;

    for(int i_trk; i_trk<noDups.size();i_trk++){
      Track* trk=noDups.at(i_trk);
      int ch=-1*trk->getCharge();  //track charge is flipped
      std::pair<CalCluster*,Track*>  trkcluPair= _vtx_histos->getTrackClusterPair(trk,*clus_,weight);
      if(ch==-1){
	goodElectrons.push_back(trkcluPair);
      }else{
	goodPositrons.push_back(trkcluPair);
      }	      
    }

    if(debug_)
      std::cout<<"Electrons = "<<goodElectrons.size()<<"; Positrons = "<<goodPositrons.size()<<"; Photons = "<<goodPhotons.size()<<std::endl;

    std::vector<TridentCand> selected_tri;
    std::vector<TridentCand> matched_tri;

    for (auto elecluPair : goodElectrons){
      for (auto poscluPair : goodPositrons){
	//std::cout<<"trying to match ele+pos to vertex"<<std::endl;
	Vertex* matchedVtx=matchPairToVertex(elecluPair.second,poscluPair.second,*vtxs_);
	if(matchedVtx==NULL) { //if we didn't find a match, continue
	  //	  std::cout<<"No vertex match"<<std::endl;
	  continue;
	}
	//	std::cout<<"found a match!"<<std::endl;
	TridentCand cand={matchedVtx,elecluPair.first,poscluPair.first};
	matched_tri.push_back(cand);
      }
    }
    
    std::cout<<"Found "<<matched_tri.size()<<" Matched Vertices"<<std::endl;
    for ( int i_vtx = 0; i_vtx < matched_tri.size(); i_vtx++ ) {
        
        vtxSelector->getCutFlowHisto()->Fill(0.,weight);
        
	TridentCand cand=matched_tri.at(i_vtx);
        Vertex* vtx = cand.vertex;
        Particle* ele = nullptr;
        Track* ele_trk = nullptr;
        Particle* pos = nullptr;
        Track* pos_trk = nullptr;
	CalCluster* ele_clu=cand.eleClu;
	CalCluster* pos_clu=cand.posClu;

        //Trigger requirement - *really hate* having to do it here for each vertex.
	//std::cout<<"Checking Trigger"<<std::endl;
	//        if (isData) {
        //    if (!vtxSelector->passCutEq("Pair1_eq",(int)evth_->isPair1Trigger(),weight))
	///         break;
	// }
                
        bool foundParts = _ah->GetParticlesFromVtx(vtx,ele,pos);
	//	std::cout<<"Checking Particles"<<std::endl;
        if (!foundParts) {
            //std::cout<<"TridentAnaProcessorRedoParticles::WARNING::Found vtx without ele/pos. Skip."
            continue;
        }
        
        if (!trkColl_.empty()) {
	  //	  std::cout<<"Checking Tracks"<<std::endl;
            bool foundTracks = _ah->MatchToGBLTracks((ele->getTrack()).getID(),(pos->getTrack()).getID(),
                                                     ele_trk, pos_trk, *trks_);
            if (!foundTracks) {
	      std::cout<<"TridentAnaProcessorRedoParticles::ERROR couldn't find ele/pos in the GBLTracks collection"<<std::endl;
	      continue;  
            }
        }
        else {
            ele_trk = (Track*)ele->getTrack().Clone();
            pos_trk = (Track*)pos->getTrack().Clone();
        }
        
        //Add the momenta to the tracks
        ele_trk->setMomentum(ele->getMomentum()[0],ele->getMomentum()[1],ele->getMomentum()[2]);
        pos_trk->setMomentum(pos->getMomentum()[0],pos->getMomentum()[1],pos->getMomentum()[2]);
                
        
        //Tracks in opposite volumes - useless
        //if (!vtxSelector->passCutLt("eleposTanLambaProd_lt",ele_trk->getTanLambda() * pos_trk->getTanLambda(),weight)) 
        //  continue;
        
        //Ele Track-cluster match
	/*  this part is already done and hardcoded in TridentHistos */
	//        if (!vtxSelector->passCutLt("eleTrkCluMatch_lt",ele->getGoodnessOfPID(),weight))
        //    continue;

        //Pos Track-cluster match
        //if (!vtxSelector->passCutLt("posTrkCluMatch_lt",pos->getGoodnessOfPID(),weight))
        //    continue;
       

        
        TVector3 ele_mom;
        ele_mom.SetX(ele->getMomentum()[0]);
        ele_mom.SetY(ele->getMomentum()[1]);
        ele_mom.SetZ(ele->getMomentum()[2]);

        TVector3 pos_mom;
        pos_mom.SetX(pos->getMomentum()[0]);
        pos_mom.SetY(pos->getMomentum()[1]);
        pos_mom.SetZ(pos->getMomentum()[2]);
        
        
        //Beam Electron cut
        if (!vtxSelector->passCutLt("eleMom_lt",ele_mom.Mag(),weight))
            continue;

        //Ele Track Quality
        if (!vtxSelector->passCutLt("eleTrkChi2_lt",ele_trk->getChi2Ndf(),weight))
            continue;
        
        //Pos Track Quality
        if (!vtxSelector->passCutLt("posTrkChi2_lt",pos_trk->getChi2Ndf(),weight))
            continue;

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
	   
        if (!vtxSelector->passCutGt("minVtxMom_gt",(ele_mom+pos_mom).Mag()/beamE_,weight))
            continue;

        _vtx_histos->Fill1DVertex(vtx,
                                  ele,
                                  pos,
                                  ele_trk,
                                  pos_trk,
                                  weight);
        
        _vtx_histos->Fill2DHistograms(vtx,weight);
        _vtx_histos->Fill2DTrack(ele_trk,weight,"ele_");
        _vtx_histos->Fill2DTrack(pos_trk,weight,"pos_");
        
        selected_tri.push_back(cand);       
        vtxSelector->clearSelector();
    }
    
    std::cout<<"Found "<<selected_tri.size()<<" Selected Vertices"<<std::endl;
    _vtx_histos->Fill1DHisto("n_vertices_h",selected_tri.size()); 
    if (trks_)
        _vtx_histos->Fill1DHisto("n_tracks_h",trks_->size()); 
    
    
    //Make Plots for each region: loop on each region. Check if the region has the cut and apply it

    for ( auto cand : selected_tri) {
      Vertex* vtx = cand.vertex;
      Particle* ele = nullptr;
      Track* ele_trk = nullptr;
      Particle* pos = nullptr;
      Track* pos_trk = nullptr;
      CalCluster* ele_clu=cand.eleClu;
      CalCluster* pos_clu=cand.posClu;
      for (auto region : _regions ) {
	
            //No cuts.
            _reg_vtx_selectors[region]->getCutFlowHisto()->Fill(0.,weight);
            
            
            Particle* ele = nullptr;
            Particle* pos = nullptr;
            
            _ah->GetParticlesFromVtx(vtx,ele,pos);
                                  
            double ele_E = ele->getEnergy();
            double pos_E = pos->getEnergy();


            //Compute analysis variables here.
            
            Track ele_trk = ele->getTrack();
            Track pos_trk = pos->getTrack();
            
            //Get the shared info - TODO change and improve
            
            Track* ele_trk_gbl = nullptr;
            Track* pos_trk_gbl = nullptr;
            
            if (!trkColl_.empty()) {
                bool foundTracks = _ah->MatchToGBLTracks(ele_trk.getID(),pos_trk.getID(),
                                                         ele_trk_gbl, pos_trk_gbl, *trks_);
                
                if (!foundTracks) {
                    //std::cout<<"TridentAnaProcessorRedoParticles::ERROR couldn't find ele/pos in the GBLTracks collection"<<std::endl;
                    continue;  
                }
            }
            else {
                
                ele_trk_gbl = (Track*) ele_trk.Clone();
                pos_trk_gbl = (Track*) pos_trk.Clone();
            }            


	    double corr_eleClusterTime=666;
	    double corr_posClusterTime=666;
	    
	    if(ele_clu !=NULL) corr_eleClusterTime = ele_clu->getTime() - timeOffset_;
	    if(pos_clu !=NULL) corr_posClusterTime = pos_clu->getTime() - timeOffset_;
	    
	    //	std::cout<<"Checking ClusterTimeDiff"<<std::endl;
	    //Ele Pos Cluster Tme Difference
	    if(ele_clu!=NULL && pos_clu!=NULL)
	      if (!_reg_vtx_selectors[region]->passCutLt("eleposCluTimeDiff_lt",fabs(corr_eleClusterTime - corr_posClusterTime),weight))
		continue;
	    
	    //Ele Track-Cluster Time Difference
	    if(ele_clu!=NULL)
	      if (!_reg_vtx_selectors[region]->passCutLt("eleTrkCluTimeDiff_lt",fabs(ele_trk.getTrackTime() - corr_eleClusterTime),weight))
		continue;
	    
	    //Pos Track-Cluster Time Difference
	    if(pos_clu!=NULL)
	      if (!_reg_vtx_selectors[region]->passCutLt("posTrkCluTimeDiff_lt",fabs(pos_trk.getTrackTime() - corr_posClusterTime),weight))
		continue;
	    
                       
            bool foundL1ele = false;
            bool foundL2ele = false;
            _ah->InnermostLayerCheck(ele_trk_gbl, foundL1ele, foundL2ele);   
            
            bool foundL1pos = false;
            bool foundL2pos = false;
            _ah->InnermostLayerCheck(pos_trk_gbl, foundL1pos, foundL2pos);  
            
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
	    //std::cout<<"passed layer cut"<<std::endl;
                                  

            //N selected vertices - this is quite a silly cut to make at the end. But okay. that's how we decided atm.
            if (!_reg_vtx_selectors[region]->passCutEq("nVtxs_eq",selected_tri.size(),weight))	      
	      continue;
            _reg_vtx_histos[region]->Fill2DHistograms(vtx,weight);
            _reg_vtx_histos[region]->Fill1DVertex(vtx,
                                                  ele,
                                                  pos,
                                                  ele_trk_gbl,
                                                  pos_trk_gbl,
                                                  weight);

            _reg_vtx_histos[region]->Fill2DTrack(ele_trk_gbl,weight,"ele_");
            _reg_vtx_histos[region]->Fill2DTrack(pos_trk_gbl,weight,"pos_");
            

            
            if (trks_)
                _reg_vtx_histos[region]->Fill1DHisto("n_tracks_h",trks_->size(),weight);
            _reg_vtx_histos[region]->Fill1DHisto("n_vertices_h",selected_tri.size(),weight);
            
            
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
        }// regions
    } // preselected vertices
    
    
    
    return true;
}

void TridentAnaProcessorRedoParticles::finalize() {
    
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
    
    outF_->Close();
        
}

//get the v0 from the tracks after unduplicating and matching to clusters
Vertex* TridentAnaProcessorRedoParticles::matchPairToVertex(Track* eleTrk,Track* posTrk, std::vector<Vertex*>& verts){
  Vertex* matchedVert=NULL; 
  for (auto vtx: verts){
    Particle* ele=NULL; 
    Particle* pos=NULL; 
    bool foundParts = _ah->GetParticlesFromVtx(vtx,ele,pos);
    Track eleVtx=ele->getTrack();
    Track posVtx=pos->getTrack();   
    if (eleVtx.getID()==eleTrk->getID() && posVtx.getID()==posTrk->getID())
      matchedVert=vtx;
  }
  return matchedVert; 
}




DECLARE_PROCESSOR(TridentAnaProcessorRedoParticles);
