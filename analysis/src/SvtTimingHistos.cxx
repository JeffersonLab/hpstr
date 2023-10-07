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
        RawSvtHit *rhit = (RawSvtHit*)hit->getRawHits().At(0);
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
      RawSvtHit *rhit = (RawSvtHit*)hit->getRawHits().At(0);
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
  
    for (int i=0;i<nTracks; i++){
      Track* trk=tracks->at(i);
      int nHitsOnTrack=trk->getTrackerHitCount();
      float trkP=(float)trk->getP();    
      Fill1DHisto("trkChiSq_h",(float)trk->getChi2(),weight);
      Fill1DHisto("trkTime_h",(float)trk->getTrackTime(),weight);
      Fill1DHisto("trkP_h",trkP,weight);
      Fill1DHisto("trkNHits_h",(float)trk->getTrackerHitCount(),weight);   
    }
}

void SvtTimingHistos::FillHitsOnTrack(Track* trk, std::map<std::string,double> *timingCalib, int phase, CalCluster *cluster,  float weight){
  
  int fitNumber=0;
  int secondFit=1;
  //  std::string refModStringT="ly11_m0"; //"L5T_axial_ele"
  //  std::string refModStringB="ly11_m1"; //"L5B_stereo_ele"
  std::string refModStringT="ly8_m0"; //"L3T_stereo"
  std::string refModStringB="ly8_m1"; //"L3B_axial"

  bool useCalibration=true; 
  if(timingCalib->size()<1){
    if(debug_)std::cout<<"Calibration Map is not there...no calib"<<std::endl;
    useCalibration=false;
  }
  //////////////   take this out...just for testing //////////
  //  useCalibration=false;

  int nHitsOnTrack=trk->getTrackerHitCount();
  int trkCh=trk->getCharge();
  std::string chStr="electron";
  if(trkCh>0){
    chStr="positron";
    //refModStringT="ly11_m2";
    //    refModStringB="ly11_m3";
  }

  float clTimeOffset=37.5; //ns
  float roughSVTHitTimeOffsetSlim=-7.0; //ns
  float roughSVTHitTimeOffsetOld=-16.5; //ns
  

  float clTime=(float)cluster->getTime()-clTimeOffset;
  float trkTime=(float)trk->getTrackTime()-roughSVTHitTimeOffsetOld;
  float trkP=(float)trk->getP();
  float refTime=-666.;
  bool hasRefHit=false;
  auto trkHits=trk->getSvtHits();
  Fill1DHisto("trkChiSq_h",(float)trk->getChi2(),weight);
  Fill1DHisto("trkTime_h",trkTime,weight);
  Fill1DHisto("trkTime_Minus_CluTime_h",trkTime-clTime,weight);
  Fill1DHisto("ecalClusterTime_h",clTime,weight);
  Fill1DHisto("ecalClusterEnergy_h",(float) cluster->getEnergy(),weight);
  Fill2DHisto("cluTime_vs_trkTime_hh",clTime,trkTime,weight);
  Fill1DHisto("ecalClusterTime_h",clTime,weight);
  if(trk->isTopTrack()){
    Fill1DHisto("trkTime_Top_h",trkTime,weight);
    Fill1DHisto("trkTime_Minus_CluTime_Top_h",trkTime-clTime,weight);
    Fill1DHisto("ecalClusterTime_Top_h",clTime,weight);
    Fill2DHisto("cluTime_vs_trkTime_Top_hh",clTime,trkTime,weight);
  } else {
      Fill1DHisto("trkTime_Bot_h",trkTime,weight);
      Fill1DHisto("trkTime_Minus_CluTime_Bot_h",trkTime-clTime,weight);
      Fill1DHisto("ecalClusterTime_Bot_h",clTime,weight);
      Fill2DHisto("cluTime_vs_trkTime_Bot_hh",clTime,trkTime,weight);
  }

  Fill1DHisto("trkP_h",trkP,weight);
  Fill1DHisto("trkNHits_h",(float)trk->getTrackerHitCount(),weight);   

  Fill1DHisto(chStr+"_trkChiSq_h",(float)trk->getChi2(),weight);
  Fill1DHisto(chStr+"_trkTime_h",trkTime,weight);
  Fill1DHisto(chStr+"_trkTime_Minus_CluTime_h",trkTime-clTime,weight);
  Fill1DHisto(chStr+"_ecalClusterTime_h",clTime,weight);
  Fill2DHisto(chStr+"_cluTime_vs_trkTime_hh",clTime,trkTime,weight);
  if(trk->isTopTrack()){
    Fill1DHisto(chStr+"_trkTime_Top_h",trkTime,weight);
    Fill1DHisto(chStr+"_trkTime_Minus_CluTime_Top_h",trkTime-clTime,weight);
    Fill1DHisto(chStr+"_ecalClusterTime_Top_h",clTime,weight);
    Fill2DHisto(chStr+"_cluTime_vs_trkTime_Top_hh",clTime,trkTime,weight);
  } else {
      Fill1DHisto(chStr+"_trkTime_Bot_h",trkTime,weight);
      Fill1DHisto(chStr+"_trkTime_Minus_CluTime_Bot_h",trkTime-clTime,weight);
      Fill1DHisto(chStr+"_ecalClusterTime_Bot_h",clTime,weight);
      Fill2DHisto(chStr+"_cluTime_vs_trkTime_Bot_hh",clTime,trkTime,weight);
  }
  Fill1DHisto(chStr+"_trkP_h",trkP,weight);  
  Fill1DHisto(chStr+"_trkNHits_h",(float)trk->getTrackerHitCount(),weight);   
  double trkTimeCorr=0; 
  for (int i=0; i < nHitsOnTrack; i++) 
    {
      double calibForLayer=0.0; 
      TrackerHit *hit =(TrackerHit*) trkHits.At(i);
      RawSvtHit *rhit = (RawSvtHit*)hit->getRawHits().At(0);
      auto mod = std::to_string(rhit->getModule());
      auto lay = std::to_string(rhit->getLayer());
      std::string swName="ly"+lay+"_m"+mod;
      std::string sensorName= mmapper_->getStringFromSw("ly"+lay+"_m"+mod);
      std::string phSt=std::to_string(phase);
      std::string calibName=sensorName+"_phase"+phSt;
      if(useCalibration){
        if(debug_)std::cout<<"Getting calib for "<<calibName<<std::endl;
        calibForLayer=-timingCalib->at(calibName);
        if(debug_)std::cout<<calibName<<" constant = "<<calibForLayer<<std::endl;
      } else {//use rough time offset
        if (rhit->getLayer()<5)
          calibForLayer=-roughSVTHitTimeOffsetSlim;
        else 
          calibForLayer=-roughSVTHitTimeOffsetOld;

      }
      if (swName==refModStringT || swName==refModStringB){
        hasRefHit=true;
        refTime=hit->getTime();
      }
      
      trkTimeCorr+=hit->getTime()+calibForLayer;
      float hitTime=hit->getTime()+calibForLayer; 

      
      if(debug_)std::cout<<"Original hit time = "<<hit->getTime()<<";  shift = "<<calibForLayer<<"; final hit time = "<<hitTime<<std::endl;

      Fill1DHisto(sensorName+"_trkSiCluster_Time_h",hitTime, weight);
      //      Fill1DHisto(sensorName+"_trkRawHit_Time_h", rhit->getT0(fitNumber)+calibForLayer, weight);     
      Fill1DHisto(sensorName+"_"+chStr+"_trkSiCluster_Time_h",hitTime, weight);
      //Fill1DHisto(sensorName+"_"+chStr+"_trkRawHit_Time_h", rhit->getT0(fitNumber)+calibForLayer, weight);

      Fill2DHisto("HitMinCluTime_Versus_Layer_hh", rhit->getLayer()-1, hitTime-clTime,weight);
      Fill2DHisto(chStr+"_HitMinCluTime_Versus_Layer_hh", rhit->getLayer()-1, hitTime-clTime,weight);
      if(trk->isTopTrack()){
        Fill2DHisto("HitMinCluTime_Versus_Layer_Top_hh", rhit->getLayer()-1, hitTime-clTime,weight);
        Fill2DHisto(chStr+"_HitMinCluTime_Versus_Layer_Top_hh", rhit->getLayer()-1, hitTime-clTime,weight);
      }else{
        Fill2DHisto("HitMinCluTime_Versus_Layer_Bot_hh", rhit->getLayer()-1, hitTime-clTime,weight);
        Fill2DHisto(chStr+"_HitMinCluTime_Versus_Layer_Bot_hh", rhit->getLayer()-1, hitTime-clTime,weight);
      }

      Fill1DHisto(sensorName+"_trkSiCluster_Time_Minus_CluTime_h",hitTime-clTime, weight);
      Fill2DHisto(sensorName+"_trkSiCluster_Time_Versus_CluTime_hh",hitTime, clTime, weight);

      Fill1DHisto(sensorName+"_"+chStr+"_trkSiCluster_Time_Minus_CluTime_h",hitTime-clTime, weight);
      Fill2DHisto(sensorName+"_"+chStr+"_trkSiCluster_Time_Versus_CluTime_hh",hitTime, clTime, weight);

      if(rhit->getFitN()==2){
	//Fill1DHisto(sensorName+"_trkSiCluster_2Fits_Time_h",hitTime, weight);
        //	Fill1DHisto(sensorName+"_trkRawHit_2Fits_Time_h", rhit->getT0(fitNumber)+calibForLayer, weight);     
	//Fill1DHisto(sensorName+"_"+chStr+"_trkSiCluster_2Fits_Time_h",hitTime, weight);
	//Fill1DHisto(sensorName+"_"+chStr+"_trkRawHit_2Fits_Time_h", rhit->getT0(fitNumber)+calibForLayer, weight);
      }
      
    }
  trkTimeCorr=trkTimeCorr/nHitsOnTrack;
  Fill1DHisto("CalibratedTrackTime_h", trkTimeCorr, weight);
  Fill1DHisto(chStr+"_CalibratedTrackTime_h", trkTimeCorr, weight);
  Fill1DHisto("CalibratedTrackTime_Minus_CluTime_h", trkTimeCorr-clTime, weight);
  Fill1DHisto(chStr+"_CalibratedTrackTime_Minus_CluTime_h", trkTimeCorr-clTime, weight);
  Fill2DHisto("CalibratedTrackTime_Versus_CluTime_hh",clTime,trkTimeCorr,weight);
  Fill2DHisto(chStr+"_CalibratedTrackTime_Versus_CluTime_hh",clTime,trkTimeCorr,weight);

  if(trk->isTopTrack()){
    Fill1DHisto("CalibratedTrackTime_Top_h", trkTimeCorr, weight);
    Fill1DHisto(chStr+"_CalibratedTrackTime_Top_h", trkTimeCorr, weight);
    Fill1DHisto("CalibratedTrackTime_Minus_CluTime_Top_h", trkTimeCorr-clTime, weight);
    Fill1DHisto(chStr+"_CalibratedTrackTime_Minus_CluTime_Top_h", trkTimeCorr-clTime, weight);
    Fill2DHisto("CalibratedTrackTime_Versus_CluTime_Top_hh",clTime,trkTimeCorr,weight);
    Fill2DHisto(chStr+"_CalibratedTrackTime_Versus_CluTime_Top_hh",clTime,trkTimeCorr,weight);
  }else {
    Fill1DHisto("CalibratedTrackTime_Bot_h", trkTimeCorr, weight);
    Fill1DHisto(chStr+"_CalibratedTrackTime_Bot_h", trkTimeCorr, weight);
    Fill1DHisto("CalibratedTrackTime_Minus_CluTime_Bot_h", trkTimeCorr-clTime, weight);
    Fill1DHisto(chStr+"_CalibratedTrackTime_Minus_CluTime_Bot_h", trkTimeCorr-clTime, weight);
    Fill2DHisto("CalibratedTrackTime_Versus_CluTime_Bot_hh",clTime,trkTimeCorr,weight);
    Fill2DHisto(chStr+"_CalibratedTrackTime_Versus_CluTime_Bot_hh",clTime,trkTimeCorr,weight);
  }
  // if track doesn't have a hit in reference layer, just return
  /*
  if(!hasRefHit)
    return; 
  Fill1DHisto("CalTrackTime_Minus_M3L0_h", trkTimeCorr-refTime, weight);
  Fill1DHisto(chStr+"_CalTrackTime_Minus_M3L0_h", trkTimeCorr-refTime, weight);

  if(trk->isTopTrack()){
    Fill1DHisto("CalTrackTime_Minus_M3L0_Top_h", trkTimeCorr-refTime, weight);
    Fill1DHisto(chStr+"_CalTrackTime_Minus_M3L0_Top_h", trkTimeCorr-refTime, weight);
  }else {
    Fill1DHisto("CalTrackTime_Minus_M3L0_Bot_h", trkTimeCorr-refTime, weight);
    Fill1DHisto(chStr+"_CalTrackTime_Minus_M3L0_Bot_h", trkTimeCorr-refTime, weight);
  }
  
  for (int i=0; i < nHitsOnTrack; i++) 
    {      
      double calibForLayer=0.0; 
      TrackerHit *hit =(TrackerHit*) trkHits.At(i);
      RawSvtHit *rhit = (RawSvtHit*)hit->getRawHits().At(0);
      auto mod = std::to_string(rhit->getModule());
      auto lay = std::to_string(rhit->getLayer());
      std::string sensorName= mmapper_->getStringFromSw("ly"+lay+"_m"+mod);
      std::string phSt=std::to_string(phase);
      std::string calibName=sensorName+"_phase"+phSt;
      if(useCalibration){
        calibForLayer=timingCalib->at(calibName);
        //std::cout<<calibName<<" constant = "<<calibForLayer<<std::endl;
      } else {
        //std::cout<<calibName<<" not found in map????"<<std::endl;
      }
      //      Fill2DHisto(sensorName+"_trkSiCluster_M3L0_Minus_LX_Time_Versus_Chi2_hh", refTime-hit->getTime()-calibForLayer,  (float)trk->getChi2(), weight);
      Fill1DHisto(sensorName+"_trkSiCluster_M3L0_Minus_LX_Time_h", refTime-hit->getTime()-calibForLayer, weight);
      Fill2DHisto(sensorName+"_trkSiCluster_M3L0_Versus_LX_Time_hh", refTime,hit->getTime()+calibForLayer, weight);
      Fill1DHisto(sensorName+"_"+chStr+"_trkSiCluster_M3L0_Minus_LX_Time_h", refTime-hit->getTime()-calibForLayer, weight);
      //      Fill2DHisto(sensorName+"_"+chStr+"_trkSiCluster_M3L0_Minus_LX_Time_Versus_Chi2_hh", refTime-hit->getTime()-calibForLayer,  (float)trk->getChi2(), weight);
      Fill2DHisto(sensorName+"_"+chStr+"_trkSiCluster_M3L0_Versus_LX_Time_hh", refTime,hit->getTime()+calibForLayer, weight);
      if(rhit->getFitN()==2){
	//Fill1DHisto(sensorName+"_trkSiCluster_M3L0_Minus_LX_2Fits_Time_h", refTime-hit->getTime()-calibForLayer, weight);
	//Fill2DHisto(sensorName+"_trkSiCluster_M3L0_Versus_LX_2Fits_Time_hh", refTime,hit->getTime()+calibForLayer, weight);
	//Fill1DHisto(sensorName+"_"+chStr+"_trkSiCluster_M3L0_Minus_LX_2Fits_Time_h", refTime-hit->getTime()-calibForLayer, weight);
	//Fill2DHisto(sensorName+"_"+chStr+"_trkSiCluster_M3L0_Versus_LX_2Fits_Time_hh", refTime,hit->getTime()+calibForLayer, weight);
      }
    }      
  */

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
