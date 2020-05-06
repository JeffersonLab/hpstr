#ifndef TRACKEFFICHISTOS_H
#define TRACKEFFICHISTOS_H

#include "HistoManager.h"
#include "Track.h"
#include "CalCluster.h"
#include "Vertex.h"
#include "Particle.h"
#include <string>
#include <vector>

class TrackEfficHistos : public HistoManager {

 public:
    
 TrackEfficHistos(const std::string& inputName) : 
    HistoManager(inputName) 
    {
        m_name = inputName;
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

 private:
    
    // Vertices
    //        std::vector<std::string> vPs{"vtx_chi2", "vtx_X", "vtx_Y", "vtx_Z", "vtx_sigma_X","vtx_sigma_Y","vtx_sigma_Z","vtx_InvM","vtx_InvMErr"};
    
    //bool doTrkCompPlots{false};
    
};

#endif //TRACKEFFIC_H
