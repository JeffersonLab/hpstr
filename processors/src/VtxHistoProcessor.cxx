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
    
    //Get the vtx vs InvM and p 
    
    if (debug_) {
        std::cout<<"Getting..."<< (std::string("/")+selection_+"/"+selection_+"_vtx_p_svt_z_hh").c_str() << std::endl;
    }
    _histos2d["h_vtx_z_p"]    = (TH2F*) inF_->Get((std::string("/")+selection_+"/"+selection_+"_vtx_p_svt_z_hh").c_str());
    _histos2d["h_vtx_z_p"]->SetDirectory(0);
    _histos2d["h_vtx_z_p"]->RebinX(rebin_);
    _histos2d["h_vtx_z_InvM"] = (TH2F*) inF_->Get((std::string("/")+selection_+"/"+selection_+"_vtx_InvM_vtx_svt_z_hh").c_str());
    _histos2d["h_vtx_z_InvM"]->SetDirectory(0);
    _histos2d["h_vtx_z_InvM"]->RebinX(rebin_);
    
}

bool VtxHistoProcessor::process() {

    
    for (it2d_ it = _histos2d.begin(); it!=_histos2d.end();++it) { 
        _histos1d[it->first+"_mu"] = new TH1F(
            (it->first+"_mu").c_str(),
            (it->first+"_mu").c_str(),
            it->second->GetXaxis()->GetNbins(),
            it->second->GetXaxis()->GetXmin(),
            it->second->GetXaxis()->GetXmin());
        
        _histos1d[it->first+"_sigma"] = new TH1F(
            (it->first+"_sigma").c_str(),
            (it->first+"_sigma").c_str(),
            it->second->GetXaxis()->GetNbins(),
            it->second->GetXaxis()->GetXmin(),
            it->second->GetXaxis()->GetXmax());
        _histos1d[it->first+"_mu"]->Sumw2();
        _histos1d[it->first+"_sigma"]->Sumw2();
        
        
        HistogramHelpers::profileYwithIterativeGaussFit(it->second,_histos1d[it->first+"_mu"],_histos1d[it->first+"_sigma"]);
    }
        
    return true;
}

void VtxHistoProcessor::finalize() { 

    outF_->cd();
    
    _histos1d["h_vtx_z_InvM_mu"]->Write();
    _histos1d["h_vtx_z_InvM_sigma"]->Write();
    
    _histos1d["h_vtx_z_p_mu"]->Write();
    _histos1d["h_vtx_z_p_sigma"]->Write();
    
    outF_->Close();
    inF_->Close();
    delete inF_;
}

DECLARE_PROCESSOR(VtxHistoProcessor); 
