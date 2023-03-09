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
    signalMTT_->addVariableZalpha(-0.102157, 0.0271352, 4.18);
    tritrigMTT_->addVariableZalpha(-0.102157, 0.0271352, 4.18);
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

        //SUMMARY PERSISTENT CUTS
        for(cut_iter_ it=persistentCuts_->begin(); it!=persistentCuts_->end(); it++){
            std::string cutname = it->first;
            double cutvalue = persistentCutsSelector_->getCut(cutname);
            int cutid = persistentCutsSelector_->getCutID(cutname);
            std::cout << "CUTNAME: " << cutname << std::endl;
            std::cout << "CUT ID: " << cutid << std::endl;
            summaryHistos_->Fill2DHisto("persistent_cuts_hh",(double)iteration, (double)cutid,cutvalue);
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
        if(debug_) std::cout << "Calculating zcut for each cut" << std::endl;
        for(int e=0;  e < tritrigMTT_->GetEntries(); e++){
            tritrigMTT_->GetEntry(e);

            //Apply persistent cuts
            if(failPersistentCuts(tritrigMTT_))
                continue;

            //Before Impact parameter cut
            tritrigHistos_->Fill2DHisto("z0_v_recon_z_hh",tritrigMTT_->getValue("unc_vtx_z"),tritrigMTT_->getValue("unc_vtx_ele_track_z0"));
            tritrigHistos_->Fill2DHisto("z0_v_recon_z_hh",tritrigMTT_->getValue("unc_vtx_z"),tritrigMTT_->getValue("unc_vtx_pos_track_z0"));

            //Plot after impact parameter cut
            tritrigHistos_->Fill2DHisto("z0_v_recon_z_post_cut_hh",tritrigMTT_->getValue("unc_vtx_z"),tritrigMTT_->getValue("unc_vtx_ele_track_z0"));
            tritrigHistos_->Fill2DHisto("z0_v_recon_z_post_cut_hh",tritrigMTT_->getValue("unc_vtx_z"),tritrigMTT_->getValue("unc_vtx_pos_track_z0"));

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

        //NEVERMIND!
        //Define the iteration of ztail_nevents by fitting the tritrig_zVtx with Gaus+Exp, then finding
        //the position in z that cuts cut_fraction of background in the tail. 
        //NEVERMIND!

        //No longer going to use this
        /*
        //If ztail_nevents is provided as one of the Test Cuts, update value of ztail_nevents_
        if(persistentCuts_->find("ztail_nevents") != persistentCuts_->end()){
            ztail_nevents_ = persistentCutsSelector_->getCut("ztail_nevents");
            testCutsSelector_->setCutValue("ztail_nevents",
        }
        */

        //Find zcut value for each Test Cut by fitting the tail of the bkg zvtx distribution
        if(debug_) std::cout << "Calculating Zcut for each cut" << std::endl;
        std::map<std::string, double> zcuts;
        for(cut_iter_ it=testCuts_->begin(); it!=testCuts_->end(); it++){
            std::string cutname = it->first;
            //double zcut = cutHistos_->fitZTail("tritrig_zVtx_"+cutname+"_h",10.0); //<- TO DO: make
            double zcut;
            zcut = cutHistos_->shosFitZTail(cutname,ztail_nevents_); //<- TO DO: make

            /*
            if(cutname == "ztail_nevents")
                zcut = cutHistos_->shosFitZTail(cutname,testCutsSelector_->getCut("ztail_nevents")); //<- TO DO: make
            else
                zcut = cutHistos_->shosFitZTail(cutname,ztail_nevents_); //<- TO DO: make
                */
            zcuts[cutname] = zcut;
            if(debug_) std::cout << "Zcut for cut " << cutname << ": " << zcut << "[mm]" << std::endl;
        }
        
        //At this stage, for each Test Cut, the zcut value has been found. 
        //Using this zcut, we count the background events, measure the signal zvt selection 
        //efficiency, and then calculate the expected signal
        std::map<std::string,double> scaled_backgrounds;
        std::map<std::string,double> n_offs;


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

        //Make signal selected vtxZ distribution for each cut...
        //these selections are used to calculated NSig for each cut
        for(int e=0;  e < signalMTT_->GetEntries(); e++){
            signalMTT_->GetEntry(e);

            //Apply persistent cuts
            if(failPersistentCuts(signalMTT_))
                continue;

            //Loop over Test Cuts
            for(cut_iter_ it=testCuts_->begin(); it!=testCuts_->end(); it++){
                std::string cutname = it->first;
                std::string cutvar = testCutsSelector_->getCutVar(cutname);
                double cutvalue = testCutsSelector_->getCut(cutname);
                //Test Cut
                if(!testCutsSelector_->passCutGTorLT(cutname, signalMTT_->getValue(cutvar)))
                    continue;
                //Zcut for this Test Cut
                if(signalMTT_->getValue("unc_vtx_z") < zcuts[cutname]) continue;

                //Build signal Vd Selection 
                cutHistos_->Fill1DHisto("signal_vdSelZ_"+cutname+"_h",signalMTT_->getValue("true_vtx_z"),1.0);
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
            double cutvalue = testCutsSelector_->getCut(cutname);
            int cutid = testCutsSelector_->getCutID(cutname);

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

           //Nsig_map[cutname] = Nsig;
           //double n_on = Nsig + scaled_backgrounds[cutname]; <--Testing changes to this
           //double tau = 1./mcScale_["tritrig"];  <-- Changing this to be 1 for now. So n_off is now the ztail fit
           //double n_off = n_offs[cutname]; <-- Testing changes to this definition

           Nsig_map[cutname] = Nsig;
           double n_on = Nsig + ztail_nevents_;
           double tau = 1.; 
           double n_off = ztail_nevents_; 
               
           //calculate ZBi value
           double ZBi = calculateZBi(n_on, n_off, tau);
           if(Nsig == 0.0)
               ZBi = -999.9;

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
           //for(int i=0; i < n_offs[cutname]; i++){
           //    debugHistos_->Fill1DHisto(cutname+"_ZBi_tritrigStats_h",(double)iteration,ZBi/n_offs[cutname]);
           //}

           for(int i=0; i < vdSelZ_h_clone->GetEntries(); i++){
               debugHistos_->Fill1DHisto(cutname+"_ZBi_vdStats_h",(double)iteration, ZBi/vdSelZ_h_clone->GetEntries());
           }
           debugHistos_->get1dHisto("debug_"+cutname+"_Zcut_h")->SetBinContent(iteration, zcuts[cutname]);
           //debugHistos_->get1dHisto("debug_"+cutname+"_Noff_unscaled_h")->SetBinContent(iteration,n_offs[cutname]);
           //debugHistos_->get1dHisto("debug_"+cutname+"_Nbkg_scaled_h")->SetBinContent(iteration,scaled_backgrounds[cutname]);
           debugHistos_->get1dHisto("debug_"+cutname+"_Nsig_h")->SetBinContent(iteration,Nsig);
           debugHistos_->get1dHisto("debug_"+cutname+"_Non_h")->SetBinContent(iteration,n_on);


           //Fill Summary Histograms for Test Cuts
           summaryHistos_->Fill2DHisto("test_cuts_values_hh",(double)iteration, (double)cutid,cutvalue);
           summaryHistos_->set2DHistoYlabel("test_cuts_values_hh",cutid,cutname);
           summaryHistos_->Fill2DHisto("test_cuts_ZBi_hh",(double)iteration, (double)cutid,ZBi);
           summaryHistos_->set2DHistoYlabel("test_cuts_ZBi_hh",cutid,cutname);

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

        summaryHistos_->Fill2DHisto("best_test_cut_ZBi_hh",(double)iteration, best_ZBi, (double)testCutsSelector_->getCutID(best_cutname));

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
        //debugHistos_->get1dHisto("debug_Best_cut_Noff_unscaled_h")->SetBinContent(iteration, n_offs[best_cutname]);
        //debugHistos_->get1dHisto("debug_Best_cut_Nbkg_scaled_h")->SetBinContent(iteration, scaled_backgrounds[best_cutname]);
        debugHistos_->get1dHisto("debug_Best_cut_Nsig_h")->SetBinContent(iteration, Nsig_map[best_cutname]);
        //debugHistos_->get1dHisto("debug_Best_cut_Non_h")->SetBinContent(iteration, Nsig_map[best_cutname]+scaled_backgrounds[best_cutname]);
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
