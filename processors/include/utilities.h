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


    bool hasCollection(EVENT::LCEvent* lc_event,const std::string& collection);

    Vertex* buildVertex(EVENT::Vertex* lc_vertex);
    
    Particle* buildParticle(EVENT::ReconstructedParticle* lc_particle, 
                            EVENT::LCCollection* gbl_kink_data,
                            EVENT::LCCollection* track_data);

    Track* buildTrack(EVENT::Track* lc_track, 
            EVENT::LCCollection* gbl_kink_data, 
            EVENT::LCCollection* track_data);


    bool IsSameTrack(Track* trk1, Track* trk2);

    RawSvtHit* buildRawHit(EVENT::TrackerRawData* rawTracker_hit,
            EVENT::LCCollection* raw_svt_hit_fits);

    TrackerHit* buildTrackerHit(IMPL::TrackerHitImpl* lc_trackerHit,bool rotate=true, int type = 0);

    CalCluster* buildCalCluster(EVENT::Cluster* lc_cluster);

    bool addRawInfoTo3dHit(TrackerHit* tracker_hit,
                           IMPL::TrackerHitImpl* lc_tracker_hit,
                           EVENT::LCCollection* raw_svt_fits,
                           std::vector<RawSvtHit*>* rawHits = nullptr, int type = 0);


    bool isUsedByTrack(IMPL::TrackerHitImpl* lc_tracker_hit,
            EVENT::Track* lc_track);

    bool isUsedByTrack(TrackerHit* tracker_hit,
            EVENT::Track* lc_track);

    bool getParticlesFromVertex(Vertex* vtx, Particle* ele, Particle* pos);
    
    //TODO: extern?
    static UTIL::BitField64 decoder("system:6,barrel:3,layer:4,module:12,sensor:1,side:32:-2,strip:12");

}


#endif //UTILITIES
