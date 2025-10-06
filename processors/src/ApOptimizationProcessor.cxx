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
        signalMCAnaTreename_ = parameters.getString("signalMCAnaTreename", signalMCAnaTreename_);
        signal_pdgid_ = parameters.getString("signal_pdgid", signal_pdgid_);
        signal_sf_ = parameters.getDouble("signal_sf", signal_sf_);
        signal_mass_ = parameters.getDouble("signal_mass", signal_mass_);
        mass_window_nsigma_ = parameters.getDouble("mass_window_nsigma", mass_window_nsigma_);
        signalVtxMCSelection_ = parameters.getString("signalVtxMCSelection", signalVtxMCSelection_);
        eps_ = pow(10, parameters.getInteger("eps", eps_));  // get logeps and convert to eps

        // New Variables
        // new_variables_ = parameters.getVString("add_new_variables", new_variables_);
        // new_variable_params_ = parameters.getVDouble("new_variable_params", new_variable_params_);

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

    return variables;
}

void ApOptimizationProcessor::initialize(std::string inFilename, std::string outFilename) {
    std::cout << "[ApOptimizationProcessor] Initialize " << inFilename << std::endl;

    opts_.fMode = "UPDATE";
    outFileName_ = outFilename;

    // TODO: write equation to find mass resolution at signal mass
    massResolution_ = 1.0;

    // Define Mass window
    lowMass_ = signal_mass_ - mass_window_nsigma_ * massResolution_;
    highMass_ = signal_mass_ + mass_window_nsigma_ * massResolution_;

    std::cout << "[SimpZBiOptimization]::Mass Window: " << lowMass_ << " - " << highMass_ << std::endl;

    // TODO: Get the signal pretrigger simulated vertex z distribution

    // Read signal ana vertex tuple, and convert to mutable tuple
    std::cout << "[SimpZBiOptimization]::Reading Signal AnaVertex Tuple from file " << signalVtxAnaFilename_.c_str()
              << std::endl;
    signalVtxAnaFile_ = new TFile(signalVtxAnaFilename_.c_str(), "READ");
    signal_tree_ = (TTree*)signalVtxAnaFile_->Get(signalVtxAnaTreename_.c_str());

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
    // testCutHistos_->DefineHistos();

    // Add Test Cut Analysis Histograms necessary for calculating background and signal
    std::cout << "[SimpZBiOptimization]::Initializing Test Cut Analysis Histograms" << std::endl;
    for (range_cut_iter_ it = testCutsPtr_->begin(); it != testCutsPtr_->end(); it++) {
        std::string name = it->first;
        std::string var = testCutsSelector_->getCutVar(name, true);
        std::string drawstring = var + ">>" + name + "_pdf_h(200,0,100)";
        signal_tree_->Draw(drawstring.c_str());
        TH1F* sig_h = (TH1F*)gDirectory->Get((name + "_pdf_h").c_str());
        std::cout << "Entries in signal histogram for " << name << ": " << sig_h->GetEntries() << std::endl;
        testCutHistos_->addHisto1d(sig_h);
        testVarPDFs_[name] = (TH1F*)testCutHistos_->getPDF(name + "_pdf_h");
    }

    signal_pretrig_sim_tree_->Draw("vtx.z >> h_pretrig_signal_vtxz(200, -50, 150)");
    h_pretrig_signal_vtxz_ = (TH1D*)gDirectory->Get("h_pretrig_signal_vtxz");
    h_pretrig_signal_vtxz_->Sumw2();

    // Initialize processor histograms that summarize iterative results
    std::cout << "[SimpZBiOptimization]::Initializing Itearitve Result Histograms" << std::endl;
    processorHistos_ = std::make_shared<ZBiHistos>("zbi_processor");
    processorHistos_->defineZBiCutflowProcessorHistograms();
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

double ApOptimizationProcessor::computeTruthSignalShape(double z, double ztarget, double EAp) {
    // EAp in GeV, mass in GeV
    // z, ztarget in mm
    double gamma = EAp / signal_mass_;
    double ctau = 0.08 * pow(1e-4 / eps_, 2) * 0.1 / signal_mass_;  // mm
    return exp(-(z - ztarget) / (gamma * ctau)) / (gamma * ctau);
}

double ApOptimizationProcessor::computePromptYield(TH1D* h_mass_data_rad_cuts, double bin_width) {
    double Nbin =
        h_mass_data_rad_cuts->GetBinContent(h_mass_data_rad_cuts->FindBin(signal_mass_));  // content at signal mass
    double alpha = 1 / 137.0;
    double factors = 3 * TMath::Pi() * pow(eps_, 2) / (2 * alpha);

    return radFrac_ * factors * Nbin * signal_mass_ / bin_width;
}

double ApOptimizationProcessor::computeDisplacedYield(TH1D* h_mass_data_rad_cuts, TH1D* h_chi_eff, double ztarget,
                                                      double EAp, double bin_width) {
    double expected_signal = 0.0;
    for (int i = 1; i <= h_chi_eff->GetNbinsX(); i++) {
        double z = h_chi_eff->GetBinCenter(i);
        double Ntruth = computeTruthSignalShape(z, ztarget, EAp);
        double eff = h_chi_eff->GetBinContent(i) * f_xi_eff_->Eval(z);
        expected_signal += eff * Ntruth;
    }
    std::cout << "Integral: " << expected_signal << std::endl;
    expected_signal *= computePromptYield(h_mass_data_rad_cuts, bin_width);
    std::cout << "Expected Displaced Signal Yield: " << expected_signal << std::endl;
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

    // Initialize output file
    std::cout << "[SimpZBiOptimization]::Output File: " << outFileName_.c_str() << std::endl;
    outFile_ = new TFile(outFileName_.c_str(), "UPDATE");

    // define fraction of radiative selected signal to simulated signal
    RDF::RInterface<Detail::RDF::RJittedFilter, void> df_signal_ = applyFilter(df_signal_mod_, "psum > 3.0", "radCut");
    RDF::RInterface<Detail::RDF::RJittedFilter, void> df_bkg_ = applyFilter(df_bkg_mod_, "psum > 3.0", "radCut");

    RDF::RResultPtr<TH1D> h_signal_vtxz_rad_selected =
        df_signal_.Histo1D({"h_signal_vtxz_rad_selected", ";vtx_z [mm];Events/0.5mm", 200, -50, 150}, "true_ap.vtx_z_");
    RDF::RResultPtr<TH1D> h_data_mass_rad_selected =
        df_bkg_.Histo1D({"h_data_mass_rad_selected", ";mass [GeV];Events/0.5GeV", 100, 0, 0.4}, "vertex.invM_");

    h_data_mass_rad_selected->Sumw2();
    TH1D* h_data_mass_rad_ = (TH1D*)h_data_mass_rad_selected->Clone("h_data_mass_rad_");

    h_signal_vtxz_rad_selected->Sumw2();
    h_signal_vtxz_rad_ = (TH1D*)h_signal_vtxz_rad_selected->Clone("h_signal_vtxz_rad_");
    TH1D* h_xi_eff = (TH1D*)h_signal_vtxz_rad_selected->Clone("h_xi_eff");
    h_xi_eff->Divide(h_pretrig_signal_vtxz_);

    if (outFile_)
        outFile_->cd();
    else {
        std::cout << "Error: output file not found!" << std::endl;
        return false;
    }
    TDirectory* eff_dir{nullptr};
    std::string eff_folder = "selection_efficiency";
    if (!eff_folder.empty()) {
        eff_dir = outFile_->mkdir(eff_folder.c_str(), "", true);
        eff_dir->cd();
    }

    h_data_mass_rad_selected->Write();
    h_signal_vtxz_rad_selected->Write();
    h_pretrig_signal_vtxz_->Write();
    h_xi_eff->Write();
    // outFile_->Close();

    f_xi_eff_ = new TF1("f_xi_eff_", "exp([0] + [1]*x + [2]*x*x)", 0, 150);
    h_xi_eff->Fit(f_xi_eff_, "SRQ", "", 0, 150);
    double ztarget = -0.5;  // mm
    double xi_eff_0 = f_xi_eff_->Eval(ztarget);
    f_xi_eff_->SetParameters(f_xi_eff_->GetParameter(0) - log(xi_eff_0), f_xi_eff_->GetParameter(1),
                             f_xi_eff_->GetParameter(2));
    auto fit_params = f_xi_eff_->GetParameters();

    // Step_size defines n% of signal distribution to cut in a given variable
    double cutFraction = step_size_;
    if (max_iteration_ > (int)1.0 / cutFraction) max_iteration_ = (int)1.0 / cutFraction;

    // Iteratively cut n% of the signal distribution for a given Test Cut variable
    int n_quantiles_ = max_iteration_;
    // Store result histograms for each Test Cut variable
    std::map<std::string, std::vector<RDF::RResultPtr<TH1D>>> sig_hists;
    std::map<std::string, std::vector<RDF::RResultPtr<TH1D>>> bkg_hists;

    for (range_cut_iter_ it = testCutsPtr_->begin(); it != testCutsPtr_->end(); it++) {
        std::cout << "Getting quantiles for Test Cut variable " << it->first << std::endl;
        std::string cutname = it->first;
        std::string cutvar = testCutsSelector_->getCutVar(cutname);
        std::cout << "Test Cut variable: " << cutvar << std::endl;
        // Get cutvalue that corresponds to cutting n% of signal distribution in cutvar from PDF
        TH1F* pdf = testVarPDFs_[cutname];
        if (!pdf) {
            std::cout << "Error: could not find PDF for Test Cut variable " << cutvar << std::endl;
            continue;
        }
        double* quantiles = new double[n_quantiles_];
        for (int i = 0; i < n_quantiles_; i++) {
            quantiles[i] = 1 - step_size_ * i;
        }
        double* quantile_pos = new double[n_quantiles_];
        pdf->GetQuantiles(n_quantiles_, quantile_pos, quantiles);

        testVarQuantiles_[cutname] = quantile_pos;

        // // Prepare storage for signal and background histograms for this Test Cut
        std::vector<RDF::RResultPtr<TH1D>> sig_h;
        std::vector<RDF::RResultPtr<TH1D>> bkg_h;

        sig_hists.insert(std::make_pair(cutname, sig_h));
        bkg_hists.insert(std::make_pair(cutname, bkg_h));
    }

    for (int iteration = 0; iteration < max_iteration_; iteration++) {
        double cutSignal = (double)iteration * step_size_;

        // TODO: flexible hit requirements -- select L1L1, L1L2, L2L2 category in config
        RDF::RInterface<Detail::RDF::RJittedFilter, void> df_signal_temp =
            applyFilter(df_signal_, "eleL1 && posL1", "L1L1");
        RDF::RInterface<Detail::RDF::RJittedFilter, void> df_bkg_temp = applyFilter(df_bkg_, "eleL1 && posL1", "L1L1");

        // At the start of each iteration, apply persistent cuts and save cut values
        for (range_cut_iter_ it = persistentCutsPtr_->begin(); it != persistentCutsPtr_->end(); it++) {
            std::string cutname = it->first;
            persistentCutsLog_[cutname] = std::make_pair(persistentCutsSelector_->getCutRange(cutname), iteration);
            std::string cutvar = persistentCutsSelector_->getCutVar(cutname);

            std::string filter = cutvar + " > " + std::to_string(persistentCutsSelector_->getCutRange(cutname).first) +
                                 " && " + cutvar + " < " +
                                 std::to_string(persistentCutsSelector_->getCutRange(cutname).second);

            df_signal_temp = applyFilter(df_signal_temp, filter, cutname + "_persistent");
            df_bkg_temp = applyFilter(df_bkg_temp, filter, cutname + "_persistent");
        }

        if (debug_) {
            std::cout << "Applied Persistent Cuts" << std::endl;
            auto filter_names = df_bkg_temp.GetFilterNames();
            for (auto name : filter_names) {
                std::cout << "Applied filter: " << name << std::endl;
            }
        }

        // Loop over each Test Cut. Cut n% of signal distribution in Test Cut variable
        if (debug_) std::cout << "Looping over Signal Test Cuts" << std::endl;

        for (range_cut_iter_ it = testCutsPtr_->begin(); it != testCutsPtr_->end(); it++) {
            std::string cutname = it->first;
            double cutvalue = testVarQuantiles_[cutname][iteration];

            if (testCutsSelector_->getCutRange(cutname).first < -999.) {
                testCutsSelector_->setCutValue(cutname, std::make_pair(-999.9, cutvalue));

            } else if (testCutsSelector_->getCutRange(cutname).second > 999.) {
                testCutsSelector_->setCutValue(cutname, std::make_pair(cutvalue, 999.9));
            }

            if (debug_) {
                std::cout << "Test Cut " << cutname << " " << testCutsSelector_->getCutRange(cutname).first << " "
                          << testCutsSelector_->getCutRange(cutname).second << " cuts " << cutSignal
                          << "% of signal distribution in this variable " << std::endl;
            }
            std::string cutvar = testCutsSelector_->getCutVar(cutname);

            std::string filter = cutvar + " > " + std::to_string(testCutsSelector_->getCutRange(cutname).first) +
                                 " && " + cutvar + " < " +
                                 std::to_string(testCutsSelector_->getCutRange(cutname).second);

            std::cout << "Applying filter: " << filter << std::endl;

            auto c_bkg = applyFilter(df_bkg_temp, filter);
            auto c_sig = applyFilter(df_signal_temp, filter);

            // Only booking the operations here, to minimize looping over entries
            // {hname, ";" + xtitle + ";" + ytitle, bins, minX, maxX}, var)
            sig_hists.at(cutname).push_back(
                c_sig.Histo1D({("h_zvtx_signal_" + cutname + "_" + std::to_string(iteration)).c_str(),
                               ";z_{vtx}/mm;N_{events}/a.u.", 200, -50.0, 150.0},
                              "vertex_z"));
            bkg_hists.at(cutname).push_back(
                c_bkg.Histo1D({("h_zvtx_background_" + cutname + "_" + std::to_string(iteration)).c_str(),
                               ";z_{vtx}/mm;N_{events}/a.u.", 200, -50.0, 150.0},
                              "vertex_z"));
        }

        for (range_cut_iter_ it = testCutsPtr_->begin(); it != testCutsPtr_->end(); it++) {
            std::string cutname = it->first;
            sig_hists.at(cutname).at(iteration)->Sumw2();
            TH1D* h_chi_eff = (TH1D*)sig_hists.at(cutname).at(iteration)->Clone(
                ("h_chi_eff_" + cutname + "_" + std::to_string(iteration)).c_str());
            h_chi_eff->Divide(h_signal_vtxz_rad_);

            double Nsig = computeDisplacedYield(h_data_mass_rad_, h_chi_eff, ztarget, 0.8 * 3.74, 0.5);

            std::cout << "Expected Displaced Signal for Test Cut " << cutname << " at iteration " << iteration
                      << " (cutting " << cutSignal * 100 << "% of signal in this variable): " << Nsig << std::endl;

            bkg_hists.at(cutname).at(iteration)->Sumw2();
            std::string fitname = cutname + "_" + std::to_string(iteration);
            std::vector<double> fit_params_vec = fitZBkgTail(bkg_hists.at(cutname).at(iteration), fitname, false);

            // double z_cut = fit_params_vec.at(2) + fit_params_vec.at(3) / fit_params_vec.at(0) *
            //                                            (fit_params_vec.at(0) * fit_params_vec.at(0) / 2. -
            //                                             Math::log(min_ztail_events_ * fit_params_vec.at(0) /
            //                                                       (fit_params_vec.at(1) * fit_params_vec.at(3))));

            // std::cout << z_cut << std::endl;

            // Calcuate the Binomial Significance ZBi corresponding to each Test Cut
            // Test Cut with maximum ZBi after cutting n% of signal distribution in a given variable is selected
            // Test Cut value is added to list of Persistent Cuts, and is applied to all events in following iterations.

            // // Create Directory for this iteration
            // std::string iteration_subdir = "testCuts_pct_sig_cut_" + std::to_string(cutSignal);
            // TDirectory* dir{nullptr};
            // if (!iteration_subdir.empty()) {
            //     dir = outFile_->mkdir(iteration_subdir.c_str(), "", true);
            // }
            // double best_zbi = -9999.9;
            // double best_zcut = -9999.9;
            // double best_nsig = -9999.9;
            // double best_nbkg = -9999.9;
            // std::string best_cutname;
            // double best_cutvalue;

            // // Loop over Test Cuts
            // if (debug_) std::cout << "Calculate ZBi for each Test Cut " << std::endl;
            // for (cut_iter_ it = testCutsPtr_->begin(); it != testCutsPtr_->end(); it++) {
            //     std::string cutname = it->first;
            //     double cutvalue = testCutsSelector_->getCut(cutname);
            //     int cutid = testCutsSelector_->getCutID(cutname);
            //     if (debug_) {
            //         std::cout << "Calculating ZBi for Test Cut " << cutname << std::endl;
            //         std::cout << "Test Cut ID: " << cutid << " | Test Cut Value: " << cutvalue << std::endl;
            //     }

            //     // Build Background Model (exponential fit to z vtx for bkg), used to estimate nbkg in Signal Region
            //     double start_fit =
            //         500.0 * background_sf_;  // start fit at point where start_fit events are expected in z tail
            //     TF1* bkg_model = (TF1*)testCutHistos_->fitExponentialTail("background_zVtx_" + cutname, start_fit);

            //     // CD to output file to save resulting plots
            //     outFile_->cd();

            //     // Graphs track the performance of a Test Cut as a function of Zcut position
            //     TGraph* zcutscan_zbi_g = new TGraph();
            //     TGraph* zcutscan_nsig_g = new TGraph();
            //     TGraph* zcutscan_nbkg_g = new TGraph();
            //     TGraph* nbkg_zbi_g = new TGraph();
            //     TGraph* nsig_zbi_g = new TGraph();
            //     configureGraphs(zcutscan_zbi_g, zcutscan_nsig_g, zcutscan_nbkg_g, nbkg_zbi_g, nsig_zbi_g, cutname);

            //     TH2F* nsig_zcut_hh = new TH2F(("nsig_v_zcut_zbi_" + cutname + "_hh").c_str(),
            //                                   ("nsig_v_zcut_zbi_" + cutname + "_hh; zcut [mm]; NSig").c_str(), 200,
            //                                   -50.3, 149.7, 3000, 0.0, 300.0);

            //     TH2F* nbkg_zcut_hh = new TH2F(("nbkg_v_zcut_zbi_" + cutname + "_hh").c_str(),
            //                                   ("nbkg_v_zcut_zbi_" + cutname + "_hh; zcut [mm]; Nbkg").c_str(), 200,
            //                                   -50.3, 149.7, 3000, 0.0, 300.0);

            //     // Find maximum position of Zcut --> ZBi calculation requires non-zero background
            //     // Start the Zcut position at the target
            //     double zcut_step = 0.1;
            //     // get minimum zcut position (start of bkg model)
            //     double max_zcut = bkg_model->GetXmin();
            //     // TODO: get highest zvtx from background sample
            //     double endIntegral;

            //     // TODO: find cutoff at which min_ztail_events_ are left in the tail of the background model

            //     // If configuration does not specify scanning Zcut values, use single Zcut position at maximum
            //     position. double min_zcut = bkg_model->GetXmin(); if (!scan_zcut_) min_zcut = max_zcut;

            //     // TODO: Get the signal vtx z histo *before* zcut is applied

            //     outFile_->cd(("testCuts_pct_sig_cut_" + std::to_string(cutSignal)).c_str());

            //     // Scan Zcut position and calculate ZBi
            //     double best_scan_zbi = -999.9;
            //     double best_scan_zcut;
            //     double best_scan_nsig;
            //     double best_scan_nbkg;
            //     if (debug_) std::cout << "Scanning zcut position" << std::endl;
            //     for (double zcut = min_zcut; zcut < (max_zcut + zcut_step); zcut = zcut + zcut_step) {
            //         double Nbkg = bkg_model->Integral(zcut, endIntegral);

            //         // TODO: Get the Signal truth vertex z distribution beyond the reconstructed vertex Zcut
            //         // Binning must match Signal pre-trigger distribution, in order to take Efficiency between them.

            //         // TODO: Calculate expected signal yield Nsig
            //         double Nsig = 0.0;

            //         // scale Nsig
            //         Nsig = Nsig * signal_sf_;

            //         // Round Nsig, Nbkg
            //         Nsig = round(Nsig);
            //         Nbkg = round(Nbkg);

            //         // Calculate ZBi for this Test Cut using this zcut value
            //         double n_on = Nsig + Nbkg;
            //         double tau = 1.0;  // TODO: look up what tau is
            //         double n_off = Nbkg;
            //         double ZBi = calculateZBi(n_on, n_off, tau);
            //         // std::cout << "ZBi before rounding: " << ZBi << std::endl;

            //         // Update Test Cut with best scan values
            //         if (ZBi > best_scan_zbi) {
            //             best_scan_zbi = ZBi;
            //             best_scan_zcut = zcut;
            //             best_scan_nsig = Nsig;
            //             best_scan_nbkg = Nbkg;
            //         }

            //         // Fill TGraphs
            //         zcutscan_zbi_g->SetPoint(zcutscan_zbi_g->GetN(), zcut, ZBi);
            //         zcutscan_nbkg_g->SetPoint(zcutscan_nbkg_g->GetN(), zcut, Nbkg);
            //         zcutscan_nsig_g->SetPoint(zcutscan_nsig_g->GetN(), zcut, Nsig);
            //         nbkg_zbi_g->SetPoint(nbkg_zbi_g->GetN(), Nbkg, ZBi);
            //         nsig_zbi_g->SetPoint(nsig_zbi_g->GetN(), Nsig, ZBi);

            //         nsig_zcut_hh->Fill(zcut, Nsig, ZBi);
            //         nbkg_zcut_hh->Fill(zcut, Nbkg, ZBi);
            //     }

            //     // Write graph of zcut vs zbi for the Test Cut
            //     writeGraph(outFile_, "testCuts_pct_sig_cut_" + std::to_string(cutSignal), zcutscan_zbi_g);
            //     writeGraph(outFile_, "testCuts_pct_sig_cut_" + std::to_string(cutSignal), zcutscan_nbkg_g);
            //     writeGraph(outFile_, "testCuts_pct_sig_cut_" + std::to_string(cutSignal), zcutscan_nsig_g);

            //     // delete TGraph pointers
            //     delete zcutscan_zbi_g;
            //     delete zcutscan_nsig_g;
            //     delete zcutscan_nbkg_g;
            //     delete nbkg_zbi_g;
            //     delete nsig_zbi_g;
            //     delete nsig_zcut_hh;
            //     delete nbkg_zcut_hh;

            //     // Fill Summary Histograms Test Cuts at best zcutscan value
            //     processorHistos_->Fill2DHisto("test_cuts_values_hh", (double)cutSignal, (double)cutid, cutvalue);
            //     processorHistos_->set2DHistoYlabel("test_cuts_values_hh", cutid, cutname);

            //     processorHistos_->Fill2DHisto("test_cuts_ZBi_hh", (double)cutSignal, (double)cutid, best_scan_zbi);
            //     processorHistos_->set2DHistoYlabel("test_cuts_ZBi_hh", cutid, cutname);

            //     processorHistos_->Fill2DHisto("test_cuts_zcut_hh", (double)cutSignal, (double)cutid, best_scan_zcut);
            //     processorHistos_->set2DHistoYlabel("test_cuts_zcut_hh", cutid, cutname);

            //     processorHistos_->Fill2DHisto("test_cuts_nsig_hh", (double)cutSignal, (double)cutid, best_scan_nsig);
            //     processorHistos_->set2DHistoYlabel("test_cuts_nsig_hh", cutid, cutname);

            //     processorHistos_->Fill2DHisto("test_cuts_nbkg_hh", (double)cutSignal, (double)cutid, best_scan_nbkg);
            //     processorHistos_->set2DHistoYlabel("test_cuts_nbkg_hh", cutid, cutname);

            //     // Check if the best cutscan zbi for this Test Cut is the best overall ZBi for all Test Cuts
            //     if (best_scan_zbi > best_zbi) {
            //         best_zbi = best_scan_zbi;
            //         best_cutname = cutname;
            //         best_cutvalue = cutvalue;
            //         best_zcut = best_scan_zcut;
            //         best_nsig = best_scan_nsig;
            //         best_nbkg = best_scan_nbkg;
            //     }

            // }  // END LOOP OVER TEST CUTS

            // if (debug_) {
            //     std::cout << "Iteration " << iteration << " Best Test Cut is " << best_cutname << " " <<
            //     best_cutvalue
            //               << " with ZBi=" << best_zbi << std::endl;
            //     std::cout << "Update persistent cuts list with this best test cut..." << std::endl;
            //     std::cout << "[Persistent Cuts] Before update:" << std::endl;
            //     persistentCutsSelector_->printCuts();
            // }

            // // Find best Test Cut for iteration. Add Test Cut value to Persistent Cuts list
            // processorHistos_->Fill2DHisto("best_test_cut_ZBi_hh", (double)cutSignal, best_zbi,
            //                               (double)testCutsSelector_->getCutID(best_cutname));
            // processorHistos_->Fill1DHisto("best_test_cut_ZBi_h", (double)cutSignal, best_zbi);
            // processorHistos_->Fill1DHisto("best_test_cut_zcut_h", (double)cutSignal, best_zcut);
            // processorHistos_->Fill1DHisto("best_test_cut_nsig_h", (double)cutSignal, best_nsig);
            // processorHistos_->Fill1DHisto("best_test_cut_nbkg_h", (double)cutSignal, best_nbkg);
            // processorHistos_->Fill1DHisto("best_test_cut_id_h", (double)cutSignal,
            //                               (double)testCutsSelector_->getCutID(best_cutname));

            // persistentCutsSelector_->setCutValue(best_cutname, best_cutvalue);
            // if (debug_) {
            //     std::cout << "[Persistent Cuts] After update:" << std::endl;
            //     persistentCutsSelector_->printCuts();
            // }

            // // Write iteration histos
            // signalHistos_->writeHistos(outFile_, "signal_pct_sig_cut_" + std::to_string(cutSignal));
            // bkgHistos_->writeHistos(outFile_, "background_pct_sig_cut_" + std::to_string(cutSignal));
            // if (iteration == max_iteration_ - 1)
            // testCutHistos_->writeHistos(outFile_, "testCuts");
        }
    }

    // TODO: get snapshot of strictest test cuts (last iteration)

    std::cout << "Original number of signal events: " << *og_counts_sig << std::endl;
    std::cout << "Original number of background events: " << *og_counts_bkg << std::endl;
    // outFile_ = new TFile(outFileName_.c_str(), "UPDATE");
    if (outFile_)
        outFile_->cd();
    else {
        std::cout << "Error: output file not found!" << std::endl;
        return false;
    }
    TDirectory* dir{nullptr};
    std::string folder = "test_hists";
    if (!folder.empty()) {
        dir = outFile_->mkdir(folder.c_str(), "", true);
        dir->cd();
    }
    for (range_cut_iter_ it = testCutsPtr_->begin(); it != testCutsPtr_->end(); it++) {
        std::string cutname = it->first;
        for (int i = 0; i < sig_hists.at(cutname).size(); i++) {
            sig_hists.at(cutname)[i]->Write();
            bkg_hists.at(cutname)[i]->Write();

            TH1D* sh = (TH1D*)sig_hists.at(cutname)[i]->Clone();
            sh->SetName((std::string(sh->GetName()) + "_div_bkg").c_str());
            sh->Divide(bkg_hists.at(cutname)[i].GetPtr());
            sh->Write();
        }
    }
    std::cout << df_signal_mod_.GetNRuns() << std::endl;

    // for (auto& h_bkg : h_bkg_zcuts) {
    //     h_bkg->Write();
    // }

    return true;
}

void ApOptimizationProcessor::finalize() {
    std::cout << "[ApOptimizationProcessor] finalize()" << std::endl;

    if (debug_) {
        std::cout << "FINAL LIST OF PERSISTENT CUTS " << std::endl;
        persistentCutsSelector_->printCuts();
    }

    processorHistos_->saveHistos(outFile_);
    // testCutHistos_->writeGraphs(outFile_, "");
    testCutHistos_->saveHistos(outFile_);

    outFile_->Close();
}

bool ApOptimizationProcessor::failPersistentCuts(TTree* tree) {
    bool failCuts = false;
    // for (range_cut_iter_ it = persistentCutsPtr_->begin(); it != persistentCutsPtr_->end(); it++) {
    //     std::string cutname = it->first;
    //     std::string cutvar = persistentCutsSelector_->getCutVar(cutname);
    //     // If no value inside the tuple exists for this cut, do not apply the cut.
    //     double var;
    //     tree->SetBranchAddress(cutvar.c_str(), &var);
    //     if (!tree->GetBranch(cutvar.c_str())) continue;
    // if (!persistentCutsSelector_->passCutGTorLT(cutname, var)) {
    //     failCuts = true;
    //     break;
    // }
    // }

    return failCuts;
}

bool ApOptimizationProcessor::failTestCut(std::string cutname, TTree* tree) {
    // std::string cutvar = testCutsSelector_->getCutVar(cutname);
    // double cutvalue = testCutsSelector_->getCut(cutname);
    // // If cut variable is not found in the list of tuples, do not apply cut
    // double var;
    // tree->SetBranchAddress(cutvar.c_str(), &var);
    // if (!tree->GetBranch(cutvar.c_str())) return false;
    // if (!testCutsSelector_->passCutGTorLT(cutname, var))
    //     return true;
    // else
    //     return false;
    return false;
}

DECLARE_PROCESSOR(ApOptimizationProcessor);
