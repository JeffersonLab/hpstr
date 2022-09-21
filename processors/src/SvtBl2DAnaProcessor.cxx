#include "SvtBl2DAnaProcessor.h"
#include "TBranch.h"
#include <fstream>
#include <map>

SvtBl2DAnaProcessor::SvtBl2DAnaProcessor(const std::string& name, Process& process) : Processor(name,process){
    mmapper_ = new ModuleMapper();
}

SvtBl2DAnaProcessor::~SvtBl2DAnaProcessor(){ 
}


void SvtBl2DAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring SvtBl2DAnaProcessor" << std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        rawSvtHitsColl_  = parameters.getString("rawSvtHitsColl");
        histCfgFilename_ = parameters.getString("histCfg");
        triggerBankColl_   = parameters.getString("triggerBankColl"); 
        triggerFilename_   = parameters.getString("triggerBankCfg"); 
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }
    std::cout << "TRIGGER FILE NAME: " << triggerFilename_ << std::endl;
}

void SvtBl2DAnaProcessor::initialize(TTree* tree) {
    std::cout << "[SvtBl2DAnaProcessor] Initializing" << std::endl;
    svtCondHistos = new Svt2DBlHistos("raw_hits", mmapper_);

    std::cout << "[SvtBl2DAnaProcessor] Load JSON" << std::endl;
    svtCondHistos->loadHistoConfig(histCfgFilename_);

    if (debug_ > 0) std::cout << "[SvtBl2DAnaProcessor] Define 2DHistos" << std::endl;
    svtCondHistos->Svt2DBlHistos::DefineHistos();
    if (debug_ > 0) std::cout << "[SvtBl2DAnaProcessor] Defined 2DHistos" << std::endl;

    tree_ = tree;
    tree_->SetBranchAddress(rawSvtHitsColl_.c_str(), &rawSvtHits_, &brawSvtHits_);
    tree_->SetBranchAddress(triggerBankColl_.c_str(), &triggerBank_, &btriggerBank_);
    if (debug_ > 0) std::cout << "[SvtBl2DAnaProcessor] TTree Initialized" << std::endl;

    //Read triggerBankCollection configuration file to only run on specified triggers
    if(!triggerFilename_.empty()){
        std::ifstream i_file(triggerFilename_);
        i_file >> triggers_;
        i_file.close();
    }
}

bool SvtBl2DAnaProcessor::process(IEvent* ievent) {

    TSData* tsdata = (TSData*) triggerBank_;
    
    //Build binary map of triggers based on TSData.h
    //make this just trigger map
    //add random -> tsdata->ext.Pulsar
    prescaledtriggerMap_["Single_0_Top"] = tsdata->prescaled.Single_0_Top;  
    prescaledtriggerMap_["Single_1_Top"] = tsdata->prescaled.Single_1_Top; 
    prescaledtriggerMap_["Single_2_Top"] = tsdata->prescaled.Single_2_Top; 
    prescaledtriggerMap_["Single_3_Top"] = tsdata->prescaled.Single_3_Top; 
    prescaledtriggerMap_["Single_0_Bot"] = tsdata->prescaled.Single_0_Bot; 
    prescaledtriggerMap_["Single_1_Bot"] = tsdata->prescaled.Single_1_Bot; 
    prescaledtriggerMap_["Single_2_Bot"] = tsdata->prescaled.Single_2_Bot; 
    prescaledtriggerMap_["Single_3_Bot"] = tsdata->prescaled.Single_3_Bot; 
    prescaledtriggerMap_["Pair_0      "] = tsdata->prescaled.Pair_0      ; 
    prescaledtriggerMap_["Pair_1      "] = tsdata->prescaled.Pair_1      ; 
    prescaledtriggerMap_["Pair_2      "] = tsdata->prescaled.Pair_2      ; 
    prescaledtriggerMap_["Pair_3      "] = tsdata->prescaled.Pair_3      ; 
    prescaledtriggerMap_["LED         "] = tsdata->prescaled.LED         ; 
    prescaledtriggerMap_["Cosmic      "] = tsdata->prescaled.Cosmic      ; 
    prescaledtriggerMap_["Hodoscope   "] = tsdata->prescaled.Hodoscope   ; 
    prescaledtriggerMap_["Pulser      "] = tsdata->prescaled.Pulser      ; 
    prescaledtriggerMap_["Mult_0      "] = tsdata->prescaled.Mult_0      ; 
    prescaledtriggerMap_["Mult_1      "] = tsdata->prescaled.Mult_1      ;
    prescaledtriggerMap_["FEE_Top     "] = tsdata->prescaled.FEE_Top     ;
    prescaledtriggerMap_["FEE_Bot     "] = tsdata->prescaled.FEE_Bot     ;   

    //dont need ext trigger map to require specific triggers
    //unless we want to require randoms
    exttriggerMap_["Single_0_Top"] = tsdata->ext.Single_0_Top; 
    exttriggerMap_["Single_1_Top"] = tsdata->ext.Single_1_Top; 
    exttriggerMap_["Single_2_Top"] = tsdata->ext.Single_2_Top; 
    exttriggerMap_["Single_3_Top"] = tsdata->ext.Single_3_Top; 
    exttriggerMap_["Single_0_Bot"] = tsdata->ext.Single_0_Bot; 
    exttriggerMap_["Single_1_Bot"] = tsdata->ext.Single_1_Bot; 
    exttriggerMap_["Single_2_Bot"] = tsdata->ext.Single_2_Bot; 
    exttriggerMap_["Single_3_Bot"] = tsdata->ext.Single_3_Bot; 
    exttriggerMap_["Pair_0      "] = tsdata->ext.Pair_0      ; 
    exttriggerMap_["Pair_1      "] = tsdata->ext.Pair_1      ; 
    exttriggerMap_["Pair_2      "] = tsdata->ext.Pair_2      ; 
    exttriggerMap_["Pair_3      "] = tsdata->ext.Pair_3      ; 
    exttriggerMap_["LED         "] = tsdata->ext.LED         ; 
    exttriggerMap_["Cosmic      "] = tsdata->ext.Cosmic      ; 
    exttriggerMap_["Hodoscope   "] = tsdata->ext.Hodoscope   ; 
    exttriggerMap_["Pulser      "] = tsdata->ext.Pulser      ; 
    exttriggerMap_["Mult_0      "] = tsdata->ext.Mult_0      ; 
    exttriggerMap_["Mult_1      "] = tsdata->ext.Mult_1      ;
    exttriggerMap_["FEE_Top     "] = tsdata->ext.FEE_Top     ;
    exttriggerMap_["FEE_Bot     "] = tsdata->ext.FEE_Bot     ;   
    
    bool triggerFound = false;
    for (auto trigger : triggers_.items()){
        if ( (prescaledtriggerMap_[trigger.key()] > 0) || (exttriggerMap_[trigger.key()]) > 0 ){
            triggerFound = true;
        }
    }

    if (!triggerFound)
        return true;
    
    svtCondHistos->FillHistograms(rawSvtHits_,1.);

    return true;
}

void SvtBl2DAnaProcessor::finalize() {
    std::cout << "[SvtBl2DAnaProcessor] Finalizing" << std::endl;
    svtCondHistos->saveHistos(outF_,"");
    delete svtCondHistos;
    svtCondHistos = nullptr;
}

DECLARE_PROCESSOR(SvtBl2DAnaProcessor);
