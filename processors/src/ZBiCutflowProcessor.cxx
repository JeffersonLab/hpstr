#include "ZBiCutflowProcessor.h"
//#include <FlatTupleMaker.h>
#include <string>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include "TEfficiency.h"

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
        std::cout << "DEBUG IS " << (bool) debug_ << std::endl;
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
        if(debug_)
            std::cout << "Reading variable: " << br->GetFullName() << std::endl;
        double* value = new double;
        std::string varname = (std::string)br->GetFullName();
        if(debug_)
            std::cout << "Storing varname " << varname << " in tuple map" << std::endl;
        tuple_map[varname] = value;
        tree->SetBranchAddress(varname.c_str(),tuple_map[varname]);
    }
}

void ZBiCutflowProcessor::initialize(std::string inFilename, std::string outFilename){
    std::cout << "INITIALIZE" << std::endl;
    //mc scaling
    double Lumi =10.7; //pb-1
    mcScale_["tritrig"] = 1.416e9*Lumi/(50000*9853);
    mcScale_["wab"] = 0.1985e12*Lumi/(100000*9966);
    mcScale_["rad"] = 66.36e6*Lumi/(10000*9959);
    mcScale_["rad_slic"] = 66.36e6*Lumi/(10000*9959);



    outFile_ = new TFile(outFileName_.c_str(),"RECREATE");

    //init cut selector
    //cutSelector_ = new IterativeCutSelector("iterativeCuts",cutSelectionCfg_);
    cutSelector_ = new IterativeCutSelector("iterativeCuts","/sdf/group/hps/users/alspellm/src/test/hpstr/analysis/selections/simps/iterativeCuts.json");
    cutSelector_->setDebug(debug_);
    cutSelector_->LoadSelection();

    //signal 
    signalHistos_ = new ZBiHistos("signal");
    signalHistos_->debugMode(debug_);
    std::cout << "loading signal histo config" << std::endl;
    signalHistos_->loadHistoConfig(signalHistCfgFilename_);
    std::cout << "Defining signal histos" << std::endl;
    signalHistos_->DefineHistos();
    if(debug_)
        signalHistos_->printHistos1d();

    //Get the pretrigger vtx distribution ONLY NEED TO DO THIS ONCE!
    TFile* vdSimFile = new TFile(vdSimFilename_.c_str(), "READ");
    signalHistos_->GetHistosFromFile(vdSimFile,"mcAna_mc625Z_h","mcAna");
    delete vdSimFile;
    signalHistos_->get1dHistos()["signal_vdSimZ_h"] = signalHistos_->plot1D("signal_vdSimZ_h","true z_{vtx} [mm]", 200, -50.3, 149.7);
    for(int i=0; i < 201; i++)
        signalHistos_->get1dHisto("signal_vdSimZ_h")->SetBinContent(i,signalHistos_->get1dHisto("mcAna_mc625Z_h")->GetBinContent(i));

    //cut_histos
    cutHistos_ = new ZBiHistos("");
    cutHistos_->debugMode(debug_);
    for(cut_iter_ it=cuts_.begin(); it!=cuts_.end(); it++){
        std::string cutname = it->first;
        cutHistos_->get1dHistos()["signal_vdSelZ_"+cutname] = cutHistos_->plot1D("signal_vdSelZ_"+cutname+"_h","true z_{vtx} [mm]", 200, -50.3, 149.7);
        cutHistos_->get1dHistos()["tritrig_zVtx_"+cutname] = cutHistos_->plot1D("tritrig_zVtx_"+cutname+"_h","unc z_{vtx} [mm]", 150, -50.0, 100.0);
    }

    //init list of variables to use
    cut_vars_ = {"unc_vtx_psum", "unc_vtx_ele_track_p", "unc_vtx_chi2"};

    
    //init list of cuts with strings (will be configured later)
    cutlist_strings_ = {"unc_vtx_psum_gt", "unc_vtx_ele_track_p_gt", "unc_vtx_pos_track_p_gt", "unc_vtx_psum_lt", "unc_vtx_ele_track_p_lt", "unc_vtx_pos_track_p_lt", "unc_vtx_chi2_lt"};
    /*
    //Convert list of cut strings to map that holds cut values
    for(std::vector<std::string>::iterator it=cutlist_strings_.begin(); it != cutlist_strings_.end(); ++it){
        cuts_[*it] = -999.9;
    }*/

    //Initialize tritrig tuple
    if(debug_) std::cout << "Reading tuple from tritrig file" << std::endl;
    TFile* tritrigFile = new TFile(tritrigFilename_.c_str(),"READ");
    tritrigTree_ = (TTree*)tritrigFile->Get("vtxana_kf_Tight_2016_simp_reach_dev/vtxana_kf_Tight_2016_simp_reach_dev_tree");
    readFlatTuple(tritrigTree_, tritrig_tuple_);

    //Initialize signal tuple
    if(debug_) std::cout << "Reading tuple from signal file" << std::endl;
    TFile* signalFile = new TFile(signalFilename_.c_str(),"READ");
    signalTree_ = (TTree*)signalFile->Get("vtxana_kf_Tight_2016_simp_reach_dev/vtxana_kf_Tight_2016_simp_reach_dev_tree");
    readFlatTuple(signalTree_, signal_tuple_);

    //Fill signal variable histograms with initial values
    if(debug_)
        std::cout << "Filling initial signal histograms for each variable corresponding to cut selection" << std::endl;
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
    }

    //Get intial integral values for each signal cut variable histogram
    for(std::vector<std::string>::iterator it=cut_vars_.begin(); it !=cut_vars_.end(); it++){
        std::string cutvar = (std::string)*it;
        initialIntegrals_[cutvar] = signalHistos_->getIntegral("signal_"+cutvar+"_h");
    }
}

//bool hasCut(const std::string& cutname){
//    if (cuts.find(cutname) != cuts.end()
//}
//Make my own BaseSelector class...

bool ZBiCutflowProcessor::process(){
    std::cout << "PROCESS" << std::endl;

    //Initialize various values (NEED TO BE CONFIGURABLE THROUGH SINGLE JSON)
    double mV_MeV = 55.0;
    double mAp_MeV = mV_MeV*(3/1.8);
    double radFrac = 0.07;
    double radAcc = 0.125;
    double massRes_MeV = 3.0;
    double dNdm = 372000.0;

    double m_pi = mAp_MeV/3.0;
    double alpha_D = 0.01;
    double m_l = 0.511;
    double f_pi = m_pi/(4.*M_PI);

    double lowMass = mV_MeV - 2.0*massRes_MeV/2.0;
    double highMass = mV_MeV + 2.0*massRes_MeV/2.0;


    //Get the map of loaded cuts
    cuts_ = cutSelector_->getCuts();

    //Loop over all cuts, find cutvar value that cuts specified fraction of signal 
    //from initial signal histograms 
    double cutFraction = 0.01;
    for(cut_iter_ it=cuts_.begin(); it!=cuts_.end(); it++){
        std::string cutname = it->first;
        std::string cutvar = cutSelector_->getCutVar(cutname);
        if(debug_){
            std::cout << "Cutname: " << cutname << " | Cutvar: " << cutvar << std::endl;
        }
        
        //only bother with cuts that correspond to the cut variables specified
        if(std::find(cut_vars_.begin(), cut_vars_.end(), cutvar) == cut_vars_.end()){
            if(debug_)
                std::cout << "Variable " << cutvar << " not found in list of cut variables" << std::endl;
            continue;
        }

        if(debug_)
            std::cout << "Determining variable value that cuts signal by specified fraction of initial" << std::endl;
        bool isCutGT = cutSelector_->isCutGreaterThan(cutname);
        double cutvalue = signalHistos_->cutFractionOfIntegral("signal_"+cutvar+"_h", isCutGT, cutFraction, initialIntegrals_[cutvar]);
        cutSelector_->setCutValue(cutname, cutvalue);
    }
    
    //Loop over each cut indpendently, build background zvtx disribution. Fit zvtx with Gaus+Tail to determine zcut

    //read tritrig events. For each defined cut, check if event passes. If pass, fill z vtx distribution
    for(int e=0; e < 500; e++){
        tritrigTree_->GetEntry(e);
        for(cut_iter_ it=cuts_.begin(); it!=cuts_.end(); it++){
            //Only consider vertices within defined mass window centered on Vd mass
            if(*tritrig_tuple_["unc_vtx_mass"] > highMass) continue;
            if(*tritrig_tuple_["unc_vtx_mass"] < lowMass) continue;
            std::string cutname = it->first;
            std::string cutvar = cutSelector_->getCutVar(cutname);
            bool isCutGT = cutSelector_->isCutGreaterThan(cutname);
            if(isCutGT){
                if(!cutSelector_->passCutGt(cutname,*tritrig_tuple_[cutvar],1.0))
                    continue;
            }
            else{
                if(!cutSelector_->passCutLt(cutname,*tritrig_tuple_[cutvar],1.0))
                    continue;
            }
            cutHistos_->Fill1DHisto("tritrig_zVtx_"+cutname+"_h",*tritrig_tuple_["unc_vtx_z"],1.0);
        }
    }
    //For each tritrig cut zvtx distribution, fit ZTail to determine zcut value
    std::map<std::string, double> zcuts;
    for(cut_iter_ it=cuts_.begin(); it!=cuts_.end(); it++){
        std::string cutname = it->first;
        double zcut = cutHistos_->fitZTail("tritrig_zVtx_"+cutname+"_h",100.0); //<- TO DO: make
        //100.0 configurable
        zcuts[cutname] = zcut;
    }
    
    //Count background rate for each cut after applying the corresponding zcut
    std::map<std::string,double> scaled_backgrounds;
    std::map<std::string,double> n_offs;
    for(cut_iter_ it=cuts_.begin(); it!=cuts_.end(); it++){
        std::string cutname = it->first;
        scaled_backgrounds[cutname] = 0.0;
        n_offs[cutname] = 0.0;
    }
    for(int e=0; e < 500; e++){
        tritrigTree_->GetEntry(e);
        for(cut_iter_ it=cuts_.begin(); it!=cuts_.end(); it++){
            //Only consider vertices within defined mass window centered on Vd mass
            if(*tritrig_tuple_["unc_vtx_mass"] > highMass) continue;
            if(*tritrig_tuple_["unc_vtx_mass"] < lowMass) continue;
            std::string cutname = it->first;
            std::string cutvar = cutSelector_->getCutVar(cutname);
            bool isCutGT = cutSelector_->isCutGreaterThan(cutname);
            if(isCutGT){
                if(!cutSelector_->passCutGt(cutname,*tritrig_tuple_[cutvar],1.0))
                    continue;
            }
            else{
                if(!cutSelector_->passCutLt(cutname,*tritrig_tuple_[cutvar],1.0))
                    continue;
            }
            if(*tritrig_tuple_["unc_vtx_z"] < zcuts[cutname]) continue;
            scaled_backgrounds[cutname] += mcScale_["tritrig"];
            n_offs[cutname] += 1.0;
        }
    }

    //Make vdZ Selection for each cut...these selections are used to calculated NSig for each cut
    for(int e=0; e < 500; e++){
        signalTree_->GetEntry(e);
        for(cut_iter_ it=cuts_.begin(); it!=cuts_.end(); it++){
            std::string cutname = it->first;
            std::string cutvar = cutSelector_->getCutVar(cutname);
            bool isCutGT = cutSelector_->isCutGreaterThan(cutname);
            if(isCutGT){
                if(!cutSelector_->passCutGt(cutname,*signal_tuple_[cutvar],1.0))
                    continue;
            }
            else{
                if(!cutSelector_->passCutLt(cutname,*signal_tuple_[cutvar],1.0))
                    continue;
            }
            if(*signal_tuple_["unc_vtx_z"] < zcuts[cutname]) continue;
            cutHistos_->get1dHisto("signal_vdSelZ_"+cutname)->Fill(*signal_tuple_["true_vtx_z"]);
        }
    }

    //Cacluate the expected signal for each cut using the vdSelZ histograms
    for(cut_iter_ it=cuts_.begin(); it!=cuts_.end(); it++){
        std::string cutname = it->first;
        TH1F* vdSelZ_h = cutHistos_->get1dHisto("signal_vdSelZ"+cutname);
        TH1F* vdSimZ_h = signalHistos_->get1dHisto("signal_vdSimZ_h");

        //make efficiencies to get F(z)
        TEfficiency* effCalc_h = new TEfficiency(*vdSelZ_h, *vdSimZ_h);
        //TEfficiency* effCalc_h = new TEfficiency(vdSelZ_h&,signalHistos_->get1dHisto("signal_vdSimZ_h")&);
        double logEps2 = -7.5;
        double eps2 = std::pow(10, logEps2);
        double eps = std::sqrt(eps2);

        std::string mesons[2] = {"rho","phi"};
        for(int i =0; i < sizeof(mesons); i++){
            bool rho = false;
            bool phi = false;
            if(mesons[i] == "rho") rho = true;
            else phi = true;
        }


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
