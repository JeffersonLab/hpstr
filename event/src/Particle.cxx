/**
 * @file Particle.cxx
 * @brief Class used to encapsulate information about a particle. 
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Particle.h"

ClassImp(Particle)

Particle::Particle()
    : TObject() { 
}

Particle::~Particle() {
    Clear();
    delete tracks_; 
    delete clusters_; 
    delete particles_;
}

void Particle::Clear(Option_t* /* option */) {
    TObject::Clear();
    tracks_->Delete();
    clusters_->Delete(); 
    particles_->Delete();     
    n_daughters_ = 0;    
}

void Particle::addTrack(TObject* track) {
    tracks_->Add(track);
}

void Particle::addCluster(TObject* cluster) {
    clusters_->Add(static_cast<TObject*>(cluster));
}

void Particle::addParticle(Particle* particle) {
    ++n_daughters_;
    particles_->Add(static_cast<TObject*>(particle)); 
}

void Particle::setMomentum(const double* momentum) {
    px_ = momentum[0];
    py_ = momentum[1];
    pz_ = momentum[2];
}

std::vector<double> Particle::getMomentum() const { return { px_, py_, pz_ }; }

void Particle::setCorrMomentum(const double* momentum) {
    px_corr_ = momentum[0];
    py_corr_ = momentum[1];
    pz_corr_ = momentum[2];
}

std::vector<double> Particle::getCorrMomentum() const { return { px_, py_, pz_ }; };

void Particle::setVertexPosition(const float* vtx_pos) {
    vtx_x_ = static_cast<double>(vtx_pos[0]);
    vtx_y_ = static_cast<double>(vtx_pos[1]);
    vtx_z_ = static_cast<double>(vtx_pos[2]);
}

std::vector<double> Particle::getVertexPosition() const { 
    return { vtx_x_, vtx_y_, vtx_z_ }; 
}
