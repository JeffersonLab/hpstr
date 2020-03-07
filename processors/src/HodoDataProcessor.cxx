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
  
  // Interface to implementation casst. Not sure why we bother with an interface.
  Event* event = static_cast<Event*> (ievent);
  
  EVENT::LCCollection* lcio_hits{nullptr};
  EVENT::LCCollection* lcio_hits_generic{nullptr};
  EVENT::LCCollection* lcio_clus_generic{nullptr};
  
  try{
    lcio_hits =  static_cast<EVENT::LCCollection*>(event->getLCCollection(hitCollLcio_));
  }catch(EVENT::DataNotAvailableException e){
    if(debug_ > 0) std::cout << "Barfed on not finding the hodoscope collections. \n";
    return false;
  }
  
  try{
    lcio_hits_generic =  static_cast<EVENT::LCCollection*>(event->getLCCollection(hitCollLcio_Generic_));
  }catch(EVENT::DataNotAvailableException e){
    if(debug_ > 0) std::cout << "Barfed on not finding the generic hodoscope collections. \n";
    return false;
  }
  
  IMPL::LCGenericObjectImpl *ghit_ix = static_cast<IMPL::LCGenericObjectImpl *>(lcio_hits_generic->getElementAt(0));
  IMPL::LCGenericObjectImpl *ghit_iy = static_cast<IMPL::LCGenericObjectImpl *>(lcio_hits_generic->getElementAt(1));
  IMPL::LCGenericObjectImpl *ghit_layer = static_cast<IMPL::LCGenericObjectImpl *>(lcio_hits_generic->getElementAt(2));
  IMPL::LCGenericObjectImpl *ghit_hole = static_cast<IMPL::LCGenericObjectImpl *>(lcio_hits_generic->getElementAt(3));
  IMPL::LCGenericObjectImpl *ghit_energy = static_cast<IMPL::LCGenericObjectImpl *>(lcio_hits_generic->getElementAt(4));
  IMPL::LCGenericObjectImpl *ghit_time = static_cast<IMPL::LCGenericObjectImpl *>(lcio_hits_generic->getElementAt(5));
//  IMPL::LCGenericObjectImpl *ghit_ids = static_cast<IMPL::LCGenericObjectImpl *>(lcio_hits_generic->getElementAt(6));
  
  if( debug_ >2 ){
    std::cout << "Hodo n-hits: " << lcio_hits->getNumberOfElements() << ", " << ghit_ix->getNInt() << std::endl;
    for(int i=0; i< lcio_hits->getNumberOfElements(); ++i){
      IMPL::CalorimeterHitImpl *hit=static_cast<IMPL::CalorimeterHitImpl *>(lcio_hits->getElementAt(i));
      
      int ix = getIdentifierFieldValue("ix", hit);
      int iy = getIdentifierFieldValue("iy", hit);
      int layer = getIdentifierFieldValue("layer", hit);
      int hole = getIdentifierFieldValue("hole", hit);
      
      if( ix != ghit_ix->getIntVal(i) || iy != ghit_iy->getIntVal(i) || layer != ghit_layer->getIntVal(i) || hole != ghit_hole->getIntVal(i) ){
        std::cout << "IDs not equal!!\n";
        printf("ID1 (ix,iy,layer,hole) = (%3d,%3d,%3d,%3d)\n",ghit_ix->getIntVal(i),ghit_iy->getIntVal(i),ghit_layer->getIntVal(i),ghit_hole->getIntVal(i));
        printf("ID2 (ix,iy,layer,hole) = (%3d,%3d,%3d,%3d)\n",ix,iy,layer,hole);
      }
      
      
      if( abs(hit->getEnergy() - ghit_energy->getDoubleVal(i)) > 0.01 ){
        std::cout << "Energies are different !\n";
      }
      if( abs(hit->getTime() - ghit_time->getDoubleVal(i)) > 0.01 ){
        std::cout << "Times are different !\n";
      }
    }
  }
  
  // Loop through the hits and add them to the event.
  for(int i=0; i< ghit_ix->getNInt(); ++i){
    
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

  if(debug_ > 1){
      if( gclus_iy->getNInt() != gclus_ix->getNInt() ||
         gclus_layer->getNInt() != gclus_ix->getNInt() ||
         gclus_energy->getNDouble() != gclus_ix->getNInt() ||
         gclus_time->getNDouble() != gclus_ix->getNInt()
         ){
        std::cout << "Cluster not read properly.\n";
        std::cout << "N ix    = " << gclus_ix->getNInt() << std::endl;
        std::cout << "N iy    = " << gclus_iy->getNInt() << std::endl;
        std::cout << "N layer = " << gclus_layer->getNInt() << std::endl;
        std::cout << "N energ = " << gclus_energy->getNDouble() << std::endl;
        std::cout << "N time  = " << gclus_time->getNDouble() << std::endl;
      }

    std::cout << "ix: ";
    for(int i=0; i < gclus_ix->getNInt(); ++i){
      std::cout << gclus_ix->getIntVal(i) << "; ";
    }
    std::cout << std::endl;
    
    std::cout << "iy: ";
    for(int i=0; i < gclus_iy->getNInt(); ++i){
      std::cout << gclus_iy->getIntVal(i) << "; ";
    }
    std::cout << std::endl;
    
    std::cout << "layer: ";
    for(int i=0; i < gclus_layer->getNInt(); ++i){
      std::cout << gclus_layer->getIntVal(i) << "; ";
    }
    std::cout << std::endl;
    
    std::cout << "energy: ";
    for(int i=0; i < gclus_energy->getNDouble(); ++i){
      std::cout << gclus_energy->getDoubleVal(i) << "; ";
    }
    std::cout << std::endl;
    
    std::cout << "time:   ";
    for(int i=0; i < gclus_time->getNDouble(); ++i){
      std::cout << gclus_time->getDoubleVal(i) << "; ";
    }
    std::cout << std::endl;

  }
    
//  for(int i=0; i < gclus_ix->getNInt(); ++i){
//    clusters_.push_back(new HodoCluster(
//                                        gclus_ix->getIntVal(i),
//                                        gclus_iy->getIntVal(i),
//                                        gclus_layer->getIntVal(i),
//                                        gclus_energy->getDoubleVal(i),
//                                        gclus_time->getDoubleVal(i)));
//  }
  
  return true;
}

UTIL::BitFieldValue HodoDataProcessor::getIdentifierFieldValue(std::string field, EVENT::CalorimeterHit* hit){
  
  UTIL::BitField64 decoder(encoder_string_);
  long long value = (long long)( hit->getCellID0() & 0xffffffff ) | ( (long long)( hit->getCellID1() ) << 32 ) ;
  decoder.setValue(value);
  
  return decoder[field];
}

DECLARE_PROCESSOR(HodoDataProcessor); 
