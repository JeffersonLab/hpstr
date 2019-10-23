/**
 * @file MCEcalHit.cxx
 * @brief Class used to encapsulate mc tracker hit information
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 */

#include "MCEcalHit.h"

ClassImp(MCEcalHit)

MCEcalHit::MCEcalHit()
    : TObject() { 
    }

MCEcalHit::~MCEcalHit() { 
    Clear(); 
}

void MCEcalHit::Clear(Option_t* /* options */) { 
    TObject::Clear(); 
}

void MCEcalHit::setPosition(const double* position, bool rotate) {

    //svt angle: it's already with minus sign.
    float svtAngle = 30.5e-3;
    //Rotate the the input position automatically to match with the SVT tracker system
    if (rotate)
    {
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
