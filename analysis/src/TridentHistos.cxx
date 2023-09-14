#include "TridentHistos.h"
#include "TLorentzVector.h"
#include "TVector3.h"
#include <iostream>

void TridentHistos::BuildAxes(){}

void TridentHistos::Define2DHistos() {

    //TODO improve naming
    std::string h_name = "";
  
    //TODO improve binning
    if (doTrkCompPlots) {
        /*
        for (unsigned int itp = 0; itp<tPs.size(); ++itp){
      
            if (debug_)
                std::cout<<"Bulding:: TH2::" + m_name+"_"+tPs[itp]+"_vs_"+tPs[itp] << std::endl;
            
            histos2d[m_name+"_"+tPs[itp]+"_vs_"+tPs[itp] ] = plot2D(m_name+tPs[itp]+"_vs_"+tPs[itp],
                                                                    tPs[itp],axes[tPs[itp]][0],axes[tPs[itp]][1],axes[tPs[itp]][2],
                                                                    tPs[itp],axes[tPs[itp]][0],axes[tPs[itp]][1],axes[tPs[itp]][2]);
      
        }//loop on vars
        */
    }//do track comparison
    /*
      histos2d[m_name+"_vtxY_vs_vtxX"] = plot2D(m_name+"_vtxY_vs_vtxX",
      "vtxX",axes["vtx_X"][0],axes["vtx_X"][1],axes["vtx_X"][2],
      "vtxY",axes["vtx_Y"][0],axes["vtx_Y"][1],axes["vtx_Y"][2]);
    */
  
  
}//define 2dhistos


void TridentHistos::Fill1DVertex(Vertex* vtx, 
                                 Particle* ele, 
                                 Particle* pos, 
                                 Track* ele_trk,
                                 Track* pos_trk,
                                 double trkTimeOffset,
                                 float weight) {
    
    Fill1DVertex(vtx,weight);

    if (ele_trk)
      Fill1DTrack(ele_trk,trkTimeOffset,weight,"ele_");
    if (pos_trk)
      Fill1DTrack(pos_trk,trkTimeOffset,weight,"pos_");
    
    
    TLorentzVector p_ele;
    //p_ele.SetPxPyPzE(ele->getMomentum()[0], ele->getMomentum()[1],ele->getMomentum()[2],ele->getEnergy());
    p_ele.SetPxPyPzE(ele_trk->getMomentum()[0],ele_trk->getMomentum()[1],ele_trk->getMomentum()[2],ele->getEnergy());
    
    TLorentzVector p_pos;
    //p_pos.SetPxPyPzE(pos->getMomentum()[0], pos->getMomentum()[1],pos->getMomentum()[2],pos->getEnergy());
    p_pos.SetPxPyPzE(pos_trk->getMomentum()[0],pos_trk->getMomentum()[1],pos_trk->getMomentum()[2],pos->getEnergy());
    
    //Fill ele and pos information
    //  Fill1DHisto("ele_p_h",p_ele.P(),weight);
    // Fill1DHisto("pos_p_h",p_pos.P(),weight);

    //Compute some extra variables 
    
    //TODO::Rotate them
    p_ele.RotateY(-0.0305);
    p_pos.RotateY(-0.0305);

    //Massless electrons. TODO fix initialization
    TLorentzVector p_beam(0.,0.,2.3,2.3);
    TLorentzVector p_v0   = p_ele+p_pos;
    TLorentzVector p_miss =  p_beam - p_v0;
        
    double thetax_v0_val   = TMath::ATan2(p_v0.X(),p_v0.Z());
    double thetax_pos_val  = TMath::ATan2(p_pos.X(),p_pos.Z());
    
    double thetay_miss_val = TMath::ATan2(p_miss.Y(),p_miss.Z());
    double thetay_pos_val  = TMath::ATan2(p_pos.Y(),p_pos.Z());

    double pt_ele = p_ele.Pt();
    double pt_pos = p_pos.Pt();

    double pt_asym_val = (pt_ele-pt_pos) / (pt_ele+pt_pos);
    
    double thetay_diff_val;
    
    if (thetay_pos_val>0) {
        thetay_diff_val = thetay_miss_val - thetay_pos_val;
    }
    else {
        thetay_diff_val = thetay_pos_val - thetay_miss_val;
    }

    //Fill event information

    //Esum
    Fill1DHisto("Esum_h",ele->getEnergy() + pos->getEnergy(),weight);
    Fill1DHisto("Psum_h",p_ele.P() + p_pos.P());
    Fill1DHisto("PtAsym_h",pt_asym_val,weight);
    Fill1DHisto("thetax_v0_h",thetax_v0_val,weight);
    Fill1DHisto("thetax_pos_h",thetax_pos_val,weight);
    Fill1DHisto("thetay_pos_h",thetay_pos_val,weight);
    Fill1DHisto("thetay_miss_h",thetay_miss_val,weight);
    Fill1DHisto("thetay_diff_h",thetay_diff_val,weight);
    //
    //std::cout<<"filling z/tanlambda 2d plots"<<std::endl;
    //std::cout<<ele_trk->getZ0()/ele_trk->getTanLambda()<<std::endl;
    Fill2DHisto("ele_vtx_z_vs_z0_over_tanLambda_hh",ele_trk->getZ0()/ele_trk->getTanLambda(), vtx->getZ());
    Fill2DHisto("pos_vtx_z_vs_z0_over_tanLambda_hh",pos_trk->getZ0()/pos_trk->getTanLambda(), vtx->getZ());
    if(fabs(vtx->getInvMass()-0.0925)<0.010){
      Fill2DHisto("ele_vtx_z_vs_z0_over_tanLambda_m_eq_92_hh",ele_trk->getZ0()/ele_trk->getTanLambda(), vtx->getZ());
      Fill2DHisto("pos_vtx_z_vs_z0_over_tanLambda_m_eq_92_hh",pos_trk->getZ0()/pos_trk->getTanLambda(), vtx->getZ());
    }

}


void TridentHistos::Fill2DTrack(Track* track, float weight, const std::string& trkname) {
    
    
    if (track) {
        
        double d0 = track->getD0();
        double z0 = track->getZ0();
        Fill2DHisto(trkname+"tanlambda_vs_phi0_hh",track->getPhi(),track->getTanLambda(), weight);
        Fill2DHisto(trkname+"d0_vs_p_hh",track->getP(),d0,weight);
        Fill2DHisto(trkname+"d0_vs_phi0_hh",track->getPhi(),d0,weight);
        Fill2DHisto(trkname+"d0_vs_tanlambda_hh",track->getTanLambda(),d0,weight);
        
        Fill2DHisto(trkname+"z0_vs_p_hh",track->getP(),z0,weight);
        Fill2DHisto(trkname+"z0_vs_phi0_hh",track->getPhi(),z0,weight);
        Fill2DHisto(trkname+"z0_vs_tanlambda_hh",track->getTanLambda(),z0,weight);
        
    }
}

void TridentHistos::Fill1DTrack(Track* track, double trkTimeOffset,float weight, const std::string& trkname) {
    
    double charge = (double) track->getCharge();

      //2D hits
    int n_hits_2d = track->getTrackerHitCount();
    //    std::cout<<"Fill1DTrack::Number of hits on track = "<<n_hits_2d<<std::endl;
    //    if (!track->isKalmanTrack())
    //    n_hits_2d*=2;

    TVector3 p_trk;
    p_trk.SetXYZ(track->getMomentum()[0],track->getMomentum()[1],track->getMomentum()[2]);
    
    
    Fill1DHisto(trkname+"p_h",p_trk.Mag(),weight);
    Fill1DHisto(trkname+"d0_h"       ,track->getD0()          ,weight);
    Fill1DHisto(trkname+"Phi_h"      ,track->getPhi()         ,weight);
    Fill1DHisto(trkname+"Omega_h"    ,track->getOmega()       ,weight);
    Fill1DHisto(trkname+"pT_h"       ,-1*charge*track->getPt()   ,weight);
    Fill1DHisto(trkname+"invpT_h"    ,-1*charge/track->getPt()   ,weight);
    Fill1DHisto(trkname+"TanLambda_h",track->getTanLambda()   ,weight);
    Fill1DHisto(trkname+"Z0_h"       ,track->getZ0()          ,weight);
    Fill1DHisto(trkname+"Z0_over_TanLambda_h",track->getZ0()/track->getTanLambda()   ,weight);
    Fill1DHisto(trkname+"time_h"     ,track->getTrackTime()-trkTimeOffset   ,weight);
    Fill1DHisto(trkname+"chi2_h"     ,track->getChi2()        ,weight);
    Fill1DHisto(trkname+"chi2ndf_h"  ,track->getChi2Ndf()     ,weight);
    Fill1DHisto(trkname+"nShared_h"  ,track->getNShared()     ,weight);
    Fill1DHisto(trkname+"nHits_2d_h" ,n_hits_2d               ,weight);
    

    bool hasLayer4=false;
    for (int ihit=0; ihit<track->getTrackerHitCount();++ihit) {
      TrackerHit* hit = (TrackerHit*) track->getSvtHits().At(ihit);
      RawSvtHit* rhit=(RawSvtHit*)(hit->getRawHits()).At(0);
      int layer=rhit->getLayer();
      if(layer==4){
        if(hasLayer4)
          std::cout<<"What...I already counted layer 4!"<<std::endl;
        else
          hasLayer4=true;
      }
      Fill1DHisto(trkname+"layersHit_h",layer,weight);
    }


    //All Tracks
    Fill1DHisto(trkname+"sharingHits_h",0,weight);
    if (track->getNShared() == 0)
        Fill1DHisto(trkname+"sharingHits_h",1.,weight);
    else {
        //track has shared hits
        if (track->getSharedLy0())
            Fill1DHisto(trkname+"sharingHits_h",2.,weight);
        if (track->getSharedLy1())
            Fill1DHisto(trkname+"sharingHits_h",3.,weight);
        if (track->getSharedLy0() && track->getSharedLy1())
            Fill1DHisto(trkname+"sharingHits_h",4.,weight);
        if (!track->getSharedLy0() && !track->getSharedLy1())
            Fill1DHisto(trkname+"sharingHits_h",5.,weight);
    }
    /*                
    if (track -> is345Seed())
        Fill1DHisto(trkname+"strategy_h",0,weight);
    if (track-> is456Seed())
        Fill1DHisto(trkname+"strategy_h",1,weight);
    if (track-> is123SeedC4())
        Fill1DHisto(trkname+"strategy_h",2,weight);
    if (track->is123SeedC5())
        Fill1DHisto(trkname+"strategy_h",3,weight);
    if (track->isMatchedTrack())
        Fill1DHisto(trkname+"strategy_h",4,weight);
    if (track->isGBLTrack())
        Fill1DHisto(trkname+"strategy_h",5,weight);
    */  
        
    Fill1DHisto(trkname+"type_h",track->getType(),weight);
}

void TridentHistos::Fill1DVertex(Vertex* vtx, float weight) {
    
    Fill1DHisto("vtx_chi2_h"   ,vtx->getChi2(),weight);
    Fill1DHisto("vtx_X_h"      ,vtx->getX(),weight);
    Fill1DHisto("vtx_Y_h"      ,vtx->getY(),weight);
    Fill1DHisto("vtx_Z_h"      ,vtx->getZ(),weight);
    
    TVector3 vtxPosSvt;
    vtxPosSvt.SetX(vtx->getX());
    vtxPosSvt.SetY(vtx->getY());
    vtxPosSvt.SetZ(vtx->getZ());
    
    vtxPosSvt.RotateY(-0.0305);

    Fill1DHisto("vtx_X_svt_h",vtxPosSvt.X(),weight);
    Fill1DHisto("vtx_Y_svt_h",vtxPosSvt.Y(),weight);
    Fill1DHisto("vtx_Z_svt_h",vtxPosSvt.Z(),weight);
    
    // 0 xx 1 xy 2 xz 3 yy 4 yz 5 zz
    Fill1DHisto("vtx_sigma_X_h",sqrt(vtx->getCovariance()[0]),weight);
    Fill1DHisto("vtx_sigma_Y_h",sqrt(vtx->getCovariance()[3]),weight);
    Fill1DHisto("vtx_sigma_Z_h",sqrt(vtx->getCovariance()[5]),weight);
    Fill1DHisto("vtx_InvM_h"   ,vtx->getInvMass(),weight);
    Fill1DHisto("vtx_InvMErr_Z_h",vtx->getInvMassErr(),weight);
    Fill1DHisto("vtx_px_h",vtx->getP().X());
    Fill1DHisto("vtx_py_h",vtx->getP().Y());
    Fill1DHisto("vtx_pz_h",vtx->getP().Z());
    Fill1DHisto("vtx_p_h" ,vtx->getP().Mag());
}

void TridentHistos::Fill1DHistograms(Track *track, Vertex* vtx, float weight ) {
    
    if (track) {
      Fill1DTrack(track,-666);
    }
  
    //Vertices
    
    if (vtx) {
        Fill1DVertex(vtx);
    }
}


void TridentHistos::Fill1DTrackTruth(Track *track, Track* truth_track, float weight, const std::string& trkname) {
    
    if (!track || !truth_track)
        return;
    
    //Momentum 
    std::vector<double> trk_mom = track->getMomentum();
    std::vector<double> trk_truth_mom = truth_track->getMomentum();
    
    double d0 = track->getD0();
    double d0err = track->getD0Err();
    double d0_truth = truth_track->getD0();
    double phi = track->getPhi();
    double phi_truth = truth_track->getPhi();
    double phierr = track->getPhiErr();
    double omega = track->getOmega();
    double omega_truth = truth_track->getOmega();
    double omegaerr = track->getOmegaErr();
    double tanLambda = track->getTanLambda();
    double tanLambda_truth = truth_track->getTanLambda();
    double tanLambdaerr = track->getTanLambdaErr();
    double z0 = track->getZ0();
    double z0_truth = truth_track->getZ0();
    double z0err = track->getZ0Err();
    double p = track->getP();
    //Charge different wrt Robert's plots.
    double invPt = -1.*(double) track->getCharge()/track->getPt();  
    double p_truth = truth_track->getP();
    double invPt_truth = -1*(double) track->getCharge()/truth_track->getPt();
        
    double diff_percent_invpT = ((invPt - invPt_truth) / invPt_truth) * 100.;
    
    // truth residuals
    Fill1DHisto(trkname+"d0_truth_res_h",       d0 - d0_truth                  , weight);
    Fill1DHisto(trkname+"Phi_truth_res_h",      phi - phi_truth                , weight);
    Fill1DHisto(trkname+"Omega_truth_res_h",    omega - omega_truth            , weight);
    Fill1DHisto(trkname+"TanLambda_truth_res_h",tanLambda - tanLambda_truth    , weight);
    Fill1DHisto(trkname+"Z0_truth_res_h",       z0 - z0_truth                  , weight);
    Fill1DHisto(trkname+"p_truth_res_h",        p  - p_truth                   , weight);
    Fill1DHisto(trkname+"invpT_truth_res_h",    invPt - invPt_truth            , weight);
    Fill1DHisto(trkname+"invpT_truth_res_percent_h", diff_percent_invpT        , weight);
    Fill1DHisto(trkname+"px_truth_res_h",       trk_mom[0]  - trk_truth_mom[0] , weight);
    Fill1DHisto(trkname+"py_truth_res_h",       trk_mom[1]  - trk_truth_mom[1] , weight);
    Fill1DHisto(trkname+"pz_truth_res_h",       trk_mom[2]  - trk_truth_mom[2] , weight);
    
    // truth pulls
    Fill1DHisto(trkname+"d0_truth_pull_h",       (d0 - d0_truth)               / d0err, weight);
    Fill1DHisto(trkname+"Phi_truth_pull_h",      (phi - phi_truth)             / phierr  , weight);
    Fill1DHisto(trkname+"Omega_truth_pull_h",    (omega - omega_truth)         / omegaerr, weight);
    Fill1DHisto(trkname+"TanLambda_truth_pull_h",(tanLambda - tanLambda_truth) / tanLambdaerr, weight);
    Fill1DHisto(trkname+"Z0_truth_pull_h",       (z0 - z0_truth)               / z0err, weight);
    
}



void TridentHistos::Fill2DHistograms(Vertex* vtx, float weight) {    

    if (vtx) {
                
        //TODO Improve this.
        TVector3 vtxPosSvt;
        vtxPosSvt.SetX(vtx->getX());
        vtxPosSvt.SetY(vtx->getY());
        vtxPosSvt.SetZ(vtx->getZ());
        
        vtxPosSvt.RotateY(-0.0305);
        

        double vtxP = vtx->getP().Mag();
        
        Fill2DHisto("vtx_InvM_vtx_z_hh",vtx->getInvMass(),vtx->getZ(),weight);
        Fill2DHisto("vtx_InvM_vtx_svt_z_hh",vtx->getInvMass(),vtxPosSvt.Z(),weight);
        Fill2DHisto("vtx_p_svt_z_hh",vtxP,vtxPosSvt.Z(),weight);
        Fill2DHisto("vtx_p_svt_x_hh",vtxP,vtxPosSvt.X(),weight);
        Fill2DHisto("vtx_p_svt_y_hh",vtxP,vtxPosSvt.Y(),weight);
        
        Fill2DHisto("vtx_p_sigmaZ_hh",vtxP,vtx->getCovariance()[5],weight);
        Fill2DHisto("vtx_p_sigmaX_hh",vtxP,vtx->getCovariance()[3],weight);
        Fill2DHisto("vtx_p_sigmaY_hh",vtxP,vtx->getCovariance()[0],weight);
    }
}

void TridentHistos::FillTrackComparisonHistograms(Track* track_x, Track* track_y, float weight) {

    if (doTrkCompPlots) {
        /*
        histos2d[m_name+"_d0_vs_d0"              ]->Fill(track_x->getD0(),track_y->getD0(),weight);
        histos2d[m_name+"_Phi_vs_Phi"            ]->Fill(track_x->getPhi(),track_y->getPhi(),weight);
        histos2d[m_name+"_Omega_vs_Omega"        ]->Fill(track_x->getOmega(),track_y->getOmega(),weight);
        histos2d[m_name+"_TanLambda_vs_TanLambda"]->Fill(track_x->getTanLambda(),track_y->getTanLambda(),weight);
        histos2d[m_name+"_Z0_vs_Z0"              ]->Fill(track_x->getZ0(),track_y->getZ0(),weight);
        histos2d[m_name+"_time_vs_time"          ]->Fill(track_x->getTrackTime(),track_y->getTrackTime(),weight);
        histos2d[m_name+"_chi2_vs_chi2"          ]->Fill(track_x->getChi2Ndf(),
                                                         track_y->getChi2Ndf(),
                                                         weight);
        */
    }

}


//Residual Plots ============ They should probably go somewhere else ====================


void TridentHistos::FillResidualHistograms(Track* track, int ly, double res, double sigma) {
    
    double trk_mom = track->getP();
    std::string lyr = std::to_string(ly);
    
    TrackerHit* hit = nullptr;
    //Get the hits on track 
    for (int ihit = 0; ihit<track->getSvtHits().GetEntries();++ihit) {
        TrackerHit* tmphit = (TrackerHit*) track->getSvtHits().At(ihit);
        if (tmphit->getLayer() == ly) {
            hit = tmphit;
            break;
        }
    }
    
    if (!hit) {
        std::cout<<"Hit-on-track residual infos not found on hit on track list for ly="<<ly<<std::endl;
    }
    
    double hit_y = -9999.;
    if (hit) {
        hit_y = hit->getPosition()[1];
    }
    
    //General Plots
    Fill1DHisto("u_res_ly_"+lyr+"_h",res);
    Fill2DHisto("u_res_ly_"+lyr+"_vsp_hh",trk_mom,res);
    Fill2DHisto("u_res_ly_"+lyr+"_vsy_hh",hit_y,res);
    
    //Top = 0 bottom=1 - Per Volume
    std::string vol = track->getTanLambda()>0 ? "top" : "bot";
    Fill1DHisto("u_res_ly_"+lyr+"_"+vol+"_h",res);
    Fill2DHisto("u_res_ly_"+lyr+"_"+vol+"_vsp_hh",trk_mom,res);
        
}


std::pair<CalCluster*, Track*> TridentHistos::getTrackClusterPair(Track* trk,std::vector<CalCluster*>& clusters, float weight){

  double minDelta=666666;
  double minDeltaX=666666;
  double minDeltaY=666666;
  CalCluster* bestCluster=NULL; 
  double minDeltaCut=20.0;
  double clXMin=666666; 
  double clYMin=666666; 
  for(auto clu:clusters){
    double clX=(clu->getPosition()).at(0);
    double clY=(clu->getPosition()).at(1);
    if(trk->getTanLambda()*clY<0)//trk and cluster in same half
      continue; 
    int trkCh=trk->getCharge();
    if(trkCh*clX>0)  //positrons on positron side and vice/versa...sign is flipped in track collection? 
      continue;
    std::vector<double> posAtECal=trk->getPositionAtEcal();
    double delY=clY-posAtECal.at(1);
    double delX=clX-posAtECal.at(0);
    if(abs(delY)<minDelta){//right now, just take delta Y
      bestCluster=clu;
      minDelta=abs(delY);
      minDeltaX=delX;
      minDeltaY=delY;
      clXMin=clX; 
      clYMin=clY; 
    }
  }

  if(minDelta<minDeltaCut){
    Fill1DHisto("trkClMinDelta_h",minDelta,weight);  //should equal deltaY for now
    Fill1DHisto("trkClDeltaX_h",minDeltaX,weight); 
    Fill1DHisto("trkClDeltaY_h",minDeltaY,weight);
    Fill2DHisto("trkClDeltaXY_hh",minDeltaX,minDeltaY,weight);
    Fill2DHisto("trkClDeltaXvsX_hh",clXMin,minDeltaX,weight);
    Fill2DHisto("trkClDeltaYvsY_hh",clYMin,minDeltaY,weight);
    Fill2DHisto("trkClDeltaYvsX_hh",clXMin,minDeltaY,weight);
    Fill2DHisto("trkClDeltaXvsY_hh",clYMin,minDeltaX,weight);
  }

  return std::pair<CalCluster*,Track*>(bestCluster,trk);
}

/*
 *  fill cluster/track times and other ecal stuff for both WAB and trident events
 *  mg...5/9/20 currently just do time
 */
void TridentHistos::FillTrackClusterHistos(std::pair<CalCluster, Track> ele, std::pair<CalCluster, Track> posOrGamma, double calTimeOffset, double trkTimeOffset,std::vector<CalCluster*>  * clusterList, double weight){
  CalCluster eleClu=ele.first;
  Track eleTrk=ele.second;
  CalCluster posClu=posOrGamma.first; 
  Track posTrk=posOrGamma.second; //these "positrons" may be gammas
  

  double ele_cluTime=eleClu.getTime()-calTimeOffset;
  double pos_cluTime=posClu.getTime()-calTimeOffset;
  double ele_trkTime=eleTrk.getTrackTime()-trkTimeOffset;
  double pos_trkTime=posTrk.getTrackTime()-trkTimeOffset;


  double ele_cluX=eleClu.getPosition().at(0);
  double ele_cluY=eleClu.getPosition().at(1);
  double pos_cluX=posClu.getPosition().at(0);
  double pos_cluY=posClu.getPosition().at(1);

  ////
  double ele_trkX=eleTrk.getPositionAtEcal().at(0);
  double ele_trkY=eleTrk.getPositionAtEcal().at(1);
  double pos_trkX=posTrk.getPositionAtEcal().at(0);
  double pos_trkY=posTrk.getPositionAtEcal().at(1);
  ////

  double ele_clu_trk_deltaX=ele_cluX-ele_trkX;
  double pos_clu_trk_deltaX=pos_cluX-pos_trkX;

  double ele_clu_trk_deltaY=ele_cluY-ele_trkY;
  double pos_clu_trk_deltaY=pos_cluY-pos_trkY;

  //  if(eleClu.getTime()>-300){
  Fill1DHisto("ele_cl_time_h", eleClu.getTime()-calTimeOffset,weight);
  Fill1DHisto("ele_cl_ene_h",eleClu.getEnergy(),weight);
  Fill1DHisto("ele_clu_trk_deltaX_h",ele_clu_trk_deltaX,weight);
  Fill1DHisto("ele_clu_trk_deltaY_h",ele_clu_trk_deltaY,weight);

  Fill2DHisto("ele_clu_trk_deltaX_vs_cluX_hh",ele_cluX,ele_clu_trk_deltaX,weight);
  Fill2DHisto("ele_clu_trk_deltaY_vs_cluX_hh",ele_cluX,ele_clu_trk_deltaY,weight);
  Fill2DHisto("ele_cluY_vs_cluX_hh",ele_cluX,ele_cluY,weight);

  double pele=sqrt(eleTrk.getMomentum()[0]*eleTrk.getMomentum()[0]+
                   eleTrk.getMomentum()[1]*eleTrk.getMomentum()[1]+
                   eleTrk.getMomentum()[2]*eleTrk.getMomentum()[2]);
  Fill1DHisto("ele_cltrk_time_diff_h", eleClu.getTime()-calTimeOffset-eleTrk.getTrackTime()+trkTimeOffset,weight);
  Fill1DHisto("ele_EOverp_h",eleClu.getEnergy()/pele,weight);
  Fill2DHisto("ele_EOverp_vs_cluX_hh",ele_cluX,eleClu.getEnergy()/pele,weight);
  
  Fill1DHisto("pos_cl_time_h", posClu.getTime()-calTimeOffset,weight);
  Fill1DHisto("pos_cl_ene_h",posClu.getEnergy(),weight);
  Fill1DHisto("pos_clu_trk_deltaX_h",pos_clu_trk_deltaX,weight);
  Fill1DHisto("pos_clu_trk_deltaY_h",pos_clu_trk_deltaY,weight);
  Fill1DHisto("pos_cltrk_time_diff_h", posClu.getTime()-calTimeOffset-posTrk.getTrackTime()+trkTimeOffset,weight);
  double ppos=sqrt(posTrk.getMomentum()[0]*posTrk.getMomentum()[0]+
		     posTrk.getMomentum()[1]*posTrk.getMomentum()[1]+
		     posTrk.getMomentum()[2]*posTrk.getMomentum()[2]);
  Fill1DHisto("pos_EOverp_h",posClu.getEnergy()/ppos,weight);
  Fill2DHisto("pos_EOverp_vs_cluX_hh",pos_cluX,posClu.getEnergy()/ppos,weight);
  Fill2DHisto("pos_cluY_vs_cluX_hh",pos_cluX,pos_cluY,weight);
  Fill2DHisto("pos_clu_trk_deltaX_vs_cluX_hh",pos_cluX,pos_clu_trk_deltaX,weight);
  Fill2DHisto("pos_clu_trk_deltaY_vs_cluX_hh",pos_cluX,pos_clu_trk_deltaY,weight);

  //find the closest cluster that makes sense
  double minDistX=99999; 
  double minDistY=99999; 
  double minDist=99999; 
  int ele_minDistCluIndex=-666; 
  for(int i_clu=0; i_clu<clusterList->size(); i_clu++){
    CalCluster* clu=clusterList->at(i_clu);
    double clX=clu->getPosition().at(0);
    double clY=clu->getPosition().at(1);
    double clTime=clu->getTime()-calTimeOffset;
    if(ele_trkY*clY<0)// make sure track and cluster in same half of ecal
      continue;    
    //add in a cluster-track time cut to get rid of garbage
    double absClTrkTimeDiff=abs(clTime-ele_trkTime);    
    if(absClTrkTimeDiff>10)
      continue; 

    //find the min distance in 
    double dist=sqrt(pow(ele_trkX-clX,2)+pow(ele_trkY-clY,2));
    if(dist<minDist){
     ele_minDistCluIndex=i_clu;
    }    
  }

  minDistX=99999; 
  minDistY=99999; 
  minDist=99999; 
  int pos_minDistCluIndex=-666; 
  for(int i_clu=0; i_clu<clusterList->size(); i_clu++){
    CalCluster* clu=clusterList->at(i_clu);
    double clX=clu->getPosition().at(0);
    double clY=clu->getPosition().at(1);
    double clTime=clu->getTime()-calTimeOffset;
    if(pos_trkY*clY<0)// make sure track and cluster in same half of ecal
      continue;    
    //add in a cluster-track time cut to get rid of garbage
    double absClTrkTimeDiff=abs(clTime-pos_trkTime);    
    if(absClTrkTimeDiff>10)
      continue; 
    
    //find the min distance in 
    double dist=sqrt(pow(pos_trkX-clX,2)+pow(pos_trkY-clY,2));
    if(dist<minDist){
      pos_minDistCluIndex=i_clu;
    }    
  }
  //////   do new electron cluster stuff
  bool foundOgCluster=false;
  bool foundNewCluster=false;
  bool foundSameCluster=false;
  if(eleClu.getTime()>-300)
    foundOgCluster=true;
  if(ele_minDistCluIndex>-1){
    foundNewCluster=true;
    CalCluster* ele_myBestClu=clusterList->at(ele_minDistCluIndex);
    //see if this cluster is the same as cluster found by hps-java
    double clX=ele_myBestClu->getPosition().at(0);
    double clY=ele_myBestClu->getPosition().at(1);
    double clE=ele_myBestClu->getEnergy();
    double clTime=ele_myBestClu->getTime()-calTimeOffset;;
    if(clTime!= ele_cluTime){
      //found a different cluster...plot new parameters
      double new_deltaX=clX-ele_trkX; 
      double new_deltaY=clY-ele_trkY; 
      Fill1DHisto("new_ele_cl_time_h",clTime,weight);
      Fill1DHisto("new_ele_cl_ene_h",clE,weight);
      Fill1DHisto("new_ele_clu_trk_deltaX_h",new_deltaX,weight);
      Fill1DHisto("new_ele_clu_trk_deltaY_h",new_deltaY,weight);
      Fill1DHisto("new_ele_EOverp_h",clE/pele,weight);
      Fill2DHisto("new_ele_clu_trk_deltaX_vs_cluX_hh",clX,new_deltaX,weight);
      Fill2DHisto("new_ele_clu_trk_deltaY_vs_cluX_hh",clX,new_deltaY,weight);      
      Fill2DHisto("new_ele_EOverp_vs_cluX_hh",clX,clE/pele,weight);
      Fill2DHisto("new_ele_cluY_vs_cluX_hh",clX,clY,weight);
    }else{
      foundSameCluster=true;
    }    
  } 
  int clMatchCode=-666;
  if(foundSameCluster)
    clMatchCode=0;
  else if(foundOgCluster and !foundNewCluster)
    clMatchCode=-1; 
  else if(!foundOgCluster and foundNewCluster)
    clMatchCode=1; 
  else if(foundOgCluster and foundNewCluster)
    clMatchCode=2; 
  else
    clMatchCode=-2; // didn't find cluster match with either old or new algorithm
  Fill1DHisto("new_ele_cluster_match_code_h",clMatchCode,weight);

  
  //////   do new positron cluster stuff
  foundOgCluster=false;
  foundNewCluster=false;
  foundSameCluster=false;
  if(posClu.getTime()>-300)
    foundOgCluster=true;
  if(pos_minDistCluIndex>-1){
    foundNewCluster=true;
    CalCluster* pos_myBestClu=clusterList->at(pos_minDistCluIndex);
    //see if this cluster is the same as cluster found by hps-java
    double clX=pos_myBestClu->getPosition().at(0);
    double clY=pos_myBestClu->getPosition().at(1);
    double clE=pos_myBestClu->getEnergy();
    double clTime=pos_myBestClu->getTime()-calTimeOffset;;
    if(clTime!= pos_cluTime){
      //found a different cluster...plot new parameters
      double new_deltaX=clX-pos_trkX; 
      double new_deltaY=clY-pos_trkY; 
      Fill1DHisto("new_pos_cl_time_h",clTime,weight);
      Fill1DHisto("new_pos_cl_ene_h",clE,weight);
      Fill1DHisto("new_pos_clu_trk_deltaX_h",new_deltaX,weight);
      Fill1DHisto("new_pos_clu_trk_deltaY_h",new_deltaY,weight);
      Fill1DHisto("new_pos_EOverp_h",clE/ppos,weight);
      Fill2DHisto("new_pos_clu_trk_deltaX_vs_cluX_hh",clX,new_deltaX,weight);
      Fill2DHisto("new_pos_clu_trk_deltaY_vs_cluX_hh",clX,new_deltaY,weight);
      Fill2DHisto("new_pos_EOverp_vs_cluX_hh",clX,clE/ppos,weight);
      Fill2DHisto("new_pos_cluY_vs_cluX_hh",clX,clY,weight);
    }else{
      foundSameCluster=true;
    }    
  } 
  clMatchCode=-666;
  if(foundSameCluster)
    clMatchCode=0;
  else if(foundOgCluster and !foundNewCluster)
    clMatchCode=-1; 
  else if(!foundOgCluster and foundNewCluster)
    clMatchCode=1; 
  else if(foundOgCluster and foundNewCluster)
    clMatchCode=2; 
  else
    clMatchCode=-2; // didn't find cluster match with either old or new algorithm
  Fill1DHisto("new_pos_cluster_match_code_h",clMatchCode,weight);


  Fill1DHisto("cl_time_diff_h", posClu.getTime()-eleClu.getTime(),weight);    
  Fill1DHisto("trk_time_diff_h", posTrk.getTrackTime()-eleTrk.getTrackTime(),weight);    
  Fill1DHisto("gamma_cl_time_h", posClu.getTime()-calTimeOffset,weight);
  Fill1DHisto("gamma_cl_ene_h",posClu.getEnergy(),weight);

}
/*
 *  fill  WAB specific histos
 *  already have track and cluster plots from above methods
 */
void TridentHistos::FillWABHistos(std::pair<CalCluster*, Track*> ele, CalCluster* gamma,double weight){
  CalCluster* eleClu=ele.first;
  Track* eleTrk=ele.second;
  double esum=0;
  if(eleClu)
    esum=eleClu->getEnergy()+gamma->getEnergy();
  double psum=sqrt(eleTrk->getMomentum()[0]*eleTrk->getMomentum()[0]+
		   eleTrk->getMomentum()[1]*eleTrk->getMomentum()[1]+
		   eleTrk->getMomentum()[2]*eleTrk->getMomentum()[2])+ gamma->getEnergy();
  if(eleClu)
    Fill1DHisto("wab_esum_h",esum,weight); 
  Fill1DHisto("wab_psum_h",psum,weight);   
}
