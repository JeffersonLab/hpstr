#include "TrackEfficHistos.h"
#include "TLorentzVector.h"
#include "TVector3.h"
#include <iostream>

//void TrackEfficHistos::BuildAxes(){}
//      histos2d[m_name+"_vtxY_vs_vtxX"] = plot2D(m_name+"_vtxY_vs_vtxX",
//      "vtxX",axes["vtx_X"][0],axes["vtx_X"][1],axes["vtx_X"][2],
//      "vtxY",axes["vtx_Y"][0],axes["vtx_Y"][1],axes["vtx_Y"][2]);
//    */
  
  
//}//define 2dhistos


void TrackEfficHistos::FillEffPlots(std::pair<CalCluster*,Track*> ele, 
                               	std::pair<CalCluster*,Track*> pos, 
                               float weight) {   
  
  CalCluster* eleClu=ele.first;
  CalCluster* posClu=pos.first;
  Track* eleTrk=ele.second;
  Track* posTrk=pos.second;
  bool hasElectronTrack=false;
  bool hasPositronTrack=false;
  /*
  double elePid=ele->getGoodnessOfPID();
  double posPid=pos->getGoodnessOfPID();
  if(elePid<99999&&elePid>0)
    hasElectronTrack=true;
  //Pos Track-cluster match
  if(posPid<99999&&posPid>0)
    hasPositronTrack=true;	
  */

  if(eleTrk!=NULL)
    hasElectronTrack=true;
  if(posTrk!=NULL)
    hasPositronTrack=true;

  //  if (cluSelector->passCutLt("posTrkCluMatch_lt",positron->getGoodnessOfPID(),weight))
  //  std::cout<<"FillEffPlots::filling plots"<<std::endl;

  double eleE=eleClu->getEnergy();
  double eleX=eleClu->getPosition().at(0);
  double eleY=eleClu->getPosition().at(1);
  double posE=posClu->getEnergy();
  double posX=posClu->getPosition().at(0);
  double posY=posClu->getPosition().at(1);

  Fill1DHisto("clE_ele_allpos_allele_h",eleE,weight);
  Fill1DHisto("clX_ele_allpos_allele_h",eleX,weight);
  Fill1DHisto("clY_ele_allpos_allele_h",eleY,weight);
  Fill1DHisto("clE_pos_allpos_allele_h",posE,weight);
  Fill1DHisto("clX_pos_allpos_allele_h",posX,weight);
  Fill1DHisto("clY_pos_allpos_allele_h",posY,weight);
  Fill2DHisto("clY_vs_clX_ele_allpos_allele_hh",eleX,eleY,weight);
  Fill2DHisto("clY_vs_clE_ele_allpos_allele_hh",eleE,eleY,weight);
  Fill2DHisto("clE_vs_clX_ele_allpos_allele_hh",eleX,eleE,weight);        
  Fill2DHisto("clY_vs_clX_pos_allpos_allele_hh",posX,posY,weight);
  Fill2DHisto("clY_vs_clE_pos_allpos_allele_hh",posE,posY,weight);
  Fill2DHisto("clE_vs_clX_pos_allpos_allele_hh",posX,posE,weight);
  
  if(hasElectronTrack && hasPositronTrack){
    Fill1DHisto("clE_ele_foundpos_foundele_h",eleE,weight);
    Fill1DHisto("clX_ele_foundpos_foundele_h",eleX,weight);
    Fill1DHisto("clY_ele_foundpos_foundele_h",eleY,weight);
    Fill1DHisto("clE_pos_foundpos_foundele_h",posE,weight);
    Fill1DHisto("clX_pos_foundpos_foundele_h",posX,weight);
    Fill1DHisto("clY_pos_foundpos_foundele_h",posY,weight);
    Fill2DHisto("clY_vs_clX_ele_foundpos_foundele_hh",eleX,eleY,weight);
    Fill2DHisto("clY_vs_clE_ele_foundpos_foundele_hh",eleE,eleY,weight);
    Fill2DHisto("clE_vs_clX_ele_foundpos_foundele_hh",eleX,eleE,weight);        
    Fill2DHisto("clY_vs_clX_pos_foundpos_foundele_hh",posX,posY,weight);
    Fill2DHisto("clY_vs_clE_pos_foundpos_foundele_hh",posE,posY,weight);
    Fill2DHisto("clE_vs_clX_pos_foundpos_foundele_hh",posX,posE,weight);        
  }else  if(!hasElectronTrack && hasPositronTrack){
    Fill1DHisto("clE_ele_foundpos_missele_h",eleE,weight);
    Fill1DHisto("clX_ele_foundpos_missele_h",eleX,weight);
    Fill1DHisto("clY_ele_foundpos_missele_h",eleY,weight);
    Fill1DHisto("clE_pos_foundpos_missele_h",posE,weight);
    Fill1DHisto("clX_pos_foundpos_missele_h",posX,weight);
    Fill1DHisto("clY_pos_foundpos_missele_h",posY,weight);   
    Fill2DHisto("clY_vs_clX_ele_foundpos_missele_hh",eleX,eleY,weight);
    Fill2DHisto("clY_vs_clE_ele_foundpos_missele_hh",eleE,eleY,weight);
    Fill2DHisto("clE_vs_clX_ele_foundpos_missele_hh",eleX,eleE,weight);        
    Fill2DHisto("clY_vs_clX_pos_foundpos_missele_hh",posX,posY,weight);
    Fill2DHisto("clY_vs_clE_pos_foundpos_missele_hh",posE,posY,weight);
    Fill2DHisto("clE_vs_clX_pos_foundpos_missele_hh",posX,posE,weight);  
  }else  if(hasElectronTrack && !hasPositronTrack){
    Fill1DHisto("clE_ele_misspos_foundele_h",eleE,weight);
    Fill1DHisto("clX_ele_misspos_foundele_h",eleX,weight);
    Fill1DHisto("clY_ele_misspos_foundele_h",eleY,weight);
    Fill1DHisto("clE_pos_misspos_foundele_h",posE,weight);
    Fill1DHisto("clX_pos_misspos_foundele_h",posX,weight);
    Fill1DHisto("clY_pos_misspos_foundele_h",posY,weight); 
    Fill2DHisto("clY_vs_clX_ele_misspos_foundele_hh",eleX,eleY,weight);
    Fill2DHisto("clY_vs_clE_ele_misspos_foundele_hh",eleE,eleY,weight);
    Fill2DHisto("clE_vs_clX_ele_misspos_foundele_hh",eleX,eleE,weight);        
    Fill2DHisto("clY_vs_clX_pos_misspos_foundele_hh",posX,posY,weight);
    Fill2DHisto("clY_vs_clE_pos_misspos_foundele_hh",posE,posY,weight);
    Fill2DHisto("clE_vs_clX_pos_misspos_foundele_hh",posX,posE,weight);  
  } else{
    Fill1DHisto("clE_ele_misspos_missele_h",eleE,weight);
    Fill1DHisto("clX_ele_misspos_missele_h",eleX,weight);
    Fill1DHisto("clY_ele_misspos_missele_h",eleY,weight);
    Fill1DHisto("clE_pos_misspos_missele_h",posE,weight);
    Fill1DHisto("clX_pos_misspos_missele_h",posX,weight);
    Fill1DHisto("clY_pos_misspos_missele_h",posY,weight);
    Fill2DHisto("clY_vs_clX_ele_misspos_missele_hh",eleX,eleY,weight);
    Fill2DHisto("clY_vs_clE_ele_misspos_missele_hh",eleE,eleY,weight);
    Fill2DHisto("clE_vs_clX_ele_misspos_missele_hh",eleX,eleE,weight);        
    Fill2DHisto("clY_vs_clX_pos_misspos_missele_hh",posX,posY,weight);
    Fill2DHisto("clY_vs_clE_pos_misspos_missele_hh",posE,posY,weight);
    Fill2DHisto("clE_vs_clX_pos_misspos_missele_hh",posY,posE,weight);  
  }     
}

void TrackEfficHistos::FillPreSelectionPlots(CalCluster* clu, 
                               float weight) {

    
  //  std::cout<<"FillPreSelectionPlots::filling plots"<<std::endl;
  Fill1DHisto("clTime_preselect_h",clu->getTime(),weight);
  Fill1DHisto("clE_preselect_h",clu->getEnergy(),weight);
  Fill1DHisto("clX_preselect_h",clu->getPosition().at(0),weight);
  Fill1DHisto("clY_preselect_h",clu->getPosition().at(1),weight);
  Fill2DHisto("clY_vs_clX_preselect_hh",clu->getPosition().at(0),clu->getPosition().at(1),weight);
  Fill2DHisto("clY_vs_clE_preselect_hh",clu->getEnergy(),clu->getPosition().at(1),weight);
  Fill2DHisto("clE_vs_clX_preselect_hh",clu->getPosition().at(0),clu->getEnergy(),weight);

}
 
void TrackEfficHistos::FillPairSelectionPlots(CalCluster* eleClu, CalCluster* posClu, 
                               float weight) {
  // std::cout<<"FillPairSelectionPlots::filling plots"<<std::endl;
  Fill1DHisto("clE_ele_allpairs_h",eleClu->getEnergy(),weight);
  Fill1DHisto("clX_ele_allpairs_h",eleClu->getPosition().at(0),weight);
  Fill1DHisto("clY_ele_allpairs_h",eleClu->getPosition().at(1),weight);
  Fill1DHisto("clE_pos_allpairs_h",posClu->getEnergy(),weight);
  Fill1DHisto("clX_pos_allpairs_h",posClu->getPosition().at(0),weight);
  Fill1DHisto("clY_pos_allpairs_h",posClu->getPosition().at(1),weight);
  Fill2DHisto("clY_vs_clX_ele_allpairs_hh",eleClu->getPosition().at(0),eleClu->getPosition().at(1),weight);
  Fill2DHisto("clY_vs_clE_ele_allpairs_hh",eleClu->getEnergy(),eleClu->getPosition().at(1),weight);
  Fill2DHisto("clE_vs_clX_ele_allpairs_hh",eleClu->getPosition().at(0),eleClu->getEnergy(),weight);        
  Fill2DHisto("clY_vs_clX_pos_allpairs_hh",posClu->getPosition().at(0),posClu->getPosition().at(1),weight);
  Fill2DHisto("clY_vs_clE_pos_allpairs_hh",posClu->getEnergy(),posClu->getPosition().at(1),weight);
  Fill2DHisto("clE_vs_clX_pos_allpairs_hh",posClu->getPosition().at(0),posClu->getEnergy(),weight);

  double clTimeDiff=eleClu->getTime()-posClu->getTime();
  double eSum=eleClu->getEnergy()+posClu->getEnergy();

  Fill1DHisto("clTime_ele_allpairs_h",eleClu->getTime(),weight);
  Fill1DHisto("clTime_pos_allpairs_h",posClu->getTime(),weight);
  Fill1DHisto("clTime_eleMinusPos_allpairs_h",clTimeDiff,weight);

}


/*
 *   Do track-cluster matching based on distance between cluster and extrapolated track
 */
std::pair<CalCluster*, Track*> TrackEfficHistos::getClusterTrackPair(CalCluster* cluster,std::vector<Track*>& tracks, float weight){

  double clX=(cluster->getPosition()).at(0);
  double clY=(cluster->getPosition()).at(1);
  double minDelta=666666;
  double minDeltaX=666666;
  double minDeltaY=666666;
  Track* bestTrack=NULL; 
  double minDeltaCut=20.0;
  for(auto trk:tracks){
    if(trk->getTanLambda()*clY<0)//trk and cluster in same half
      continue; 
    int trkCh=trk->getCharge();
    if(trkCh*clX>0)  //positrons on positron side and vice/versa...sign is flipped in track collection? 
      continue;
    std::vector<double> posAtECal=trk->getPositionAtEcal();
    double delY=clY-posAtECal.at(1);
    double delX=clX-posAtECal.at(0);
    if(abs(delY)<minDelta){//right now, just take delta Y
      bestTrack=trk;
      minDelta=abs(delY);
      minDeltaX=delX;
      minDeltaY=delY;
    }
  }

  if(minDelta<minDeltaCut){
    Fill1DHisto("trkClMinDelta_h",minDelta,weight);  //should equal deltaY for now
    Fill1DHisto("trkClDeltaX_h",minDeltaX,weight); 
    Fill1DHisto("trkClDeltaY_h",minDeltaY,weight);
    Fill2DHisto("trkClDeltaXY_hh",minDeltaX,minDeltaY,weight);
    Fill2DHisto("trkClDeltaXvsX_hh",clX,minDeltaX,weight);
    Fill2DHisto("trkClDeltaYvsY_hh",clY,minDeltaY,weight);
    Fill2DHisto("trkClDeltaYvsX_hh",clX,minDeltaY,weight);
    Fill2DHisto("trkClDeltaXvsY_hh",clY,minDeltaX,weight);
  }


  return std::pair<CalCluster*,Track*>(cluster,bestTrack);
}
