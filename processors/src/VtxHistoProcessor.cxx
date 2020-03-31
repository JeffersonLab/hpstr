/**
 * @file VtxHistoProcessor.cxx
 * @brief Post processor of vtx histos
 * @author PF, SLAC National Accelerator Laboratory
 */

#include "VtxHistoProcessor.h"

VtxHistoProcessor::VtxHistoProcessor(const std::string& name, Process& process)
    : Processor(name, process) { 
    }

VtxHistoProcessor::~VtxHistoProcessor() { 
}

void VtxHistoProcessor::configure(const ParameterSet& parameters) {

    std::cout << "Configuring VtxHistoProcessor" << std::endl;
    try
    {
        debug_          = parameters.getInteger("debug");
        rebin_          = parameters.getInteger("rebin");
        selection_      = parameters.getString("selection");
        projections_    = parameters.getVString("projections");
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }
}

void VtxHistoProcessor::initialize(std::string inFilename, std::string outFilename) {
    // Init Files
    inF_ = new TFile(inFilename.c_str());
    outF_ = new TFile(outFilename.c_str(),"RECREATE");

    //To save the projections
    HistogramHelpers::OpenProjectionFile();    
    
    //Get the vtx vs InvM and p 
    
    if (debug_) {
        std::cout<<"Getting..."<< (std::string("/")+selection_+"/"+selection_+"_vtx_p_svt_z_hh").c_str() << std::endl;
    }

    for (auto projection_ : projections_) {
        if (debug_) {
            std::cout<<"Getting... " <<(std::string("/")+selection_+"/"+selection_+"_"+projection_).c_str() << std::endl;
        }
        
        _histos2d[projection_] = (TH2F*) inF_->Get((std::string("/")+selection_+"/"+selection_+"_"+projection_).c_str());
        _histos2d[projection_]->SetDirectory(0);
        _histos2d[projection_]->RebinX(rebin_);
    }
}

bool VtxHistoProcessor::process() {
    
    for (it2d_ it = _histos2d.begin(); it!=_histos2d.end();++it) { 
        _histos1d[it->first+"_mu"] = new TH1F(
            (it->first+"_mu").c_str(),
            (it->first+"_mu").c_str(),
            it->second->GetXaxis()->GetNbins(),
            it->second->GetXaxis()->GetXmin(),
            it->second->GetXaxis()->GetXmax());
        
        _histos1d[it->first+"_sigma"] = new TH1F(
            (it->first+"_sigma").c_str(),
            (it->first+"_sigma").c_str(),
            it->second->GetXaxis()->GetNbins(),
            it->second->GetXaxis()->GetXmin(),
            it->second->GetXaxis()->GetXmax());
        
        _histos1d[it->first+"_mu"]->Sumw2();
        _histos1d[it->first+"_sigma"]->Sumw2();
        
        
        HistogramHelpers::profileYwithIterativeGaussFit(it->second,_histos1d[it->first+"_mu"],_histos1d[it->first+"_sigma"],1,0);
    }       
    return true;
}

void VtxHistoProcessor::finalize() { 

    outF_->cd();
    
    for (it1d_ it = _histos1d.begin(); it!=_histos1d.end(); ++ it) {
        it->second->Write();
    }

    for (it2d_ it = _histos2d.begin(); it!=_histos2d.end(); ++ it) {
        it->second->Write();
    }
    
    outF_->Close();
    inF_->Close();

    //Close the projection file
    HistogramHelpers::CloseProjectionFile();

    delete inF_;
}

DECLARE_PROCESSOR(VtxHistoProcessor); 
