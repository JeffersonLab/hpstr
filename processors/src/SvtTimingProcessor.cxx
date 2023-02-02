/**
 * @file SvtTimingProcessor.cxx
 * @brief AnaProcessor used fill histograms to compare simulations
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 */     
#include "SvtTimingProcessor.h"
#include <iostream>

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
	//        ecalHitColl_     = parameters.getString("ecalHitColl");
        //ecalClusColl_    = parameters.getString("ecalClusColl");
        histCfgFilename_ = parameters.getString("histCfg");
        selectionCfg_         = parameters.getString("selectionjson",selectionCfg_); 
        regionSelections_ = parameters.getVString("regionDefinitions",regionSelections_);
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

}

void SvtTimingProcessor::initialize(TTree* tree) {
    tree_= tree;
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
    tree_->SetBranchAddress("EventHeader"       , &evth_    , &bevth_    );                       
    //tree_->SetBranchAddress(ecalHitColl_.c_str()  , &ecalHits_    , &becalHits_    );
    //tree_->SetBranchAddress(ecalClusColl_.c_str() , &ecalClusters_, &becalClusters_);

}

bool SvtTimingProcessor::process(IEvent* ievent) {
  double weight=1.0;
  float modTime=(evth_->getEventTime()%24)/4.;
  //  std::cout<<"event phase = "<<modTime<<std::endl;

  histos->FillRawHits(rawHits_);
  histos->FillTrackerHits(trkrHits_);
  histos->FillTracks(tracks_);

  for (auto region : _regions ) {
    //    std::cout<<"pass cut?  "<<_reg_phase_selectors[region]->passCutEq("modTime_eq",modTime,weight)<<std::endl;
    if (_reg_phase_selectors[region] && !_reg_phase_selectors[region]->passCutEq("modTime_eq",modTime,weight))
      continue;

    _reg_phase_histos[region]->FillRawHits(rawHits_);
    _reg_phase_histos[region]->FillTrackerHits(trkrHits_);
    _reg_phase_histos[region]->FillTracks(tracks_);

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
