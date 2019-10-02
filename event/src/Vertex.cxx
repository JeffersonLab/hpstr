/** 
 * @file Vertex.h
 * @brief Class used to encapsulate vertex information.
 * @author PF, SLAC
 */

#include "Vertex.h"

ClassImp(Vertex)

Vertex::Vertex()
: TObject() {
}

Vertex::~Vertex() {
  Clear();
  delete tracks_;
}

void Vertex::Clear(Option_t *option) {
  TObject::Clear();
  if (tracks_)
    tracks_->Delete();
  pos_.Clear();
  p1_.Clear();
  p2_.Clear();
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

void Vertex::setCovariance( const std::vector<float>* vec){ 
  covariance_ = *vec;
}

void Vertex::setPos(const float* pos, bool rotate) {
  float svtAngle = 30.5e-3;
  if (rotate) {
    y_ = pos[2];
    z_ = pos[1]*sin(svtAngle) + pos[0]*cos(svtAngle);
    x_ = pos[0]*cos(svtAngle) - pos[0]*sin(svtAngle);
  }
  else {
    x_ = pos[1];
    y_ = pos[2];
    z_ = pos[0];
  }
  
}
