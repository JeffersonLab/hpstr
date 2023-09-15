/** 
 * @file Vertex.h
 * @brief Class used to encapsulate vertex information.
 * @author PF, SLAC
 * @author Cameron Bravo, SLAC
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
    pos_.Clear();
    p1_.Clear();
    p2_.Clear();
    //parts_->Delete();
    TObject::Clear();
}

void Vertex::addParticle(TObject* part)
{ 
    n_parts_++;
    parts_.Add(part);
}

void Vertex::setCovariance( const std::vector<float>& vec){ 
    covariance_ = vec;
}

void Vertex::setPos(const float* pos, bool rotate) {
    float svtAngle = 30.5e-3;
    if (rotate) {
        pos_.SetY(pos[2]);
        pos_.SetZ(pos[1]*sin(svtAngle) + pos[0]*cos(svtAngle));
        pos_.SetX(pos[1]*cos(svtAngle) - pos[0]*sin(svtAngle));
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
        //First Track
        p1_.SetX(parameters_[1]);
        p1_.SetY(parameters_[6]);
        p1_.SetZ(parameters_[4]);

        //Second Track
        p2_.SetX(parameters_[3]);
        p2_.SetY(parameters_[2]);
        p2_.SetZ(parameters_[5]);

        //Invariant Mass
        invM_    = parameters_[0];
        invMerr_ = parameters_[7];
        
        //Vtx momentum
        p_ = p1_ + p2_ ; 
    }
    else if (parameters_.size() == 21 ) //0 V0PzErr, 1 invMass, 2 V0Pz, 3 V0Py, 4 V0Px, 5 V0PErr, 6 V0TargProjY, 7 V0TargProjXErr, 8 V0TargProjYErr, 9 invMassError, 10 p1X, 11 p2Y, 12 p2X, 13 V0P, 14 p1Z, 15 p1Y, 16 p2Z, 17 V0TargProjX, 18 layerCode, 19 V0PxErr, 20 V0PyErr
    {
        //First Track
        p1_.SetX(parameters_[10]);
        p1_.SetY(parameters_[15]);
        p1_.SetZ(parameters_[14]);

        //Second Track
        p2_.SetX(parameters_[12]);
        p2_.SetY(parameters_[11]);
        p2_.SetZ(parameters_[16]);

        //Invariant Mass
        invM_    = parameters_[1];
        invMerr_ = parameters_[9];
        
        //Vtx momentum
        p_ = p1_ + p2_ ; 
    }
    else if (parameters_.size() == 24 ) //0 V0PzErr, 1 invMass, 2 V0Pz, 3 vXErr, 4 V0Py, 5 V0Px, 6 V0PErr, 7 V0TargProjY, 8 vZErr, 9 V0TargProjXErr, 10 vYErr, 11 V0TargProjYErr, 12 invMassError, 13 p1X, 14 p2Y, 15 p2X, 16 V0P, 17 p1Z, 18 p1Y, 19 p2Z, 20 V0TargProjX, 21 layerCode, 22 V0PxErr, 23 V0PyErr
    {
        //First Track
        p1_.SetX(parameters_[13]);
        p1_.SetY(parameters_[18]);
        p1_.SetZ(parameters_[17]);

        //Second Track
        p2_.SetX(parameters_[15]);
        p2_.SetY(parameters_[14]);
        p2_.SetZ(parameters_[19]);

        p_ = p1_ + p2_;

        invM_    = parameters_[1];
        invMerr_ = parameters_[12];
    }
}
