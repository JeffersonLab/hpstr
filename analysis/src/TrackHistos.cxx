#include "TrackHistos.h"
#include <iostream>

void TrackHistos::Define1DHistos() {

    //TODO improve naming
    std::string h_name = "";
    
    for (auto hist : _h_configs.items()) {
        h_name = m_name+"_"+hist.key();
        histos1d[h_name] = plot1D(h_name,hist.value().at("xtitle"),
                                  hist.value().at("bins"),
                                  hist.value().at("minX"),
                                  hist.value().at("maxX"));
        
        std::string ytitle = hist.value().at("ytitle");
        histos1d[h_name]->GetYaxis()->SetTitle(ytitle.c_str());
        
        if (hist.value().contains("labels")) {
            std::vector<std::string> labels = hist.value().at("labels").get<std::vector<std::string> >();
        
            if (labels.size() < hist.value().at("bins")) {
                std::cout<<"Cannot apply labels to histogram:"<<h_name<<std::endl;
            }
            else {
                for (int i = 1; i<=hist.value().at("bins");++i)
                    histos1d[h_name]->GetXaxis()->SetBinLabel(i,labels[i-1].c_str());
            }
        }
       
    }   
    
    //Hit content
    //shared hits
    //location of hit in first layer
    //Total charge of hit in first layer
    //size of hit in first layer
  
}
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
    
    //TODO improve
  
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
        histos1d[m_name+"_sharingHits_h"]->Fill(0.,weight);
        if (track->getNShared() == 0)
            histos1d[m_name+"_sharingHits_h"]->Fill(1.,weight);
        else {
            //track has shared hits
            if (track->getSharedLy0())
                histos1d[m_name+"_sharingHits_h"]->Fill(2.,weight);
            if (track->getSharedLy1())
                histos1d[m_name+"_sharingHits_h"]->Fill(3.,weight);
            if (track->getSharedLy0() && track->getSharedLy1())
                histos1d[m_name+"_sharingHits_h"]->Fill(4.,weight);
            if (!track->getSharedLy0() && !track->getSharedLy1())
                histos1d[m_name+"_sharingHits_h"]->Fill(5.,weight);
        }
    
    
        //TODO improve this
        if (track -> is345Seed())
            histos1d[m_name+"_strategy_h"]->Fill(0);
        if (track-> is456Seed())
            histos1d[m_name+"_strategy_h"]->Fill(1);
        if (track-> is123SeedC4())
            histos1d[m_name+"_strategy_h"]->Fill(2);
        if (track->is123SeedC5())
            histos1d[m_name+"_strategy_h"]->Fill(3);
        if (track->isMatchedTrack())
            histos1d[m_name+"_strategy_h"]->Fill(4);
        if (track->isGBLTrack())
            histos1d[m_name+"_strategy_h"]->Fill(5);
    
        histos1d[m_name+"_type_h"     ]->Fill(track->getType()     ,weight);
    
    }
  
    //Vertices
    //TODO improve this
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



