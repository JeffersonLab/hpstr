/**
 * @file SvtTimingProcessor.cxx
 * @brief AnaProcessor used fill histograms for SVT Timing Calibrations
 * @author Cameron Bravo, Matt Graham, SLAC National Accelerator Laboratory
 */     
#include "SvtTimingProcessor.h"
#include <iostream>
#include <fstream>

//TODO CHECK THIS DESTRUCTOR
SvtTimingProcessor::~SvtTimingProcessor(){}

SvtTimingProcessor::SvtTimingProcessor(const std::string& name, Process& process) : Processor(name,process){
    mmapper_ = new ModuleMapper();
}

void SvtTimingProcessor::configure(const ParameterSet& parameters) {
  std::cout << "Configuring SvtTimingProcessor" << std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        anaName_         = parameters.getString("anaName");
        rawHitColl_         = parameters.getString("rawHitColl");
        trkColl_         = parameters.getString("trkColl");
        trkrHitColl_     = parameters.getString("trkrHitColl");
        fspColl_     = parameters.getString("fspColl");
	//        ecalHitColl_     = parameters.getString("ecalHitColl");
        //ecalClusColl_    = parameters.getString("ecalClusColl");
        histCfgFilename_ = parameters.getString("histCfg");
        selectionCfg_         = parameters.getString("selectionjson",selectionCfg_); 
        regionSelections_ = parameters.getVString("regionDefinitions",regionSelections_);
        timingCalibDir_ = parameters.getString("timingCalibDir");
        postfixTiming_ = parameters.getString("postfixTiming");
        runNumber_=parameters.getInteger("runNumber");
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

}

void SvtTimingProcessor::initialize(TTree* tree) {
    tree_= tree;
    _ah =  std::make_shared<AnaHelpers>();
    // init histos
    histos = new SvtTimingHistos(anaName_.c_str(),mmapper_);
    histos->loadHistoConfig(histCfgFilename_);
    histos->DefineHistos();
    if (!selectionCfg_.empty()) {
        evtSelector_ = std::make_shared<BaseSelector>(name_+"_evtSelector",selectionCfg_);
        evtSelector_->setDebug(debug_);
        evtSelector_->LoadSelection();        
    }

     for (unsigned int i_reg = 0; i_reg < regionSelections_.size(); i_reg++) {
        std::string regname = AnaHelpers::getFileName(regionSelections_[i_reg],false);
        std::cout<<"Setting up region:: " << regname <<std::endl;
        _reg_phase_selectors[regname] = std::make_shared<BaseSelector>(anaName_+"_"+regname, regionSelections_[i_reg]);
        _reg_phase_selectors[regname]->setDebug(debug_);
        _reg_phase_selectors[regname]->LoadSelection();

        _reg_phase_histos[regname] = std::make_shared<SvtTimingHistos>(anaName_+"_"+regname,mmapper_);
        _reg_phase_histos[regname]->loadHistoConfig(histCfgFilename_);
        _reg_phase_histos[regname]->DefineHistos();
        _regions.push_back(regname);
     }

    // init TTree
    tree_->SetBranchAddress(rawHitColl_.c_str()  , &rawHits_    , &brawHits_    );
    tree_->SetBranchAddress(trkrHitColl_.c_str()  , &trkrHits_    , &btrkrHits_    );
    tree_->SetBranchAddress(trkColl_.c_str()      , &tracks_      , &btracks_      );
    tree_->SetBranchAddress(fspColl_.c_str()      , &fsps_      , &bfsps_      );
    tree_->SetBranchAddress("EventHeader"       , &evth_    , &bevth_    );                       
    //tree_->SetBranchAddress(ecalHitColl_.c_str()  , &ecalHits_    , &becalHits_    );
    //tree_->SetBranchAddress(ecalClusColl_.c_str() , &ecalClusters_, &becalClusters_);

    std::string timingCalibFile_=timingCalibDir_+"/run"+std::to_string(runNumber_)+"/run"+std::to_string(runNumber_)+postfixTiming_;
    std::ifstream calibFile(timingCalibFile_);
    std::string sensorName;
    double calibMean;
    
    if(calibFile.is_open()){
      std::cout<<"SvtTimingProcessor::Loading Timing Calibration File: "<<timingCalibFile_<<std::endl;
      while(calibFile.good()){
        calibFile>>sensorName>>calibMean;
        std::cout<<"name = "<<sensorName<<"  time shift = "<<calibMean<<std::endl;
        timingCalibConstants_.insert( std::pair<std::string, double>(sensorName, calibMean));
        
      }

    } else {
      std::cout<<"SvtTimingProcessor::Timing Calibration File Not Found:  "<<timingCalibFile_<<std::endl;
    }
    
}

bool SvtTimingProcessor::process(IEvent* ievent) {
  double weight=1.0;
  float modTime=(evth_->getEventTime()%24)/4.;
  int phase=int(modTime);
  if(debug_)
    std::cout<<"event phase = "<<modTime<<std::endl;

  //  histos->FillRawHits(rawHits_);
  //  histos->FillTrackerHits(trkrHits_);    
  //  int nTracks = tracks_->size();
  //for (int i=0;i<nTracks; i++){
  //   Track* trk=tracks_->at(i);
  //   int nHitsOnTrack=trk->getTrackerHitCount();
  //   float trkP=(float)trk->getP();
  //   if(!evtSelector_->passCutGt("nHits_gt",nHitsOnTrack,weight))
  //      continue;
  //   if(!evtSelector_->passCutGt("trkP_gt",trkP,weight))
  //      continue;
  //   histos->FillHitsOnTrack(trk,&timingCalibConstants_, phase,weight);
  //}

  int nFSPs=fsps_->size();
  for(int i=0; i<nFSPs; i++){
    Particle* fsp=fsps_->at(i); 
    Track trk=fsp->getTrack(); 
    CalCluster cluster=fsp->getCluster();
    
    //require both a track and matched cluster
    if(&trk==NULL)
      continue;
    if(&cluster==NULL)
      continue; 

    bool foundL1 = false;
    bool foundL2 = false;
    //_ah->InnermostLayerCheck(pos_trk_from_trkList, foundL1pos, foundL2pos);  
    _ah->InnermostLayerCheck(&trk, foundL1, foundL2);  
    
    int nHitsOnTrack=trk.getTrackerHitCount();
    float trkP=(float)trk.getP();
    if(!evtSelector_->passCutGt("nHits_gt",nHitsOnTrack,weight))
      continue;
    if(!evtSelector_->passCutGt("trkP_gt",trkP,weight))
      continue;
    if(debug_)
      std::cout<<"Filling Histo"<<std::cout;
    histos->FillHitsOnTrack(&trk,&timingCalibConstants_, phase, &cluster, weight);
  }

  for (auto region : _regions ) {
    if (_reg_phase_selectors[region] && !_reg_phase_selectors[region]->passCutEq("modTime_eq",modTime,weight))
      continue;
   
    for (int i=0;i<nFSPs; i++){
      Particle* fsp=fsps_->at(i); 
      Track trk=fsp->getTrack(); 
      CalCluster cluster=fsp->getCluster();
      
      //require both a track and matched cluster
      if(&trk==NULL)
        continue;
      if(&cluster==NULL)
        continue;       
      int nHitsOnTrack=trk.getTrackerHitCount();
      float trkP=(float)trk.getP();
      if(!evtSelector_->passCutGt("nHits_gt",nHitsOnTrack,weight))
        continue;
      if(!evtSelector_->passCutGt("trkP_gt",trkP,weight))
        continue;
      /*
      std::cout<<"region = "<<region<<";  trkP = "<<trkP<<std::endl;
      std::cout<<"pass cut gt?  "<<_reg_phase_selectors[region]->passCutGt("trkP_gt",trkP,weight)<<std::endl;
      std::cout<<"pass cut lt?  "<<_reg_phase_selectors[region]->passCutLt("trkP_lt",trkP,weight)<<std::endl;
      */
      if (_reg_phase_selectors[region] && !_reg_phase_selectors[region]->passCutGt("trkP_gt",trkP,weight))
        continue;
      if (_reg_phase_selectors[region] && !_reg_phase_selectors[region]->passCutLt("trkP_lt",trkP,weight))
        continue;
      //std::cout<<"region = "<<region<<" Passed all cuts"<<std::endl;
      //      _reg_phase_histos[region]->FillHitsOnTrack(trk,weight);
      _reg_phase_histos[region]->FillHitsOnTrack(&trk, &timingCalibConstants_, phase, &cluster, weight);
    }

  }

  //histos->FillEcalHits(ecalHits_);
  //histos->FillEcalClusters(ecalClusters_);
  
    return true;
}

void SvtTimingProcessor::finalize() {

    histos->saveHistos(outF_, anaName_.c_str());
    delete histos;
    histos = nullptr;

    for (reg_it it = _reg_phase_histos.begin(); it!=_reg_phase_histos.end(); ++it) {
        std::string dirName = anaName_+"_"+it->first;
        (it->second)->saveHistos(outF_,dirName);
    }

}
DECLARE_PROCESSOR(SvtTimingProcessor);
