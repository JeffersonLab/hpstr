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

void ZBiCutflowProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring ZBiCutflowProcessor" << std::endl;
    try
    {
        debug_ = parameters.getInteger("debug",debug_);
        cuts_cfgFile_ = parameters.getString("cuts_cfgFile",cuts_cfgFile_);
        signalFilename_ = parameters.getString("signalFilename", signalFilename_);
        tritrigFilename_ = parameters.getString("tritrigFilename", tritrigFilename_);
        vdSimFilename_ = parameters.getString("vdSimFilename", vdSimFilename_);
        vdMassMeV_ = parameters.getDouble("vdMassMeV", vdMassMeV_);
        //ApMassMeV_ = parameters.getDouble("ApMassMeV", ApMassMeV_);
        signalHistCfgFilename_ = parameters.getString("signalHistCfgFilename",signalHistCfgFilename_);
        outFileName_ = parameters.getString("outFileName",outFileName_);
        cutVariables_ = parameters.getVString("cutVariables", cutVariables_);
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }
}

void ZBiCutflowProcessor::initializeFlatTuple(TTree* tree, std::map<std::string, double*> &tuple_map){
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
    std::cout << "[ZBiCutflowProcessor] Initialize" << std::endl;

    //init cut selector and load cuts from configuration file
    cutSelector_ = new IterativeCutSelector("iterativeCuts",cuts_cfgFile_);
    cutSelector_->setDebug(debug_);
    cutSelector_->LoadSelection();
    //Get pointer to map of cuts...allows cuts to be updated here
    cuts_ = cutSelector_->getPointerToCuts();
    
    //Only allow cut variable to persist if it exists in the data
    std::cout << "Checking that all cut variables exist in the input data" << std::endl;
    for(std::vector<std::string>::iterator it=cutVariables_.begin(); it !=cutVariables_.end(); it++){
        std::string cutvar = (std::string)*it;
        doesCutVariableExist(cutvar);
    }

    //Only allow cut to persist if the variable it cuts on is in the list of cutvariables
    std::cout << "Only use cuts that correspond to the configured list of variables to cut on" << std::endl;
    for(cut_iter_ it=cuts_->begin(); it!=cuts_->end(); it++){
        std::string cutname = it->first;
        std::string cutvariable = cutSelector_->getCutVar(cutname);
        std::cout << "Checking that variable exists for cut variable " << cutvariable << std::endl;
        bool found = false;
        //if(cutVariables_.find(cutvariable) != cutVariables_.end()) found = true;
        for(std::vector<std::string>::iterator iit=cutVariables_.begin(); iit !=cutVariables_.end(); iit++){
            if((std::string)*iit == cutvariable){
                std::cout << "Found " << *iit << " in list of cut variables" << std::endl;
                found = true;
            }
        }
        if(!found){
            std::cout << "[ZBiCutflowProcessor] WARNING::Cut variable corresponding to " << cutname << " does not exist!" << std::endl;
            std::cout << "Removing " << cutname << " from list of cuts." << std::endl;
            cutSelector_->eraseCut(cutname);
        }
    }

    //mc scaling... TODO: make configurable 
    double Lumi =10.7; //pb-1
    mcScale_["tritrig"] = 1.416e9*Lumi/(50000*9853);
    mcScale_["wab"] = 0.1985e12*Lumi/(100000*9966);
    mcScale_["rad"] = 66.36e6*Lumi/(10000*9959);
    mcScale_["rad_slic"] = 66.36e6*Lumi/(10000*9959);

    //Initialize output file
    outFile_ = new TFile(outFileName_.c_str(),"RECREATE");

    //Initialize Signal tuple histograms for each variable
    signalHistos_ = new ZBiHistos("signal");
    signalHistos_->debugMode(debug_);
    signalHistos_->loadHistoConfig(signalHistCfgFilename_);
    signalHistos_->DefineHistos();
    if(debug_)
        signalHistos_->printHistos1d();

    //Initialize input signal tuple
    if(debug_) std::cout << "Initializing input tuple from signal file" << std::endl;
    TFile* signalFile = new TFile(signalFilename_.c_str(),"READ");
    //TODO make this selection configurable
    signalTree_ = (TTree*)signalFile->Get("vtxana_kf_Tight_2016_simp_reach_dev/vtxana_kf_Tight_2016_simp_reach_dev_tree");
    initializeFlatTuple(signalTree_, signal_tuple_);

    //Initialize input tritrig tuple
    if(debug_) std::cout << "Initializing input tuple from tritrig file" << std::endl;
    TFile* tritrigFile = new TFile(tritrigFilename_.c_str(),"READ");
    //TODO make this selection configurable
    tritrigTree_ = (TTree*)tritrigFile->Get("vtxana_kf_Tight_2016_simp_reach_dev/vtxana_kf_Tight_2016_simp_reach_dev_tree");
    initializeFlatTuple(tritrigTree_, tritrig_tuple_);

    //Get the pretrigger vtx distribution ONLY NEED TO DO THIS ONCE!
    std::cout << "Getting pretrigger signal vtx distribution" << std::endl;
    vdSimZ_h_ = new TH1F("signal_vdSimZ_h_","signal_vdSimZ;true z_{vtx} [mm];events", 200, -50.3, 149.7);
    std::cout << vdSimFilename_ << std::endl;
    TFile* vdSimFile = new TFile(vdSimFilename_.c_str(), "READ");
    TH1F* vdSimZ_h = (TH1F*)vdSimFile->Get("mcAna/mcAna_mc625Z_h");
    for(int i=0; i < 201; i++){
        vdSimZ_h_->SetBinContent(i,vdSimZ_h->GetBinContent(i));
    }
    vdSimFile->Close();
    delete vdSimFile;

    //Initialize z vertex distribution histograms corresponding to each cut. These are used to calculate
    //expected signal and number of background events for each cut. 
    std::cout << "Initializing z vertex distributions for selected signal and tritrig" << std::endl;
    cutHistos_ = new ZBiHistos("cutHistos");
    cutHistos_->debugMode(debug_);
    for(cut_iter_ it=cuts_->begin(); it!=cuts_->end(); it++){
        std::string cutname = it->first;
        cutHistos_->addHisto1d("signal_vdSelZ_"+cutname+"_h","true z_{vtx} [mm]",200, -50.3, 149.7);
        cutHistos_->addHisto1d("tritrig_zVtx_"+cutname+"_h","unc z_{vtx} [mm]",150, -50.0, 100.0);
    }
    cutHistos_->printHistos1d();

    //Fill signal variable histograms with initial values for each variable
    std::cout << "Filling initial signal histograms for each variable corresponding to cut selection" << std::endl;
    for(int e=0;  e < signalTree_->GetEntries(); e++){
        signalTree_->GetEntry(e);
        //Loop over each signal variable, if cut exists for variable, fill histogram for that variable
        for(std::vector<std::string>::iterator it=cutVariables_.begin(); it !=cutVariables_.end(); it++){
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
    signalHistos_->printHistos1d();

    //Get intial integral values for each signal cut variable histogram
    std::cout << "Getting intial signal histogram integrals to be used as reference for signal cutting" << std::endl;
    for(std::vector<std::string>::iterator it=cutVariables_.begin(); it !=cutVariables_.end(); it++){
        std::string cutvar = (std::string)*it;
        initialIntegrals_[cutvar] = signalHistos_->getIntegral("signal_"+cutvar+"_h");
    }
}

bool ZBiCutflowProcessor::process(){
    std::cout << "[ZBiCutflowProcessor] process()" << std::endl;

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

    //Loop over all cuts, find cutvar value that cuts specified fraction of signal 
    //from initial signal histograms 
    double cutFraction = 0.01;
    for(cut_iter_ it=cuts_->begin(); it!=cuts_->end(); it++){
        std::string cutname = it->first;
        std::string cutvar = cutSelector_->getCutVar(cutname);
        if(debug_){
            std::cout << "Cutname: " << cutname << " | Cutvar: " << cutvar << std::endl;
        }

        if(debug_)
            std::cout << "Determining variable value that cuts signal by specified fraction of initial" << std::endl;
        bool isCutGT = cutSelector_->isCutGreaterThan(cutname);
        double cutvalue = signalHistos_->cutFractionOfIntegral("signal_"+cutvar+"_h", isCutGT, cutFraction, initialIntegrals_[cutvar]);
        cutSelector_->setCutValue(cutname, cutvalue);
        if(debug_) std::cout << "Cut " << cutname << " updated to value " << cutSelector_->getCut(cutname) << std::endl;
    }

    //Loop over each cut indpendently, build background zvtx disribution. Fit zvtx with Gaus+Tail to determine zcut
    //read tritrig events. For each defined cut, check if event passes. If pass, fill z vtx distribution
    for(int e=0;  e < tritrigTree_->GetEntries(); e++){
        tritrigTree_->GetEntry(e);
        for(cut_iter_ it=cuts_->begin(); it!=cuts_->end(); it++){
            //Only consider vertices within defined mass window centered on Vd mass
            if(*tritrig_tuple_["unc_vtx_mass"]*1000.0 > highMass) continue;
            if(*tritrig_tuple_["unc_vtx_mass"]*1000.0 < lowMass) continue;
            std::string cutname = it->first;
            std::string cutvar = cutSelector_->getCutVar(cutname);
            if(!cutSelector_->passCutGTorLT(cutname, *tritrig_tuple_[cutvar]))
                continue;
            cutHistos_->Fill1DHisto("tritrig_zVtx_"+cutname+"_h",*tritrig_tuple_["unc_vtx_z"],1.0);
        }
    }
    
    //For each tritrig cut zvtx distribution, fit ZTail to determine zcut value
    if(debug_) std::cout << "Making tritrig zvtx distributions for each cut" << std::endl;
    std::map<std::string, double> zcuts;
    for(cut_iter_ it=cuts_->begin(); it!=cuts_->end(); it++){
        std::string cutname = it->first;
        std::cout << cutname << std::endl;
        double zcut = cutHistos_->fitZTail("tritrig_zVtx_"+cutname+"_h",100.0); //<- TO DO: make
        //100.0 configurable
        zcuts[cutname] = zcut;
        if(debug_) std::cout << "Zcut for cut " << cutname << ": " << zcut << "[mm]" << std::endl;
    }
    
    //Count background rate for each cut after applying the corresponding zcut
    std::map<std::string,double> scaled_backgrounds;
    std::map<std::string,double> n_offs;
    for(cut_iter_ it=cuts_->begin(); it!=cuts_->end(); it++){
        std::string cutname = it->first;
        scaled_backgrounds[cutname] = 0.0;
        n_offs[cutname] = 0.0;
    }
    for(int e=0;  e < tritrigTree_->GetEntries(); e++){
        tritrigTree_->GetEntry(e);
        for(cut_iter_ it=cuts_->begin(); it!=cuts_->end(); it++){
            //Only consider vertices within defined mass window centered on Vd mass
            if(*tritrig_tuple_["unc_vtx_mass"]*1000.0 > highMass) continue;
            if(*tritrig_tuple_["unc_vtx_mass"]*1000.0 < lowMass) continue;
            std::string cutname = it->first;
            std::string cutvar = cutSelector_->getCutVar(cutname);
            if(!cutSelector_->passCutGTorLT(cutname, *tritrig_tuple_[cutvar]))
                continue;
            if(*tritrig_tuple_["unc_vtx_z"] < zcuts[cutname]) continue;
            scaled_backgrounds[cutname] += mcScale_["tritrig"];
            n_offs[cutname] += 1.0;
        }
    }

    //Make signal selected vtxZ distribution for each cut...these selections are used to calculated NSig for each cut
    for(int e=0;  e < signalTree_->GetEntries(); e++){
        signalTree_->GetEntry(e);
        for(cut_iter_ it=cuts_->begin(); it!=cuts_->end(); it++){
            std::string cutname = it->first;
            std::string cutvar = cutSelector_->getCutVar(cutname);
            if(!cutSelector_->passCutGTorLT(cutname, *signal_tuple_[cutvar]))
                continue;
            if(*signal_tuple_["unc_vtx_z"] < zcuts[cutname]) continue;
            cutHistos_->Fill1DHisto("signal_vdSelZ_"+cutname+"_h",*signal_tuple_["true_vtx_z"],1.0);
        }
    }

    std::cout << "Calculating the expected signal, and therefore ZBi, for each cut" << std::endl;
    for(cut_iter_ it=cuts_->begin(); it!=cuts_->end(); it++){
        std::string cutname = it->first;
        if(debug_) std::cout << "Calculating Nsig for cut " << cutname << std::endl;
        //Get the selected vd vtxz distribution
        //In order to make TEfficiency with vdSimZ_h and vdSelZ_h, vdSelZ_h must be cloned from vdSimZ_h
        TH1F* vdSelZ_h = (TH1F*)cutHistos_->get1dHisto("cutHistos_signal_vdSelZ_"+cutname+"_h");
        TH1F* vdSelZ_h_clone = (TH1F*)vdSimZ_h_->Clone(("clone_cutHistos_signal_vdSelZ_"+cutname+"_h").c_str());
        for(int i=0; i<201; i++){
            vdSelZ_h_clone->SetBinContent(i,vdSelZ_h->GetBinContent(i));
        }
        //make efficiencies to get F(z)
        TEfficiency* effCalc_h = new TEfficiency(*vdSelZ_h_clone, *vdSimZ_h_);

        //perform NSig calculation for each of the two possible mesons and combine rates
        double logEps2 = -7.5;
        double eps2 = std::pow(10, logEps2);
        double eps = std::sqrt(eps2);
        double Nsig = 0.0;
        std::string mesons[2] = {"rho","phi"};
        for(int i =0; i < 2; i++){
            bool rho = false;
            bool phi = false;
            if(mesons[i] == "rho") rho = true;
            else phi = true;
            
            double ctau = simpEqs_->getCtau(mAp_MeV,m_pi,mV_MeV,eps,alpha_D,f_pi,m_l,rho);
            double E_V = 1.35; //GeV
            double gcTau = ctau * simpEqs_->gamma(mV_MeV/1000.0,E_V);

            double effVtx = 0.0;
            for(int zbin =0; zbin < 201; zbin++){
                double zz = vdSelZ_h_clone->GetBinLowEdge(zbin);
                if(zz < -4.3) continue;
                effVtx += (TMath::Exp((-4.3-zz)/gcTau)/gcTau)*(effCalc_h->GetEfficiency(zbin) - effCalc_h->GetEfficiencyErrorLow(zbin))*vdSelZ_h_clone->GetBinWidth(zbin);
            }

            double tot_apProd = (3.*137/2.)*3.14159*(mAp_MeV*eps2*radFrac*dNdm)/radAcc;
            double br_Vpi_val = simpEqs_->br_Vpi(mAp_MeV,m_pi,mV_MeV,alpha_D,f_pi,rho,phi);
            double br_V_to_ee = 1.0;

            Nsig = Nsig + tot_apProd*effVtx*br_V_to_ee*br_Vpi_val;
        }

       double n_on = Nsig + scaled_backgrounds[cutname];
       double tau = 1./mcScale_["tritrig"];
       double n_off = n_offs[cutname];

       //calculate ZBi value
       double ZBi = calculateZBi(n_on, n_off, tau);
       if(debug_){
           std::cout << "Values for cut " << cutname << std::endl;
           std::cout << "Nsig: " << Nsig << std::endl;
           std::cout << "Scaled tritrig: " << scaled_backgrounds[cutname];
           std::cout << "n_on: " << n_on << std::endl;
           std::cout << "n_off: " << n_off << std::endl;
           std::cout << "tau: " << tau << std::endl;
           std::cout << "ZBi: " << ZBi << std::endl;
       }
       std::pair<double, double> value_pair;
       value_pair.first = (double)cutSelector_->getCut(cutname);
       value_pair.second = ZBi;
       ZBi_matrix_[cutname].push_back(value_pair);
       //ZBi_matrix_[cutname].push_back(((double)cutSelector_->getCut(cutname),ZBi));

       //CLEAR POINTERS
       delete effCalc_h;
    }

    //Find the cut that results in the largest ZBi value
    double best_ZBi = -9999.9;
    std::string best_cutname;
    double best_cutvalue;
    for(std::map<std::string,std::vector<std::pair<double, double>>>::iterator it=ZBi_matrix_.begin(); it != ZBi_matrix_.end(); it++){
        std::string cutname = it->first; 
        double cutvalue = it->second.at(0).first;
        double zbi = it->second.at(0).second;
        if(zbi > best_ZBi){
            best_ZBi = zbi;
            best_cutname = cutname;
            best_cutvalue = cutvalue;
        }
    }
    if(debug_) std::cout << "Best cut " << best_cutname << " has value: " << best_cutvalue << " with ZBi: " << best_ZBi << std::endl;
}

void ZBiCutflowProcessor::finalize() {
    std::cout << "Finalize" << std::endl;
    outFile_->cd();
    signalHistos_->saveHistos(outFile_,"signal");
    outFile_->Close();

    //outFile_->cd();
    //signalHistos_->saveHistos(outFile_);

}

double ZBiCutflowProcessor::calculateZBi(double n_on, double n_off, double tau){
    double P_Bi = TMath::BetaIncomplete(1./(1.+tau),n_on,n_off+1);
    double Z_Bi = std::pow(2,0.5)*TMath::ErfInverse(1-2*P_Bi);
    return Z_Bi;
}

bool ZBiCutflowProcessor::doesCutVariableExist(std::string cutvariable){
    //Check that variable being used to cut actually exists in the signal data...
    //for(std::map<std::string,double*>::iterator it=signal_tuple_.begin(); it != signal_tuple.end(); it++){
    bool exists = true;

    if(signal_tuple_.find(cutvariable) == signal_tuple_.end()){
        std::cout << "[ZBiCutflowProcessor] WARNING::Cut variable " << cutvariable << " does not exist in the signal data" << std::endl;
        std::cout << "Removing " << cutvariable << " from list of cut variables." << std::endl;
        bool exists = false;
    }
    if(tritrig_tuple_.find(cutvariable) == tritrig_tuple_.end()){
        std::cout << "[ZBiCutflowProcessor] WARNING::Cut variable " << cutvariable << " does not exist in the tritrig data" << std::endl;
        std::cout << "Removing " << cutvariable << " from list of cut variables." << std::endl;
        bool exists = false;
    }
    if(exists == false){
        auto itr = std::find(cutVariables_.begin(), cutVariables_.end(), cutvariable);
        if (itr != cutVariables_.end()) cutVariables_.erase(itr);
    }
    return exists;
}

DECLARE_PROCESSOR(ZBiCutflowProcessor);
