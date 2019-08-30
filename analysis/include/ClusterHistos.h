#ifndef CLUSTERHISTOS_H
#define CLUSTERHISTOS_H

#include "TFile.h"
#include "HistoManager.h"

#include "TrackerHit.h"

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
  
     
};


#endif
