/** 
 * @file Vertex.h
 * @brief Class used to encapsulate vertex information.
 * @author PF, SLAC
 */

#include "Vertex.h"
#include <iostream>

ClassImp(Vertex)

Vertex::Vertex()
    : TObject() {
}

Vertex::~Vertex() {
  Clear();
}

void Vertex::Clear(Option_t *option) {
  //TObject::Clear();
  if (tracks_)
    tracks_->Delete();
  //pos_.Clear();
  //p1_.Clear();
  //p2_.Clear();
  ntracks_ = 0;
  x_ = -999.;
  y_ = -999.;
  z_ = -999.;
}

bool Vertex::vxTracksAvailable() const {

    if (tracks_)
        return true;

    return false;
}

int Vertex::nTracks() const {
  if (vxTracksAvailable())
    return tracks_->GetEntries();
  return -1;
}

void Vertex::setCovariance( const std::vector<float>& vec){ 
  covariance_ = vec;
}

void Vertex::setPos(const float* pos, bool rotate) {
  float svtAngle = 30.5e-3;
  if (rotate) {
    y_ = pos[2];
    z_ = pos[1]*sin(svtAngle) + pos[0]*cos(svtAngle);
    x_ = pos[0]*cos(svtAngle) - pos[0]*sin(svtAngle);
  }
  else {
    // The vertex object should already by in HPS coordinates!  (if not in the SVT already)
    x_ = pos[0];
    y_ = pos[1];
    z_ = pos[2];
  }  

  pos_.SetX(x_);
  pos_.SetY(y_);
  pos_.SetZ(z_);
  
  
}

void Vertex::setVtxParameters(const std::vector<float>& parameters) { 
  parameters_ = parameters;
  
  //2016 invM,p1X, p2Y, p2X, p1Z, p2Z, p1Y,invMerr
  if (parameters_.size() == 8 ) { 
    invM_    = parameters_[0];
    p1x_     = parameters_[1];
    p2y_     = parameters_[2];
    p2x_     = parameters_[3];
    p1z_     = parameters_[4];
    p2z_     = parameters_[5];
    p1y_     = parameters_[6];
    invMerr_ = parameters_[7];
    
    //Build the TVector3
    p1_.SetX(p1x_);
    p1_.SetY(p1y_);
    p1_.SetZ(p1z_);

    //Build the TVector3
    //Build the TVector3
    p2_.SetX(p2x_);
    p2_.SetY(p2y_);
    p2_.SetZ(p2z_);
  }
}
