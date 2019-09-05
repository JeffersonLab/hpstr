#ifndef CLUSTERHISTOS_H
#define CLUSTERHISTOS_H

#include "TFile.h"
#include "HistoManager.h"

#include "TrackerHit.h"
#include "RawSvtHit.h"

#include <string>


class ClusterHistos : public HistoManager{
  
 public:
 ClusterHistos(const std::string& inputName):HistoManager(inputName)
  {m_name = inputName;
  }

  virtual void Define3DHistos(){};
  virtual void Define2DHistos();
  virtual void Define1DHistos();

  //virtual void GetHistosFromFile(TFile* inFile, const std::string& name,const std::string& folder="");


  void FillHistograms(TrackerHit* hit,float weight = 1.);
  //void BuildAxesMap();
  

 private:
  
  std::vector<std::string> layers{"ly0","ly1","ly2","ly3","ly4","ly5","ly6"};
  std::vector<std::string> volumes{"top","bottom"};
  std::vector<std::string> sides{"axial","stereo"};
  std::vector<std::string> variables{"charge","cluSize"};

  std::map<std::string, int>    cluSizeMap;
  std::map<std::string, double> chargeMap;
  std::map<std::string, double> cluPositionMap;
  
  
};


#endif
