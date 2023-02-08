#include "ZBiCutflowProcessor.h"
//#include <FlatTupleMaker.h>
#include <string>
#include <cstdlib>
#include <iostream>
#include <fstream>

ZBiCutflowProcessor::ZBiCutflowProcessor(const std::string& name, Process& process) : Processor(name,process) {
    std::cout << "[ZBiCutflowPRocessor] Constructor()" << std::endl;

}

ZBiCutflowProcessor::~ZBiCutflowProcessor(){}

void ZBiCutflowProcessor::configure(const ParameterSet& parameters) {
    std::cout << "[ZBiCutflowProcessor] configure()" << std::endl;
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
        ztail_events_ = parameters.getDouble("ztail_events",ztail_events_);
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
        double* value = new double;
        std::string varname = (std::string)br->GetFullName();
        tuple_map[varname] = value;
        tree->SetBranchAddress(varname.c_str(),tuple_map[varname]);
    }
}

void ZBiCutflowProcessor::initialize(std::string inFilename, std::string outFilename){
    std::cout << "[ZBiCutflowProcessor] Initialize" << std::endl;

    //initialize cut selector for 'Persistent Cuts' (which are applied to every event)
    persistentCutsSelector_ = new IterativeCutSelector("bestCuts", cuts_cfgFile_);
    persistentCutsSelector_->LoadSelection();
    persistentCuts_ = persistentCutsSelector_->getPointerToCuts();

    /*
    if(debug_){
        std::cout << "[Persistent Cuts] initialized: " << std::endl;
        persistentCutsSelector_->printCuts();
    }
    */

    //initalize Test Cuts (which will be iteratively changed)
    testCutsSelector_ = new IterativeCutSelector("testCuts",cuts_cfgFile_);
    testCutsSelector_->LoadSelection();
    testCuts_ = testCutsSelector_->getPointerToCuts();
    /*
    if(debug_){ 
        std::cout << "[Test Cuts] initialized. " << std::endl;
        testCutsSelector_->printCuts();
    }
    */

    //Only allow cut to persist if the variable it cuts on is in the list of cutvariables
    if(debug_) std::cout << "Removing all cuts whose variables are not specified in list of 'cut variables'" << std::endl;
    for(cut_iter_ it=testCuts_->begin(); it!=testCuts_->end();){
        std::string cutname = it->first;
        std::string cutvariable = testCutsSelector_->getCutVar(cutname);
        bool found = false;
        for(std::vector<std::string>::iterator iit=cutVariables_.begin(); iit !=cutVariables_.end(); iit++){
            if((std::string)*iit == cutvariable){
                found = true;
                break;
            }
        }
        if(!found){
            std::cout << "[ZBiCutflowProcessor] WARNING::Cut variable corresponding to " << cutname << " does not exist!" << std::endl;
            std::cout << "Removing " << cutname << " from list of cuts." << std::endl;
            it = testCuts_->erase(it);
        }
        else
            ++it;
    }

    for(cut_iter_ it=persistentCuts_->begin(); it!=persistentCuts_->end();){
        std::string cutname = it->first;
        std::string cutvariable = persistentCutsSelector_->getCutVar(cutname);
        bool found = false;
        for(std::vector<std::string>::iterator iit=cutVariables_.begin(); iit !=cutVariables_.end(); iit++){
            if((std::string)*iit == cutvariable){
                found = true;
                break;
            }
        }
        if(!found){
            it = persistentCuts_->erase(it);
        }
        else
            ++it;
    }

    if(debug_){ 
        std::cout << "[Test Cuts] Initialized and Filtered:" << std::endl;
        testCutsSelector_->printCuts();
    }

    if(debug_){
        std::cout << "[Persistent Cuts] Initialized and Filtered:" << std::endl;
        persistentCutsSelector_->printCuts();
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
    signalHistos_->defineAnalysisHistos();

    ////Initialize  tritrig histograms for each variable
    tritrigHistos_ = new ZBiHistos("tritrig");
    tritrigHistos_->debugMode(debug_);
    tritrigHistos_->loadHistoConfig(signalHistCfgFilename_);
    tritrigHistos_->DefineHistos();
    tritrigHistos_->defineAnalysisHistos();

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
    //This is used to calculate the Efficiency Vertex (therefore Nsig) later
    std::cout << "Getting pretrigger signal vtx distribution" << std::endl;
    vdSimZ_h_ = new TH1F("signal_vdSimZ_h_","signal_vdSimZ;true z_{vtx} [mm];events", 200, -50.3, 149.7);
    TFile* vdSimFile = new TFile(vdSimFilename_.c_str(), "READ");
    TH1F* vdSimZ_h = (TH1F*)vdSimFile->Get("mcAna/mcAna_mc625Z_h");
    for(int i=0; i < 201; i++){
        vdSimZ_h_->SetBinContent(i,vdSimZ_h->GetBinContent(i));
    }
    vdSimFile->Close();
    delete vdSimFile;

    //Initialize  vertex z distribution histograms corresponding to each cut. These are used to calculate
    //expected signal and number of background events for each cut. 
    std::cout << "Initializing z vertex distributions for selected signal and tritrig" << std::endl;
    cutHistos_ = new ZBiHistos("cutHistos");
    cutHistos_->debugMode(debug_);
    cutHistos_->defineCutlistHistos(*testCuts_);

    debugHistos_ = new ZBiHistos("debug");
    //Initialize debug zbi histograms
    for(cut_iter_ it=testCuts_->begin(); it!=testCuts_->end(); it++){
        std::string cutname = it->first;
        debugHistos_->addHisto1d(cutname+"_ZBi_tritrigStats_h","iterations",100,-0.5,99.5);
        debugHistos_->addHisto1d(cutname+"_ZBi_vdStats_h","iterations",100,-0.5,99.5);
        debugHistos_->addHisto1d(cutname+"_Noff_unscaled_h","iterations",100,-0.5,99.5);
        debugHistos_->addHisto1d(cutname+"_Nbkg_scaled_h","iterations",100,-0.5,99.5);
        debugHistos_->addHisto1d(cutname+"_Nsig_h","iterations",100,-0.5,99.5);
        debugHistos_->addHisto1d(cutname+"_Non_h","iterations",100,-0.5,99.5);
        debugHistos_->addHisto1d(cutname+"_Zcut_h","iterations",100,-0.5,99.5);
    }
    debugHistos_->addHisto1d("Best_ZBi_h","iterations",100,-0.5,99.5);
    debugHistos_->addHisto1d("Best_cut_Noff_unscaled_h","iterations",100,-0.5,99.5);
    debugHistos_->addHisto1d("Best_cut_Nbkg_scaled_h","iterations",100,-0.5,99.5);
    debugHistos_->addHisto1d("Best_cut_Nsig_h","iterations",100,-0.5,99.5);
    debugHistos_->addHisto1d("Best_cut_Non_h","iterations",100,-0.5,99.5);
    debugHistos_->addHisto1d("Best_cut_Zcut_h","iterations",150,-0.5,99.5);

    //Initialize various values (NEED TO BE CONFIGURABLE THROUGH SINGLE JSON)
    double mV_MeV = 55.0;
    double mAp_MeV = mV_MeV*(3/1.8);
    double radFrac = 0.07;
    double radAcc = 0.125;
    double massRes_MeV = 3.0;
    //double dNdm = 372000.0; Not sure where I got this value from anymore...
    double dNdm = 513800.0;

    double m_pi = mAp_MeV/3.0;
    double alpha_D = 0.01;
    double m_l = 0.511;
    double f_pi = m_pi/(4.*M_PI);

    lowMass_ = mV_MeV - 2.0*massRes_MeV/2.0;
    highMass_ = mV_MeV + 2.0*massRes_MeV/2.0;

    //Calculate the Impact Parameter Cut (Z0 vs VtxZ) using the signal distribution
    std::cout << "Calculating Impact Parameter Cut" << std::endl;
    calculateImpactParameterCut();

    //Fill signal variable histograms with initial values for each variable
    std::cout << "Filling initial signal histograms for each variable corresponding to cut selection" << std::endl;
    for(int e=0;  e < signalTree_->GetEntries(); e++){
        signalTree_->GetEntry(e);
        if(*signal_tuple_["unc_vtx_mass"]*1000.0 > highMass_) continue;
        if(*signal_tuple_["unc_vtx_mass"]*1000.0 < lowMass_) continue;

        //Apply the impact parameter cut 
        if(failImpactParameterCut(signal_tuple_))
            continue;

        for(std::map<std::string,double*>::iterator it=signal_tuple_.begin(); it != signal_tuple_.end(); it++){
            std::string var = it->first;
            if(signalHistos_->get1dHistos().count("signal_"+var+"_h")){
                signalHistos_->Fill1DHisto(var+"_h",*signal_tuple_[var]);
            }
        }
    }

    //Get intial integral values for each signal cut variable histogram
    std::cout << "Getting initial signal histogram integrals to be used as reference for signal cutting" << std::endl;
    for(std::vector<std::string>::iterator it=cutVariables_.begin(); it !=cutVariables_.end(); it++){
        std::string cutvar = (std::string)*it;
        initialIntegrals_[cutvar] = signalHistos_->getIntegral("signal_"+cutvar+"_h");
    }
    signalHistos_->writeHistos(outFile_,"");

    //Set the values for the initial set of 'persistentCuts_' based on the signal variable histograms cutting 0%
    for(cut_iter_ it=persistentCuts_->begin(); it!=persistentCuts_->end(); it++){
        std::string cutname = it->first;
        std::string cutvar = persistentCutsSelector_->getCutVar(cutname);
        bool isCutGT = testCutsSelector_->isCutGreaterThan(cutname);
        double cutvalue = signalHistos_->cutFractionOfIntegral("signal_"+cutvar+"_h", isCutGT, 0.0, initialIntegrals_[cutvar]);
        persistentCutsSelector_->setCutValue(cutname, cutvalue);
        if(debug_) std::cout << "[Persistent Cuts] Cut "<< cutname << " updated to: " << persistentCutsSelector_->getCut(cutname) << std::endl;
    }


    //DEBUGGING EQUATIONS//
    double logEps2 = -7.5;
    double eps2 = std::pow(10, logEps2);
    double eps = std::sqrt(eps2);
    SimpEquations *debugEQs = new SimpEquations();
    bool rho = true;
    bool phi = false;
    double br_vpi = simpEqs_->br_Vpi(mAp_MeV,m_pi,mV_MeV,alpha_D,f_pi,rho,phi);
    double ctau = simpEqs_->getCtau(mAp_MeV,m_pi,mV_MeV,eps,alpha_D,f_pi,m_l,rho);
    double E_V = 1.35; //GeV
    double gcTau = ctau * simpEqs_->gamma(mV_MeV/1000.0,E_V);
}

bool ZBiCutflowProcessor::process(){
    std::cout << "[ZBiCutflowProcessor] process()" << std::endl;

    //Initialize various values (NEED TO BE CONFIGURABLE THROUGH SINGLE JSON)
    double mV_MeV = 55.0;
    double mAp_MeV = mV_MeV*(3/1.8);
    double radFrac = 0.07;
    double radAcc = 0.125;
    double massRes_MeV = 3.0;
    //double dNdm = 372000.0; <- not sure where i got this number...
    double dNdm = 513800.0;

    double m_pi = mAp_MeV/3.0;
    double alpha_D = 0.01;
    double m_l = 0.511;
    double f_pi = m_pi/(4.*M_PI);

    double lowMass_ = mV_MeV - 2.0*massRes_MeV/2.0;
    double highMass_ = mV_MeV + 2.0*massRes_MeV/2.0;

    //Each iteration loops over the set of cuts being tested. The expected signal and background
    //rate is calculated given each cut independently. 
    //We calculate ZBi, the chosen metric of signal vs background, for each cut.
    //Within an iteration, the cut the gives the largest ZBi value is selected as the best_cut.
    //On the next iteration, the best_cut is applied to all events, then the process repeats.
    
    double cutFraction = 0.005; //<- TODO: Make configurable


    for(int iteration = 1; iteration < 50; iteration ++){
        if(debug_) std::cout << "############### ITERATION " 
            << iteration << " #####################" << std::endl;

        //Reset histograms at the start of each iteration
        if(debug_) std::cout << "Resetting histograms" << std::endl;
        signalHistos_->resetHistograms1d();
        tritrigHistos_->resetHistograms1d();
        cutHistos_->resetHistograms1d();

        //Fill signal variable histograms after applying the persistent cuts
        for(int e=0;  e < signalTree_->GetEntries(); e++){
            signalTree_->GetEntry(e);
            //Cut events outside the chosen mass window
            if(*signal_tuple_["unc_vtx_mass"]*1000.0 > highMass_) continue;
            if(*signal_tuple_["unc_vtx_mass"]*1000.0 < lowMass_) continue;

            //Apply the impact parameter cut 
            if(failImpactParameterCut(signal_tuple_))
                continue;

            //Apply persistent cuts
            if(failPersistentCuts(signal_tuple_))
                continue;

            //Fill signal variable histograms
            for(std::map<std::string,double*>::iterator it=signal_tuple_.begin(); it != signal_tuple_.end(); it++){
                std::string var = it->first;
                if(signalHistos_->get1dHistos().count("signal_"+var+"_h")){
                    signalHistos_->Fill1DHisto(var+"_h",*signal_tuple_[var]);
                }
            }
        }
        
        //For each Test Cut, find the cut value that cuts the specified fraction of signal in that variable
        for(cut_iter_ it=testCuts_->begin(); it!=testCuts_->end(); it++){
            std::string cutname = it->first;
            std::string cutvar = testCutsSelector_->getCutVar(cutname);
            if(debug_){
                std::cout << "[Test Cuts] Finding test cut value for: " << cutname << " | Cutvar: " << cutvar << std::endl;
            }
            bool isCutGT = testCutsSelector_->isCutGreaterThan(cutname);
            double cutvalue = signalHistos_->cutFractionOfIntegral("signal_"+cutvar+"_h", isCutGT, iteration*cutFraction, initialIntegrals_[cutvar]);
            testCutsSelector_->setCutValue(cutname, cutvalue);
            if(debug_) std::cout << "[Test Cuts]: " << cutname << " updated to value " << testCutsSelector_->getCut(cutname) << std::endl;
        }

        //Loop over each Test Cut and build the background vtx z disribution. 
        //Fit vtx z with Gaus+Tail to determine zcut 
        if(debug_) std::cout << "Calculating zcut for each cut" << std::endl;
        for(int e=0;  e < tritrigTree_->GetEntries(); e++){
            tritrigTree_->GetEntry(e);
            //Only consider vertices within defined mass window centered on Vd mass
            if(*tritrig_tuple_["unc_vtx_mass"]*1000.0 > highMass_) continue;
            if(*tritrig_tuple_["unc_vtx_mass"]*1000.0 < lowMass_) continue;

            //Apply persistent cuts
            if(failPersistentCuts(tritrig_tuple_))
                continue;

            //Before Impact parameter cut
            tritrigHistos_->Fill2DHisto("z0_v_recon_z_hh",*tritrig_tuple_["unc_vtx_z"],*tritrig_tuple_["unc_vtx_ele_track_z0"]);
            tritrigHistos_->Fill2DHisto("z0_v_recon_z_hh",*tritrig_tuple_["unc_vtx_z"],*tritrig_tuple_["unc_vtx_pos_track_z0"]);

            //Apply the impact parameter cut 
            if(failImpactParameterCut(tritrig_tuple_))
                continue;

            //Plot after impact parameter cut
            tritrigHistos_->Fill2DHisto("z0_v_recon_z_post_impactparam_cut_hh",*tritrig_tuple_["unc_vtx_z"],*tritrig_tuple_["unc_vtx_ele_track_z0"]);
            tritrigHistos_->Fill2DHisto("z0_v_recon_z_post_impactparam_cut_hh",*tritrig_tuple_["unc_vtx_z"],*tritrig_tuple_["unc_vtx_pos_track_z0"]);

            //Fill tritrig variable histos before Test Cut and Zcut are applied
            for(std::map<std::string,double*>::iterator it=tritrig_tuple_.begin(); it != tritrig_tuple_.end(); it++){
                std::string var = it->first;
                if(tritrigHistos_->get1dHistos().count("tritrig_"+var+"_h")){
                    tritrigHistos_->Fill1DHisto(var+"_h",*tritrig_tuple_[var]);
                }
            }

            //Fill tritrig zvtx
            for(cut_iter_ it=testCuts_->begin(); it!=testCuts_->end(); it++){
                std::string cutname = it->first;
                std::string cutvar = testCutsSelector_->getCutVar(cutname);
                double cutvalue = testCutsSelector_->getCut(cutname);
                if(!testCutsSelector_->passCutGTorLT(cutname, *tritrig_tuple_[cutvar]))
                    continue;

                //If event passes, fill zVtx distribution
                cutHistos_->Fill1DHisto("tritrig_zVtx_"+cutname+"_h",*tritrig_tuple_["unc_vtx_z"],1.0);
            }
        }

        //Find zcut value for each Test Cut by fitting the tail of the bkg zvtx distribution
        if(debug_) std::cout << "Calculating Zcut for each cut" << std::endl;
        std::map<std::string, double> zcuts;
        for(cut_iter_ it=testCuts_->begin(); it!=testCuts_->end(); it++){
            std::string cutname = it->first;
            std::cout << cutname << std::endl;
            //double zcut = cutHistos_->fitZTail("tritrig_zVtx_"+cutname+"_h",10.0); //<- TO DO: make
            double zcut = cutHistos_->shosFitZTail(cutname,ztail_events_); //<- TO DO: make
            zcuts[cutname] = zcut;
            if(debug_) std::cout << "Zcut for cut " << cutname << ": " << zcut << "[mm]" << std::endl;
        }
        
        //At this stage, for each Test Cut, the zcut value has been found. 
        //Using this zcut, we count the background events, measure the signal zvt selection 
        //efficiency, and then calculate the expected signal
        std::map<std::string,double> scaled_backgrounds;
        std::map<std::string,double> n_offs;

        //Initialize map to hold the background rate
        for(cut_iter_ it=testCuts_->begin(); it!=testCuts_->end(); it++){
            std::string cutname = it->first;
            scaled_backgrounds[cutname] = 0.0;
            n_offs[cutname] = 0.0;
        }

        //Count the background rate for each cut
        if(debug_) std::cout << "Initial tritrig count: " << tritrigTree_->GetEntries() << std::endl;
        for(int e=0;  e < tritrigTree_->GetEntries(); e++){
            tritrigTree_->GetEntry(e);
            //Mass Window
            if(*tritrig_tuple_["unc_vtx_mass"]*1000.0 > highMass_) continue;
            if(*tritrig_tuple_["unc_vtx_mass"]*1000.0 < lowMass_) continue;

            //Apply the impact parameter cut 
            if(failImpactParameterCut(tritrig_tuple_))
                continue;

            //Apply persistent cuts
            if(failPersistentCuts(tritrig_tuple_))
                continue;

            //Apply Test Cuts (and corresponding Zcut) independently and count background 
            for(cut_iter_ it=testCuts_->begin(); it!=testCuts_->end(); it++){
                std::string cutname = it->first;
                std::string cutvar = testCutsSelector_->getCutVar(cutname);
                //Test Cut
                if(!testCutsSelector_->passCutGTorLT(cutname, *tritrig_tuple_[cutvar]))
                    continue;
                //Zcut for this Test Cut
                if(*tritrig_tuple_["unc_vtx_z"] < zcuts[cutname]) continue;

                scaled_backgrounds[cutname] += mcScale_["tritrig"];
                n_offs[cutname] += 1.0;
            }
        }
        if(debug_){
            for(cut_iter_ it=testCuts_->begin(); it!=testCuts_->end(); it++){
                std::cout << "N Tritrig Events After applying test cut " << it->first << " " << 
                    testCutsSelector_->getCut(it->first) << ": " << n_offs[it->first] << std::endl;
            }

        }

        //Make signal selected vtxZ distribution for each cut...
        //these selections are used to calculated NSig for each cut
        for(int e=0;  e < signalTree_->GetEntries(); e++){
            signalTree_->GetEntry(e);
            //Mass Window
            if(*signal_tuple_["unc_vtx_mass"]*1000.0 > highMass_) continue;
            if(*signal_tuple_["unc_vtx_mass"]*1000.0 < lowMass_) continue;

            //Apply the impact parameter cut 
            if(failImpactParameterCut(signal_tuple_))
                continue;

            //Apply persistent cuts
            if(failPersistentCuts(signal_tuple_))
                continue;

            //Loop over Test Cuts
            for(cut_iter_ it=testCuts_->begin(); it!=testCuts_->end(); it++){
                std::string cutname = it->first;
                std::string cutvar = testCutsSelector_->getCutVar(cutname);
                double cutvalue = testCutsSelector_->getCut(cutname);
                //Test Cut
                if(!testCutsSelector_->passCutGTorLT(cutname, *signal_tuple_[cutvar]))
                    continue;
                //Zcut for this Test Cut
                if(*signal_tuple_["unc_vtx_z"] < zcuts[cutname]) continue;

                //Build signal Vd Selection 
                cutHistos_->Fill1DHisto("signal_vdSelZ_"+cutname+"_h",*signal_tuple_["true_vtx_z"],1.0);
            }
        }

        //Calculate Nsig, and ZBi, for each Test Cut list
        std::map<std::string,std::pair<double,double>> iter_zbi_map;
        std::map<std::string, double> Nsig_map;
        for(cut_iter_ it=testCuts_->begin(); it!=testCuts_->end(); it++){
            //Get the selected vd vtxz distribution
            //In order to make TEfficiency with vdSimZ_h and vdSelZ_h, 
            //vdSelZ_h must be cloned from vdSimZ_h
            std::string cutname = it->first;
            TH1F* vdSelZ_h = (TH1F*)cutHistos_->get1dHisto("cutHistos_signal_vdSelZ_"+cutname+"_h");
            TH1F* vdSelZ_h_clone = (TH1F*)vdSimZ_h_->Clone(("clone_cutHistos_signal_vdSelZ_"+cutname+"_h").c_str());
            for(int i=0; i<201; i++){
                vdSelZ_h_clone->SetBinContent(i,vdSelZ_h->GetBinContent(i));
            }
            //make efficiencies to get F(z)
            TEfficiency* effCalc_h = new TEfficiency(*vdSelZ_h_clone, *vdSimZ_h_);

            //perform NSig calculation for each of the two possible mesons and combine rates
            double logEps2 = -6.5;
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
                    if(zz < zcuts[cutname]) continue;
                    effVtx += (TMath::Exp((-4.3-zz)/gcTau)/gcTau)*(effCalc_h->GetEfficiency(zbin) - effCalc_h->GetEfficiencyErrorLow(zbin))*vdSelZ_h_clone->GetBinWidth(zbin);
                }

                double tot_apProd = (3.*137/2.)*3.14159*(mAp_MeV*eps2*radFrac*dNdm)/radAcc;
                double br_Vpi_val = simpEqs_->br_Vpi(mAp_MeV,m_pi,mV_MeV,alpha_D,f_pi,rho,phi);
                double br_V_to_ee = 1.0;

                Nsig = Nsig + tot_apProd*effVtx*br_V_to_ee*br_Vpi_val;
            }

           Nsig_map[cutname] = Nsig;
           double n_on = Nsig + scaled_backgrounds[cutname];
           double tau = 1./mcScale_["tritrig"];
           double n_off = n_offs[cutname];

           //calculate ZBi value
           double ZBi = calculateZBi(n_on, n_off, tau);
           if(debug_){
               std::cout << "[Test Cut] Values" << cutname << std::endl;
               std::cout << "Nsig: " << Nsig << std::endl;
               std::cout << "Scaled tritrig: " << scaled_backgrounds[cutname];
               std::cout << "n_on: " << n_on << std::endl;
               std::cout << "n_off: " << n_off << std::endl;
               std::cout << "tau: " << tau << std::endl;
               std::cout << "ZBi: " << ZBi << std::endl;
           }
           std::pair<double, double> value_pair;
           value_pair.first = (double)testCutsSelector_->getCut(cutname);
           value_pair.second = ZBi;
           iter_zbi_map[cutname] = value_pair;
           global_ZBi_map_[cutname].push_back(value_pair);

           //fill debug histos
           for(int i=0; i < n_offs[cutname]; i++){
               debugHistos_->Fill1DHisto(cutname+"_ZBi_tritrigStats_h",(double)iteration,ZBi/n_offs[cutname]);
           }

           for(int i=0; i < vdSelZ_h_clone->GetEntries(); i++){
               debugHistos_->Fill1DHisto(cutname+"_ZBi_vdStats_h",(double)iteration, ZBi/vdSelZ_h_clone->GetEntries());
           }
           debugHistos_->get1dHisto("debug_"+cutname+"_Zcut_h")->SetBinContent(iteration, zcuts[cutname]);
           debugHistos_->get1dHisto("debug_"+cutname+"_Noff_unscaled_h")->SetBinContent(iteration,n_offs[cutname]);
           debugHistos_->get1dHisto("debug_"+cutname+"_Nbkg_scaled_h")->SetBinContent(iteration,scaled_backgrounds[cutname]);
           debugHistos_->get1dHisto("debug_"+cutname+"_Nsig_h")->SetBinContent(iteration,Nsig);
           debugHistos_->get1dHisto("debug_"+cutname+"_Non_h")->SetBinContent(iteration,n_on);

           //CLEAR POINTERS
           delete effCalc_h;

        }

        //Find the cut that results in the largest ZBi value
        double best_ZBi = -9999.9;
        std::string best_cutname;
        double best_cutvalue;
        for(std::map<std::string,std::pair<double,double>>::iterator it=iter_zbi_map.begin(); it!=iter_zbi_map.end(); it++){
            std::string cutname = it->first; 
            double cutvalue = it->second.first;
            double zbi = it->second.second;
            if(zbi > best_ZBi){
                best_ZBi = zbi;
                best_cutname = cutname;
                best_cutvalue = cutvalue;
            }
        }

        if(debug_){
            std::cout << "Iteration " << iteration << " Best Test Cut is " << best_cutname << " " 
                << best_cutvalue << " with ZBi=" << best_ZBi << std::endl;
            std::cout << "Update persistent cuts list with this best test cut..." << std::endl;
            std::cout << "[Persistent Cuts] Before update:" << std::endl;
            persistentCutsSelector_->printCuts();
        }

        //Keep the cut that results in the largest ZBi value and apply that cut
        //to all events in the next iteration
        persistentCutsSelector_->setCutValue(best_cutname, best_cutvalue);
        if(debug_){
            std::cout << "[Persistent Cuts] After update:" << std::endl;
            persistentCutsSelector_->printCuts();
        }

        //Write iteration histos
        signalHistos_->writeHistos(outFile_,"signal_iter_"+std::to_string(iteration));
        tritrigHistos_->writeHistos(outFile_,"tritrig_iter_"+std::to_string(iteration));
        cutHistos_->writeHistos(outFile_, "testCuts_iter_"+std::to_string(iteration));


        //Fill debug histo
        debugHistos_->get1dHisto("debug_Best_ZBi_h")->SetBinContent(iteration, best_ZBi);
        debugHistos_->get1dHisto("debug_Best_cut_Noff_unscaled_h")->SetBinContent(iteration, n_offs[best_cutname]);
        debugHistos_->get1dHisto("debug_Best_cut_Nbkg_scaled_h")->SetBinContent(iteration, scaled_backgrounds[best_cutname]);
        debugHistos_->get1dHisto("debug_Best_cut_Nsig_h")->SetBinContent(iteration, Nsig_map[best_cutname]);
        debugHistos_->get1dHisto("debug_Best_cut_Non_h")->SetBinContent(iteration, Nsig_map[best_cutname]+scaled_backgrounds[best_cutname]);
        debugHistos_->get1dHisto("debug_Best_cut_Zcut_h")->SetBinContent(iteration, zcuts[best_cutname]);
    }

    //After all iterations, find the single best cut (resulting in largest overall ZBi value)
    double final_best_ZBi = -9999.9;
    std::string final_best_cutname;
    double final_best_cutvalue;

    //Find the largest ZBI value for each cut
    std::map<std::string,std::pair<double,double>> cut_best_zbi_map;
    for(std::map<std::string,std::vector<std::pair<double, double>>>::iterator it=global_ZBi_map_.begin(); it != global_ZBi_map_.end(); it++){
        std::string cutname = it->first; 
        std::vector<std::pair<double,double>> cut_pairs = it->second;

        double cut_best_zbi = -9999.9;
        std::pair<double,double> cut_best_pair;
        for(std::vector<std::pair<double,double>>::iterator itt=cut_pairs.begin(); itt!=cut_pairs.end(); itt++){
            if(itt->second > cut_best_zbi){
                cut_best_pair = *itt;
            }
        }
        cut_best_zbi_map[cutname] = cut_best_pair;
        if(debug_) std::cout << "Largest ZBi for cut " << cutname << " is " << cut_best_pair.second << " with cutvalue " << cut_best_pair.first << std::endl;

        if(cut_best_pair.second > final_best_ZBi){
            final_best_ZBi = cut_best_pair.second;
            final_best_cutvalue = cut_best_pair.first;
            final_best_cutname = cutname;
        }
    }

    if(debug_) std::cout << "Final best cut " << final_best_cutname << " has value: " 
        << final_best_cutvalue << " with ZBi: " << final_best_ZBi << std::endl;
}

void ZBiCutflowProcessor::printZBiMatrix(){
    typedef std::map<std::string,std::vector<std::pair<double,double>>>::iterator iter;
    for(iter it = global_ZBi_map_.begin(); it != global_ZBi_map_.end(); it++){
        std::cout << '\n';
        std::cout << it->first << ": ";
        for(std::vector<std::pair<double,double>>::iterator vec_it = it->second.begin(); vec_it != it->second.end(); vec_it++){
            double cutvalue = vec_it->first;
            double zbi = vec_it->second;
            std::cout << " |" << cutvalue << "," << zbi << "| ";
        }
        std::cout << '\n';
    }
}

void ZBiCutflowProcessor::finalize() {
    std::cout << "[ZBiCutflowProcessor] finalize()" << std::endl;

    if(debug_){
        std::cout << "FINAL LIST OF PERSISTENT CUTS " << std::endl;
        persistentCutsSelector_->printCuts();
    }

    debugHistos_->saveHistos(outFile_);
    printZBiMatrix();
}

double ZBiCutflowProcessor::calculateZBi(double n_on, double n_off, double tau){
    double P_Bi = TMath::BetaIncomplete(1./(1.+tau),n_on,n_off+1);
    double Z_Bi = std::pow(2,0.5)*TMath::ErfInverse(1-2*P_Bi);
    return Z_Bi;
}

bool ZBiCutflowProcessor::failImpactParameterCut(std::map<std::string,double*> tuple){

    bool failImpactParamCut = false;

    //Get impact parameter cut corresponding to Test Cut
    std::vector<double> cut_params = impact_param_cut_;
    double zrecon = *tuple["unc_vtx_z"];
    //Do not apply cut to vertices that are reconstructed before 0 mm
    if(zrecon < 0.0)
        return false;

    std::map<std::string, double> track_z0;
    track_z0["ele"] = *tuple["unc_vtx_ele_track_z0"];
    track_z0["pos"] = *tuple["unc_vtx_pos_track_z0"];
    for(std::map<std::string,double>::iterator it=track_z0.begin(); it!=track_z0.end(); it++){
        double z0 = it->second;
        if(z0 > 0){
            if(z0 < cut_params[0] + cut_params[1]*zrecon){
                failImpactParamCut = true;
                break;
            }
        }
        else {
            if(z0 > cut_params[2] + cut_params[3]*zrecon){
                failImpactParamCut = true;
                break;
            }
        }
    }

    return failImpactParamCut;
}

bool ZBiCutflowProcessor::failPersistentCuts(std::map<std::string,double*> tuple){
    bool failCuts = false;
    for(cut_iter_ it=persistentCuts_->begin(); it!=persistentCuts_->end(); it++){
        std::string cutname = it->first;
        std::string cutvar = persistentCutsSelector_->getCutVar(cutname);
        if(!persistentCutsSelector_->passCutGTorLT(cutname, *tuple[cutvar])){ 
            failCuts = true;
            break;
        }
    }
    return failCuts;
}

void ZBiCutflowProcessor::calculateImpactParameterCut(){

    //Loop over signal events to calculate the impact parameter cut (z0 vs z)
    for(int e=0; e<signalTree_->GetEntries(); e++){
        signalTree_->GetEntry(e);

        //Define Mass Window centered on Vd mass
        if(*signal_tuple_["unc_vtx_mass"]*1000.0 > highMass_) continue;
        if(*signal_tuple_["unc_vtx_mass"]*1000.0 < lowMass_) continue;

        //Fill impact parameter 2d histogram...used to calculate impact parameter cut
        signalHistos_->Fill2DHisto("z0_v_recon_z_hh",*signal_tuple_["unc_vtx_z"],*signal_tuple_["unc_vtx_ele_track_z0"]);
        signalHistos_->Fill2DHisto("z0_v_recon_z_hh",*signal_tuple_["unc_vtx_z"],*signal_tuple_["unc_vtx_pos_track_z0"]);
    }
    std::cout << "Getting impact parameter cut" << std::endl;
    impact_param_cut_ = signalHistos_->impactParameterCut();
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
