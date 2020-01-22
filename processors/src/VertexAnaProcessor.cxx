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
    }
    catch (std::runtime_error& error) 
    {
        std::cout<<error.what()<<std::endl;
    }
}

void VertexAnaProcessor::initialize(TTree* tree) {
    tree_ = tree;
    vtxSelector  = std::make_shared<BaseSelector>("default",selectionCfg_);
    vtxSelector->setDebug(debug_);
    vtxSelector->LoadSelection();
        
    _vtx_histos = std::make_shared<TrackHistos>("default");
    _vtx_histos->loadHistoConfig(histoCfg_);
    _vtx_histos->DefineHistos();
    
    //Number of vertices after clean up
    nvtxs_ = new TH1F("n_vtxs","n_vtxs",10,0,10);
    nvtxs_->GetXaxis()->SetTitle("N_{vtxs}");
    nvtxs_->GetYaxis()->SetTitle("events");

    //init Reading Tree
    tree_->SetBranchAddress(vtxColl_.c_str(), &vtxs_ , &bvtxs_);
}

bool VertexAnaProcessor::process(IEvent* ievent) { 
    
    double weight = 1.;

    //Store processed number of events
    std::vector<Vertex*> selected_vtxs;
    
    for ( int i_vtx = 0; i_vtx <  vtxs_->size(); i_vtx++ ) {
        
        vtxSelector->getCutFlowHisto()->Fill(0.,weight);
        
        Vertex* vtx = vtxs_->at(i_vtx);
        Particle* ele = nullptr;
        Particle* pos = nullptr;
        
        if (!vtxSelector->passCutLt("chi2unc_lt",vtx->getChi2(),weight))
            continue;

        for (int ipart = 0; ipart < vtx->getParticles()->GetEntries(); ++ipart) {
            int pdg_id = ((Particle*)vtx->getParticles()->At(ipart))->getPDG();
            if (pdg_id == 11) {
                ele = (Particle*)vtx->getParticles()->At(ipart);
            }
            else if (pdg_id == -11) {
                pos = (Particle*)vtx->getParticles()->At(ipart);
            }
            
            else {
                std::cout<<"VertexAnaProcessor::Wrong particle ID "<< pdg_id <<"associated to vertex. Skip."<<std::endl;
                continue;
            }
        }
        
        
        if (!ele || !pos) {
            std::cout<<"VertexAnaProcessor::Vertex formed without ele/pos. Skip."<<std::endl;
            continue;
        }
        
        
        //Get the ele and pos tracks
        Track ele_trk = ele->getTrack();
        Track pos_trk = pos->getTrack();
        
        
        if (!vtxSelector->passCutLt("eleposTanLambaProd_lt",ele_trk.getTanLambda() * pos_trk.getTanLambda(),weight)) 
            continue;
        
        if (!vtxSelector->passCutLt("eleTrkCluMatch_lt",ele->getGoodnessOfPID(),weight))
            continue;
        if (!vtxSelector->passCutLt("posTrkCluMatch_lt",pos->getGoodnessOfPID(),weight))
            continue;
        
        
        double corr_eleClusterTime = ele->getCluster().getTime() - timeOffset_;
        double corr_posClusterTime = pos->getCluster().getTime() - timeOffset_;
        
        if (!vtxSelector->passCutLt("eleTrkCluTimeDiff_lt",fabs(ele_trk.getTrackTime() - corr_eleClusterTime),weight))
            continue;
        
        if (!vtxSelector->passCutLt("posTrkCluTimeDiff_lt",fabs(pos_trk.getTrackTime() - corr_posClusterTime),weight))
            continue;
        
        if (!vtxSelector->passCutLt("eleposCluTimeDiff_lt",fabs(corr_eleClusterTime - corr_posClusterTime),weight))
            continue;
        
        if (!vtxSelector->passCutLt("eleTrkChi2_lt",ele_trk.getChi2Ndf(),weight))
            continue;
        
        if (!vtxSelector->passCutLt("posTrkChi2_lt",pos_trk.getChi2Ndf(),weight))
            continue;
        
        TVector3 ele_mom;
        ele_mom.SetX(ele->getMomentum()[0]);
        ele_mom.SetY(ele->getMomentum()[1]);
        ele_mom.SetZ(ele->getMomentum()[2]);
        
        if (!vtxSelector->passCutLt("eleMom_lt",ele_mom.Mag(),weight))
            continue;
        
        _vtx_histos->Fill1DHistograms(nullptr,vtx,weight);
        _vtx_histos->Fill2DHistograms(nullptr,vtx,weight);
        
        selected_vtxs.push_back(vtx);       
    }
    
    nvtxs_->Fill(selected_vtxs.size(),weight);

    //Cut on number of vertices
    if (!vtxSelector->passCutEq("numVtx_eq",vtxs_->size(),weight))
        return false;
    
    
        
    return true;
}

void VertexAnaProcessor::finalize() {
    
    outF_->cd();
    vtxSelector->getCutFlowHisto()->Write();
    _vtx_histos->saveHistos();
    nvtxs_->Write();
    outF_->Close();
    
    
}

DECLARE_PROCESSOR(VertexAnaProcessor);
