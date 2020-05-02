#include "MCTrackerHit.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <cmath>

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

void MCTrackerHit::setPosition(const double* position, bool rotate) {

    // Rotate the the input position automatically to match with the 
    // SVT tracker system
    if (rotate) {
        //x_ = position[1];
        y_ = position[2];
        z_ = position[1] * sin(svt_angle_) + position[0]*cos(svt_angle_);
        x_ = position[1] * cos(svt_angle_) - position[0]*sin(svt_angle_);
    }
    else {
        x_ = position[0]; 
        y_ = position[1];
        z_ = position[2];
    }
}
