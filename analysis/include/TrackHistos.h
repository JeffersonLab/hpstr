#ifndef TRACKHISTOS_H
#define TRACKHISTOS_H

#include "HistoManager.h"
#include "Track.h"
#include <string>
#include <vector>

class TrackHistos : public HistoManager {

 public:
 
 TrackHistos(const std::string& inputName) : 
  HistoManager(inputName) 
  {m_name = inputName;
    BuildAxes();
  }
  
  virtual void Define3DHistos(){};
  virtual void Define2DHistos();
  virtual void Define1DHistos();

  void BuildAxes();

  void Fill1DHistograms(Track* track, float weight = 1.);

  //track_x goes for x axis, and y for y axis
  void FillTrackComparisonHistograms(Track* track_x, Track* track_y, float weight = 1.);
  void doTrackComparisonPlots(bool doplots) {doTrkCompPlots = doplots;};
  
 private:
  std::vector<std::string> tPs{"d0","Phi","Omega","TanLambda","Z0","time","chi2"};
  std::map<std::string, std::vector<float> > axes; 
  bool doTrkCompPlots{false};
  
};

#endif //TRACKHISTOS_H
