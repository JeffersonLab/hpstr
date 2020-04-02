#include "TrackHistos.h"
#include "TLorentzVector.h"
#include "TVector3.h"
#include <iostream>

void TrackHistos::BuildAxes(){}

void TrackHistos::Define2DHistos() {

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


void TrackHistos::Fill1DVertex(Vertex* vtx, 
                               Particle* ele, 
                               Particle* pos, 
                               Track* ele_trk,
                               Track* pos_trk,
                               float weight) {
    
    Fill1DVertex(vtx,weight);

    //TODO remove hardcode!
    if (ele_trk)
        Fill1DTrack(ele_trk,weight,"ele_");
    if (pos_trk)
        Fill1DTrack(pos_trk,weight,"pos_");
    
    
    TLorentzVector p_ele;
    //p_ele.SetPxPyPzE(ele->getMomentum()[0], ele->getMomentum()[1],ele->getMomentum()[2],ele->getEnergy());
    p_ele.SetPxPyPzE(ele_trk->getMomentum()[0],ele_trk->getMomentum()[1],ele_trk->getMomentum()[2],ele->getEnergy());
    
    TLorentzVector p_pos;
    //p_pos.SetPxPyPzE(pos->getMomentum()[0], pos->getMomentum()[1],pos->getMomentum()[2],pos->getEnergy());
    p_pos.SetPxPyPzE(pos_trk->getMomentum()[0],pos_trk->getMomentum()[1],pos_trk->getMomentum()[2],pos->getEnergy());
    
    //Fill ele and pos information
    Fill1DHisto("ele_p_h",p_ele.P(),weight);
    Fill1DHisto("pos_p_h",p_pos.P(),weight);

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
}


void TrackHistos::Fill2DTrack(Track* track, float weight, const std::string& trkname) {
    
    
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

void TrackHistos::Fill1DTrack(Track* track, float weight, const std::string& trkname) {
    
    double charge = (double) track->getCharge();

    //2D hits
    int n_hits_2d = track->getTrackerHitCount();
    if (!track->isKalmanTrack())
        n_hits_2d*=2;

    Fill1DHisto(trkname+"d0_h"       ,track->getD0()          ,weight);
    Fill1DHisto(trkname+"Phi_h"      ,track->getPhi()         ,weight);
    Fill1DHisto(trkname+"Omega_h"    ,track->getOmega()       ,weight);
    Fill1DHisto(trkname+"pT_h"       ,-1*charge*track->getPt()   ,weight);
    Fill1DHisto(trkname+"invpT_h"    ,-1*charge/track->getPt()   ,weight);
    Fill1DHisto(trkname+"TanLambda_h",track->getTanLambda()   ,weight);
    Fill1DHisto(trkname+"Z0_h"       ,track->getZ0()          ,weight);
    Fill1DHisto(trkname+"time_h"     ,track->getTrackTime()   ,weight);
    Fill1DHisto(trkname+"chi2_h"     ,track->getChi2Ndf()     ,weight);
    Fill1DHisto(trkname+"nShared_h"  ,track->getNShared()     ,weight);
    Fill1DHisto(trkname+"nHits_2d_h" ,n_hits_2d               ,weight);
        
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
        
        
    Fill1DHisto(trkname+"type_h",track->getType(),weight);
}

void TrackHistos::Fill1DVertex(Vertex* vtx, float weight) {
    
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

void TrackHistos::Fill1DHistograms(Track *track, Vertex* vtx, float weight ) {
    
    if (track) {
        Fill1DTrack(track);
    }
  
    //Vertices
    
    if (vtx) {
        Fill1DVertex(vtx);
    }
}


void TrackHistos::Fill1DTrackTruth(Track *track, Track* truth_track, float weight, const std::string& trkname) {
    
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



void TrackHistos::Fill2DHistograms(Vertex* vtx, float weight) {    

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

void TrackHistos::FillTrackComparisonHistograms(Track* track_x, Track* track_y, float weight) {

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



