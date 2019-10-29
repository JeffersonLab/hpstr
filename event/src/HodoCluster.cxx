/**
 * @file HodoCluster.cxx
 * @brief Class used to encapsulate hodoscope cluster information.
 * @author Maurik Holtrop, University of New Hampshire
 */

#include "HodoCluster.h"

ClassImp(HodoCluster)

HodoCluster::~HodoCluster() {
    Clear();
    delete hits_;   
}

void HodoCluster::Clear(Option_t* /*option*/) {
    TObject::Clear();
    hits_->Delete();
    n_hits_ = 0;
}

void HodoCluster::addHit(TObject* hit) {
    ++n_hits_;
    hits_->Add(hit);
}
