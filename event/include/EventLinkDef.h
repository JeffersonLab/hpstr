#ifdef __CINT__

#pragma link off all globals; 
#pragma link off all classes; 
#pragma link off all functions; 

#pragma link C++ nestedclasses;

#pragma link C++ class CalCluster+;
#pragma link C++ class CalHit+;
#pragma link C++ class EventHeader+; 
#pragma link C++ class TriggerData+;
#pragma link C++ class VTPData+;
#pragma link C++ class VTPData::bHeader+;
#pragma link C++ class VTPData::bTail+;
#pragma link C++ class VTPData::eHeader+;
#pragma link C++ class VTPData::hpsCluster+;
#pragma link C++ class VTPData::hpsSingleTrig+;
#pragma link C++ class VTPData::hpsPairTrig+;
#pragma link C++ class VTPData::hpsCalibTrig+;
#pragma link C++ class VTPData::hpsClusterMult+;
#pragma link C++ class VTPData::hpsFEETrig+;
#pragma link C++ class TSData+;
#pragma link C++ class TSData::tsHeader+;
#pragma link C++ class TSData::tsBits+;
#pragma link C++ class Particle+; 
#pragma link C++ class MCParticle+; 
#pragma link C++ class Track+;
#pragma link C++ class Vertex+;
#pragma link C++ class TrackerHit+;
#pragma link C++ class RawSvtHit+;

// This is to create the dictionary for stl containers
#pragma link C++ class vector<TObject>     +;
#pragma link C++ class vector<TObject*>    +;
#pragma link C++ class vector<RawSvtHit>   +;
#pragma link C++ class vector<RawSvtHit*>  +;
#pragma link C++ class vector<TrackerHit>  +;
#pragma link C++ class vector<TrackerHit*> +;
#pragma link C++ class vector<Track>       +;
#pragma link C++ class vector<Track*>      +;
#pragma link C++ class vector<Vertex*>     +;
#pragma link C++ class vector<Vertex>      +;
#pragma link C++ class vector<Particle>      +;
#pragma link C++ class vector<Particle*>      +;
#pragma link C++ class vector<MCParticle>      +;
#pragma link C++ class vector<MCParticle*>      +;
#pragma link C++ class vector<VTPData::hpsCluster>      +;
#pragma link C++ class vector<VTPData::hpsSingleTrig>   +;
#pragma link C++ class vector<VTPData::hpsPairTrig>     +;
#pragma link C++ class vector<VTPData::hpsCalibTrig>    +;
#pragma link C++ class vector<VTPData::hpsClusterMult>  +;
#pragma link C++ class vector<VTPData::hpsFEETrig>      +;
#pragma link C++ class vector<TSData::tsBits>           +;
#endif
