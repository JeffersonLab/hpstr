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
        debug_   = parameters.getInteger("debug",debug_);
        anaName_ = parameters.getString("anaName",anaName_);
        vtxColl_ = parameters.getString("vtxColl",vtxColl_);
        trkColl_ = parameters.getString("trkColl",trkColl_);
        hitColl_ = parameters.getString("hitColl",hitColl_);
        ecalColl_ = parameters.getString("ecalColl",ecalColl_);
	mcColl_  = parameters.getString("mcColl",mcColl_);
	
        selectionCfg_   = parameters.getString("vtxSelectionjson",selectionCfg_);
        histoCfg_ = parameters.getString("histoCfg",histoCfg_);
        timeOffset_ = parameters.getDouble("CalTimeOffset",timeOffset_);
        beamE_  = parameters.getDouble("beamE",beamE_);
        isData_  = parameters.getInteger("isData",isData_);

        //region definitions
        regionSelections_ = parameters.getVString("regionDefinitions",regionSelections_);


    }
    catch (std::runtime_error& error) 
    {
        std::cout<<error.what()<<std::endl;
    }
}

void VertexAnaProcessor::initialize(TTree* tree) {
    tree_ = tree;
    _ah =  std::make_shared<AnaHelpers>();

    vtxSelector  = std::make_shared<BaseSelector>(anaName_+"_"+"vtxSelection",selectionCfg_);
    vtxSelector->setDebug(debug_);
    vtxSelector->LoadSelection();

    _vtx_histos = std::make_shared<TrackHistos>(anaName_+"_"+"vtxSelection");
    _vtx_histos->loadHistoConfig(histoCfg_);
    _vtx_histos->DefineHistos();


    //For each region initialize plots

    for (unsigned int i_reg = 0; i_reg < regionSelections_.size(); i_reg++) {
        std::string regname = AnaHelpers::getFileName(regionSelections_[i_reg],false);
        std::cout<<"Setting up region:: " << regname <<std::endl;   
        _reg_vtx_selectors[regname] = std::make_shared<BaseSelector>(anaName_+"_"+regname, regionSelections_[i_reg]);
        _reg_vtx_selectors[regname]->setDebug(debug_);
        _reg_vtx_selectors[regname]->LoadSelection();

        _reg_vtx_histos[regname] = std::make_shared<TrackHistos>(anaName_+"_"+regname);
        _reg_vtx_histos[regname]->loadHistoConfig(histoCfg_);
        _reg_vtx_histos[regname]->DefineHistos();

        _reg_tuples[regname] = std::make_shared<FlatTupleMaker>(anaName_+"_"+regname+"_tree");
        _reg_tuples[regname]->addVariable("unc_vtx_mass");
        _reg_tuples[regname]->addVariable("unc_vtx_z");
        if(!isData_) 
        {
            _reg_tuples[regname]->addVariable("true_vtx_z");
            _reg_tuples[regname]->addVariable("true_vtx_mass");
        }

        _regions.push_back(regname);
    }


    //init Reading Tree
    tree_->SetBranchAddress(vtxColl_.c_str(), &vtxs_ , &bvtxs_);
    tree_->SetBranchAddress(hitColl_.c_str(), &hits_   , &bhits_);
    tree_->SetBranchAddress(ecalColl_.c_str(), &ecal_  , &becal_);
    tree_->SetBranchAddress("EventHeader",&evth_ , &bevth_);
    if(!isData_ && !mcColl_.empty()) tree_->SetBranchAddress(mcColl_.c_str() , &mcParts_, &bmcParts_);
    //If track collection name is empty take the tracks from the particles. TODO:: change this
    if (!trkColl_.empty())
        tree_->SetBranchAddress(trkColl_.c_str(),&trks_, &btrks_);
}

bool VertexAnaProcessor::process(IEvent* ievent) { 
    if(debug_) {
      std:: cout << "----------------- Event " << evth_->getEventNumber() << " -----------------" << std::endl;
    }
    HpsEvent* hps_evt = (HpsEvent*) ievent;
    double weight = 1.;


    //Get "true" mass
    double apMass = -0.9;
    double apZ = -0.9;

    if (mcParts_) {
        for(int i = 0; i < mcParts_->size(); i++)
        {
            if(mcParts_->at(i)->getPDG() == 622) 
            {
                apMass = mcParts_->at(i)->getMass();
                apZ = mcParts_->at(i)->getVertexPosition().at(2);
            }
        }
    }
    //Store processed number of events
    std::vector<Vertex*> selected_vtxs;
    bool passVtxPresel = false;

    // Fill some diagnostic histos
    for ( int i_ecal = 0; i_ecal < ecal_->size(); i_ecal++ ) {

      if (vtxs_->size() == 0){
	_vtx_histos->Fill1DHisto("EecalClus_noVtxs_h",ecal_->at(i_ecal)->getEnergy());
      } else {
	_vtx_histos->Fill1DHisto("EecalClus_isVtxs_h",ecal_->at(i_ecal)->getEnergy());
      }
    }

    
    if (vtxs_->size() == 0){
      _vtx_histos->Fill1DHisto("n_ecalClus_noVtxs_h",ecal_->size());
      _vtx_histos->Fill1DHisto("n_tracks_noVtxs_h",trks_->size());
      for (int i_trk = 0; i_trk < trks_->size(); i_trk++ ){
	_vtx_histos->Fill1DHisto("Ptracks_noVtxs_h",trks_->at(i_trk)->getP());
      }
      
    } else {
      _vtx_histos->Fill1DHisto("n_ecalClus_isVtxs_h",ecal_->size());
      _vtx_histos->Fill1DHisto("n_tracks_isVtxs_h",trks_->size());
      for (int i_trk = 0; i_trk < trks_->size(); i_trk++ ){
	_vtx_histos->Fill1DHisto("Ptracks_isVtxs_h",trks_->at(i_trk)->getP());
      }
    }

    // Loop over vertices in event and make selections
    for ( int i_vtx = 0; i_vtx <  vtxs_->size(); i_vtx++ ) {
          vtxSelector->getCutFlowHisto()->Fill(0.,weight);

        Vertex* vtx = vtxs_->at(i_vtx);
        Particle* ele = nullptr;
        Track* ele_trk = nullptr;
        Particle* pos = nullptr;
        Track* pos_trk = nullptr;

        //Trigger requirement - *really hate* having to do it here for each vertex.

        if (isData_) {
            if (!vtxSelector->passCutEq("Pair1_eq",(int)evth_->isPair1Trigger(),weight))
                break;
        }

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

        CalCluster eleClus = ele->getCluster();
        CalCluster posClus = pos->getCluster();


        //Compute analysis variables here.
        TLorentzVector p_ele;
        p_ele.SetPxPyPzE(ele_trk->getMomentum()[0],ele_trk->getMomentum()[1],ele_trk->getMomentum()[2],ele->getEnergy());
        TLorentzVector p_pos;
        p_pos.SetPxPyPzE(pos_trk->getMomentum()[0],pos_trk->getMomentum()[1],pos_trk->getMomentum()[2],ele->getEnergy());

        //Tracks in opposite volumes - useless
        //if (!vtxSelector->passCutLt("eleposTanLambaProd_lt",ele_trk->getTanLambda() * pos_trk->getTanLambda(),weight)) 
        //  continue;

        //Ele Track-cluster match
        if (!vtxSelector->passCutLt("eleTrkCluMatch_lt",ele->getGoodnessOfPID(),weight))
            continue;

        //Pos Track-cluster match
        if (!vtxSelector->passCutLt("posTrkCluMatch_lt",pos->getGoodnessOfPID(),weight))
            continue;

        //Require Positron Cluster exists
        if (!vtxSelector->passCutGt("posClusE_gt",posClus.getEnergy(),weight))
            continue;

        //Require Positron Cluster does NOT exists
        if (!vtxSelector->passCutLt("posClusE_lt",posClus.getEnergy(),weight))
            continue;


        double corr_eleClusterTime = ele->getCluster().getTime() - timeOffset_;
        double corr_posClusterTime = pos->getCluster().getTime() - timeOffset_;

        double botClusTime = 0.0;
        if(ele->getCluster().getPosition().at(1) < 0.0) botClusTime = ele->getCluster().getTime();
        else botClusTime = pos->getCluster().getTime();

        //Bottom Cluster Time
        if (!vtxSelector->passCutLt("botCluTime_lt", botClusTime, weight))
            continue;

        if (!vtxSelector->passCutGt("botCluTime_gt", botClusTime, weight))
            continue;

        //Ele Pos Cluster Time Difference
        if (!vtxSelector->passCutLt("eleposCluTimeDiff_lt",fabs(corr_eleClusterTime - corr_posClusterTime),weight))
            continue;

        //Ele Track-Cluster Time Difference
        if (!vtxSelector->passCutLt("eleTrkCluTimeDiff_lt",fabs(ele_trk->getTrackTime() - corr_eleClusterTime),weight))
            continue;

        //Pos Track-Cluster Time Difference
        if (!vtxSelector->passCutLt("posTrkCluTimeDiff_lt",fabs(pos_trk->getTrackTime() - corr_posClusterTime),weight))
            continue;

        TVector3 ele_mom;
        //ele_mom.SetX(ele->getMomentum()[0]);
        //ele_mom.SetY(ele->getMomentum()[1]);
        //ele_mom.SetZ(ele->getMomentum()[2]);
        ele_mom.SetX(ele_trk->getMomentum()[0]);
        ele_mom.SetY(ele_trk->getMomentum()[1]);
        ele_mom.SetZ(ele_trk->getMomentum()[2]);


        TVector3 pos_mom;
        //pos_mom.SetX(pos->getMomentum()[0]);
        //pos_mom.SetY(pos->getMomentum()[1]);
        //pos_mom.SetZ(pos->getMomentum()[2]);
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

        //Ele Track Quality - Chi2Ndf
        if (!vtxSelector->passCutLt("eleTrkChi2Ndf_lt",ele_trk->getChi2Ndf(),weight))
            continue;

        //Pos Track Quality - Chi2Ndf
        if (!vtxSelector->passCutLt("posTrkChi2Ndf_lt",pos_trk->getChi2Ndf(),weight))
            continue;

        //Ele min momentum cut
        if (!vtxSelector->passCutGt("eleMom_gt",ele_mom.Mag(),weight))
            continue;

        //Pos min momentum cut
        if (!vtxSelector->passCutGt("posMom_gt",pos_mom.Mag(),weight))
            continue;

        //Ele nHits
        int ele2dHits = ele_trk->getTrackerHitCount();
        if (!ele_trk->isKalmanTrack()) 
            ele2dHits*=2;

        if (!vtxSelector->passCutGt("eleN2Dhits_gt",ele2dHits,weight))  {
            continue;
        }

        //Pos nHits
        int pos2dHits = pos_trk->getTrackerHitCount();
        if (!pos_trk->isKalmanTrack()) 
            pos2dHits*=2;

        if (!vtxSelector->passCutGt("posN2Dhits_gt",pos2dHits,weight))  {
            continue;
        }

        //Less than 4 shared hits for ele/pos track
        if (!vtxSelector->passCutLt("eleNshared_lt",ele_trk->getNShared(),weight)) {
            continue;
        }

        if (!vtxSelector->passCutLt("posNshared_lt",pos_trk->getNShared(),weight)) {
            continue;
        }


        //Vertex Quality
        if (!vtxSelector->passCutLt("chi2unc_lt",vtx->getChi2(),weight))
            continue;

        //Max vtx momentum
        if (!vtxSelector->passCutLt("maxVtxMom_lt",(ele_mom+pos_mom).Mag(),weight))
            continue;

        //Min vtx momentum

        if (!vtxSelector->passCutGt("minVtxMom_gt",(ele_mom+pos_mom).Mag(),weight))
            continue;

        _vtx_histos->Fill1DVertex(vtx,
                ele,
                pos,
                ele_trk,
                pos_trk,
                weight);

        _vtx_histos->Fill1DHisto("vtx_Psum_h", p_ele.P()+p_pos.P(), weight);
        _vtx_histos->Fill1DHisto("vtx_Esum_h", ele_E + pos_E, weight);
	_vtx_histos->Fill1DHisto("ele_pos_clusTimeDiff_h", fabs(corr_eleClusterTime - corr_posClusterTime), weight);
        _vtx_histos->Fill2DHisto("ele_vtxZ_iso_hh", TMath::Min(ele_trk->getIsolation(0), ele_trk->getIsolation(1)), vtx->getZ(), weight);
        _vtx_histos->Fill2DHisto("pos_vtxZ_iso_hh", TMath::Min(pos_trk->getIsolation(0), pos_trk->getIsolation(1)), vtx->getZ(), weight);
        _vtx_histos->Fill2DHistograms(vtx,weight);
        _vtx_histos->Fill2DTrack(ele_trk,weight,"ele_");
        _vtx_histos->Fill2DTrack(pos_trk,weight,"pos_");
        _vtx_histos->Fill1DHisto("mcMass622_h",apMass); 
        _vtx_histos->Fill1DHisto("mcZ622_h",apZ); 
	
	passVtxPresel = true;

        selected_vtxs.push_back(vtx);       
        vtxSelector->clearSelector();
    }
    
    // std::cout << "Number of selected vtxs: " << selected_vtxs.size() << std::endl;

    _vtx_histos->Fill1DHisto("n_vertices_h",selected_vtxs.size()); 
    if (trks_)
      _vtx_histos->Fill1DHisto("n_tracks_h",trks_->size()); 


    //not working atm
    //hps_evt->addVertexCollection("selected_vtxs", selected_vtxs);

    //Make Plots for each region: loop on each region. Check if the region has the cut and apply it
    //TODO Clean this up => Cuts should be implemented in each region? 
    //TODO Bring the preselection out of this stupid loop


    //TODO add yields. => Quite terrible way to loop. 
    for (auto region : _regions ) {

        int nGoodVtx = 0;
        Vertex* goodVtx = nullptr;

        for ( auto vtx : selected_vtxs) {

            //No cuts.
            _reg_vtx_selectors[region]->getCutFlowHisto()->Fill(0.,weight);


            Particle* ele = nullptr;
            Particle* pos = nullptr;

            _ah->GetParticlesFromVtx(vtx,ele,pos);

            //vtx Z position
	    if (!_reg_vtx_selectors[region]->passCutGt("uncVtxZ_gt",vtx->getZ(),weight))
	        continue;
	    
            //Chi2
            if (!_reg_vtx_selectors[region]->passCutLt("chi2unc_lt",vtx->getChi2(),weight))
                continue;

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
                    if (debug_)
                        std::cout<<"VertexAnaProcessor::ERROR couldn't find ele/pos in the "<<trkColl_ <<"collection"<<std::endl;
                    continue;  
                }
            }
            else {

                ele_trk_gbl = (Track*) ele_trk.Clone();
                pos_trk_gbl = (Track*) pos_trk.Clone();
            }

            //Add the momenta to the tracks
            //ele_trk_gbl->setMomentum(ele->getMomentum()[0],ele->getMomentum()[1],ele->getMomentum()[2]);
            //pos_trk_gbl->setMomentum(pos->getMomentum()[0],pos->getMomentum()[1],pos->getMomentum()[2]);
            TVector3 recEleP(ele->getMomentum()[0],ele->getMomentum()[1],ele->getMomentum()[2]);
            TLorentzVector p_ele;
            p_ele.SetPxPyPzE(ele_trk_gbl->getMomentum()[0],ele_trk_gbl->getMomentum()[1],ele_trk_gbl->getMomentum()[2], ele_E);
            TLorentzVector p_pos;
            p_pos.SetPxPyPzE(pos_trk_gbl->getMomentum()[0],pos_trk_gbl->getMomentum()[1],pos_trk_gbl->getMomentum()[2], pos_E);


            if (debug_) {
                std::cout<<"Check on ele_Track"<<std::endl;
                std::cout<<"Number of hits:"<<ele_trk_gbl->getTrackerHitCount()<<std::endl;
            }

            bool foundL1ele = false;
            bool foundL2ele = false;
            _ah->InnermostLayerCheck(ele_trk_gbl, foundL1ele, foundL2ele);   


            if (debug_) {
                std::cout<<"Check on pos_Track"<<std::endl;
                std::cout<<"Number of hits:"<<ele_trk_gbl->getTrackerHitCount()<<std::endl;
            }
            bool foundL1pos = false;
            bool foundL2pos = false;

            _ah->InnermostLayerCheck(pos_trk_gbl, foundL1pos, foundL2pos);  

            if (debug_) {
                std::cout<<"Check on pos_Track"<<std::endl;
                std::cout<<"Innermost:"<<foundL1pos<<" Second Innermost:"<<foundL2pos<<std::endl;
            }

            //L1 requirement
            if (!_reg_vtx_selectors[region]->passCutEq("L1Requirement_eq",(int)(foundL1ele&&foundL1pos),weight))
                continue;

            //L2 requirement
            if (!_reg_vtx_selectors[region]->passCutEq("L2Requirement_eq",(int)(foundL2ele&&foundL2pos),weight))
                continue;

            //L1 requirement for positron
            if (!_reg_vtx_selectors[region]->passCutEq("L1PosReq_eq",(int)(foundL1pos),weight))
                continue;
            if (debug_)
                std::cout<<"Track passed"<<std::endl;

            //ESum low cut 
            if (!_reg_vtx_selectors[region]->passCutLt("eSum_lt",(ele_E+pos_E),weight))
                continue;

            //ESum high cut
            if (!_reg_vtx_selectors[region]->passCutGt("eSum_gt",(ele_E+pos_E),weight))
                continue;

            //PSum low cut 
            if (!_reg_vtx_selectors[region]->passCutLt("pSum_lt",(p_ele.P()+p_pos.P()),weight))
                continue;

            //PSum high cut
            if (!_reg_vtx_selectors[region]->passCutGt("pSum_gt",(p_ele.P()+p_pos.P()),weight))
                continue;

            //Require Electron Cluster exists
            if (!_reg_vtx_selectors[region]->passCutGt("eleClusE_gt",eleClus.getEnergy(),weight))
                continue;

            //Require Electron Cluster does NOT exists
            if (!_reg_vtx_selectors[region]->passCutLt("eleClusE_lt",eleClus.getEnergy(),weight))
                continue;
             
	    //Max P_ele
	    if (!_reg_vtx_selectors[region]->passCutLt("eleMom_lt",p_ele.P(),weight))
	        continue;

	    //Max P_pos
	    if (!_reg_vtx_selectors[region]->passCutLt("posMom_lt",p_pos.P(),weight))
	        continue;

	    //Max vtx momentum
	    
	    if (!_reg_vtx_selectors[region]->passCutLt("maxVtxMom_lt",(p_ele+p_pos).P(),weight))
	        continue;


            //No shared hits requirement
            if (!_reg_vtx_selectors[region]->passCutEq("ele_sharedL0_eq",(int)ele_trk_gbl->getSharedLy0(),weight))
                continue;
            if (!_reg_vtx_selectors[region]->passCutEq("pos_sharedL0_eq",(int)pos_trk_gbl->getSharedLy0(),weight))
                continue;
            if (!_reg_vtx_selectors[region]->passCutEq("ele_sharedL1_eq",(int)ele_trk_gbl->getSharedLy1(),weight))
                continue;
            if (!_reg_vtx_selectors[region]->passCutEq("pos_sharedL1_eq",(int)pos_trk_gbl->getSharedLy1(),weight))
                continue;

	    //Min vtx Y pos
	    if (!_reg_vtx_selectors[region]->passCutGt("VtxYPos_gt", vtx->getY(), weight))
	        continue;
	
	    //Max vtx Y pos
	    if (!_reg_vtx_selectors[region]->passCutLt("VtxYPos_lt", vtx->getY(), weight))
	        continue;

            //If this is MC check if MCParticle matched to the electron track is from rad or recoil
            if(!isData_)
            {
                //Build map of hits and the associated MC part ids for later
                TRefArray* ele_trk_hits = ele_trk_gbl->getSvtHits();
                std::map<int, std::vector<int> > trueHitIDs;
                for(int i = 0; i < hits_->size(); i++)
                {
                    TrackerHit* hit = hits_->at(i);
                    trueHitIDs[hit->getID()] = hit->getMCPartIDs();
                }
                //std::cout << "There are " << ele_trk_hits->GetEntries() << " hits on this track" << std::endl;
                //Count the number of hits per part on the track
                std::map<int, int> nHits4part;
                for(int i = 0; i < ele_trk_hits->GetEntries(); i++)
                {
                    TrackerHit* eleHit = (TrackerHit*)ele_trk_hits->At(i);
                    for(int idI = 0; idI < trueHitIDs[eleHit->getID()].size(); idI++ )
                    {
                        int partID = trueHitIDs[eleHit->getID()].at(idI);
                        if ( nHits4part.find(partID) == nHits4part.end() ) 
                        {
                            // not found
                            nHits4part[partID] = 1;
                        } 
                        else 
                        {
                            // found
                            nHits4part[partID]++;
                        }
                    }
                }

                //Determine the MC part with the most hits on the track
                int maxNHits = 0;
                int maxID = 0;
                for (std::map<int,int>::iterator it=nHits4part.begin(); it!=nHits4part.end(); ++it)
                {
                    if(it->second > maxNHits)
                    {
                        maxNHits = it->second;
                        maxID = it->first;
                    }
                }

                //Find the correct mc part and grab mother id
                int isRadEle = -999;
                int isRecEle = -999;
                TVector3 trueEleP;

                trueEleP.SetXYZ(-999,-999,-999);
                if (mcParts_) {
                    for(int i = 0; i < mcParts_->size(); i++)
                    {
                        int momPDG = mcParts_->at(i)->getMomPDG();
                        if(mcParts_->at(i)->getPDG() == 11 && momPDG == 622) 
                        {
                            std::vector<double> lP = mcParts_->at(i)->getMomentum();
                            trueEleP.SetXYZ(lP[0],lP[1],lP[2]);
                        }
                        if(mcParts_->at(i)->getID() != maxID) continue;
                        if(momPDG == 625) isRadEle = 1;
                        if(momPDG == 623) isRecEle = 1;
                    }
                }
                double momRatio = recEleP.Mag() / trueEleP.Mag();
                double momAngle = trueEleP.Angle(recEleP) * TMath::RadToDeg();
                if (!_reg_vtx_selectors[region]->passCutLt("momRatio_lt", momRatio, weight)) continue;
                if (!_reg_vtx_selectors[region]->passCutGt("momRatio_gt", momRatio, weight)) continue;
                if (!_reg_vtx_selectors[region]->passCutLt("momAngle_lt", momAngle, weight)) continue;

                if (!_reg_vtx_selectors[region]->passCutEq("isRadEle_eq", isRadEle, weight)) continue;
                if (!_reg_vtx_selectors[region]->passCutEq("isRecEle_eq", isRecEle, weight)) continue;
            }

            goodVtx = vtx;
            nGoodVtx++;
        } // preselected vertices


        //N selected vertices - this is quite a silly cut to make at the end. But okay. that's how we decided atm.
        if (!_reg_vtx_selectors[region]->passCutEq("nVtxs_eq", nGoodVtx, weight))
            continue;
	//Move to after N vertices cut (was filled before)
        _reg_vtx_histos[region]->Fill1DHisto("n_vertices_h", nGoodVtx, weight);
            
        Vertex* vtx = goodVtx;

        Particle* ele = nullptr;
        Particle* pos = nullptr;

        if (!vtx || !_ah->GetParticlesFromVtx(vtx,ele,pos))
            continue;
        
        CalCluster eleClus = ele->getCluster();
        CalCluster posClus = pos->getCluster();

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
                if (debug_)
                    std::cout<<"VertexAnaProcessor::ERROR couldn't find ele/pos in the "<<trkColl_ <<"collection"<<std::endl;
                continue;  
            }
        }
        else {

            ele_trk_gbl = (Track*) ele_trk.Clone();
            pos_trk_gbl = (Track*) pos_trk.Clone();
        }

        //Add the momenta to the tracks
        //ele_trk_gbl->setMomentum(ele->getMomentum()[0],ele->getMomentum()[1],ele->getMomentum()[2]);
        //pos_trk_gbl->setMomentum(pos->getMomentum()[0],pos->getMomentum()[1],pos->getMomentum()[2]);
        TVector3 recEleP(ele->getMomentum()[0],ele->getMomentum()[1],ele->getMomentum()[2]);
        TLorentzVector p_ele;
        p_ele.SetPxPyPzE(ele_trk_gbl->getMomentum()[0],ele_trk_gbl->getMomentum()[1],ele_trk_gbl->getMomentum()[2], ele_E);
        TLorentzVector p_pos;
        p_pos.SetPxPyPzE(pos_trk_gbl->getMomentum()[0],pos_trk_gbl->getMomentum()[1],pos_trk_gbl->getMomentum()[2], pos_E);

        _reg_vtx_histos[region]->Fill2DHistograms(vtx,weight);
        _reg_vtx_histos[region]->Fill1DVertex(vtx,
                ele,
                pos,
                ele_trk_gbl,
                pos_trk_gbl,
                weight);

        _reg_vtx_histos[region]->Fill1DHisto("vtx_Psum_h", p_ele.P()+p_pos.P(), weight);
        _reg_vtx_histos[region]->Fill1DHisto("vtx_Esum_h", eleClus.getEnergy()+posClus.getEnergy(), weight);
        _reg_vtx_histos[region]->Fill2DHisto("ele_vtxZ_iso_hh", TMath::Min(ele_trk_gbl->getIsolation(0), ele_trk_gbl->getIsolation(1)), vtx->getZ(), weight);
        _reg_vtx_histos[region]->Fill2DHisto("pos_vtxZ_iso_hh", TMath::Min(pos_trk_gbl->getIsolation(0), pos_trk_gbl->getIsolation(1)), vtx->getZ(), weight);
        _reg_vtx_histos[region]->Fill2DTrack(ele_trk_gbl,weight,"ele_");
        _reg_vtx_histos[region]->Fill2DTrack(pos_trk_gbl,weight,"pos_");
        _reg_vtx_histos[region]->Fill1DHisto("mcMass622_h",apMass);
        _reg_vtx_histos[region]->Fill1DHisto("mcZ622_h",apZ);

        if (trks_) _reg_vtx_histos[region]->Fill1DHisto("n_tracks_h",trks_->size(),weight);

        //Just for the selected vertex
        _reg_tuples[region]->setVariableValue("unc_vtx_mass", vtx->getInvMass());
        if(!isData_) 
        {
            _reg_tuples[region]->setVariableValue("true_vtx_z", apZ);
            _reg_tuples[region]->setVariableValue("true_vtx_mass", apMass);
        }

        //TODO put this in the Vertex!
        TVector3 vtxPosSvt;
        vtxPosSvt.SetX(vtx->getX());
        vtxPosSvt.SetY(vtx->getY());
        vtxPosSvt.SetZ(vtx->getZ());
        vtxPosSvt.RotateY(-0.0305);

        _reg_tuples[region]->setVariableValue("unc_vtx_z"   , vtxPosSvt.Z());
        _reg_tuples[region]->fill();
    }// regions



    return true;
}

void VertexAnaProcessor::finalize() {

    //TODO clean this up a little.
    outF_->cd();
    _vtx_histos->saveHistos(outF_,_vtx_histos->getName());
    outF_->cd(_vtx_histos->getName().c_str());
    vtxSelector->getCutFlowHisto()->Write();


    for (reg_it it = _reg_vtx_histos.begin(); it!=_reg_vtx_histos.end(); ++it) {
        std::string dirName = anaName_+"_"+it->first;
        (it->second)->saveHistos(outF_,dirName);
        outF_->cd(dirName.c_str());
        _reg_vtx_selectors[it->first]->getCutFlowHisto()->Write();
        //Save tuples
        _reg_tuples[it->first]->writeTree();
    }

    outF_->Close();

}

DECLARE_PROCESSOR(VertexAnaProcessor);
