/**
 * @file SvtRawDataAnaProcessor.cxx
 * @brief AnaProcessor used fill histograms to study svt hit fitting
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 */     
#include "SvtRawDataAnaProcessor.h"
#include <iostream>

SvtRawDataAnaProcessor::SvtRawDataAnaProcessor(const std::string& name, Process& process) : Processor(name,process){
    mmapper_ = new ModuleMapper();
}
//TODO CHECK THIS DESTRUCTOR
SvtRawDataAnaProcessor::~SvtRawDataAnaProcessor(){}


void SvtRawDataAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring SvtRawDataAnaProcessor" << std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        anaName_         = parameters.getString("anaName");
        svtHitColl_     = parameters.getString("trkrHitColl");
        histCfgFilename_ = parameters.getString("histCfg");
        regionSelections_ = parameters.getVString("regionDefinitions");
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

}

void SvtRawDataAnaProcessor::initialize(TTree* tree) {
    tree_= tree;
    // init histos
    //histos = new RawSvtHitHistos(anaName_.c_str(), mmapper_);
    //histos->loadHistoConfig(histCfgFilename_);
    //histos->DefineHistos();
    //std::cout<<"hello4"<<std::endl;
    //std::cout<<svtHitColl_.c_str()<<std::endl;
    ///std::cout<<svtHits_->size()<<std::endl;
    tree_->SetBranchAddress(svtHitColl_.c_str()  , &svtHits_    , &bsvtHits_    );
    for (unsigned int i_reg = 0; i_reg < regionSelections_.size(); i_reg++) 
    {
        std::string regname = AnaHelpers::getFileName(regionSelections_[i_reg],false);
        std::cout << "Setting up region:: " << regname << std::endl;
        reg_selectors_[regname] = std::make_shared<BaseSelector>(regname, regionSelections_[i_reg]);
        reg_selectors_[regname]->setDebug(debug_);
        reg_selectors_[regname]->LoadSelection();

        reg_histos_[regname] = std::make_shared<RawSvtHitHistos>(regname);
        reg_histos_[regname]->loadHistoConfig(histCfgFilename_);
        //reg_histos_[regname]->doTrackComparisonPlots(false);
        reg_histos_[regname]->DefineHistos();

        regions_.push_back(regname);
    }
    }

bool SvtRawDataAnaProcessor::process(IEvent* ievent) {
    //std::cout<<"hello5"<<std::endl;
    Float_t TimeRef=0.0;
    Float_t AmpRef=0.0;
    for (unsigned int i_reg = 0; i_reg < regionSelections_.size(); i_reg++) 
    {
        for(unsigned int i = 0; i < nhits; i++){
            RawSvtHit * thisHit = svtHits_->at(i);
            int getNum = thisHit->getFitN();
            for(unsigned int J=0; J<getNum; J++){
                if(!(reg_selectors_[regions_[i_reg]]->passCutEq("getN_et",getNum,weight))){continue;}
                if(!(reg_selectors_[regions_[i_reg]]->passCutLt("getId_lt",J,weight))){continue;} 
                if(!(reg_selectors_[regions_[i_reg]]->passCutGt("getId_gt",J,weight))){continue;}
                if(!(reg_selectors_[regions_[i_reg]]->passCutLt("chi_lt",thisHit->getChiSq(J),weight))){continue;}
                if(!(reg_selectors_[regions_[i_reg]]->passCutGt("chi_gt",thisHit->getChiSq(J),weight))){continue;}
                if(reg_selectors_[regions_[i_reg]]->passCutEq("doing_ct",1.0,weight)){
                    if(!(((thisHit->getT0(J))-TimeRef)**2<(thisHit->getT0((J+1)%2)-TimeRef)**2)){continue;}          
                }
                if(reg_selectors_[regions_[i_reg]]->passCutEq("doing_ft",1.0,weight)){
                    if((((thisHit->getT0(J))-TimeRef)**2<(thisHit->getT0((J+1)%2)-TimeRef)**2)){continue;}          
                }
                 if(reg_selectors_[regions_[i_reg]]->passCutEq("doing_sa",1.0,weight)){
                    if(!(((thisHit->getAmp(J))-AmpRef)**2<(thisHit->getAmp((J+1)%2)-AmpRef)**2)){continue;}          
                }
                if(reg_selectors_[regions_[i_reg]]->passCutEq("doing_la",1.0,weight)){
                    if((((thisHit->getAmp(J))-AmpRef)**2<(thisHit->getAmp((J+1)%2)-AmpRef)**2)){continue;}          
                }
                Float_t TimeDiff=-42069.0;
                Float_t AmpDiff=-42069.0;
                if(getNum==2){
                    TimeDiff=(thisHit->getT0(J))-(thisHit->getT0((J+1)%2));
                    AmpDiff=(thisHit->getT0(J))-(thisHit->getT0((J+1)%2)); 
                }
                reg_histos_[regions_[i_reg]]->FillHistograms(thisHit,weight,J,i,TimeDiff,AmpDiff);
            }
            //std::string regname = AnaHelpers::getFileName(regionSelections_[i_reg],false);
        //std::cout << "Setting up region:: " << regname << std::endl;
        //reg_selectors_[regname] = std::make_shared<BaseSelector>(regname, regionSelections_[i_reg]);
        //reg_selectors_[regname]->setDebug(debug_);
        //reg_selectors_[regname]->LoadSelection();

        //reg_histos_[regname] = std::make_shared<RawSvtHitHistos>(regname);
        //reg_histos_[regname]->loadHistoConfig(histCfgFilename_);
        //reg_histos_[regname]->doTrackComparisonPlots(false);
        //reg_histos_[regname]->DefineHistos();

        //regions_.push_back(regname);
            //reg_histos->FillHistograms(svtHits_,1.);
    }
    return true;
   
}
void SvtRawDataAnaProcessor::finalize() {

    //outF_->cd();
    //for(reg_it it = reg_histos_.begin(); it!=reg_histos_.end(); ++it){
    //    std::string dirName = it->first;
    //    (it->second)->saveHistos(outF_,dirName);
    //    outF_->cd(dirName.c_str());
    //    reg_selectors_[it->first]->getCutFlowHisto()->Scale(.5);
    //    reg_selectors_[it->first]->getCutFlowHisto()->Write();
    //}
    //std::cout<<"gotToHEREANDFUCKINGEXPLODED"<<std::endl;
    //histos->saveHistosSVT(outF_, anaName_.c_str());
    //delete histos;
    //histos = nullptr;
    //
    trkHistos_->saveHistos(outF_,trkCollName_);
    delete trkHistos_;
    trkHistos_ = nullptr;
    if (trkSelector_)
        trkSelector_->getCutFlowHisto()->Write();

    if (truthHistos_) {
        truthHistos_->saveHistos(outF_,trkCollName_+"_truth");
        delete truthHistos_;
        truthHistos_ = nullptr;
    }
}

DECLARE_PROCESSOR(SvtRawDataAnaProcessor);
