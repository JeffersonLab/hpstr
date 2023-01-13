#include "ZBiCutflowProcessor.h"
//#include <FlatTupleMaker.h>
#include <string>
#include <cstdlib>
#include <iostream>
#include <fstream>

ZBiCutflowProcessor::ZBiCutflowProcessor(const std::string& name, Process& process) : Processor(name,process) {
    std::cout << "CONSTRUCTOR" << std::endl;

}

ZBiCutflowProcessor::~ZBiCutflowProcessor(){}

/*
ZBiCutflowProcessor::loadHistoConfig(const std::string histoConfigFile) {
    std::ifstream i_file(histoConfigFile);
    i_file >> _h_configs;
    if (debug_) {
        for (auto& el : _h_configs.items())
            std::cout << el.key() << " : " << el.value() << "\n";
    }
    i_file.close();
}
*/

/*
bool ZBiCutflowProcessor::LoadCuts(){
    if (cuts_cfgFile.empty()){
        std::cout<<"ERROR ZBiCutflowProcessor::Configuration File not specified";
        return false;
    }

    std::ifstream i_file(cuts_cfgFile);
    i_file >> iter_cuts_;
    for (auto cut : iter_cuts_.items()) {
        cuts[cut.key()].first = cut.value().at("cut")
    }
}
*/

void ZBiCutflowProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring ZBiCutflowProcessor" << std::endl;
    try
    {
        debug_ = parameters.getInteger("debug",debug_);
        //cuts_cfgFile_ = parameters.getSTring("cuts_cfgFile",cuts_cfgFile);
        signalFilename_ = parameters.getString("signalFilename", signalFilename_);
        tritrigFilename_ = parameters.getString("tritrigFilename", tritrigFilename_);
        //radSlicFilename_ = parameters.getString("radSlicFilename",radSlicFilename_);
        vdSimFilename_ = parameters.getString("vdSimFilename", vdSimFilename_);
        vdMassMeV_ = parameters.getDouble("vdMassMeV", vdMassMeV_);
        //ApMassMeV_ = parameters.getDouble("ApMassMeV", ApMassMeV_);
        signalHistCfgFilename_ = parameters.getString("signalHistCfgFilename",signalHistCfgFilename_);
        outFileName_ = parameters.getString("outFileName",outFileName_);
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

}

void ZBiCutflowProcessor::readFlatTuple(TTree* tree, std::map<std::string, double*> &tuple_map){
    int nBr = tree->GetListOfBranches()->GetEntries();
    for(int iBr = 0; iBr < nBr; iBr++){
        TBranch *br = dynamic_cast<TBranch*>(tree->GetListOfBranches()->At(iBr)); 
        std::cout << "Reading variable: " << br->GetFullName() << std::endl;
        double* value = new double;
        std::string varname = (std::string)br->GetFullName();
        tuple_map[varname] = value;
        tree->SetBranchAddress(varname.c_str(),tuple_map[varname]);
    }
}

void ZBiCutflowProcessor::initialize(std::string inFilename, std::string outFilename){
    std::cout << "INITIALIZE" << std::endl;

    outFile_ = new TFile(outFileName_.c_str(),"RECREATE");

    signalHistos_ = new ZBiHistos("signal");
    std::cout << "loading signal histo config" << std::endl;
    signalHistos_->loadHistoConfig(signalHistCfgFilename_);
    std::cout << "Defining signal histos" << std::endl;
    signalHistos_->DefineHistos();
    signalHistos_->printHistos1d();

    
    //init list of cuts with strings (will be configured later)
    cutlist_strings_ = {"unc_vtx_psum_gt", "unc_vtx_ele_track_p_gt", "unc_vtx_pos_track_p_gt", "unc_vtx_psum_lt", "unc_vtx_ele_track_p_lt", "unc_vtx_pos_track_p_lt", "unc_vtx_chi2_lt"};
    //Convert list of cut strings to map that holds cut values
    for(std::vector<std::string>::iterator it=cutlist_strings_.begin(); it != cutlist_strings_.end(); ++it){
        cuts_[*it] = -999.9;
    }

    //init list of variables to use
    cut_vars_ = {"unc_vtx_psum", "unc_vtx_ele_track_p", "unc_vtx_chi2"};


    //Read signal tuple
    TFile* signalFile = new TFile(signalFilename_.c_str(),"READ");
    signalTree_ = (TTree*)signalFile->Get("vtxana_kf_Tight_2016_simp_reach_dev/vtxana_kf_Tight_2016_simp_reach_dev_tree");
    readFlatTuple(signalTree_, signal_tuple_);
    std::cout << "POST READ MAP SIZE: " << signal_tuple_.size() << std::endl;

}

bool ZBiCutflowProcessor::process(){
    std::cout << "PROCESS" << std::endl;

    //Fill signal variable histograms
    for(int e=0; e < 500; e++){
        signalTree_->GetEntry(e);
        for(std::vector<std::string>::iterator it=cut_vars_.begin(); it !=cut_vars_.end(); it++){
            std::string cutvar = (std::string)*it;
            if(signalHistos_->get1dHistos().count("signal_"+cutvar+"_h")){
                signalHistos_->Fill1DHisto(cutvar+"_h",*signal_tuple_[cutvar]);
            }
            else{
                std::cout << "No Histogram named " << cutvar+"_h" << " exists for variable " << cutvar << std::endl; 
                abort();
            }
        }

        /*
        for(std::map<std::string, double*>::iterator it=signal_tuple_.begin(); it != signal_tuple_.end(); it++){
            std::string varname = (std::string)it->first;
            std::cout << "varname: " << varname << " " << *it->second << std::endl;
        }
        */
        
    }

}

void ZBiCutflowProcessor::finalize() {
    std::cout << "Finalize" << std::endl;
    outFile_->cd();
    signalHistos_->saveHistos(outFile_,"signal");
    outFile_->Close();

    //outFile_->cd();
    //signalHistos_->saveHistos(outFile_);

}

DECLARE_PROCESSOR(ZBiCutflowProcessor);
