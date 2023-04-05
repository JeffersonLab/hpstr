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
        ztail_nevents_ = parameters.getDouble("ztail_events",ztail_nevents_);
        zalpha_slope_ = parameters.getDouble("zalpha_slope",zalpha_slope_);
        scan_zcut_ = parameters.getInteger("scan_zcut",scan_zcut_);
        step_size_ = parameters.getDouble("step_size",step_size_);
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

    //initalize Test Cuts (which will be iteratively changed)
    testCutsSelector_ = new IterativeCutSelector("testCuts",cuts_cfgFile_);
    testCutsSelector_->LoadSelection();
    testCuts_ = testCutsSelector_->getPointerToCuts();

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

    //Initialize  vertex z distribution histograms corresponding to each cut. These are used to calculate
    //expected signal and number of background events for each cut. 
    std::cout << "Initializing z vertex distributions for selected signal and tritrig" << std::endl;
    cutHistos_ = new ZBiHistos("cutHistos");
    cutHistos_->debugMode(debug_);
    cutHistos_->defineCutlistHistos(*testCuts_);

    summaryHistos_ = new ZBiHistos("summary");
    summaryHistos_->defineIterHistos();

    //Get the Vd pretrigger simulated vertex z distribution
    getVdSimZ();

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

    //Initialize and read tuples from input files
    if(debug_) std::cout << "Initializing input tuple from signal file" << std::endl;
    TFile* signalFile = new TFile(signalFilename_.c_str(),"READ");
    TFile* tritrigFile = new TFile(tritrigFilename_.c_str(),"READ");
    signalMTT_ = new MutableTTree(signalFile, "vtxana_kf_Tight_2016_simp_reach_dev/vtxana_kf_Tight_2016_simp_reach_dev_tree");
    signalMTT_->defineMassWindow(lowMass_, highMass_);
    tritrigMTT_ = new MutableTTree(tritrigFile, "vtxana_kf_Tight_2016_simp_reach_dev/vtxana_kf_Tight_2016_simp_reach_dev_tree");
    tritrigMTT_->defineMassWindow(lowMass_, highMass_);

    //Add new variables here
    signalMTT_->addVariableZalpha(-0.102157, zalpha_slope_, 4.18);
    tritrigMTT_->addVariableZalpha(-0.102157, zalpha_slope_, 4.18);
    signalMTT_->Fill();
    tritrigMTT_->Fill();

    //Impact Parameter and Zalpha distributions
    //Signal
    for(int e=0; e < signalMTT_->GetEntries(); e++){
        signalMTT_->GetEntry(e);

        std::vector<std::string> variables = signalMTT_->getAllVariables();
        for(std::vector<std::string>::iterator it=variables.begin(); it != variables.end(); it++){
            std::string var = *it;
            if(signalHistos_->get1dHistos().count("signal_"+var+"_h")){
                signalHistos_->Fill1DHisto(var+"_h", signalMTT_->getValue(var));
            }
        }

        signalHistos_->Fill2DHisto("z0_v_recon_z_hh",signalMTT_->getValue("unc_vtx_z"),signalMTT_->getValue("unc_vtx_ele_track_z0"));
        signalHistos_->Fill2DHisto("z0_v_recon_z_hh",signalMTT_->getValue("unc_vtx_z"),signalMTT_->getValue("unc_vtx_pos_track_z0"));
        signalHistos_->Fill2DHisto("z0_v_recon_z_alpha_hh",signalMTT_->getValue("unc_vtx_ele_track_zalpha"), signalMTT_->getValue("unc_vtx_ele_track_z0"));
        signalHistos_->Fill2DHisto("z0_v_recon_z_alpha_hh",signalMTT_->getValue("unc_vtx_pos_track_zalpha"), signalMTT_->getValue("unc_vtx_pos_track_z0"));
    }

    //Impact Parameter and Zalpha distributions
    //Tritrig
    for(int e=0; e < tritrigMTT_->GetEntries(); e++){
        tritrigMTT_->GetEntry(e);

        tritrigHistos_->Fill2DHisto("z0_v_recon_z_hh",tritrigMTT_->getValue("unc_vtx_z"),tritrigMTT_->getValue("unc_vtx_ele_track_z0"));
        tritrigHistos_->Fill2DHisto("z0_v_recon_z_hh",tritrigMTT_->getValue("unc_vtx_z"),tritrigMTT_->getValue("unc_vtx_pos_track_z0"));
        tritrigHistos_->Fill2DHisto("z0_v_recon_z_alpha_hh",tritrigMTT_->getValue("unc_vtx_ele_track_zalpha"), tritrigMTT_->getValue("unc_vtx_ele_track_z0"));
        tritrigHistos_->Fill2DHisto("z0_v_recon_z_alpha_hh",tritrigMTT_->getValue("unc_vtx_pos_track_zalpha"), tritrigMTT_->getValue("unc_vtx_pos_track_z0"));
    }
    
    //Write init histos
    signalHistos_->writeHistos(outFile_,"");
    tritrigHistos_->writeHistos(outFile_,"");

    //Fill initial histograms of signal variables
    std::cout << "Getting initial signal histogram integrals to be used as reference for signal cutting" << std::endl;
    std::vector<std::string> variables = signalMTT_->getAllVariables();
    for(std::vector<std::string>::iterator it=variables.begin(); it != variables.end(); it++){
        std::string var = *it;
        initialIntegrals_[var] = signalHistos_->getIntegral("signal_"+var+"_h");
    }

    //Set the values for the initial set of 'persistentCuts_' based on the signal variable histograms cutting 0%
    for(cut_iter_ it=persistentCuts_->begin(); it!=persistentCuts_->end(); it++){
        std::string cutname = it->first;
        std::string cutvar = persistentCutsSelector_->getCutVar(cutname);
        //If the cut does not correspond to a variable in the signal tuple, skip the cut. 
        //The value of a cut fitting this criteria will be whatever it says in the cut config file
        if (std::find(variables.begin(), variables.end(), cutvar) == variables.end())
            continue;
        bool isCutGT = persistentCutsSelector_->isCutGreaterThan(cutname);
        double cutvalue = signalHistos_->cutFractionOfIntegral("signal_"+cutvar+"_h", isCutGT, 0.0, initialIntegrals_[cutvar]);
        persistentCutsSelector_->setCutValue(cutname, cutvalue);
        if(debug_) std::cout << "[Persistent Cuts] Cut "<< cutname << " updated to: " << persistentCutsSelector_->getCut(cutname) << std::endl;
    }
}

double ZBiCutflowProcessor::round(double var){
    float value = (int)(var * 100 + .5);
    return (double)value / 100;
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
    
    double cutFraction = step_size_;
    int max_iteration = (int)1.0/cutFraction;
    max_iteration = 25;

    for(int iteration = 1; iteration < max_iteration; iteration ++){
        double cutSignal = (double)iteration*step_size_*100.0;
        cutSignal = round(cutSignal);
        if(debug_) std::cout << "############### ITERATION " 
            << iteration << " #####################" << std::endl;

        //Reset histograms at the start of each iteration
        if(debug_) std::cout << "Resetting histograms" << std::endl;
        signalHistos_->resetHistograms1d();
        signalHistos_->resetHistograms2d();
        tritrigHistos_->resetHistograms1d();
        cutHistos_->resetHistograms1d();
        cutHistos_->resetHistograms2d();

        //SUMMARY PERSISTENT CUTS
        for(cut_iter_ it=persistentCuts_->begin(); it!=persistentCuts_->end(); it++){
            std::string cutname = it->first;
            double cutvalue = persistentCutsSelector_->getCut(cutname);
            int cutid = persistentCutsSelector_->getCutID(cutname);
            std::cout << "CUTNAME: " << cutname << std::endl;
            std::cout << "CUT ID: " << cutid << std::endl;
            summaryHistos_->Fill2DHisto("persistent_cuts_hh",(double)cutSignal, (double)cutid,cutvalue);
            summaryHistos_->set2DHistoYlabel("persistent_cuts_hh",cutid,cutname);
        }

        //Fill signal variable histograms after applying the persistent cuts
        for(int e=0;  e < signalMTT_->GetEntries(); e++){
            signalMTT_->GetEntry(e);

            //Apply persistent cuts
            if(failPersistentCuts(signalMTT_))
                continue;

            //Fill signal variable histograms
            std::vector<std::string> variables = signalMTT_->getAllVariables();
            for(std::vector<std::string>::iterator it=variables.begin(); it != variables.end(); it++){
                std::string var = *it;
                if(signalHistos_->get1dHistos().count("signal_"+var+"_h")){
                    signalHistos_->Fill1DHisto(var+"_h",signalMTT_->getValue(var));
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
        for(int e=0;  e < tritrigMTT_->GetEntries(); e++){
            tritrigMTT_->GetEntry(e);

            //Apply persistent cuts
            if(failPersistentCuts(tritrigMTT_))
                continue;

            //Before Impact parameter cut
            tritrigHistos_->Fill2DHisto("z0_v_recon_z_hh",tritrigMTT_->getValue("unc_vtx_z"),tritrigMTT_->getValue("unc_vtx_ele_track_z0"));
            tritrigHistos_->Fill2DHisto("z0_v_recon_z_hh",tritrigMTT_->getValue("unc_vtx_z"),tritrigMTT_->getValue("unc_vtx_pos_track_z0"));

            /*
        if(debug_) std::cout << "Calculating zcut for each cut" << std::endl;
            //Plot after impact parameter cut
            tritrigHistos_->Fill2DHisto("z0_v_recon_z_post_cut_hh",tritrigMTT_->getValue("unc_vtx_z"),tritrigMTT_->getValue("unc_vtx_ele_track_z0"));
            tritrigHistos_->Fill2DHisto("z0_v_recon_z_post_cut_hh",tritrigMTT_->getValue("unc_vtx_z"),tritrigMTT_->getValue("unc_vtx_pos_track_z0"));
            */

            //Fill tritrig variable histos before Test Cut and Zcut are applied
            std::vector<std::string> variables = tritrigMTT_->getAllVariables();
            for(std::vector<std::string>::iterator it=variables.begin(); it != variables.end(); it++){
                std::string var = *it;
                if(tritrigHistos_->get1dHistos().count("tritrig_"+var+"_h")){
                    tritrigHistos_->Fill1DHisto(var+"_h",tritrigMTT_->getValue(var));
                }
            }

            //Fill tritrig zvtx
            for(cut_iter_ it=testCuts_->begin(); it!=testCuts_->end(); it++){
               std::string cutname = it->first;
               std::string cutvar = testCutsSelector_->getCutVar(cutname);
               if(failTestCut(cutname, tritrigMTT_))
                   continue;
                //If event passes, fill zVtx distribution
                cutHistos_->Fill1DHisto("tritrig_zVtx_"+cutname+"_h",tritrigMTT_->getValue("unc_vtx_z"),mcScale_["tritrig"]);
                if(cutHistos_->get1dHistos().count("cutHistos_"+cutvar+"_h")){
                    cutHistos_->Fill1DHisto(cutvar+"_h",tritrigMTT_->getValue(cutvar));
                }
            }
        }

        


        //02282023 disabling this block of code because changing n_on_bkg = 0.5, n_off=0.5, tau = 1 by definition.
        /*
        //Initialize map to hold the background rate
        for(cut_iter_ it=testCuts_->begin(); it!=testCuts_->end(); it++){
            std::string cutname = it->first;
            scaled_backgrounds[cutname] = 0.0;
            n_offs[cutname] = 0.0;
        }

        //Count the background rate for each cut
        for(int e=0;  e < tritrigMTT_->GetEntries(); e++){
            tritrigMTT_->GetEntry(e);

            //Apply persistent cuts
            if(failPersistentCuts(tritrigMTT_))
                continue;

            //Apply Test Cuts (and corresponding Zcut) independently and count background 
            for(cut_iter_ it=testCuts_->begin(); it!=testCuts_->end(); it++){
                std::string cutname = it->first;
                std::string cutvar = testCutsSelector_->getCutVar(cutname);
                //Test Cut
                if(!testCutsSelector_->passCutGTorLT(cutname, tritrigMTT_->getValue(cutvar)))
                    continue;
                //Zcut for this Test Cut
                if(tritrigMTT_->getValue("unc_vtx_z") < zcuts[cutname]) continue;

                scaled_backgrounds[cutname] += mcScale_["tritrig"]; //May need to be 1/this
                n_offs[cutname] += 1.0;
            }
        }
        if(debug_){
            for(cut_iter_ it=testCuts_->begin(); it!=testCuts_->end(); it++){
                std::cout << "N Tritrig Events After applying test cut " << it->first << " " << 
                    testCutsSelector_->getCut(it->first) << ": " << n_offs[it->first] << std::endl;
            }

        }
        */
        //At this stage, for each Test Cut, the zcut value has been found. 
        //Using this zcut, we count the background events, measure the signal zvt selection 
        //efficiency, and then calculate the expected signal
        std::map<std::string,double> scaled_backgrounds;
        std::map<std::string,double> n_offs;

        //Make 2d hsitogram of signal true_z vs unc_z, so that zcuts can be easily applied
        for(int e=0;  e < signalMTT_->GetEntries(); e++){
            signalMTT_->GetEntry(e);

            //Apply persistent cuts
            if(failPersistentCuts(signalMTT_))
                continue;

            //Loop over Test Cuts and plot unc_vtx_z vs true_vtx_z
            for(cut_iter_ it=testCuts_->begin(); it!=testCuts_->end(); it++){
                std::string cutname = it->first;
                std::string cutvar = testCutsSelector_->getCutVar(cutname);
                double cutvalue = testCutsSelector_->getCut(cutname);
                //Test Cut
                if(!testCutsSelector_->passCutGTorLT(cutname, signalMTT_->getValue(cutvar)))
                    continue;
                cutHistos_->Fill2DHisto("unc_vtx_z_vs_true_vtx_z_"+cutname+"_hh",signalMTT_->getValue("unc_vtx_z"),signalMTT_->getValue("true_vtx_z"),1.0);
            }
        }

        /*
        //Find zcut value for each Test Cut by fitting the tail of the bkg zvtx distribution
        if(debug_) std::cout << "Calculating Zcut for each cut" << std::endl;
        std::map<std::string, double> zcuts;
        for(cut_iter_ it=testCuts_->begin(); it!=testCuts_->end(); it++){
            std::string cutname = it->first;
            //double zcut = cutHistos_->fitZTail("tritrig_zVtx_"+cutname+"_h",10.0); //<- TO DO: make
            double zcut;
            zcut = cutHistos_->shosFitZTail(cutname,ztail_nevents_); //<- TO DO: make
            zcuts[cutname] = zcut;
            if(debug_) std::cout << "Zcut for cut " << cutname << ": " << zcut << "[mm]" << std::endl;
        }
        */


        double best_zbi = -9999.9;
        std::string best_cutname;
        double best_cutvalue;
        //For each Test Cut, loop over all possible zcut values and calculate ZBi
        for(cut_iter_ it=testCuts_->begin(); it!=testCuts_->end(); it++){
            std::string cutname = it->first;
            double cutvalue = testCutsSelector_->getCut(cutname);
            int cutid = testCutsSelector_->getCutID(cutname);
            std::cout << "Calculating NSig for Test Cut " << cutname << std::endl;
            std::cout << "Test Cut ID: " << cutid << " | Test Cut Value: " << cutvalue << std::endl;

            //Make TGraph to track zbi vs zcut
            TGraph* zcutscan_zbi_g = new TGraph();
            zcutscan_zbi_g->SetName(("zcut_vs_zbi_"+cutname+"_g").c_str());
            zcutscan_zbi_g->SetTitle(("zcut_vs_zbi_"+cutname+"_g;zcut [mm];zbi").c_str());

            TGraph* zcutscan_nsig_g = new TGraph();
            zcutscan_nsig_g->SetName(("zcut_vs_nsig_"+cutname+"_g").c_str());
            zcutscan_nsig_g->SetTitle(("zcut_vs_nsig_"+cutname+"_g;zcut [mm];nsig").c_str());

            TGraph* zcutscan_nbkg_g = new TGraph();
            zcutscan_nbkg_g->SetName(("zcut_vs_nbkg_"+cutname+"_g").c_str());
            zcutscan_nsig_g->SetTitle(("zcut_vs_nbkg_"+cutname+"_g;zcut [mm];nbkg").c_str());

            TGraph* nbkg_zbi_g = new TGraph();
            nbkg_zbi_g->SetName(("nbkg_vs_zbi_"+cutname+"_g").c_str());
            nbkg_zbi_g->SetTitle(("nbkg_vs_zbi_"+cutname+"_g;nbkg;zbi").c_str());

            TGraph* nsig_zbi_g = new TGraph();
            nsig_zbi_g->SetName(("nsig_vs_zbi_"+cutname+"_g").c_str());
            nsig_zbi_g->SetTitle(("nsig_vs_zbi_"+cutname+"_g;nsig;zbi").c_str());

            TH2F* nsig_zcut_hh = new TH2F(("nsig_v_zcut_zbi_"+cutname+"_hh").c_str(),("nsig_v_zcut_zbi_"+cutname+"_hh; zcut [mm]; Nbkg").c_str(),200,-50.3,149.7,3000,0.0,300.0);

            TH2F* nbkg_zcut_hh = new TH2F(("nbkg_v_zcut_zbi_"+cutname+"_hh").c_str(),("nbkg_v_zcut_zbi_"+cutname+"_hh; zcut [mm]; Nbkg").c_str(),200,-50.3,149.7,3000,0.0,300.0);

            //Fit the zvtx background distribution to get the background model as function of z
            std::cout << "[ZBiCutflowProcessor]::Fitting ztail" << std::endl;
            //TF1* bkg_model = (TF1*)cutHistos_->getZTailFit(cutname); <- Fit not great for low MC stats cases
            TF1* bkg_model = (TF1*)cutHistos_->fitZTailWithExp(cutname); //Fit 10.0 < vtxz < 100.0 with exp 
            if(bkg_model == nullptr){
                continue;
            }

            //max zcut should be set to position beyond which only 0.5 background events exist in model
            //If background events = 0, ZBi calculation breaks.
            double max_zcut = -4.0;
            std::cout << "bkg_model xmax: " << bkg_model->GetXmax() << std::endl;
            double testIntegral = bkg_model->Integral(max_zcut, bkg_model->GetXmax());
            while(testIntegral > ztail_nevents_){
                max_zcut = max_zcut+0.1;
                testIntegral = bkg_model->Integral(max_zcut, bkg_model->GetXmax());
            }
            std::cout << "Maximum Zcut value: " << max_zcut << " gives NBackground = " << testIntegral << std::endl;

            //If we do not want to scan zcut values, set zcut scan window to a single value
            double min_zcut = -4.0;
            if(!scan_zcut_)
                min_zcut = max_zcut;
            std::cout << "Minimum Zcut value: " << min_zcut << std::endl;

            //Scan over range of zcut values, fit background model to determine Nbkg
            double best_scan_zbi = -999.9;
            double best_scan_zcut;
            double best_scan_nsig;
            double best_scan_nbkg;
            std::cout << "Looping over zcut values between: " << min_zcut << " to " << max_zcut << std::endl;
            for(double zcut = min_zcut; zcut < std::round(max_zcut+1.0); zcut = zcut+1.0){
                std::cout << "iterative zcut = " << zcut << std::endl;
                double Nbkg = bkg_model->Integral(zcut,bkg_model->GetXmax());
                std::cout << "Nbkg between " << zcut << " and " << bkg_model->GetXmax() << " = " << Nbkg << std::endl;

                //Get the 2D histogram of signal unc_vtx_z vs true_vtx_z for the corresponding Test Cut
                //Take the Y projection of unc_vtx_z < zcut to be the signal_vdSelZ
                TH2F* vtx_z_hh = (TH2F*)cutHistos_->get2dHisto("cutHistos_unc_vtx_z_vs_true_vtx_z_"+cutname+"_hh");
                std::cout << "Taking y projection of unc_v_true_vtx_z between " << vtx_z_hh->GetXaxis()->FindBin(zcut)+1<<                   " and " << vtx_z_hh->GetXaxis()->GetNbins() << std::endl;

                TH1F* true_vtx_z_h = (TH1F*)vtx_z_hh->ProjectionY((std::to_string(zcut)+"_"+cutname+"_"+"true_vtx_z_projy").c_str(),vtx_z_hh->GetXaxis()->FindBin(zcut)+1,vtx_z_hh->GetXaxis()->GetNbins(),"");

                //outFile_->cd();
                //TDirectory* dir{nullptr};
                //dir = outFile_->mkdir(("vdSelZ_"+std::to_string(iteration)).c_str(),"",true);
                //dir->cd();
                //true_vtx_z_h->Write();

                //Initialize the vd selection histogram by cloning the structure of vdSimZ
                TH1F* vdSelZ_h = (TH1F*)vdSimZ_h_->Clone(("cutHistos_signal_vdSelZ_"+cutname+"_h").c_str());
                for(int i=0; i<201; i++){
                    vdSelZ_h->SetBinContent(i,true_vtx_z_h->GetBinContent(i));
                }
                //take the efficiency of vdSelZ to vdSimZ to get F(z)
                TEfficiency* effCalc_h = new TEfficiency(*vdSelZ_h, *vdSimZ_h_);
                //effCalc_h->SetName((cutname+"eff_zcut_"+std::to_string(zcut)).c_str());
                //outFile_->cd();
                //dir = outFile_->mkdir(("effCalc_"+std::to_string(iteration)).c_str(),"",true);
                //dir->cd();
                //effCalc_h->Write();

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
                        double zz = vdSelZ_h->GetBinLowEdge(zbin);
                        if(zz < zcut) continue;
                        effVtx += (TMath::Exp((-4.3-zz)/gcTau)/gcTau)*(effCalc_h->GetEfficiency(zbin) - effCalc_h->GetEfficiencyErrorLow(zbin))*vdSelZ_h->GetBinWidth(zbin);
                    }

                    double tot_apProd = (3.*137/2.)*3.14159*(mAp_MeV*eps2*radFrac*dNdm)/radAcc;
                    double br_Vpi_val = simpEqs_->br_Vpi(mAp_MeV,m_pi,mV_MeV,alpha_D,f_pi,rho,phi);
                    double br_V_to_ee = 1.0;
                    Nsig = Nsig + tot_apProd*effVtx*br_V_to_ee*br_Vpi_val;
                }

                //CLEAR POINTERS
                delete effCalc_h;

                //Round Nsig, Nbkg, and then ZBi later
                Nsig = round(Nsig);
                Nbkg = round(Nbkg);

                //Calculate ZBi for this Test Cut using this zcut value
                double n_on = Nsig + Nbkg;
                double tau = 1.0;
                double n_off = Nbkg;
                double ZBi = calculateZBi(n_on, n_off, tau);
                ZBi = round(ZBi);

                std::cout << "Nsig = " << Nsig << std::endl;
                std::cout << "n_bkg: " << Nbkg << std::endl;
                std::cout << "n_on: " << n_on << std::endl;
                std::cout << "n_off: " << n_off << std::endl;
                std::cout << "ZBi: " << ZBi << std::endl;

                //Update Test Cut with best scan values
                if(ZBi > best_scan_zbi){
                    best_scan_zbi = ZBi;
                    best_scan_zcut = zcut;
                    best_scan_nsig = Nsig;
                    best_scan_nbkg = Nbkg;
                }
                
                //Fill TGraphs
                zcutscan_zbi_g->SetPoint(zcutscan_zbi_g->GetN(),zcut, ZBi);
                zcutscan_nbkg_g->SetPoint(zcutscan_nbkg_g->GetN(),zcut, Nbkg);
                zcutscan_nsig_g->SetPoint(zcutscan_nsig_g->GetN(),zcut, Nsig);
                nbkg_zbi_g->SetPoint(nbkg_zbi_g->GetN(),Nbkg, ZBi);
                nsig_zbi_g->SetPoint(nsig_zbi_g->GetN(),Nsig, ZBi);

                nsig_zcut_hh->Fill(zcut,Nsig,ZBi);
                nbkg_zcut_hh->Fill(zcut,Nbkg,ZBi);
            }

            //Write graph of zcut vs zbi for the Test Cut
            writeGraph(outFile_, "testCuts_pct_sig_cut_"+std::to_string(cutSignal), zcutscan_zbi_g);
            writeGraph(outFile_, "testCuts_pct_sig_cut_"+std::to_string(cutSignal), zcutscan_nbkg_g);
            writeGraph(outFile_, "testCuts_pct_sig_cut_"+std::to_string(cutSignal), zcutscan_nsig_g);

            //Save 2d histos
            outFile_->cd();
            TDirectory* dir{nullptr};
            dir = outFile_->mkdir(("testCuts_pct_sig_cut_"+std::to_string(cutSignal)).c_str(),"",true);
            dir->cd();
            nsig_zcut_hh->Write();
            nbkg_zcut_hh->Write();

            //delete TGraph pointers
            delete zcutscan_zbi_g;
            delete zcutscan_nsig_g;
            delete zcutscan_nbkg_g;
            delete nbkg_zbi_g;
            delete nsig_zbi_g;
            delete nsig_zcut_hh;
            delete nbkg_zcut_hh;

           //Fill Summary Histograms Test Cuts at best zcutscan value
           summaryHistos_->Fill2DHisto("test_cuts_values_hh",(double)cutSignal, (double)cutid,cutvalue);
           summaryHistos_->set2DHistoYlabel("test_cuts_values_hh",cutid,cutname);

           summaryHistos_->Fill2DHisto("test_cuts_ZBi_hh",(double)cutSignal, (double)cutid,best_scan_zbi);
           summaryHistos_->set2DHistoYlabel("test_cuts_ZBi_hh",cutid,cutname);

           summaryHistos_->Fill2DHisto("test_cuts_zcut_hh",(double)cutSignal, (double)cutid,best_scan_zcut);
           summaryHistos_->set2DHistoYlabel("test_cuts_zcut_hh",cutid,cutname);

           summaryHistos_->Fill2DHisto("test_cuts_nsig_hh",(double)cutSignal, (double)cutid,best_scan_nsig);
           summaryHistos_->set2DHistoYlabel("test_cuts_nsig_hh",cutid,cutname);

           summaryHistos_->Fill2DHisto("test_cuts_nbkg_hh",(double)cutSignal, (double)cutid,best_scan_nbkg);
           summaryHistos_->set2DHistoYlabel("test_cuts_nbkg_hh",cutid,cutname);

            //Check if the best cutscan zbi for this Test Cut is the best overall ZBi for all Test Cuts
            if(best_scan_zbi > best_zbi){
                best_zbi = best_scan_zbi;
                best_cutname = cutname;
                best_cutvalue = cutvalue;
            }
        } //END LOOP OVER TEST CUTS

        //Find the overall Best Test Cut for this iteration. Apply the new best cut to the list of
        //persistent cuts, so that it carries over to the next iteration
        summaryHistos_->Fill2DHisto("best_test_cut_ZBi_hh",(double)cutSignal, best_zbi, (double)testCutsSelector_->getCutID(best_cutname));

        if(debug_){
            std::cout << "Iteration " << iteration << " Best Test Cut is " << best_cutname << " " 
                << best_cutvalue << " with ZBi=" << best_zbi << std::endl;
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
        signalHistos_->writeHistos(outFile_,"signal_pct_sig_cut_"+std::to_string(cutSignal));
        tritrigHistos_->writeHistos(outFile_,"tritrig_pct_sig_cut_"+std::to_string(cutSignal));
        cutHistos_->writeHistos(outFile_, "testCuts_pct_sig_cut_"+std::to_string(cutSignal));
    }
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

    summaryHistos_->saveHistos(outFile_);
    cutHistos_->writeGraphs(outFile_,"");

    printZBiMatrix();
}

double ZBiCutflowProcessor::calculateZBi(double n_on, double n_off, double tau){
    double P_Bi = TMath::BetaIncomplete(1./(1.+tau),n_on,n_off+1);
    double Z_Bi = std::pow(2,0.5)*TMath::ErfInverse(1-2*P_Bi);
    return Z_Bi;
}

bool ZBiCutflowProcessor::failImpactParameterZalphaCut(double ele_track_z0, double pos_track_z0, double vtx_z, double zalpha_cut_lt){
    double ele_zalpha = transformImpactParameterZalpha(ele_track_z0, vtx_z);
    double pos_zalpha = transformImpactParameterZalpha(pos_track_z0, vtx_z);
    if(ele_zalpha > zalpha_cut_lt || pos_zalpha > zalpha_cut_lt)
        return true;
    else
        return false;
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

bool ZBiCutflowProcessor::failPersistentCuts(MutableTTree* MTT){
    bool failCuts = false;
    for(cut_iter_ it=persistentCuts_->begin(); it!=persistentCuts_->end(); it++){
        std::string cutname = it->first;
        std::string cutvar = persistentCutsSelector_->getCutVar(cutname);
        //If no value inside the tuple exists for this cut, do not apply the cut.
        if(!MTT->variableExists(cutvar))
            continue;
        if(!persistentCutsSelector_->passCutGTorLT(cutname, MTT->getValue(cutvar))){ 
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
    impact_param_cut_ = signalHistos_->impactParameterCut();
}

bool ZBiCutflowProcessor::failTestCut(std::string cutname, MutableTTree* MTT){

    std::string cutvar = testCutsSelector_->getCutVar(cutname);
    double cutvalue = testCutsSelector_->getCut(cutname);
    //If cut variable is not found in the list of tuples, do not apply cut
    if(!MTT->variableExists(cutvar))
        return false;
    if(!testCutsSelector_->passCutGTorLT(cutname, MTT->getValue(cutvar)))
        return true;
    else
        return false;
}

bool ZBiCutflowProcessor::failTestCut(std::string cutname, std::map<std::string,double*> tuple){

    std::string cutvar = testCutsSelector_->getCutVar(cutname);
    double cutvalue = testCutsSelector_->getCut(cutname);
    //If cut variable is not found in the list of tuples, do not apply cut
    if(tuple.find(cutvar) == tuple.end())
        return false;
    if(!testCutsSelector_->passCutGTorLT(cutname, *tuple[cutvar]))
        return true;
    else
        return false;
}

double ZBiCutflowProcessor::transformImpactParameterZalpha(double track_z0, double vtx_z){

    //Get the impact parameter cut fit parameters and transform
    double a_p = impact_param_cut_[0];
    double b_p = impact_param_cut_[1];
    double a_d = impact_param_cut_[2];
    double b_d = impact_param_cut_[3];
    double beta = impact_param_cut_[4];
    double z_alpha = impact_param_cut_[5];

    double new_track_z;

    if(track_z0 > 0)
        new_track_z = vtx_z - ( ((track_z0-a_p)/b_p) - z_alpha );
    else
        new_track_z = vtx_z - ( ((track_z0-a_d)/b_d) - z_alpha );

    return new_track_z;
}

void ZBiCutflowProcessor::getVdSimZ(){
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
}


void ZBiCutflowProcessor::writeGraph(TFile* outF, std::string folder, TGraph* g){
    if (outF) outF->cd();
    TDirectory* dir{nullptr};
    std::cout<<folder.c_str()<<std::endl;
    if (!folder.empty()) {
        dir = outF->mkdir(folder.c_str(),"",true);
        dir->cd();
    }
    g->Write();
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
