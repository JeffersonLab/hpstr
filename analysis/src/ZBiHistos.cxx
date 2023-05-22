#include "ZBiHistos.h"
#include <sstream>

ZBiHistos::ZBiHistos() {
}

ZBiHistos::~ZBiHistos() {
}

ZBiHistos::ZBiHistos(const std::string& inputName){
    m_name = inputName;
}

void ZBiHistos::addHisto1d(std::string histoname, std::string xtitle, int nbinsX, float xmin, float xmax){
    histos1d[m_name+"_"+histoname] = plot1D(m_name+"_"+histoname, xtitle, nbinsX, xmin, xmax);
}

void ZBiHistos::addHisto2d(std::string histoname, std::string xtitle, int nbinsX, float xmin, float xmax, std::string ytitle, int nbinsY, float ymin, float ymax){
    histos2d[m_name+"_"+histoname] = plot2D(m_name+"_"+histoname, xtitle, nbinsX, xmin, xmax, ytitle, nbinsY, ymin, ymax);
}

void ZBiHistos::resetHistograms1d(){
    for(it1d it=histos1d.begin(); it != histos1d.end(); it ++){
        if(it->second != nullptr)
            it->second->Reset();
    }
}

void ZBiHistos::resetHistograms2d(){
    for(it2d it=histos2d.begin(); it != histos2d.end(); it ++){
        if(it->second != nullptr)
            it->second->Reset();
    }
}

double ZBiHistos::integrateHistogram1D(std::string histoname){
    int xmax; 
    int xmin;
    double integral;

    //Check if histogram exists to integrate
    if(!histos1d.count(histoname)){
        std::cout << "[ZBiHistos}::ERROR::NO HISTOGRAM NAMED " << histoname << 
            " FOUND! DEFINE HISTOGRAM IN HISTOCONFIG!" << std::endl;
        integral = -9999.9;
    }

    else{
        xmax = histos1d[histoname]->FindLastBinAbove(0.0);
        xmin = histos1d[histoname]->FindFirstBinAbove(0.0);
        integral = histos1d[histoname]->Integral(xmin,xmax);
    }
    return integral;
}

double ZBiHistos::cutFractionOfSignalVariable(std::string cutvariable, bool isCutGreaterThan, double cutFraction, double initialIntegral){

    TH1F* histo = histos1d[m_name+"_"+cutvariable+"_h"];
    int xmax = histo->FindLastBinAbove(0.0);
    int xmin = histo->FindFirstBinAbove(0.0);
    
    double cutvalue;
    if(isCutGreaterThan){
       cutvalue = histo->GetXaxis()->GetBinLowEdge(xmin); 
       while(histo->Integral(xmin,xmax) > initialIntegral*(1.0-cutFraction)){
           xmin = xmin + 1;
           cutvalue = histo->GetXaxis()->GetBinLowEdge(xmin);
       }
    }
    else {
       cutvalue = histo->GetXaxis()->GetBinUpEdge(xmax); 
       while(histo->Integral(xmin,xmax) > initialIntegral*(1.0-cutFraction)){
           xmax = xmax - 1;
           cutvalue = histo->GetXaxis()->GetBinUpEdge(xmax);
       }
    }

    return cutvalue;
}

void ZBiHistos::defineZBiCutflowProcessorHistograms(){
    //These histograms are used specifically for tracking the ZBitCutflowProcessor iterative process
    addHisto2d("persistent_cuts_hh","pct_sig_cut", 1000,-0.5,99.5,"cut_id",25,0.5,25.5);
    addHisto2d("test_cuts_ZBi_hh","pct_sig_cut", 1000,-0.5,99.5,"cut_id",25,0.5,25.5);
    addHisto2d("test_cuts_values_hh","pct_sig_cut", 1000,-0.5,99.5,"cut_id",25,0.5,25.5);
    addHisto2d("test_cuts_zcut_hh","pct_sig_cut", 1000,-0.5,99.5,"zcut",25,0.5,25.5);
    addHisto2d("test_cuts_nbkg_hh","pct_sig_cut", 1000,-0.5,99.5,"zcut",25,0.5,25.5);
    addHisto2d("test_cuts_nsig_hh","pct_sig_cut", 1000,-0.5,99.5,"zcut",25,0.5,25.5);
    addHisto2d("best_test_cut_ZBi_hh","pct_sig_cut", 1000,-0.5,99.5,"ZBi",2000,0.0,20.0);
}

void ZBiHistos::set2DHistoYlabel(std::string histoName, int ybin, std::string ylabel){
    histos2d[m_name+"_"+histoName]->GetYaxis()->SetBinLabel(ybin, ylabel.c_str());
}

std::vector<double> ZBiHistos::defineImpactParameterCut(double alpha){
    //alpha defines the % of signal we allow to be cut. Default is 15%
    TH2F* hh = (TH2F*)histos2d[m_name+"_z0_v_recon_z_hh"];
    addHisto1d("impact_parameter_up_h","recon_z [mm]", 450, -20.0, 70.0);
    addHisto1d("impact_parameter_down_h","recon_z [mm]", 450, -20.0, 70.0);
    TH1F* up_h = (TH1F*)histos1d[m_name+"_impact_parameter_up_h"];
    TH1F* down_h = (TH1F*)histos1d[m_name+"_impact_parameter_down_h"];

    for(int i=0; i < hh->GetNbinsX(); i++){

        TH1F* projy = (TH1F*)hh->ProjectionY(("projy_bin_"+std::to_string(i+1)).c_str(),i+1,i+1);
        if(projy->GetEntries() < 1) continue;

        //Impact param for z0 > 0
        int start_bin = projy->FindBin(0.0);
        int end_bin = projy->FindLastBinAbove(0.0); 
        double refIntegral = projy->Integral(start_bin,end_bin);
        int cutz0_bin = start_bin; 
        double testIntegral = refIntegral;
        while(testIntegral > (1.0-alpha)*refIntegral && cutz0_bin < end_bin-1){
            cutz0_bin = cutz0_bin + 1;           
            testIntegral = projy->Integral(cutz0_bin, end_bin);
        }
        
        double cutz0_up = projy->GetXaxis()->GetBinLowEdge(cutz0_bin);
        for(int j=0; j<(int)refIntegral; j++){
            up_h->Fill(hh->GetXaxis()->GetBinCenter(i+1),cutz0_up/refIntegral);
        }

        //impact param for z0 < 0
        end_bin = projy->FindFirstBinAbove(0.0); 
        start_bin = projy->FindBin(0.0);
        refIntegral = projy->Integral(end_bin, start_bin);
        cutz0_bin = start_bin; 
        testIntegral = refIntegral;
        while(testIntegral > (1.0-alpha)*refIntegral && cutz0_bin > end_bin+1){
            cutz0_bin = cutz0_bin - 1;           
            testIntegral = projy->Integral(end_bin, cutz0_bin);
        }
        
        double cutz0_down = projy->GetXaxis()->GetBinUpEdge(cutz0_bin);
        for(int j=0; j<(int)refIntegral; j++){
            down_h->Fill(hh->GetXaxis()->GetBinCenter(i+1),cutz0_down/refIntegral);
        }
    }

    TF1* fitFunc = new TF1("linear_fit","[0]*(x-[1])",5.0,70.0);
    TFitResultPtr fitResult = (TFitResultPtr)up_h->Fit("linear_fit","QS","",5.0,70.0);
    fitFunc->Draw();
    double m_p = fitResult->GetParams()[0];
    double a_p = fitResult->GetParams()[1];

    fitResult = (TFitResultPtr)down_h->Fit("linear_fit","QS","",5.0,70.0);
    fitFunc->Draw();
    double m_d = fitResult->GetParams()[0];
    double a_d = fitResult->GetParams()[1];

    //Find the location in z0 where two lines meet
    double diff = 999.9;
    double x = 10.0;
    while(std::abs((m_p*(x-a_p) - (m_d*(x-a_d)))) < diff ){
    //while( std::abs((a_p+b_p*x) - (a_d+b_d*x)) < diff ){
        diff = std::abs((m_p*(x-a_p)) - (m_d*(x-a_d)));    
        x = x - 0.01;
    }
    double beta = .5*((m_p*(x-a_p)) + (m_p*(x-a_d)));
    std::vector<double> params {m_p,a_p,m_d,a_d, beta, x};

    delete fitFunc;
    return params;
}

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

    double best_chi2 = 9999999.9;
    double best_parm0, best_parm1;

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
        }
    }

    fitFunc->SetParameters(best_parm0,best_parm1);
    fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "LSIM", "", xmin, xmax);
    fitFunc->Draw();

    return fitFunc;
}


TF1* ZBiHistos::getZTailFit(std::string cutname){
    TF1* fitFunc = new TF1("fitfunc", "[0]* exp( (x<=([1]+[3]))*(-0.5*((x-[1])^2)/([2]^2)) + (x>=[1]+[3])*(-0.5*([3]^2/[2]^2)-(x-[1]-[3])/[4])  )", -100.0, 100.0);

    std::string histoname = m_name+"_tritrig_zVtx_"+cutname+"_h";
    if(histos1d[histoname]->GetEntries() < 1){
        std::cout << "WARNING: Background Model is NULL: " << 
            cutname << " tritrig zVtx distribution was empty and could not be fit!" << std::endl;
        return nullptr;
    }

    TFitResultPtr gausResult = (TFitResultPtr)histos1d[histoname]->Fit("gaus","QS"); 
    double gaus0 = gausResult->GetParams()[0];
    double gaus1 = gausResult->GetParams()[1];
    double gaus2 = gausResult->GetParams()[2];
    //gausResult = histos1d[histoname]->Fit("gaus","QS","",gaus1-2.5*gaus2, gaus1+10.0*gaus2);
    gausResult = histos1d[histoname]->Fit("gaus","QS","",gaus1-2.5*gaus2, gaus1+10.0*gaus2);
    gaus0 = gausResult->GetParams()[0];
    gaus1 = gausResult->GetParams()[1];
    gaus2 = gausResult->GetParams()[2];
    double xmin = gaus1 - 2.5*gaus2;
    double xmax = histos1d[histoname]->GetBinCenter(histos1d[histoname]->FindLastBinAbove(0.0)+1);
    xmax = 100.0;

    //Fit function for a few different seeds for tail start, and length, keep the best fit
    double tailZ = gaus1 + 3.0*gaus2;
    double tail_l = 50.0;
    TRandom3* ran = new TRandom3();
    ran->SetSeed(0);

    double best_chi2 = 9999999.9;
    double best_tailZ;
    double best_tail_l;
    double best_gaus0, best_gaus1, best_gaus2;

    //Having issues with this fit sometimes. It occassionally underestimates bkg model
    int iteration = 80;
    tail_l = 50.0;
    for(int i =10; i < iteration; i=i+2){
        tailZ = gaus1 +(double)(iteration/10.0)*gaus2;
        fitFunc->SetParameters(gaus0, gaus1, gaus2, tailZ, tail_l);
        TFitResultPtr fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "QLSIM", "", xmin,xmax);
        if(fitResult->Ndf() <= 0)
            continue;
        if(fitResult->Chi2()/fitResult->Ndf() < best_chi2){
            best_chi2 = fitResult->Chi2()/fitResult->Ndf();
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

double ZBiHistos::shosFitZTail(std::string cutname, double max_tail_events){
    TF1* fitFunc = new TF1("fitfunc", "[0]* exp( (x<=([1]+[3]))*(-0.5*((x-[1])^2)/([2]^2)) + (x>=[1]+[3])*(-0.5*([3]^2/[2]^2)-(x-[1]-[3])/[4])  )", -100.0, 100.0);

    std::string histoname = m_name+"_tritrig_zVtx_"+cutname+"_h";
    if(histos1d[histoname]->GetEntries() < 1)
        return -4.3;

    TFitResultPtr gausResult = (TFitResultPtr)histos1d[histoname]->Fit("gaus","QS"); 
    double gaus0 = gausResult->GetParams()[0];
    double gaus1 = gausResult->GetParams()[1];
    double gaus2 = gausResult->GetParams()[2];
    gausResult = histos1d[histoname]->Fit("gaus","QS","",gaus1-3.0*gaus2, gaus1+10.0*gaus2);
    gaus0 = gausResult->GetParams()[0];
    gaus1 = gausResult->GetParams()[1];
    gaus2 = gausResult->GetParams()[2];
    double xmin = gaus1 - 2.5*gaus2;
    double xmax = histos1d[histoname]->GetBinCenter(histos1d[histoname]->FindLastBinAbove(0.0)+1);
    xmax = 100.0;

    //Fit function for a few different seeds for tail start, and length, keep the best fit
    double tailZ = gaus1 + 3.0*gaus2;
    double tail_l = 50.0;
    TRandom3* ran = new TRandom3();
    ran->SetSeed(0);

    double best_chi2 = 99999.9;
    double best_tailZ;
    double best_tail_l;
    double best_gaus0, best_gaus1, best_gaus2;

    int iteration = 80;
    for(int i =10; i < iteration; i=i+2){
        tail_l = 50.0;
        tailZ = gaus1 +(double)(iteration/10.0)*gaus2;
        fitFunc->SetParameters(gaus0, gaus1, gaus2, tailZ, tail_l);
        TFitResultPtr fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "LSIM", "", xmin,xmax);
        if(fitResult->Ndf() <= 0)
            continue;
        if(fitResult->Chi2() < best_chi2){
            best_chi2 = fitResult->Chi2()/fitResult->Ndf();
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
    while(testIntegral > max_tail_events){
        zcut = zcut+0.1;
        testIntegral = fitFunc->Integral(zcut, 100.0);
    }
    fitFunc->Draw();
    delete ran;
    return zcut;
}

double ZBiHistos::fitZTail(std::string zVtxHistoname, double max_tail_events){
    TF1* fitFunc = new TF1("fitfunc","[0]*exp( (((x-[1])/[2])<[3])*(-0.5*(x-[1])^2/[2]^2) + (((x-[1])/[2])>=[3])*(0.5*[3]^2-[3]*(x-[1])/[2]))", -100.0, 100.0);
    
    std::string histoname = m_name+"_"+zVtxHistoname;
    if(histos1d[histoname]->GetEntries() < 1)
        return -4.3;
    TFitResultPtr gausResult = (TFitResultPtr)histos1d[histoname]->Fit("gaus","QS"); 
    double gaus1 = gausResult->GetParams()[1];
    double gaus2 = gausResult->GetParams()[2];
    gausResult = histos1d[histoname]->Fit("gaus","QS","",gaus1-3.0*gaus2, gaus1+10.0*gaus2);
    gaus1 = gausResult->GetParams()[1];
    gaus2 = gausResult->GetParams()[2];
    double tailZ = gaus1 + 3.0*gaus2;

    fitFunc->SetParameters(gausResult->GetParams()[0], gaus1, gaus2, 3.0);
    TFitResultPtr fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "LSIM", "", gaus1-2.0*gaus2, gaus1+10.0*gaus2);
    
    double zcut = -6.0;
    double testIntegral = fitFunc->Integral(zcut, 90.0);
    while(testIntegral > max_tail_events){
        zcut = zcut+0.1;
        testIntegral = fitFunc->Integral(zcut, 90.0);
    }
    fitFunc->Draw();
    return zcut;
}

void ZBiHistos::writeGraphs(TFile* outF, std::string folder){
    if (outF) outF->cd();
    TDirectory* dir{nullptr};
    std::cout<<folder.c_str()<<std::endl;
    if (!folder.empty()) {
        dir = outF->mkdir(folder.c_str(),"",true);
        dir->cd();
    }
    for(std::map<std::string,TGraph*>::iterator it=graphs_.begin(); it != graphs_.end(); ++it){
        if (!it->second){
            std::cout<<it->first<<" Null ptr in saving.."<<std::endl;
            continue;
        }
        it->second->Write();
    }
}

void ZBiHistos::writeHistos(TFile* outF, std::string folder) {
    if (outF) outF->cd();
    TDirectory* dir{nullptr};
    std::cout<<folder.c_str()<<std::endl;
    if (!folder.empty()) {
        dir = outF->mkdir(folder.c_str(),"",true);
        dir->cd();
    }
    for (it1d it = histos1d.begin(); it!=histos1d.end(); ++it) {
        if (!it->second){
            std::cout<<it->first<<" Null ptr in saving.."<<std::endl;
            continue;
        }
        it->second->Write();
    }
    for (it2d it = histos2d.begin(); it!=histos2d.end(); ++it) {
        std::cout << it->first << std::endl;
        if (!it->second){
            std::cout<<it->first<<" Null ptr in saving.."<<std::endl;
            continue;
        }
        it->second->Write();
    }
}

