/**
 * @file EventHeader.cxx
 * @brief Class used to encapsulate event information.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "EventHeader.h"

ClassImp(EventHeader)

EventHeader::EventHeader()
    : TObject() { 
}

EventHeader::~EventHeader() {
    Clear();
}

void EventHeader::Clear(Option_t* /* option */) {
    TObject::Clear(); 
}
