#ifndef THREEPRONGHISTOS_H
#define THREEPRONGHISTOS_H

#include "HistoManager.h"
#include "Track.h"
#include "CalCluster.h"
#include "Vertex.h"
#include "Particle.h"
//#include "AnaHelpers.h"
#include <string>
#include <vector>

class ThreeProngHistos : public HistoManager {

 public:
    
 ThreeProngHistos(const std::string& inputName) : 
    HistoManager(inputName) 
    {
        m_name = inputName;
        //	_ah =  std::make_shared<AnaHelpers>();
        //        BuildAxes();
    }
    
    virtual void Define3DHistos(){};
    virtual void Define2DHistos(){};
    

    void BuildAxes(){};
    
    void FillThreeProngPlots(Particle* ele, 	Particle* pos,  Particle* rec,float weight=1.0);
 private:
    // Vertices
    //        std::vector<std::string> vPs{"vtx_chi2", "vtx_X", "vtx_Y", "vtx_Z", "vtx_sigma_X","vtx_sigma_Y","vtx_sigma_Z","vtx_InvM","vtx_InvMErr"};
    
    //bool doTrkCompPlots{false};
    
};

#endif //TRACKEFFIC_H
