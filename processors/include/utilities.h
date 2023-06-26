#ifndef _UTILITIES_
#define _UTILITIES_

#include <EVENT/LCCollection.h>
#include <EVENT/Track.h>
#include <EVENT/ReconstructedParticle.h>
#include <EVENT/Vertex.h>
#include <EVENT/TrackerHit.h>
#include <EVENT/TrackState.h>
#include <EVENT/TrackerRawData.h>
#include <EVENT/CalorimeterHit.h>
#include <EVENT/Cluster.h>
#include <IMPL/CalorimeterHitImpl.h>
#include <IMPL/LCGenericObjectImpl.h>
#include <IMPL/TrackerHitImpl.h>
#include <IMPL/ClusterImpl.h>
#include <UTIL/LCRelationNavigator.h>
#include <UTIL/BitField64.h>

#include <vector>

//-----------//
//   hpstr   //
//-----------//
#include "Collections.h"
#include "Processor.h"
#include "Particle.h"
#include "Track.h"
#include "Vertex.h"
#include "RawSvtHit.h"
#include "CalCluster.h"
#include "CalHit.h"
#include "Event.h"
#include "TrackerHit.h"

namespace utils {
    /**
     * @brief description
     * 
     * @param lc_event 
     * @param collection 
     * @return true 
     * @return false 
     */
    bool hasCollection(EVENT::LCEvent* lc_event,const std::string& collection);

    /**
     * @brief description
     * 
     * @param lc_vertex 
     * @return Vertex* 
     */
    Vertex* buildVertex(EVENT::Vertex* lc_vertex);
    
    /**
     * @brief description
     * 
     * @param lc_particle 
     * @param gbl_kink_data 
     * @param track_data 
     * @return Particle* 
     */
    Particle* buildParticle(EVENT::ReconstructedParticle* lc_particle, 
                            std::string trackstate_location,
                            EVENT::LCCollection* gbl_kink_data,
                            EVENT::LCCollection* track_data);

    /**
     * @brief description
     * 
     * @param lc_track 
     * @param gbl_kink_data 
     * @param track_data 
     * @return Track* 
     */
    Track* buildTrack(EVENT::Track* lc_track, 
                      std::string trackstate_location,
                      EVENT::LCCollection* gbl_kink_data, 
                      EVENT::LCCollection* track_data);

    /**
     * @brief description
     * 
     * @param trk1 
     * @param trk2 
     * @return true 
     * @return false 
     */
    bool IsSameTrack(Track* trk1, Track* trk2);

    RawSvtHit* buildRawHit(EVENT::TrackerRawData* rawTracker_hit,
                           EVENT::LCCollection* raw_svt_hit_fits);

    /**
     * @brief description
     * 
     * @param lc_trackerHit 
     * @param rotate 
     * @param type 
     * @return TrackerHit* 
     */
    TrackerHit* buildTrackerHit(IMPL::TrackerHitImpl* lc_trackerHit,bool rotate=true, int type = 0);

    /**
     * @brief description
     * 
     * @param lc_cluster 
     * @return CalCluster* 
     */
    CalCluster* buildCalCluster(EVENT::Cluster* lc_cluster);

    /**
     * @brief description
     * 
     * @param tracker_hit 
     * @param lc_tracker_hit 
     * @param raw_svt_fits 
     * @param rawHits 
     * @param type 
     * @return true 
     * @return false 
     */
    bool addRawInfoTo3dHit(TrackerHit* tracker_hit,
                           IMPL::TrackerHitImpl* lc_tracker_hit,
                           EVENT::LCCollection* raw_svt_fits,
                           std::vector<RawSvtHit*>* rawHits = nullptr, int type = 0, bool storeRawHit = true);


    /**
     * @brief description
     * 
     * @param lc_tracker_hit 
     * @param lc_track 
     * @return true 
     * @return false 
     */
    bool isUsedByTrack(IMPL::TrackerHitImpl* lc_tracker_hit,
                       EVENT::Track* lc_track);

    /**
     * @brief description
     * 
     * @param tracker_hit 
     * @param lc_track 
     * @return true 
     * @return false 
     */
    bool isUsedByTrack(TrackerHit* tracker_hit,
                       EVENT::Track* lc_track);

    /**
     * @brief description
     * 
     * @param vtx 
     * @param ele 
     * @param pos 
     * @return true 
     * @return false 
     */
    bool getParticlesFromVertex(Vertex* vtx, Particle* ele, Particle* pos);
    
    /**
     * @brief description
     * 
     * \todo extern?
     */
    static UTIL::BitField64 decoder("system:6,barrel:3,layer:4,module:12,sensor:1,side:32:-2,strip:12");

}

#endif //UTILITIES
