#include "FindableTrack.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <iostream>

ClassImp(FindableTrack)

FindableTrack::FindableTrack(int lcio_id, int hit_count, bool is_findable) 
    : lcio_id_(lcio_id), hit_count_(hit_count), is_findable_(is_findable) { 

}

FindableTrack::~FindableTrack() {}

void FindableTrack::Print(Option_t* option) const { 
    std::cout << "FindableTrack { LCIO ID: " << lcio_id_ 
              << " Hit count: " << hit_count_ << " } " << std::endl;
}
