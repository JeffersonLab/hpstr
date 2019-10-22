/**
 * @file CalCluster.cxx
 * @brief Class used to encapsulate calorimeter cluster information.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "CalCluster.h"

ClassImp(CalCluster)

CalCluster::CalCluster()
    : TObject() { 
}

CalCluster::~CalCluster() {
    Clear();
    delete hits_;   
}

void CalCluster::Clear(Option_t* /*option*/) {
    TObject::Clear();
    hits_->Delete();
    seed_hit_ = nullptr; 
    n_hits_ = 0;
}

void CalCluster::setPosition(const float* position) {
    x_ = position[0];
    y_ = position[1];
    z_ = position[2];
}

void CalCluster::addHit(TObject* hit) { 
    ++n_hits_;
    hits_->Add(hit);
}
