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

void AnaHelpers::InnermostLayerCheck(Track* trk, bool& foundL1, bool& foundL2) {
    for (int ihit=0; ihit<trk->getSvtHits()->GetEntries();++ihit) {
        TrackerHit* hit3d = (TrackerHit*) trk->getSvtHits()->At(ihit);
        if (hit3d->getLayer() == 0 ) {
            foundL1 = true;
        }
        if (hit3d->getLayer() == 1) {
            foundL2 = true;
        }
    }
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


//TODO clean bit up 
bool AnaHelpers::GetParticlesFromVtx(Vertex* vtx, Particle*& ele, Particle*& pos) {
    
    bool foundele = false;
    bool foundpos = false;
    
    for (int ipart = 0; ipart < vtx->getParticles()->GetEntries(); ++ipart) {
        
        int pdg_id = ((Particle*)vtx->getParticles()->At(ipart))->getPDG();
        
        if (pdg_id == 11) {
            ele =  ((Particle*)vtx->getParticles()->At(ipart));
            foundele=true;
        }
        else if (pdg_id == -11) {
            pos = (Particle*)vtx->getParticles()->At(ipart);
            foundpos=true;
        }
    }

    if (!ele || !pos) {
        std::cout<<"Vertex formed without ele/pos. Skip."<<std::endl;
        return false;
    }
    
    return foundele && foundpos;
}

Particle* AnaHelpers::GetParticleFromCluster(CalCluster* clu,std::vector<Particle*>& partList){
  Particle* matchedPart=NULL;
  for(auto part:partList){
    CalCluster partCluster=part->getCluster();
    if(partCluster.getEnergy()!=-9999){
      double energy=partCluster.getEnergy();
      if(clu->getEnergy() == energy)
	matchedPart=part;
	  
    }
  }
  return matchedPart;    
}

Track* AnaHelpers::GetGBLTrackFromParticle(std::vector<Track*>& trks,Particle* part){
  int trkID=(part->getTrack()).getID();
  for (auto trk :trks){
    if (trkID == trk->getID()) 
      return trk;
  }
  return NULL; 
}


/*
 *  check for track duplicates and return bool=true if this is best track
 *  first test: # of hits; if same number of hits, return best chi2
 */
bool AnaHelpers::IsBestTrack(Particle* part,std::vector<Track*>& trks){
  Track* trk=GetGBLTrackFromParticle(trks, part);
  return IsBestTrack(trk,trks);
}

bool AnaHelpers::IsBestTrack(Track* trk,std::vector<Track*>& trks){
  int maxSharedHits=1;
  if(trk==NULL)
    return true;
  for (auto testTrk : trks){
    if (trk == testTrk)
      continue;
    int shrdHits=CountSharedTrackHits(trk,testTrk);
    if(shrdHits>maxSharedHits){
      std::cout<<"shared hits = "<<shrdHits<<std::endl;
    }
    if(shrdHits>maxSharedHits){
      if(testTrk->getTrackerHitCount()>trk->getTrackerHitCount()){
	return false;
      }else if (testTrk->getTrackerHitCount()==trk->getTrackerHitCount() ){
	if(testTrk->getChi2Ndf()<trk->getChi2Ndf())
	  return false;
      }   
    }   
  }
  return true;
}

int AnaHelpers::CountSharedTrackHits(Track* trk1,Track* trk2){

  TRefArray* trk1Hits=trk1->getSvtHits();
  TRefArray* trk2Hits=trk2->getSvtHits();
  int cnt=0;
  //  std::cout<<trk1Hits->GetEntries()<<"    "<<trk2Hits->GetEntries()<<std::endl;
   for(int ihit=0;ihit<trk1Hits->GetEntries();ihit++){
     TrackerHit* hit1=(TrackerHit*)trk1Hits->At(ihit);
     for(int jhit=0;jhit<trk2Hits->GetEntries();jhit++){
       TrackerHit* hit2=(TrackerHit*)trk2Hits->At(jhit);
       //       std::cout<<hit1->getID()<<"   "<<hit2->getID()<<std::endl;
       if(hit1->getID()==hit2->getID())
 	cnt++;
     }
   }
   return cnt;
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
/*  
 *  get 2-cluster coplanarity
 *  this was copied from my DST code -- mg
 */
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


std::vector<int> AnaHelpers::getMCParticleLayersHit(MCParticle* mcpart, std::vector<MCTrackerHit*>& mchits){
  
  std::vector<int> layersHit; 
  for(auto mchit: mchits){
    if(mchit->getMCParticleID()==mcpart->getID()){ //hit has MCParticle under consideration
      if(std::find(layersHit.begin(), layersHit.end(), mchit->getLayer()) == layersHit.end())
	std::cout<<"Found hit in layer = "<<mchit->getLayer()<<std::endl;
	layersHit.push_back(mchit->getLayer());
    }
  }  
  std::cout<<"Number of layers hit = "<<layersHit.size()<<std::endl;
  return layersHit; 
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
}

TVector3 AnaHelpers::rotateToSvtFrame(TVector3 v) {    
    v.RotateY(SVT_ANGLE);
    return v;   
}

TVector3 AnaHelpers::rotationToSvtFrame(const TVector3&  v) {
    return rotSvt*v;
}
