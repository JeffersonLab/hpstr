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
    x_ = position[0]; 
    y_ = position[1];
    z_ = position[2]; 
}

void TrackerHit::setCovarianceMatrix(const std::vector<float> covariance_matrix) {
    cxx_ = covariance_matrix[0];
    cxy_ = covariance_matrix[1];
    cxz_ = covariance_matrix[2];
    cyy_ = covariance_matrix[3];
    cyz_ = covariance_matrix[4];
    czz_ = covariance_matrix[5];
}

std::vector<double> TrackerHit::getCovarianceMatrix() const { 
    return { cxx_, cxy_, cxz_, cyy_, cyz_, czz_ }; 
}
