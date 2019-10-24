#include "TrackHistos.h"
#include <iostream>

void TrackHistos::Define1DHistos() {

  //TODO improve naming
  std::string h_name = "";

  histos1d[m_name+"_n_tracks"] = plot1D(m_name+"_n_tracks",
					"n_tracks",10,0,10);

  for (unsigned int itp = 0; itp<tPs.size(); ++itp){ 
    
    histos1d[m_name+"_"+tPs[itp] ] = plot1D(m_name+tPs[itp],
					    tPs[itp],axes[tPs[itp]][0],axes[tPs[itp]][1],axes[tPs[itp]][2]);
  }
 
  int nbinsX = 6;
  histos1d[m_name+"_strategy" ] = plot1D(m_name+"_strategy",
					 "Strategy",nbinsX,-0.5,nbinsX-0.5);
  
  std::string labels[6] = {"s345","s456","s123c4","s123c5","Match","GBL"};
  
  for (int i = 1; i<=nbinsX; ++i) 
    histos1d[m_name+"_strategy"]->GetXaxis()->SetBinLabel(i,labels[i-1].c_str());
  
  histos1d[m_name+"_type" ] = plot1D(m_name+"_type",
					 "Type",64,0,64);

  histos1d[m_name+"_nShared"]      = plot1D(m_name+"_nShared",
					    "nShared Hits",8,-0.5,7.5);
  histos1d[m_name+"_sharingHits"]  = plot1D(m_name+"_sharingHits",
					    "sharingHits",6,-0.5,5.5);

  labels[0]="All Tracks";
  labels[1]="nShared = 0";
  labels[2]="SharedLy0";
  labels[3]="SharedLy1";
  labels[4]="SharedLy0AndLy1";
  labels[5]="Shared Others";

  
  for (int i = 1; i<=nbinsX; ++i) 
    histos1d[m_name+"_sharingHits"]->GetXaxis()->SetBinLabel(i,labels[i-1].c_str());
  
  
  //Hit content
  //shared hits
  //location of hit in first layer
  //Total charge of hit in first layer
  //size of hit in first layer
  
  //TODO: Move it somewhere else?
  //Vertices 
  
  histos1d[m_name+"_n_vertices"] = plot1D(m_name+"_n_vertices",
					  "N Vertices",10,0,10);
  
  for ( auto vp : vPs) {
    histos1d[m_name+"_"+vp] = plot1D(m_name+vp,
				     vp,axes[vp][0],axes[vp][1],axes[vp][2]);
  }
}

//TODO make it load from a configuration file!
void TrackHistos::BuildAxes() {

    axes["d0"].push_back(200);
    axes["d0"].push_back(-10);
    axes["d0"].push_back(10);

    axes["Phi"].push_back(100);
    axes["Phi"].push_back(-0.5);
    axes["Phi"].push_back(0.5);

    axes["Omega"].push_back(100);
    axes["Omega"].push_back(-0.002);
    axes["Omega"].push_back(0.002);

    axes["TanLambda"].push_back(200);
    axes["TanLambda"].push_back(-0.6);
    axes["TanLambda"].push_back(0.6);

    axes["Z0"].push_back(200);
    axes["Z0"].push_back(-20);
    axes["Z0"].push_back(20);

    axes["time"].push_back(200);
    axes["time"].push_back(-10);
    axes["time"].push_back(10);

  axes["chi2"].push_back(200);
  axes["chi2"].push_back(0);
  axes["chi2"].push_back(30); 

  axes["vtx_chi2"].push_back(200);
  axes["vtx_chi2"].push_back(0);
  axes["vtx_chi2"].push_back(30); 

  axes["vtx_X"].push_back(200);
  axes["vtx_X"].push_back(-50);
  axes["vtx_X"].push_back(50);

  axes["vtx_Y"].push_back(200);
  axes["vtx_Y"].push_back(-50);
  axes["vtx_Y"].push_back(50);

  axes["vtx_Z"].push_back(200);
  axes["vtx_Z"].push_back(-50);
  axes["vtx_Z"].push_back(50);

  axes["vtx_sigma_X"].push_back(100);
  axes["vtx_sigma_X"].push_back(0);
  axes["vtx_sigma_X"].push_back(10);

  axes["vtx_sigma_Y"].push_back(100);
  axes["vtx_sigma_Y"].push_back(0);
  axes["vtx_sigma_Y"].push_back(10);

  axes["vtx_sigma_Z"].push_back(100);
  axes["vtx_sigma_Z"].push_back(0);
  axes["vtx_sigma_Z"].push_back(10);
  
  axes["vtx_InvM"].push_back(100);
  axes["vtx_InvM"].push_back(0);
  axes["vtx_InvM"].push_back(0.2);

  axes["vtx_InvMErr"].push_back(100);
  axes["vtx_InvMErr"].push_back(0);
  axes["vtx_InvMErr"].push_back(0.2);
  
}



void TrackHistos::Define2DHistos() {

  //TODO improve naming
  std::string h_name = "";
  
  //TODO improve binning
  if (doTrkCompPlots) {
    
    for (unsigned int itp = 0; itp<tPs.size(); ++itp){
      
      if (debug_)
	std::cout<<"Bulding:: TH2::" + m_name+"_"+tPs[itp]+"_vs_"+tPs[itp] << std::endl;
      
      histos2d[m_name+"_"+tPs[itp]+"_vs_"+tPs[itp] ] = plot2D(m_name+tPs[itp]+"_vs_"+tPs[itp],
							      tPs[itp],axes[tPs[itp]][0],axes[tPs[itp]][1],axes[tPs[itp]][2],
							      tPs[itp],axes[tPs[itp]][0],axes[tPs[itp]][1],axes[tPs[itp]][2]);
      
    }//loop on vars
  }//do track comparison
    
  histos2d[m_name+"_vtxY_vs_vtxX"] = plot2D(m_name+"_vtxY_vs_vtxX",
					    "vtxX",axes["vtx_X"][0],axes["vtx_X"][1],axes["vtx_X"][2],
					    "vtxY",axes["vtx_Y"][0],axes["vtx_Y"][1],axes["vtx_Y"][2]);
  
  
}//define 2dhistos

//TODO Improve passing a vector to fill histos
void TrackHistos::Fill1DHisto(const std::string& histoName,float value, float weight) {
  if (histos1d[m_name+"_"+histoName])
    histos1d[m_name+"_"+histoName]->Fill(value,weight);
  else
    std::cout<<"ERROR::Fill1DHisto Histogram not found! "<<m_name+"_"+histoName<<std::endl;
}

void TrackHistos::Fill1DHistograms(Track *track, Vertex* vtx, float weight ) {
    
  //TODO improve
  
  if (track) {
    
    histos1d[m_name+"_d0"       ]->Fill(track->getD0()          ,weight);
    histos1d[m_name+"_Phi"      ]->Fill(track->getPhi()         ,weight);
    histos1d[m_name+"_Omega"    ]->Fill(track->getOmega()       ,weight);
    histos1d[m_name+"_TanLambda"]->Fill(track->getTanLambda()   ,weight);
    histos1d[m_name+"_Z0"       ]->Fill(track->getZ0()          ,weight);
    histos1d[m_name+"_time"     ]->Fill(track->getTrackTime()   ,weight);
    histos1d[m_name+"_chi2"     ]->Fill(track->getChi2Ndf()     ,weight);
    histos1d[m_name+"_nShared"  ]->Fill(track->getNShared()     ,weight);
    
    //All Tracks
    histos1d[m_name+"_sharingHits"]->Fill(0.,weight);
    if (track->getNShared() == 0)
      histos1d[m_name+"_sharingHits"]->Fill(1.,weight);
    else {
      //track has shared hits
      if (track->getSharedLy0())
	histos1d[m_name+"_sharingHits"]->Fill(2.,weight);
      if (track->getSharedLy1())
	histos1d[m_name+"_sharingHits"]->Fill(3.,weight);
      if (track->getSharedLy0() && track->getSharedLy1())
	histos1d[m_name+"_sharingHits"]->Fill(4.,weight);
      if (!track->getSharedLy0() && !track->getSharedLy1())
	histos1d[m_name+"_sharingHits"]->Fill(5.,weight);
    }
    
    
    //TODO improve this
    if (track -> is345Seed())
      histos1d[m_name+"_strategy"]->Fill(0);
    if (track-> is456Seed())
      histos1d[m_name+"_strategy"]->Fill(1);
    if (track-> is123SeedC4())
      histos1d[m_name+"_strategy"]->Fill(2);
    if (track->is123SeedC5())
      histos1d[m_name+"_strategy"]->Fill(3);
    if (track->isMatchedTrack())
      histos1d[m_name+"_strategy"]->Fill(4);
    if (track->isGBLTrack())
      histos1d[m_name+"_strategy"]->Fill(5);
    
    histos1d[m_name+"_type"     ]->Fill(track->getType()     ,weight);
    
  }
  
  //Vertices
  //TODO improve this
  if (vtx) {
    
    histos1d[m_name+"_vtx_chi2"]->Fill(vtx->getChi2(),weight);
    histos1d[m_name+"_vtx_X"]->Fill(vtx->getX(),weight);
    histos1d[m_name+"_vtx_Y"]->Fill(vtx->getY(),weight);
    histos1d[m_name+"_vtx_Z"]->Fill(vtx->getZ(),weight);
    // 0 xx 1 xy 2 xz 3 yy 4 yz 5 zz
    histos1d[m_name+"_vtx_sigma_X"]->Fill(sqrt(vtx->getCovariance()[0]),weight);
    histos1d[m_name+"_vtx_sigma_Y"]->Fill(sqrt(vtx->getCovariance()[3]),weight);
    histos1d[m_name+"_vtx_sigma_Z"]->Fill(sqrt(vtx->getCovariance()[5]),weight);
    histos1d[m_name+"_vtx_InvM"]->Fill(vtx->getInvMass(),weight);
    histos1d[m_name+"_vtx_InvMErr"]->Fill(vtx->getInvMassErr(),weight);
  }
  

}



void TrackHistos::FillTrackComparisonHistograms(Track* track_x, Track* track_y, float weight) {

    if (doTrkCompPlots) {
        histos2d[m_name+"_d0_vs_d0"              ]->Fill(track_x->getD0(),track_y->getD0(),weight);
        histos2d[m_name+"_Phi_vs_Phi"            ]->Fill(track_x->getPhi(),track_y->getPhi(),weight);
        histos2d[m_name+"_Omega_vs_Omega"        ]->Fill(track_x->getOmega(),track_y->getOmega(),weight);
        histos2d[m_name+"_TanLambda_vs_TanLambda"]->Fill(track_x->getTanLambda(),track_y->getTanLambda(),weight);
        histos2d[m_name+"_Z0_vs_Z0"              ]->Fill(track_x->getZ0(),track_y->getZ0(),weight);
        histos2d[m_name+"_time_vs_time"          ]->Fill(track_x->getTrackTime(),track_y->getTrackTime(),weight);
        histos2d[m_name+"_chi2_vs_chi2"          ]->Fill(track_x->getChi2Ndf(),
                track_y->getChi2Ndf(),
                weight);
    }
}



