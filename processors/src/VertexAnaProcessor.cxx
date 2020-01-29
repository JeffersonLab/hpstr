/** 
 *@file VertexAnaProcessor.cxx
 *@brief Main vertex analysis processor
 *@author PF, SLAC
 */

#include "VertexAnaProcessor.h"
#include <iostream>

VertexAnaProcessor::VertexAnaProcessor(const std::string& name, Process& process) : Processor(name,process) {

}

//TODO Check this destructor

VertexAnaProcessor::~VertexAnaProcessor(){}

void VertexAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring VertexAnaProcessor" <<std::endl;
    try 
    {
        debug_   = parameters.getInteger("debug");
        anaName_ = parameters.getString("anaName");
        vtxColl_ = parameters.getString("vtxColl");
        trkColl_ = parameters.getString("trkColl");
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

void VertexAnaProcessor::initialize(TTree* tree) {
    tree_ = tree;
    _ah =  std::make_shared<AnaHelpers>();
    vtxSelector  = std::make_shared<BaseSelector>("vtxSelection",selectionCfg_);
    vtxSelector->setDebug(debug_);
    vtxSelector->LoadSelection();
        
    _vtx_histos = std::make_shared<TrackHistos>("vtxSelection");
    _vtx_histos->loadHistoConfig(histoCfg_);
    _vtx_histos->DefineHistos();
    

    //For each region initialize plots
    
    for (unsigned int i_reg = 0; i_reg < regionSelections_.size(); i_reg++) {
        std::string regname = AnaHelpers::getFileName(regionSelections_[i_reg],false);
        std::cout<<"Setting up region:: " << regname <<std::endl;   
        _reg_vtx_selectors[regname] = std::make_shared<BaseSelector>(regname, regionSelections_[i_reg]);
        _reg_vtx_selectors[regname]->setDebug(debug_);
        _reg_vtx_selectors[regname]->LoadSelection();
        
        _reg_vtx_histos[regname] = std::make_shared<TrackHistos>(regname);
        _reg_vtx_histos[regname]->loadHistoConfig(histoCfg_);
        _reg_vtx_histos[regname]->DefineHistos();
        
        _regions.push_back(regname);
    }
    
    
    //init Reading Tree
    tree_->SetBranchAddress(vtxColl_.c_str(), &vtxs_ , &bvtxs_);
    tree_->SetBranchAddress("EventHeader",&evth_ , &bevth_);
    tree_->SetBranchAddress(trkColl_.c_str(),&trks_, &btrks_);
}

bool VertexAnaProcessor::process(IEvent* ievent) { 
    
    HpsEvent* hps_evt = (HpsEvent*) ievent;
    double weight = 1.;

    //Store processed number of events
    std::vector<Vertex*> selected_vtxs;
    
    for ( int i_vtx = 0; i_vtx <  vtxs_->size(); i_vtx++ ) {
        
        vtxSelector->getCutFlowHisto()->Fill(0.,weight);
        
        Vertex* vtx = vtxs_->at(i_vtx);
        Particle* ele = nullptr;
        Track* ele_trk = nullptr;
        Particle* pos = nullptr;
        Track* pos_trk = nullptr;

        //Trigger requirement - *really hate* having to do it here for each vertex.
            
        if (isData) {
            if (!vtxSelector->passCutEq("Pair1_eq",(int)evth_->isPair1Trigger(),weight))
                break;
        }
                
        bool foundParts = _ah->GetParticlesFromVtx(vtx,ele,pos);
        if (!foundParts) {
            std::cout<<"VertexAnaProcessor::WARNING::Found vtx without ele/pos. Skip.";
            continue;
        }
        
        bool foundTracks = _ah->MatchToGBLTracks((ele->getTrack()).getID(),(pos->getTrack()).getID(),
                                                 ele_trk, pos_trk, *trks_);
        
        if (!foundTracks) {
            std::cout<<"VertexAnaProcessor::ERROR couldn't find ele/pos in the GBLTracks collection"<<std::endl;
            continue;  
        }

        bool foundL1ele = false;
        bool foundL2ele = false;
        _ah->InnermostLayerCheck(ele_trk, foundL1ele, foundL2ele);   
        
        bool foundL1pos = false;
        bool foundL2pos = false;
        _ah->InnermostLayerCheck(pos_trk, foundL1pos, foundL2pos);  
        
        //L1 requirement
        if (!vtxSelector->passCutEq("L1Requirement_eq",(int)(foundL1ele&&foundL1pos),weight))
            continue;
        
        //L2 requirement
        if (!vtxSelector->passCutEq("L2Requirement_eq",(int)(foundL2ele&&foundL2pos),weight))
            continue;
        
        //Tracks in opposite volumes - useless
        //if (!vtxSelector->passCutLt("eleposTanLambaProd_lt",ele_trk->getTanLambda() * pos_trk->getTanLambda(),weight)) 
        //  continue;
        
        //Ele Track-cluster match
        if (!vtxSelector->passCutLt("eleTrkCluMatch_lt",ele->getGoodnessOfPID(),weight))
            continue;

        //Pos Track-cluster match
        if (!vtxSelector->passCutLt("posTrkCluMatch_lt",pos->getGoodnessOfPID(),weight))
            continue;

        double corr_eleClusterTime = ele->getCluster().getTime() - timeOffset_;
        double corr_posClusterTime = pos->getCluster().getTime() - timeOffset_;
                
        //Ele Pos Cluster Tme Difference
        if (!vtxSelector->passCutLt("eleposCluTimeDiff_lt",fabs(corr_eleClusterTime - corr_posClusterTime),weight))
            continue;
        
        //Ele Track-Cluster Time Difference
        if (!vtxSelector->passCutLt("eleTrkCluTimeDiff_lt",fabs(ele_trk->getTrackTime() - corr_eleClusterTime),weight))
            continue;
        
        //Pos Track-Cluster Time Difference
        if (!vtxSelector->passCutLt("posTrkCluTimeDiff_lt",fabs(pos_trk->getTrackTime() - corr_posClusterTime),weight))
            continue;
        
        TVector3 ele_mom;
        ele_mom.SetX(ele->getMomentum()[0]);
        ele_mom.SetY(ele->getMomentum()[1]);
        ele_mom.SetZ(ele->getMomentum()[2]);
        
        
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

        
        
                
        //Fill Histograms for Vtx Preselection
        _vtx_histos->Fill1DHistograms(nullptr,vtx,weight);
        _vtx_histos->Fill2DHistograms(nullptr,vtx,weight);
        
        
        selected_vtxs.push_back(vtx);       
        vtxSelector->clearSelector();
    }
    
    _vtx_histos->Fill1DHisto("n_vertices_h",selected_vtxs.size()); 
    
    
    //not working atm
    //hps_evt->addVertexCollection("selected_vtxs", selected_vtxs);
    
    //Make Plots for each region: loop on each region. Check if the region has the cut and apply it
    //TODO Clean this up => Cuts should be implemented in each region? 
    //TODO Bring the preselection out of this stupid loop

    
    //TODO add yields.
    for (auto region : _regions ) {
        
        for ( auto vtx : selected_vtxs) {
            
            //No cuts.
            _reg_vtx_selectors[region]->getCutFlowHisto()->Fill(0.,weight);
            
            
            //N selected vertices
            if (!_reg_vtx_selectors[region]->passCutEq("nVtxs_eq",selected_vtxs.size(),weight))
                continue;
            
            
            Particle* ele = nullptr;
            Particle* pos = nullptr;
            
            _ah->GetParticlesFromVtx(vtx,ele,pos);
            
            //Chi2
            if (!_reg_vtx_selectors[region]->passCutLt("chi2unc_lt",vtx->getChi2(),weight))
                continue;
            
            double ele_E = ele->getEnergy();
            double pos_E = pos->getEnergy();
            
            
            //ESum 
            if (!_reg_vtx_selectors[region]->passCutLt("eSum_lt",(ele_E+pos_E)/beamE_,weight))
                continue;
            
            //ESum 
            if (!_reg_vtx_selectors[region]->passCutGt("eSum_gt",(ele_E+pos_E)/beamE_,weight))
                continue;
            
            //_reg_vtx_histos[region]->Fill1DHistograms(nullptr,vtx,weight);
            
            

            //Compute analysis variables here.
            
            //Total number of tracks in the event:
            
            int Ntracks = trks_->size();
            
            Track ele_trk = ele->getTrack();
            Track pos_trk = pos->getTrack();
            
            //Get the shared info - TODO change and improve
            
            Track* ele_trk_gbl = nullptr;
            Track* pos_trk_gbl = nullptr;
            
            bool foundTracks = _ah->MatchToGBLTracks(ele_trk.getID(),pos_trk.getID(),
                                                     ele_trk_gbl, pos_trk_gbl, *trks_);
            
            if (!foundTracks) {
                std::cout<<"VertexAnaProcessor::ERROR couldn't find ele/pos in the GBLTracks collection"<<std::endl;
                continue;  
            }
            
            
            //No shared hits requirement
            if (!_reg_vtx_selectors[region]->passCutEq("ele_sharedL0_eq",(int)ele_trk_gbl->getSharedLy0(),weight))
                continue;
            if (!_reg_vtx_selectors[region]->passCutEq("pos_sharedL0_eq",(int)pos_trk_gbl->getSharedLy0(),weight))
                continue;
            if (!_reg_vtx_selectors[region]->passCutEq("ele_sharedL1_eq",(int)ele_trk_gbl->getSharedLy1(),weight))
                continue;
            if (!_reg_vtx_selectors[region]->passCutEq("pos_sharedL1_eq",(int)pos_trk_gbl->getSharedLy1(),weight))
                continue;
            
            
            _reg_vtx_histos[region]->Fill1DHisto("n_tracks_h",Ntracks,weight);
            _reg_vtx_histos[region]->Fill1DHisto("n_vertices_h",selected_vtxs.size(),weight);
            _reg_vtx_histos[region]->Fill1DVertex(vtx,
                                                  ele,
                                                  pos,
                                                  ele_trk_gbl,
                                                  pos_trk_gbl,
                                                  weight);
            _reg_vtx_histos[region]->Fill2DHistograms(nullptr,vtx,weight);
        }// selected vertices
    }//regions
    
    return true;
}

void VertexAnaProcessor::finalize() {
    
    //TODO clean this up a little.
    outF_->cd();
    _vtx_histos->saveHistos(outF_,_vtx_histos->getName());
    outF_->cd(_vtx_histos->getName().c_str());
    vtxSelector->getCutFlowHisto()->Write();
        
    
    for (reg_it it = _reg_vtx_histos.begin(); it!=_reg_vtx_histos.end(); ++it) {
        (it->second)->saveHistos(outF_,it->first);
        outF_->cd((it->first).c_str());
        _reg_vtx_selectors[it->first]->getCutFlowHisto()->Write();
    }
    
    outF_->Close();
        
}

DECLARE_PROCESSOR(VertexAnaProcessor);
