/**
 * @file MCParticle.cxx
 * @brief Class used to encapsulate information about a particle. 
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 */

#include "MCParticle.h"

ClassImp(MCParticle)

MCParticle::MCParticle()
    : TObject() { 
}

MCParticle::~MCParticle() {
    Clear();
    delete daughters_;
}

void MCParticle::Clear(Option_t* /* option */) {
    TObject::Clear();
    daughters_->Delete();     
    n_daughters_ = 0;    
}

void MCParticle::addDaughter(MCParticle* particle) {
    ++n_daughters_;
    daughters_->Add(static_cast<TObject*>(particle)); 
}

void MCParticle::setMomentum(const double* momentum) {
    px_ = momentum[0];
    py_ = momentum[1];
    pz_ = momentum[2];
}

std::vector<double> MCParticle::getMomentum() const { return { px_, py_, pz_ }; }

void MCParticle::setVertexPosition(const double* vtx_pos) {
    vtx_x_ = static_cast<double>(vtx_pos[0]);
    vtx_y_ = static_cast<double>(vtx_pos[1]);
    vtx_z_ = static_cast<double>(vtx_pos[2]);
}

std::vector<double> MCParticle::getVertexPosition() const { 
    return { vtx_x_, vtx_y_, vtx_z_ }; 
}
