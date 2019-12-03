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
    pos_.Clear();
    //p1_.Clear();
    //p2_.Clear();
    ntracks_ = 0;
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
        pos_.SetY(pos[2]);
        pos_.SetZ(pos[1]*sin(svtAngle) + pos[0]*cos(svtAngle));
        pos_.SetX(pos[0]*cos(svtAngle) - pos[0]*sin(svtAngle));
    }
    else {
        // The vertex object should already by in HPS coordinates!  (if not in the SVT already)
        pos_.SetX(pos[0]);
        pos_.SetY(pos[1]);
        pos_.SetZ(pos[2]);
    }  
}

void Vertex::setVtxParameters(const std::vector<float>& parameters) { 
    parameters_ = parameters;

    //std::cout << "There are " << parameters.size() << " parameters in this vertex" << std::endl;
    //2016 invM,p1X, p2Y, p2X, p1Z, p2Z, p1Y,invMerr
    if (parameters_.size() == 8 ) 
    { 
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
    else if (parameters_.size() == 24 ) //V0PzErr, invMass, V0Pz, vXErr, V0Py, V0Px, V0PErr, V0TargProjY, vZErr, V0TargProjXErr, vYErr, V0TargProjYErr, invMassError, p1X, p2Y, p2X, V0P, p1Z, p1Y, p2Z, V0TargProjX, layerCode, V0PxErr, V0PyErr
    {
        invM_    = parameters_[1];
        invMerr_ = parameters_[12];
    }
}
