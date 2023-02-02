#include "SvtTimingHistos.h"
#include <math.h>

SvtTimingHistos::SvtTimingHistos(const std::string& inputName, ModuleMapper* mmapper) {
    m_name = inputName;
    mmapper_ = mmapper;
}


void SvtTimingHistos::Define1DHistos() {

}

void SvtTimingHistos::DefineHistos(){
  std::string makeMultiplesTag = "svtHit";
  std::vector<std::string> sensorNames;
  mmapper_->getStrings(sensorNames);
  HistoManager::DefineHistos(sensorNames, makeMultiplesTag );
  HistoManager::DefineHistos(sensorNames, "trkrHit" );
  HistoManager::DefineHistos(sensorNames, "trkRawHit" );
  HistoManager::DefineHistos(sensorNames, "trkSiCluster" );

  
}

void SvtTimingHistos::FillRawHits(std::vector<RawSvtHit*> *rawHits, float weight ) {
    int nHits = rawHits->size();
    Fill1DHisto("numTrkrHits_h", (float)nHits, weight);
    std::string h_name = "";
    int fitNumber=0;//take the first fit even if there is pileup
    
    for (int i=0; i < nHits; i++) 
    {      
        RawSvtHit *hit = rawHits->at(i);
        auto mod = std::to_string(hit->getModule());
        auto lay = std::to_string(hit->getLayer());
       
        std::string sensorName= mmapper_->getStringFromSw("ly"+lay+"_m"+mod);
       
        //        std::cout<<sensorName<<std::endl;
        Fill1DHisto(sensorName+"_svtHit_Edep_h", hit->getAmp(fitNumber), weight); // Scaled to MeV
        Fill1DHisto(sensorName+"_svtHit_Time_h", hit->getT0(fitNumber), weight);
        Fill1DHisto(sensorName+"_svtHit_TimeWide_h", hit->getT0(fitNumber), weight);

        //        Fill1DHisto("svtHitEdep"+lname, hit->getAmp(fitNumber), weight); // Scaled to MeV
        ///Fill1DHisto("svtHitTime"+lname, hit->getT0(fitNumber), weight);
    }

    

}


void SvtTimingHistos::FillTrackerHits(std::vector<TrackerHit*> *trkrHits, float weight ) {
    int nHits = trkrHits->size();
    Fill1DHisto("numTrkrHits_h", (float)nHits, weight);
    std::string h_name = "";
    bool hasRefHit=false;
    std::string refModString="ly11_m0"; //"L5T_axial_ele"
    float refTime=-666.;

    for (int i=0; i < nHits; i++) 
    {
        TrackerHit *hit = trkrHits->at(i);
        RawSvtHit *rhit = (RawSvtHit*)hit->getRawHits()->At(0);
        auto mod = std::to_string(rhit->getModule());
        auto lay = std::to_string(rhit->getLayer());
        std::string swName="ly"+lay+"_m"+mod;
        if (swName==refModString){
          hasRefHit=true;
          refTime=hit->getTime();
        }

        std::string sensorName= mmapper_->getStringFromSw("ly"+lay+"_m"+mod);
        Fill1DHisto(sensorName+"_trkrHit_Edep_h", hit->getCharge()*1000000.0, weight); // Scaled to MeV
        Fill1DHisto(sensorName+"_trkrHit_Time_h", hit->getTime(), weight);
    }

    // if track doesn't have a hit in reference layer, just return
    if(!hasRefHit)
      return; 
    for (int i=0; i < nHits; i++) 
    {      
      TrackerHit *hit = trkrHits->at(i);
      RawSvtHit *rhit = (RawSvtHit*)hit->getRawHits()->At(0);
      auto mod = std::to_string(rhit->getModule());
      auto lay = std::to_string(rhit->getLayer());
      std::string sensorName= mmapper_->getStringFromSw("ly"+lay+"_m"+mod);
        //        std::cout<<sensorName<<std::endl;
      Fill1DHisto(sensorName+"_trkrHit_L5AxEle_Minus_LX_TimeWide_h", refTime-hit->getTime(), weight);
    }

}

void SvtTimingHistos::FillTracks(std::vector<Track*> *tracks, float weight ) {
    int nTracks = tracks->size();
    Fill1DHisto("numTracks_h", (float)nTracks, weight);
    int fitNumber=0;
    std::string refModString="ly11_m0"; //"L5T_axial_ele"

    for (int i=0;i<nTracks; i++){
      Track* trk=tracks->at(i);
      int nHitsOnTrack=trk->getTrackerHitCount();
      Fill1DHisto("trkChiSq_h",(float)trk->getChi2(),weight);
      Fill1DHisto("trkTime_h",(float)trk->getTrackTime(),weight);
      Fill1DHisto("trkNHits_h",(float)trk->getTrackerHitCount(),weight);   
      auto trkHits=trk->getSvtHits();
      float refTime=-666.;
      bool hasRefHit=false;
      for (int i=0; i < nHitsOnTrack; i++) 
        {
          TrackerHit *hit =(TrackerHit*) trkHits->At(i);
          RawSvtHit *rhit = (RawSvtHit*)hit->getRawHits()->At(0);
          auto mod = std::to_string(rhit->getModule());
          auto lay = std::to_string(rhit->getLayer());
          std::string swName="ly"+lay+"_m"+mod;
          if (swName==refModString){
            hasRefHit=true;
            refTime=hit->getTime();
          }
          
          std::string sensorName= mmapper_->getStringFromSw("ly"+lay+"_m"+mod);
          Fill1DHisto(sensorName+"_trkSiCluster_Time_h", hit->getTime(), weight);
          Fill1DHisto(sensorName+"_trkRawHit_Time_h", rhit->getT0(fitNumber), weight);
          
        }
      
      // if track doesn't have a hit in reference layer, just return
      if(!hasRefHit)
        continue; 
      for (int i=0; i < nHitsOnTrack; i++) 
        {      
          TrackerHit *hit =(TrackerHit*) trkHits->At(i);
          RawSvtHit *rhit = (RawSvtHit*)hit->getRawHits()->At(0);
          auto mod = std::to_string(rhit->getModule());
          auto lay = std::to_string(rhit->getLayer());
          std::string sensorName= mmapper_->getStringFromSw("ly"+lay+"_m"+mod);
          //        std::cout<<sensorName<<std::endl;
          Fill1DHisto(sensorName+"_trkSiCluster_L5AxEle_Minus_LX_Time_h", refTime-hit->getTime(), weight);
        }      
    }
}

void SvtTimingHistos::FillEcalHits(std::vector<CalHit*> *ecalHits, float weight ) {
    int nHits = ecalHits->size();
    Fill1DHisto("numEcalHits_h", (float)nHits, weight);
    for (int i=0; i < nHits; i++) 
    {
        CalHit *hit = ecalHits->at(i);
        Fill1DHisto("ecalHitEnergy_h", hit->getEnergy()*1000.0, weight); // Scaled to MeV
    }
}

void SvtTimingHistos::FillEcalClusters(std::vector<CalCluster*> *ecalClusters, float weight ) {
    int nClusters = ecalClusters->size();
    Fill1DHisto("numEcalClusters_h", (float)nClusters, weight);
    for (int i=0; i < nClusters; i++) 
    {
        CalCluster *cluster = ecalClusters->at(i);
        Fill1DHisto("ecalClusterEnergy_h", cluster->getEnergy()*1000.0, weight); // Scaled to MeV
    }
}
