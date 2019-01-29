/**
 * @file RawSvtHit.cxx
 * @brief Class used to encapsulate raw svt hit information
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 */

#include "RawSvtHit.h"

ClassImp(RawSvtHit)

RawSvtHit::RawSvtHit()
    : TObject() { 
    }

RawSvtHit::~RawSvtHit() { 
    Clear(); 
}

void RawSvtHit::Clear() { 
    TObject::Clear(); 
}

void RawSvtHit::setADCs(int adcs[6]) {
    adcs_[0] = adcs[0]; 
    adcs_[1] = adcs[1]; 
    adcs_[2] = adcs[2]; 
    adcs_[3] = adcs[3]; 
    adcs_[4] = adcs[4]; 
    adcs_[5] = adcs[5]; 
}

