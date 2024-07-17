#include "ThreeProngHistos.h"
#include "TLorentzVector.h"
#include "TVector3.h"
#include <iostream>


//fill plots for 3-prong tridents....
//the "ele" always has higher  energy cluster than "recoil"
void ThreeProngHistos::FillThreeProngPlots(Particle* ele, 
                                           Particle* pos,
                                           Particle* recoil,
                                           float weight) { 
  CalCluster eleClu=ele->getCluster();
  CalCluster posClu=pos->getCluster();
  CalCluster recClu=recoil->getCluster();
  Track eleTrk=ele->getTrack();
  Track posTrk=pos->getTrack();
  Track recTrk=recoil->getTrack();
  bool hasElectronTrack=false;
  bool hasPositronTrack=false;
  bool hasRecoilTrack=false;
  if(eleTrk.getTrackerHitCount()>0)
    hasElectronTrack=true;
  if(posTrk.getTrackerHitCount()>0)
    hasPositronTrack=true;
  if(recTrk.getTrackerHitCount()>0)
    hasRecoilTrack=true;
  if(hasElectronTrack)
    if(eleTrk.getCharge()==1){
      std::cout<<"FillThreeProngPlots::Oops...positron pointing to electron-side cluster...lets skip it...return without filling"<<std::endl;
      return;
    }
       
  if(hasPositronTrack)
    if(posTrk.getCharge()==-1){
      std::cout<<"FillThreeProngPlots::Oops...electron pointing to positron-side cluster...lets skip it...return without filling"<<std::endl;
      return;
    }

  if(hasRecoilTrack)
    if(recTrk.getCharge()==1){
      std::cout<<"FillThreeProngPlots::Oops...positron pointing to recoil cluster...lets skip it for now...return without filling"<<std::endl;
      return;
    }


  int evtCharge=0;
  if(hasElectronTrack)
    evtCharge+=eleTrk.getCharge();
  if(hasPositronTrack)
    evtCharge+=posTrk.getCharge();
  if(hasRecoilTrack)
    evtCharge+=recTrk.getCharge();


  int eleTrkNHits=eleTrk.getTrackerHitCount();
  int posTrkNHits=posTrk.getTrackerHitCount();
  int recTrkNHits=recTrk.getTrackerHitCount();
  double eleE=eleClu.getEnergy();
  double eleX=eleClu.getPosition().at(0);
  double eleY=eleClu.getPosition().at(1);
  double eleZ=eleClu.getPosition().at(2);
  double posE=posClu.getEnergy();
  double posX=posClu.getPosition().at(0);
  double posY=posClu.getPosition().at(1);
  double posZ=posClu.getPosition().at(2);
  double recE=recClu.getEnergy();
  double recX=recClu.getPosition().at(0);
  double recY=recClu.getPosition().at(1);
  double recZ=recClu.getPosition().at(2);
  double esum=eleE+posE+recE; 
  double eleTrkMom=eleTrk.getMomentum()[2];
  double posTrkMom=posTrk.getMomentum()[2];
  double recTrkMom=recTrk.getMomentum()[2];
 //roughly the amount of "energy in E"...true for photons, approximation for charged tracks
  double eleClYEne=sin(atan2(eleY,eleZ))*eleE; 
  double posClYEne=sin(atan2(posY,posZ))*posE; 
  double recClYEne=sin(atan2(recY,recZ))*recE; 
  double netClYEne=posClYEne+eleClYEne+recClYEne; 
  bool electronsSameHalf=false;
  if(eleY*recY)
    electronsSameHalf=true;


  std::string threeProngDir="/threeProngers/";

  Fill1DHisto("clE_sum_allpos_allele_allrec_h",esum);
  Fill1DHisto("clNet_EY_allpos_allele_allrec_h",netClYEne);
  Fill1DHisto("clE_ele_allpos_allele_allrec_h",eleE);
  Fill1DHisto("clX_ele_allpos_allele_allrec_h",eleX);
  Fill1DHisto("clY_ele_allpos_allele_allrec_h",eleY);
  Fill1DHisto("clE_pos_allpos_allele_allrec_h",posE);
  Fill1DHisto("clX_pos_allpos_allele_allrec_h",posX);
  Fill1DHisto("clY_pos_allpos_allele_allrec_h",posY);
  Fill1DHisto("clE_rec_allpos_allele_allrec_h",recE);
  Fill1DHisto("clX_rec_allpos_allele_allrec_h",recX);
  Fill1DHisto("clY_rec_allpos_allele_allrec_h",recY);

  if(hasRecoilTrack&&hasPositronTrack&&hasElectronTrack){
    Fill1DHisto("clE_sum_foundpos_foundele_foundrec_h",esum);
    Fill1DHisto("clNet_EY_foundpos_foundele_foundrec_h",netClYEne);
    Fill1DHisto("clE_ele_foundpos_foundele_foundrec_h",eleE);
    Fill1DHisto("clX_ele_foundpos_foundele_foundrec_h",eleX);
    Fill1DHisto("clY_ele_foundpos_foundele_foundrec_h",eleY);
    Fill1DHisto("clE_pos_foundpos_foundele_foundrec_h",posE);
    Fill1DHisto("clX_pos_foundpos_foundele_foundrec_h",posX);
    Fill1DHisto("clY_pos_foundpos_foundele_foundrec_h",posY);
    Fill1DHisto("clE_rec_foundpos_foundele_foundrec_h",recE);
    Fill1DHisto("clX_rec_foundpos_foundele_foundrec_h",recX);
    Fill1DHisto("clY_rec_foundpos_foundele_foundrec_h",recY);

    Fill2DHisto("clY_vs_clE_ele_foundpos_foundele_foundrec_hh",eleE,eleY);
    Fill2DHisto("clY_vs_clX_ele_foundpos_foundele_foundrec_hh",eleX,eleY);
    Fill2DHisto("clE_vs_clX_ele_foundpos_foundele_foundrec_hh",eleX,eleE);
    Fill2DHisto("clY_vs_clE_pos_foundpos_foundele_foundrec_hh",posE,posY);
    Fill2DHisto("clY_vs_clX_pos_foundpos_foundele_foundrec_hh",posX,posY);
    Fill2DHisto("clE_vs_clX_pos_foundpos_foundele_foundrec_hh",posX,posE);
    Fill2DHisto("clY_vs_clE_rec_foundpos_foundele_foundrec_hh",recE,recY);
    Fill2DHisto("clY_vs_clX_rec_foundpos_foundele_foundrec_hh",recX,recY);
    Fill2DHisto("clE_vs_clX_rec_foundpos_foundele_foundrec_hh",recX,recE);

    Fill1DHisto("nHits_ele_foundpos_foundele_foundrec_h",eleTrkNHits,weight);
    Fill1DHisto("trkMom_ele_foundpos_foundele_foundrec_h",eleTrkMom,weight);
    Fill1DHisto("trkEoverP_ele_foundpos_foundele_foundrec_h",eleE/eleTrkMom,weight);
    Fill1DHisto("nHits_pos_foundpos_foundele_foundrec_h",posTrkNHits,weight);
    Fill1DHisto("trkMom_pos_foundpos_foundele_foundrec_h",posTrkMom,weight);
    Fill1DHisto("trkEoverP_pos_foundpos_foundele_foundrec_h",posE/posTrkMom,weight);
    Fill1DHisto("nHits_rec_foundpos_foundele_foundrec_h",recTrkNHits,weight);
    Fill1DHisto("trkMom_rec_foundpos_foundele_foundrec_h",recTrkMom,weight);
    Fill1DHisto("trkEoverP_rec_foundpos_foundele_foundrec_h",recE/recTrkMom,weight);

  }
  if(!hasRecoilTrack&&hasPositronTrack&&hasElectronTrack){
    Fill1DHisto("clE_sum_foundpos_foundele_missrec_h",esum);
    Fill1DHisto("clNet_EY_foundpos_foundele_missrec_h",netClYEne);
    Fill1DHisto("clE_ele_foundpos_foundele_missrec_h",eleE);
    Fill1DHisto("clX_ele_foundpos_foundele_missrec_h",eleX);
    Fill1DHisto("clY_ele_foundpos_foundele_missrec_h",eleY);
    Fill1DHisto("clE_pos_foundpos_foundele_missrec_h",posE);
    Fill1DHisto("clX_pos_foundpos_foundele_missrec_h",posX);
    Fill1DHisto("clY_pos_foundpos_foundele_missrec_h",posY);
    Fill1DHisto("clE_rec_foundpos_foundele_missrec_h",recE);
    Fill1DHisto("clX_rec_foundpos_foundele_missrec_h",recX);
    Fill1DHisto("clY_rec_foundpos_foundele_missrec_h",recY);

    
    Fill2DHisto("clY_vs_clE_ele_foundpos_foundele_missrec_hh",eleE,eleY);
    Fill2DHisto("clY_vs_clX_ele_foundpos_foundele_missrec_hh",eleX,eleY);
    Fill2DHisto("clE_vs_clX_ele_foundpos_foundele_missrec_hh",eleX,eleE);
    Fill2DHisto("clY_vs_clE_pos_foundpos_foundele_missrec_hh",posE,posY);
    Fill2DHisto("clY_vs_clX_pos_foundpos_foundele_missrec_hh",posX,posY);
    Fill2DHisto("clE_vs_clX_pos_foundpos_foundele_missrec_hh",posX,posE);
    Fill2DHisto("clY_vs_clE_rec_foundpos_foundele_missrec_hh",recE,recY);
    Fill2DHisto("clY_vs_clX_rec_foundpos_foundele_missrec_hh",recX,recY);
    Fill2DHisto("clE_vs_clX_rec_foundpos_foundele_missrec_hh",recX,recE);

    Fill1DHisto("nHits_ele_foundpos_foundele_missrec_h",eleTrkNHits,weight);
    Fill1DHisto("trkMom_ele_foundpos_foundele_missrec_h",eleTrkMom,weight);
    Fill1DHisto("trkEoverP_ele_foundpos_foundele_missrec_h",eleE/eleTrkMom,weight);
    Fill1DHisto("nHits_pos_foundpos_foundele_missrec_h",posTrkNHits,weight);
    Fill1DHisto("trkMom_pos_foundpos_foundele_missrec_h",posTrkMom,weight);
    Fill1DHisto("trkEoverP_pos_foundpos_foundele_missrec_h",posE/posTrkMom,weight);  
  }
  if(hasRecoilTrack&&!hasPositronTrack&&hasElectronTrack){
    Fill1DHisto("clE_sum_misspos_foundele_foundrec_h",esum);
    Fill1DHisto("clNet_EY_misspos_foundele_foundrec_h",netClYEne);
    Fill1DHisto("clE_ele_misspos_foundele_foundrec_h",eleE);
    Fill1DHisto("clX_ele_misspos_foundele_foundrec_h",eleX);
    Fill1DHisto("clY_ele_misspos_foundele_foundrec_h",eleY);
    Fill1DHisto("clE_pos_misspos_foundele_foundrec_h",posE);
    Fill1DHisto("clX_pos_misspos_foundele_foundrec_h",posX);
    Fill1DHisto("clY_pos_misspos_foundele_foundrec_h",posY);
    Fill1DHisto("clE_rec_misspos_foundele_foundrec_h",recE);
    Fill1DHisto("clX_rec_misspos_foundele_foundrec_h",recX);
    Fill1DHisto("clY_rec_misspos_foundele_foundrec_h",recY);

    
    Fill2DHisto("clY_vs_clE_ele_misspos_foundele_foundrec_hh",eleE,eleY);
    Fill2DHisto("clY_vs_clX_ele_misspos_foundele_foundrec_hh",eleX,eleY);
    Fill2DHisto("clE_vs_clX_ele_misspos_foundele_foundrec_hh",eleX,eleE);
    Fill2DHisto("clY_vs_clE_pos_misspos_foundele_foundrec_hh",posE,posY);
    Fill2DHisto("clY_vs_clX_pos_misspos_foundele_foundrec_hh",posX,posY);
    Fill2DHisto("clE_vs_clX_pos_misspos_foundele_foundrec_hh",posX,posE);
    Fill2DHisto("clY_vs_clE_rec_misspos_foundele_foundrec_hh",recE,recY);
    Fill2DHisto("clY_vs_clX_rec_misspos_foundele_foundrec_hh",recX,recY);
    Fill2DHisto("clE_vs_clX_rec_misspos_foundele_foundrec_hh",recX,recE);

    Fill1DHisto("nHits_ele_misspos_foundele_foundrec_h",eleTrkNHits,weight);
    Fill1DHisto("trkMom_ele_misspos_foundele_foundrec_h",eleTrkMom,weight);
    Fill1DHisto("trkEoverP_ele_misspos_foundele_foundrec_h",eleE/eleTrkMom,weight); 
    Fill1DHisto("nHits_rec_misspos_foundele_foundrec_h",recTrkNHits,weight);
    Fill1DHisto("trkMom_rec_misspos_foundele_foundrec_h",recTrkMom,weight);
    Fill1DHisto("trkEoverP_rec_misspos_foundele_foundrec_h",recE/recTrkMom,weight);

  }
  if(hasRecoilTrack&&hasPositronTrack&&!hasElectronTrack){
    Fill1DHisto("clE_sum_foundpos_missele_foundrec_h",esum);
    Fill1DHisto("clNet_EY_foundpos_missele_foundrec_h",netClYEne);
    Fill1DHisto("clE_ele_foundpos_missele_foundrec_h",eleE);
    Fill1DHisto("clX_ele_foundpos_missele_foundrec_h",eleX);
    Fill1DHisto("clY_ele_foundpos_missele_foundrec_h",eleY);
    Fill1DHisto("clE_pos_foundpos_missele_foundrec_h",posE);
    Fill1DHisto("clX_pos_foundpos_missele_foundrec_h",posX);
    Fill1DHisto("clY_pos_foundpos_missele_foundrec_h",posY);
    Fill1DHisto("clE_rec_foundpos_missele_foundrec_h",recE);
    Fill1DHisto("clX_rec_foundpos_missele_foundrec_h",recX);
    Fill1DHisto("clY_rec_foundpos_missele_foundrec_h",recY);

    
    Fill2DHisto("clY_vs_clE_ele_foundpos_missele_foundrec_hh",eleE,eleY);
    Fill2DHisto("clY_vs_clX_ele_foundpos_missele_foundrec_hh",eleX,eleY);
    Fill2DHisto("clE_vs_clX_ele_foundpos_missele_foundrec_hh",eleX,eleE);
    Fill2DHisto("clY_vs_clE_pos_foundpos_missele_foundrec_hh",posE,posY);
    Fill2DHisto("clY_vs_clX_pos_foundpos_missele_foundrec_hh",posX,posY);
    Fill2DHisto("clE_vs_clX_pos_foundpos_missele_foundrec_hh",posX,posE);
    Fill2DHisto("clY_vs_clE_rec_foundpos_missele_foundrec_hh",recE,recY);
    Fill2DHisto("clY_vs_clX_rec_foundpos_missele_foundrec_hh",recX,recY);
    Fill2DHisto("clE_vs_clX_rec_foundpos_missele_foundrec_hh",recX,recE);

    Fill1DHisto("nHits_pos_foundpos_missele_foundrec_h",posTrkNHits,weight);
    Fill1DHisto("trkMom_pos_foundpos_missele_foundrec_h",posTrkMom,weight);
    Fill1DHisto("trkEoverP_pos_foundpos_missele_foundrec_h",posE/posTrkMom,weight);
    Fill1DHisto("nHits_rec_foundpos_missele_foundrec_h",recTrkNHits,weight);
    Fill1DHisto("trkMom_rec_foundpos_missele_foundrec_h",recTrkMom,weight);
    Fill1DHisto("trkEoverP_rec_foundpos_missele_foundrec_h",recE/recTrkMom,weight);

  }
 if(!hasRecoilTrack&&hasPositronTrack&&!hasElectronTrack){
    Fill1DHisto("clE_sum_foundpos_missele_missrec_h",esum);
    Fill1DHisto("clNet_EY_foundpos_missele_missrec_h",netClYEne);
    Fill1DHisto("clE_ele_foundpos_missele_missrec_h",eleE);
    Fill1DHisto("clX_ele_foundpos_missele_missrec_h",eleX);
    Fill1DHisto("clY_ele_foundpos_missele_missrec_h",eleY);
    Fill1DHisto("clE_pos_foundpos_missele_missrec_h",posE);
    Fill1DHisto("clX_pos_foundpos_missele_missrec_h",posX);
    Fill1DHisto("clY_pos_foundpos_missele_missrec_h",posY);
    Fill1DHisto("clE_rec_foundpos_missele_missrec_h",recE);
    Fill1DHisto("clX_rec_foundpos_missele_missrec_h",recX);
    Fill1DHisto("clY_rec_foundpos_missele_missrec_h",recY);

    Fill1DHisto("nHits_pos_foundpos_missele_missrec_h",posTrkNHits,weight);
    Fill1DHisto("trkMom_pos_foundpos_missele_missrec_h",posTrkMom,weight);
    Fill1DHisto("trkEoverP_pos_foundpos_missele_missrec_h",posE/posTrkMom,weight);
  }
 if(!hasRecoilTrack&&!hasPositronTrack&&!hasElectronTrack){
    Fill1DHisto("clE_sum_misspos_missele_missrec_h",esum);
    Fill1DHisto("clNet_EY_misspos_missele_missrec_h",netClYEne);
    Fill1DHisto("clE_ele_misspos_missele_missrec_h",eleE);
    Fill1DHisto("clX_ele_misspos_missele_missrec_h",eleX);
    Fill1DHisto("clY_ele_misspos_missele_missrec_h",eleY);
    Fill1DHisto("clE_pos_misspos_missele_missrec_h",posE);
    Fill1DHisto("clX_pos_misspos_missele_missrec_h",posX);
    Fill1DHisto("clY_pos_misspos_missele_missrec_h",posY);
    Fill1DHisto("clE_rec_misspos_missele_missrec_h",recE);
    Fill1DHisto("clX_rec_misspos_missele_missrec_h",recX);
    Fill1DHisto("clY_rec_misspos_missele_missrec_h",recY);


    Fill2DHisto("clY_vs_clE_ele_misspos_missele_missrec_hh",eleE,eleY);
    Fill2DHisto("clY_vs_clX_ele_misspos_missele_missrec_hh",eleX,eleY);
    Fill2DHisto("clE_vs_clX_ele_misspos_missele_missrec_hh",eleX,eleE);
    Fill2DHisto("clY_vs_clE_pos_misspos_missele_missrec_hh",posE,posY);
    Fill2DHisto("clY_vs_clX_pos_misspos_missele_missrec_hh",posX,posY);
    Fill2DHisto("clE_vs_clX_pos_misspos_missele_missrec_hh",posX,posE);
    Fill2DHisto("clY_vs_clE_rec_misspos_missele_missrec_hh",recE,recY);
    Fill2DHisto("clY_vs_clX_rec_misspos_missele_missrec_hh",recX,recY);
    Fill2DHisto("clE_vs_clX_rec_misspos_missele_missrec_hh",recX,recE);
  }
}
