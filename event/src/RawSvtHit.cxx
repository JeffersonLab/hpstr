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

void RawSvtHit::setFit(double fit[5]) {
    fit_[0] = fit[0]; 
    fit_[1] = fit[1]; 
    fit_[2] = fit[2]; 
    fit_[3] = fit[3]; 
    fit_[4] = fit[4]; 
}

void RawSvtHit::setSimPos(double hitPos[3]) {
    simPos_[0] = hitPos[0]; 
    simPos_[1] = hitPos[1]; 
    simPos_[2] = hitPos[2]; 
}

void RawSvtHit::setADCs(int adcs[6]) {
    adcs_[0] = adcs[0]; 
    adcs_[1] = adcs[1]; 
    adcs_[2] = adcs[2]; 
    adcs_[3] = adcs[3]; 
    adcs_[4] = adcs[4]; 
    adcs_[5] = adcs[5]; 
}

void RawSvtHit::setSystem(int system) {
    system_ = system; 
}

void RawSvtHit::setBarrel(int barrel) {
    barrel_ = barrel; 
}

void RawSvtHit::setLayer(int layer) {
    layer_ = layer; 
}

void RawSvtHit::setModule(int module) {
    module_ = module; 
}

void RawSvtHit::setSensor(int sensor) {
    sensor_ = sensor; 
}

void RawSvtHit::setSide(int side) {
    side_ = side; 
}

void RawSvtHit::setStrip(int strip) {
    strip_ = strip; 
}

void RawSvtHit::setSimEdep(double hitEdep) {
    simEdep_ = hitEdep; 
}

double * RawSvtHit::getFit() {
    return fit_;
}

double * RawSvtHit::getSimPos() {
    return simPos_;
}

int * RawSvtHit::getADCs() {
    return adcs_;
}

int RawSvtHit::getSystem() {
    return system_; 
}

int RawSvtHit::getBarrel() {
    return barrel_; 
}

int RawSvtHit::getLayer() {
    return layer_; 
}

int RawSvtHit::getModule() {
    return module_;
}

int RawSvtHit::getSensor() {
    return sensor_;
}

int RawSvtHit::getSide() {
    return side_;
}

int RawSvtHit::getStrip() {
    return strip_;
}

