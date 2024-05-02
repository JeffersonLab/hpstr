#include "TridentHistos.h"
#include "TLorentzVector.h"
#include "TVector3.h"
#include <iostream>
#include <regex>
#include <cstddef>
void TridentHistos::BuildAxes(){}

void TridentHistos::DefineHistos(){  
  std::vector<std::string> layerTags;
  unsigned char lbits=15;
  int counter=15; 
  while(counter>=0){
    std::string lstring="";
    for (int i = 4; i < 8; i++) { //just doing sensor layers 1-4
      //      char* tmpSt;
      //      printf("%d", !!((lbits << i) & 0x80));
      int tmpCh=!!((lbits << i) & 0x80);
      std::string tmpSt="0";
      if(tmpCh==1)
	tmpSt="1";
      lstring+=tmpSt;      
    }
    //    printf("\n");
    layerTags.push_back(lstring);
    counter--;
    lbits--;
  }
       
  std::vector<std::string> halfTags{"","top","bot"};
  std::vector<std::string> trkTags{"pos","ele"};
  std::vector<std::string> trkCombos;  
  std::vector<std::string>::iterator halfIter = halfTags.begin();
  for(halfIter; halfIter < halfTags.end(); halfIter++){
    std::string half=*halfIter;
    std::vector<std::string>::iterator trkIter = trkTags.begin();
    for(trkIter; trkIter < trkTags.end(); trkIter++){
      std::string trk=*trkIter; 
      std::string comboStr=half+"_"+trk;
      if(half=="")
	comboStr=trk;
      trkCombos.push_back(comboStr);
    }
  }  

  /*
  std::vector<std::string> trkLayerCombos; 
  std::vector<std::string>::iterator trkIter = trkCombos.begin();
  for(trkIter;trkIter<trkCombos.end();trkIter++){
    std::string trkSt=*trkIter;
    std::vector<std::string>::iterator lIter = layerTags.begin();
    for(lIter;lIter<layerTags.end();lIter++){
      std::string lSt=*lIter; 
      std::string comboStr="L"+lSt+"_"+trkSt;
      if(trkSt=="")
	comboStr="L"+lSt;
      trkLayerCombos.push_back(comboStr);
      //      std::cout<<comboStr<<std::endl;

    }
  }
  */
  //  std::vector<std::string>  layerTagsForV0{"1111","0011","0111"};
  std::vector<std::string>  layerTagsForV0{"1111","0011","0111","1011"};
  std::vector<std::string> trkLayerCombos; 
  std::vector<std::string>::iterator trkIter = trkCombos.begin();
  for(trkIter;trkIter<trkCombos.end();trkIter++){
    std::string trkSt=*trkIter;
    std::vector<std::string>::iterator lIter = layerTagsForV0.begin();
    for(lIter;lIter<layerTagsForV0.end();lIter++){
      std::string l1St=*lIter; 
      std::vector<std::string>::iterator l2Iter = layerTagsForV0.begin();
      for(l2Iter;l2Iter<layerTagsForV0.end();l2Iter++){
	std::string l2St=*l2Iter; 
	std::string comboStr="pos"+l1St+"_ele"+l2St+"_"+trkSt;
	trkLayerCombos.push_back(comboStr);
      }
      
    }
  }


  std::vector<std::string> V0Combos{"","top_pos","bot_pos"};  
  std::vector<std::string> v0LayerCombos; 
  std::vector<std::string>::iterator v0Iter = V0Combos.begin();
  for(v0Iter;v0Iter<V0Combos.end();v0Iter++){
    std::string v0St=*v0Iter;
    std::vector<std::string>::iterator lIter = layerTagsForV0.begin();
    for(lIter;lIter<layerTagsForV0.end();lIter++){
      std::string l1St=*lIter; 
      std::vector<std::string>::iterator l2Iter = layerTagsForV0.begin();
      for(l2Iter;l2Iter<layerTagsForV0.end();l2Iter++){
	std::string l2St=*l2Iter; 
	std::string comboStr="pos"+l1St+"_ele"+l2St+"_"+v0St;
	if(v0St=="")
	  comboStr="pos"+l1St+"_ele"+l2St;
	v0LayerCombos.push_back(comboStr);
      }
    }
  }



  DefineHistosFromTemplateOnly(trkCombos,"_tc_h");  //should match both 1d and 2d histos
  DefineHistosFromTemplateOnly(halfTags, "_hc_h");  //should match both 1d and 2d histos (though, I think there are o
  //  DefineHistosFromTemplateOnly(trkLayerCombos,"_tc_h");   //should match both 1d and 2d histos
  DefineHistosFromTemplateOnly(V0Combos,"_vc_h");   //should match both 1d and 2d histos
  //  DefineHistosFromTemplateOnly(V0Combos,"_tc_h");   //should match both 1d and 2d histos
  DefineHistosFromTemplateOnly(v0LayerCombos,"_vc_h");   //should match both 1d and 2d histos
  DefineHistosFromTemplateOnly(trkLayerCombos,"_tc_h");   //should match both 1d and 2d histos

  DefineOneTimeHistos();   
}


void TridentHistos::Define2DHistos() {
  
}//define 2dhistos

void TridentHistos::AssignLayerCode(Track* ele_trk,
				    Track* pos_trk){
  std::string eleLayerCode=getLayerCodeFromTrack(ele_trk);
  std::string posLayerCode=getLayerCodeFromTrack(pos_trk);
  this->layerCode="pos"+posLayerCode+"_ele"+eleLayerCode+"_";
}

void TridentHistos::Fill1DVertex(Vertex* vtx, 
                                 Particle* ele, 
                                 Particle* pos, 
                                 Track* ele_trk,
                                 Track* pos_trk,
                                 double trkTimeOffset,
                                 float weight) {
    

    if (ele_trk)
      Fill1DTrack(ele_trk,trkTimeOffset,weight,"ele_");
    if (pos_trk)
      Fill1DTrack(pos_trk,trkTimeOffset,weight,"pos_");
    //    std::cout<<"TridentHistos::Fill1DVertex  1d track histos filled"<<std::endl;
    std::string half="top";
    if(pos_trk->getTanLambda()<0)
      half="bot";    
    std::string posTag=half+"_pos_";
    half="top";
    if(ele_trk->getTanLambda()<0)
      half="bot";
    std::string eleTag=half+"_ele_";
    
    //    std::string eleLayerCode=getLayerCodeFromTrack(ele_trk);
    //    std::string posLayerCode=getLayerCodeFromTrack(pos_trk);
    //    std::string layerCode="pos"+eleLayerCode+"_ele"+posLayerCode+"_";

    Fill1DHisto("vtx_chi2_vc_h"   ,vtx->getChi2(),weight);
    Fill1DHisto("vtx_X_vc_h"      ,vtx->getX(),weight);
    Fill1DHisto("vtx_Y_vc_h"      ,vtx->getY(),weight);
    Fill1DHisto("vtx_Z_vc_h"      ,vtx->getZ(),weight);

    Fill1DHisto(posTag+"vtx_chi2_vc_h"   ,vtx->getChi2(),weight);
    Fill1DHisto(posTag+"vtx_X_vc_h"      ,vtx->getX(),weight);
    Fill1DHisto(posTag+"vtx_Y_vc_h"      ,vtx->getY(),weight);
    Fill1DHisto(posTag+"vtx_Z_vc_h"      ,vtx->getZ(),weight);

    Fill1DHisto(layerCode+"vtx_chi2_vc_h"   ,vtx->getChi2(),weight);
    Fill1DHisto(layerCode+"vtx_X_vc_h"      ,vtx->getX(),weight);
    Fill1DHisto(layerCode+"vtx_Y_vc_h"      ,vtx->getY(),weight);
    Fill1DHisto(layerCode+"vtx_Z_vc_h"      ,vtx->getZ(),weight);

    Fill1DHisto(layerCode+posTag+"vtx_chi2_vc_h"   ,vtx->getChi2(),weight);
    Fill1DHisto(layerCode+posTag+"vtx_X_vc_h"      ,vtx->getX(),weight);
    Fill1DHisto(layerCode+posTag+"vtx_Y_vc_h"      ,vtx->getY(),weight);
    Fill1DHisto(layerCode+posTag+"vtx_Z_vc_h"      ,vtx->getZ(),weight);

    TVector3 vtxPosSvt;
    vtxPosSvt.SetX(vtx->getX());
    vtxPosSvt.SetY(vtx->getY());
    vtxPosSvt.SetZ(vtx->getZ());
    
    vtxPosSvt.RotateY(-0.0305);

    //Fill1DHisto("vtx_X_svt_vc_h",vtxPosSvt.X(),weight);
    //Fill1DHisto("vtx_Y_svt_vc_h",vtxPosSvt.Y(),weight);
    //Fill1DHisto("vtx_Z_svt_vc_h",vtxPosSvt.Z(),weight);
    
    // 0 xx 1 xy 2 xz 3 yy 4 yz 5 zz
    Fill1DHisto("vtx_sigma_X_vc_h",sqrt(vtx->getCovariance()[0]),weight);
    Fill1DHisto("vtx_sigma_Y_vc_h",sqrt(vtx->getCovariance()[3]),weight);
    Fill1DHisto("vtx_sigma_Z_vc_h",sqrt(vtx->getCovariance()[5]),weight);
    Fill1DHisto("vtx_InvM_vc_h"   ,vtx->getInvMass(),weight);
    Fill1DHisto("vtx_InvMErr_Z_vc_h",vtx->getInvMassErr(),weight);
    Fill1DHisto("vtx_px_vc_h",vtx->getP().X());
    Fill1DHisto("vtx_py_vc_h",vtx->getP().Y());
    Fill1DHisto("vtx_pz_vc_h",vtx->getP().Z());
    Fill1DHisto("vtx_p_vc_h" ,vtx->getP().Mag());
    
    Fill1DHisto(posTag+"vtx_sigma_X_vc_h",sqrt(vtx->getCovariance()[0]),weight);
    Fill1DHisto(posTag+"vtx_sigma_Y_vc_h",sqrt(vtx->getCovariance()[3]),weight);
    Fill1DHisto(posTag+"vtx_sigma_Z_vc_h",sqrt(vtx->getCovariance()[5]),weight);
    Fill1DHisto(posTag+"vtx_InvM_vc_h"   ,vtx->getInvMass(),weight);
    Fill1DHisto(posTag+"vtx_InvMErr_Z_vc_h",vtx->getInvMassErr(),weight);
    Fill1DHisto(posTag+"vtx_px_vc_h",vtx->getP().X());
    Fill1DHisto(posTag+"vtx_py_vc_h",vtx->getP().Y());
    Fill1DHisto(posTag+"vtx_pz_vc_h",vtx->getP().Z());
    Fill1DHisto(posTag+"vtx_p_vc_h" ,vtx->getP().Mag());

    Fill1DHisto(layerCode+"vtx_sigma_X_vc_h",sqrt(vtx->getCovariance()[0]),weight);
    Fill1DHisto(layerCode+"vtx_sigma_Y_vc_h",sqrt(vtx->getCovariance()[3]),weight);
    Fill1DHisto(layerCode+"vtx_sigma_Z_vc_h",sqrt(vtx->getCovariance()[5]),weight);
    Fill1DHisto(layerCode+"vtx_InvM_vc_h"   ,vtx->getInvMass(),weight);
    Fill1DHisto(layerCode+"vtx_InvMErr_Z_vc_h",vtx->getInvMassErr(),weight);
    Fill1DHisto(layerCode+"vtx_px_vc_h",vtx->getP().X());
    Fill1DHisto(layerCode+"vtx_py_vc_h",vtx->getP().Y());
    Fill1DHisto(layerCode+"vtx_pz_vc_h",vtx->getP().Z());
    Fill1DHisto(layerCode+"vtx_p_vc_h" ,vtx->getP().Mag());
    
    Fill1DHisto(layerCode+posTag+"vtx_sigma_X_vc_h",sqrt(vtx->getCovariance()[0]),weight);
    Fill1DHisto(layerCode+posTag+"vtx_sigma_Y_vc_h",sqrt(vtx->getCovariance()[3]),weight);
    Fill1DHisto(layerCode+posTag+"vtx_sigma_Z_vc_h",sqrt(vtx->getCovariance()[5]),weight);
    Fill1DHisto(layerCode+posTag+"vtx_InvM_vc_h"   ,vtx->getInvMass(),weight);
    Fill1DHisto(layerCode+posTag+"vtx_InvMErr_Z_vc_h",vtx->getInvMassErr(),weight);
    Fill1DHisto(layerCode+posTag+"vtx_px_vc_h",vtx->getP().X());
    Fill1DHisto(layerCode+posTag+"vtx_py_vc_h",vtx->getP().Y());
    Fill1DHisto(layerCode+posTag+"vtx_pz_vc_h",vtx->getP().Z());
    Fill1DHisto(layerCode+posTag+"vtx_p_vc_h" ,vtx->getP().Mag());

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
    double thetay_v0_val   = TMath::ATan2(p_v0.Y(),p_v0.Z());
    
    double thetax_miss_val = TMath::ATan2(p_miss.X(),p_miss.Z());
    double thetay_miss_val = TMath::ATan2(p_miss.Y(),p_miss.Z());

    double pt_ele = p_ele.Pt();
    double pt_pos = p_pos.Pt();

    double pt_asym_val = (pt_ele-pt_pos) / (pt_ele+pt_pos);
    
    
    //Fill event information

    //1D histos
    Fill1DHisto("Esum_vc_h",ele->getEnergy() + pos->getEnergy(),weight);
    Fill1DHisto("Psum_vc_h",p_ele.P() + p_pos.P());
    Fill1DHisto("PtAsym_vc_h",pt_asym_val,weight);
    Fill1DHisto("Pmiss_vc_h",p_miss.P());
    Fill1DHisto("thetax_v0_vc_h",thetax_v0_val,weight);
    Fill1DHisto("thetay_v0_vc_h",thetay_v0_val,weight);
    Fill1DHisto("thetax_miss_vc_h",thetax_miss_val,weight);
    Fill1DHisto("thetay_miss_vc_h",thetay_miss_val,weight);


    //1D histos
    Fill1DHisto(posTag+"Esum_vc_h",ele->getEnergy() + pos->getEnergy(),weight);
    Fill1DHisto(posTag+"Psum_vc_h",p_ele.P() + p_pos.P());
    Fill1DHisto(posTag+"PtAsym_vc_h",pt_asym_val,weight);
    Fill1DHisto(posTag+"Pmiss_vc_h",p_miss.P());
    Fill1DHisto(posTag+"thetax_v0_vc_h",thetax_v0_val,weight);
    Fill1DHisto(posTag+"thetay_v0_vc_h",thetay_v0_val,weight);
    Fill1DHisto(posTag+"thetax_miss_vc_h",thetax_miss_val,weight);
    Fill1DHisto(posTag+"thetay_miss_vc_h",thetay_miss_val,weight);

  //1D histos
    Fill1DHisto(layerCode+"Esum_vc_h",ele->getEnergy() + pos->getEnergy(),weight);
    Fill1DHisto(layerCode+"Psum_vc_h",p_ele.P() + p_pos.P());
    Fill1DHisto(layerCode+"PtAsym_vc_h",pt_asym_val,weight);
    Fill1DHisto(layerCode+"Pmiss_vc_h",p_miss.P());
    Fill1DHisto(layerCode+"thetax_v0_vc_h",thetax_v0_val,weight);
    Fill1DHisto(layerCode+"thetay_v0_vc_h",thetay_v0_val,weight);
    Fill1DHisto(layerCode+"thetax_miss_vc_h",thetax_miss_val,weight);
    Fill1DHisto(layerCode+"thetay_miss_vc_h",thetay_miss_val,weight);


    //1D histos
    Fill1DHisto(layerCode+posTag+"Esum_vc_h",ele->getEnergy() + pos->getEnergy(),weight);
    Fill1DHisto(layerCode+posTag+"Psum_vc_h",p_ele.P() + p_pos.P());
    Fill1DHisto(layerCode+posTag+"PtAsym_vc_h",pt_asym_val,weight);
    Fill1DHisto(layerCode+posTag+"Pmiss_vc_h",p_miss.P());
    Fill1DHisto(layerCode+posTag+"thetax_v0_vc_h",thetax_v0_val,weight);
    Fill1DHisto(layerCode+posTag+"thetay_v0_vc_h",thetay_v0_val,weight);
    Fill1DHisto(layerCode+posTag+"thetax_miss_vc_h",thetax_miss_val,weight);
    Fill1DHisto(layerCode+posTag+"thetay_miss_vc_h",thetay_miss_val,weight);

    Fill2DHisto("vtx_InvM_vtx_z_vc_hh",vtx->getInvMass(),vtx->getZ(),weight);
    Fill2DHisto(posTag+"vtx_InvM_vtx_z_vc_hh",vtx->getInvMass(),vtx->getZ(),weight);
    Fill2DHisto(layerCode+"vtx_InvM_vtx_z_vc_hh",vtx->getInvMass(),vtx->getZ(),weight);
    Fill2DHisto(layerCode+posTag+"vtx_InvM_vtx_z_vc_hh",vtx->getInvMass(),vtx->getZ(),weight);

    //
    Fill2DHisto("ele_vtx_z_vs_z0_over_tanLambda_tc_hh",ele_trk->getZ0()/ele_trk->getTanLambda(), vtx->getZ());
    Fill2DHisto("pos_vtx_z_vs_z0_over_tanLambda_tc_hh",pos_trk->getZ0()/pos_trk->getTanLambda(), vtx->getZ());
    Fill2DHisto(eleTag+"vtx_z_vs_z0_over_tanLambda_tc_hh",ele_trk->getZ0()/ele_trk->getTanLambda(), vtx->getZ());
    Fill2DHisto(posTag+"vtx_z_vs_z0_over_tanLambda_tc_hh",pos_trk->getZ0()/pos_trk->getTanLambda(), vtx->getZ());
    if(fabs(vtx->getInvMass()-0.0925)<0.010){
      Fill2DHisto("ele_vtx_z_vs_z0_over_tanLambda_m_eq_92_tc_hh",ele_trk->getZ0()/ele_trk->getTanLambda(), vtx->getZ());
      Fill2DHisto("pos_vtx_z_vs_z0_over_tanLambda_m_eq_92_tc_hh",pos_trk->getZ0()/pos_trk->getTanLambda(), vtx->getZ());
      Fill2DHisto(eleTag+"vtx_z_vs_z0_over_tanLambda_m_eq_92_tc_hh",ele_trk->getZ0()/ele_trk->getTanLambda(), vtx->getZ());
      Fill2DHisto(posTag+"vtx_z_vs_z0_over_tanLambda_m_eq_92_tc_hh",pos_trk->getZ0()/pos_trk->getTanLambda(), vtx->getZ());
    }

}


void TridentHistos::Fill2DTrack(Track* track, float weight, const std::string& trkname) {
    
    
    if (track) {
      std::string half="top";
      
      if(track->getTanLambda()<0)
	half="bot";
      std::string tag=half+"_"+trkname;
      //      std::string layerCode="L"+getLayerCodeFromTrack(track)+"_";
      double d0 = track->getD0();
      double z0 = track->getZ0();
      Fill2DHisto(trkname+"tanlambda_vs_phi0_tc_hh",track->getPhi(),track->getTanLambda(), weight);
      Fill2DHisto(trkname+"d0_vs_p_tc_hh",track->getP(),d0,weight);
      Fill2DHisto(trkname+"d0_vs_phi0_tc_hh",track->getPhi(),d0,weight);
      Fill2DHisto(trkname+"d0_vs_tanlambda_tc_hh",track->getTanLambda(),d0,weight);
      
      Fill2DHisto(trkname+"z0_vs_p_tc_hh",track->getP(),z0,weight);
      Fill2DHisto(trkname+"z0_vs_phi0_tc_hh",track->getPhi(),z0,weight);
      Fill2DHisto(trkname+"z0_vs_tanlambda_tc_hh",track->getTanLambda(),z0,weight);

      Fill2DHisto(tag+"tanlambda_vs_phi0_tc_hh",track->getPhi(),track->getTanLambda(), weight);
      Fill2DHisto(tag+"d0_vs_p_tc_hh",track->getP(),d0,weight);
      Fill2DHisto(tag+"d0_vs_phi0_tc_hh",track->getPhi(),d0,weight);
      Fill2DHisto(tag+"d0_vs_tanlambda_tc_hh",track->getTanLambda(),d0,weight);
      
      Fill2DHisto(tag+"z0_vs_p_tc_hh",track->getP(),z0,weight);
      Fill2DHisto(tag+"z0_vs_phi0_tc_hh",track->getPhi(),z0,weight);
      Fill2DHisto(tag+"z0_vs_tanlambda_tc_hh",track->getTanLambda(),z0,weight);


      Fill2DHisto(layerCode+trkname+"tanlambda_vs_phi0_tc_hh",track->getPhi(),track->getTanLambda(), weight);
      Fill2DHisto(layerCode+trkname+"d0_vs_p_tc_hh",track->getP(),d0,weight);
      Fill2DHisto(layerCode+trkname+"d0_vs_phi0_tc_hh",track->getPhi(),d0,weight);
      Fill2DHisto(layerCode+trkname+"d0_vs_tanlambda_tc_hh",track->getTanLambda(),d0,weight);
      
      Fill2DHisto(layerCode+trkname+"z0_vs_p_tc_hh",track->getP(),z0,weight);
      Fill2DHisto(layerCode+trkname+"z0_vs_phi0_tc_hh",track->getPhi(),z0,weight);
      Fill2DHisto(layerCode+trkname+"z0_vs_tanlambda_tc_hh",track->getTanLambda(),z0,weight);

      Fill2DHisto(layerCode+tag+"tanlambda_vs_phi0_tc_hh",track->getPhi(),track->getTanLambda(), weight);
      Fill2DHisto(layerCode+tag+"d0_vs_p_tc_hh",track->getP(),d0,weight);
      Fill2DHisto(layerCode+tag+"d0_vs_phi0_tc_hh",track->getPhi(),d0,weight);
      Fill2DHisto(layerCode+tag+"d0_vs_tanlambda_tc_hh",track->getTanLambda(),d0,weight);
      
      Fill2DHisto(layerCode+tag+"z0_vs_p_tc_hh",track->getP(),z0,weight);
      Fill2DHisto(layerCode+tag+"z0_vs_phi0_tc_hh",track->getPhi(),z0,weight);
      Fill2DHisto(layerCode+tag+"z0_vs_tanlambda_tc_hh",track->getTanLambda(),z0,weight);
      
      
    }
}

void TridentHistos::Fill1DTrack(Track* track, double trkTimeOffset,float weight, const std::string& trkname) {
    
    double charge = (double) track->getCharge();

      //2D hits
    int n_hits_2d = track->getTrackerHitCount();

    TVector3 p_trk;
    p_trk.SetXYZ(track->getMomentum()[0],track->getMomentum()[1],track->getMomentum()[2]);
    std::string half="top";

    if(track->getTanLambda()<0)
      half="bot";
    
    std::string tag=half+"_"+trkname;

    //    std::string layerCode="L"+getLayerCodeFromTrack(track)+"_"+tag;
    //    std::string layerCode="L"+getLayerCodeFromTrack(track)+"_";

    Fill1DHisto(trkname+"p_tc_h",p_trk.Mag(),weight);
    Fill1DHisto(trkname+"d0_tc_h"       ,track->getD0()          ,weight);
    Fill1DHisto(trkname+"Phi_tc_h"      ,track->getPhi()         ,weight);
    Fill1DHisto(trkname+"Omega_tc_h"    ,track->getOmega()       ,weight);
    Fill1DHisto(trkname+"pT_tc_h"       ,charge*track->getPt()   ,weight);
    Fill1DHisto(trkname+"invpT_tc_h"    ,charge/track->getPt()   ,weight);
    Fill1DHisto(trkname+"TanLambda_tc_h",track->getTanLambda()   ,weight);
    Fill1DHisto(trkname+"Z0_tc_h"       ,track->getZ0()          ,weight);
    Fill1DHisto(trkname+"Z0_over_TanLambda_tc_h",track->getZ0()/track->getTanLambda()   ,weight);
    Fill1DHisto(trkname+"time_tc_h"     ,track->getTrackTime()-trkTimeOffset   ,weight);
    Fill1DHisto(trkname+"chi2_tc_h"     ,track->getChi2()        ,weight);
    Fill1DHisto(trkname+"chi2ndf_tc_h"  ,track->getChi2Ndf()     ,weight);
    Fill1DHisto(trkname+"nShared_tc_h"  ,track->getNShared()     ,weight);
    Fill1DHisto(trkname+"nHits_2d_tc_h" ,n_hits_2d             ,weight);


    Fill1DHisto(tag+"p_tc_h",p_trk.Mag(),weight);
    Fill1DHisto(tag+"d0_tc_h"       ,track->getD0()          ,weight);
    Fill1DHisto(tag+"Phi_tc_h"      ,track->getPhi()         ,weight);
    Fill1DHisto(tag+"Omega_tc_h"    ,track->getOmega()       ,weight);
    Fill1DHisto(tag+"pT_tc_h"       ,-1*charge*track->getPt()   ,weight);
    Fill1DHisto(tag+"invpT_tc_h"    ,-1*charge/track->getPt()   ,weight);
    Fill1DHisto(tag+"TanLambda_tc_h",track->getTanLambda()   ,weight);
    Fill1DHisto(tag+"Z0_tc_h"       ,track->getZ0()          ,weight);
    Fill1DHisto(tag+"Z0_over_TanLambda_tc_h",track->getZ0()/track->getTanLambda()   ,weight);
    Fill1DHisto(tag+"time_tc_h"     ,track->getTrackTime()-trkTimeOffset   ,weight);
    Fill1DHisto(tag+"chi2_tc_h"     ,track->getChi2()        ,weight);
    Fill1DHisto(tag+"chi2ndf_tc_h"  ,track->getChi2Ndf()     ,weight);
    Fill1DHisto(tag+"nShared_tc_h"  ,track->getNShared()     ,weight);
    Fill1DHisto(tag+"nHits_2d_tc_h" ,n_hits_2d               ,weight);
    
    Fill1DHisto(layerCode+trkname+"p_tc_h",p_trk.Mag(),weight);
    Fill1DHisto(layerCode+trkname+"d0_tc_h"       ,track->getD0()          ,weight);
    Fill1DHisto(layerCode+trkname+"Phi_tc_h"      ,track->getPhi()         ,weight);
    Fill1DHisto(layerCode+trkname+"Omega_tc_h"    ,track->getOmega()       ,weight);
    Fill1DHisto(layerCode+trkname+"pT_tc_h"       ,charge*track->getPt()   ,weight);
    Fill1DHisto(layerCode+trkname+"invpT_tc_h"    ,charge/track->getPt()   ,weight);
    Fill1DHisto(layerCode+trkname+"TanLambda_tc_h",track->getTanLambda()   ,weight);
    Fill1DHisto(layerCode+trkname+"Z0_tc_h"       ,track->getZ0()          ,weight);
    Fill1DHisto(layerCode+trkname+"Z0_over_TanLambda_tc_h",track->getZ0()/track->getTanLambda()   ,weight);
    Fill1DHisto(layerCode+trkname+"time_tc_h"     ,track->getTrackTime()-trkTimeOffset   ,weight);
    Fill1DHisto(layerCode+trkname+"chi2_tc_h"     ,track->getChi2()        ,weight);
    Fill1DHisto(layerCode+trkname+"chi2ndf_tc_h"  ,track->getChi2Ndf()     ,weight);
    Fill1DHisto(layerCode+trkname+"nShared_tc_h"  ,track->getNShared()     ,weight);
    Fill1DHisto(layerCode+trkname+"nHits_2d_tc_h" ,n_hits_2d             ,weight);


    Fill1DHisto(layerCode+tag+"p_tc_h",p_trk.Mag(),weight);
    Fill1DHisto(layerCode+tag+"d0_tc_h"       ,track->getD0()          ,weight);
    Fill1DHisto(layerCode+tag+"Phi_tc_h"      ,track->getPhi()         ,weight);
    Fill1DHisto(layerCode+tag+"Omega_tc_h"    ,track->getOmega()       ,weight);
    Fill1DHisto(layerCode+tag+"pT_tc_h"       ,-1*charge*track->getPt()   ,weight);
    Fill1DHisto(layerCode+tag+"invpT_tc_h"    ,-1*charge/track->getPt()   ,weight);
    Fill1DHisto(layerCode+tag+"TanLambda_tc_h",track->getTanLambda()   ,weight);
    Fill1DHisto(layerCode+tag+"Z0_tc_h"       ,track->getZ0()          ,weight);
    Fill1DHisto(layerCode+tag+"Z0_over_TanLambda_tc_h",track->getZ0()/track->getTanLambda()   ,weight);
    Fill1DHisto(layerCode+tag+"time_tc_h"     ,track->getTrackTime()-trkTimeOffset   ,weight);
    Fill1DHisto(layerCode+tag+"chi2_tc_h"     ,track->getChi2()        ,weight);
    Fill1DHisto(layerCode+tag+"chi2ndf_tc_h"  ,track->getChi2Ndf()     ,weight);
    Fill1DHisto(layerCode+tag+"nShared_tc_h"  ,track->getNShared()     ,weight);
    Fill1DHisto(layerCode+tag+"nHits_2d_tc_h" ,n_hits_2d               ,weight);
  

	
    /*
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
    */

    bool hasLayer4=false;
    for (auto & layer : track->getHitLayers()) {
      if(layer==4){
        if(hasLayer4)
          std::cout<<"What...I already counted layer 4!"<<std::endl;
        else
          hasLayer4=true;
      }
      Fill1DHisto(trkname+"layersHit_tc_h",layer,weight);
      Fill1DHisto(tag+"layersHit_tc_h",layer,weight);
      Fill1DHisto(layerCode+trkname+"layersHit_tc_h",layer,weight);
      Fill1DHisto(layerCode+tag+"layersHit_tc_h",layer,weight);      
    }     
   
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
  /*
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

        //Fill2DHisto("vtx_p_svt_z_hh",vtxP,vtxPosSvt.Z(),weight);
	// Fill2DHisto("vtx_p_svt_x_hh",vtxP,vtxPosSvt.X(),weight);
        //Fill2DHisto("vtx_p_svt_y_hh",vtxP,vtxPosSvt.Y(),weight);
        
        //Fill2DHisto("vtx_p_sigmaZ_hh",vtxP,vtx->getCovariance()[5],weight);
	// Fill2DHisto("vtx_p_sigmaX_hh",vtxP,vtx->getCovariance()[3],weight);
        //Fill2DHisto("vtx_p_sigmaY_hh",vtxP,vtx->getCovariance()[0],weight);
    }
  */
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
//void TridentHistos::FillTrackClusterHistos(std::pair<CalCluster, Track*> ele, std::pair<CalCluster, Track*> posOrGamma, double calTimeOffset, double trkTimeOffset,std::vector<CalCluster*>  * clusterList, double weight){
void TridentHistos::FillTrackClusterHistos(std::pair<CalCluster, Track*> ele, std::pair<CalCluster, Track*> posOrGamma, double calTimeOffset, double trkTimeOffset, double weight){
  CalCluster eleClu=ele.first;
  Track* eleTrk=ele.second;
  CalCluster posClu=posOrGamma.first; 
  Track* posTrk=posOrGamma.second; //these "positrons" may be gammas
  
  std::string half="top";  
  if(eleTrk->getTanLambda()<0)
    half="bot";
  std::string eleTag=half+"_ele_";

  half="top";
  if(posTrk->getTanLambda()<0)
    half="bot";
  std::string posTag=half+"_pos_";

  //  this does not work, but maybe in new tuple???
    //std::string eleLayerCode="L"+getLayerCodeFromTrack(&eleTrk)+"_";
    //std::string posLayerCode="L"+getLayerCodeFromTrack(&posTrk)+"_";
    

  double ele_cluTime=eleClu.getTime()-calTimeOffset;
  double pos_cluTime=posClu.getTime()-calTimeOffset;
  double ele_trkTime=eleTrk->getTrackTime()-trkTimeOffset;
  double pos_trkTime=posTrk->getTrackTime()-trkTimeOffset;


  double ele_cluX=eleClu.getPosition().at(0);
  double ele_cluY=eleClu.getPosition().at(1);
  double pos_cluX=posClu.getPosition().at(0);
  double pos_cluY=posClu.getPosition().at(1);

  ////
  double ele_trkX=eleTrk->getPositionAtEcal().at(0);
  double ele_trkY=eleTrk->getPositionAtEcal().at(1);
  double pos_trkX=posTrk->getPositionAtEcal().at(0);
  double pos_trkY=posTrk->getPositionAtEcal().at(1);
  ////

  double ele_clu_trk_deltaX=ele_cluX-ele_trkX;
  double pos_clu_trk_deltaX=pos_cluX-pos_trkX;

  double ele_clu_trk_deltaY=ele_cluY-ele_trkY;
  double pos_clu_trk_deltaY=pos_cluY-pos_trkY;

  //  if(eleClu.getTime()>-300){
  Fill1DHisto("ele_cl_time_tc_h", eleClu.getTime()-calTimeOffset,weight);
  Fill1DHisto("ele_cl_ene_tc_h",eleClu.getEnergy(),weight);
  Fill1DHisto("ele_clu_trk_deltaX_tc_h",ele_clu_trk_deltaX,weight);
  Fill1DHisto("ele_clu_trk_deltaY_tc_h",ele_clu_trk_deltaY,weight);

  
  Fill1DHisto(layerCode+"ele_cl_time_tc_h", eleClu.getTime()-calTimeOffset,weight);
  Fill1DHisto(layerCode+"ele_cl_ene_tc_h",eleClu.getEnergy(),weight);
  Fill1DHisto(layerCode+"ele_clu_trk_deltaX_tc_h",ele_clu_trk_deltaX,weight);
  Fill1DHisto(layerCode+"ele_clu_trk_deltaY_tc_h",ele_clu_trk_deltaY,weight);

  Fill2DHisto("ele_clu_trk_deltaX_vs_cluX_hc_hh",ele_cluX,ele_clu_trk_deltaX,weight);
  Fill2DHisto("ele_clu_trk_deltaY_vs_cluX_hc_hh",ele_cluX,ele_clu_trk_deltaY,weight);
  Fill2DHisto("ele_cluY_vs_cluX_hc_hh",ele_cluX,ele_cluY,weight);

  Fill1DHisto(eleTag+"cl_time_tc_h", eleClu.getTime()-calTimeOffset,weight);
  Fill1DHisto(eleTag+"cl_ene_tc_h",eleClu.getEnergy(),weight);
  Fill1DHisto(eleTag+"clu_trk_deltaX_tc_h",ele_clu_trk_deltaX,weight);
  Fill1DHisto(eleTag+"clu_trk_deltaY_tc_h",ele_clu_trk_deltaY,weight);
  
  Fill1DHisto(layerCode+eleTag+"cl_time_tc_h", eleClu.getTime()-calTimeOffset,weight);
  Fill1DHisto(layerCode+eleTag+"cl_ene_tc_h",eleClu.getEnergy(),weight);
  Fill1DHisto(layerCode+eleTag+"clu_trk_deltaX_tc_h",ele_clu_trk_deltaX,weight);
  Fill1DHisto(layerCode+eleTag+"clu_trk_deltaY_tc_h",ele_clu_trk_deltaY,weight);
  
  Fill2DHisto(eleTag+"clu_trk_deltaX_vs_cluX_hc_hh",ele_cluX,ele_clu_trk_deltaX,weight);
  Fill2DHisto(eleTag+"clu_trk_deltaY_vs_cluX_hc_hh",ele_cluX,ele_clu_trk_deltaY,weight);
  Fill2DHisto(eleTag+"cluY_vs_cluX_hc_hh",ele_cluX,ele_cluY,weight);

  double pele=sqrt(eleTrk->getMomentum()[0]*eleTrk->getMomentum()[0]+
                   eleTrk->getMomentum()[1]*eleTrk->getMomentum()[1]+
                   eleTrk->getMomentum()[2]*eleTrk->getMomentum()[2]);
  Fill1DHisto("ele_cltrk_time_diff_tc_h", eleClu.getTime()-calTimeOffset-eleTrk->getTrackTime()+trkTimeOffset,weight);
  Fill1DHisto("ele_EOverp_tc_h",eleClu.getEnergy()/pele,weight);
  Fill2DHisto("ele_EOverp_vs_cluX_hc_hh",ele_cluX,eleClu.getEnergy()/pele,weight);
  //  if(eleClu.getEnergy()/pele<=0.02){
  //  std::cout<<"ele E/p<=0.02:  "<<eleClu.getEnergy()<<"   "<<pele<<std::endl;
  //  std::cout<<"electron momentum::  x = "<<eleTrk->getMomentum()[0]<<"  y = "<<eleTrk->getMomentum()[1]<<"  z = "<<eleTrk->getMomentum()[2]<<std::endl;
  //  std::cout<<"cluster time = "<<ele_cluTime<<"  track time = "<<ele_trkTime<<std::endl;
  //}
   
  Fill1DHisto(layerCode+"ele_cltrk_time_diff_tc_h", eleClu.getTime()-calTimeOffset-eleTrk->getTrackTime()+trkTimeOffset,weight);
  Fill1DHisto(layerCode+"ele_EOverp_tc_h",eleClu.getEnergy()/pele,weight);
  
  Fill1DHisto(eleTag+"cltrk_time_diff_tc_h", eleClu.getTime()-calTimeOffset-eleTrk->getTrackTime()+trkTimeOffset,weight);
  Fill1DHisto(eleTag+"EOverp_tc_h",eleClu.getEnergy()/pele,weight);
  Fill2DHisto(eleTag+"EOverp_vs_cluX_hc_hh",ele_cluX,eleClu.getEnergy()/pele,weight);
  
  Fill1DHisto(layerCode+eleTag+"cltrk_time_diff_tc_h", eleClu.getTime()-calTimeOffset-eleTrk->getTrackTime()+trkTimeOffset,weight);
  Fill1DHisto(layerCode+eleTag+"EOverp_tc_h",eleClu.getEnergy()/pele,weight);
  
  Fill1DHisto("pos_cl_time_tc_h", posClu.getTime()-calTimeOffset,weight);
  Fill1DHisto("pos_cl_ene_tc_h",posClu.getEnergy(),weight);
  Fill1DHisto("pos_clu_trk_deltaX_tc_h",pos_clu_trk_deltaX,weight);
  Fill1DHisto("pos_clu_trk_deltaY_tc_h",pos_clu_trk_deltaY,weight);
  Fill1DHisto("pos_cltrk_time_diff_tc_h", posClu.getTime()-calTimeOffset-posTrk->getTrackTime()+trkTimeOffset,weight);

  Fill1DHisto(posTag+"cl_time_tc_h", posClu.getTime()-calTimeOffset,weight);
  Fill1DHisto(posTag+"cl_ene_tc_h",posClu.getEnergy(),weight);
  Fill1DHisto(posTag+"clu_trk_deltaX_tc_h",pos_clu_trk_deltaX,weight);
  Fill1DHisto(posTag+"clu_trk_deltaY_tc_h",pos_clu_trk_deltaY,weight);
  Fill1DHisto(posTag+"cltrk_time_diff_tc_h", posClu.getTime()-calTimeOffset-posTrk->getTrackTime()+trkTimeOffset,weight);
  
  Fill1DHisto(layerCode+"pos_cl_time_tc_h", posClu.getTime()-calTimeOffset,weight);
  Fill1DHisto(layerCode+"pos_cl_ene_tc_h",posClu.getEnergy(),weight);
  Fill1DHisto(layerCode+"pos_clu_trk_deltaX_tc_h",pos_clu_trk_deltaX,weight);
  Fill1DHisto(layerCode+"pos_clu_trk_deltaY_tc_h",pos_clu_trk_deltaY,weight);
  Fill1DHisto(layerCode+"pos_cltrk_time_diff_tc_h", posClu.getTime()-calTimeOffset-posTrk->getTrackTime()+trkTimeOffset,weight);

  Fill1DHisto(layerCode+posTag+"cl_time_tc_h", posClu.getTime()-calTimeOffset,weight);
  Fill1DHisto(layerCode+posTag+"cl_ene_tc_h",posClu.getEnergy(),weight);
  Fill1DHisto(layerCode+posTag+"clu_trk_deltaX_tc_h",pos_clu_trk_deltaX,weight);
  Fill1DHisto(layerCode+posTag+"clu_trk_deltaY_tc_h",pos_clu_trk_deltaY,weight);
  Fill1DHisto(layerCode+posTag+"cltrk_time_diff_tc_h", posClu.getTime()-calTimeOffset-posTrk->getTrackTime()+trkTimeOffset,weight);
 
  double ppos=sqrt(posTrk->getMomentum()[0]*posTrk->getMomentum()[0]+
		     posTrk->getMomentum()[1]*posTrk->getMomentum()[1]+
		     posTrk->getMomentum()[2]*posTrk->getMomentum()[2]);
  Fill1DHisto("pos_EOverp_tc_h",posClu.getEnergy()/ppos,weight);
  Fill1DHisto(layerCode+"pos_EOverp_tc_h",posClu.getEnergy()/ppos,weight);
  Fill2DHisto("pos_EOverp_vs_cluX_hc_hh",pos_cluX,posClu.getEnergy()/ppos,weight);
  Fill2DHisto("pos_cluY_vs_cluX_hc_hh",pos_cluX,pos_cluY,weight);
  Fill2DHisto("pos_clu_trk_deltaX_vs_cluX_hc_hh",pos_cluX,pos_clu_trk_deltaX,weight);
  Fill2DHisto("pos_clu_trk_deltaY_vs_cluX_hc_hh",pos_cluX,pos_clu_trk_deltaY,weight);

  Fill1DHisto(posTag+"EOverp_tc_h",posClu.getEnergy()/ppos,weight);
  Fill1DHisto(layerCode+posTag+"EOverp_tc_h",posClu.getEnergy()/ppos,weight);
  Fill2DHisto(posTag+"EOverp_vs_cluX_hc_hh",pos_cluX,posClu.getEnergy()/ppos,weight);
  Fill2DHisto(posTag+"cluY_vs_cluX_hc_hh",pos_cluX,pos_cluY,weight);
  Fill2DHisto(posTag+"clu_trk_deltaX_vs_cluX_hc_hh",pos_cluX,pos_clu_trk_deltaX,weight);
  Fill2DHisto(posTag+"clu_trk_deltaY_vs_cluX_hc_hh",pos_cluX,pos_clu_trk_deltaY,weight);
  //find the closest cluster that makes sense
  /*
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
  */
  //////   do new electron cluster stuff
  /*
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
    //    if(clTime!= ele_cluTime){
    if(clTime!= ele_cluTime && !foundOgCluster){ //only plot if there was no match
      //found a different cluster...plot new parameters
      double new_deltaX=clX-ele_trkX; 
      double new_deltaY=clY-ele_trkY; 
      Fill1DHisto("ele_new_cl_time_tc_h",clTime,weight);
      Fill1DHisto("ele_new_cl_ene_tc_h",clE,weight);
      Fill1DHisto("ele_new_clu_trk_deltaX_tc_h",new_deltaX,weight);
      Fill1DHisto("ele_new_clu_trk_deltaY_tc_h",new_deltaY,weight);
      Fill1DHisto("ele_new_EOverp_tc_h",clE/pele,weight);
  */
      /*
      Fill1DHisto(layerCode+"ele_new_cl_time_tc_h",clTime,weight);
      Fill1DHisto(layerCode+"ele_new_cl_ene_tc_h",clE,weight);
      Fill1DHisto(layerCode+"ele_new_clu_trk_deltaX_tc_h",new_deltaX,weight);
      Fill1DHisto(layerCode+"ele_new_clu_trk_deltaY_tc_h",new_deltaY,weight);
      Fill1DHisto(layerCode+"ele_new_EOverp_tc_h",clE/pele,weight);
      */
  /*
      Fill2DHisto("ele_new_clu_trk_deltaX_vs_cluX_hc_hh",clX,new_deltaX,weight);
      Fill2DHisto("ele_new_clu_trk_deltaY_vs_cluX_hc_hh",clX,new_deltaY,weight);      
      Fill2DHisto("ele_new_EOverp_vs_cluX_hc_hh",clX,clE/pele,weight);
      Fill2DHisto("ele_new_cluY_vs_cluX_hc_hh",clX,clY,weight);

      Fill1DHisto(eleTag+"new_cl_time_tc_h",clTime,weight);
      Fill1DHisto(eleTag+"new_cl_ene_tc_h",clE,weight);
      Fill1DHisto(eleTag+"new_clu_trk_deltaX_tc_h",new_deltaX,weight);
      Fill1DHisto(eleTag+"new_clu_trk_deltaY_tc_h",new_deltaY,weight);
      Fill1DHisto(eleTag+"new_EOverp_tc_h",clE/pele,weight);
  */
      /*
      Fill1DHisto(layerCode+eleTag+"new_cl_time_tc_h",clTime,weight);
      Fill1DHisto(layerCode+eleTag+"new_cl_ene_tc_h",clE,weight);
      Fill1DHisto(layerCode+eleTag+"new_clu_trk_deltaX_tc_h",new_deltaX,weight);
      Fill1DHisto(layerCode+eleTag+"new_clu_trk_deltaY_tc_h",new_deltaY,weight);
      Fill1DHisto(layerCode+eleTag+"new_EOverp_tc_h",clE/pele,weight);
      */
  /*
      Fill2DHisto(eleTag+"new_clu_trk_deltaX_vs_cluX_hc_hh",clX,new_deltaX,weight);
      Fill2DHisto(eleTag+"new_clu_trk_deltaY_vs_cluX_hc_hh",clX,new_deltaY,weight);      
      Fill2DHisto(eleTag+"new_EOverp_vs_cluX_hc_hh",clX,clE/pele,weight);
      Fill2DHisto(eleTag+"new_cluY_vs_cluX_hc_hh",clX,clY,weight);
      

    }else{
      foundSameCluster=true;
    }    
  } 
  */
  /*
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
  Fill1DHisto("ele_new_cluster_match_code_tc_h",clMatchCode,weight);
  Fill1DHisto(eleTag+"new_cluster_match_code_tc_h",clMatchCode,weight);
 
  Fill1DHisto(layerCode+"ele_new_cluster_match_code_tc_h",clMatchCode,weight);
  Fill1DHisto(layerCode+eleTag+"new_cluster_match_code_tc_h",clMatchCode,weight);
  
  
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
    //    if(clTime!= pos_cluTime){
    if(clTime!= pos_cluTime&& !foundOgCluster){ //only plot if there was no match
      //found a different cluster...plot new parameters
      double new_deltaX=clX-pos_trkX; 
      double new_deltaY=clY-pos_trkY; 
      Fill1DHisto("pos_new_cl_time_tc_h",clTime,weight);
      Fill1DHisto("pos_new_cl_ene_tc_h",clE,weight);
      Fill1DHisto("pos_new_clu_trk_deltaX_tc_h",new_deltaX,weight);
      Fill1DHisto("pos_new_clu_trk_deltaY_tc_h",new_deltaY,weight);
      Fill1DHisto("pos_new_EOverp_tc_h",clE/ppos,weight);
     
      Fill1DHisto(layerCode+"pos_new_cl_time_tc_h",clTime,weight);
      Fill1DHisto(layerCode+"pos_new_cl_ene_tc_h",clE,weight);
      Fill1DHisto(layerCode+"pos_new_clu_trk_deltaX_tc_h",new_deltaX,weight);
      Fill1DHisto(layerCode+"pos_new_clu_trk_deltaY_tc_h",new_deltaY,weight);
      Fill1DHisto(layerCode+"pos_new_EOverp_tc_h",clE/ppos,weight);
    
      Fill2DHisto("pos_new_clu_trk_deltaX_vs_cluX_hc_hh",clX,new_deltaX,weight);
      Fill2DHisto("pos_new_clu_trk_deltaY_vs_cluX_hc_hh",clX,new_deltaY,weight);
      Fill2DHisto("pos_new_EOverp_vs_cluX_hc_hh",clX,clE/ppos,weight);
      Fill2DHisto("pos_new_cluY_vs_cluX_hc_hh",clX,clY,weight);

      Fill1DHisto(posTag+"new_cl_time_tc_h",clTime,weight);
      Fill1DHisto(posTag+"new_cl_ene_tc_h",clE,weight);
      Fill1DHisto(posTag+"new_clu_trk_deltaX_tc_h",new_deltaX,weight);
      Fill1DHisto(posTag+"new_clu_trk_deltaY_tc_h",new_deltaY,weight);
      Fill1DHisto(posTag+"new_EOverp_tc_h",clE/ppos,weight);
      
	Fill1DHisto(layerCode+posTag+"new_cl_time_tc_h",clTime,weight);
      Fill1DHisto(layerCode+posTag+"new_cl_ene_tc_h",clE,weight);
      Fill1DHisto(layerCode+posTag+"new_clu_trk_deltaX_tc_h",new_deltaX,weight);
      Fill1DHisto(layerCode+posTag+"new_clu_trk_deltaY_tc_h",new_deltaY,weight);
      Fill1DHisto(layerCode+posTag+"new_EOverp_tc_h",clE/ppos,weight);
      
      Fill2DHisto(posTag+"new_clu_trk_deltaX_vs_cluX_hc_hh",clX,new_deltaX,weight);
      Fill2DHisto(posTag+"new_clu_trk_deltaY_vs_cluX_hc_hh",clX,new_deltaY,weight);
      Fill2DHisto(posTag+"new_EOverp_vs_cluX_hc_hh",clX,clE/ppos,weight);
      Fill2DHisto(posTag+"new_cluY_vs_cluX_hc_hh",clX,clY,weight);

    }else{
      foundSameCluster=true;
    }    
  } 
  */
  /*
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
  Fill1DHisto("pos_new_cluster_match_code_tc_h",clMatchCode,weight);
  Fill1DHisto(posTag+"new_cluster_match_code_tc_h",clMatchCode,weight);
  //  Fill1DHisto(layerCode+"pos_new_cluster_match_code_tc_h",clMatchCode,weight);
  //  Fill1DHisto(layerCode+posTag+"new_cluster_match_code_tc_h",clMatchCode,weight);
  */
  Fill1DHisto("cl_time_diff_vc_h", posClu.getTime()-eleClu.getTime(),weight);    
  Fill1DHisto("trk_time_diff_vc_h", posTrk->getTrackTime()-eleTrk->getTrackTime(),weight);    

  Fill1DHisto(posTag+"cl_time_diff_vc_h", posClu.getTime()-eleClu.getTime(),weight);    
  Fill1DHisto(posTag+"trk_time_diff_vc_h", posTrk->getTrackTime()-eleTrk->getTrackTime(),weight);    


  //  Fill1DHisto("gamma_cl_time_h", posClu.getTime()-calTimeOffset,weight);
  //Fill1DHisto("gamma_cl_ene_h",posClu.getEnergy(),weight);
  
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

void TridentHistos::Fill1DVertex(Vertex* vtx, float weight){
  //kept for now...
}


/*
 *  overwrite the saveHistos routine to make layer-based directories....
 */

void TridentHistos::saveHistos(TFile* outF,std::string folder) {
    if (outF) outF->cd();
    TDirectory* dir{nullptr};
    if (!folder.empty()) {
        dir = outF->mkdir(folder.c_str());
        dir->cd();
    }
    
    std::regex pat;
  
    for (it3d it = histos3d.begin(); it!=histos3d.end(); ++it) {
        if (!it->second){
            continue;
        }
	setOutputDir(outF,folder,it->first);      
        it->second->Write();
    }
    for (it2d it = histos2d.begin(); it!=histos2d.end(); ++it) {
        if (!(it->second)) {
            continue;
        }
	setOutputDir(outF,folder,it->first);
        it->second->Write();
    }

    for (it1d it = histos1d.begin(); it!=histos1d.end(); ++it) {
      if (!it->second){
	continue;
      }   
      setOutputDir(outF,folder,it->first);
      it->second->Write();
    }

    //dir->Write();
    //if (dir) {delete dir; dir=0;}

    Clear();

}

/*
 *  Figure out if we want this histo in a layer-seperated directory, find layer code, 
 *  make directory if needed, an cd to appropriate dir
 */
void   TridentHistos::setOutputDir(TFile* outF, std::string folder, std::string histoName){
  std::string fullDirPath=folder;
  std::string layerCode="";
  //first, check if this histo was split by both ele/pos layers (e.g. for V0 quantities)
  std::string dualLayerCode=splitByElePosLayerCombos(histoName);
  if(dualLayerCode.length()>0)
    layerCode=dualLayerCode;
  else
    layerCode=getLayerCodeFromHistoName(histoName);      
  if(layerCode.length()>0){
    fullDirPath=folder+"/"+layerCode;
    if(outF->GetDirectory(fullDirPath.c_str())==0){
      TDirectory* ldir{nullptr};
      ldir=outF->mkdir(std::string(fullDirPath).c_str());
    }
  }
  outF->GetDirectory(fullDirPath.c_str())->cd();
}


/*
 *check if the histo name has a layer code in it; if it does, return the layer code
 *else return empty string
 *....the easy way to do this is to use regex but it's broken for GCC 4.8.5??
 */

std::string TridentHistos::getLayerCodeFromHistoName(std::string histoName){
  std::string layerCode="";
  int nLayers=4;
  int padding=1;//the "L" 
  for (int i=0; i<histoName.length()-(padding+nLayers); i++)
    if(isLayerCode(histoName, nLayers,i)){
      layerCode=histoName.substr(i,padding+nLayers);
      break;
    }
  
  return layerCode;      
}

bool TridentHistos::isLayerCode(std::string histoName, int nLayers, int ptr ){
  if(histoName.substr(ptr,1)!="L")//my code starts with capital L
    return false;
  for(int i=0; i<nLayers;i++){
    if(! (histoName.substr(ptr+i+1,1)=="0" ||histoName.substr(ptr+i+1,1)=="1"))
      return false;    
  }
  //if we make it this far, it must be true
  return true;

}


std::string TridentHistos::getLayerCodeFromTrack(Track* trk){
  std::string s1="0";
  std::string s2="0"; 
  std::string s3="0"; 
  std::string s4="0"; 
  for (auto & layer: trk->getHitLayers()) {
    //    TrackerHit* hit = (TrackerHit*) trk->getSvtHits().At(ihit);
    //RawSvtHit* rhit=(RawSvtHit*)(hit->getRawHits()).At(0);
    //int layer=rhit->getLayer();
    //    if(layer == 0 ){
    //std::cout<<"I didn't think you could have layer 0???"<<std::endl;
    //}
    if (layer == 0 ) {
      s1="1";
    }
    if (layer == 1) {
      s2="1";
    }
    if (layer == 2) {
      s3="1";
    }
    if (layer == 3) {
      s4="1";
    }
  }
  
  return std::string(s1+s2+s3+s4);

}

/*
 *   look for the electron/positron layer codes for splitting V0-related quantities
 *   code should look like  posXXXX_eleXXXX
 */
std::string  TridentHistos::splitByElePosLayerCombos(std::string histoName){
  std::string layerCode="";
  int nLayers=4;
  int padding=3;//the "ele" or "pos" 
  std::string firstSt="pos"; 
  std::string secSt="ele"; 
  bool foundFirst=false;
  for (int i=0; i<histoName.length()-(padding+nLayers); i++){
    foundFirst=false;
    layerCode="";
    if(histoName.substr(i,padding)==firstSt && (histoName.substr(i+padding,1)=="0"||histoName.substr(i+padding,1)=="1")){
      foundFirst=true; 
      layerCode=histoName.substr(i,padding+nLayers);
    }
    if(foundFirst){
      if(histoName.substr(i+padding+nLayers+1,padding)==secSt && (histoName.substr(i+2*padding+nLayers+1,1)=="0"||histoName.substr(i+2*padding+nLayers+1,1)=="1")){
	layerCode+="_"+histoName.substr(i+padding+nLayers+1,padding+nLayers);
	return layerCode;
      }
    }
  }
  return layerCode;
  
}

/*************************************************
*     
*   Makes histograms based on template in json file
*   like DefineHistos but _only_ makes the histos
*   that are split by the makeCopyJsonTag.
*
*************************************************/

void TridentHistos::DefineHistosFromTemplateOnly(std::vector<std::string> histoCopyNames, std::string makeCopyJsonTag){
    if (debug_ ) std::cout << "[HistoManager] DefineHistosFromTemplateOnly" << std::endl;
    std::string h_name = "";
    if (debug_ ){    
        for (auto hist : _h_configs.items()){
            std::cout<<hist.key()<<std::endl;
        }
    }
    for (auto hist : _h_configs.items()) {
        bool singleCopy = true;
        for(int i = 0; i < histoCopyNames.size(); i++){
            h_name = m_name+"_" + hist.key() ;	   
	    std::size_t found = (hist.key()).find_last_of("_");
	    std::string extension = hist.key().substr(found+1);
            if (histoCopyNames.size() > 1 && std::string(hist.key()).find(makeCopyJsonTag) != std::string::npos){
                h_name = m_name+"_"+ histoCopyNames.at(i) + "_" + hist.key() ;
		if (debug_ )std::cout <<"DefineHistosFromTemplateOnly:  base hist name:  "<<m_name<<"_"<< hist.key()<<"  copy tag:  "<<makeCopyJsonTag<<"  hist copy list size " << histoCopyNames.size() << std::endl;
		if (debug_ )std::cout<<"new name = "<<h_name<<std::endl;

		if(histoCopyNames.at(i)=="")
		  h_name = m_name+"_"+ hist.key();//this lets you use and empty string and not screw up spacing
                singleCopy = false;
            }          
	    ////this just moves the loop break to earlier ... could just add switch to "Define Histos"
            if(singleCopy)
                break;
	    
            if(debug_){
                std::cout << "DefineHistosFromTemplateOnly: " << h_name << std::endl;  
                std::cout << extension << hist.value().at("xtitle") << std::endl;
            }
            if (extension == "h") {
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
                    }//bins
                }//labels
            }//1D histo

            else if (extension == "hh") {
                histos2d[h_name] = plot2D(h_name,
                        hist.value().at("xtitle"),hist.value().at("binsX"),hist.value().at("minX"),hist.value().at("maxX"),
                        hist.value().at("ytitle"),hist.value().at("binsY"),hist.value().at("minY"),hist.value().at("maxY"));
            }


        }
    }//loop on config
    if(debug_)std::cout<<"DefineHistosFromTemplateOnly:  Done with copies"<<std::endl;
}

/*
 *    call this after you've made all of the split histograms via DefineHistosFromTemplateOnly
 *    to make the histograms with no split
 */
void TridentHistos::DefineOneTimeHistos(){
    if(debug_)std::cout << "[HistoManager] DefineOneTimeHistos" << std::endl;
    std::string h_name = "";
   
    for (auto hist : _h_configs.items()) {

	h_name =hist.key() ;
	bool foundMatch=false;
	//check 1d histo list
	for (auto i=histos1d.begin(); i!=histos1d.end(); i++){
	  if( std::string(i->first).find(h_name) != std::string::npos && std::string(i->first).find(m_name)!= std::string::npos){
	    foundMatch=true;
	  }
	}
	if(foundMatch)
	  continue;
	//check 2d histo list
	for (auto i=histos2d.begin(); i!=histos2d.end(); i++){
	  if( std::string(i->first).find(h_name) != std::string::npos && std::string(i->first).find(m_name)!= std::string::npos){
	    foundMatch=true;
	  }
	}

	if(foundMatch)
	  continue;

	//if we get here, haven't found any existing histos with a name like stub=hname
	//so, make it.  
	h_name=m_name+"_"+hist.key();
	std::size_t found = (hist.key()).find_last_of("_");
	std::string extension = hist.key().substr(found+1);
	if(debug_){
	  std::cout << "DefineOneTimeHistos: " << h_name << std::endl;  
	  std::cout << extension << hist.value().at("xtitle") << std::endl;
	}
	if (extension == "h") {
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
	    }//bins
	  }//labels
	}
	else if (extension == "hh") {
	  histos2d[h_name] = plot2D(h_name,
				    hist.value().at("xtitle"),hist.value().at("binsX"),hist.value().at("minX"),hist.value().at("maxX"),
				    hist.value().at("ytitle"),hist.value().at("binsY"),hist.value().at("minY"),hist.value().at("maxY"));
	}

	  
    }
    if(debug_)std::cout<<"DefineOneTimeHistos:  Done with copies"<<std::endl;
}

