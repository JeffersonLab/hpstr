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
        signalVtxSubsetAnaFilename_ = parameters.getString("signalVtxSubsetAnaFilename", signalVtxSubsetAnaFilename_);
        signalVtxAnaTreename_ = parameters.getString("signalVtxAnaTreename", signalVtxAnaTreename_);
        signalMCAnaFilename_ = parameters.getString("signalMCAnaFilename", signalMCAnaFilename_);
        signalMCAnaTreename_ = parameters.getString("signalMCAnaTreename", signalMCAnaTreename_);
        signal_pdgid_ = parameters.getString("signal_pdgid", signal_pdgid_);
        signal_sf_ = parameters.getDouble("signal_sf", signal_sf_);
        signal_mass_ = parameters.getDouble("signal_mass", signal_mass_);
        mass_window_nsigma_ = parameters.getDouble("mass_window_nsigma", mass_window_nsigma_);
        signalVtxMCSelection_ = parameters.getString("signalVtxMCSelection", signalVtxMCSelection_);
        eps_ = pow(10, parameters.getInteger("eps", eps_));  // get logeps and convert to eps

        // Optimization config
        max_iteration_ = parameters.getInteger("max_iteration", max_iteration_);
        step_size_ = parameters.getDouble("step_size", step_size_);
        min_ztail_events_ = parameters.getDouble("ztail_events", min_ztail_events_);
        scan_zcut_ = parameters.getInteger("scan_zcut", scan_zcut_);

        // Expected Signal Calculation
        radFrac_ = parameters.getDouble("radFrac", radFrac_);
        eq_cfgFile_ = parameters.getString("eq_cfgFile", eq_cfgFile_);
        hit_category_ = parameters.getString("hit_category", hit_category_);
        ztarget_ = parameters.getDouble("ztarget", ztarget_);
        psum_cut_ = parameters.getDouble("psum_cut", psum_cut_);
    } catch (std::runtime_error& error) {
        std::cout << error.what() << std::endl;
    }
}

void ApOptimizationProcessor::initialize(std::string inFilename, std::string outFilename) {
    std::cout << "[ApOptimizationProcessor] Initialize " << inFilename << std::endl;

    opts_.fMode = "UPDATE";
    outFileName_ = outFilename;

    // TODO: write equation to find mass resolution at signal mass
    massResolution_ = getMassResolution(signal_mass_);

    // Define Mass window
    lowMass_ = signal_mass_ - mass_window_nsigma_ * massResolution_;
    highMass_ = signal_mass_ + mass_window_nsigma_ * massResolution_;
    massWindow_ = "vertex.invM_ > " + std::to_string(lowMass_) + " && vertex.invM_ < " + std::to_string(highMass_);

    if (debug_) std::cout << "[SimpZBiOptimization]::Mass Window: " << lowMass_ << " - " << highMass_ << std::endl;

    // Read signal ana vertex tuple, and convert to mutable tuple
    std::cout << "[SimpZBiOptimization]::Reading Signal AnaVertex Tuple from file " << signalVtxAnaFilename_.c_str()
              << std::endl;
    signalVtxAnaFile_ = new TFile(signalVtxAnaFilename_.c_str(), "READ");
    signal_tree_ = (TTree*)signalVtxAnaFile_->Get(signalVtxAnaTreename_.c_str());

    TFile* signalVtxSubsetAnaFile = new TFile(signalVtxSubsetAnaFilename_.c_str(), "READ");
    TTree* signal_subset_tree = (TTree*)signalVtxSubsetAnaFile->Get(signalVtxAnaTreename_.c_str());

    std::cout << "[SimpZBiOptimization]::Reading Signal MC Tuple from file " << signalMCAnaFilename_.c_str()
              << std::endl;
    signalMCAnaFile_ = new TFile(signalMCAnaFilename_.c_str(), "READ");
    signal_pretrig_sim_tree_ = (TTree*)signalMCAnaFile_->Get(signalMCAnaTreename_.c_str());

    // Read background ana vertex tuple, and convert to mutable tuple
    std::cout << "[SimpZBiOptimization]::Reading Background AnaVertex Tuple from file " << bkgVtxAnaFilename_.c_str()
              << std::endl;
    bkgVtxAnaFile_ = new TFile(bkgVtxAnaFilename_.c_str(), "READ");
    bkg_tree_ = (TTree*)bkgVtxAnaFile_->Get(bkgVtxAnaTreename_.c_str());

    // Initialize Persistent Cut Selector. These cuts are applied to all events.
    // Persistent Cut values are updated each iteration with the value of the best performing Test Cut in
    // that iteration.
    std::cout << "[SimpZBiOptimization]::Initializing Set of Persistent Cuts" << std::endl;
    persistentCutsSelector_ = new TreeCutSelector("persistentCuts", cuts_cfgFile_);
    persistentCutsSelector_->LoadSelection();
    persistentCutsPtr_ = persistentCutsSelector_->getPointerToCuts();

    // initalize Test Cuts
    std::cout << "[SimpZBiOptimization]::Initializing Set of Test Cuts" << std::endl;
    testCutsSelector_ = new TreeCutSelector("testCuts", cuts_cfgFile_);
    testCutsSelector_->LoadSelection();
    testCutsPtr_ = testCutsSelector_->getPointerToCuts();
    // testCutsSelector_->filterCuts(cutVariables_);

    // Initialize signal histograms
    std::cout << "[SimpZBiOptimization]::Initializing Signal Variable Histograms" << std::endl;
    signalHistos_ = std::make_shared<ZBiHistos>("signal");
    signalHistos_->debugMode(debug_);
    signalHistos_->loadHistoConfig(variableHistCfgFilename_);

    // Initialize background histograms
    std::cout << "[SimpZBiOptimization]::Initializing Background Variable Histograms" << std::endl;
    bkgHistos_ = std::make_shared<ZBiHistos>("background");
    bkgHistos_->debugMode(debug_);
    bkgHistos_->loadHistoConfig(variableHistCfgFilename_);

    // Initialize Test Cut histograms
    std::cout << "[SimpZBiOptimization]::Initializing Test Cut Variable Histograms" << std::endl;
    testCutHistos_ = std::make_shared<ZBiHistos>("testCutHistos");
    testCutHistos_->debugMode(debug_);

    // Initialize processor histograms that summarize iterative results
    std::cout << "[SimpZBiOptimization]::Initializing Iterative Result Histograms" << std::endl;
    processorHistos_ = std::make_shared<ZBiHistos>("zbi_processor");

    // Add Test Cut Analysis Histograms necessary for calculating background and signal
    std::cout << "[SimpZBiOptimization]::Initializing Test Cut Analysis Histograms" << std::endl;
    for (range_cut_iter_ it = testCutsPtr_->begin(); it != testCutsPtr_->end(); it++) {
        std::string name = it->first;
        std::string var = testCutsSelector_->getCutVar(name, true);

        json cut_cfg = signalHistos_->getConfig();
        double* bins_and_limits = getBinsAndLimits(cut_cfg, name);
        std::string drawstring = var + ">>" + name + "_pdf_h(" + std::to_string((int)bins_and_limits[0]) + "," +
                                 std::to_string(bins_and_limits[1]) + "," + std::to_string(bins_and_limits[2]) + ")";
        signal_tree_->Draw(
            drawstring.c_str(),
            ("psum > " + std::to_string(psum_cut_) + " && " + getHitCategoryCut() + " && " + massWindow_).c_str());
        TH1F* sig_h = (TH1F*)gDirectory->Get((name + "_pdf_h").c_str());
        testCutHistos_->addHisto1d(sig_h);

        delete[] bins_and_limits;

        testVarPDFs_[name] = (TH1F*)testCutHistos_->getPDF(name + "_pdf_h");

        bool forward = testCutsSelector_->getCutRange(name).first < -999 ? false : true;
        testVarCDFs_[name] = (TH1F*)testVarPDFs_[name]->GetCumulative(forward);

        TGraphErrors* g_zbi = processorHistos_->configureGraph(("g_zbi_vs_" + name).c_str(), "", "Z_{Bi}");
        processorHistos_->addGraph(g_zbi);

        TGraphErrors* g_nsig = processorHistos_->configureGraph(("g_nsig_vs_" + name).c_str(), "", "N_{sig}");
        processorHistos_->addGraph(g_nsig);

        TGraphErrors* g_nbkg = processorHistos_->configureGraph(("g_nbkg_vs_" + name).c_str(), "", "N_{bkg}");
        processorHistos_->addGraph(g_nbkg);

        TGraphErrors* g_zcut = processorHistos_->configureGraph(("g_zcut_vs_" + name).c_str(), "", "z_{cut}/mm");
        processorHistos_->addGraph(g_zcut);

        TGraphErrors* g_cut_frac =
            processorHistos_->configureGraph(("g_cut_frac_vs_" + name).c_str(), "", "N_{sig}^{cut}/N_{sig}^{original}");
        processorHistos_->addGraph(g_cut_frac);

        if (name == "pos_z0" || name == "ele_z0" || name == "min_y0") {
            TGraphErrors* g_zoffset =
                processorHistos_->configureGraph(("g_zoffset_vs_" + name).c_str(), "", "z_{offset}/mm");
            processorHistos_->addGraph(g_zoffset);

            TGraphErrors* g_alpha = processorHistos_->configureGraph(("g_alpha_vs_" + name).c_str(), "", "#alpha");
            processorHistos_->addGraph(g_alpha);
        }
    }

    signal_tree_->Draw(
        "vertex.invM_ >> h_signal_mass(200, 0, 0.2)",
        ("psum > " + std::to_string(psum_cut_) + " && " + getHitCategoryCut() + " && " + massWindow_).c_str());
    TH1D* h_signal_mass = (TH1D*)gDirectory->Get("h_signal_mass");
    testCutHistos_->addHisto1d(h_signal_mass);

    std::cout << "[SimpZBiOptimization]::Calculating Signal Vertex Efficiency" << std::endl;
    signal_pretrig_sim_tree_->Draw("vtx.z >> h_pretrig_signal_vtxz(200, -50, 150)");
    h_pretrig_signal_vtxz_ = (TH1D*)gDirectory->Get("h_pretrig_signal_vtxz");
    h_pretrig_signal_vtxz_->Sumw2();

    signal_subset_tree->Draw("true_ap.vtx_z_ >> h_signal_vtxz_rad_(200, -50, 150)",
                             ("psum > " + std::to_string(psum_cut_) + " && " + massWindow_).c_str());
    h_signal_vtxz_rad_ = (TH1D*)gDirectory->Get("h_signal_vtxz_rad_");
    h_signal_vtxz_rad_->SetTitle(";vtx_z [mm];Events/0.5mm");
    h_signal_vtxz_rad_->Sumw2();

    TH1D* h_xi_eff = (TH1D*)h_signal_vtxz_rad_->Clone("h_xi_eff");
    h_xi_eff->Divide(h_pretrig_signal_vtxz_);

    signalHistos_->addHisto1d(h_signal_vtxz_rad_);
    signalHistos_->addHisto1d(h_pretrig_signal_vtxz_);
    signalHistos_->addHisto1d(h_xi_eff);

    bkg_tree_->Draw("vertex.getZ() >> h_data_vtxz_rad(200, -50, 150)",
                    ("psum > " + std::to_string(psum_cut_) + " && " + massWindow_).c_str());
    auto h_data_vtxz_rad_ = (TH1D*)gDirectory->Get("h_data_vtxz_rad");
    h_data_vtxz_rad_->SetTitle(";vtx_z [mm];Events/0.5mm");

    bkgHistos_->addHisto1d(h_data_vtxz_rad_);

    f_xi_eff_ = new TF1("f_xi_eff_", "exp([0] + [1]*x + [2]*x*x)", 0, 150);
    h_xi_eff->Fit(f_xi_eff_, "SRQ", "", 0, 150);

    double xi_eff_0 = f_xi_eff_->Eval(ztarget_);
    f_xi_eff_->SetParameters(f_xi_eff_->GetParameter(0) - log(xi_eff_0), f_xi_eff_->GetParameter(1),
                             f_xi_eff_->GetParameter(2));
}

bool ApOptimizationProcessor::process() {
    std::cout << "[ApOptimizationProcessor]::process()" << std::endl;

    // Fill Initial Signal histograms
    std::cout << "[SimpZBiOptimization]::Filling initial signal histograms" << std::endl;
    auto df_signal_mod_ = prepareDF(RDataFrame(*signal_tree_));
    auto og_counts_sig = df_signal_mod_.Count();

    std::vector<std::string> signalVars = getListOfVariables(signalHistos_->getConfig(), false);
    df_signal_mod_.Snapshot("trees/signal", outFileName_, signalVars);

    std::cout << "[SimpZBiOptimization]::Filling initial background histograms" << std::endl;
    auto df_bkg_mod_ = prepareDF(RDataFrame(*bkg_tree_));
    auto og_counts_bkg = df_bkg_mod_.Count();
    std::vector<std::string> bkgVars = getListOfVariables(bkgHistos_->getConfig(), true);
    df_bkg_mod_.Snapshot("trees/background", outFileName_, bkgVars, opts_);

    // define fraction of radiative selected signal to simulated signal
    RDF::RInterface<Detail::RDF::RJittedFilter, void> df_signal_ =
        applyFilter(df_signal_mod_, "psum > " + std::to_string(psum_cut_), "radCut");
    RDF::RInterface<Detail::RDF::RJittedFilter, void> df_bkg_ =
        applyFilter(df_bkg_mod_, "psum > " + std::to_string(psum_cut_), "radCut");

    RDF::RResultPtr<TH1D> h_data_mass_rad_selected =
        df_bkg_.Histo1D({"h_data_mass_rad_selected", ";mass [GeV];Events/0.5GeV", 100, 0, 0.4}, "vertex.invM_");

    h_data_mass_rad_selected->Sumw2();
    TH1D* h_data_mass_rad_ = (TH1D*)h_data_mass_rad_selected->Clone("h_data_mass_rad_");
    testCutHistos_->addHisto1d(h_data_mass_rad_);

    df_signal_ = applyFilter(df_signal_, massWindow_, "massWindow");
    df_bkg_ = applyFilter(df_bkg_, massWindow_, "massWindow");

    df_signal_ = applyFilter(df_signal_, getHitCategoryCut(), getHitCategoryCut());
    df_bkg_ = applyFilter(df_bkg_, getHitCategoryCut(), getHitCategoryCut());

    // Step_size defines n% of signal distribution to cut in a given variable
    double cutFraction = step_size_;
    if (max_iteration_ > (int)1.0 / cutFraction) max_iteration_ = (int)1.0 / cutFraction;

    // Iteratively cut n% of the signal distribution for a given Test Cut variable
    int n_quantiles_ = max_iteration_;
    // Store result histograms for each Test Cut variable
    std::map<std::string, std::vector<RDF::RResultPtr<TH1D>>> sig_hists;
    std::map<std::string, std::vector<RDF::RResultPtr<TH1D>>> bkg_hists;

    std::map<std::string, std::vector<RDF::RInterface<Detail::RDF::RJittedFilter, void>>> df_signal_cut;
    std::map<std::string, std::vector<RDF::RInterface<Detail::RDF::RJittedFilter, void>>> df_bkg_cut;

    std::map<std::string, double> bestCuts;

    for (range_cut_iter_ it = testCutsPtr_->begin(); it != testCutsPtr_->end(); it++) {
        if (debug_) std::cout << "Getting quantiles for Test Cut variable " << it->first << std::endl;
        std::string cutname = it->first;
        // if (cutname == "ele_L1_iso_significance" || cutname == "pos_L1_iso_significance") {
        //     // these variables have been changed from iso to Riso in the cuts file, so skip them here
        //     continue;
        // }
        std::string cutvar = testCutsSelector_->getCutVar(cutname);
        if (debug_) std::cout << "Test Cut variable: " << cutvar << std::endl;

        TH1F* cdf = testVarCDFs_[cutname];
        if (!cdf) {
            std::cout << "Error: could not find CDF for Test Cut variable " << cutvar << std::endl;
            continue;
        }

        // quantiles of un-cut distribution
        // TH1F* pdf = testVarPDFs_[cutname];
        // double* quantiles = getQuantileArray(testCutsSelector_->getCutRange(cutname), n_quantiles_);
        // double* quantile_pos = new double[n_quantiles_];
        // pdf->GetQuantiles(n_quantiles_, quantile_pos, quantiles);
        // testVarQuantiles_[cutname] = quantile_pos;

        // Prepare storage for signal and background histograms for this Test Cut
        std::vector<RDF::RResultPtr<TH1D>> sig_h;
        std::vector<RDF::RResultPtr<TH1D>> bkg_h;

        sig_hists.insert(std::make_pair(cutname, sig_h));
        bkg_hists.insert(std::make_pair(cutname, bkg_h));

        df_signal_cut[cutname].push_back(df_signal_);
        df_bkg_cut[cutname].push_back(df_bkg_);

        // Apply persistent cuts and save cut values (all cuts except the one being tested)
        for (range_cut_iter_ it = persistentCutsPtr_->begin(); it != persistentCutsPtr_->end(); it++) {
            std::string persistent_cut = it->first;
            if (persistent_cut != cutname) {
                std::string cutvar_persistent = persistentCutsSelector_->getCutVar(persistent_cut);
                if (persistent_cut == "pos_z0" || persistent_cut == "ele_z0" || persistent_cut == "min_y0") {
                    double zoffset = persistentCutsSelector_->getCutRange(persistent_cut).first;  // mm
                    double alpha = persistentCutsSelector_->getCutRange(persistent_cut).second;   // rad
                    std::string filter = "vertex_z * " + std::to_string(alpha) + " - abs(" + cutvar_persistent +
                                         ") < " + std::to_string(alpha) + " * " + std::to_string(zoffset);

                    df_signal_cut[cutname].back() =
                        applyFilter(df_signal_cut[cutname].back(), filter, persistent_cut + "_persistent");
                    df_bkg_cut[cutname].back() =
                        applyFilter(df_bkg_cut[cutname].back(), filter, persistent_cut + "_persistent");
                } else {
                    std::string filter = "";
                    if (persistentCutsSelector_->getCutRange(persistent_cut).first < -999.) {
                        filter = cutvar_persistent + " < " +
                                 std::to_string(persistentCutsSelector_->getCutRange(persistent_cut).second);

                    } else if (persistentCutsSelector_->getCutRange(persistent_cut).second > 999.) {
                        filter = cutvar_persistent + " > " +
                                 std::to_string(persistentCutsSelector_->getCutRange(persistent_cut).first);
                    }

                    // applying persistent cuts on top of radiative, massWindow and hit category cuts
                    df_signal_cut[cutname].back() =
                        applyFilter(df_signal_cut[cutname].back(), filter, persistent_cut + "_persistent");
                    df_bkg_cut[cutname].back() =
                        applyFilter(df_bkg_cut[cutname].back(), filter, persistent_cut + "_persistent");
                }
            }
        }

        std::cout << "Applying persistent cuts complete. Getting quantiles for Test Cut variable " << cutname
                  << std::endl;

        // Get cutvalue that corresponds to cutting n% of signal distribution in cutvar
        if (cutname == "pos_z0" || cutname == "ele_z0" || cutname == "min_y0") {
            auto pdf_signal_cut =
                df_signal_cut[cutname].back().Histo2D({("h_pdf2d_signal_cut_" + cutname).c_str(),
                                                       ";z_{vtx}/mm;y_{0}/mm;N_{events}/a.u.", 120, -5, 25, 200, 0, 2},
                                                      "vertex_z", "abs_" + cutvar);

            testZoffsetAlpha_[cutname] = getZoffsetAlpha(pdf_signal_cut, n_quantiles_, 10);

            std::cout << "Quantiles for Test Cut variable " << cutname << ": ";
            for (int i = 0; i < n_quantiles_; i++) {
                std::cout << "zoffset " << testZoffsetAlpha_[cutname][i].first << " alpha "
                          << testZoffsetAlpha_[cutname][i].second << std::endl;
            }
            std::cout << "done" << std::endl;
        } else {
            json cut_cfg = signalHistos_->getConfig();
            double* bins_and_limits = getBinsAndLimits(cut_cfg, cutname);

            auto pdf_signal_cut =
                df_signal_cut[cutname].back().Histo1D({("h_pdf_signal_cut_" + cutname).c_str(), ";x;y",
                                                       (int)bins_and_limits[0], bins_and_limits[1], bins_and_limits[2]},
                                                      cutvar);

            delete[] bins_and_limits;

            // quantiles of cut distribution
            double* quantile_pos = new double[n_quantiles_];
            double* quantiles = getQuantileArray(testCutsSelector_->getCutRange(cutname), n_quantiles_);
            pdf_signal_cut->GetQuantiles(n_quantiles_, quantile_pos, quantiles);
            testVarQuantiles_[cutname] = quantile_pos;

            std::cout << "Quantiles for Test Cut variable " << cutname << ": ";
            for (int i = 0; i < n_quantiles_; i++) {
                std::cout << testVarQuantiles_[cutname][i] << " ";
            }
            std::cout << std::endl;
        }

        double best_ZBi = -99;
        double best_cutval;

        for (int iteration = 0; iteration < max_iteration_; iteration++) {
            double cutvalue;
            std::string filter = "";

            double cutSignal = (double)iteration * step_size_;
            if (cutname == "pos_z0" || cutname == "ele_z0" || cutname == "min_y0") {
                double zoffset = testZoffsetAlpha_[cutname][iteration].first;  // mm
                double alpha = testZoffsetAlpha_[cutname][iteration].second;   // rad

                std::string filter = "vertex_z * " + std::to_string(alpha) + " - abs(" + cutvar + ") < " +
                                     std::to_string(alpha) + " * " + std::to_string(zoffset);

                cutvalue = zoffset;  // for storing the best cut

                // applying test cut on top of persistent cuts
                df_signal_cut[cutname].push_back(
                    applyFilter(df_signal_cut[cutname].front(), filter,
                                cutname + "_" + std::to_string(zoffset) + "_" + std::to_string(alpha)));
                df_bkg_cut[cutname].push_back(
                    applyFilter(df_bkg_cut[cutname].front(), filter,
                                cutname + "_" + std::to_string(zoffset) + "_" + std::to_string(alpha)));
            } else {
                cutvalue = testVarQuantiles_[cutname][iteration];

                if (testCutsSelector_->getCutRange(cutname).first < -999.) {
                    testCutsSelector_->setCutValue(cutname, std::make_pair(-9999.9, cutvalue));
                    filter = cutvar + " < " + std::to_string(testCutsSelector_->getCutRange(cutname).second);
                } else if (testCutsSelector_->getCutRange(cutname).second > 999.) {
                    testCutsSelector_->setCutValue(cutname, std::make_pair(cutvalue, 9999.9));
                    filter = cutvar + " > " + std::to_string(testCutsSelector_->getCutRange(cutname).first);
                }
                // applying test cut on top of persistent cuts
                df_signal_cut[cutname].push_back(
                    applyFilter(df_signal_cut[cutname].front(), filter, cutname + "_" + std::to_string(iteration)));
                df_bkg_cut[cutname].push_back(
                    applyFilter(df_bkg_cut[cutname].front(), filter, cutname + "_" + std::to_string(iteration)));
            }

            // Saving 2D histograms of z0 vs vtx z
            // Histo2D({"histName", "histTitle", binsx, minx, maxx, binsy, miny, maxy}, "myValueX", "myValueY");
            auto h_signal_2d = df_signal_cut[cutname].back().Histo2D(
                {("h_2d_signal_" + cutname + std::to_string(iteration)).c_str(), ";z_{vtx}/mm;y0min/mm;N_{events}/a.u.",
                 120u, -5, 25, 200u, 0, 2},
                "vertex_z", "min_y0");
            TH2D* h2d =
                (TH2D*)h_signal_2d->Clone(("h_zvtx_y0min_signal_" + cutname + std::to_string(iteration)).c_str());
            testCutHistos_->addHisto2d(h2d);

            auto h_bkg_2d =
                df_bkg_cut[cutname].back().Histo2D({("h_2d_bkg_" + cutname + std::to_string(iteration)).c_str(),
                                                    ";z_{vtx}/mm;y0min/mm;N_{events}/a.u.", 120u, -5, 25, 200u, 0, 2},
                                                   "vertex_z", "min_y0");
            TH2D* h2d_bkg = (TH2D*)h_bkg_2d->Clone(("h_zvtx_y0min_bkg_" + cutname + std::to_string(iteration)).c_str());
            testCutHistos_->addHisto2d(h2d_bkg);

            // Only booking the operations here, to minimize looping over entries
            // {hname, ";" + xtitle + ";" + ytitle, bins, minX, maxX}, var)
            sig_hists.at(cutname).push_back(df_signal_cut[cutname].back().Histo1D(
                {("h_zvtx_signal_" + cutname + "_" + std::to_string(iteration)).c_str(), ";z_{vtx}/mm;N_{events}/a.u.",
                 200, -50.0, 150.0},
                "vertex_z"));
            bkg_hists.at(cutname).push_back(df_bkg_cut[cutname].back().Histo1D(
                {("h_zvtx_background_" + cutname + "_" + std::to_string(iteration)).c_str(),
                 ";z_{vtx}/mm;N_{events}/a.u.", 200, -50.0, 150.0},
                "vertex_z"));

            bkg_hists.at(cutname).at(iteration)->Sumw2();
            std::string fitname = cutname + "_" + std::to_string(iteration);
            std::vector<double> fit_params_vec = fitZBkgTail(bkg_hists.at(cutname).at(iteration), fitname, true);

            double z_cut = fit_params_vec.at(2) + fit_params_vec.at(3) / fit_params_vec.at(0) *
                                                      (fit_params_vec.at(0) * fit_params_vec.at(0) / 2. -
                                                       Math::log(min_ztail_events_ * fit_params_vec.at(0) /
                                                                 (fit_params_vec.at(1) * fit_params_vec.at(3))));

            if (debug_) std::cout << "z cut for " << cutname << " " << z_cut << std::endl;

            auto all_cuts_sig = applyFilter(df_signal_cut[cutname].at(iteration + 1),
                                            "vertex_z > " + std::to_string(z_cut), cutname + "_zcut");
            auto all_cuts_bkg = applyFilter(df_bkg_cut[cutname].at(iteration + 1),
                                            "vertex_z > " + std::to_string(z_cut), cutname + "_zcut");

            auto zsig =
                all_cuts_sig.Histo1D({("h_zvtx_signal_" + cutname + "_all_cuts_" + std::to_string(iteration)).c_str(),
                                      ";z_{vtx}/mm;N_{events}/a.u.", 200, -50.0, 150.0},
                                     "vertex_z");
            auto zbkg = all_cuts_bkg.Histo1D(
                {("h_zvtx_background_" + cutname + "_all_cuts_" + std::to_string(iteration)).c_str(),
                 ";z_{vtx}/mm;N_{events}/a.u.", 200, -50.0, 150.0},
                "vertex_z");

            // Also save z vtx for signal after all cuts applied
            sig_hists.at(cutname).push_back(
                all_cuts_sig.Histo1D({("h_zvtx_signal_" + cutname + "_zcut_" + std::to_string(iteration)).c_str(),
                                      ";z_{vtx}/mm;N_{events}/a.u.", 200, -50.0, 150.0},
                                     "vertex_z"));

            zsig->Sumw2();
            TH1D* h_chi_eff = (TH1D*)zsig->Clone(("h_chi_eff_" + cutname + "_" + std::to_string(iteration)).c_str());
            h_chi_eff->Divide(h_signal_vtxz_rad_);

            double Nsig = computeDisplacedYield(h_data_mass_rad_, h_chi_eff, 0.8 * 3.74, 0.5);

            double Nbkg = 0.0;
            if (zbkg->Integral() > 10) {
                zbkg->Fit("expo", "QRS", "", z_cut, zbkg->GetXaxis()->GetXmax());
                TF1* fit = zbkg->GetFunction("expo");
                Nbkg = fit->Integral(z_cut, zbkg->GetXaxis()->GetXmax());
            } else {
                Nbkg = zbkg->Integral();
            }

            if (debug_) {
                std::cout << "Test Cut " << cutname << " at iteration " << iteration << " (cutting " << cutSignal * 100
                          << "% of signal in this variable). bkg: " << Nbkg << " sig: " << Nsig << std::endl;
            }

            // scale Nsig
            Nsig = Nsig * signal_sf_;

            // Round Nsig, Nbkg
            Nsig = round(Nsig);
            Nbkg = round(Nbkg);

            // Calculate ZBi for this Test Cut using this zcut value
            double n_on = Nsig + Nbkg;
            double tau = 1.0;  // TODO: look up what tau is
            double n_off = Nbkg;
            double ZBi = calculateZBi(n_on, n_off, tau);

            if (ZBi > best_ZBi) {
                best_ZBi = ZBi;
                best_cutval = cutvalue;
            }

            if (iteration > 0) {
                if (cutname == "pos_z0" || cutname == "ele_z0" || cutname == "min_y0") {
                    double zoffset = testZoffsetAlpha_[cutname][iteration].first;  // mm
                    double alpha = testZoffsetAlpha_[cutname][iteration].second;   // rad

                    processorHistos_->getGraph("g_nsig_vs_" + cutname)->AddPoint(iteration, Nsig);
                    processorHistos_->getGraph("g_nbkg_vs_" + cutname)->AddPoint(iteration, Nbkg);
                    processorHistos_->getGraph("g_zcut_vs_" + cutname)->AddPoint(iteration, z_cut);
                    processorHistos_->getGraph("g_zbi_vs_" + cutname)->AddPoint(iteration, ZBi);
                    processorHistos_->getGraph("g_cut_frac_vs_" + cutname)
                        ->AddPoint(iteration, cutFraction * iteration);
                    processorHistos_->getGraph("g_zoffset_vs_" + cutname)->AddPoint(iteration, zoffset);
                    processorHistos_->getGraph("g_alpha_vs_" + cutname)->AddPoint(iteration, alpha);
                } else {
                    processorHistos_->getGraph("g_nsig_vs_" + cutname)->AddPoint(cutvalue, Nsig);
                    processorHistos_->getGraph("g_nbkg_vs_" + cutname)->AddPoint(cutvalue, Nbkg);
                    processorHistos_->getGraph("g_zcut_vs_" + cutname)->AddPoint(cutvalue, z_cut);
                    processorHistos_->getGraph("g_zbi_vs_" + cutname)->AddPoint(cutvalue, ZBi);
                    processorHistos_->getGraph("g_cut_frac_vs_" + cutname)
                        ->AddPoint(cutvalue, cdf->GetBinContent(cdf->FindBin(cutvalue)));
                }
            }
        }
        bestCuts.insert(std::make_pair(cutname, best_cutval));
    }

    // for (auto const& pair : bestCuts) {
    //     std::string cutname = pair.first;
    //     double cutval = pair.second;
    //     std::cout << "Updating persistent cuts with best cut for " << cutname << ": " << cutval << std::endl;
    //     if (persistentCutsSelector_->getCutRange(cutname).first < -999.) {
    //         persistentCutsSelector_->setCutValue(cutname, std::make_pair(-9999.9, cutval));
    //     } else {
    //         persistentCutsSelector_->setCutValue(cutname, std::make_pair(cutval, 9999.9));
    //     }
    // }

    // save sig_hists, bkg_hists to outfile_
    outFile_ = TFile::Open(outFileName_.c_str(), "UPDATE");
    outFile_->cd();
    TDirectory* dir{nullptr};
    std::string folder = "output_hists";
    if (!folder.empty()) {
        dir = outFile_->mkdir(folder.c_str(), "", true);
        dir->cd();
    }
    for (range_cut_iter_ it = testCutsPtr_->begin(); it != testCutsPtr_->end(); it++) {
        std::string cutname = it->first;
        for (int i = 0; i < sig_hists.at(cutname).size(); i++) {
            sig_hists.at(cutname)[i]->Write();
        }
        for (int i = 0; i < bkg_hists.at(cutname).size(); i++) {
            bkg_hists.at(cutname)[i]->Write();
        }
    }
    outFile_->Close();

    // std::cout << "Original number of signal events: " << *og_counts_sig << std::endl;
    // std::cout << "Original number of background events: " << *og_counts_bkg << std::endl;

    std::cout << df_signal_mod_.GetNRuns() << std::endl;

    return true;
}

void ApOptimizationProcessor::finalize() {
    std::cout << "[ApOptimizationProcessor] finalize()" << std::endl;

    if (debug_) {
        std::cout << "FINAL LIST OF PERSISTENT CUTS " << std::endl;
        persistentCutsSelector_->printCuts();
    }

    // Initialize output file
    std::cout << "[SimpZBiOptimization]::Output File: " << outFileName_.c_str() << std::endl;
    outFile_ = new TFile(outFileName_.c_str(), "UPDATE");

    processorHistos_->saveHistos(outFile_, "processorHistos");
    processorHistos_->writeGraphs(outFile_, "processorGraphs");
    processorHistos_->writeHistosFromDF(outFile_, "processorHistos");
    testCutHistos_->saveHistos(outFile_, "testCutHistos");
    signalHistos_->saveHistos(outFile_, "signal");
    bkgHistos_->saveHistos(outFile_, "background");

    outFile_->Close();
}

RDF::RInterface<Detail::RDF::RLoopManager, void> ApOptimizationProcessor::prepareDF(RDataFrame df) {
    std::cout << "[ApOptimizationProcessor]::prepareDF" << std::endl;

    auto df_new =
        df.Define("vertex_z", [](const Vertex& vtx) { return vtx.getZ(); }, {"vertex."})
            .Define("ele_p", [](const Particle& ele) { return ele.getP(); }, {"ele."})
            .Define("pos_p", [](const Particle& pos) { return pos.getP(); }, {"pos."})
            .Define("ele_z0err", [](const Particle& ele) { return ele.getTrack().getZ0Err(); }, {"ele."})
            .Define("pos_z0err", [](const Particle& pos) { return pos.getTrack().getZ0Err(); }, {"pos."})
            .Define("ele_track_time", [](const Particle& ele) { return ele.getTrack().getTrackTime(); }, {"ele."})
            .Define("pos_track_time", [](const Particle& pos) { return pos.getTrack().getTrackTime(); }, {"pos."})
            .Define("vertex_cxx", [](const Vertex& vtx) { return vtx.getCovariance().at(0); }, {"vertex."})
            .Define("vertex_cyy", [](const Vertex& vtx) { return vtx.getCovariance().at(2); }, {"vertex."})
            .Define("vertex_czz", [](const Vertex& vtx) { return vtx.getCovariance().at(5); }, {"vertex."})
            .Define("ele_d0", [](const Particle& ele) { return ele.getTrack().getD0(); }, {"ele."})
            .Define("pos_d0", [](const Particle& pos) { return pos.getTrack().getD0(); }, {"pos."})
            .Define("ele_tanLambda", [](const Particle& ele) { return ele.getTrack().getTanLambda(); }, {"ele."})
            .Define("pos_tanLambda", [](const Particle& pos) { return pos.getTrack().getTanLambda(); }, {"pos."})
            .Define("ele_phi0", [](const Particle& ele) { return ele.getTrack().getPhi(); }, {"ele."})
            .Define("pos_phi0", [](const Particle& pos) { return pos.getTrack().getPhi(); }, {"pos."})
            .Define("ele_z0", [](const Particle& ele) { return ele.getTrack().getZ0(); }, {"ele."})
            .Define("pos_z0", [](const Particle& pos) { return pos.getTrack().getZ0(); }, {"pos."})
            .Define("abs_ele_z0", [](const Particle& ele) { return std::abs(ele.getTrack().getZ0()); }, {"ele."})
            .Define("abs_pos_z0", [](const Particle& pos) { return std::abs(pos.getTrack().getZ0()); }, {"pos."})
            .Define("abs_min_y0", [](const double min_y0) { return std::abs(min_y0); }, {"min_y0"})
            .Define("ele_nhits", [](const Particle& ele) { return ele.getTrack().getSvtHits().GetEntries(); }, {"ele."})
            .Define("pos_nhits", [](const Particle& pos) { return pos.getTrack().getSvtHits().GetEntries(); },
                    {"pos."});

    return df_new;
}

void ApOptimizationProcessor::fillEventHistograms(std::shared_ptr<ZBiHistos> histos,
                                                  RDF::RInterface<Detail::RDF::RLoopManager, void> df, bool isBkg) {
    json histo_cfg = histos->getConfig();
    std::string histo_cfg_name = histos->getName();

    for (auto& hist_config : histo_cfg.items()) {
        std::string h_name = hist_config.key();
        if (h_name.find("true_") != std::string::npos && isBkg) {
            std::cout << "Skipping true histogram for background: " << h_name << std::endl;
            continue;  // skip true histograms for background
        }
        std::string xtitle = hist_config.value().at("xtitle");
        std::string ytitle = hist_config.value().at("ytitle");
        int bins = hist_config.value().at("bins");
        double minX = hist_config.value().at("minX");
        double maxX = hist_config.value().at("maxX");
        std::string var = hist_config.value().at("var");
        h_name = histo_cfg_name + "_" + h_name;
        histos->addHistoFromDF(
            df.Histo1D({h_name.c_str(), (";" + xtitle + ";" + ytitle).c_str(), bins, minX, maxX}, var.c_str()));
    }
}

std::vector<std::string> ApOptimizationProcessor::getListOfVariables(json histo_cfg, bool isBkg) {
    std::vector<std::string> variables;

    for (auto& hist_config : histo_cfg.items()) {
        std::string h_name = hist_config.key();
        if (h_name.find("true_") != std::string::npos && isBkg) {
            std::cout << "Skipping true histogram for background: " << h_name << std::endl;
            continue;  // skip true histograms for background
        }
        std::string var = hist_config.value().at("var");
        variables.push_back(var);
    }

    variables.push_back("eleL1");
    variables.push_back("posL1");
    variables.push_back("eleL2");
    variables.push_back("posL2");

    return variables;
}

void ApOptimizationProcessor::configureGraphs(TGraph* zcutscan_zbi_g, TGraph* zcutscan_nsig_g, TGraph* zcutscan_nbkg_g,
                                              TGraph* nbkg_zbi_g, TGraph* nsig_zbi_g, std::string cutname) {
    zcutscan_zbi_g->SetName(("zcut_vs_zbi_" + cutname + "_g").c_str());
    zcutscan_zbi_g->SetTitle(("zcut_vs_zbi_" + cutname + "_g;zcut [mm];zbi").c_str());
    zcutscan_zbi_g->SetMarkerStyle(8);
    zcutscan_zbi_g->SetMarkerSize(2.0);
    zcutscan_zbi_g->SetMarkerColor(2);

    zcutscan_nsig_g->SetName(("zcut_vs_nsig_" + cutname + "_g").c_str());
    zcutscan_nsig_g->SetTitle(("zcut_vs_nsig_" + cutname + "_g;zcut [mm];nsig").c_str());
    zcutscan_nsig_g->SetMarkerStyle(23);
    zcutscan_nsig_g->SetMarkerSize(2.0);
    zcutscan_nsig_g->SetMarkerColor(57);

    zcutscan_nbkg_g->SetName(("zcut_vs_nbkg_" + cutname + "_g").c_str());
    zcutscan_nbkg_g->SetTitle(("zcut_vs_nbkg_" + cutname + "_g;zcut [mm];nbkg").c_str());
    zcutscan_nbkg_g->SetMarkerStyle(45);
    zcutscan_nbkg_g->SetMarkerSize(2.0);
    zcutscan_nbkg_g->SetMarkerColor(49);

    nbkg_zbi_g->SetName(("nbkg_vs_zbi_" + cutname + "_g").c_str());
    nbkg_zbi_g->SetTitle(("nbkg_vs_zbi_" + cutname + "_g;nbkg;zbi").c_str());

    nsig_zbi_g->SetName(("nsig_vs_zbi_" + cutname + "_g").c_str());
    nsig_zbi_g->SetTitle(("nsig_vs_zbi_" + cutname + "_g;nsig;zbi").c_str());
}

double ApOptimizationProcessor::computeTruthSignalShape(double z, double EAp) {
    // EAp in GeV, mass in GeV
    // z, ztarget in mm
    double gamma = EAp / signal_mass_;
    double ctau = 0.08 * pow(1e-4 / eps_, 2) * 0.1 / signal_mass_;  // mm
    return exp(-(z - ztarget_) / (gamma * ctau)) / (gamma * ctau);
}

double ApOptimizationProcessor::computePromptYield(TH1D* h_mass_data_rad_cuts, double bin_width) {
    double Nbin =
        h_mass_data_rad_cuts->GetBinContent(h_mass_data_rad_cuts->FindBin(signal_mass_));  // content at signal mass
    double alpha = 1 / 137.0;
    double factors = 3 * TMath::Pi() * pow(eps_, 2) / (2 * alpha);

    return radFrac_ * factors * Nbin * signal_mass_ / bin_width;
}

double ApOptimizationProcessor::computeDisplacedYield(TH1D* h_mass_data_rad_cuts, TH1D* h_chi_eff, double EAp,
                                                      double bin_width) {
    double expected_signal = 0.0;
    for (int i = 1; i <= h_chi_eff->GetNbinsX(); i++) {
        double z = h_chi_eff->GetBinCenter(i);
        double Ntruth = computeTruthSignalShape(z, EAp);
        double eff = h_chi_eff->GetBinContent(i) * f_xi_eff_->Eval(z);
        expected_signal += eff * Ntruth;
    }
    // std::cout << "Integral: " << expected_signal << std::endl;
    expected_signal *= computePromptYield(h_mass_data_rad_cuts, bin_width);
    // std::cout << "Expected Displaced Signal Yield: " << expected_signal << std::endl;
    return expected_signal;
}

std::vector<double> ApOptimizationProcessor::fitZBkgTail(RDF::RResultPtr<TH1D> h_bkg, std::string fitname,
                                                         bool doGausAndTail) {
    std::cout << "[ApOptimizationProcessor]::fitZBkgTail " << fitname << std::endl;
    std::vector<double> fit_params;

    if (!h_bkg || h_bkg->GetEntries() == 0) {
        std::cerr << "Error: h_bkg is null or empty!" << std::endl;
        return fit_params;
    }

    if (doGausAndTail) {
        TF1* fit_core = new TF1(("fit_core_" + fitname).c_str(), "gaus", -10, 10);
        int fit_status = h_bkg->Fit(fit_core, "RQ");
        if (fit_status != 0) {
            std::cerr << "Error: Gaussian fit failed!" << std::endl;
            return fit_params;
        }
        double scale = fit_core->GetParameter(0);
        double mean = fit_core->GetParameter(1);
        double sigma = fit_core->GetParameter(2);

        double nbr_sigma = 1.;
        TF1* fit_expo_tail = new TF1(("fit_expo_tail_" + fitname).c_str(),
                                     Form("%f*exp( 0.5*[0]**2 - [0]*((x-%f)/%f) )", scale, mean, sigma));
        fit_status = h_bkg->Fit(fit_expo_tail, "Q", "", nbr_sigma * sigma + mean, 20);
        if (fit_status != 0) {
            std::cerr << "Error: Exponential tail fit failed!" << std::endl;
            return fit_params;
        }
        int counter = 0;
        while (abs(fit_expo_tail->GetParameter(0) - nbr_sigma) > 0.01) {
            nbr_sigma = fit_expo_tail->GetParameter(0);
            h_bkg->Fit(fit_expo_tail, "Q", "", nbr_sigma * sigma + mean, 20);
            counter += 1;
            if (counter > 10) {
                break;
            }
        }

        if (debug_) std::cout << "number sigma: " << nbr_sigma << std::endl;

        TF1* fit_expo_tail_final =
            new TF1(("fit_expo_tail_final_" + fitname).c_str(),
                    Form("%f*exp( 0.5*[0]**2 - [0]*((x-%f)/%f) )", scale, mean, sigma), nbr_sigma * sigma + mean, 20);
        h_bkg->Fit(fit_expo_tail_final, "RQ");

        fit_params.push_back(nbr_sigma);
        fit_params.push_back(scale);
        fit_params.push_back(mean);
        fit_params.push_back(sigma);

        fit_core->SetLineColor(kBlack);
        fit_core->SetLineStyle(2);
        fit_expo_tail_final->SetLineColor(kRed);
        fit_expo_tail_final->SetLineStyle(1);
        h_bkg->GetListOfFunctions()->Add(fit_core);
        h_bkg->GetListOfFunctions()->Add(fit_expo_tail_final);

        return fit_params;
    } else {
        TF1* fit_expo_tail = new TF1(("fit_expo_tail_" + fitname).c_str(), "[0]*exp(-[1]*x)", 2, 50);
        int fit_status = h_bkg->Fit(fit_expo_tail, "RQ");
        if (fit_status != 0) {
            std::cerr << "Error: Exponential tail fit failed!" << std::endl;
            return fit_params;
        }
        fit_params.push_back(fit_expo_tail->GetParameter(0));
        fit_params.push_back(fit_expo_tail->GetParameter(1));

        fit_expo_tail->SetLineColor(kRed);
        fit_expo_tail->SetLineStyle(1);
        h_bkg->GetListOfFunctions()->Add(fit_expo_tail);
        return fit_params;
    }
}

double* ApOptimizationProcessor::getQuantileArray(std::pair<double, double> range, int n_quantiles,
                                                  double original_cut) {
    double* quantiles = new double[n_quantiles];
    if (original_cut > -999.) {
        // figure out how to best vary around original cut
    }
    if (range.first < -999.) {
        for (int i = 0; i < n_quantiles; i++) {
            quantiles[i] = 1 - step_size_ * i;
        }
    } else if (range.second > 999.) {
        for (int i = 0; i < n_quantiles; i++) {
            quantiles[i] = step_size_ * i;
        }
    }
    return quantiles;
}

double* ApOptimizationProcessor::getBinsAndLimits(json histo_cfg, std::string varname) {
    for (auto& hist_config : histo_cfg.items()) {
        // check if key contains varname
        if (hist_config.key().find(varname) != std::string::npos) {
            double bins = hist_config.value().at("bins");
            double minX = hist_config.value().at("minX");
            double maxX = hist_config.value().at("maxX");
            return new double[3]{bins, minX, maxX};
        }
    }
    return new double[3]{100, 0, 100};
}

// Get mass resolution in GeV
double ApOptimizationProcessor::getMassResolution(double mass) {
    mass *= 1000.;  // convert to MeV
    if (hit_category_ == "l1l1") {
        return (0.4840 + 0.0421 * mass) / 1000.;
    } else if (hit_category_ == "l1l2") {
        return (0.8124 + 0.0422 * mass) / 1000.;
    } else {
        return 0.005;  // default 5 MeV
    }
}

std::string ApOptimizationProcessor::getHitCategoryCut() {
    if (hit_category_ == "l1l1") {
        return "(eleL1 && eleL2 && posL1 && posL2)";
    } else if (hit_category_ == "l1l2") {
        return "((eleL1 && eleL2 && posL2 && !posL1) || (posL1 && posL2 && eleL2 && !eleL1))";
    } else if (hit_category_ == "l2l2") {
        return "(eleL2 && !eleL1 && posL2 && !posL1)";
    } else {
        return "(1)";  // no cut
    }
}

DECLARE_PROCESSOR(ApOptimizationProcessor);
