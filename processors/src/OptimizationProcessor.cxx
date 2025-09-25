/**
 * @file OptimizationProcessor.cxx
 * @author Sarah Gaiser, based on ZBiProcessor by Alic Spellman
 * @brief Processor to optimize cuts based on ZBi significance calculation.
 */

#include "OptimizationProcessor.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

OptimizationProcessor::OptimizationProcessor(const std::string& name, Process& process) : Processor(name, process) {}

OptimizationProcessor::~OptimizationProcessor() {}

void OptimizationProcessor::configure(const ParameterSet& parameters) {
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

        // New Variables //
        new_variables_ = parameters.getVString("add_new_variables", new_variables_);
        new_variable_params_ = parameters.getVDouble("new_variable_params", new_variable_params_);

        max_iteration_ = parameters.getInteger("max_iteration", max_iteration_);
        step_size_ = parameters.getDouble("step_size", step_size_);
    } catch (std::runtime_error& error) {
        std::cout << error.what() << std::endl;
    }
}

void OptimizationProcessor::initialize(std::string inFilename, std::string outFilename) {
    std::cout << "[OptimizationProcessor] Initialize " << inFilename << std::endl;
    // create output file
    outFile_ = new TFile(outFilename.c_str(), "RECREATE");

    // Define Mass window
    lowMass_ = signal_mass_ - mass_window_nsigma_ * massResolution_;
    highMass_ = signal_mass_ + mass_window_nsigma_ * massResolution_;

    std::cout << "[OptimizationProcessor]::Mass Window: " << lowMass_ << " - " << highMass_ << std::endl;

    // Read signal ana vertex tuple, and convert to mutable tuple
    signalVtxAnaFile_ = new TFile(signalVtxAnaFilename_.c_str(), "READ");

    // Initialize Persistent Cut Selector. These cuts are applied to all events.
    // Persistent Cut values are updated each iteration with the value of the best performing Test Cut in
    // that iteration.
    std::cout << "[OptimizationProcessor]::Initializing Set of Persistent Cuts" << std::endl;
    persistentCutsSelector_ = new IterativeCutSelector("persistentCuts", cuts_cfgFile_);
    persistentCutsSelector_->LoadSelection();
    persistentCutsPtr_ = persistentCutsSelector_->getPointerToCuts();
    std::cout << "Persistent Cuts: " << std::endl;
    persistentCutsSelector_->printCuts();

    // Initialize Test Cuts
    std::cout << "[OptimizationProcessor]::Initializing Set of Test Cuts" << std::endl;
    testCutsSelector_ = new IterativeCutSelector("testCuts", cuts_cfgFile_);
    testCutsSelector_->LoadSelection();
    testCutsPtr_ = testCutsSelector_->getPointerToCuts();
    testCutsSelector_->filterCuts(cutVariables_);
    std::cout << "Test Cuts: " << std::endl;
    testCutsSelector_->printCuts();

    // Initialize signal histograms
    std::cout << "[OptimizationProcessor]::Initializing Signal Variable Histograms" << std::endl;
    signalHistos_ = std::make_shared<ZBiHistos>("signal");
    signalHistos_->debugMode(debug_);
    signalHistos_->loadHistoConfig(variableHistCfgFilename_);
    signalHistos_->DefineHistos();

    // Initialize background histograms
    std::cout << "[OptimizationProcessor]::Initializing Background Variable Histograms" << std::endl;
    bkgHistos_ = std::make_shared<ZBiHistos>("background");
    bkgHistos_->debugMode(debug_);
    bkgHistos_->loadHistoConfig(variableHistCfgFilename_);
    bkgHistos_->DefineHistos();

    // Initialize Test Cut histograms
    std::cout << "[OptimizationProcessor]::Initializing Test Cut Variable Histograms" << std::endl;
    testCutHistos_ = std::make_shared<ZBiHistos>("testCutHistos");
    testCutHistos_->debugMode(debug_);
    testCutHistos_->DefineHistos();

    // Initialize processor histograms that summarize iterative results
    std::cout << "[OptimizationProcessor]::Initializing Iterative Result Histograms" << std::endl;
    processorHistos_ = std::make_shared<ZBiHistos>("zbi_processor");
    processorHistos_->defineZBiCutflowProcessorHistograms();

    // Write initial variable histograms for signal and background
    std::cout << "[OptimizationProcessor]::Writing Initial Histograms" << std::endl;
    signalHistos_->writeHistos(outFile_, "initial_signal");
    bkgHistos_->writeHistos(outFile_, "initial_background");
}

bool OptimizationProcessor::process() {
    std::cout << "[OptimizationProcessor]::process()" << std::endl;
    return true;
}

void OptimizationProcessor::finalize() {
    std::cout << "[OptimizationProcessor]::finalize()" << std::endl;

    if (debug_) {
        std::cout << "FINAL LIST OF PERSISTENT CUTS " << std::endl;
        persistentCutsSelector_->printCuts();
    }

    processorHistos_->saveHistos(outFile_);
    testCutHistos_->writeGraphs(outFile_, "testCutGraphs");
    outFile_->Close();
    if (signalVtxAnaFile_) signalVtxAnaFile_->Close();
}

double OptimizationProcessor::calculateZBi(double n_on, double n_off, double tau) {
    double P_Bi = TMath::BetaIncomplete(1. / (1. + tau), n_on, n_off + 1);
    std::cout << "P_Bi: " << P_Bi << std::endl;
    double Z_Bi = std::pow(2, 0.5) * TMath::ErfInverse(1 - 2 * P_Bi);
    std::cout << "Z_Bi: " << Z_Bi << std::endl;
    return Z_Bi;
}

void OptimizationProcessor::writeTH1F(TFile* outF, std::string folder, TH1F* h) {
    if (outF) outF->cd();
    TDirectory* dir{nullptr};
    if (!folder.empty()) {
        dir = outF->mkdir(folder.c_str(), "", true);
        dir->cd();
    }
    h->Write();
}

void OptimizationProcessor::writeGraph(TFile* outF, std::string folder, TGraph* g) {
    if (outF) outF->cd();
    TDirectory* dir{nullptr};
    if (!folder.empty()) {
        dir = outF->mkdir(folder.c_str(), "", true);
        dir->cd();
    }
    g->Write();
}

DECLARE_PROCESSOR(OptimizationProcessor);
