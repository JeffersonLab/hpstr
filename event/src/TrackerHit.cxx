/**
 * @file TrackerHit.cxx
 * @brief Class used to encapsulate tracker hit information
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "TrackerHit.h"

ClassImp(TrackerHit)

TrackerHit::TrackerHit()
    : TObject() { 
}

TrackerHit::~TrackerHit() { 
    Clear(); 
}

void TrackerHit::Clear(Option_t* /* options */) { 
    TObject::Clear(); 
}

void TrackerHit::setPosition(const double* position) {
    x = position[0]; 
    y = position[1];
    z = position[2]; 
}

std::vector<double> TrackerHit::getPosition() const { 
    return { x, y, z }; 
}


void TrackerHit::setCovarianceMatrix(const std::vector<float> covariance_matrix) {
	cxx = covariance_matrix[0];
	cxy = covariance_matrix[1];
	cxz = covariance_matrix[2];
	cyy = covariance_matrix[3];
	cyz = covariance_matrix[4];
	czz = covariance_matrix[5];
}

std::vector<double> TrackerHit::getCovarianceMatrix() const { 
    return { cxx, cxy, cxz, cyy, cyz, czz }; 
}
