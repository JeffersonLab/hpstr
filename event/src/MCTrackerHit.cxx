/**
 * @file MCTrackerHit.cxx
 * @brief Class used to encapsulate mc tracker hit information
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 */

#include "MCTrackerHit.h"

ClassImp(MCTrackerHit)

MCTrackerHit::MCTrackerHit()
    : TObject() { 
    }

MCTrackerHit::~MCTrackerHit() { 
    Clear(); 
}

void MCTrackerHit::Clear(Option_t* /* options */) { 
    TObject::Clear(); 
}

void MCTrackerHit::setMomentum(const float* momentum, bool rotate) {

    //svt angle: it's already with minus sign.
    float svtAngle = 30.5e-3;
    //Rotate the the input position automatically to match with the SVT tracker system
    if (rotate)
    {
        //x_ = position[1];
        py_ = momentum[2];
        pz_ = momentum[1] * sin(svtAngle) + momentum[0]*cos(svtAngle);
        px_ = momentum[1] * cos(svtAngle) - momentum[0]*sin(svtAngle);
    }
    else {
        px_ = momentum[0]; 
        py_ = momentum[1];
        pz_ = momentum[2];
    }
}

void MCTrackerHit::setPosition(const double* position, bool rotate) {

    //svt angle: it's already with minus sign.
    float svtAngle = 30.5e-3;
    //Rotate the the input position automatically to match with the SVT tracker system
    if (rotate)
    {
        //x_ = position[1];
        y_ = position[2];
        z_ = position[1] * sin(svtAngle) + position[0]*cos(svtAngle);
        x_ = position[1] * cos(svtAngle) - position[0]*sin(svtAngle);
    }
    else {
        x_ = position[0]; 
        y_ = position[1];
        z_ = position[2];
    }
}
