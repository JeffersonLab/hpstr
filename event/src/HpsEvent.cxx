#include "HpsEvent.h"

HpsEvent::HpsEvent() {}

void HpsEvent::addCollection(const std::string name, TClonesArray* collection) {}
void HpsEvent::setTree(TTree* tree) {tree_ = tree;}
