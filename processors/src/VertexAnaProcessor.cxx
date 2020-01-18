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
    
    //init Reading Tree
    tree_->SetBranchAddress(vtxColl_.c_str(), &vtxs_ , &bvtxs_);
}

bool VertexAnaProcessor::process(IEvent* ievent) { 
    
    double weight = 1.;

    //Store processed number of events
    vtxSelector->getCutFlowHisto()->Fill(0.,weight);
        
    if (!vtxSelector->passCutEq("numVtx_eq",vtxs_->size(),weight))
        return false;
    if (!vtxSelector->passCutLt("chi2unc_lt",vtxs_->at(0)->getChi2(),weight))
        return false;
    
    //Track associated to the vertex
    Vertex* vtx = vtxs_->at(0);
    Particle* ele = nullptr;
    Particle* pos = nullptr;
    
    
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
            return false;
        }
    }

    
    if (!ele || !pos) {
        std::cout<<"VertexAnaProcessor::Vertex formed without ele/pos. Skip."<<std::endl;
        return false;
    }

    
    //Get the ele and pos tracks
    Track ele_trk = ele->getTrack();
    Track pos_trk = pos->getTrack();
    
    
    if (!vtxSelector->passCutLt("eleposTanLambaProd_lt",ele_trk.getTanLambda() * pos_trk.getTanLambda(),weight)) 
        return false;
    
    if (!vtxSelector->passCutLt("eleTrkCluMatch_lt",ele->getGoodnessOfPID(),weight))
        return false;
    if (!vtxSelector->passCutLt("posTrkCluMatch_lt",pos->getGoodnessOfPID(),weight))
        return false;
    
    
    double corr_eleClusterTime = ele->getCluster().getTime() - timeOffset_;
    double corr_posClusterTime = pos->getCluster().getTime() - timeOffset_;
    
    if (!vtxSelector->passCutLt("eleTrkCluTimeDiff_lt",fabs(ele_trk.getTrackTime() - corr_eleClusterTime),weight))
        return false;

    if (!vtxSelector->passCutLt("posTrkCluTimeDiff_lt",fabs(pos_trk.getTrackTime() - corr_posClusterTime),weight))
        return false;

    if (!vtxSelector->passCutLt("eleposCluTimeDiff_lt",fabs(corr_eleClusterTime - corr_posClusterTime),weight))
        return false;
    
    if (!vtxSelector->passCutLt("eleTrkChi2_lt",ele_trk.getChi2Ndf(),weight))
        return false;

    if (!vtxSelector->passCutLt("posTrkChi2_lt",pos_trk.getChi2Ndf(),weight))
        return false;
    
    TVector3 ele_mom;
    ele_mom.SetX(ele->getMomentum()[0]);
    ele_mom.SetY(ele->getMomentum()[1]);
    ele_mom.SetZ(ele->getMomentum()[2]);
    

    if (!vtxSelector->passCutLt("eleMom_lt",ele_mom.Mag(),weight))
        return false;
    
    
    _vtx_histos->Fill1DHistograms(nullptr,vtxs_->at(0),weight);
    _vtx_histos->Fill2DHistograms(nullptr,vtxs_->at(0),weight);
    
    return true;
}

void VertexAnaProcessor::finalize() {
    
    outF_->cd();
    vtxSelector->getCutFlowHisto()->Write();
    _vtx_histos->saveHistos();
    outF_->Close();
    
    
}

DECLARE_PROCESSOR(VertexAnaProcessor);
