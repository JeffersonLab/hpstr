/**
 * @file ECalDataProcessor.cxx
 * @brief Processor used to convert ECal LCIO data to ROOT. 
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "ECalDataProcessor.h"
#include "Event.h"

ECalDataProcessor::ECalDataProcessor(const std::string& name, Process& process)
    : Processor(name, process) { 
    }

ECalDataProcessor::~ECalDataProcessor() { 
}

void ECalDataProcessor::initialize(TTree* tree) {
    tree->Branch(Collections::ECAL_HITS, &cal_hits_);
    tree->Branch(Collections::ECAL_CLUSTERS, &clusters_);
}

bool ECalDataProcessor::process(IEvent* ievent) {

    for(int i = 0; i < cal_hits_.size(); i++) delete cal_hits_.at(i);
    for(int i = 0; i < clusters_.size(); i++) delete clusters_.at(i);
    cal_hits_.clear();
    clusters_.clear();
    // Attempt to retrieve the collection "TimeCorrEcalHits" from the event. If
    // the collection doesn't exist, handle the DataNotAvailableCollection and
    // attempt to retrieve the collection "EcalCalHits". If that collection 
    // doesn't exist, the DST maker will fail.

    //dynamic cast
    Event* event = static_cast<Event*> (ievent);
    EVENT::LCCollection* hits{nullptr};
    std::string hits_coll_name = Collections::ECAL_TIME_CORR_HITS; 
    try { 
        hits =  static_cast<EVENT::LCCollection*>(event->getLCCollection(hits_coll_name)); 
    } catch (EVENT::DataNotAvailableException e) { 
        hits_coll_name = Collections::ECAL_HITS; 
        hits =  static_cast<EVENT::LCCollection*>(event->getLCCollection(hits_coll_name)); 
    }

    // A calorimeter hit
    IMPL::CalorimeterHitImpl* lc_hit{nullptr}; 

    // Get the collection of Ecal hits from the event.
    std::map< std::pair<int,int>, CalHit*> hit_map;

    // Loop through all of the hits and add them to event.
    for (int ihit=0; ihit < hits->getNumberOfElements(); ++ihit) {

        // Get the ith hit from the LC Event.  
        IMPL::CalorimeterHitImpl* lc_hit 
            = static_cast<IMPL::CalorimeterHitImpl*>(hits->getElementAt(ihit));

        // Get the unique cell id of this hit. Combine it with the integer time,
        // since a crystal can be hit more than once.
        int id0 = lc_hit->getCellID0();

        // 0.1 ns resolution is sufficient to distinguish any 2 hits on the same crystal.
        int id1 = static_cast<int>(10.0*lc_hit->getTime()); 

        CalHit* cal_hit = new CalHit();

        // Store the hit in the map for easy access later.
        hit_map[ std::make_pair(id0,id1) ] = cal_hit;

        // Set the energy of the Ecal hit
        cal_hit->setEnergy(lc_hit->getEnergy());

        // Set the hit time of the Ecal hit
        cal_hit->setTime(lc_hit->getTime());

        // Set the indices of the crystal
        int index_x = this->getIdentifierFieldValue("ix", lc_hit);
        int index_y = this->getIdentifierFieldValue("iy", lc_hit);

        cal_hit->setCrystalIndices(index_x, index_y);
        cal_hits_.push_back(cal_hit);

    }
    
    // Get the collection of Ecal clusters from the event
    EVENT::LCCollection* clusters 
        = static_cast<EVENT::LCCollection*>(event->getLCCollection(Collections::ECAL_CLUSTERS));
    
    // Loop over all clusters and fill the event
    for(int icluster = 0; icluster < clusters->getNumberOfElements(); ++icluster) {
        
        // Get an Ecal cluster from the LCIO collection
        IMPL::ClusterImpl* lc_cluster = static_cast<IMPL::ClusterImpl*>(clusters->getElementAt(icluster));

        // Add a cluster to the event
        CalCluster* cluster = new CalCluster();

        // Set the cluster position
        cluster->setPosition(lc_cluster->getPosition());

        // Set the cluster energy
        cluster->setEnergy(lc_cluster->getEnergy());
        
        // Get the ecal hits used to create the cluster
        EVENT::CalorimeterHitVec lc_hits = lc_cluster->getCalorimeterHits();

        // Loop over all of the Ecal hits and add them to the Ecal cluster.  The
        // seed hit is set to be the hit with the highest energy.  The cluster time
        // is set to be the hit time of the seed hit.
        double senergy = 0; 
        double stime = 0; 
        CalHit* seed_hit{nullptr}; 
        for(int ihit = 0; ihit < (int) lc_hits.size(); ++ihit) {
            
            // Get an Ecal hit
            lc_hit  = static_cast<IMPL::CalorimeterHitImpl*>(lc_hits[ihit]); 
        
            int id0=lc_hit->getCellID0();
            int id1=(int)(10.0*lc_hit->getTime());

            if (hit_map.find(std::make_pair(id0,id1)) == hit_map.end()) {
                throw std::runtime_error("[ EcalDataProcessor ]: Hit not found in map, but is in the cluster."); 
            } else {
                // Get the hit and add it to the cluster
                CalHit* cal_hit = hit_map[std::make_pair(id0,id1)];
                cluster->addHit(cal_hit);
            
                if (senergy < lc_hit->getEnergy()) { 
                    senergy = lc_hit->getEnergy(); 
                    seed_hit = cal_hit; 
                    stime = cal_hit->getTime(); 
                } 
            }
        }

        // Set the time of the cluster
        cluster->setTime(stime); 

        // Set the cluster seed. 
        cluster->setSeed(seed_hit); 
        clusters_.push_back(cluster);
    }

    return true;
}

void ECalDataProcessor::finalize() { 
}

UTIL::BitFieldValue ECalDataProcessor::getIdentifierFieldValue(std::string field, EVENT::CalorimeterHit* hit){

    UTIL::BitField64 decoder(encoder_string_);
    long64 value = long64( hit->getCellID0() & 0xffffffff ) | ( long64( hit->getCellID1() ) << 32 ) ;
    decoder.setValue(value); 

    return decoder[field]; 
}

DECLARE_PROCESSOR(ECalDataProcessor); 
