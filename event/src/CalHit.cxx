/**
 * @file CalHit.cxx
 * @brief Class that encapsulate calorimeter hit information
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "CalHit.h"

ClassImp(CalHit)

CalHit::CalHit() 
    : TObject() {
}

CalHit::~CalHit() {
    Clear(); 
}

void CalHit::Clear(Option_t* /* options */) {
    TObject::Clear(); 
}

void CalHit::setCrystalIndices(int index_x, int index_y) {
    index_x_ = index_x;
    index_y_ = index_y;
}
