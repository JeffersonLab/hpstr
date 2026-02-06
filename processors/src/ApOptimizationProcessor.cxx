#include "ApOptimizationProcessor.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

ApOptimizationProcessor::ApOptimizationProcessor(const std::string& name, Process& process)
    : OptimizationProcessor(name, process) {}

ApOptimizationProcessor::~ApOptimizationProcessor() {}

void ApOptimizationProcessor::configure(const ParameterSet& parameters) {
    std::cout << "ApOptimizationProcessor::configure" << std::endl;
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
        min_ztail_events_ = parameters.getDouble("min_ztail_events", min_ztail_events_);
        scan_zcut_ = parameters.getInteger("scan_zcut", scan_zcut_);
        fixed_zcut_ = parameters.getDouble("fixed_zcut", fixed_zcut_);

        // Expected Signal Calculation
        radFrac_ = parameters.getDouble("radFrac", radFrac_);
        hit_category_ = parameters.getString("hit_category", hit_category_);
        ztarget_ = parameters.getDouble("ztarget", ztarget_);
        psum_cut_ = parameters.getDouble("psum_cut", psum_cut_);
    } catch (std::runtime_error& error) {
        std::cout << error.what() << std::endl;
    }
}

void ApOptimizationProcessor::initialize(std::string inFilename, std::string outFilename) {
    std::cout << "ApOptimizationProcessor::initialize with output file " << outFilename << std::endl;

    outFileName_ = outFilename;

    // TODO: write equation to find mass resolution at signal mass
    massResolution_ = getMassResolution(signal_mass_);

    // Define Mass window
    lowMass_ = signal_mass_ - mass_window_nsigma_ * massResolution_;
    highMass_ = signal_mass_ + mass_window_nsigma_ * massResolution_;
    massWindow_ =
        "(vertex.invM_ > " + std::to_string(lowMass_) + " && vertex.invM_ < " + std::to_string(highMass_) + ")";

    // Read signal ana vertex tuple, and convert to mutable tuple
    std::cout << "ApOptimizationProcessor::initialize: Reading Signal AnaVertex Tuple from file "
              << signalVtxAnaFilename_.c_str() << std::endl;
    signalVtxAnaFile_ = new TFile(signalVtxAnaFilename_.c_str(), "READ");
    signal_tree_ = (TTree*)signalVtxAnaFile_->Get(signalVtxAnaTreename_.c_str());

    TFile* signalVtxSubsetAnaFile = new TFile(signalVtxSubsetAnaFilename_.c_str(), "READ");
    TTree* signal_subset_tree = (TTree*)signalVtxSubsetAnaFile->Get(signalVtxAnaTreename_.c_str());

    std::cout << "ApOptimizationProcessor::initialize: Reading Signal MC Tuple from file "
              << signalMCAnaFilename_.c_str() << std::endl;
    signalMCAnaFile_ = new TFile(signalMCAnaFilename_.c_str(), "READ");
    signal_pretrig_sim_tree_ = (TTree*)signalMCAnaFile_->Get(signalMCAnaTreename_.c_str());

    // Read background ana vertex tuple, and convert to mutable tuple
    std::cout << "ApOptimizationProcessor::initialize: Reading Background AnaVertex Tuple from file "
              << bkgVtxAnaFilename_.c_str() << std::endl;
    bkgVtxAnaFile_ = new TFile(bkgVtxAnaFilename_.c_str(), "READ");
    bkg_tree_ = (TTree*)bkgVtxAnaFile_->Get(bkgVtxAnaTreename_.c_str());

    // Initialize Persistent Cut Selector. These cuts are applied to all events.
    std::cout << "ApOptimizationProcessor::initialize: Initializing Set of Persistent Cuts" << std::endl;
    persistentCutsSelector_ = new TreeCutSelector("persistentCuts", cuts_cfgFile_);
    persistentCutsSelector_->LoadSelection();
    persistentCutsPtr_ = persistentCutsSelector_->getPointerToCuts();

    // initalize Test Cuts
    std::cout << "ApOptimizationProcessor::initialize: Initializing Set of Test Cuts" << std::endl;
    testCutsSelector_ = new TreeCutSelector("testCuts", cuts_cfgFile_);
    testCutsSelector_->LoadSelection();
    testCutsPtr_ = testCutsSelector_->getPointerToCuts();
    testCutsSelector_->filterCuts(cutVariables_);

    // Initialize signal histograms
    signalHistos_ = std::make_shared<ZBiHistos>("signal");
    signalHistos_->debugMode(debug_);
    signalHistos_->loadHistoConfig(variableHistCfgFilename_);

    // Initialize background histograms
    bkgHistos_ = std::make_shared<ZBiHistos>("background");
    bkgHistos_->debugMode(debug_);
    bkgHistos_->loadHistoConfig(variableHistCfgFilename_);

    // Initialize Test Cut histograms
    testCutHistos_ = std::make_shared<ZBiHistos>("testCutHistos");
    testCutHistos_->debugMode(debug_);

    // Initialize processor histograms that summarize iterative results
    processorHistos_ = std::make_shared<ZBiHistos>("zbi_processor");

    initialCuts_ = "(psum > " + std::to_string(psum_cut_) + ") && " + massWindow_;

    std::cout << "ApOptimizationProcessor::initialize: Setting up Test Cut PDFs and Quantiles" << std::endl;
    json cut_cfg = signalHistos_->getConfig();
    // Add Test Cut Analysis Histograms necessary for calculating background and signal
    for (cut_iter_ it = testCutsPtr_->begin(); it != testCutsPtr_->end(); it++) {
        std::string cut_name = it->first;
        std::string cut_var = testCutsSelector_->getCutVar(cut_name, true);

        persistentCutStrings_[cut_name] = initialCuts_ + " && " + getHitCategoryCut() + " && " +
                                          persistentCutsSelector_->getFullCutString({cut_name}, true);

        // Initialize graphs to store iterative results for each Test Cut variable
        initializeGraphs(cut_name);

        // Determine pdf and cdf for test cut variable (not meaningful for z0 and y0 variables)
        initializeTestCutPDF(cut_name, cut_var, persistentCutStrings_[cut_name].c_str());

        // Get cutvalue that corresponds to cutting n% of signal distribution in cutvar
        if (cut_name == "pos_z0" || cut_name == "ele_z0" || cut_name == "min_y0") {
            std::cout << "ApOptimizationProcessor::initialize: Setting up z0/y0 cut quantiles for cut " << cut_name
                      << std::endl;

            signal_tree_->Draw(
                (cut_var + ":vertex.getZ() >> h_pdf2d_signal_cut_" + cut_name + "(120, -5, 25, 200, 0, 2)").c_str(),
                persistentCutStrings_[cut_name].c_str());
            TH2F* pdf_signal_cut = (TH2F*)gDirectory->Get(("h_pdf2d_signal_cut_" + cut_name).c_str());

            testZoffsetAlpha_[cut_name] = getZoffsetAlpha(pdf_signal_cut, max_iteration_, 10);

            testVarQuantiles_[cut_name] = new double[max_iteration_];
            for (int i = 0; i < max_iteration_; i++) {
                testVarQuantiles_[cut_name][i] = i * step_size_;
            }
        } else {
            // find initial cut fraction to vary quantile calculation around
            double initial_cut_value;
            if (persistentCutsSelector_->getCutRange(cut_name).first < -999.) {
                initial_cut_value = persistentCutsSelector_->getCutRange(cut_name).second;
            } else if (persistentCutsSelector_->getCutRange(cut_name).second > 999.) {
                initial_cut_value = persistentCutsSelector_->getCutRange(cut_name).first;
            }
            double initial_cut_frac =
                testVarCDFs_[cut_name]->GetBinContent(testVarCDFs_[cut_name]->FindBin(initial_cut_value));

            // quantiles of cut distribution
            double* quantile_pos = new double[max_iteration_];
            double* quantiles =
                getQuantileArray(testCutsSelector_->getCutRange(cut_name), max_iteration_, initial_cut_frac);

            testVarPDFs_[cut_name]->GetQuantiles(max_iteration_, quantile_pos, quantiles);
            testVarQuantiles_[cut_name] = quantiles;
            testVarQuantilePos_[cut_name] = quantile_pos;
        }
    }

    // Calculate signal vertex efficiency vs vertex z using subset of signal MC with truth information
    determineSignalVertexEfficiencyXi(signal_subset_tree);

    // Save data mass distribution after initial cuts for signal estimation
    std::string mass_bin_str = "(" + std::to_string((int)massbins_[0]) + "," + std::to_string(massbins_[1]) + "," +
                               std::to_string(massbins_[2]) + ")";
    bkg_tree_->Draw(("vertex.invM_ >> h_data_mass" + mass_bin_str).c_str(), initialCuts_.c_str());
    h_data_mass_rad_ = (TH1D*)gDirectory->Get("h_data_mass");
    h_data_mass_rad_->SetTitle((";m_{inv} [GeV];Events/" + std::to_string(mass_bin_width_) + "GeV").c_str());

    // apply initial cuts to background and keep for record
    bkg_tree_->Draw(("vertex.getZ() >> h_data_vtxz_rad" + zbin_str_).c_str(), initialCuts_.c_str());
    auto h_data_vtxz_rad_ = (TH1D*)gDirectory->Get("h_data_vtxz_rad");
    h_data_vtxz_rad_->SetTitle((";z_{vtx} [mm];Events/" + std::to_string(z_bin_width_) + "mm").c_str());

    bkgHistos_->addHisto1d(h_data_vtxz_rad_);
}

bool ApOptimizationProcessor::process() {
    std::cout << "ApOptimizationProcessor::process: Start of processing events" << std::endl;

    auto t1 = std::chrono::high_resolution_clock::now();

    for (cut_iter_ it = testCutsPtr_->begin(); it != testCutsPtr_->end(); it++) {
        std::string cutname = it->first;
        std::string cutvar = testCutsSelector_->getCutVar(cutname, true);

        // iteratively cut n% of the signal distribution for a given Test Cut variable
        for (int iteration = 0; iteration < max_iteration_; iteration++) {
            double cutvalue;
            double cutfraction = testVarQuantiles_[cutname][iteration];
            std::string filter = "";
            if (cutname == "pos_z0" || cutname == "ele_z0" || cutname == "min_y0") {
                double zoffset = testZoffsetAlpha_[cutname][iteration].first;  // mm
                double alpha = testZoffsetAlpha_[cutname][iteration].second;   // rad
                filter = "vertex.getZ() * " + std::to_string(alpha) + " - abs(" + cutvar + ") < " +
                         std::to_string(alpha) + " * " + std::to_string(zoffset);
            } else {
                cutvalue = testVarQuantilePos_[cutname][iteration];
                if (testCutsSelector_->getCutRange(cutname).first < -999.) {
                    filter = cutvar + " < " + std::to_string(cutvalue);
                } else if (testCutsSelector_->getCutRange(cutname).second > 999.) {
                    filter = cutvar + " > " + std::to_string(cutvalue);
                }
            }

            if (debug_) std::cout << "Applying Test Cut " << filter << " at iteration " << iteration << std::endl;

            std::string tight_cuts = persistentCutStrings_[cutname] + " && " + filter;

            bkg_tree_->Draw(
                ("vertex.getZ() >> h_bkg_vtxz_cut_" + cutname + "_" + std::to_string(iteration) + zbin_str_).c_str(),
                tight_cuts.c_str());
            auto h_bkg_vtxz_cut =
                (TH1D*)gDirectory->Get(("h_bkg_vtxz_cut_" + cutname + "_" + std::to_string(iteration)).c_str());
            h_bkg_vtxz_cut->SetTitle(
                (";z_{vtx} [mm];Background Events/" + std::to_string(z_bin_width_) + "mm").c_str());
            h_bkg_vtxz_cut->Sumw2();
            bkgHistos_->addHisto1d(h_bkg_vtxz_cut);

            std::vector<double> zcuts;
            if (fixed_zcut_ > -99.0) {
                // use fixed zcut from config
                zcuts.push_back(fixed_zcut_);
            } else {
                // find zcuts corresponding to different expected background levels
                zcuts = findZcut(h_bkg_vtxz_cut, true, iteration);
            }

            double Nsig_for_best_zcut = 0.0;
            double Nbkg_for_best_zcut = 0.0;
            double best_z_cut = 0.0;
            double ZBi_for_best_zcut = -99.0;

            for (double zcut : zcuts) {
                if (debug_) std::cout << "Using zcut = " << zcut << " mm" << std::endl;

                // Apply zcut to signal and background and store vertex z histograms after cut
                std::string zcut_filter = tight_cuts + " && (vertex.getZ() > " + std::to_string(zcut) + ")";

                signal_tree_->Draw(
                    ("vertex.getZ() >> h_signal_vtxz_cut_" + cutname + "_" + std::to_string(iteration) + zbin_str_)
                        .c_str(),
                    zcut_filter.c_str());
                auto h_signal_vtxz_cut =
                    (TH1D*)gDirectory->Get(("h_signal_vtxz_cut_" + cutname + "_" + std::to_string(iteration)).c_str());
                h_signal_vtxz_cut->SetTitle(
                    (";z_{vtx} [mm];Signal Events/" + std::to_string(z_bin_width_) + "mm").c_str());
                h_signal_vtxz_cut->Sumw2();
                signalHistos_->addHisto1d(h_signal_vtxz_cut);

                TH1F* h_chi =
                    (TH1F*)h_signal_vtxz_cut->Clone(("h_chi_" + cutname + "_" + std::to_string(iteration)).c_str());
                h_chi->Divide(h_signal_vtxz_rad_);
                processorHistos_->addHisto1d(h_chi);

                double Nbkg = h_bkg_vtxz_cut->Integral(h_bkg_vtxz_cut->FindBin(zcut), h_bkg_vtxz_cut->GetNbinsX());
                double Nsig = computeDisplacedYield(h_chi, 0.9 * 3.74);
                double zbi = calculateZBi((Nsig + Nbkg), Nbkg, 1.0);

                if (debug_)
                    std::cout << "Expected Signal: " << Nsig << ", Expected Background: " << Nbkg << ", ZBi: " << zbi
                              << std::endl;

                if (zbi > ZBi_for_best_zcut) {
                    ZBi_for_best_zcut = zbi;
                    best_z_cut = zcut;
                    Nsig_for_best_zcut = Nsig;
                    Nbkg_for_best_zcut = Nbkg;
                }
            }

            if (cutname == "pos_z0" || cutname == "ele_z0" || cutname == "min_y0") {
                double zoffset = testZoffsetAlpha_[cutname][iteration].first;  // mm
                double alpha = testZoffsetAlpha_[cutname][iteration].second;   // rad

                processorHistos_->getGraph("g_nsig_vs_" + cutname)->AddPoint(iteration, Nsig_for_best_zcut);
                processorHistos_->getGraph("g_nbkg_vs_" + cutname)->AddPoint(iteration, Nbkg_for_best_zcut);
                processorHistos_->getGraph("g_zcut_vs_" + cutname)->AddPoint(iteration, best_z_cut);
                processorHistos_->getGraph("g_zbi_vs_" + cutname)->AddPoint(iteration, ZBi_for_best_zcut);
                processorHistos_->getGraph("g_cut_frac_vs_" + cutname)->AddPoint(iteration, cutfraction);
                processorHistos_->getGraph("g_zoffset_vs_" + cutname)->AddPoint(iteration, zoffset);
                processorHistos_->getGraph("g_alpha_vs_" + cutname)->AddPoint(iteration, alpha);
            } else {
                processorHistos_->getGraph("g_nsig_vs_" + cutname)->AddPoint(cutvalue, Nsig_for_best_zcut);
                processorHistos_->getGraph("g_nbkg_vs_" + cutname)->AddPoint(cutvalue, Nbkg_for_best_zcut);
                processorHistos_->getGraph("g_zcut_vs_" + cutname)->AddPoint(cutvalue, best_z_cut);
                processorHistos_->getGraph("g_zbi_vs_" + cutname)->AddPoint(cutvalue, ZBi_for_best_zcut);
                processorHistos_->getGraph("g_cut_frac_vs_" + cutname)->AddPoint(cutvalue, cutfraction);
            }
        }
    }

    auto t2 = std::chrono::high_resolution_clock::now();

    /* Getting number of milliseconds as an integer. */
    auto time_in_sec = std::chrono::duration_cast<std::chrono::seconds>(t2 - t1);

    std::cout << "ApOptimizationProcessor::process: End of processing events. Time taken: " << time_in_sec.count()
              << " s" << std::endl;
    return true;
}

void ApOptimizationProcessor::finalize() {
    std::cout << "ApOptimizationProcessor::finalize" << std::endl;

    // Initialize output file
    outFile_ = new TFile(outFileName_.c_str(), "RECREATE");

    processorHistos_->saveHistos(outFile_, "processorHistos");
    processorHistos_->writeGraphs(outFile_, "processorGraphs");
    testCutHistos_->saveHistos(outFile_, "testCutHistos");
    signalHistos_->saveHistos(outFile_, "signal");
    bkgHistos_->saveHistos(outFile_, "background");

    outFile_->Close();
}

double ApOptimizationProcessor::computeTruthSignalShape(double z, double EAp) {
    // EAp in GeV, mass in GeV
    // z, ztarget in mm
    double gamma = EAp / signal_mass_;
    double ctau = 0.08 * pow(1e-4 / eps_, 2) * 0.1 / signal_mass_;  // mm
    return exp(-(z - ztarget_) / (gamma * ctau)) / (gamma * ctau);
}

double ApOptimizationProcessor::computePromptYield() {
    double Nbin = h_data_mass_rad_->GetBinContent(h_data_mass_rad_->FindBin(signal_mass_));  // content at signal mass
    double alpha = 1 / 137.0;
    double factors = 3 * TMath::Pi() * pow(eps_, 2) / (2 * alpha);

    return radFrac_ * factors * Nbin * signal_mass_ / mass_bin_width_;
}

double ApOptimizationProcessor::computeDisplacedYield(TH1* h_chi_eff, double EAp) {
    double expected_signal = 0.0;
    for (int i = 1; i <= h_chi_eff->GetNbinsX(); i++) {
        double z = h_chi_eff->GetBinCenter(i);
        double Ntruth = computeTruthSignalShape(z, EAp);
        double eff = h_chi_eff->GetBinContent(i) * f_xi_eff_->Eval(z);
        expected_signal += eff * Ntruth;
    }
    expected_signal *= computePromptYield();

    if (debug_) std::cout << "Expected Displaced Signal Yield: " << expected_signal << std::endl;
    return expected_signal;
}

std::vector<double> ApOptimizationProcessor::fitZBkgTail(TH1* h_bkg, std::string fitname) {
    std::vector<double> fit_params;
    TF1* bkg_tail_fit = new TF1(fitname.c_str(), "[0]*exp([2]*(x-[1]))", 0.0, 10.0);
    bkg_tail_fit->SetParameters(1000.0, 30.0, -0.1);
    h_bkg->Fit(bkg_tail_fit, "RQ");
    fit_params.push_back(bkg_tail_fit->GetParameter(0));  // A
    fit_params.push_back(bkg_tail_fit->GetParameter(1));  // z0
    fit_params.push_back(bkg_tail_fit->GetParameter(2));  // slope
    return fit_params;
}

std::vector<double> ApOptimizationProcessor::findZcut(TH1* h_bkg_vtxz_cut, bool fromTailFit, int iteration) {
    std::vector<double> zcuts;
    double A = 0.0;
    double z0 = 0.0;
    double slope = 0.0;

    if (fromTailFit) {
        // fit z tail to find zcut corresponding to ztail_events
        std::vector<double> fit_params = fitZBkgTail((TH1*)h_bkg_vtxz_cut, "bkg_tail_fit" + std::to_string(iteration));
        A = fit_params[0];
        z0 = fit_params[1];
        slope = fit_params[2];
    }

    for (double ztail_events = min_ztail_events_; ztail_events <= 20; ztail_events += 0.5) {
        if (fromTailFit) {
            // fit z tail to find zcut corresponding to ztail_events
            double zcut = z0 - (log(ztail_events) - log(A)) / slope;
            zcuts.push_back(zcut);
        } else {
            double cumulative_bkg = 0.0;
            for (int bin = h_bkg_vtxz_cut->GetNbinsX(); bin >= min_ztail_events_; bin--) {
                cumulative_bkg += h_bkg_vtxz_cut->GetBinContent(bin);
                if (cumulative_bkg >= 1.0) {
                    double zcut = h_bkg_vtxz_cut->GetBinLowEdge(bin);
                    zcuts.push_back(zcut);
                    break;
                }
            }
        }
        if (!scan_zcut_) {
            break;  // only need one zcut if not scanning
        }
    }

    return zcuts;
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

void ApOptimizationProcessor::initializeTestCutPDF(std::string cut_name, std::string cut_var,
                                                   std::string persistent_cuts) {
    json cut_cfg = signalHistos_->getConfig();
    double* bins_and_limits = getBinsAndLimits(cut_cfg, cut_name);
    std::string drawstring = cut_var + ">>" + cut_name + "_pdf_h(" + std::to_string((int)bins_and_limits[0]) + "," +
                             std::to_string(bins_and_limits[1]) + "," + std::to_string(bins_and_limits[2]) + ")";

    signal_tree_->Draw(drawstring.c_str(), persistent_cuts.c_str());
    // sig_h is used to determine pdf for test cut 'cut_name'
    TH1F* sig_h = (TH1F*)gDirectory->Get((cut_name + "_pdf_h").c_str());
    sig_h->Sumw2();
    sig_h->Scale(1. / sig_h->Integral());
    sig_h->SetTitle(("; " + cut_var + "; normalized units").c_str());
    testCutHistos_->addHisto1d(sig_h);

    delete[] bins_and_limits;

    // actual definition of pdf and cdf
    testVarPDFs_[cut_name] = (TH1F*)testCutHistos_->getPDF(cut_name + "_pdf_h");

    bool forward = testCutsSelector_->getCutRange(cut_name).first < -999 ? false : true;
    testVarCDFs_[cut_name] = (TH1F*)testVarPDFs_[cut_name]->GetCumulative(forward);
    testCutHistos_->addHisto1d(testVarCDFs_[cut_name]);
}

void ApOptimizationProcessor::initializeGraphs(std::string cut_name) {
    TGraphErrors* g_zbi = processorHistos_->configureGraph(("g_zbi_vs_" + cut_name).c_str(), "", "Z_{Bi}");
    processorHistos_->addGraph(g_zbi);

    TGraphErrors* g_nsig = processorHistos_->configureGraph(("g_nsig_vs_" + cut_name).c_str(), "", "N_{sig}");
    processorHistos_->addGraph(g_nsig);

    TGraphErrors* g_nbkg = processorHistos_->configureGraph(("g_nbkg_vs_" + cut_name).c_str(), "", "N_{bkg}");
    processorHistos_->addGraph(g_nbkg);

    TGraphErrors* g_zcut = processorHistos_->configureGraph(("g_zcut_vs_" + cut_name).c_str(), "", "z_{cut}/mm");
    processorHistos_->addGraph(g_zcut);

    TGraphErrors* g_cut_frac =
        processorHistos_->configureGraph(("g_cut_frac_vs_" + cut_name).c_str(), "", "N_{sig}^{cut}/N_{sig}^{original}");
    processorHistos_->addGraph(g_cut_frac);

    // Additional graphs for zoffset and alpha if test cut variable is related to track z0 or y0
    if (cut_name == "pos_z0" || cut_name == "ele_z0" || cut_name == "min_y0") {
        TGraphErrors* g_zoffset =
            processorHistos_->configureGraph(("g_zoffset_vs_" + cut_name).c_str(), "", "z_{offset}/mm");
        processorHistos_->addGraph(g_zoffset);

        TGraphErrors* g_alpha = processorHistos_->configureGraph(("g_alpha_vs_" + cut_name).c_str(), "", "#alpha");
        processorHistos_->addGraph(g_alpha);
    }
}

void ApOptimizationProcessor::determineSignalVertexEfficiencyXi(TTree* signal_subset_tree) {
    // vertex z distribution from generated A' sample
    signal_pretrig_sim_tree_->Draw(("vtx.z >> h_pretrig_signal_vtxz" + zbin_str_).c_str());
    h_pretrig_signal_vtxz_ = (TH1D*)gDirectory->Get("h_pretrig_signal_vtxz");
    h_pretrig_signal_vtxz_->Sumw2();

    // vertex z distribution from reconstructed A' (corresponding to generated sample)
    // apply psum and mass window cuts == initial cuts
    signal_subset_tree->Draw(("true_ap.vtx_z_ >> h_signal_vtxz_rad_subset" + zbin_str_).c_str(), initialCuts_.c_str());
    h_signal_vtxz_rad_subset_ = (TH1D*)gDirectory->Get("h_signal_vtxz_rad_subset");
    h_signal_vtxz_rad_subset_->SetTitle((";z_{truth} [mm];Events/" + std::to_string(z_bin_width_) + "mm").c_str());
    h_signal_vtxz_rad_subset_->Sumw2();

    TH1D* h_xi_eff = (TH1D*)h_signal_vtxz_rad_subset_->Clone("h_xi_eff");
    h_xi_eff->Divide(h_pretrig_signal_vtxz_);

    f_xi_eff_ = new TF1("f_xi_eff_", "exp([0] + [1]*x + [2]*x*x)", 0, 150);
    h_xi_eff->Fit(f_xi_eff_, "SRQ", "", 0, 150);

    double xi_eff_0 = f_xi_eff_->Eval(ztarget_);
    f_xi_eff_->SetParameters(f_xi_eff_->GetParameter(0) - log(xi_eff_0), f_xi_eff_->GetParameter(1),
                             f_xi_eff_->GetParameter(2));

    signalHistos_->addHisto1d(h_pretrig_signal_vtxz_);
    signalHistos_->addHisto1d(h_signal_vtxz_rad_subset_);
    signalHistos_->addHisto1d(h_xi_eff);

    // determine h_signal_vtxz_rad_ for full signal sample
    signal_tree_->Draw(("vertex.getZ() >> h_signal_vtxz_rad" + zbin_str_).c_str(), initialCuts_.c_str());
    h_signal_vtxz_rad_ = (TH1D*)gDirectory->Get("h_signal_vtxz_rad");
    h_signal_vtxz_rad_->SetTitle((";z_{vtx} [mm];Events/" + std::to_string(z_bin_width_) + "mm").c_str());
    h_signal_vtxz_rad_->Sumw2();

    signalHistos_->addHisto1d(h_signal_vtxz_rad_);
}

double* ApOptimizationProcessor::getQuantileArray(std::pair<double, double> range, int n_quantiles,
                                                  double initial_cut_frac) {
    double* quantiles = new double[n_quantiles];
    double start_frac = 1.0;

    if (debug_)
        std::cout << "ApOptimizationProcessor::getQuantileArray: Initial cut fraction = " << initial_cut_frac
                  << std::endl;
    if (initial_cut_frac > -999.) {
        // recenter quantiles around initial cut fraction
        start_frac = initial_cut_frac - (n_quantiles / 2) * step_size_;
        if (debug_)
            std::cout << "ApOptimizationProcessor::getQuantileArray: Start at fraction = " << start_frac << std::endl;
        for (int i = 0; i < n_quantiles; i++) {
            if (start_frac + step_size_ * i < 0.)
                quantiles[i] = 0.;
            else if (start_frac + step_size_ * i > 1.)
                quantiles[i] = 1.;
            else
                quantiles[i] = start_frac + step_size_ * i;
        }
        if (range.first < -999.) {
            for (int i = 0; i < n_quantiles; i++) {
                quantiles[i] = 1 - quantiles[i];
            }
        }
    } else {
        if (range.first < -999.) {
            for (int i = 0; i < n_quantiles; i++) {
                quantiles[i] = 1 - step_size_ * i;
            }
        } else if (range.second > 999.) {
            for (int i = 0; i < n_quantiles; i++) {
                quantiles[i] = step_size_ * i;
            }
        }
    }
    return quantiles;
}

std::vector<std::pair<double, double>> ApOptimizationProcessor::getZoffsetAlpha(TH2F* h_y0_vs_z, int n_quantiles,
                                                                                int nbins) {
    double* quantiles = getQuantileArray(std::make_pair(0, 9999.9), n_quantiles);
    std::vector<std::pair<double, double>> zoffset_alpha;
    processorHistos_->addHisto2d(h_y0_vs_z);
    for (int q = 0; q < n_quantiles; q++) {
        TGraphErrors* g_y0_vs_z =
            processorHistos_->configureGraph(("g_y0_vs_z_q" + std::to_string(q)).c_str(), "z_{vtx} [mm]", "y_{0} [mm]");
        processorHistos_->addGraph(g_y0_vs_z);
    }
    if (debug_)
        std::cout << "ApOptimizationProcessor::getZoffsetAlpha: Number of z bins = "
                  << int(h_y0_vs_z->GetNbinsX() / nbins) << std::endl;

    for (int i = 1; i <= int(h_y0_vs_z->GetNbinsX() / nbins); i++) {
        auto h_proj = h_y0_vs_z->ProjectionY(("h_y0_vs_z_py_" + std::to_string(i)).c_str(), nbins * i, nbins * (i + 1));
        processorHistos_->addHisto1d(h_proj);

        if (h_proj->GetEntries() < 50) continue;

        double* quantile_pos = new double[n_quantiles];
        h_proj->GetQuantiles(n_quantiles, quantile_pos, quantiles);
        for (int q = 0; q < n_quantiles; q++) {
            processorHistos_->getGraph(("g_y0_vs_z_q" + std::to_string(q)).c_str())
                ->AddPoint((double)((TAxis*)h_y0_vs_z->GetXaxis())->GetBinCenter(int(nbins * (i + 0.5))),
                           quantile_pos[q]);
            double error = 0.;

            processorHistos_->getGraph(("g_y0_vs_z_q" + std::to_string(q)).c_str())->SetPointError(q, 0.0, error);
        }
    }
    for (int q = 0; q < n_quantiles; q++) {
        if (q == 0) {
            zoffset_alpha.push_back(std::make_pair(0.0, 0.0));
            continue;
        }
        TF1* f1 = new TF1(("f1_y0_vs_z_q" + std::to_string(q)).c_str(), "[0] + [1]*x", -0.5, 15);
        processorHistos_->getGraph(("g_y0_vs_z_q" + std::to_string(q)).c_str())->Fit(f1, "QRS");

        double zoffset = -f1->GetParameter(0) / f1->GetParameter(1);
        zoffset_alpha.push_back(std::make_pair(zoffset, f1->GetParameter(1)));
        if (debug_)
            std::cout << "Quantile " << q << " : zoffset = " << zoffset << " alpha = " << f1->GetParameter(1)
                      << std::endl;
    }
    return zoffset_alpha;
}

DECLARE_PROCESSOR(ApOptimizationProcessor);
