/**
 * @file Apv25RoXtalkAnaProcessor.cxx
 * @brief AnaProcessor used fill histograms to compare simulations
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 */     
#include "Apv25RoXtalkAnaProcessor.h"
#include <iostream>

Apv25RoXtalkAnaProcessor::Apv25RoXtalkAnaProcessor(const std::string& name, Process& process) : Processor(name,process){}
//TODO CHECK THIS DESTRUCTOR
Apv25RoXtalkAnaProcessor::~Apv25RoXtalkAnaProcessor(){}


void Apv25RoXtalkAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring Apv25RoXtalkAnaProcessor" << std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        anaName_         = parameters.getString("anaName");
        rawHitColl_      = parameters.getString("rawHitColl");
        initSyncPhase_   = parameters.getInteger("debug");
        histCfgFilename_ = parameters.getString("histCfg");
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

}

void Apv25RoXtalkAnaProcessor::initialize(TTree* tree) {
    tree_= tree;
    modMap_ = new ModuleMapper(2019);

    // init TTree
    tree_->SetBranchAddress(rawHitColl_.c_str() , &rawHits_ , &brawHits_ );
    tree_->SetBranchAddress("EventHeader"       , &evth_    , &bevth_    );

}

bool Apv25RoXtalkAnaProcessor::process(IEvent* ievent) {

    //std::cout << "[Apv25RoXtalkAnaProcessor] Event Time: " << evth_->getEventTime() << std::endl;
    //std::cout << "[Apv25RoXtalkAnaProcessor] Event Hit Multi: " << rawHits_->size() << std::endl;
    //std::cout << "[Apv25RoXtalkAnaProcessor] ModMap Test: " << modMap_->getStringFromHw("F1H2") << std::endl;

    eventTimes.push_back(evth_->getEventTime());
    hitMultis.push_back(rawHits_->size());
    int lFEBMulti = 0;
    int hFEBMulti = 0;
    for (int i = 0; i < rawHits_->size(); i++)
    {
        int lay = rawHits_->at(i)->getLayer();
        int mod = rawHits_->at(i)->getModule();
        int feb = std::atoi(&modMap_->getHwFromSw("ly" + std::to_string(lay) + "_m" + std::to_string(mod) ).at(1));
        if (feb > 4) hFEBMulti++;
        else lFEBMulti++;
    }
    lFEBMultis.push_back(lFEBMulti);
    hFEBMultis.push_back(hFEBMulti);

    return true;
}

void Apv25RoXtalkAnaProcessor::finalize() {

    std::cout << "[Apv25RoXtalkAnaProcessor] Finalizing" << std::endl;
    TH1D * hitN_h = new TH1D("hitN_h", "hitN_h", 500, 0, 2000);
    TH1D * lFEBN_h = new TH1D("lFEBN_h", "lFEBN_h", 500, 0, 2000);
    TH1D * hFEBN_h = new TH1D("hFEBN_h", "hFEBN_h", 500, 0, 2000);
    TH2D * FEBN_hh = new TH2D("FEBN_hh", "FEBN_hh", 500, 0, 2000, 500, 0, 2000);
    for (int i = 0; i < hitMultis.size(); i++)
    {
        hitN_h->Fill(hitMultis[i]);
        lFEBN_h->Fill(lFEBMultis[i]);
        hFEBN_h->Fill(hFEBMultis[i]);
        FEBN_hh->Fill(hFEBMultis[i], lFEBMultis[i]);
    }
    TFile * outF = new TFile((anaName_+".root").c_str(), "RECREATE");
    outF->cd();
    hitN_h->Write();
    lFEBN_h->Write();
    hFEBN_h->Write();
    FEBN_hh->Write();
    outF->Close();

}

DECLARE_PROCESSOR(Apv25RoXtalkAnaProcessor);
