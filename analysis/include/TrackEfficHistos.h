#ifndef TRACKEFFICHISTOS_H
#define TRACKEFFICHISTOS_H

#include "HistoManager.h"
#include "Track.h"
#include "CalCluster.h"
#include "Vertex.h"
#include "Particle.h"
#include "AnaHelpers.h"
#include <string>
#include <vector>

class TrackEfficHistos : public HistoManager {

 public:
    
 TrackEfficHistos(const std::string& inputName, std::shared_ptr<AnaHelpers> ah) : 
    HistoManager(inputName) 
    {
        m_name = inputName;
        _ah=ah;
        //	_ah =  std::make_shared<AnaHelpers>();
        //        BuildAxes();
    }
    
    virtual void Define3DHistos(){};
    virtual void Define2DHistos(){};
    

    void BuildAxes(){};
    
    void FillEffPlots(std::pair<CalCluster*,Track*> ele, std::pair<CalCluster*,Track*> pos, float weight = 1.);
    void FillPreSelectionPlots(CalCluster*, float weight=1.);
    void FillPairSelectionPlots(CalCluster* eleClu, CalCluster* posClu, 
				float weight); 
    std::pair<CalCluster*, Track*> getClusterTrackPair(CalCluster* cluster,std::vector<Track*>& tracks, float weight=1.0);
    void FillEffPlots(Particle* ele, 	Particle* pos,  float weight=1.0);
  void SetCalTimeOffset(double offset){
    std::cout<<"setting TrackEfficHistos::timeOffset_ = "<<offset<<std::endl;
    timeOffset_ = offset;
  };

  
private:
  std::shared_ptr<AnaHelpers> _ah ;
  double timeOffset_{0.0}; 
  // Vertices
    //        std::vector<std::string> vPs{"vtx_chi2", "vtx_X", "vtx_Y", "vtx_Z", "vtx_sigma_X","vtx_sigma_Y","vtx_sigma_Z","vtx_InvM","vtx_InvMErr"};
    
    //bool doTrkCompPlots{false};
    
};

#endif //TRACKEFFIC_H
