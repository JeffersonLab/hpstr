#include "AnaHelpers.h"



std::string AnaHelpers::getFileName(std::string filePath, bool withExtension)
{
    char sep = '/';
    
    size_t i = filePath.rfind(sep, filePath.length());
    std::string baseFile="";

    if (i != std::string::npos) {
        baseFile = filePath.substr(i+1, filePath.length() - i);
    }
    
    if (withExtension)
        return baseFile;
    else {
        i = baseFile.find_last_of('.');
        return baseFile.substr(0,i);
    }
    
    return "";
}

/*  
 *  checks if cluster is in fiducial region
 *  this was copied from my DST code -- mg
 */
bool AnaHelpers::IsECalFiducial(CalCluster* clu){
  if(clu==NULL)
    return false;
  bool in_fid=false; 
  double x_edge_low = -262.74; 
  double x_edge_high = 347.7; 
  double y_edge_low = 33.54; 
  double y_edge_high = 75.18; 
        
  double x_gap_low = -106.66; 
  double x_gap_high = 42.17; 
  double y_gap_high = 47.18; 

  double x=clu->getPosition().at(0);
  double y=clu->getPosition().at(1);
  y=abs(y);

  if( x>x_edge_low && x < x_edge_high && y > y_edge_low && y < y_edge_high)
    if ( !(x > x_gap_low && x < x_gap_high && y > y_edge_low && y < y_gap_high) )
      in_fid = true;
    
  return in_fid;  
}

void AnaHelpers::InnermostLayerCheck(Track* trk, bool& foundL1, bool& foundL2) {
    bool isKF = trk->isKalmanTrack();
    int innerCount = 0;
    bool hasL1 = false;
    bool hasL2 = false;
    bool hasL3 = false;
    for (int ihit=0; ihit<trk->getHitLayers().size();++ihit) {
        int hit3d = trk->getHitLayers().at(ihit);
        if(isKF){
            if (hit3d == 0 ) {
                innerCount++;
            }
            if (hit3d == 1) {
                innerCount++;
            }
            if (hit3d == 2) {
                innerCount++;
                hasL2 = true;
            }
            if (hit3d == 3) {
                innerCount++;
                hasL3 = true;
            }
        }
        else{
            if (hit3d == 0 ) {
                innerCount++;
            }
            if (hit3d == 1) {
                innerCount++;
                hasL1 = true;
            }
        }
    }
}

double AnaHelpers::GetClusterCoplanarity(CalCluster* cl1,CalCluster* cl2){
  double photon_nom_x=42.52; //nominal photon position
  double radian=180.0/3.14; 
  double cl1X=cl1->getPosition().at(0);
  double cl1Y=cl1->getPosition().at(1);
  double cl1Z=cl1->getPosition().at(2);
  double cl2X=cl2->getPosition().at(0);
  double cl2Y=cl2->getPosition().at(1);
  double cl2Z=cl2->getPosition().at(2);
  double cl1E=cl1->getEnergy();  
  double cl2E=cl2->getEnergy();

  double cl1_impact_angle=atan2(cl1Y,cl1X-photon_nom_x)*radian;
  double cl2_impact_angle=atan2(cl2Y,cl2X-photon_nom_x)*radian;
  if(cl1_impact_angle<0.)
    cl1_impact_angle+=360.;
  if(cl2_impact_angle<0.)
    cl2_impact_angle+=360.;
  if (cl1Y>0)
    return cl2_impact_angle-cl1_impact_angle;
  else
    return cl1_impact_angle-cl2_impact_angle;
}



bool AnaHelpers::MatchToGBLTracks(int ele_id, int pos_id, Track* & ele_trk, Track* & pos_trk, std::vector<Track*>& trks) {

    bool foundele = false;
    bool foundpos = false;

    for (auto trk : trks) {
        if (ele_id == trk->getID())  {
            ele_trk = trk;
            foundele = true;
        }
        if (pos_id == trk->getID()) {
            pos_trk = trk;
            foundpos = true;
        }
    }
    return foundele * foundpos;
}

Track* AnaHelpers::GetTrackFromParticle(std::vector<Track*>& trks,Particle* part){
  int trkID=(part->getTrack()).getID();
  for (auto trk :trks){
    if (trkID == trk->getID()) 
      return trk;
  }
  return NULL; 
}

Particle* AnaHelpers::GetParticleFromCluster(std::vector<Particle*>& parts,CalCluster* cluster){
  CalHit* clusSeed=(CalHit*)cluster->getSeed();
  for (auto part :parts){
    CalCluster pClus=part->getCluster();
    if(pClus.getNHits() == 0)
      continue;
    CalHit* pClusSeed=(CalHit*)pClus.getSeed();
    if ( clusSeed== pClusSeed) 
      return part;
  }
  return NULL; 
}

//TODO clean bit up 
bool AnaHelpers::GetParticlesFromVtx(Vertex* vtx, Particle*& ele, Particle*& pos) {


    bool foundele = false;
    bool foundpos = false;

    for (int ipart = 0; ipart < vtx->getParticles().GetEntries(); ++ipart) {


        int pdg_id = ((Particle*)vtx->getParticles().At(ipart))->getPDG();
        if (debug_) std::cout<<"In Loop "<<pdg_id<< " "<< ipart<<std::endl;

        if (pdg_id == 11) {
            ele =  ((Particle*)vtx->getParticles().At(ipart));
            foundele=true;
            if (debug_) std::cout<<"found ele "<< (int)foundele<<std::endl;
        }
        else if (pdg_id == -11) {
            pos = (Particle*)vtx->getParticles().At(ipart);
            foundpos=true;
            if  (debug_) std::cout<<"found pos "<<(int)foundpos<<std::endl;

        }
    }

    if (!ele || !pos) {
        std::cout<<"Vertex formed without ele/pos. Skip."<<std::endl;
        return false;
    }
    if (debug_) std::cout<<"returning "<<(int) (foundele && foundpos) <<std::endl;
    return foundele && foundpos;
}

//TODO clean bit up 
bool AnaHelpers::GetParticlesFromVtxAndParticleList(std::vector<Particle*>& parts, Vertex* vtx, Particle*& ele, Particle*& pos) {


    bool foundele = false;
    bool foundpos = false;
    bool matchele=false;
    bool matchpos=false;
    Particle* eleTmp=nullptr;
    Particle* posTmp=nullptr;
    for (int ipart = 0; ipart < vtx->getParticles().GetEntries(); ++ipart) {


        int pdg_id = ((Particle*)vtx->getParticles().At(ipart))->getPDG();
        if (debug_) std::cout<<"In Loop "<<pdg_id<< " "<< ipart<<std::endl;

        if (pdg_id == 11) {
            eleTmp =  ((Particle*)vtx->getParticles().At(ipart));
            foundele=true;
            if (debug_) std::cout<<"found ele "<< (int)foundele<<std::endl;
        }
        else if (pdg_id == -11) {
            posTmp = (Particle*)vtx->getParticles().At(ipart);
            foundpos=true;
            if  (debug_) std::cout<<"found pos "<<(int)foundpos<<std::endl;

        }
    }

    if (!eleTmp || !posTmp) {
        std::cout<<"Vertex formed without ele/pos. Skip."<<std::endl;
        return false;
    }

    double eleEne=eleTmp->getMomentum().at(2);//this is a dumb way to match but particles don't have IDs...
    double posEne=posTmp->getMomentum().at(2);

    for (auto part :parts){     
      
      if (eleEne == part->getMomentum().at(2)) { 
        matchele=true;
        ele=part;
     }
      if (posEne == part->getMomentum().at(2)) {
        matchpos=true;
        pos=part;
      }
      
    }
    
    return foundele && foundpos && matchpos &&matchele;
}


AnaHelpers::AnaHelpers() {
    rotSvt.RotateY(SVT_ANGLE);

    rotSvt_sym =  new TMatrixDSym(3);

    (*rotSvt_sym)(0,0) = cos(SVT_ANGLE);
    (*rotSvt_sym)(0,1) = 0.;
    (*rotSvt_sym)(0,2) = sin(SVT_ANGLE);

    (*rotSvt_sym)(1,0) = 0.;
    (*rotSvt_sym)(1,1) = 1.;
    (*rotSvt_sym)(1,2) = 0.;

    (*rotSvt_sym)(1,0) = -sin(SVT_ANGLE);
    (*rotSvt_sym)(1,1) = 0.;
    (*rotSvt_sym)(1,2) = cos(SVT_ANGLE);

    debug_ = false;

}

TVector3 AnaHelpers::rotateToSvtFrame(TVector3 v) {

    v.RotateY(SVT_ANGLE);
    return v;   
}

TVector3 AnaHelpers::rotationToSvtFrame(const TVector3&  v) {
    return rotSvt*v;
}
