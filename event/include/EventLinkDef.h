#ifdef __CINT__

#pragma link off all globals; 
#pragma link off all classes; 
#pragma link off all functions; 

#pragma link C++ nestedclasses;

#pragma link C++ class CalCluster+;
#pragma link C++ class CalHit+;
#pragma link C++ class EventHeader+; 
#pragma link C++ class Particle+; 
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
#endif
