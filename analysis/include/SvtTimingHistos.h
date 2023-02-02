#ifndef SVTTIMING_ANAHISTOS_H
#define SVTTIMING_ANAHISTOS_H

// HPSTR
#include "HistoManager.h"
#include "RawSvtHit.h"
#include "TrackerHit.h"
#include "Track.h"
#include "CalHit.h"
#include "CalCluster.h"
#include "ModuleMapper.h"
#include <string>
#include <vector>
#include <map>

class SvtTimingHistos : public HistoManager {

 public:
    
    SvtTimingHistos(const std::string& inputName,ModuleMapper* mmapper) ;
    
    virtual void Define3DHistos(){};
    virtual void Define2DHistos(){};
    virtual void Define1DHistos();
    void DefineHistos();

    void FillRawHits(std::vector<RawSvtHit*> *trkrHits,float weight = 1.);
    void FillTrackerHits(std::vector<TrackerHit*> *trkrHits, float weight = 1.);
    void FillTracks(std::vector<Track*> *tracks, float weight = 1.);
    void FillHitsOnTrack(Track* track,  std::map<std::string,double> *timingCalib, int phase, float weight =1. );
    void FillEcalHits(std::vector<CalHit*> *ecalHits, float weight = 1.);
    void FillEcalClusters(std::vector<CalCluster*> *ecalClusters, float weight = 1.);
    
 private:
    
    ModuleMapper* mmapper_;
};

#endif //SVTTIMING_ANAHISTOS_H
