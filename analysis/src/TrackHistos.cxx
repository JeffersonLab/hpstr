#include "TrackHistos.h"
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


void TrackHistos::Fill1DHistograms(Track *track, Vertex* vtx, float weight ) {
    
    if (track) {
      
        Fill1DHisto("d0_h"       ,track->getD0()          ,weight);
        Fill1DHisto("Phi_h"      ,track->getPhi()         ,weight);
        Fill1DHisto("Omega_h"    ,track->getOmega()       ,weight);
        Fill1DHisto("TanLambda_h",track->getTanLambda()   ,weight);
        Fill1DHisto("Z0_h"       ,track->getZ0()          ,weight);
        Fill1DHisto("time_h"     ,track->getTrackTime()   ,weight);
        Fill1DHisto("chi2_h"     ,track->getChi2Ndf()     ,weight);
        Fill1DHisto("nShared_h"  ,track->getNShared()     ,weight);
        
        //All Tracks
        Fill1DHisto("sharingHits_h",0,weight);
        if (track->getNShared() == 0)
            Fill1DHisto("sharingHits_h",0,weight);
        else {
            //track has shared hits
            if (track->getSharedLy0())
                Fill1DHisto("sharingHits_h",2.,weight);
            if (track->getSharedLy1())
                Fill1DHisto("sharingHits_h",3.,weight);
            if (track->getSharedLy0() && track->getSharedLy1())
                Fill1DHisto("sharingHits_h",4.,weight);
            if (!track->getSharedLy0() && !track->getSharedLy1())
                Fill1DHisto("sharingHits_h",5.,weight);
        }
                
        if (track -> is345Seed())
            Fill1DHisto("strategy_h",0,weight);
        if (track-> is456Seed())
            Fill1DHisto("strategy_h",1,weight);
        if (track-> is123SeedC4())
            Fill1DHisto("strategy_h",2,weight);
        if (track->is123SeedC5())
            Fill1DHisto("strategy_h",3,weight);
        if (track->isMatchedTrack())
            Fill1DHisto("strategy_h",4,weight);
        if (track->isGBLTrack())
            Fill1DHisto("strategy_h",5,weight);
        
        
        Fill1DHisto("type_h",track->getType(),weight);
    }
  
    //Vertices
    
    if (vtx) {
        
        Fill1DHisto("vtx_chi2_h"   ,vtx->getChi2(),weight);
        Fill1DHisto("vtx_X_h"      ,vtx->getX(),weight);
        Fill1DHisto("vtx_Y_h"      ,vtx->getY(),weight);
        Fill1DHisto("vtx_Z_h"      ,vtx->getZ(),weight);
        // 0 xx 1 xy 2 xz 3 yy 4 yz 5 zz
        Fill1DHisto("vtx_sigma_X_h",sqrt(vtx->getCovariance()[0]),weight);
        Fill1DHisto("vtx_sigma_Y_h",sqrt(vtx->getCovariance()[3]),weight);
        Fill1DHisto("vtx_sigma_Z_h",sqrt(vtx->getCovariance()[5]),weight);
        Fill1DHisto("vtx_InvM_h"   ,vtx->getInvMass(),weight);
        Fill1DHisto("vtx_InvMErr_Z_h",vtx->getInvMassErr(),weight);
        
    }
  

}


void TrackHistos::Fill2DHistograms(Track* track, Vertex* vtx, float weight) { 
    
    if (track) {
        Fill2DHisto("tanlambda_vs_phi0_hh",track->getPhi(),track->getTanLambda(), weight);
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



