/** 
 * @file Vertex.h
 * @brief Class used to encapsulate vertex information.
 * @author PF, SLAC
 */

#include "Vertex.h"

ClassImp(Vertex)

Vertex::Vertex()
    : TObject() {
    }

Vertex::~Vertex() {
    Clear();
    delete tracks_;
}

void Vertex::Clear(Option_t *option) {
    TObject::Clear();
    if (tracks_)
        tracks_->Delete();
    pos_.Clear();
    p1_.Clear();
    p2_.Clear();
    ntracks_ = 0;
    x_ = -999.;
    y_ = -999.;
    z_ = -999.;
}

bool Vertex::vxTracksAvailable() const {

    if (tracks_)
        return true;

    return false;
}

void Vertex::setCovariance ( const std::vector<float>* vec) {
}

