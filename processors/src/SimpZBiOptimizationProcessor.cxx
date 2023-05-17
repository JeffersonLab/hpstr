#include "SimpZBiOptimizationProcessor.h"
#include <string>
#include <cstdlib>
#include <iostream>
#include <fstream>

SimpZBiOptimizationProcessor::SimpZBiOptimizationProcessor(const std::string& name, Process& process) 
    : Processor(name,process) {
    std::cout << "[SimpZBiOptimizationProcessor] Constructor()" << std::endl;
}

SimpZBiOptimizationProcessor::~SimpZBiOptimizationProcessor(){}

void SimpZBiOptimizationProcessor::configure(const ParameterSet& parameters) {
    std::cout << "[SimpZBiOptimizationProcessor] configure()" << std::endl;
    try
    {
        //Basic config
        debug_ = parameters.getInteger("debug",debug_);
        cuts_cfgFile_ = parameters.getString("cuts_cfgFile",cuts_cfgFile_);
        outFileName_ = parameters.getString("outFileName",outFileName_);
        cutVariables_ = parameters.getVString("cutVariables", cutVariables_);
        min_ztail_events_ = parameters.getDouble("ztail_events",min_ztail_events_);
        scan_zcut_ = parameters.getInteger("scan_zcut",scan_zcut_);
        step_size_ = parameters.getDouble("step_size",step_size_);

        //Background
        bkgVtxAnaFilename_ = parameters.getString("bkgVtxAnaFilename", bkgVtxAnaFilename_);
        bkgVtxAnaTreename_ = parameters.getString("bkgVtxAnaTreename",bkgVtxAnaTreename_);
        background_sf_ = parameters.getDouble("background_sf",background_sf_);

        //MC Signal
        signalHistCfgFilename_ = 
            parameters.getString("signalHistCfgFilename",signalHistCfgFilename_);
        signalVtxAnaFilename_ = 
            parameters.getString("signalVtxAnaFilename", signalVtxAnaFilename_);
        signalVtxAnaTreename_ = 
            parameters.getString("signalVtxAnaTreename",signalVtxAnaTreename_);
        signalMCAnaFilename_ =
            parameters.getString("signalMCAnaFilename",signalMCAnaFilename_);
        signal_pdgid_ =
            parameters.getString("signal_pdgid", signal_pdgid_);
        signal_sf_ = parameters.getDouble("signal_sf", signal_sf_);
        signal_mass_ = parameters.getDouble("signal_mass",signal_mass_);
        massResolution_ = parameters.getDouble("massRes",massResolution_);

        //Expected Signal Calculation
        radFrac_ = parameters.getDouble("radFrac",radFrac_);
        radAcc_ = parameters.getDouble("simp_radAcc",radAcc_);
        dNdm_ = parameters.getDouble("dNdm",dNdm_);
        logEps2_ = parameters.getDouble("logEps2",logEps2_);

        // New Variables //
        new_variables_ = parameters.getVString("add_new_variables", new_variables_);
        new_variable_params_ = parameters.getVDouble("new_variable_params", new_variable_params_);

        //Dev
        testSpecialCut_ = parameters.getInteger("testSpecialCut", testSpecialCut_);

    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }
}

void SimpZBiOptimizationProcessor::addNewVariables(MutableTTree* MTT, std::string variable, double param){
    if(variable == "zalpha"){
        MTT->addVariableZalpha(param);
        std::cout << "[MutableTTree]::Adding New Variable addVariableZalpha with param " << param << std::endl;
    }
    if(variable == "zbravo"){
        MTT->addVariableZbravo();
        std::cout << "[MutableTTree]::Adding New Variable addVariablebravo" << std::endl;
    }
    if(variable == "zbravoalpha"){
        MTT->addVariableZbravoAlpha(param);
        std::cout << "[MutableTTree]::Adding New Variable addVariableZbravoAlpha with param " << param << std::endl;
    }
    if(variable == "zbravosum"){
        MTT->addVariableZbravosum();
        std::cout << "[MutableTTree]::Adding New Variable addVariableZbravosum" << std::endl;
    }
    if(variable == "zbravosumalpha"){
        MTT->addVariableZbravosumAlpha(param);
        std::cout << "[MutableTTree]::Adding New Variable addVariableZbravosumAlpha with param " << param << std::endl;
    }
}

void SimpZBiOptimizationProcessor::fillEventHistograms(std::shared_ptr<ZBiHistos> histos, MutableTTree* MTT){
    
    //histos->Fill histograms for each variable defined in tree
    std::vector<std::string> variables = MTT->getAllVariables();
    for(std::vector<std::string>::iterator it=variables.begin(); it != variables.end(); it++) {
        std::string var = *it;
        histos->Fill1DHisto(var+"_h", MTT->getValue(var));
    }

    //always fill
    if(MTT->variableExists("unc_vtx_z") && MTT->variableExists("unc_vtx_ele_track_z0") 
            && MTT->variableExists("unc_vtx_pos_track_z0")){
        histos->Fill2DHisto("z0_v_recon_z_hh",MTT->getValue("unc_vtx_z"),MTT->getValue("unc_vtx_ele_track_z0"));
        histos->Fill2DHisto("z0_v_recon_z_hh",MTT->getValue("unc_vtx_z"),MTT->getValue("unc_vtx_pos_track_z0"));
    }

    //Investigating new variables
    if(MTT->variableExists("unc_vtx_ele_track_zalpha") && MTT->variableExists("unc_vtx_pos_track_zalpha")){
        histos->Fill2DHisto("z0_v_recon_zalpha_hh",MTT->getValue("unc_vtx_ele_track_zalpha"),MTT->getValue("unc_vtx_ele_track_z0"));
        histos->Fill2DHisto("z0_v_recon_zalpha_hh",MTT->getValue("unc_vtx_pos_track_zalpha"),MTT->getValue("unc_vtx_pos_track_z0"));
    }
    if(MTT->variableExists("unc_vtx_ele_zbravoalpha") && MTT->variableExists("unc_vtx_pos_zbravoalpha")){
        histos->Fill2DHisto("zbravo_v_zbravoalpha_hh",MTT->getValue("unc_vtx_ele_zbravoalpha"),MTT->getValue("unc_vtx_ele_zbravo"));
        histos->Fill2DHisto("zbravo_v_zbravoalpha_hh",MTT->getValue("unc_vtx_pos_zbravoalpha"),MTT->getValue("unc_vtx_pos_zbravo"));
    }
    if(MTT->variableExists("unc_vtx_ele_zbravo") && MTT->variableExists("unc_vtx_pos_zbravo")){
        histos->Fill2DHisto("zbravo_v_recon_z_hh",MTT->getValue("unc_vtx_z"),MTT->getValue("unc_vtx_ele_zbravo"));
        histos->Fill2DHisto("zbravo_v_recon_z_hh",MTT->getValue("unc_vtx_z"),MTT->getValue("unc_vtx_pos_zbravo"));
    }
    if(MTT->variableExists("unc_vtx_zbravosum")){
        histos->Fill2DHisto("zbravosum_v_recon_z_hh",MTT->getValue("unc_vtx_z"),MTT->getValue("unc_vtx_zbravosum"));
        histos->Fill2DHisto("zbravosum_v_zbravosumalpha_hh",MTT->getValue("unc_vtx_zbravosumalpha"),MTT->getValue("unc_vtx_zbravosum"));
    }
}

void SimpZBiOptimizationProcessor::initialize(std::string inFilename, std::string outFilename){
    std::cout << "[SimpZBiOptimizationProcessor] Initialize" << std::endl;

    //Mass window
    lowMass_ = signal_mass_ - 2.0*massResolution_/2.0;
    highMass_ = signal_mass_ + 2.0*massResolution_/2.0;
    std::cout << "MASS WINDOW: " << lowMass_ << " - " << highMass_ << std::endl;

    //Initialize output file
    outFile_ = new TFile(outFileName_.c_str(),"RECREATE");

    //Get the signal pretrigger simulated vertex z distribution
    getSignalMCAnaVtxZ_h(signalMCAnaFilename_, signal_pdgid_); 

    //Read signal ana vertex tuple, and convert to mutable tuple
    TFile* signalVtxAnaFile = new TFile(signalVtxAnaFilename_.c_str(),"READ");
    signalMTT_ = new MutableTTree(signalVtxAnaFile, signalVtxAnaTreename_);
    signalMTT_->defineMassWindow(lowMass_, highMass_);

    //Read background ana vertex tuple, and convert to mutable tuple
    TFile* bkgVtxAnaFile = new TFile(bkgVtxAnaFilename_.c_str(),"READ");
    bkgMTT_ = new MutableTTree(bkgVtxAnaFile, bkgVtxAnaTreename_);
    bkgMTT_->defineMassWindow(lowMass_, highMass_);

    /*
    //Add new variables and set new variable parameters from config
    //New variables must have corresponding entries in MutableTTree.cxx
    for(std::vector<std::string>::iterator it = new_variables_.begin(); it != new_variables_.end(); it++){
        std::cout << "Adding New Variable " << *it << std::endl;
        int param_idx = std::distance(new_variables_.begin(), it);
        signalMTT_->addNewVariable(*it, new_variable_params_.at(param_idx));
        bkgMTT_->addNewVariable(*it, new_variable_params_.at(param_idx));
    }*/

    for(std::vector<std::string>::iterator it = new_variables_.begin(); it != new_variables_.end(); it++){
        int param_idx = std::distance(new_variables_.begin(), it);
        addNewVariables(signalMTT_, *it, new_variable_params_.at(param_idx));
        addNewVariables(bkgMTT_, *it, new_variable_params_.at(param_idx));
    }

    //Apply any variable shifts here
    std::cout << "Shifting background unc_vtx_ele_track_z0 by 0.1" << std::endl;
    bkgMTT_->shiftVariable("unc_vtx_ele_track_z0", 0.1);
    std::cout << "Shifting background unc_vtx_pos_track_z0 by 0.1" << std::endl;
    bkgMTT_->shiftVariable("unc_vtx_pos_track_z0", 0.1);
    
    //Initialize new trees with new variables, and variable shifts, applied
    signalMTT_->Fill();
    bkgMTT_->Fill();

    //initialize cut selector for 'Persistent Cuts' (which are applied to every event)
    persistentCutsSelector_ = new IterativeCutSelector("persistentCuts", cuts_cfgFile_);
    persistentCutsSelector_->LoadSelection();
    persistentCutsSelector_->filterCuts(cutVariables_);
    persistentCutsPtr_ = persistentCutsSelector_->getPointerToCuts();
    std::cout << "Persistent Cuts: " << std::endl;
    persistentCutsSelector_->printCuts();

    //initalize Test Cuts (which will be iteratively changed)
    testCutsSelector_ = new IterativeCutSelector("testCuts",cuts_cfgFile_);
    testCutsSelector_->LoadSelection();
    testCutsSelector_->filterCuts(cutVariables_);
    testCutsPtr_ = testCutsSelector_->getPointerToCuts();
    std::cout << "Test Cuts: " << std::endl;
    testCutsSelector_->printCuts();

    //Initialize signal histograms
    signalHistos_= std::make_shared<ZBiHistos>("signal");
    signalHistos_->debugMode(debug_);
    signalHistos_->loadHistoConfig(signalHistCfgFilename_);
    signalHistos_->DefineHistos();

    ////Initialize background histograms
    bkgHistos_ = std::make_shared<ZBiHistos>("background");
    bkgHistos_->debugMode(debug_);
    bkgHistos_->loadHistoConfig(signalHistCfgFilename_);
    bkgHistos_->DefineHistos();
    
    //Initialize Test Cut histograms
    testCutHistos_ = std::make_shared<ZBiHistos>("testCutHistos");
    testCutHistos_->debugMode(debug_);
    testCutHistos_->DefineHistos();
    //Add Test Cut analysis histograms 
    for(cut_iter_ it=testCutsPtr_->begin(); it!=testCutsPtr_->end(); it++){
        std::string name = it->first;
        //Used to select true z vertex distribution given a cut in unc_vtx_z
        testCutHistos_->addHisto2d("unc_vtx_z_vs_true_vtx_z_"+name+"_hh","unc z_{vtx} [mm]",
            1500, -50.0, 100.0,"true z_{vtx} [mm]",200,-50.3,149.7);
        //signalSelZ
        testCutHistos_->addHisto1d("signal_SelZ_"+name+"_h","true z_{vtx} [mm]",
                200, -50.3, 149.7);
        //tritrig zVtx
        testCutHistos_->addHisto1d("background_zVtx_"+name+"_h","unc z_{vtx} [mm]",
                150, -50.0, 100.0);
    }

    //Initialize processor histograms that summarize iterative results
    processorHistos_ = std::make_shared<ZBiHistos>("zbi_processor");
    processorHistos_->defineZBiCutflowProcessorHistograms();

    //Fill Initial Signal histograms
    std::cout << "Filling initial signal histograms" << std::endl;
    for(int e=0; e < signalMTT_->GetEntries(); e++){
        signalMTT_->GetEntry(e);
        if(testSpecialCut_){
            if(!signalMTT_->testImpactParameterCut())
                continue;
        }
        fillEventHistograms(signalHistos_, signalMTT_);
    }

    std::cout << "Filling initial background histograms" << std::endl;
    //Fill Initial Background Histograms
    for(int e=0; e < bkgMTT_->GetEntries(); e++){
        bkgMTT_->GetEntry(e);
        if(testSpecialCut_){
            if(!bkgMTT_->testImpactParameterCut())
                continue;
        }
        fillEventHistograms(bkgHistos_, bkgMTT_);
    }
    
    //Write initial variable histograms for signal and background
    std::cout << "Writing Initial Histograms" << std::endl;
    signalHistos_->writeHistos(outFile_,"initial_signal");
    bkgHistos_->writeHistos(outFile_,"initial_background");

    //Integrate each signal variable distribution
    //This value is the reference for cutting n% of signal in a given variable
    std::cout << "Integrate initial signal histograms" << std::endl;
    for(cut_iter_ it=testCutsPtr_->begin(); it!=testCutsPtr_->end(); it++){
        std::string cutname = it->first;
        std::cout << "Cutname: " << cutname << std::endl;
        std::string var = persistentCutsSelector_->getCutVar(cutname);
        std::cout << "Integrating signal histogram for " << var << std::endl;
        initialIntegrals_[var] = signalHistos_->integrateHistogram1D("signal_"+var+"_h");
    }

    //Set the values for the initial set of 'persistentCuts' based on the signal 
    //variable histograms cutting 0% of signal
    std::cout << "Initialize persistent cuts" << std::endl;
    for(cut_iter_ it=persistentCutsPtr_->begin(); it!=persistentCutsPtr_->end(); it++){
        std::string cutname = it->first;
        std::string cutvar = persistentCutsSelector_->getCutVar(cutname);
        bool isCutGT = persistentCutsSelector_->isCutGreaterThan(cutname);
        double cutvalue = signalHistos_->cutFractionOfSignalVariable(cutvar, isCutGT, 0.0, initialIntegrals_[cutvar]);
        persistentCutsSelector_->setCutValue(cutname, cutvalue);
        if(debug_) std::cout << "[Persistent Cuts] Cut "<< cutname << " updated to: " << 
            persistentCutsSelector_->getCut(cutname) << std::endl;
    }
}

double SimpZBiOptimizationProcessor::round(double var){
    float value = (int)(var * 100 + .5);
    return (double)value / 100;
}

bool SimpZBiOptimizationProcessor::process(){
    std::cout << "[SimpZBiOptimizationProcessor] process()" << std::endl;

    //Each iteration loops over the set of cuts being tested. The expected signal and background
    //rate is calculated given each cut independently. 
    //We calculate ZBi, the chosen metric of signal vs background, for each cut.
    //Within an iteration, the cut the gives the largest ZBi value is selected as the best_cut.
    //On the next iteration, the best_cut is applied to all events, then the process repeats.
    
    double cutFraction = step_size_;
    int max_iteration = (int)1.0/cutFraction;
    max_iteration = 5;

    //For each Test Cut variable, cut n% of signal in that variable, to determine the 
    //Test Cut value. 
    //Apply each Test Cut independently to the signal and background events. 
    //Model the resulting background for each Test Cut.
    //Calculate the background rate for each Test Cut background model, as a function of zcut. 
    //Calculate the expected signal for each Test Cut, as a function of zcut.
    //Calculate ZBi for each Test Cut, as a function of zcut, where ZBi = f(Nsig,Nbkg).
    //For a given Test Cut, choose the largest ZBi value as a function of zcut.
    //Compare the largest ZBi values of each Test Cut. 
    //The Test Cut with the overall largest ZBi value is chosen to be the single best 
    //cut that maximizes the signal significance. 
    //The best Test Cut is henceforth applied to all events, and the iterative process continues.

    for(int iteration = 0; iteration < max_iteration; iteration ++){
        double cutSignal = (double)iteration*step_size_*100.0;
        cutSignal = round(cutSignal);
        if(debug_) std::cout << "############### ITERATION " 
            << iteration << " #####################" << std::endl;

        //Reset histograms at the start of each iteration
        signalHistos_->resetHistograms1d();
        signalHistos_->resetHistograms2d();
        bkgHistos_->resetHistograms1d();
        bkgHistos_->resetHistograms2d();
        testCutHistos_->resetHistograms1d();
        testCutHistos_->resetHistograms2d();

        //At the start of each iteration, save the persistent cut values
        for(cut_iter_ it=persistentCutsPtr_->begin(); it!=persistentCutsPtr_->end(); it++){
            std::string cutname = it->first;
            double cutvalue = persistentCutsSelector_->getCut(cutname);
            int cutid = persistentCutsSelector_->getCutID(cutname);
            processorHistos_->Fill2DHisto("persistent_cuts_hh",(double)cutSignal, 
                    (double)cutid,cutvalue);
            processorHistos_->set2DHistoYlabel("persistent_cuts_hh",cutid,cutname);
        }

        //Fill signal variable distributions
        for(int e=0;  e < signalMTT_->GetEntries(); e++){
            signalMTT_->GetEntry(e);

            //Apply persistent cuts to all events
            if(failPersistentCuts(signalMTT_))
                continue;

            fillEventHistograms(signalHistos_, signalMTT_);
        }
        
        //For each Test Cut, find the signal distribution in that Test Cut variable.
        //Cut n% of the signal using that Test Cut variable
        for(cut_iter_ it=testCutsPtr_->begin(); it!=testCutsPtr_->end(); it++){
            std::string cutname = it->first;
            std::string cutvar = testCutsSelector_->getCutVar(cutname);
            
            bool isCutGT = testCutsSelector_->isCutGreaterThan(cutname);
            double cutvalue = signalHistos_->cutFractionOfSignalVariable(cutvar, isCutGT, 
                    iteration*cutFraction, initialIntegrals_[cutvar]);
            testCutsSelector_->setCutValue(cutname, cutvalue);
            if(debug_) std::cout << "[Test Cuts]: " << cutname << 
                " updated to value " << testCutsSelector_->getCut(cutname) << std::endl;
        }

        //Build Background Model for each Test Cut
        for(int e=0;  e < bkgMTT_->GetEntries(); e++){
            bkgMTT_->GetEntry(e);

            //Apply persistent cuts
            if(failPersistentCuts(bkgMTT_))
                continue;

            fillEventHistograms(bkgHistos_, bkgMTT_);

            for(cut_iter_ it=testCutsPtr_->begin(); it!=testCutsPtr_->end(); it++){
               std::string cutname = it->first;
               std::string cutvar = testCutsSelector_->getCutVar(cutname);

               //apply Test Cut
               if(failTestCut(cutname, bkgMTT_))
                   continue;

                //If event passes, fill zVtx distribution
                testCutHistos_->Fill1DHisto("background_zVtx_"+cutname+"_h",
                        bkgMTT_->getValue("unc_vtx_z"),1.0);
            }
        }

        //Knowing that we will eventually need to calculate the expected signal, and that 
        //the expected signal calculation requires both the 'true_vtx_z' distribution, and
        //the 'unc_vtx_z' distribution (which is used to apply the zcut), we simply bundle
        //these distributions together here for convenience.
        //By combining these two 1D distributions into a single 2D distribution, we can 
        //apply a zcut on 'unc_vtx_z', and return the corresponding 'true_vtx_z' distribution.
        for(int e=0;  e < signalMTT_->GetEntries(); e++){
            signalMTT_->GetEntry(e);

            //Apply persistent cuts
            if(failPersistentCuts(signalMTT_))
                continue;

            //Loop over each Test Cut and plot unc_vtx_z vs true_vtx_z
            for(cut_iter_ it=testCutsPtr_->begin(); it!=testCutsPtr_->end(); it++){
                std::string cutname = it->first;
                std::string cutvar = testCutsSelector_->getCutVar(cutname);
                double cutvalue = testCutsSelector_->getCut(cutname);
                //Apply Test Cut
                if(!testCutsSelector_->passCutGTorLT(cutname, signalMTT_->getValue(cutvar)))
                    continue;
                testCutHistos_->Fill2DHisto("unc_vtx_z_vs_true_vtx_z_"+cutname+"_hh",
                        signalMTT_->getValue("unc_vtx_z"),signalMTT_->getValue("true_vtx_z"),1.0);
            }
        }

        //Now we start calculating the signal significance, ZBi = f(Nsig,Nbkg), corresponding
        //to each Test Cut independently. 
        //Nsig and Nbkg both depend on the location of the Zcut, so we can scan over all
        //possible Zcut values, and calculate ZBi for each zcut value. 
        //The largest ZBi for a given Test Cut, across all Zcut values, is chosen as the 
        //best possible signal significance for that Test Cut.

        //Store and update values of the overall best Test Cut
        double best_zbi = -9999.9;
        std::string best_cutname;
        double best_cutvalue;

        //For each Test Cut, loop over all possible zcut values and calculate ZBi
        for(cut_iter_ it=testCutsPtr_->begin(); it!=testCutsPtr_->end(); it++){
            std::string cutname = it->first;
            double cutvalue = testCutsSelector_->getCut(cutname);
            int cutid = testCutsSelector_->getCutID(cutname);
            std::cout << "Calculating NSig for Test Cut " << cutname << std::endl;
            std::cout << "Test Cut ID: " << cutid << " | Test Cut Value: " 
                << cutvalue << std::endl;

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

            TH2F* nsig_zcut_hh = new TH2F(("nsig_v_zcut_zbi_"+cutname+"_hh").c_str(),
                    ("nsig_v_zcut_zbi_"+cutname+"_hh; zcut [mm]; Nbkg").c_str(),
                    200,-50.3,149.7,3000,0.0,300.0);

            TH2F* nbkg_zcut_hh = new TH2F(("nbkg_v_zcut_zbi_"+cutname+"_hh").c_str(),
                    ("nbkg_v_zcut_zbi_"+cutname+"_hh; zcut [mm]; Nbkg").c_str(),
                    200,-50.3,149.7,3000,0.0,300.0);

            //Fit the zvtx background distribution to get the background model as function of z
            std::cout << "[SimpZBiOptimizationProcessor]::Fitting ztail" << std::endl;
            
            TF1* bkg_model = (TF1*)testCutHistos_->fitZTailWithExp(cutname); 


            
            //If background events = 0, ZBi calculation breaks.
            double max_zcut = -4.0;
            double testIntegral = bkg_model->Integral(max_zcut, bkg_model->GetXmax());
            //Find maximum possible zcut position to test, based on some final number
            //of background events in the signal region 'min_ztail_events_'
            while(testIntegral > min_ztail_events_){
                max_zcut = max_zcut+0.1;
                testIntegral = bkg_model->Integral(max_zcut, bkg_model->GetXmax());
            }
            std::cout << "Maximum Zcut value: " << max_zcut << 
                " gives NBackground = " << testIntegral << std::endl;

            //If we do not want to scan zcut values, set zcut scan window to a single value
            double min_zcut = -4.0;
            if(!scan_zcut_)
                min_zcut = max_zcut;
            std::cout << "Minimum Zcut value: " << min_zcut << std::endl;

            outFile_->cd();
            //Get signal unc_vtx_z vs true_vtx_z for the corresponding Test Cut
            TH2F* vtx_z_hh = (TH2F*)testCutHistos_->get2dHisto("testCutHistos_unc_vtx_z_vs_true_vtx_z_"+cutname+"_hh");

            //Get the signal vtx z selection efficiency *before* zcut is applied
            TH1F* true_vtx_NoZ_h = (TH1F*)vtx_z_hh->ProjectionY((cutname+"_"+"true_vtx_z_projy").c_str(),1,vtx_z_hh->GetXaxis()->GetNbins(),"");
            TH1F* signalSelNoZ_h = 
                (TH1F*)signalSimZ_h_->Clone(("testCutHistos_signal_SelNoZ_"+cutname+"_h").c_str());
            for(int i=0; i<201; i++){
                signalSelNoZ_h->SetBinContent(i,true_vtx_NoZ_h->GetBinContent(i));
            }
            TEfficiency* Seff_e = new TEfficiency(*signalSelNoZ_h, *signalSimZ_h_);
            Seff_e->SetName("prompt_signal_efficiency");

            //Scan over range of zcut values, fit background model to determine Nbkg
            double best_scan_zbi = -999.9;
            double best_scan_zcut;
            double best_scan_nsig;
            double best_scan_nbkg;
            std::cout << "Looping over zcut values between: " << min_zcut << 
                " to " << max_zcut << std::endl;
            for(double zcut = min_zcut; zcut < std::round(max_zcut+1.0); zcut = zcut+1.0){
                double Nbkg = bkg_model->Integral(zcut,bkg_model->GetXmax());
                
                TH1F* true_vtx_z_h = (TH1F*)vtx_z_hh->ProjectionY((std::to_string(zcut)+"_"+cutname+"_"+"true_vtx_z_projy").c_str(),vtx_z_hh->GetXaxis()->FindBin(zcut)+1,vtx_z_hh->GetXaxis()->GetNbins(),"");

                //We need to take the signal selection efficiency, the binned ratio of 
                //selected signal in Z, to the simulated signal in Z. 
                //This requires that the binning matches, so we force them to match here.
                TH1F* signalSelZ_h = 
                    (TH1F*)signalSimZ_h_->Clone(("testCutHistos_signal_SelZ_"+cutname+"_h").c_str());
                for(int i=0; i<201; i++){
                    signalSelZ_h->SetBinContent(i,true_vtx_z_h->GetBinContent(i));
                }

                //The efficiency of signalSelZ to signalSimZ gives us F(z), which is used
                //in calculating the expected signal for both SIMPS and displaced A's
                TEfficiency* effCalc_h = new TEfficiency(*signalSelZ_h, *signalSimZ_h_);

                double Nsig = 0.0;
                double simpAp_mass_MeV = signal_mass_*(3/1.8);
                double m_pi = simpAp_mass_MeV/3.0;
                double alpha_D = 0.01;
                double m_l = 0.511;
                double f_pi = m_pi/(4.*M_PI);

                double eps2 = std::pow(10, logEps2_);
                double eps = std::sqrt(eps2);
                std::string mesons[2] = {"rho","phi"};
                for(int i =0; i < 2; i++){
                    bool rho = false;
                    bool phi = false;
                    if(mesons[i] == "rho") rho = true;
                    else phi = true;

                    double ctau = simpEqs_->getCtau(simpAp_mass_MeV,m_pi,signal_mass_,eps,alpha_D,f_pi,m_l,rho);
                    double E_V = 1.35; //GeV <-this absolutely needs to be fixed
                    double gcTau = ctau * simpEqs_->gamma(signal_mass_/1000.0,E_V);

                    double effVtx = 0.0;
                    for(int zbin =0; zbin < 201; zbin++){
                        double zz = signalSelZ_h->GetBinLowEdge(zbin);
                        if(zz < zcut) continue;
                        effVtx += (TMath::Exp((-4.3-zz)/gcTau)/gcTau)*
                            (effCalc_h->GetEfficiency(zbin) - 
                             effCalc_h->GetEfficiencyErrorLow(zbin))*
                            signalSelZ_h->GetBinWidth(zbin);
                    }

                    double tot_apProd = (3.*137/2.)*3.14159*(simpAp_mass_MeV*eps2*radFrac_*dNdm_)/
                        radAcc_;

                    double br_Vpi_val = 
                        simpEqs_->br_Vpi(simpAp_mass_MeV,m_pi,signal_mass_,alpha_D,f_pi,rho,phi);
                    
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

                std::cout << "Zcut = " << zcut << std::endl;
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
            writeGraph(outFile_, "testCutsPtr_pct_sig_cut_"+std::to_string(cutSignal), 
                    zcutscan_zbi_g);
            writeGraph(outFile_, "testCutsPtr_pct_sig_cut_"+std::to_string(cutSignal), 
                    zcutscan_nbkg_g);
            writeGraph(outFile_, "testCutsPtr_pct_sig_cut_"+std::to_string(cutSignal), 
                    zcutscan_nsig_g);

            //delete TGraph pointers
            delete zcutscan_zbi_g;
            delete zcutscan_nsig_g;
            delete zcutscan_nbkg_g;
            delete nbkg_zbi_g;
            delete nsig_zbi_g;
            delete nsig_zcut_hh;
            delete nbkg_zcut_hh;

           //Fill Summary Histograms Test Cuts at best zcutscan value
           processorHistos_->Fill2DHisto("test_cuts_values_hh",(double)cutSignal, (double)cutid,cutvalue);
           processorHistos_->set2DHistoYlabel("test_cuts_values_hh",cutid,cutname);

           processorHistos_->Fill2DHisto("test_cuts_ZBi_hh",(double)cutSignal, (double)cutid,best_scan_zbi);
           processorHistos_->set2DHistoYlabel("test_cuts_ZBi_hh",cutid,cutname);

           processorHistos_->Fill2DHisto("test_cuts_zcut_hh",(double)cutSignal, (double)cutid,best_scan_zcut);
           processorHistos_->set2DHistoYlabel("test_cuts_zcut_hh",cutid,cutname);

           processorHistos_->Fill2DHisto("test_cuts_nsig_hh",(double)cutSignal, (double)cutid,best_scan_nsig);
           processorHistos_->set2DHistoYlabel("test_cuts_nsig_hh",cutid,cutname);

           processorHistos_->Fill2DHisto("test_cuts_nbkg_hh",(double)cutSignal, (double)cutid,best_scan_nbkg);
           processorHistos_->set2DHistoYlabel("test_cuts_nbkg_hh",cutid,cutname);

            //Check if the best cutscan zbi for this Test Cut is the best overall ZBi for all Test Cuts
            if(best_scan_zbi > best_zbi){
                best_zbi = best_scan_zbi;
                best_cutname = cutname;
                best_cutvalue = cutvalue;
            }
        } //END LOOP OVER TEST CUTS

        //Find the overall Best Test Cut for this iteration. Apply the new best cut to the list of
        //persistent cuts, so that it carries over to the next iteration
        processorHistos_->Fill2DHisto("best_test_cut_ZBi_hh",(double)cutSignal, best_zbi, 
                (double)testCutsSelector_->getCutID(best_cutname));

        if(debug_){
            std::cout << "Iteration " << iteration << " Best Test Cut is " << best_cutname 
                << " "  << best_cutvalue << " with ZBi=" << best_zbi << std::endl;
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
        bkgHistos_->writeHistos(outFile_,"tritrig_pct_sig_cut_"+std::to_string(cutSignal));
        testCutHistos_->writeHistos(outFile_, "testCutsPtr_pct_sig_cut_"+std::to_string(cutSignal));
    }
}

void SimpZBiOptimizationProcessor::printZBiMatrix(){
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

void SimpZBiOptimizationProcessor::finalize() {
    std::cout << "[SimpZBiOptimizationProcessor] finalize()" << std::endl;

    if(debug_){
        std::cout << "FINAL LIST OF PERSISTENT CUTS " << std::endl;
        persistentCutsSelector_->printCuts();
    }

    processorHistos_->saveHistos(outFile_);
    testCutHistos_->writeGraphs(outFile_,"");

    printZBiMatrix();
}

double SimpZBiOptimizationProcessor::calculateZBi(double n_on, double n_off, double tau){
    double P_Bi = TMath::BetaIncomplete(1./(1.+tau),n_on,n_off+1);
    double Z_Bi = std::pow(2,0.5)*TMath::ErfInverse(1-2*P_Bi);
    return Z_Bi;
}

bool SimpZBiOptimizationProcessor::failPersistentCuts(MutableTTree* MTT){
    bool failCuts = false;
    for(cut_iter_ it=persistentCutsPtr_->begin(); it!=persistentCutsPtr_->end(); it++){
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

    //Testing 2016 impact parameter cut 04/10/2023
    //THIS SHOULD EVENTUALLY BE REMOVED
    if(testSpecialCut_){
        //if(!MTT->impactParameterCut2016Canonical(signal_mass_))
        //    failCuts = true;
        if(!MTT->testImpactParameterCut())
            failCuts = true;
    }

    return failCuts;
}

bool SimpZBiOptimizationProcessor::failTestCut(std::string cutname, MutableTTree* MTT){

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

bool SimpZBiOptimizationProcessor::failTestCut(std::string cutname, std::map<std::string,double*> tuple){

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

void SimpZBiOptimizationProcessor::getSignalMCAnaVtxZ_h(std::string signalMCAnaFilename, 
        std::string signal_pdgid){
    //Read pre-trigger Signal MCAna vertex z distribution
    //Use mcAnaSimZ_h entries to set appropriately binned content in signalSimZ_h
    signalSimZ_h_ = new TH1F("signal_SimZ_h_","signal_SimZ;true z_{vtx} [mm];events", 200, -50.3, 149.7);
    TFile* signalMCAnaFile = new TFile(signalMCAnaFilename_.c_str(), "READ");
    TH1F* mcAnaSimZ_h = (TH1F*)signalMCAnaFile->Get(("mcAna/mcAna_mc"+signal_pdgid+"Z_h").c_str()); //<-HARDCODE CHANGE 
    for(int i=0; i < 201; i++){
        signalSimZ_h_->SetBinContent(i,mcAnaSimZ_h->GetBinContent(i));
    }
    signalMCAnaFile->Close();
    delete signalMCAnaFile;
}


void SimpZBiOptimizationProcessor::writeGraph(TFile* outF, std::string folder, TGraph* g){
    if (outF) outF->cd();
    TDirectory* dir{nullptr};
    std::cout<<folder.c_str()<<std::endl;
    if (!folder.empty()) {
        dir = outF->mkdir(folder.c_str(),"",true);
        dir->cd();
    }
    g->Write();
}

DECLARE_PROCESSOR(SimpZBiOptimizationProcessor);
