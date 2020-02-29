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
}

void Particle::Clear(Option_t* /* option */) {
    TObject::Clear();
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
