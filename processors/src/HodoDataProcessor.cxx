/**
 *  @file   HodoDataProcessor.cxx
 *  @brief  Processor for the Hodoscope, to convert LCIO to the HodoCluster and HodoHit classes in event.
 *  @author Maurik Holtrop, University of New Hampshire, 11/13/19.
*/

#include "HodoDataProcessor.h"

void HodoDataProcessor::initialize(TTree* tree) {
    tree->Branch(HODO_HITS, &cal_hits_);
    tree->Branch(HODO_CLUSTERS, &clusters_);
}

bool HodoDataProcessor::process(IEvent* ievent) {
  
  // Clean up.
  for(int i = 0; i < cal_hits_.size(); i++) delete cal_hits_.at(i);
  for(int i = 0; i < clusters_.size(); i++) delete clusters_.at(i);
  cal_hits_.clear();
  clusters_.clear();

  // Interface to implementation casst. Not sure why we bother with an interface.
  Event* event = static_cast<Event*> (ievent);
  
  EVENT::LCCollection* hits{nullptr};
  try{
    hits =  static_cast<EVENT::LCCollection*>(event->getLCCollection(HODO_HITS));
  }catch(EVENT::DataNotAvailableException e){
    std::cout << "Barfed on not finding the hodoscope collections. \n";
  }
  
  std::cout << "Hodo n-hits: " << hits->getNumberOfElements() << std::endl;
  
  return true;
}

UTIL::BitFieldValue HodoDataProcessor::getIdentifierFieldValue(std::string field, EVENT::CalorimeterHit* hit){

    UTIL::BitField64 decoder(encoder_string_);
    long long value = (long long)( hit->getCellID0() & 0xffffffff ) | ( (long long)( hit->getCellID1() ) << 32 ) ;
    decoder.setValue(value);

    return decoder[field];
}
