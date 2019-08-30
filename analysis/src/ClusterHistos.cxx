#include "ClusterHistos.h"
#include <math.h>

void ClusterHistos::Define1DHistos() {

  //TODO improve this naming scheme
  std::string h_name = "";
  
  //Cluster position
  histos1d[m_name+"_gz"] = plot1D(m_name+"_gz",
				      "Global Z [mm]",20000,-1000,2000);

  histos1d[m_name+"_rot1gz"] = plot1D(m_name+"_rot1gz",
				      "Global Z [mm]",20000,-1000,2000);


  histos1d[m_name+"_rot2gz"] = plot1D(m_name+"_rot2gz",
				      "Global Z [mm]",20000,-1000,2000);

  
  
  histos1d[m_name+"_charge"] = plot1D(m_name+"_edep",
				      "edep",30,0,3e-5);
}


void ClusterHistos::Define2DHistos() {
  std::string h_name = "";

  histos2d[m_name+"_charge_vs_gx"] = plot2D(m_name+"_charge_vs_gx",
					      "Global X [mm] ",100,0,100,
					      "edep",30,0,3e-5);

  int nbins = 1000;
  float pitch = 0.055;
  
  histos2d[m_name+"_charge_vs_gy"] = plot2D(m_name+"_charge_vs_gy",
					    "Gloabl Y [mm]",nbins,0.750,(nbins+1)*0.055,
					    "edep",30,0,3e-5); 
}

void ClusterHistos::FillHistograms(TrackerHit* hit,float weight) {


  float rotz1 = -hit->getGlobalX() * sin(30.5e-3) + hit->getGlobalZ()*cos(30.5e-3);
  float rotz2 =  hit->getGlobalX() * sin(30.5e-3) + hit->getGlobalZ()*cos(30.5e-3);


  histos1d[m_name+"_gz"]->Fill(hit->getGlobalZ(),weight);
  histos1d[m_name+"_rot1gz"]->Fill(rotz1,weight);
  histos1d[m_name+"_rot2gz"]->Fill(rotz2,weight);
  
  //1D

    if (rotz2<50) {
      
      histos1d[m_name+"_charge"]->Fill(hit->getCharge(),weight);
      
      
      //2D
      
      
      histos2d[m_name+"_charge_vs_gx"]->Fill(hit->getGlobalX(),hit->getCharge(),weight);
      histos2d[m_name+"_charge_vs_gy"]->Fill(hit->getGlobalY(),hit->getCharge(),weight);
    }
    
}



