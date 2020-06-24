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

void TrackerHit::setPosition(const double* position, bool rotate, int type) {
    
    //svt angle: it's already with minus sign.
    float svtAngle = 30.5e-3;
    //Rotate the the input position automatically to match with the SVT tracker system
    
    //Rotated Helical Track Hit
    if (type == 0) {
        x_ = position[1];
        y_ = position[2];
        z_ = position[0];
    }
    else if (type==1) {
        x_ = position[0];
        y_ = position[1];
        z_ = position[2];
    }
    else {
        std::cout<<"ERROR::TrackerHit SetPosition"<<std::endl;
    }
    
    if (rotate) {
        
        double tmpx = x_;
        double tmpz = z_;
        
        x_ = tmpx*cos(svtAngle) - tmpz*sin(svtAngle);
        z_ = tmpx*sin(svtAngle) + tmpz*cos(svtAngle);
    }
        
    
    /*
    if (rotate)
      {
      //x_ = position[1];
        y_ = position[2];
        z_ = position[1] * sin(svtAngle) + position[0]*cos(svtAngle);
        x_ = position[1] * cos(svtAngle) - position[0]*sin(svtAngle);
    }
    else {
        x_ = position[1]; 
        y_ = position[2];
        z_ = position[0];
    }
    */
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
