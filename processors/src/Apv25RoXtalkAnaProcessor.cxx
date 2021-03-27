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
        syncPhase_       = parameters.getInteger("syncPhase");
        trigPhase_       = syncPhase_%24;
        trigDel_         = parameters.getInteger("trigDel");
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
    for (int i = 0; i < 210; i++)
    {
        //////////////////lFEBrms[i] = 0.0;
        //hFEBrms[i] = 0.0;
        sps[i] = 0.0;
    }

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
        int feb = std::atoi(&modMap_->getHwFromSw(
                    "ly"+std::to_string(lay)+"_m"+std::to_string(mod)).at(1) );
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
    outF_->cd();
    hitN_h->Write();
    lFEBN_h->Write();
    hFEBN_h->Write();
    FEBN_hh->Write();
    for(int sp = 0; sp < 840; sp += 4)
    {
        syncPhase_ = sp;
        trigPhase_ = syncPhase_%24;
        emulateApv25Buff(sp);
    }

    TGraph lFEBreadRms_g(210, sps, lFEBrms);
    lFEBreadRms_g.SetName("lFEBreadRms_g");
    lFEBreadRms_g.Write();

    TGraph hFEBreadRms_g(210, sps, hFEBrms);
    hFEBreadRms_g.SetName("hFEBreadRms_g");
    hFEBreadRms_g.Write();

}

void Apv25RoXtalkAnaProcessor::emulateApv25Buff(int buffIter) {
    reads.clear();
    readEvs.clear();
    TH1D readN_h(Form("readN_iter%i_h", buffIter), Form("readN_iter%i_h", buffIter), 21, -0.5, 20.5);
    TH2D lFEBread_hh(Form("lFEBread_iter%i_hh", buffIter), ";Read Time minus Event Time [ns];Read Event Time mod 840", 500, -2000.0, 14000.0, 210, 0, 35*24);
    TH1D lFEBread_h(Form("lFEBread_iter%i_h", buffIter), ";Read Time minus Event Time [ns];Events / 8 ns", 500, -2000.0, 2000.0);
    TH2D hFEBread_hh(Form("hFEBread_iter%i_hh", buffIter), ";Read Time minus Event Time [ns];Read Event Time mod 840", 500, -2000.0, 14000.0, 210, 0, 35*24);
    TH1D hFEBread_h(Form("hFEBread_iter%i_h", buffIter), ";Read Time minus Event Time [ns];Events / 8 ns", 500, -2000.0, 2000.0);
    for (int iEv = 0; iEv < hitMultis.size(); iEv++)
    {
        // Calculate the relevant times wrt this event
        long evTime = eventTimes[iEv];
        long trigArrT = evTime + trigDel_ + (24 - (evTime+trigPhase_)%24);
        long trigSyncTime = trigArrT + (35*24 - (trigArrT+syncPhase_)%(35*24));

        // Remove reads from buffer which are in the past wrt this event
        for (int ir = reads.size() - 1; ir >= 0; ir--)
        {
            if ( reads[ir] < evTime - 840 )
            {
                reads.erase(reads.begin() + ir);
                readEvs.erase(readEvs.begin() + ir);
            }
        }
        int buffNstart = reads.size();
        //std::cout << "reads before adding: " << reads.size() << std::endl;
        for (int ss = 0; ss < 6; ss++)
        {
            if (ss > 0)
            {
                reads.push_back(reads[reads.size()-1] + 3360);
                readEvs.push_back(evTime);
            }
            else
            {
                if (buffNstart == 0)
                {
                    reads.push_back(trigSyncTime);
                    readEvs.push_back(evTime);
                }
                else if (reads[reads.size()-1] + 3360 > trigSyncTime)
                {
                    reads.push_back(reads[reads.size()-1] + 3360);
                    readEvs.push_back(evTime);
                }
                else
                {
                    reads.push_back(trigSyncTime);
                    readEvs.push_back(evTime);
                }
            }
        }
        //std::cout << "reads after adding: " << reads.size() << std::endl;
        //readN_h.Fill((double)reads.size());
        if (lFEBMultis[iEv] > 400) 
        {
            lFEBread_h.Fill( reads[0] - evTime);
            lFEBread_hh.Fill( reads[0] - evTime, readEvs[0]%(24*35) );
        }
        if (hFEBMultis[iEv] > 300) 
        {
            hFEBread_h.Fill( reads[0] - evTime);
            hFEBread_hh.Fill( reads[0] - evTime, readEvs[0]%(24*35) );
        }
    }
    lFEBrms[buffIter/4] = lFEBread_h.GetRMS();
    hFEBrms[buffIter/4] = hFEBread_h.GetRMS();
    sps[buffIter/4] = (double)buffIter;
    readN_h.Write();
    lFEBread_h.Write();
    lFEBread_hh.Write();
    hFEBread_h.Write();
    hFEBread_hh.Write();
}

DECLARE_PROCESSOR(Apv25RoXtalkAnaProcessor);
