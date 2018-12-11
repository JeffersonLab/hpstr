/**
 * @file CalorimeterHit.h
 * @brief Class that encapsulate calorimeter hit information
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "CalorimeterHit.h"

ClassImp(CalorimeterHit)

CalorimeterHit::CalorimeterHit() 
    : TObject() {
}

CalorimeterHit::~CalorimeterHit() {
    Clear(); 
}

void CalorimeterHit::Clear(Option_t* /* options */) {
    TObject::Clear(); 
}

void CalorimeterHit::setCrystalIndices(int index_x, int index_y) {
    index_x = index_x;
    index_y = index_y;
}
