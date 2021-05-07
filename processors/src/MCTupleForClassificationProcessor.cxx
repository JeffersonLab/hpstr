/**
 *@file MCTupleForClassificationProcessor.cxx
 *Extract data by loose cuts for classification analysis by machine learning
 */

#include "MCTupleForClassificationProcessor.h"
#include <iostream>
#include <math.h>

MCTupleForClassificationProcessor::MCTupleForClassificationProcessor(const std::string& name, Process& process) : Processor(name,process) {

}

//TODO Check this destructor

MCTupleForClassificationProcessor::~MCTupleForClassificationProcessor(){}

void MCTupleForClassificationProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring MCTupleForClassificationProcessor" <<std::endl;
    try
    {
        debug_   = parameters.getInteger("debug",debug_);
        anaName_ = parameters.getString("anaName",anaName_);
        vtxColl_ = parameters.getString("vtxColl",vtxColl_);
        trkColl_ = parameters.getString("trkColl",trkColl_);
        hitColl_ = parameters.getString("hitColl",hitColl_);
        mcColl_  = parameters.getString("mcColl",mcColl_);
        selectionCfg_   = parameters.getString("vtxSelectionjson",selectionCfg_);
        //histoCfg_ = parameters.getString("histoCfg",histoCfg_);
        beamE_  = parameters.getDouble("beamE",beamE_);
        isData_  = parameters.getInteger("isData",isData_);

    }
    catch (std::runtime_error& error)
    {
        std::cout<<error.what()<<std::endl;
    }
}

void MCTupleForClassificationProcessor::initialize(TTree* tree) {
    tree_ = tree;
    _ah =  std::make_shared<AnaHelpers>();

    vtxSelector  = std::make_shared<BaseSelector>(anaName_+"_"+"vtxSelection",selectionCfg_);
    vtxSelector->setDebug(debug_);
    vtxSelector->LoadSelection();

    /*
    _vtx_histos = std::make_shared<TrackHistos>(anaName_+"_"+"vtxSelection");
    _vtx_histos->loadHistoConfig(histoCfg_);
    _vtx_histos->DefineHistos();
     */

    //init Reading Tree
    tree_->SetBranchAddress(vtxColl_.c_str(), &vtxs_ , &bvtxs_);
    tree_->SetBranchAddress(hitColl_.c_str(), &hits_   , &bhits_);
    tree_->SetBranchAddress("EventHeader",&evth_ , &bevth_);
    if(!isData_ && !mcColl_.empty()) tree_->SetBranchAddress(mcColl_.c_str() , &mcParts_, &bmcParts_);
    //If track collection name is empty take the tracks from the particles. TODO:: change this
    if (!trkColl_.empty())
        tree_->SetBranchAddress(trkColl_.c_str(),&trks_, &btrks_);

    //init Writing tree
    treeOutput_ = new TTree("mcTuple", "recreate");

    treeOutput_->Branch("ele_mom_mag_", &ele_mom_mag_, "ele_mom_mag_/D");
    treeOutput_->Branch("ele_mom_theta_", &ele_mom_theta_, "ele_mom_theta_/D");
    treeOutput_->Branch("ele_mom_phi_", &ele_mom_phi_, "ele_mom_phi_/D");

    treeOutput_->Branch("pos_mom_mag_", &pos_mom_mag_, "pos_mom_mag_/D");
    treeOutput_->Branch("pos_mom_theta_", &pos_mom_theta_, "pos_mom_theta_/D");
    treeOutput_->Branch("pos_mom_phi_", &pos_mom_phi_, "pos_mom_phi_/D");

    treeOutput_->Branch("invariant_mass_", &invariant_mass_, "invariant_mass_/D");

    treeOutput_->Branch("openingAngle_", &openingAngle_, "openingAngle_/D");
}

bool MCTupleForClassificationProcessor::process(IEvent* ievent) {

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

        bool foundParts = _ah->GetParticlesFromVtx(vtx,ele,pos);
        if (!foundParts) {
            //std::cout<<"VertexAnaProcessor::WARNING::Found vtx without ele/pos. Skip."
            continue;
        }

        if (!trkColl_.empty()) {
            bool foundTracks = _ah->MatchToGBLTracks((ele->getTrack()).getID(),(pos->getTrack()).getID(),
                    ele_trk, pos_trk, *trks_);
            if (!foundTracks) {
                //std::cout<<"VertexAnaProcessor::ERROR couldn't find ele/pos in the GBLTracks collection"<<std::endl;
                continue;
            }
        }
        else {
            ele_trk = (Track*)ele->getTrack().Clone();
            pos_trk = (Track*)pos->getTrack().Clone();
        }

        //Add the momenta to the tracks - do not do that
        //ele_trk->setMomentum(ele->getMomentum()[0],ele->getMomentum()[1],ele->getMomentum()[2]);
        //pos_trk->setMomentum(pos->getMomentum()[0],pos->getMomentum()[1],pos->getMomentum()[2]);

        double ele_E = ele->getEnergy();
        double pos_E = pos->getEnergy();

        //Compute analysis variables here.
        TLorentzVector p_ele;
        p_ele.SetPxPyPzE(ele_trk->getMomentum()[0],ele_trk->getMomentum()[1],ele_trk->getMomentum()[2],ele->getEnergy());
        TLorentzVector p_pos;
        p_pos.SetPxPyPzE(pos_trk->getMomentum()[0],pos_trk->getMomentum()[1],pos_trk->getMomentum()[2],ele->getEnergy());

        TVector3 ele_mom;
        //ele_mom.SetX(ele->getMomentum()[0]);
        //ele_mom.SetY(ele->getMomentum()[1]);
        //ele_mom.SetZ(ele->getMomentum()[2]);
        ele_mom.SetX(ele_trk->getMomentum()[0]);
        ele_mom.SetY(ele_trk->getMomentum()[1]);
        ele_mom.SetZ(ele_trk->getMomentum()[2]);


        TVector3 pos_mom;
        pos_mom.SetX(pos_trk->getMomentum()[0]);
        pos_mom.SetY(pos_trk->getMomentum()[1]);
        pos_mom.SetZ(pos_trk->getMomentum()[2]);


        //Beam Electron cut
        if (!vtxSelector->passCutLt("eleMom_lt",ele_mom.Mag(),weight))
            continue;

        //Ele Track Quality - Chi2
        if (!vtxSelector->passCutLt("eleTrkChi2_lt",ele_trk->getChi2(),weight))
            continue;

        //Pos Track Quality - Chi2
        if (!vtxSelector->passCutLt("posTrkChi2_lt",pos_trk->getChi2(),weight))
            continue;

        //Ele min momentum cut
        if (!vtxSelector->passCutGt("eleMom_gt",ele_mom.Mag(),weight))
            continue;

        //Pos min momentum cut
        if (!vtxSelector->passCutGt("posMom_gt",pos_mom.Mag(),weight))
            continue;

        //Vertex Quality
        if (!vtxSelector->passCutLt("chi2unc_lt",vtx->getChi2(),weight))
            continue;

        //Max vtx momentum

        if (!vtxSelector->passCutLt("maxVtxMom_lt",(ele_mom+pos_mom).Mag(),weight))
            continue;

        //Min vtx momentum

        if (!vtxSelector->passCutGt("minVtxMom_gt",(ele_mom+pos_mom).Mag(),weight))
            continue;

        /*
        _vtx_histos->Fill1DVertex(vtx,
                ele,
                pos,
                ele_trk,
                pos_trk,
                weight);

        _vtx_histos->Fill1DHisto("vtx_Psum_h", p_ele.P()+p_pos.P(), weight);
        _vtx_histos->Fill1DHisto("vtx_Esum_h", ele_E + pos_E, weight);
        _vtx_histos->Fill2DHisto("ele_vtxZ_iso_hh", TMath::Min(ele_trk->getIsolation(0), ele_trk->getIsolation(1)), vtx->getZ(), weight);
        _vtx_histos->Fill2DHisto("pos_vtxZ_iso_hh", TMath::Min(pos_trk->getIsolation(0), pos_trk->getIsolation(1)), vtx->getZ(), weight);
        _vtx_histos->Fill2DHistograms(vtx,weight);
        _vtx_histos->Fill2DTrack(ele_trk,weight,"ele_");
        _vtx_histos->Fill2DTrack(pos_trk,weight,"pos_");
         */

        //std::cout << ele_mom.Mag() << "  " << ele_mom.Theta() << "  " << ele_mom.Phi() << " " << pos_mom.Mag()
		//<< "  " << pos_mom.Theta() << "  " << pos_mom.Phi() << " " << std::acos(ele_mom * pos_mom / (ele_mom.Mag() * pos_mom.Mag())) <<std::endl;
		ele_mom_mag_ = ele_mom.Mag();
		ele_mom_theta_ = ele_mom.Theta();
		ele_mom_phi_ = ele_mom.Phi();

		pos_mom_mag_ = pos_mom.Mag();
		pos_mom_theta_ = pos_mom.Theta();
		pos_mom_phi_ = pos_mom.Phi();

		invariant_mass_ = vtx->getInvMass();

		openingAngle_ = acos(ele_mom * pos_mom / (ele_mom.Mag() * pos_mom.Mag()));

        selected_vtxs.push_back(vtx);
        vtxSelector->clearSelector();
    }

    /*
    _vtx_histos->Fill1DHisto("n_vertices_h",selected_vtxs.size());
    if (trks_)
        _vtx_histos->Fill1DHisto("n_tracks_h",trks_->size());
     */

    if(selected_vtxs.size() == 1){
    	treeOutput_->Fill();
    }

    return true;
}

void MCTupleForClassificationProcessor::finalize() {

    //TODO clean this up a little.
    outF_->cd();
    //_vtx_histos->saveHistos(outF_,_vtx_histos->getName());
    //outF_->cd(_vtx_histos->getName().c_str());
    vtxSelector->getCutFlowHisto()->Write();

    treeOutput_->Write();

    outF_->Close();
}

DECLARE_PROCESSOR(MCTupleForClassificationProcessor);
