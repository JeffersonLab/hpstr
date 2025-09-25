#include "ZBiHistos.h"
#include <sstream>
#include "TMath.h"

ZBiHistos::ZBiHistos()
{
}

ZBiHistos::~ZBiHistos()
{
}

ZBiHistos::ZBiHistos(const std::string &inputName)
{
    m_name = inputName;
}

void ZBiHistos::addHisto1d(std::string histoname, std::string xtitle, int nbinsX, float xmin, float xmax)
{
    histos1d[m_name + "_" + histoname] = plot1D(m_name + "_" + histoname, xtitle, nbinsX, xmin, xmax);
}

void ZBiHistos::addHisto2d(std::string histoname, std::string xtitle, int nbinsX, float xmin, float xmax, std::string ytitle, int nbinsY, float ymin, float ymax)
{
    histos2d[m_name + "_" + histoname] = plot2D(m_name + "_" + histoname, xtitle, nbinsX, xmin, xmax, ytitle, nbinsY, ymin, ymax);
}

void ZBiHistos::resetHistograms1d()
{
    for (it1d it = histos1d.begin(); it != histos1d.end(); it++)
    {
        if (it->second != nullptr)
            it->second->Reset();
    }
}

void ZBiHistos::resetHistograms2d()
{
    for (it2d it = histos2d.begin(); it != histos2d.end(); it++)
    {
        if (it->second != nullptr)
            it->second->Reset();
    }
}

double ZBiHistos::integrateHistogram1D(std::string histoname)
{
    int xmax;
    int xmin;
    double integral;

    // Check if histogram exists to integrate
    if (!histos1d.count(histoname))
    {
        std::cout << "[ZBiHistos}::ERROR::NO HISTOGRAM NAMED " << histoname << " FOUND! DEFINE HISTOGRAM IN HISTOCONFIG!" << std::endl;
        integral = -9999.9;
    }

    if (histos1d[histoname] == nullptr)
    {
        std::cout << "ERROR: Histogram for " << histoname << " is NULLPTR" << std::endl;
        return -9999.9;
    }

    else
    {
        xmax = histos1d[histoname]->FindLastBinAbove(0.0);
        xmin = histos1d[histoname]->FindFirstBinAbove(0.0);
        // integral = histos1d[histoname]->Integral(0,histos1d[histoname]->GetNbinsX()+1);
        integral = histos1d[histoname]->Integral(xmin, xmax);
    }
    return integral;
}

double ZBiHistos::cutFractionOfSignalVariable(std::string cutvariable, bool isCutGreaterThan, double cutFraction, double initialIntegral)
{

    TH1F *histo = histos1d[m_name + "_" + cutvariable + "_h"];
    int xmax = histo->FindLastBinAbove(0.0);
    int xmin = histo->FindFirstBinAbove(0.0);

    if (debug_)
    {
        std::cout << "Initial Integral: " << initialIntegral << std::endl;
        std::cout << "Cut variable: " << cutvariable << std::endl;
        std::cout << "Cut fraction: " << cutFraction << std::endl;
    }

    double cutvalue;
    if (isCutGreaterThan)
    {
        cutvalue = histo->GetXaxis()->GetBinLowEdge(xmin);
        while (histo->Integral(xmin, xmax) > initialIntegral * (1.0 - cutFraction))
        {
            xmin = xmin + 1;
            cutvalue = histo->GetXaxis()->GetBinLowEdge(xmin);
        }
    }
    else
    {
        cutvalue = histo->GetXaxis()->GetBinUpEdge(xmax);
        while (histo->Integral(xmin, xmax) > initialIntegral * (1.0 - cutFraction))
        {
            xmax = xmax - 1;
            cutvalue = histo->GetXaxis()->GetBinUpEdge(xmax);
        }
    }

    return cutvalue;
}

void ZBiHistos::defineZBiCutflowProcessorHistograms()
{
    // These histograms are used specifically for tracking the ZBitCutflowProcessor iterative process
    addHisto2d("persistent_cuts_hh", "pct_sig_cut", 1000, -0.5, 99.5, "cut_id", 50, 0.5, 50.5);
    addHisto2d("test_cuts_ZBi_hh", "pct_sig_cut", 1000, -0.5, 99.5, "cut_id", 50, 0.5, 50.5);
    addHisto2d("test_cuts_values_hh", "pct_sig_cut", 1000, -0.5, 99.5, "cut_id", 50, 0.5, 50.5);
    addHisto2d("test_cuts_zcut_hh", "pct_sig_cut", 1000, -0.5, 99.5, "zcut", 50, 0.5, 50.5);
    addHisto2d("test_cuts_nbkg_hh", "pct_sig_cut", 1000, -0.5, 99.5, "zcut", 50, 0.5, 50.5);
    addHisto2d("test_cuts_nsig_hh", "pct_sig_cut", 1000, -0.5, 99.5, "zcut", 50, 0.5, 50.5);
    addHisto2d("best_test_cut_ZBi_hh", "pct_sig_cut", 1000, -0.5, 99.5, "ZBi", 2000, 0.0, 20.0);

    addHisto1d("best_test_cut_ZBi_h", "pct_sig_cut", 1000, -0.5, 99.5);
    addHisto1d("best_test_cut_zcut_h", "pct_sig_cut", 1000, -0.5, 99.5);
    addHisto1d("best_test_cut_nsig_h", "pct_sig_cut", 1000, -0.5, 99.5);
    addHisto1d("best_test_cut_nbkg_h", "pct_sig_cut", 1000, -0.5, 99.5);
    addHisto1d("best_test_cut_id_h", "pct_sig_cut", 1000, -0.5, 99.5);
}

void ZBiHistos::set2DHistoYlabel(std::string histoName, int ybin, std::string ylabel)
{
    histos2d[m_name + "_" + histoName]->GetYaxis()->SetBinLabel(ybin, ylabel.c_str());
}

std::vector<double> ZBiHistos::defineImpactParameterCut(double alpha)
{
    // alpha defines the % of signal we allow to be cut. Default is 15%
    TH2F *hh = (TH2F *)histos2d[m_name + "_z0_v_recon_z_hh"];
    addHisto1d("impact_parameter_up_h", "recon_z [mm]", 450, -20.0, 70.0);
    addHisto1d("impact_parameter_down_h", "recon_z [mm]", 450, -20.0, 70.0);
    TH1F *up_h = (TH1F *)histos1d[m_name + "_impact_parameter_up_h"];
    TH1F *down_h = (TH1F *)histos1d[m_name + "_impact_parameter_down_h"];

    for (int i = 0; i < hh->GetNbinsX(); i++)
    {

        TH1F *projy = (TH1F *)hh->ProjectionY(("projy_bin_" + std::to_string(i + 1)).c_str(), i + 1, i + 1);
        if (projy->GetEntries() < 1)
            continue;

        // Impact param for z0 > 0
        int start_bin = projy->FindBin(0.0);
        int end_bin = projy->FindLastBinAbove(0.0);
        double refIntegral = projy->Integral(start_bin, end_bin);
        int cutz0_bin = start_bin;
        double testIntegral = refIntegral;
        while (testIntegral > (1.0 - alpha) * refIntegral && cutz0_bin < end_bin - 1)
        {
            cutz0_bin = cutz0_bin + 1;
            testIntegral = projy->Integral(cutz0_bin, end_bin);
        }

        double cutz0_up = projy->GetXaxis()->GetBinLowEdge(cutz0_bin);
        for (int j = 0; j < (int)refIntegral; j++)
        {
            up_h->Fill(hh->GetXaxis()->GetBinCenter(i + 1), cutz0_up / refIntegral);
        }

        // impact param for z0 < 0
        end_bin = projy->FindFirstBinAbove(0.0);
        start_bin = projy->FindBin(0.0);
        refIntegral = projy->Integral(end_bin, start_bin);
        cutz0_bin = start_bin;
        testIntegral = refIntegral;
        while (testIntegral > (1.0 - alpha) * refIntegral && cutz0_bin > end_bin + 1)
        {
            cutz0_bin = cutz0_bin - 1;
            testIntegral = projy->Integral(end_bin, cutz0_bin);
        }

        double cutz0_down = projy->GetXaxis()->GetBinUpEdge(cutz0_bin);
        for (int j = 0; j < (int)refIntegral; j++)
        {
            down_h->Fill(hh->GetXaxis()->GetBinCenter(i + 1), cutz0_down / refIntegral);
        }
    }

    TF1 *fitFunc = new TF1("linear_fit", "[0]*(x-[1])", 5.0, 70.0);
    TFitResultPtr fitResult = (TFitResultPtr)up_h->Fit("linear_fit", "QS", "", 5.0, 70.0);
    fitFunc->Draw();
    double m_p = fitResult->GetParams()[0];
    double a_p = fitResult->GetParams()[1];

    fitResult = (TFitResultPtr)down_h->Fit("linear_fit", "QS", "", 5.0, 70.0);
    fitFunc->Draw();
    double m_d = fitResult->GetParams()[0];
    double a_d = fitResult->GetParams()[1];

    // Find the location in z0 where two lines meet
    double diff = 999.9;
    double x = 10.0;
    while (std::abs((m_p * (x - a_p) - (m_d * (x - a_d)))) < diff)
    {
        // while( std::abs((a_p+b_p*x) - (a_d+b_d*x)) < diff ){
        diff = std::abs((m_p * (x - a_p)) - (m_d * (x - a_d)));
        x = x - 0.01;
    }
    double beta = .5 * ((m_p * (x - a_p)) + (m_p * (x - a_d)));
    std::vector<double> params{m_p, a_p, m_d, a_d, beta, x};

    delete fitFunc;
    return params;
}

/*
TF1* ZBiHistos::fitExponentialPlusConst(std::string histogramName, double start_nevents){

    //Get z vertex distribution corresponding to Test Cut
    std::string histoname = m_name+"_"+histogramName+"_h";

    //If histogram is empty, return nullptr
    if(histos1d[histoname]->GetEntries() < 1){
        std::cout << "[ZBiHistos]::WARNING: Background Model is NULL: " <<
            histogramName << " distribution is empty and could not be fit!" << std::endl;
        return nullptr;
    }
    //Start fit where start_nevents are left in tail
    int lastbin = histos1d[histoname]->FindLastBinAbove(0.0);
    int firstbin = lastbin - 1;
    double test_integral = 0.0;
    while(test_integral < start_nevents && histos1d[histoname]->GetBinLowEdge(firstbin) > 0.0){
        test_integral = histos1d[histoname]->Integral(firstbin, lastbin);
        firstbin = firstbin - 1;
    }

    double xmin = histos1d[histoname]->GetBinLowEdge(firstbin);
    double xmax = histos1d[histoname]->GetBinLowEdge(lastbin+1);
    histos1d[histoname]->GetXaxis()->SetRangeUser(xmin,xmax);
    std::cout << "bkg xmin: " << xmin << std::endl;
    std::cout << "bkg xmax: " << xmax << std::endl;

    //Initially fit tail with exponential to seed params
    double best_seed_chi2 = 9999999.9;
    double seed_0;
    double seed_1;
    TF1* fitFunc_seed = new TF1("fitfunc_seed", "[0]*exp([1]*x)", xmin, xmax);
    fitFunc_seed->SetParameter(0, (double)start_nevents);
    fitFunc_seed->SetParameter(1, -0.5);
    TFitResultPtr fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc_seed, "QLSIM", "", xmin,xmax);
    seed_0 = fitResult->Parameter(0);
    seed_1 = fitResult->Parameter(1);
    if(fitResult->Ndf() <= 0){
        best_seed_chi2 = fitResult->Chi2()/fitResult->Ndf();
    }


    std::cout << "First Seed 0: " << seed_0 << std::endl;
    std::cout << "First Seed 1: " << seed_1 << std::endl;
    std::cout << "First Seed Chi2: " << best_seed_chi2 << std::endl;

    TRandom3* ran = new TRandom3();
    ran->SetSeed(0);
    TF1* fitFunc_seed = new TF1("fitfunc_seed", "[0]*exp([1]*x)", xmin, xmax);
    //fitFunc_seed->SetParameter(0, (double)start_nevents);
    fitFunc_seed->SetParameter(0, 10.0);
    fitFunc_seed->SetParameter(1, -0.5);
    TFitResultPtr fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc_seed, "QSIM", "", xmin,xmax);
    std::cout << "Start with seed 0:10 and 1:-0.5" << std::endl;

    if(fitResult->Ndf() > 0){
        best_seed_chi2 = fitResult->Chi2()/fitResult->Ndf();
        seed_0 = fitResult->Parameter(0);
        seed_1 = fitResult->Parameter(1);
    }
    std::cout << "First Seed 0: " << seed_0 << std::endl;
    std::cout << "First Seed 1: " << seed_1 << std::endl;
    std::cout << "First Seed Chi2: " << best_seed_chi2 << std::endl;
    for(int i=0; i < 50; i++){
        //fitFunc = new TF1("fitfunc_seed", "[0]*exp([1]*x)", xmin, xmax);
        //fitFunc_seed->SetParameters(std::abs(ran->Gaus(10.0,2.0)), -std::abs(ran->Uniform(0.1,0.8)));
        fitFunc_seed->SetParameters(std::abs(ran->Uniform(1.0,20.0)), -std::abs(ran->Uniform(0.1,0.8)));
        fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc_seed, "QSIM", "", xmin,xmax);

        if(fitResult->Ndf() <= 0)
            continue;

        std::cout << "Iter Seed 0: " << fitResult->Parameter(0) << std::endl;
        std::cout << "Iter Seed 1: " << fitResult->Parameter(1) << std::endl;
        std::cout << "Iter Seed Chi2: " << fitResult->Chi2()/fitResult->Ndf() << std::endl;

        if(fitResult->Chi2()/fitResult->Ndf() < best_seed_chi2){
            best_seed_chi2 = fitResult->Chi2()/fitResult->Ndf();
            seed_0 = fitResult->Parameter(0);
            seed_1 = fitResult->Parameter(1);
            std::cout << "Update Iter Seed 0: " << seed_0 << std::endl;
            std::cout << "Update Iter Seed 1: " << seed_1 << std::endl;
            std::cout << "Update Iter Seed Chi2: " << best_seed_chi2 << std::endl;
        }
    }
    std::cout << "Seed 0: " << seed_0 << std::endl;
    std::cout << "Seed 1: " << seed_1 << std::endl;
    std::cout << "Seed Chi2: " << best_seed_chi2 << std::endl;

    double best_chi2 = best_seed_chi2;
    double param_2;
    TF1* fitFunc = new TF1("fitfunc", " (x<[2])*([0]*exp([1]*x)) + (x>=[2])*([0]*exp([1]*[2]))", xmin, xmax);
    fitFunc->FixParameter(0, seed_0);
    fitFunc->FixParameter(1, seed_1);
    for(int i=0; i < 1000; i++){
        fitFunc->FixParameter(2, (double)i/10.0);
        fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "QSIM", "", xmin,xmax);

        if(fitResult->Ndf() <= 0)
            continue;

        if(fitResult->Chi2()/fitResult->Ndf() < best_chi2){
            best_chi2 = fitResult->Chi2()/fitResult->Ndf();
            param_2 = fitResult->Parameter(2);
        }
    }

    std::cout << "Final Seed 0: " << seed_0 << std::endl;
    std::cout << "Final Seed 1: " << seed_1 << std::endl;
    std::cout << "param 2: " << param_2 << std::endl;
    std::cout << "Best Chi2: " << best_chi2 << std::endl;

    delete ran;

    if(best_chi2 < best_seed_chi2){
        fitFunc->FixParameter(0, seed_0);
        fitFunc->FixParameter(1, seed_1);
        fitFunc->FixParameter(2, param_2);
        fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "QLSIM", "", xmin,xmax);
        fitFunc->Draw();
        delete fitFunc_seed;
        return fitFunc;
    }
    else{
        fitFunc->FixParameter(0, seed_0);
        fitFunc->FixParameter(1, seed_1);
        fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc_seed, "QLSIM", "", xmin,xmax);
        fitFunc_seed->Draw();
        delete fitFunc;
        return fitFunc_seed;
    }
    if(fitResult->Chi2()/fitResult->Ndf() < best_seed_chi2){
        delete fitFunc_seed;
        fitFunc->Draw();
        return fitFunc;
    }
    else{
        delete fitFunc;
        fitFunc_seed->Draw();
        return fitFunc_seed;
    }
    */

/* Fails too often. Any failure breaks the code...not very robust
TRandom3* ran = new TRandom3();
ran->SetSeed(0);

//Initially fit tail with exponential to seed params
double best_chi2 = 9999999.9;
double seed_0;
double seed_1;
TF1* fitFunc = new TF1("fitfunc", "[0]*exp([1]*x)", xmin, xmax);
TFitResultPtr fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "QLSIM", "", xmin,xmax);
seed_0 = fitResult->Parameter(0);
seed_1 = fitResult->Parameter(1);

for(int i=0; i < 30; i++){
    fitFunc = new TF1("fitfunc", "[0]*exp([1]*x)", xmin, xmax);
    fitFunc->SetParameters(ran->Gaus(seed_0,1), ran->Gaus(seed_1,1));
    fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "QLSIM", "", xmin,xmax);

    if(fitResult->Ndf() <= 0)
        continue;

    if(fitResult->Chi2()/fitResult->Ndf() < best_chi2){
        best_chi2 = fitResult->Chi2()/fitResult->Ndf();
        seed_0 = fitResult->Parameter(0);
        seed_1 = fitResult->Parameter(1);
    }
}

//Exponential plus constant
fitFunc = new TF1("fitfunc", " (x<[2])*([0]*exp([1]*x)) + (x>=[2])*([0]*exp([1]*[2]))", xmin, xmax);

best_chi2 = 9999999.9;
double best_0 = seed_0;
double best_1 = seed_1;
double best_2;

int iteration = 30;
fitFunc->FixParameter(0, seed_0);
fitFunc->FixParameter(1, seed_1);
for(int i=0; i < iteration; i++){
    //fitFunc->SetParameter(0, best_0);
    //fitFunc->SetParameter(1, best_1);
    fitFunc->SetParameter(2, ran->Uniform(50.0));
    fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "QLSIM", "", xmin,xmax);

    if(fitResult->Ndf() <= 0)
        continue;

    if(fitResult->Chi2()/fitResult->Ndf() < best_chi2){
        best_chi2 = fitResult->Chi2()/fitResult->Ndf();
        //best_0 = fitResult->Parameter(0);
        //best_1 = fitResult->Parameter(1);
        best_2 = fitResult->Parameter(2);
    }
}

//fitFunc->SetParameters(best_0, best_1, best_2);
//fitFunc->SetParameters(seed_0, seed_1, best_2);
fitFunc->FixParameter(0, seed_0);
fitFunc->FixParameter(1, seed_1);
fitFunc->FixParameter(2, best_2);
fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "QLSIM", "", xmin,xmax);
fitFunc->Draw();

return fitFunc;
}*/

TF1 *ZBiHistos::fitExponentialPlusExp(std::string histogramName, double start_nevents)
{

    // Get z vertex distribution corresponding to Test Cut
    std::string histoname = m_name + "_" + histogramName + "_h";

    // If histogram is empty, return nullptr
    if (histos1d[histoname]->GetEntries() < 1)
    {
        std::cout << "[ZBiHistos]::WARNING: Background Model is NULL: " << histogramName << " distribution is empty and could not be fit!" << std::endl;
        return nullptr;
    }
    double originalXmin = histos1d[histoname]->GetXaxis()->GetXmin();
    double originalXmax = histos1d[histoname]->GetXaxis()->GetXmax();

    // Start fit where start_nevents are left in tail
    int lastbin = histos1d[histoname]->FindLastBinAbove(0.0);
    if (histos1d[histoname]->Integral() < start_nevents)
        start_nevents = histos1d[histoname]->GetBinContent(histos1d[histoname]->GetMaximumBin());
    int firstbin = lastbin - 1;
    double test_integral = 0.0;
    while (test_integral < start_nevents && histos1d[histoname]->GetBinLowEdge(firstbin) > 0.0)
    {
        test_integral = histos1d[histoname]->Integral(firstbin, lastbin);
        firstbin = firstbin - 1;
    }

    double xmin = histos1d[histoname]->GetBinLowEdge(firstbin);
    double xmax = histos1d[histoname]->GetBinLowEdge(lastbin + 1);
    histos1d[histoname]->GetXaxis()->SetRangeUser(xmin, xmax);

    // Initially fit tail with exponential to seed params
    double best_seed_chi2 = 9999999.9;
    double seed_0;
    double seed_1;

    TF1 *fitFunc_seed = new TF1("fitFunc_seed", "[0]*exp([1]*x)", xmin, xmax);
    // TF1* fitFunc_seed = new TF1("fitFunc_seed", "expo", xmin, xmax);
    TFitResultPtr fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc_seed, "QSIM", "", xmin, xmax);
    seed_0 = fitResult->Parameter(0);
    seed_1 = fitResult->Parameter(1);
    if (fitResult->Ndf() > 0)
    {
        best_seed_chi2 = fitResult->Chi2() / fitResult->Ndf();
    }

    TRandom3 *ran = new TRandom3();
    ran->SetSeed(0);

    best_seed_chi2 = 999999.9;
    for (int i = 0; i < 50; i++)
    {
        fitFunc_seed->SetParameters(std::abs(ran->Gaus((double)start_nevents, 3.0)), -std::abs(ran->Uniform(0.01, 1.0)));
        fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc_seed, "QSIM", "", xmin, xmax);

        if (fitResult->Ndf() <= 0)
            continue;

        if (fitResult->Chi2() / fitResult->Ndf() < best_seed_chi2)
        {
            if (fitResult->Parameter(0) < 1000.0)
                continue;
            best_seed_chi2 = fitResult->Chi2() / fitResult->Ndf();
            seed_0 = fitResult->Parameter(0);
            seed_1 = fitResult->Parameter(1);
        }
    }

    double best_chi2 = 99999.9;
    double param_2;
    double best_seed_3;
    double best_seed_4;
    // TF1* fitFunc = new TF1("fitFunc", " (x<[2])*([0]*exp([1]*x)) + (x>=[2])*([3]*exp([4]*x))", xmin, xmax);
    TF1 *fitFunc = new TF1("fitFunc", " (x<[2])*([0]*exp([1]*x)) + (x>=[2])*(-[3]*x)", xmin, xmax);
    fitFunc->FixParameter(0, seed_0);
    fitFunc->FixParameter(1, seed_1);
    for (int i = 0; i < 1000; i++)
    {
        fitFunc->FixParameter(2, (double)i / 10.0);
        // TFitResultPtr intermed_fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc_seed, "QS", "", (double)i /10.0 ,xmax);
        // double seed_3 = intermed_fitResult->Parameter(0);
        // double seed_4 = intermed_fitResult->Parameter(1);
        // fitFunc->SetParameter(3,seed_3);
        // fitFunc->SetParameter(4,seed_4);

        fitFunc_seed->FixParameter(0, seed_0);
        fitFunc_seed->FixParameter(1, seed_1);
        double seed_3 = fitFunc_seed->Eval(histos1d[histoname]->GetBinCenter(histos1d[histoname]->FindLastBinAbove(0.0)));
        // std::cout << "LOOK FUCKER: " << seed_3 << std::endl;
        // fitFunc->SetParameter(3,seed_3);

        fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "QSIM", "", xmin, xmax);

        if (fitResult->Ndf() <= 0)
            continue;

        // std::cout << "test param 2: " << (double)i/10.0 << std::endl;
        // std::cout << "chi2: " << fitResult->Chi2()/fitResult->Ndf() << std::endl;
        if (fitResult->Chi2() / fitResult->Ndf() < best_chi2)
        {
            best_chi2 = fitResult->Chi2() / fitResult->Ndf();
            param_2 = fitResult->Parameter(2);
            best_seed_3 = seed_3;
            // best_seed_4 = seed_4;
        }
    }

    // delete ran;

    fitFunc->FixParameter(0, seed_0);
    fitFunc->FixParameter(1, seed_1);
    fitFunc->FixParameter(2, param_2);
    fitFunc->FixParameter(3, best_seed_3);
    // fitFunc->FixParameter(4, best_seed_4);
    fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "QSIM", "", xmin, xmax);
    fitFunc->Draw();

    // return histogram to original range
    histos1d[histoname]->GetXaxis()->SetRangeUser(originalXmin, originalXmax);

    delete fitFunc_seed;
    return fitFunc;
}

TF1 *ZBiHistos::fitExponentialTail(std::string histogramName, double start_nevents)
{

    // Get z vertex distribution corresponding to Test Cut
    std::string histoname = m_name + "_" + histogramName + "_h";

    // If histogram is empty, return nullptr
    if (histos1d[histoname]->GetEntries() < 1)
    {
        std::cout << "[ZBiHistos]::WARNING: Background Model is NULL: " << histogramName << " distribution is empty and could not be fit!" << std::endl;
        return nullptr;
    }
    double originalXmin = histos1d[histoname]->GetXaxis()->GetXmin();
    double originalXmax = histos1d[histoname]->GetXaxis()->GetXmax();

    // Start fit where start_nevents are left in tail
    int lastbin = histos1d[histoname]->FindLastBinAbove(0.0);
    if (histos1d[histoname]->Integral() < start_nevents)
        start_nevents = histos1d[histoname]->GetBinContent(histos1d[histoname]->GetMaximumBin());
    int firstbin = lastbin - 1;
    double test_integral = 0.0;
    while (test_integral < start_nevents && histos1d[histoname]->GetBinLowEdge(firstbin) > 0.0)
    {
        test_integral = histos1d[histoname]->Integral(firstbin, lastbin);
        firstbin = firstbin - 1;
    }

    double xmin = histos1d[histoname]->GetBinLowEdge(firstbin);
    double xmax = histos1d[histoname]->GetBinLowEdge(lastbin + 1);
    histos1d[histoname]->GetXaxis()->SetRangeUser(xmin, xmax);

    // Initially fit tail with exponential to seed params
    double best_seed_chi2 = 9999999.9;
    double seed_0;
    double seed_1;

    TF1 *fitFunc = new TF1("fitFunc", "[0]*exp([1]*x)", xmin, xmax);
    TFitResultPtr fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "QSIM", "", xmin, xmax);
    seed_0 = fitResult->Parameter(0);
    seed_1 = fitResult->Parameter(1);
    if (fitResult->Ndf() > 0)
    {
        best_seed_chi2 = fitResult->Chi2() / fitResult->Ndf();
    }

    TRandom3 *ran = new TRandom3();
    ran->SetSeed(0);

    // best_seed_chi2 = 999999.9;
    for (int i = 0; i < 30; i++)
    {
        fitFunc->SetParameters(std::abs(ran->Gaus((double)start_nevents, 3.0)), -std::abs(ran->Uniform(0.01, 1.0)));
        fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "QSIM", "", xmin, xmax);

        if (fitResult->Ndf() <= 0)
            continue;
        if (fitFunc->GetProb() < 0.001)
            continue;

        if (fitResult->Chi2() / fitResult->Ndf() < best_seed_chi2)
        {
            if (fitResult->Parameter(0) < 1000.0)
                continue;
            best_seed_chi2 = fitResult->Chi2() / fitResult->Ndf();
            seed_0 = fitResult->Parameter(0);
            seed_1 = fitResult->Parameter(1);
        }
    }

    double param_0 = seed_0;
    double param_1 = seed_1;
    for (int i = 0; i < 30; i++)
    {
        fitFunc->SetParameters(ran->Gaus(seed_0, 1.0), -std::abs(ran->Gaus(seed_1, 1.0)));
        fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "QSIM", "", xmin, xmax);
        if (fitResult->Ndf() <= 0)
            continue;
        if (fitFunc->GetProb() < 0.001)
            continue;

        if (fitResult->Chi2() / fitResult->Ndf() < best_seed_chi2)
        {
            if (fitResult->Parameter(0) < 1000.0)
                continue;
            best_seed_chi2 = fitResult->Chi2() / fitResult->Ndf();
            param_0 = fitResult->Parameter(0);
            param_1 = fitResult->Parameter(1);
        }
    }

    fitFunc->FixParameter(0, param_0);
    fitFunc->FixParameter(1, param_1);
    fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "QSIM", "", xmin, xmax);
    fitFunc->Draw();

    // return histogram to original range
    histos1d[histoname]->GetXaxis()->SetRangeUser(originalXmin, originalXmax);

    delete ran;
    return fitFunc;
}

TF1 *ZBiHistos::fitExponentialPlusConst(std::string histogramName, double start_nevents)
{

    // Get z vertex distribution corresponding to Test Cut
    std::string histoname = m_name + "_" + histogramName + "_h";

    // If histogram is empty, return nullptr
    if (histos1d[histoname]->GetEntries() < 1)
    {
        std::cout << "[ZBiHistos]::WARNING: Background Model is NULL: " << histogramName << " distribution is empty and could not be fit!" << std::endl;
        return nullptr;
    }
    double originalXmin = histos1d[histoname]->GetXaxis()->GetXmin();
    double originalXmax = histos1d[histoname]->GetXaxis()->GetXmax();

    // Start fit where start_nevents are left in tail
    int lastbin = histos1d[histoname]->FindLastBinAbove(0.0);
    if (histos1d[histoname]->Integral() < start_nevents)
        start_nevents = histos1d[histoname]->GetBinContent(histos1d[histoname]->GetMaximumBin());
    int firstbin = lastbin - 1;
    double test_integral = 0.0;
    while (test_integral < start_nevents && histos1d[histoname]->GetBinLowEdge(firstbin) > 0.0)
    {
        test_integral = histos1d[histoname]->Integral(firstbin, lastbin);
        firstbin = firstbin - 1;
    }

    double xmin = histos1d[histoname]->GetBinLowEdge(firstbin);
    double xmax = histos1d[histoname]->GetBinLowEdge(lastbin + 1);
    histos1d[histoname]->GetXaxis()->SetRangeUser(xmin, xmax);

    // Initially fit tail with exponential to seed params
    double best_seed_chi2 = 9999999.9;
    double seed_0;
    double seed_1;

    TF1 *fitFunc_seed = new TF1("fitFunc_seed", "[0]*exp([1]*x)", xmin, xmax);
    // TF1* fitFunc_seed = new TF1("fitFunc_seed", "expo", xmin, xmax);
    TFitResultPtr fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc_seed, "QSIM", "", xmin, xmax);
    seed_0 = fitResult->Parameter(0);
    seed_1 = fitResult->Parameter(1);
    if (fitResult->Ndf() > 0 && fitFunc_seed->GetProb() > 0.001)
    {
        best_seed_chi2 = fitResult->Chi2() / fitResult->Ndf();
    }

    TRandom3 *ran = new TRandom3();
    ran->SetSeed(0);

    // best_seed_chi2 = 999999.9;
    for (int i = 0; i < 30; i++)
    {
        fitFunc_seed->SetParameters(std::abs(ran->Gaus((double)start_nevents, 3.0)), -std::abs(ran->Uniform(0.01, 1.0)));
        fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc_seed, "QSIM", "", xmin, xmax);

        if (fitResult->Ndf() <= 0)
            continue;
        if (fitFunc_seed->GetProb() < 0.001)
            continue;

        if (fitResult->Chi2() / fitResult->Ndf() < best_seed_chi2)
        {
            if (fitResult->Parameter(0) < 1000.0)
                continue;
            best_seed_chi2 = fitResult->Chi2() / fitResult->Ndf();
            seed_0 = fitResult->Parameter(0);
            seed_1 = fitResult->Parameter(1);
        }
    }

    double best_chi2 = 99999.9;
    double param_2;
    TF1 *fitFunc = new TF1("fitFunc", " (x<[2])*([0]*exp([1]*x)) + (x>=[2])*([0]*exp([1]*[2]))", xmin, xmax);
    fitFunc->FixParameter(0, seed_0);
    fitFunc->FixParameter(1, seed_1);
    for (int i = 0; i < 1000; i++)
    {
        fitFunc->FixParameter(2, (double)i / 10.0);
        fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "QSIM", "", xmin, xmax);

        if (fitResult->Ndf() <= 0)
            continue;
        if (fitFunc->GetProb() < 0.001)
            continue;

        if (fitResult->Chi2() / fitResult->Ndf() < best_chi2)
        {
            best_chi2 = fitResult->Chi2() / fitResult->Ndf();
            param_2 = fitResult->Parameter(2);
        }
    }

    delete ran;

    fitFunc->FixParameter(0, seed_0);
    fitFunc->FixParameter(1, seed_1);
    fitFunc->FixParameter(2, param_2);
    fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "QSIM", "", xmin, xmax);
    fitFunc->Draw();

    // return histogram to original range
    histos1d[histoname]->GetXaxis()->SetRangeUser(originalXmin, originalXmax);

    delete fitFunc_seed;
    return fitFunc;
}

/*

TF1* ZBiHistos::fitExponentialTail(std::string histogramName, double start_nevents){
    //Background Model Fit Function
    //TF1* fitFunc = new TF1("fitfunc","[0]*exp([1]*x)",0.0,100.0);
    TF1* fitFunc = new TF1("fitfunc","expo",0.0,100.0);

    //Get z vertex distribution corresponding to Test Cut
    std::string histoname = m_name+"_"+histogramName+"_h";

    //If histogram is empty, return nullptr
    if(histos1d[histoname]->GetEntries() < 1){
        std::cout << "[ZBiHistos]::WARNING: Background Model is NULL: " <<
            histogramName << " distribution is empty and could not be fit!" << std::endl;
        return nullptr;
    }

    //start mod
    double originalXmin = histos1d[histoname]->GetXaxis()->GetXmin();
    double originalXmax = histos1d[histoname]->GetXaxis()->GetXmax();

    //Start fit where start_nevents are left in tail
    int lastbin = histos1d[histoname]->FindLastBinAbove(0.0);
    if(histos1d[histoname]->Integral() < start_nevents)
        start_nevents = histos1d[histoname]->GetBinContent(histos1d[histoname]->GetMaximumBin());
    int firstbin = lastbin - 1;
    double test_integral = 0.0;
    int peak_bin = histos1d[histoname]->GetMaximumBin();
    while(test_integral < start_nevents && firstbin > peak_bin){
        test_integral = histos1d[histoname]->Integral(firstbin, lastbin);
        firstbin = firstbin - 1;
    }

    double xmin = histos1d[histoname]->GetBinLowEdge(firstbin);
    double xmax = histos1d[histoname]->GetBinLowEdge(lastbin+1);
    std::cout << "fit xmin " << xmin << std::endl;
    std::cout << "fit xmax " << xmax << std::endl;
    histos1d[histoname]->GetXaxis()->SetRangeUser(xmin,xmax);

    fitFunc->SetParameters(50*start_nevents, -0.5);
    TFitResultPtr fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "QLSIM", "", xmin,xmax);
    double parm0 = fitResult->Parameter(0);
    double parm1 = fitResult->Parameter(1);

    TRandom3* ran = new TRandom3();
    ran->SetSeed(0);
    double best_chi2 = 999999999.9;
    double best_parm0 = parm0;
    double best_parm1 = parm1;

    int iteration = 50;
    for(int i=0; i < iteration; i++){
        fitFunc->SetParameters(std::abs(ran->Gaus(parm0,2)),ran->Uniform(-1.0,-0.1));
        fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "QLSIM", "", xmin,xmax);
        if(fitResult->Ndf() <= 0)
            continue;

        if(fitResult->Chi2()/fitResult->Ndf() < best_chi2){
            best_chi2 = fitResult->Chi2()/fitResult->Ndf();
            best_parm0 = fitResult->Parameter(0);
            best_parm1 = fitResult->Parameter(1);
        }
    }

    std::cout << "Fit Params: [0] = " << best_parm0 <<" [1] = " << best_parm1
       << std::endl;
    fitFunc->SetParameter(0,best_parm0);
    fitFunc->SetParameter(1,best_parm1);
    fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "LSIM", "", xmin, xmax);
    fitFunc->Draw();
    //std::cout << "Fit Params: [0] = " << best_parm0 <<" [1] = " << best_parm1
    //    << " [2] = " << best_parm2 << std::endl;
    std::cout << "Final fit param 0: " << fitResult->Parameter(0) << std::endl;
    std::cout << "Final fit param 1: " << fitResult->Parameter(1) << std::endl;
    //return histogram to original range
    fitFunc->Draw();
    histos1d[histoname]->GetXaxis()->SetRangeUser(originalXmin,originalXmax);

    return fitFunc;
}
*/
/*
TF1* ZBiHistos::fitExponentialTail(std::string histogramName, double start_nevents){
    //Background Model Fit Function
    TF1* fitFunc = new TF1("fitfunc","[0]*exp([1]*x)",0.0,100.0);

    //Get z vertex distribution corresponding to Test Cut
    std::string histoname = m_name+"_"+histogramName+"_h";

    //If histogram is empty, return nullptr
    if(histos1d[histoname]->GetEntries() < 1){
        std::cout << "[ZBiHistos]::WARNING: Background Model is NULL: " <<
            histogramName << " distribution is empty and could not be fit!" << std::endl;
        return nullptr;
    }

    //Start fit where start_nevents are left in tail
    int lastbin = histos1d[histoname]->FindLastBinAbove(0.0);
    int firstbin = lastbin - 1;
    double test_integral = 0.0;
    while(test_integral < start_nevents && histos1d[histoname]->GetBinLowEdge(firstbin) > 0.0){
        test_integral = histos1d[histoname]->Integral(firstbin, lastbin);
        firstbin = firstbin - 1;
    }

    double xmin = histos1d[histoname]->GetBinLowEdge(firstbin);
    double xmax = histos1d[histoname]->GetBinLowEdge(lastbin+1);

    //start mod
    double originalXmin = histos1d[histoname]->GetXaxis()->GetXmin();
    double originalXmax = histos1d[histoname]->GetXaxis()->GetXmax();

    //Start fit where start_nevents are left in tail
    int lastbin = histos1d[histoname]->FindLastBinAbove(0.0);
    if(histos1d[histoname]->Integral() < start_nevents)
        start_nevents = histos1d[histoname]->GetBinContent(histos1d[histoname]->GetMaximumBin());
    int firstbin = lastbin - 1;
    double test_integral = 0.0;
    while(test_integral < start_nevents && histos1d[histoname]->GetBinLowEdge(firstbin) > 0.0){
        test_integral = histos1d[histoname]->Integral(firstbin, lastbin);
        firstbin = firstbin - 1;
    }

    double xmin = histos1d[histoname]->GetBinLowEdge(firstbin);
    double xmax = histos1d[histoname]->GetBinLowEdge(lastbin+1);
    std::cout << "fit xmin " << xmin << std::endl;
    std::cout << "fit xmax " << xmax << std::endl;
    //double xmax = 100.0;
    histos1d[histoname]->GetXaxis()->SetRangeUser(xmin,xmax);

    TFitResultPtr fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "QLSIM", "", xmin,xmax);
    double parm0 = fitResult->Parameter(0);
    double parm1 = fitResult->Parameter(1);

    TRandom3* ran = new TRandom3();
    ran->SetSeed(0);
    double best_chi2 = 999999999.9;
    double best_parm0 = parm0;
    double best_parm1 = parm1;
    double best_parm2;

    //fitFunc = new TF1("fitfunc","[0]*exp([1]*x) + abs([2])",0.0,100.0);
    fitFunc = new TF1("fitfunc","[0]*exp([1]*x)",0.0,100.0);
    int iteration = 25;
    for(int i=0; i < iteration; i++){
        //fitFunc->SetParameters(ran->Gaus(parm0,2),ran->Gaus(parm1,2), ran->Uniform(10));
        fitFunc->SetParameters(ran->Gaus(parm0,2),ran->Gaus(parm1,2));
        fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "QLSIM", "", xmin,xmax);
        if(fitResult->Ndf() <= 0)
            continue;

        if(fitResult->Chi2()/fitResult->Ndf() < best_chi2){
            best_chi2 = fitResult->Chi2()/fitResult->Ndf();
            best_parm0 = fitResult->Parameter(0);
            best_parm1 = fitResult->Parameter(1);
            //best_parm2 = fitResult->Parameter(2);
        }
    }

    //fitFunc->SetParameters(best_parm0, best_parm1, best_parm2);
    fitFunc->SetParameters(best_parm0, best_parm1);
    fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "LSIM", "", xmin, xmax);
    fitFunc->Draw();
    //std::cout << "Fit Params: [0] = " << best_parm0 <<" [1] = " << best_parm1
    //    << " [2] = " << best_parm2 << std::endl;
    std::cout << "Fit Params: [0] = " << best_parm0 <<" [1] = " << best_parm1
       << std::endl;

    //return histogram to original range
    histos1d[histoname]->GetXaxis()->SetRangeUser(originalXmin,originalXmax);

    return fitFunc;
}
*/

/*
TF1* ZBiHistos::fitExponentialTail(std::string histogramName, double start_nevents){
    //Background Model Fit Function
    TF1* fitFunc = new TF1("fitfunc","[0]*exp([1]*x)",10.0,100.0);
    //TF1* fitFunc = new TF1("fitfunc","[0]*exp([1]*x) + [2]*exp([3]*x)",10.0,100.0);
    //TF1* fitFunc = new TF1("fitfunc","[0]*exp([1]*x) + [2]",10.0,100.0);

    //Get z vertex distribution corresponding to Test Cut
    std::string histoname = m_name+"_"+histogramName+"_h";

    //If histogram is empty, return nullptr
    if(histos1d[histoname]->GetEntries() < 1){
        std::cout << "[ZBiHistos]::WARNING: Background Model is NULL: " <<
            histogramName << " distribution is empty and could not be fit!" << std::endl;
        return nullptr;
    }
    //Start fit where start_nevents are left in tail
    int lastbin = histos1d[histoname]->FindLastBinAbove(0.0);
    int firstbin = lastbin - 1;
    double test_integral = 0.0;
    while(test_integral < start_nevents && histos1d[histoname]->GetBinLowEdge(firstbin) > 0.0){
        test_integral = histos1d[histoname]->Integral(firstbin, lastbin);
        firstbin = firstbin - 1;
    }

    double xmin = histos1d[histoname]->GetBinLowEdge(firstbin);
    double xmax = histos1d[histoname]->GetBinLowEdge(lastbin+1);

    TFitResultPtr fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "QLSIM", "", xmin,xmax);
    double parm0 = fitResult->Parameter(0);
    double parm1 = fitResult->Parameter(1);
    //double parm2 = fitResult->Parameter(2);
    //double parm3 = fitResult->Parameter(3);

    double best_chi2 = 9999999.9;
    double best_parm0, best_parm1, best_parm2, best_parm3;

    int iteration = 20;
    for(int i=0; i < iteration; i++){
        fitFunc->SetParameters(parm0,parm1);
        fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "QLSIM", "", xmin,xmax);
        if(fitResult->Ndf() <= 0)
            continue;
        if(fitResult->Chi2()/fitResult->Ndf() < best_chi2){
            best_chi2 = fitResult->Chi2()/fitResult->Ndf();
            best_parm0 = fitResult->Parameter(0);
            best_parm1 = fitResult->Parameter(1);
            //best_parm2 = fitResult->Parameter(2);
            //best_parm3 = fitResult->Parameter(3);
        }
    }

    //fitFunc = new TF1("fitfunc","[0]*exp([1]*x) + [2]*exp([3]*x)",10.0,100.0); //TRASH
    fitFunc = new TF1("fitfunc","[0]*exp([1]*x) + [2]",10.0,100.0);
    double parm2 = best_parm0*0.5;
    for(int i=0; i < iteration; i++){
        fitFunc->SetParameters(parm0,parm1,parm2);
        fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "QLSIM", "", xmin,xmax);
        if(fitResult->Ndf() <= 0)
            continue;
        if(fitResult->Chi2()/fitResult->Ndf() < best_chi2){
            best_chi2 = fitResult->Chi2()/fitResult->Ndf();
            best_parm0 = fitResult->Parameter(0);
            best_parm1 = fitResult->Parameter(1);
            best_parm2 = fitResult->Parameter(2);
            //best_parm3 = fitResult->Parameter(3);
        }
    }


    //fitFunc->SetParameters(best_parm0,best_parm1, best_parm2, best_parm3);
    fitFunc->SetParameters(best_parm0,best_parm1, best_parm2);
    fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "LSIM", "", xmin, xmax);
    fitFunc->Draw();

    return fitFunc;
}
*/

/*
TF1* ZBiHistos::fitExponentialTail(std::string histogramName, double start_nevents){
    //Background Model Fit Function
    TF1* fitFunc = new TF1("fitfunc","[0]*exp([1]*x)",10.0,100.0);

    //Get z vertex distribution corresponding to Test Cut
    std::string histoname = m_name+"_"+histogramName+"_h";

    //If histogram is empty, return nullptr
    if(histos1d[histoname]->GetEntries() < 1){
        std::cout << "[ZBiHistos]::WARNING: Background Model is NULL: " <<
            histogramName << " distribution is empty and could not be fit!" << std::endl;
        return nullptr;
    }
    //Start fit where start_nevents are left in tail
    int lastbin = histos1d[histoname]->FindLastBinAbove(0.0);
    int firstbin = lastbin - 1;
    double test_integral = 0.0;
    while(test_integral < start_nevents && histos1d[histoname]->GetBinLowEdge(firstbin) > 0.0){
        test_integral = histos1d[histoname]->Integral(firstbin, lastbin);
        firstbin = firstbin - 1;
    }

    double xmin = histos1d[histoname]->GetBinLowEdge(firstbin);
    double xmax = histos1d[histoname]->GetBinLowEdge(lastbin+1);

    TFitResultPtr fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "QLSIM", "", xmin,xmax);
    double parm0 = fitResult->Parameter(0);
    double parm1 = fitResult->Parameter(1);

    double best_chi2 = fitResult->Chi2()/fitResult->Ndf();
    double best_parm0 = parm0;
    double best_parm1 = parm1;

    TRandom3* ran = new TRandom3();
    ran->SetSeed(0);
    int iteration = 25;
    for(int i=0; i < iteration; i++){
        fitFunc->SetParameters(ran->Gaus(best_parm0,2),ran->Gaus(best_parm1,2));
        fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "QLSIM", "", xmin,xmax);
        if(fitResult->Ndf() <= 0)
            continue;

        if(fitResult->Chi2()/fitResult->Ndf() < best_chi2){
            best_chi2 = fitResult->Chi2()/fitResult->Ndf();
            best_parm0 = fitResult->Parameter(0);
            best_parm1 = fitResult->Parameter(1);
        }
    }

    fitFunc = new TF1("fitfunc","[0]*exp([1]*x) + [2]",10.0,100.0);
    double best_parm2;
    best_chi2 = 999999999.9;
    for(int i=0; i < iteration; i++){
        fitFunc->SetParameter(2,ran->Uniform(10));
        fitFunc->FixParameter(0,best_parm0);
        fitFunc->FixParameter(1,best_parm1);
        fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "QLSIM", "", xmin,xmax);
        if(fitResult->Ndf() <= 0)
            continue;

        if(fitResult->Chi2()/fitResult->Ndf() < best_chi2){
            best_chi2 = fitResult->Chi2()/fitResult->Ndf();
            //best_parm0 = fitResult->Parameter(0);
            //best_parm1 = fitResult->Parameter(1);
            best_parm2 = fitResult->Parameter(2);
            //best_parm3 = fitResult->Parameter(3);
        }
    }

    fitFunc->FixParameter(0,best_parm0);
    fitFunc->FixParameter(1,best_parm1);
    fitFunc->FixParameter(2,best_parm2);
    fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "LSIM", "", xmin, xmax);
    fitFunc->Draw();

    return fitFunc;
}*/

TF1 *ZBiHistos::getZTailFit(std::string cutname)
{
    TF1 *fitFunc = new TF1("fitfunc", "[0]* exp( (x<=([1]+[3]))*(-0.5*((x-[1])^2)/([2]^2)) + (x>=[1]+[3])*(-0.5*([3]^2/[2]^2)-(x-[1]-[3])/[4])  )", -100.0, 100.0);

    std::string histoname = m_name + "_tritrig_zVtx_" + cutname + "_h";
    if (histos1d[histoname]->GetEntries() < 1)
    {
        std::cout << "WARNING: Background Model is NULL: " << cutname << " tritrig zVtx distribution was empty and could not be fit!" << std::endl;
        return nullptr;
    }

    TFitResultPtr gausResult = (TFitResultPtr)histos1d[histoname]->Fit("gaus", "QS");
    double gaus0 = gausResult->GetParams()[0];
    double gaus1 = gausResult->GetParams()[1];
    double gaus2 = gausResult->GetParams()[2];
    // gausResult = histos1d[histoname]->Fit("gaus","QS","",gaus1-2.5*gaus2, gaus1+10.0*gaus2);
    gausResult = histos1d[histoname]->Fit("gaus", "QS", "", gaus1 - 2.5 * gaus2, gaus1 + 10.0 * gaus2);
    gaus0 = gausResult->GetParams()[0];
    gaus1 = gausResult->GetParams()[1];
    gaus2 = gausResult->GetParams()[2];
    double xmin = gaus1 - 2.5 * gaus2;
    double xmax = histos1d[histoname]->GetBinCenter(histos1d[histoname]->FindLastBinAbove(0.0) + 1);
    xmax = 100.0;

    // Fit function for a few different seeds for tail start, and length, keep the best fit
    double tailZ = gaus1 + 3.0 * gaus2;
    double tail_l = 50.0;
    TRandom3 *ran = new TRandom3();
    ran->SetSeed(0);

    double best_chi2 = 9999999.9;
    double best_tailZ;
    double best_tail_l;
    double best_gaus0, best_gaus1, best_gaus2;

    // Having issues with this fit sometimes. It occassionally underestimates bkg model
    int iteration = 80;
    tail_l = 50.0;
    for (int i = 10; i < iteration; i = i + 2)
    {
        tailZ = gaus1 + (double)(iteration / 10.0) * gaus2;
        fitFunc->SetParameters(gaus0, gaus1, gaus2, tailZ, tail_l);
        TFitResultPtr fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "QLSIM", "", xmin, xmax);
        if (fitResult->Ndf() <= 0)
            continue;
        if (fitResult->Chi2() / fitResult->Ndf() < best_chi2)
        {
            best_chi2 = fitResult->Chi2() / fitResult->Ndf();
            best_gaus0 = fitResult->Parameter(0);
            best_gaus1 = fitResult->Parameter(1);
            best_gaus2 = fitResult->Parameter(2);
            best_tailZ = fitResult->Parameter(3);
            best_tail_l = fitResult->Parameter(4);
            tail_l = fitResult->Parameter(4);
        }
    }

    fitFunc->SetParameters(best_gaus0, best_gaus1, best_gaus2, best_tailZ, best_tail_l);
    TFitResultPtr fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "LSIM", "", xmin, xmax);
    fitFunc->Draw();
    std::cout << "Finished fitting ztail" << std::endl;

    delete ran;
    return fitFunc;
}

double ZBiHistos::shosFitZTail(std::string cutname, double max_tail_events)
{
    TF1 *fitFunc = new TF1("fitfunc", "[0]* exp( (x<=([1]+[3]))*(-0.5*((x-[1])^2)/([2]^2)) + (x>=[1]+[3])*(-0.5*([3]^2/[2]^2)-(x-[1]-[3])/[4])  )", -100.0, 100.0);

    std::string histoname = m_name + "_tritrig_zVtx_" + cutname + "_h";
    if (histos1d[histoname]->GetEntries() < 1)
        return -4.3;

    TFitResultPtr gausResult = (TFitResultPtr)histos1d[histoname]->Fit("gaus", "QS");
    double gaus0 = gausResult->GetParams()[0];
    double gaus1 = gausResult->GetParams()[1];
    double gaus2 = gausResult->GetParams()[2];
    gausResult = histos1d[histoname]->Fit("gaus", "QS", "", gaus1 - 3.0 * gaus2, gaus1 + 10.0 * gaus2);
    gaus0 = gausResult->GetParams()[0];
    gaus1 = gausResult->GetParams()[1];
    gaus2 = gausResult->GetParams()[2];
    double xmin = gaus1 - 2.5 * gaus2;
    double xmax = histos1d[histoname]->GetBinCenter(histos1d[histoname]->FindLastBinAbove(0.0) + 1);
    xmax = 100.0;

    // Fit function for a few different seeds for tail start, and length, keep the best fit
    double tailZ = gaus1 + 3.0 * gaus2;
    double tail_l = 50.0;
    TRandom3 *ran = new TRandom3();
    ran->SetSeed(0);

    double best_chi2 = 99999.9;
    double best_tailZ;
    double best_tail_l;
    double best_gaus0, best_gaus1, best_gaus2;

    int iteration = 80;
    for (int i = 10; i < iteration; i = i + 2)
    {
        tail_l = 50.0;
        tailZ = gaus1 + (double)(iteration / 10.0) * gaus2;
        fitFunc->SetParameters(gaus0, gaus1, gaus2, tailZ, tail_l);
        TFitResultPtr fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "LSIM", "", xmin, xmax);
        if (fitResult->Ndf() <= 0)
            continue;
        if (fitResult->Chi2() < best_chi2)
        {
            best_chi2 = fitResult->Chi2() / fitResult->Ndf();
            best_gaus0 = fitResult->Parameter(0);
            best_gaus1 = fitResult->Parameter(1);
            best_gaus2 = fitResult->Parameter(2);
            best_tailZ = fitResult->Parameter(3);
            best_tail_l = fitResult->Parameter(4);
        }
    }

    fitFunc->SetParameters(best_gaus0, best_gaus1, best_gaus2, best_tailZ, best_tail_l);
    TFitResultPtr fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "LSIM", "", xmin, xmax);

    double zcut = -6.0;
    double testIntegral = fitFunc->Integral(zcut, 100.0);
    while (testIntegral > max_tail_events)
    {
        zcut = zcut + 0.1;
        testIntegral = fitFunc->Integral(zcut, 100.0);
    }
    fitFunc->Draw();
    delete ran;
    return zcut;
}

double ZBiHistos::fitZTail(std::string zVtxHistoname, double max_tail_events)
{
    TF1 *fitFunc = new TF1("fitfunc", "[0]*exp( (((x-[1])/[2])<[3])*(-0.5*(x-[1])^2/[2]^2) + (((x-[1])/[2])>=[3])*(0.5*[3]^2-[3]*(x-[1])/[2]))", -100.0, 100.0);

    std::string histoname = m_name + "_" + zVtxHistoname;
    if (histos1d[histoname]->GetEntries() < 1)
        return -4.3;
    TFitResultPtr gausResult = (TFitResultPtr)histos1d[histoname]->Fit("gaus", "QS");
    double gaus1 = gausResult->GetParams()[1];
    double gaus2 = gausResult->GetParams()[2];
    gausResult = histos1d[histoname]->Fit("gaus", "QS", "", gaus1 - 3.0 * gaus2, gaus1 + 10.0 * gaus2);
    gaus1 = gausResult->GetParams()[1];
    gaus2 = gausResult->GetParams()[2];
    double tailZ = gaus1 + 3.0 * gaus2;

    fitFunc->SetParameters(gausResult->GetParams()[0], gaus1, gaus2, 3.0);
    TFitResultPtr fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "LSIM", "", gaus1 - 2.0 * gaus2, gaus1 + 10.0 * gaus2);

    double zcut = -6.0;
    double testIntegral = fitFunc->Integral(zcut, 90.0);
    while (testIntegral > max_tail_events)
    {
        zcut = zcut + 0.1;
        testIntegral = fitFunc->Integral(zcut, 90.0);
    }
    fitFunc->Draw();
    return zcut;
}

void ZBiHistos::writeGraphs(TFile *outF, std::string folder)
{
    if (outF)
        outF->cd();
    else
        return;

    TDirectory *dir{nullptr};
    if (!folder.empty())
    {
        dir = outF->mkdir(folder.c_str(), "", true);
        dir->cd();
    }
    for (std::map<std::string, TGraph *>::iterator it = graphs_.begin(); it != graphs_.end(); ++it)
    {
        if (!it->second)
        {
            std::cout << it->first << " Null ptr in saving.." << std::endl;
            continue;
        }
        it->second->Write();
    }
}

void ZBiHistos::writeHistos(TFile *outF, std::string folder)
{
    if (outF)
        outF->cd();
    else
        return;

    TDirectory *dir{nullptr};
    std::cout << folder.c_str() << std::endl;
    if (!folder.empty())
    {
        dir = outF->mkdir(folder.c_str(), "", true);
        dir->cd();
    }
    for (it1d it = histos1d.begin(); it != histos1d.end(); ++it)
    {
        if (!it->second)
        {
            std::cout << it->first << " Null ptr in saving.." << std::endl;
            continue;
        }
        it->second->Write();
    }
    for (it2d it = histos2d.begin(); it != histos2d.end(); ++it)
    {
        if (!it->second)
        {
            std::cout << it->first << " Null ptr in saving.." << std::endl;
            continue;
        }
        it->second->Write();
    }
}
