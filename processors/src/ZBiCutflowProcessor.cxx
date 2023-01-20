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
    std::cout << "[ZBiCutflowProcessor] Initialize" << std::endl;

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

    //Initialize signal tuple
    if(debug_) std::cout << "Reading tuple from signal file" << std::endl;
    TFile* signalFile = new TFile(signalFilename_.c_str(),"READ");
    //TODO make this selection configurable
    signalTree_ = (TTree*)signalFile->Get("vtxana_kf_Tight_2016_simp_reach_dev/vtxana_kf_Tight_2016_simp_reach_dev_tree");
    readFlatTuple(signalTree_, signal_tuple_);

    //Initialize tritrig tuple
    if(debug_) std::cout << "Reading tuple from tritrig file" << std::endl;
    TFile* tritrigFile = new TFile(tritrigFilename_.c_str(),"READ");
    //TODO make this selection configurable
    tritrigTree_ = (TTree*)tritrigFile->Get("vtxana_kf_Tight_2016_simp_reach_dev/vtxana_kf_Tight_2016_simp_reach_dev_tree");
    readFlatTuple(tritrigTree_, tritrig_tuple_);

    //init cut selector and load cuts from configuration file
    cutSelector_ = new IterativeCutSelector("iterativeCuts",cuts_cfgFile_);
    cutSelector_->setDebug(debug_);
    cutSelector_->LoadSelection();
    //Get the map of loaded cuts
    cuts_ = cutSelector_->getCuts();

    std::cout << "Checking that list of cuts exists in the data" << std::endl;
    //Only allow cut variable to persist if it exists in the data
    for(std::vector<std::string>::iterator it=cutVariables_.begin(); it !=cutVariables_.end(); it++){
        std::string cutvar = (std::string)*it;
        doesCutVariableExist(cutvar);
    }

    std::cout << "Only use cuts that correspond to the configured list of variables to cut on" << std::endl;
    //Only allow cut to persist if the variable it cuts on is in the list of cutvariables
    for(cut_iter_ it=cuts_.begin(); it!=cuts_.end(); it++){
        std::string cutname = it->first;
        bool found = false;
        for(std::vector<std::string>::iterator iit=cutVariables_.begin(); iit !=cutVariables_.end(); iit++){
            std::string cutvar = (std::string)*iit;
            found = cutSelector_->hasCut(cutvar);
        }
        if(!found){
            std::cout << "[ZBiCutflowProcessor] WARNING::Cut variable corresponding to " << cutname << " does not exist!" << std::endl;
            std::cout << "Removing " << cutname << " from list of cuts." << std::endl;
            cutSelector_->eraseCut(cutname);
        }
    }

    std::cout << "Getting pretrigger vtx distribution" << std::endl;
    //Get the pretrigger vtx distribution ONLY NEED TO DO THIS ONCE!
    std::cout << vdSimFilename_ << std::endl;
    TFile* vdSimFile = new TFile(vdSimFilename_.c_str(), "READ");
    std::cout << "Getting mcAna_mc625Z_h" << std::endl;
    TH1F* vdSimZ_h = (TH1F*)vdSimFile->Get("mcAna/mcAna_mc625Z_h");
    std::cout << "initializing vdSimZ_h_" << std::endl;
    vdSimZ_h_ = new TH1F("signal_vdSimZ_h","signal_vdSimZ;true z_{vtx} [mm];events", 200, -50.3, 149.7);
    std::cout << "Filling histogram" << std::endl;
    for(int i=0; i < 201; i++)
        vdSimZ_h_->SetBinContent(i,vdSimZ_h->GetBinContent(i));
    std::cout << "Clearing pointers" << std::endl;

    vdSimFile->Close();
    delete vdSimFile;

    std::cout << "Initializing z vertex distributions for signal and tritrig" << std::endl;
    //Initialize z vertex distribution histograms corresponding to each cut. These are used to calculate
    //expected signal and number of background events for each cut. 
    cutHistos_ = new ZBiHistos("");
    cutHistos_->debugMode(debug_);
    for(cut_iter_ it=cuts_.begin(); it!=cuts_.end(); it++){
        std::string cutname = it->first;
        cutHistos_->get1dHistos()["signal_vdSelZ_"+cutname+"_h"] = cutHistos_->plot1D("signal_vdSelZ_"+cutname+"_h","true z_{vtx} [mm]", 200, -50.3, 149.7);
        cutHistos_->get1dHistos()["tritrig_zVtx_"+cutname+"_h"] = cutHistos_->plot1D("tritrig_zVtx_"+cutname+"_h","unc z_{vtx} [mm]", 150, -50.0, 100.0);
    }

    std::cout << "Filling initial signal variable histograms" << std::endl;
    //Fill signal variable histograms with initial values for each variable
    if(debug_)
        std::cout << "Filling initial signal histograms for each variable corresponding to cut selection" << std::endl;
    for(int e=0; e < 500; e++){
        signalTree_->GetEntry(e);
        //Loop over each signal variable, if cut exists for variable, fill histogram for that variable
        for(std::vector<std::string>::iterator it=cutVariables_.begin(); it !=cutVariables_.end(); it++){
            std::string cutvar = (std::string)*it;
            //if no cut exists using this variable, skip it
            if(!cutSelector_->hasCut(cutvar)) continue;
            if(signalHistos_->get1dHistos().count("signal_"+cutvar+"_h")){
                signalHistos_->Fill1DHisto(cutvar+"_h",*signal_tuple_[cutvar]);
            }
            else{
                std::cout << "No Histogram named " << cutvar+"_h" << " exists for variable " << cutvar << std::endl; 
                abort();
            }
        }
    }

    std::cout << "Getting intial signal histogram integrals to be used as reference for signal cutting" << std::endl;
    //Get intial integral values for each signal cut variable histogram
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
    for(cut_iter_ it=cuts_.begin(); it!=cuts_.end(); it++){
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
            if(!cutSelector_->passCutGTorLT(cutname, *tritrig_tuple_[cutvar]));
                continue;
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
            if(!cutSelector_->passCutGTorLT(cutname, *tritrig_tuple_[cutvar]));
                continue;
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
            if(!cutSelector_->passCutGTorLT(cutname, *signal_tuple_[cutvar]));
                continue;
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
        double Nsig = 0.0;

        //perform NSig calculation for each of the two possible mesons and combine rates
        std::string mesons[2] = {"rho","phi"};
        for(int i =0; i < sizeof(mesons); i++){
            bool rho = false;
            bool phi = false;
            if(mesons[i] == "rho") rho = true;
            else phi = true;
            
            double ctau = simpEqs_->getCtau(mAp_MeV,m_pi,mV_MeV,eps,alpha_D,f_pi,m_l,rho);
            double E_V = 1.35; //GeV
            double gcTau = ctau * simpEqs_->gamma(mV_MeV/1000.0,E_V);

            double effVtx = 0.0;
            for(int zbin =0; zbin < 201; zbin++){
                double zz = vdSelZ_h->GetBinLowEdge(zbin);
                if(zz < -4.3) continue;
                effVtx += (TMath::Exp((-4.3-zz)/gcTau)/gcTau)*(effCalc_h->GetEfficiency(zbin) - effCalc_h->GetEfficiencyErrorLow(zbin))*vdSelZ_h->GetBinWidth(zbin);
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
       //ZBi_matrix_[cutname].push_back(std::pair((double)cutSelector_->getCut(cutname),ZBi));

       //CLEAR POINTERS
       delete vdSelZ_h;
       delete vdSimZ_h;
       delete effCalc_h;
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
