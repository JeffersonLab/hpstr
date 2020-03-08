/**
 *  @file   HodoDataProcessor.cxx
 *  @brief  Processor for the Hodoscope, to convert LCIO to the HodoCluster and HodoHit classes in event.
 *  @author Maurik Holtrop, University of New Hampshire, 11/13/19.
 */

#include "HodoDataProcessor.h"

void HodoDataProcessor::initialize(TTree* tree) {
  tree->Branch(hitCollRoot_.c_str(), &hits_);
  tree->Branch(clusCollRoot_.c_str(), &clusters_);
}


void HodoDataProcessor::configure(const ParameterSet& parameters) {
  
  try
  {
    debug_         = parameters.getInteger("debug",debug_);
    hitCollLcio_   = parameters.getString("hitCollLcio",hitCollLcio_);
    hitCollRoot_   = parameters.getString("hitCollRoot",hitCollRoot_);
    clusCollLcio_  = parameters.getString("clusCollLcio",clusCollLcio_);
    clusCollRoot_  = parameters.getString("clusCollRoot",clusCollRoot_);
  }
  catch (std::runtime_error& error)
  {
    std::cout << error.what() << std::endl;
  }
}


bool HodoDataProcessor::process(IEvent* ievent) {
  
  // Clean up. TODO:: new/delete is an inefficient way to go about all this. Fix with better memory use.
  for(int i = 0; i < hits_.size(); i++) delete hits_.at(i);
  for(int i = 0; i < clusters_.size(); i++) delete clusters_.at(i);
  hits_.clear();
  clusters_.clear();
  
  // Interface to implementation cast. Not sure why we bother with an interface.
  Event* event = static_cast<Event*> (ievent);
  
  EVENT::LCCollection* lcio_hits{nullptr};
//  EVENT::LCCollection* lcio_hits_generic{nullptr};
  EVENT::LCCollection* lcio_clus_generic{nullptr};
  
  try{
    lcio_hits =  static_cast<EVENT::LCCollection*>(event->getLCCollection(hitCollLcio_));
  }catch(EVENT::DataNotAvailableException e){
    if(debug_ > 0) std::cout << "Barfed on not finding the hodoscope collections. \n";
    return false;
  }
      
  // Loop through the hits and add them to the event.
  for(int i=0; i< lcio_hits->getNumberOfElements(); ++i){
    // Grab the hit from the collection and push it as a HodoHit object onto the vector<HodoHit *> hits_
    IMPL::CalorimeterHitImpl *hit=static_cast<IMPL::CalorimeterHitImpl *>(lcio_hits->getElementAt(i));

    // TODO: This is inefficient, doing a new and delete on every hit for every event ==> implement proper memory use.
    hits_.push_back(new HodoHit(
                                getIdentifierFieldValue("ix", hit),
                                getIdentifierFieldValue("iy", hit),
                                getIdentifierFieldValue("layer", hit),
                                getIdentifierFieldValue("hole", hit),
                                hit->getEnergy(),
                                hit->getTime()
                                )
                    );
  }
  
  // Now deal with the clusters.
  
  try{
    lcio_clus_generic =  static_cast<EVENT::LCCollection*>(event->getLCCollection(clusCollLcio_));
  }catch(EVENT::DataNotAvailableException e){
    if(debug_ > 0) std::cout << "Barfed on not finding the generic hodoscope cluster collections. \n";
    return false;
  }

  IMPL::LCGenericObjectImpl *gclus_ix = static_cast<IMPL::LCGenericObjectImpl *>(lcio_clus_generic->getElementAt(0));
  IMPL::LCGenericObjectImpl *gclus_iy = static_cast<IMPL::LCGenericObjectImpl *>(lcio_clus_generic->getElementAt(1));
  IMPL::LCGenericObjectImpl *gclus_layer = static_cast<IMPL::LCGenericObjectImpl *>(lcio_clus_generic->getElementAt(2));
  IMPL::LCGenericObjectImpl *gclus_energy = static_cast<IMPL::LCGenericObjectImpl *>(lcio_clus_generic->getElementAt(3));
  IMPL::LCGenericObjectImpl *gclus_time = static_cast<IMPL::LCGenericObjectImpl *>(lcio_clus_generic->getElementAt(4));
//  IMPL::LCGenericObjectImpl *gclus_ids = static_cast<IMPL::LCGenericObjectImpl *>(lcio_hits_generic->getElementAt(5));
    
  for(int i=0; i < gclus_ix->getNInt(); ++i){
    clusters_.push_back(new HodoCluster(
                                        gclus_ix->getIntVal(i),
                                        gclus_iy->getIntVal(i),
                                        gclus_layer->getIntVal(i),
                                        gclus_energy->getDoubleVal(i),
                                        gclus_time->getDoubleVal(i)));
  }
  
  return true;
}

UTIL::BitFieldValue HodoDataProcessor::getIdentifierFieldValue(std::string field, EVENT::CalorimeterHit* hit){
  
  UTIL::BitField64 decoder(encoder_string_);
  long long value = (long long)( hit->getCellID0() & 0xffffffff ) | ( (long long)( hit->getCellID1() ) << 32 ) ;
  decoder.setValue(value);
  
  return decoder[field];
}

DECLARE_PROCESSOR(HodoDataProcessor); 
