#include "ApOptimizationProcessor.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

ApOptimizationProcessor::ApOptimizationProcessor(const std::string& name, Process& process)
    : OptimizationProcessor(name, process) {
    std::cout << "[ApOptimizationProcessor] Constructor()" << std::endl;
}

ApOptimizationProcessor::~ApOptimizationProcessor() {}

void ApOptimizationProcessor::configure(const ParameterSet& parameters) {
    std::cout << "[ApOptimizationProcessor] configure()" << std::endl;
    try {
        // Basic config
        debug_ = parameters.getInteger("debug", debug_);
        year_ = parameters.getInteger("year", year_);
        cuts_cfgFile_ = parameters.getString("cuts_cfgFile", cuts_cfgFile_);
        outFileName_ = parameters.getString("outFileName", outFileName_);
        cutVariables_ = parameters.getVString("cutVariables", cutVariables_);

        // Background
        bkgVtxAnaFilename_ = parameters.getString("bkgVtxAnaFilename", bkgVtxAnaFilename_);
        bkgVtxAnaTreename_ = parameters.getString("bkgVtxAnaTreename", bkgVtxAnaTreename_);
        background_sf_ = parameters.getDouble("background_sf", background_sf_);

        // MC Signal
        variableHistCfgFilename_ = parameters.getString("variableHistCfgFilename", variableHistCfgFilename_);
        signalVtxAnaFilename_ = parameters.getString("signalVtxAnaFilename", signalVtxAnaFilename_);
        signalVtxAnaTreename_ = parameters.getString("signalVtxAnaTreename", signalVtxAnaTreename_);
        signalMCAnaFilename_ = parameters.getString("signalMCAnaFilename", signalMCAnaFilename_);
        signal_pdgid_ = parameters.getString("signal_pdgid", signal_pdgid_);
        signal_sf_ = parameters.getDouble("signal_sf", signal_sf_);
        signal_mass_ = parameters.getDouble("signal_mass", signal_mass_);
        mass_window_nsigma_ = parameters.getDouble("mass_window_nsigma", mass_window_nsigma_);
        signalVtxMCSelection_ = parameters.getString("signalVtxMCSelection", signalVtxMCSelection_);

        // New Variables
        new_variables_ = parameters.getVString("add_new_variables", new_variables_);
        new_variable_params_ = parameters.getVDouble("new_variable_params", new_variable_params_);

        // Optimization config
        max_iteration_ = parameters.getInteger("max_iteration", max_iteration_);
        step_size_ = parameters.getDouble("step_size", step_size_);
        min_ztail_events_ = parameters.getDouble("ztail_events", min_ztail_events_);
        scan_zcut_ = parameters.getInteger("scan_zcut", scan_zcut_);

        // Expected Signal Calculation
        radFrac_ = parameters.getDouble("radFrac", radFrac_);
        eq_cfgFile_ = parameters.getString("eq_cfgFile", eq_cfgFile_);
    } catch (std::runtime_error& error) {
        std::cout << error.what() << std::endl;
    }
}

// USE JSON FILE TO LOAD IN NEW VARIABLES AND VARIABLE CONFIGURATIONS
void ApOptimizationProcessor::addNewVariables(TTree* tree, std::string variable, double param) {
    std::cout << "[ApOptimizationProcessor]::addNewVariable " << variable << " with param " << param << std::endl;

    tree->Branch(variable.c_str(), &param, (variable + "/D").c_str());
}

void ApOptimizationProcessor::fillEventHistograms(std::shared_ptr<ZBiHistos> histos, TTree* tree) {
    // need to fill the following histograms
    // z0_v_recon_z_hh, vtx_InvM_vtx_z_hh, ele_track_z0_v_pos_track_z0_hh, z0_v_unc_vtx_zalpha_hh,
    // recon_z_v_unc_vtx_zalpha_max_hh, unc_vtx_proj_x_v_unc_vtx_proj_y_hh, unc_vtx_proj_x_y_significance_hh,
    // recon_z_v_proj_sig_hh, recon_z_v_cxx_hh, recon_z_v_cyy_hh, recon_z_v_czz_hh, recon_z_v_czx_hh, recon_z_v_czy_hh,
    // recon_z_v_cyx_hh, recon_z_v_z0tanlambda_hh, recon_z_v_unc_vtx_deltaZ_hh, recon_z_v_Z0err_hh,
    // recon_z_v_track_t_hh, recon_z_v_ele_track_d0_hh, recon_z_v_pos_track_d0_hh, recon_z_v_ele_track_phi0_hh,
    // recon_z_v_pos_track_phi0_hh, recon_z_v_ele_track_px_hh, recon_z_v_pos_track_px_hh, recon_z_v_ele_track_py_hh,
    // recon_z_v_pos_track_py_hh, recon_z_v_ele_track_pz_hh, recon_z_v_pos_track_pz_hh, recon_z_v_ele_track_nhits_hh,
    // recon_z_v_pos_track_nhits_hh, ele_tanlambda_vs_phi0_hh, pos_tanlambda_vs_phi0_hh,
    // ele_cluster_energy_v_track_p_hh, pos_cluster_energy_v_track_p_hh, ele_z0_vs_tanlambda_hh, pos_z0_vs_tanlambda_hh
}

void ApOptimizationProcessor::initialize(std::string inFilename, std::string outFilename) {
    std::cout << "[ApOptimizationProcessor] Initialize " << inFilename << std::endl;

    // TODO: write equation to find mass resolution at signal mass
    massResolution_ = 1.0;

    // Define Mass window
    lowMass_ = signal_mass_ - mass_window_nsigma_ * massResolution_ / 2.0;
    highMass_ = signal_mass_ + mass_window_nsigma_ * massResolution_ / 2.0;
    std::cout << "[SimpZBiOptimization]::Mass Window: " << lowMass_ << " - " << highMass_ << std::endl;

    // Initialize output file
    std::cout << "[SimpZBiOptimization]::Output File: " << outFileName_.c_str() << std::endl;
    outFile_ = new TFile(outFileName_.c_str(), "RECREATE");

    // TODO: Get the signal pretrigger simulated vertex z distribution

    // Read signal ana vertex tuple, and convert to mutable tuple
    std::cout << "[SimpZBiOptimization]::Reading Signal AnaVertex Tuple from file " << signalVtxAnaFilename_.c_str()
              << std::endl;
    TFile* signalVtxAnaFile = new TFile(signalVtxAnaFilename_.c_str(), "READ");
    signal_tree_ = (TTree*)signalVtxAnaFile->Get(signalVtxAnaTreename_.c_str());

    // Read background ana vertex tuple, and convert to mutable tuple
    std::cout << "[SimpZBiOptimization]::Reading Background AnaVertex Tuple from file " << signalVtxAnaFilename_.c_str()
              << std::endl;
    TFile* bkgVtxAnaFile = new TFile(bkgVtxAnaFilename_.c_str(), "READ");
    bkg_tree_ = (TTree*)bkgVtxAnaFile->Get(bkgVtxAnaTreename_.c_str());

    // Add new variables from the processor configuration script
    // TODO: change this such that input trees have all needed variables
    for (std::vector<std::string>::iterator it = new_variables_.begin(); it != new_variables_.end(); it++) {
        int param_idx = std::distance(new_variables_.begin(), it);
        std::cout << "[SimpZBiOptimization]::Attempting to add new variable " << *it << " with parameter "
                  << new_variable_params_.at(param_idx) << std::endl;
        addNewVariables(signal_tree_, *it, new_variable_params_.at(param_idx));
        addNewVariables(bkg_tree_, *it, new_variable_params_.at(param_idx));
    }

    // Initialize Persistent Cut Selector. These cuts are applied to all events.
    // Persistent Cut values are updated each iteration with the value of the best performing Test Cut in
    // that iteration.
    std::cout << "[SimpZBiOptimization]::Initializing Set of Persistent Cuts" << std::endl;
    persistentCutsSelector_ = new IterativeCutSelector("persistentCuts", cuts_cfgFile_);
    persistentCutsSelector_->LoadSelection();
    persistentCutsPtr_ = persistentCutsSelector_->getPointerToCuts();
    std::cout << "Persistent Cuts: " << std::endl;
    persistentCutsSelector_->printCuts();

    // initalize Test Cuts
    std::cout << "[SimpZBiOptimization]::Initializing Set of Test Cuts" << std::endl;
    testCutsSelector_ = new IterativeCutSelector("testCuts", cuts_cfgFile_);
    testCutsSelector_->LoadSelection();
    testCutsPtr_ = testCutsSelector_->getPointerToCuts();
    testCutsSelector_->filterCuts(cutVariables_);
    std::cout << "Test Cuts: " << std::endl;
    testCutsSelector_->printCuts();

    // Initialize signal histograms
    std::cout << "[SimpZBiOptimization]::Initializing Signal Variable Histograms" << std::endl;
    signalHistos_ = std::make_shared<ZBiHistos>("signal");
    signalHistos_->debugMode(debug_);
    signalHistos_->loadHistoConfig(variableHistCfgFilename_);
    signalHistos_->DefineHistos();

    ////Initialize background histograms
    std::cout << "[SimpZBiOptimization]::Initializing Background Variable Histograms" << std::endl;
    bkgHistos_ = std::make_shared<ZBiHistos>("background");
    bkgHistos_->debugMode(debug_);
    bkgHistos_->loadHistoConfig(variableHistCfgFilename_);
    bkgHistos_->DefineHistos();

    // Initialize Test Cut histograms
    std::cout << "[SimpZBiOptimization]::Initializing Test Cut Variable Histograms" << std::endl;
    testCutHistos_ = std::make_shared<ZBiHistos>("testCutHistos");
    testCutHistos_->debugMode(debug_);
    testCutHistos_->DefineHistos();

    // Add Test Cut Analysis Histograms necessary for calculating background and signal
    std::cout << "[SimpZBiOptimization]::Initializing Test Cut Analysis Histograms" << std::endl;
    for (cut_iter_ it = testCutsPtr_->begin(); it != testCutsPtr_->end(); it++) {
        std::string name = it->first;
        // Used to select true z vertex distribution given a cut in unc_vtx_z
        testCutHistos_->addHisto2d("unc_vtx_z_vs_true_vtx_z_" + name + "_hh", "unc z_{vtx} [mm]", 1500, -50.0, 100.0,
                                   "true z_{vtx} [mm]", 200, -50.3, 149.7);
        // signalSelZ models signal selection efficiency
        testCutHistos_->addHisto1d("signal_SelZ_" + name + "_h", "true z_{vtx} [mm]", 200, -50.3, 149.7);
        // background_zVtx provides basis for background model, used to estimate nbackground in signal region
        testCutHistos_->addHisto1d("background_zVtx_" + name + "_h", "unc z_{vtx} [mm]", 150, -50.0, 100.0);
    }

    // Initialize processor histograms that summarize iterative results
    std::cout << "[SimpZBiOptimization]::Initializing Itearitve Result Histograms" << std::endl;
    processorHistos_ = std::make_shared<ZBiHistos>("zbi_processor");
    processorHistos_->defineZBiCutflowProcessorHistograms();

    // Fill Initial Signal histograms
    std::cout << "[SimpZBiOptimization]::Filling initial signal histograms" << std::endl;
    for (int e = 0; e < signal_tree_->GetEntries(); e++) {
        signal_tree_->GetEntry(e);
        fillEventHistograms(signalHistos_, signal_tree_);
    }

    std::cout << "[SimpZBiOptimization]::Filling initial background histograms" << std::endl;
    // Fill Initial Background Histograms
    for (int e = 0; e < bkg_tree_->GetEntries(); e++) {
        bkg_tree_->GetEntry(e);
        fillEventHistograms(bkgHistos_, bkg_tree_);
    }

    // Count background rate in the Control Region (used to calculate total A' Rate)

    // Write initial variable histograms for signal and background
    std::cout << "[SimpZBiOptimization]::Writing Initial Histograms" << std::endl;
    signalHistos_->writeHistos(outFile_, "initial_signal");
    bkgHistos_->writeHistos(outFile_, "initial_background");
}

bool ApOptimizationProcessor::process() {
    std::cout << "[ApOptimizationProcessor]::process()" << std::endl;

    // step_size defines n% of signal distribution to cut in a given variable
    std::cout << "step_size_ " << step_size_ << std::endl;
    double cutFraction = step_size_;
    if (max_iteration_ > (int)1.0 / cutFraction) max_iteration_ = (int)1.0 / cutFraction;

    std::cout << "max iteration: " << max_iteration_ << std::endl;
    // Iteratively cut n% of the signal distribution for a given Test Cut variable
    for (int iteration = 0; iteration < max_iteration_ + 1; iteration++) {
        double cutSignal = (double)iteration * step_size_ * 100.0;
        cutSignal = round(cutSignal);
        if (debug_) std::cout << "## ITERATION " << iteration << " ##" << std::endl;

        // Reset histograms at the start of each iteration
        // Histograms will change with each iteration.
        if (debug_) std::cout << "[SimpZBiOptimization]::Resetting Histograms" << std::endl;
        signalHistos_->resetHistograms1d();
        signalHistos_->resetHistograms2d();
        bkgHistos_->resetHistograms1d();
        bkgHistos_->resetHistograms2d();
        testCutHistos_->resetHistograms1d();
        testCutHistos_->resetHistograms2d();

        // At the start of each iteration, save the persistent cut values
        for (cut_iter_ it = persistentCutsPtr_->begin(); it != persistentCutsPtr_->end(); it++) {
            std::string cutname = it->first;
            double cutvalue = persistentCutsSelector_->getCut(cutname);
            if (debug_) {
                std::cout << "Saving persistent cut: " << cutname << " " << cutvalue << std::endl;
            }
            int cutid = persistentCutsSelector_->getCutID(cutname);
            processorHistos_->Fill2DHisto("persistent_cuts_hh", (double)cutSignal, (double)cutid, cutvalue);
            processorHistos_->set2DHistoYlabel("persistent_cuts_hh", cutid, cutname);
        }

        // Fill signal variable distributions
        for (int e = 0; e < signal_tree_->GetEntries(); e++) {
            signal_tree_->GetEntry(e);

            // Apply current set of persistent cuts to all events
            if (failPersistentCuts(signal_tree_)) continue;

            // Fill Signal variable distributions
            fillEventHistograms(signalHistos_, signal_tree_);
        }
        if (iteration == max_iteration_) {
            // Write iteration histos
            signalHistos_->writeHistos(outFile_, "signal_pct_sig_cut_" + std::to_string(cutSignal));
            bkgHistos_->writeHistos(outFile_, "background_pct_sig_cut_" + std::to_string(cutSignal));
            break;
        }

        // Initialize Signal Integrals
        if (iteration == 0) {
            // Integrate each signal variable distribution.
            // When iterating Test Cuts, we reference these intial integrals, cutting n% of the original distribution.
            std::cout << "[SimpZBiOptimization]::Integrating initial Signal distributions" << std::endl;
            for (cut_iter_ it = testCutsPtr_->begin(); it != testCutsPtr_->end(); it++) {
                std::string cutname = it->first;
                std::string var = testCutsSelector_->getCutVar(cutname);
                initialIntegrals_[var] = signalHistos_->integrateHistogram1D("signal_" + var + "_h");
                if (debug_)
                    std::cout << "Initial Integral for " << var << " is " << initialIntegrals_[var] << std::endl;
            }
        }

        // Loop over each Test Cut. Cut n% of signal distribution in Test Cut variable
        if (debug_) std::cout << "Looping over Signal Test Cuts" << std::endl;
        for (cut_iter_ it = testCutsPtr_->begin(); it != testCutsPtr_->end(); it++) {
            std::string cutname = it->first;
            std::string cutvar = testCutsSelector_->getCutVar(cutname);
            bool isCutGT = testCutsSelector_->isCutGreaterThan(cutname);
            double cutvalue = signalHistos_->cutFractionOfSignalVariable(cutvar, isCutGT, iteration * cutFraction,
                                                                         initialIntegrals_[cutvar]);
            testCutsSelector_->setCutValue(cutname, cutvalue);
            if (debug_) {
                std::cout << "Test Cut " << cutname << " " << testCutsSelector_->getCut(cutname) << " cuts "
                          << cutSignal << "% of signal distribution in this variable " << std::endl;
            }
        }

        // Fill Background Histograms corresponding to each Test Cut
        if (debug_) std::cout << "Filling Background Variables for each Test Cut" << std::endl;
        double unc_vtx_z_bkg;
        bkg_tree_->SetBranchAddress("unc_vtx_z", &unc_vtx_z_bkg);
        for (int e = 0; e < bkg_tree_->GetEntries(); e++) {
            bkg_tree_->GetEntry(e);

            // Apply persistent cuts
            if (failPersistentCuts(bkg_tree_)) continue;

            fillEventHistograms(bkgHistos_, bkg_tree_);

            // Loop over each Test Cut
            for (cut_iter_ it = testCutsPtr_->begin(); it != testCutsPtr_->end(); it++) {
                std::string cutname = it->first;
                std::string cutvar = testCutsSelector_->getCutVar(cutname);

                // apply Test Cut
                if (failTestCut(cutname, bkg_tree_)) continue;

                // If event passes Test Cut, fill vertex z distribution.
                // This distribution is used to build the Background Model corresponding to each Test Cut
                testCutHistos_->Fill1DHisto("background_zVtx_" + cutname + "_h", unc_vtx_z_bkg, background_sf_);
            }
        }

        // For each Test Cut, build relationship between Signal truth z_vtx, and reconstructed z_vtx
        // This is used to get the truth Signal Selection Efficiency F(z), given a Zcut in reconstructed z_vtx
        if (debug_) std::cout << "Build Signal truth z vs recon z" << std::endl;
        double unc_vtx_z_sig, true_vtx_z_sig;
        signal_tree_->SetBranchAddress("unc_vtx_z", &unc_vtx_z_sig);
        signal_tree_->SetBranchAddress("true_vtx_z", &true_vtx_z_sig);
        for (int e = 0; e < signal_tree_->GetEntries(); e++) {
            signal_tree_->GetEntry(e);

            // Apply persistent cuts
            if (failPersistentCuts(signal_tree_)) continue;

            // Loop over each Test Cut and plot unc_vtx_z vs true_vtx_z
            for (cut_iter_ it = testCutsPtr_->begin(); it != testCutsPtr_->end(); it++) {
                std::string cutname = it->first;
                std::string cutvar = testCutsSelector_->getCutVar(cutname);

                if (failTestCut(cutname, signal_tree_)) continue;

                testCutHistos_->Fill2DHisto("unc_vtx_z_vs_true_vtx_z_" + cutname + "_hh", unc_vtx_z_sig, true_vtx_z_sig,
                                            1.0);
            }
        }

        // Calcuate the Binomial Significance ZBi corresponding to each Test Cut
        // Test Cut with maximum ZBi after cutting n% of signal distribution in a given variable is selected
        // Test Cut value is added to list of Persistent Cuts, and is applied to all events in following iterations.

        // Create Directory for this iteration
        std::string iteration_subdir = "testCuts_pct_sig_cut_" + std::to_string(cutSignal);
        TDirectory* dir{nullptr};
        std::cout << iteration_subdir.c_str() << std::endl;
        if (!iteration_subdir.empty()) {
            dir = outFile_->mkdir(iteration_subdir.c_str(), "", true);
        }
        double best_zbi = -9999.9;
        double best_zcut = -9999.9;
        double best_nsig = -9999.9;
        double best_nbkg = -9999.9;
        std::string best_cutname;
        double best_cutvalue;

        // Loop over Test Cuts
        if (debug_) std::cout << "Calculate ZBi for each Test Cut " << std::endl;
        for (cut_iter_ it = testCutsPtr_->begin(); it != testCutsPtr_->end(); it++) {
            std::string cutname = it->first;
            double cutvalue = testCutsSelector_->getCut(cutname);
            int cutid = testCutsSelector_->getCutID(cutname);
            if (debug_) {
                std::cout << "Calculating ZBi for Test Cut " << cutname << std::endl;
                std::cout << "Test Cut ID: " << cutid << " | Test Cut Value: " << cutvalue << std::endl;
            }

            // Build Background Model, used to estimate nbkg in Signal Region
            if (debug_) std::cout << "Build Background Model" << std::endl;
            double start_fit = 500.0 * background_sf_;
            TF1* bkg_model = (TF1*)testCutHistos_->fitExponentialTail("background_zVtx_" + cutname, start_fit);
            // TF1* bkg_model = (TF1*)testCutHistos_->fitExponentialPlusConst("background_zVtx_"+cutname, start_fit);
            if (debug_) std::cout << "END Build Background Model" << std::endl;

            // Get signal unc_vtx_z vs true_vtx_z
            TH2F* vtx_z_hh =
                (TH2F*)testCutHistos_->get2dHisto("testCutHistos_unc_vtx_z_vs_true_vtx_z_" + cutname + "_hh");

            // CD to output file to save resulting plots
            outFile_->cd();

            // Graphs track the performance of a Test Cut as a function of Zcut position
            TGraph* zcutscan_zbi_g = new TGraph();
            zcutscan_zbi_g->SetName(("zcut_vs_zbi_" + cutname + "_g").c_str());
            zcutscan_zbi_g->SetTitle(("zcut_vs_zbi_" + cutname + "_g;zcut [mm];zbi").c_str());
            zcutscan_zbi_g->SetMarkerStyle(8);
            zcutscan_zbi_g->SetMarkerSize(2.0);
            zcutscan_zbi_g->SetMarkerColor(2);

            TGraph* zcutscan_nsig_g = new TGraph();
            zcutscan_nsig_g->SetName(("zcut_vs_nsig_" + cutname + "_g").c_str());
            zcutscan_nsig_g->SetTitle(("zcut_vs_nsig_" + cutname + "_g;zcut [mm];nsig").c_str());
            zcutscan_nsig_g->SetMarkerStyle(23);
            zcutscan_nsig_g->SetMarkerSize(2.0);
            zcutscan_nsig_g->SetMarkerColor(57);

            TGraph* zcutscan_nbkg_g = new TGraph();
            zcutscan_nbkg_g->SetName(("zcut_vs_nbkg_" + cutname + "_g").c_str());
            zcutscan_nbkg_g->SetTitle(("zcut_vs_nbkg_" + cutname + "_g;zcut [mm];nbkg").c_str());
            zcutscan_nbkg_g->SetMarkerStyle(45);
            zcutscan_nbkg_g->SetMarkerSize(2.0);
            zcutscan_nbkg_g->SetMarkerColor(49);

            TGraph* nbkg_zbi_g = new TGraph();
            nbkg_zbi_g->SetName(("nbkg_vs_zbi_" + cutname + "_g").c_str());
            nbkg_zbi_g->SetTitle(("nbkg_vs_zbi_" + cutname + "_g;nbkg;zbi").c_str());

            TGraph* nsig_zbi_g = new TGraph();
            nsig_zbi_g->SetName(("nsig_vs_zbi_" + cutname + "_g").c_str());
            nsig_zbi_g->SetTitle(("nsig_vs_zbi_" + cutname + "_g;nsig;zbi").c_str());

            TH2F* nsig_zcut_hh = new TH2F(("nsig_v_zcut_zbi_" + cutname + "_hh").c_str(),
                                          ("nsig_v_zcut_zbi_" + cutname + "_hh; zcut [mm]; Nbkg").c_str(), 200, -50.3,
                                          149.7, 3000, 0.0, 300.0);

            TH2F* nbkg_zcut_hh = new TH2F(("nbkg_v_zcut_zbi_" + cutname + "_hh").c_str(),
                                          ("nbkg_v_zcut_zbi_" + cutname + "_hh; zcut [mm]; Nbkg").c_str(), 200, -50.3,
                                          149.7, 3000, 0.0, 300.0);

            // Find maximum position of Zcut --> ZBi calculation requires non-zero background
            // Start the Zcut position at the target
            double zcut_step = 0.1;
            TH1F* bkg_zVtx_h = (TH1F*)testCutHistos_->get1dHisto("testCutHistos_background_zVtx_" + cutname + "_h");
            double max_zcut = bkg_model->GetXmin();
            double endIntegral =
                bkg_zVtx_h->GetBinLowEdge(bkg_zVtx_h->FindLastBinAbove(0.0)) + bkg_zVtx_h->GetBinWidth(1);
            // double endIntegral = 100.0
            double testIntegral = bkg_model->Integral(max_zcut, endIntegral);
            if (debug_)
                std::cout << "Background between " << max_zcut << "and end of histo is " << testIntegral << std::endl;
            while (testIntegral > min_ztail_events_) {
                max_zcut = max_zcut + zcut_step;
                testIntegral = bkg_model->Integral(max_zcut, endIntegral);
                if (testIntegral < min_ztail_events_) {
                    max_zcut = max_zcut - zcut_step;
                    testIntegral = bkg_model->Integral(max_zcut, endIntegral);
                    break;
                }
            }
            if (debug_)
                std::cout << "Maximum Zcut: " << max_zcut << " gives " << testIntegral << " background events"
                          << std::endl;

            // If configuration does not specify scanning Zcut values, use single Zcut position at maximum position.
            double min_zcut = bkg_model->GetXmin();
            if (!scan_zcut_) min_zcut = max_zcut;
            std::cout << "Minimum Zcut position: " << min_zcut << std::endl;

            // Get the signal vtx z selection efficiency *before* zcut is applied
            if (debug_) std::cout << "Get signal vtx z selection efficiency before Zcut" << std::endl;
            TH1F* true_vtx_NoZ_h = (TH1F*)vtx_z_hh->ProjectionY((cutname + "_" + "true_vtx_z_projy").c_str(), 1,
                                                                vtx_z_hh->GetXaxis()->GetNbins(), "");

            // Convert the truth vertex z distribution beyond Zcut into the appropriately binned Selection.
            // Binning must match Signal pre-trigger distribution, in order to take Efficiency between them.

            outFile_->cd(("testCuts_pct_sig_cut_" + std::to_string(cutSignal)).c_str());

            // Scan Zcut position and calculate ZBi
            double best_scan_zbi = -999.9;
            double best_scan_zcut;
            double best_scan_nsig;
            double best_scan_nbkg;
            if (debug_) std::cout << "Scanning zcut position" << std::endl;
            for (double zcut = min_zcut; zcut < (max_zcut + zcut_step); zcut = zcut + zcut_step) {
                double Nbkg = bkg_model->Integral(zcut, endIntegral);
                // std::cout << "zcut: " << zcut << std::endl;
                // std::cout << "Nbkg: " << Nbkg << std::endl;

                // Get the Signal truth vertex z distribution beyond the reconstructed vertex Zcut
                TH1F* true_vtx_z_h = (TH1F*)vtx_z_hh->ProjectionY(
                    (std::to_string(zcut) + "_" + cutname + "_" + "true_vtx_z_projy").c_str(),
                    vtx_z_hh->GetXaxis()->FindBin(zcut) + 1, vtx_z_hh->GetXaxis()->GetNbins(), "");

                // Convert the truth vertex z distribution beyond Zcut into the appropriately binned Selection.
                // Binning must match Signal pre-trigger distribution, in order to take Efficiency between them.

                // Get Signal Selection Efficiency, as a function of truth vertex Z, F(z)
                // if(debug_) std::cout << "Get Signal Selection Efficiency" << std::endl;
                // if(zcut == min_zcut){
                //     outFile_->cd(("testCuts_pct_sig_cut_"+std::to_string(cutSignal)).c_str());
                //     effCalc_h->Write();
                // }

                // double eps2 = std::pow(10, logEps2_);
                // double eps = std::sqrt(eps2);

                // Calculate expected signal for Neutral Dark Vector "rho"
                // double nSigRho = simpEqs_->expectedSignalCalculation(signal_mass_, eps, true, E_Vd_, effCalc_h,
                // dNdm_,
                //                                                      radFrac_, radAcc_, -4.3, zcut);

                // Calculate expected signal for Neutral Dark Vector "rho"
                // double nSigPhi = simpEqs_->expectedSignalCalculation(signal_mass_, eps, false, E_Vd_, effCalc_h,
                // dNdm_,
                //                                                      radFrac_, radAcc_, -4.3, zcut);

                /*
                //Calculate expected signal for Neutral Dark Vector "rho"
                double nSigRho = background_sf_*simpEqs_->expectedSignalCalculation(signal_mass_,
                        eps, true, false, E_Vd_, effCalc_h, -4.3, zcut);

                //Calculate expected signal for Neutral Dark Vector "phi"
                double nSigPhi = background_sf_*simpEqs_->expectedSignalCalculation(signal_mass_,
                        eps, false, true, E_Vd_, effCalc_h, -4.3, zcut);

                */
                // double Nsig = nSigRho + nSigPhi;

                // if(debug_){
                //     std::cout << "nSigRho: " << nSigRho << std::endl;
                //     std::cout << "nSigPhi: " << nSigPhi << std::endl;
                //     std::cout << "Nsig: " << Nsig << std::endl;
                // }

                // Nsig = Nsig * signal_sf_;
                // if(debug_) std::cout << "Nsig after scale factor: " << Nsig << std::endl;

                // CLEAR POINTERS
                // delete effCalc_h;

                // Round Nsig, Nbkg, and then ZBi later
                // Nsig = round(Nsig);
                // Nbkg = round(Nbkg);

                // Calculate ZBi for this Test Cut using this zcut value
                // double n_on = Nsig + Nbkg;
                // double tau = 1.0;
                // double n_off = Nbkg;
                // double ZBi = calculateZBi(n_on, n_off, tau);
                // std::cout << "ZBi before rounding: " << ZBi << std::endl;

                /*
                std::cout << "[SimpZBiOptimization]::Iteration Results:" << std::endl;
                std::cout << "Zcut = " << zcut << std::endl;
                std::cout << "Nsig = " << Nsig << std::endl;
                std::cout << "n_bkg: " << Nbkg << std::endl;
                std::cout << "n_on: " << n_on << std::endl;
                std::cout << "n_off: " << n_off << std::endl;
                std::cout << "ZBi: " << ZBi << std::endl;
                */

                // Update Test Cut with best scan values
                // if (ZBi > best_scan_zbi) {
                //     best_scan_zbi = ZBi;
                //     best_scan_zcut = zcut;
                //     best_scan_nsig = Nsig;
                //     best_scan_nbkg = Nbkg;
                // }

                // Fill TGraphs
                // zcutscan_zbi_g->SetPoint(zcutscan_zbi_g->GetN(), zcut, ZBi);
                // zcutscan_nbkg_g->SetPoint(zcutscan_nbkg_g->GetN(), zcut, Nbkg);
                // zcutscan_nsig_g->SetPoint(zcutscan_nsig_g->GetN(), zcut, Nsig);
                // nbkg_zbi_g->SetPoint(nbkg_zbi_g->GetN(), Nbkg, ZBi);
                // nsig_zbi_g->SetPoint(nsig_zbi_g->GetN(), Nsig, ZBi);

                // nsig_zcut_hh->Fill(zcut, Nsig, ZBi);
                // nbkg_zcut_hh->Fill(zcut, Nbkg, ZBi);
            }

            // Write graph of zcut vs zbi for the Test Cut
            writeGraph(outFile_, "testCuts_pct_sig_cut_" + std::to_string(cutSignal), zcutscan_zbi_g);
            writeGraph(outFile_, "testCuts_pct_sig_cut_" + std::to_string(cutSignal), zcutscan_nbkg_g);
            writeGraph(outFile_, "testCuts_pct_sig_cut_" + std::to_string(cutSignal), zcutscan_nsig_g);

            // delete TGraph pointers
            delete zcutscan_zbi_g;
            delete zcutscan_nsig_g;
            delete zcutscan_nbkg_g;
            delete nbkg_zbi_g;
            delete nsig_zbi_g;
            delete nsig_zcut_hh;
            delete nbkg_zcut_hh;

            // Fill Summary Histograms Test Cuts at best zcutscan value
            processorHistos_->Fill2DHisto("test_cuts_values_hh", (double)cutSignal, (double)cutid, cutvalue);
            processorHistos_->set2DHistoYlabel("test_cuts_values_hh", cutid, cutname);

            processorHistos_->Fill2DHisto("test_cuts_ZBi_hh", (double)cutSignal, (double)cutid, best_scan_zbi);
            processorHistos_->set2DHistoYlabel("test_cuts_ZBi_hh", cutid, cutname);

            processorHistos_->Fill2DHisto("test_cuts_zcut_hh", (double)cutSignal, (double)cutid, best_scan_zcut);
            processorHistos_->set2DHistoYlabel("test_cuts_zcut_hh", cutid, cutname);

            processorHistos_->Fill2DHisto("test_cuts_nsig_hh", (double)cutSignal, (double)cutid, best_scan_nsig);
            processorHistos_->set2DHistoYlabel("test_cuts_nsig_hh", cutid, cutname);

            processorHistos_->Fill2DHisto("test_cuts_nbkg_hh", (double)cutSignal, (double)cutid, best_scan_nbkg);
            processorHistos_->set2DHistoYlabel("test_cuts_nbkg_hh", cutid, cutname);

            // Check if the best cutscan zbi for this Test Cut is the best overall ZBi for all Test Cuts
            if (best_scan_zbi > best_zbi) {
                best_zbi = best_scan_zbi;
                best_cutname = cutname;
                best_cutvalue = cutvalue;
                best_zcut = best_scan_zcut;
                best_nsig = best_scan_nsig;
                best_nbkg = best_scan_nbkg;
            }

        }  // END LOOP OVER TEST CUTS

        if (debug_) {
            std::cout << "Iteration " << iteration << " Best Test Cut is " << best_cutname << " " << best_cutvalue
                      << " with ZBi=" << best_zbi << std::endl;
            std::cout << "Update persistent cuts list with this best test cut..." << std::endl;
            std::cout << "[Persistent Cuts] Before update:" << std::endl;
            persistentCutsSelector_->printCuts();
        }

        // Find best Test Cut for iteration. Add Test Cut value to Persistent Cuts list
        processorHistos_->Fill2DHisto("best_test_cut_ZBi_hh", (double)cutSignal, best_zbi,
                                      (double)testCutsSelector_->getCutID(best_cutname));
        processorHistos_->Fill1DHisto("best_test_cut_ZBi_h", (double)cutSignal, best_zbi);
        processorHistos_->Fill1DHisto("best_test_cut_zcut_h", (double)cutSignal, best_zcut);
        processorHistos_->Fill1DHisto("best_test_cut_nsig_h", (double)cutSignal, best_nsig);
        processorHistos_->Fill1DHisto("best_test_cut_nbkg_h", (double)cutSignal, best_nbkg);
        processorHistos_->Fill1DHisto("best_test_cut_id_h", (double)cutSignal,
                                      (double)testCutsSelector_->getCutID(best_cutname));

        persistentCutsSelector_->setCutValue(best_cutname, best_cutvalue);
        if (debug_) {
            std::cout << "[Persistent Cuts] After update:" << std::endl;
            persistentCutsSelector_->printCuts();
        }

        // Write iteration histos
        signalHistos_->writeHistos(outFile_, "signal_pct_sig_cut_" + std::to_string(cutSignal));
        bkgHistos_->writeHistos(outFile_, "background_pct_sig_cut_" + std::to_string(cutSignal));
        testCutHistos_->writeHistos(outFile_, "testCuts_pct_sig_cut_" + std::to_string(cutSignal));
    }
}

void ApOptimizationProcessor::finalize() {
    std::cout << "[ApOptimizationProcessor] finalize()" << std::endl;

    if (debug_) {
        std::cout << "FINAL LIST OF PERSISTENT CUTS " << std::endl;
        persistentCutsSelector_->printCuts();
    }

    processorHistos_->saveHistos(outFile_);
    testCutHistos_->writeGraphs(outFile_, "");
}

bool ApOptimizationProcessor::failPersistentCuts(TTree* tree) {
    bool failCuts = false;
    for (cut_iter_ it = persistentCutsPtr_->begin(); it != persistentCutsPtr_->end(); it++) {
        std::string cutname = it->first;
        std::string cutvar = persistentCutsSelector_->getCutVar(cutname);
        // If no value inside the tuple exists for this cut, do not apply the cut.
        double var;
        tree->SetBranchAddress(cutvar.c_str(), &var);
        if (!tree->GetBranch(cutvar.c_str())) continue;
        if (!persistentCutsSelector_->passCutGTorLT(cutname, var)) {
            failCuts = true;
            break;
        }
    }

    return failCuts;
}

bool ApOptimizationProcessor::failTestCut(std::string cutname, TTree* tree) {
    std::string cutvar = testCutsSelector_->getCutVar(cutname);
    double cutvalue = testCutsSelector_->getCut(cutname);
    // If cut variable is not found in the list of tuples, do not apply cut
    double var;
    tree->SetBranchAddress(cutvar.c_str(), &var);
    if (!tree->GetBranch(cutvar.c_str())) return false;
    if (!testCutsSelector_->passCutGTorLT(cutname, var))
        return true;
    else
        return false;
}

DECLARE_PROCESSOR(ApOptimizationProcessor);
