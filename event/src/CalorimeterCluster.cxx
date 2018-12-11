/**
 * @file CalorimeterCluster.cxx
 * @brief Class used to encapsulate calorimeter cluster information.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "CalorimeterCluster.h"

ClassImp(CalorimeterCluster)

CalorimeterCluster::CalorimeterCluster()
    : TObject() { 
}

CalorimeterCluster::~CalorimeterCluster() {
    Clear();
    delete hits_;   
}

void CalorimeterCluster::Clear(Option_t* /*option*/) {
    TObject::Clear();
    hits_->Delete();
    seed_hit_ = nullptr; 
    n_hits_ = 0;
}

void CalorimeterCluster::setPosition(const float* position) {
    x_ = position[0];
    y_ = position[1];
    z_ = position[2];
}

void CalorimeterCluster::addHit(TObject* hit) { 
    ++n_hits_;
    hits_->Add(hit);
}
