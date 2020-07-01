
#include "CalCluster.h"
#include "CalHit.h"
#include "Event.h"
#include "EventHeader.h"
#include "HodoCluster.h"
#include "HodoHit.h"
#include "TriggerData.h"
#include "VTPData.h"
#include "TSData.h"
#include "Particle.h"
#include "MCParticle.h"
#include "Track.h"
#include "Vertex.h"
#include "TrackerHit.h"
#include "MCTrackerHit.h"
#include "MCEcalHit.h"
#include "RawSvtHit.h"

#include <variant>

/**
 * @type Collection
 */
typedef std::variant <
    EventHeader, 
    std::vector< CalCluster >,
    std::vector< CalHit >,
    std::vector< HodoCluster >, 
    std::vector< HodoHit >, 
    std::vector< TriggerData >, 
    std::vector< VTPData >, 
    std::vector< TSData >, 
    std::vector< Particle >, 
    std::vector< Track >, 
    std::vector< MCTrackerHit >, 
    std::vector< MCEcalHit >, 
    std::vector< RawSvtHit >,
    std::vector< Vertex >, 
    std::map< int, MCParticle > 
> Collection;
