#include "ZBiHistos.h"
#include <sstream>

ZBiHistos::ZBiHistos() {
}

ZBiHistos::~ZBiHistos() {
}

ZBiHistos::ZBiHistos(const std::string& inputName){
    m_name = inputName;
}

void ZBiHistos::addHistoClone1d(TH1F* parentHisto, std::string clone_histoname){
    std::string histoname = m_name + clone_histoname;
    TH1F* clone = (TH1F*)parentHisto->Clone(histoname.c_str());    
    histos1d[histoname] = clone;
}

void ZBiHistos::addHisto1d(std::string histoname, std::string xtitle, int nbinsX, float xmin, float xmax){
    histos1d[m_name+"_"+histoname] = plot1D(m_name+"_"+histoname, xtitle, nbinsX, xmin, xmax);
}

void ZBiHistos::addHisto2d(std::string histoname, std::string xtitle, int nbinsX, float xmin, float xmax, std::string ytitle, int nbinsY, float ymin, float ymax){
    histos2d[m_name+"_"+histoname] = plot2D(m_name+"_"+histoname, xtitle, nbinsX, xmin, xmax, ytitle, nbinsY, ymin, ymax);
}

void ZBiHistos::resetHistograms1d(){
    for(it1d it=histos1d.begin(); it != histos1d.end(); it ++){
        it->second->Reset();
    }
}

void ZBiHistos::resetHistograms2d(){
    for(it2d it=histos2d.begin(); it != histos2d.end(); it ++){
        it->second->Reset();
    }
}

/*
void ZBiHistos::getVdSelZEff(std::string cutname, double zcut, TH1F* vdSimZ_h){
   //Get the 2D histogram of signal unc_vtx_z vs true_vtx_z for the corresponding Test Cut
   //Take the Y projection of unc_vtx_z < zcut to be the signal_vdSelZ
   TH2F* vtx_z_hh = (TH2F*)histos2d[m_name+"_unc_vtx_z_vs_true_vtx_z_"+cutname+"_hh"];
   std::cout << "Taking y projection of unc_v_true_vtx_z between " << vtx_z_hh->GetXaxis()->FindBin(zcut)+1 <<
       " and " << vtx_z_hh->GetXaxis()->GetNbins() << std::endl;
   TH1F* true_vtx_z_h = (TH1F*)vtx_z_hh->ProjectionY((m_name+"_"+cutname+"_"+"true_vtx_z_projy").c_str(),vtx_z_hh->GetXaxis()->FindBin(zcut)+1,vtx_z_hh->GetXaxis()->GetNbins(),"");

   //Initialize the vd selection histogram by cloning the structure of vdSimZ
   TH1F* vdSelZ_h = (TH1F*)vdSimZ_h->Clone(("signal_vdSelZ_"+cutname+"_h").c_str());
   for(int i=0; i<201; i++){
       vdSelZ_h->SetBinContent(i,vdSelZ_h->GetBinContent(i));
   }

   //take the efficiency of vdSelZ to vdSimZ to get F(z)
   TEfficiency* effCalc_h = new TEfficiency(*vdSelZ_h, *vdSimZ_h_);

}
*/

/*
bool ZBiHistos::setTestZtailNevents() {

    TF1* fitFunc = new TF1("fitfunc", "[0]* exp( (x<=([1]+[3]))*(-0.5*((x-[1])^2)/([2]^2)) + (x>=[1]+[3])*(-0.5*([3]^2/[2]^2)-(x-[1]-[3])/[4])  )", -100.0, 100.0);
    std::string histoname = m_name+"_zVtx_h";
    //if(histos1d[histoname]->GetEntries() < 1)

    TFitResultPtr gausResult = (TFitResultPtr)histos1d[histoname]->Fit("gaus","QS"); 
    double gaus1 = gausResult->GetParams()[1];
    double gaus2 = gausResult->GetParams()[2];
    std::cout << "Gaus param 1: " << gaus1 << std::endl;
    std::cout << "Gaus param 2: " << gaus2 << std::endl;
    gausResult = histos1d[histoname]->Fit("gaus","QS","",gaus1-3.0*gaus2, gaus1+10.0*gaus2);
    gaus1 = gausResult->GetParams()[1];
    gaus2 = gausResult->GetParams()[2];
    double tailZ = gaus1 + 3.0*gaus2;
    fitFunc->SetParameters(gausResult->GetParams()[0], gaus1, gaus2, gaus1+5.0*gaus2, 100.0);
    TFitResultPtr fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "LSIM", "", gaus1-3.0*gaus2, 150.0);
    double b = fitResult->GetParams()[3];
    double l = fitResult->GetParams()[4];

    double zcut = -6.0;
    double testIntegral = fitFunc->Integral(zcut, 90.0);
    while(testIntegral > max_tail_events){
        zcut = zcut+0.1;
        testIntegral = fitFunc->Integral(zcut, 90.0);
    }
    fitFunc->Draw();
    return zcut;
}
*/

double ZBiHistos::getIntegral(std::string histoname){
    int xmax; 
    int xmin;
    double integral;

    if(!histos1d.count(histoname)){
        std::cout << "NO HISTOGRAM NAMED " << histoname << " FOUND IN DEFINED HISTOS";
        integral = -9999.9;
    }
    else{
        xmax = histos1d[histoname]->FindLastBinAbove(0.0);
        xmin = histos1d[histoname]->FindFirstBinAbove(0.0);
        integral = histos1d[histoname]->Integral(xmin,xmax);
    }
    return integral;
}

void ZBiHistos::change1dHistoTitle(std::string histoname, std::string title){
    histos1d[histoname]->SetTitle(title.c_str());
}

double ZBiHistos::cutFractionOfIntegral(std::string histoname, bool isCutGreaterThan, double cutFraction, double initialIntegral){

    TH1F* histo = histos1d[histoname];
    int xmax = histo->FindLastBinAbove(0.0);
    int xmin = histo->FindFirstBinAbove(0.0);
    if(debug_){
        std::cout << "[ZBiHistos] Initial integral for " << histoname << ": " << initialIntegral << std::endl;
        std::cout << "Init xmin: " << xmin << " | Init xmax: " << xmax << std::endl;
    }
    
    double cutvalue;
    if(isCutGreaterThan){
       cutvalue = histo->GetXaxis()->GetBinLowEdge(xmin); 
       //std::cout << "initial cut value: " << cutvalue << std::endl;
       //std::cout << "first integral: " << histo->Integral(xmin,xmax) << std::endl;
       while(histo->Integral(xmin,xmax) > initialIntegral*(1.0-cutFraction)){
           //std::cout << "iter integral: " << histo->Integral(xmin,xmax);
           xmin = xmin + 1;
           cutvalue = histo->GetXaxis()->GetBinLowEdge(xmin);
           //std::cout << "update xmin: " << xmin << std::endl;
           //std::cout << "iter cut value: " << cutvalue << std::endl;
       }
    }
    else {
       cutvalue = histo->GetXaxis()->GetBinUpEdge(xmax); 
       //std::cout << "initial cut value: " << cutvalue << std::endl;
       //std::cout << "first integral: " << histo->Integral(xmin,xmax) << std::endl;
       while(histo->Integral(xmin,xmax) > initialIntegral*(1.0-cutFraction)){
           std::cout << "iter integral: " << histo->Integral(xmin,xmax);
           xmax = xmax - 1;
           cutvalue = histo->GetXaxis()->GetBinUpEdge(xmax);
           //std::cout << "update xmax: " << xmax << std::endl;
           //std::cout << "iter cut value: " << cutvalue << std::endl;
       }
    }

    if(debug_){
        std::cout << "[ZBiHistos] New Integral: " << histo->Integral(xmin,xmax) << std::endl;
        std::cout << "[ZBiHistos] (New integral/Initial integral) = " << histo->Integral(xmin,xmax)/initialIntegral << 
            std::endl;
        std::cout << "[ZBiHistos] cut value = " << cutvalue << std::endl;
    }
    return cutvalue;
}

void ZBiHistos::defineCutlistHistos(std::map<std::string,std::pair<double,int>> cutmap){
    for(std::map<std::string,std::pair<double,int>>::iterator it=cutmap.begin(); it != cutmap.end(); it++){
        std::string cutname = it->first;

        //unv_vtx_z vs true_vtx_z
        addHisto2d("unc_vtx_z_vs_true_vtx_z_"+cutname+"_hh","unc z_{vtx} [mm]", 1500, -50.0, 100.0,"true z_{vtx} [mm]",200,-50.3,149.7); 
        //vdSelZ
        addHisto1d("signal_vdSelZ_"+cutname+"_h","true z_{vtx} [mm]",200, -50.3, 149.7);
        //tritrig zVtx
        addHisto1d("tritrig_zVtx_"+cutname+"_h","unc z_{vtx} [mm]",150, -50.0, 100.0);

        //TGraphs

        //ztail fit quality
        TGraph* tgraph = new TGraph();
        tgraph->SetName((m_name+"_tritrig_zVtx_fit_chi2_"+cutname+"_g").c_str());
        graphs_[m_name+"_tritrig_zVtx_fit_chi2_"+cutname+"_g"] = tgraph;
    }
}

void ZBiHistos::defineAnalysisHistos(){

    addHisto2d("z0_v_recon_z_hh","recon_z [mm]", 450, -20.0, 70.0, "z0 [mm]", 1000,-10.0,10.0);
    addHisto2d("z0_v_recon_z_post_cut_hh","recon_z [mm]", 450, -20.0, 70.0, "z0 [mm]", 1000,-10.0,10.0);
    addHisto2d("z0_v_recon_z_alpha_hh","recon_z [mm]", 450, -20.0, 70.0, "z0 [mm]", 1000,-10.0,10.0);
    addHisto2d("z0_v_recon_z_alpha_post_cut_hh","recon_z [mm]", 450, -20.0, 70.0, "z0 [mm]", 1000,-10.0,10.0);
    addHisto1d("impact_parameter_up_h","recon_z [mm]", 450, -20.0, 70.0);
    addHisto1d("impact_parameter_down_h","recon_z [mm]", 450, -20.0, 70.0);
    //zalpha 
    addHisto1d("z_alpha_h","z_alpha", 450, -20.0, 70.0);
}

void ZBiHistos::defineIterHistos(){
    /*
    addHisto2d("persistent_cuts_hh","iteration", 100,-0.5,99.5,"cut_id",25,0.5,25.5);
    addHisto2d("test_cuts_ZBi_hh","iteration", 100,-0.5,99.5,"cut_id",25,0.5,25.5);
    addHisto2d("test_cuts_values_hh","iteration", 100,-0.5,99.5,"cut_id",25,0.5,25.5);
    addHisto2d("test_cuts_zcut_hh","iteration", 100,-0.5,99.5,"zcut",25,0.5,25.5);
    addHisto2d("test_cuts_nbkg_hh","iteration", 100,-0.5,99.5,"zcut",25,0.5,25.5);
    addHisto2d("test_cuts_nsig_hh","iteration", 100,-0.5,99.5,"zcut",25,0.5,25.5);
    addHisto2d("best_test_cut_ZBi_hh","iteration", 100,-0.5,99.5,"ZBi",2000,0.0,20.0);
    */
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

std::vector<double> ZBiHistos::impactParameterCut(){

    //Loop over 2d histo of z0 vs recon_z, taking projection at each recon_z
    //Find the value of z0 in each projection that cuts 15% of signal
    TH2F* hh = (TH2F*)histos2d[m_name+"_z0_v_recon_z_hh"];
    TH1F* up_h = (TH1F*)histos1d[m_name+"_impact_parameter_up_h"];
    TH1F* down_h = (TH1F*)histos1d[m_name+"_impact_parameter_down_h"];

    std::cout << "Nbins in impactParameterCut: " << std::to_string(hh->GetEntries()) << std::endl;
    for(int i=0; i < hh->GetNbinsX(); i++){

        TH1F* projy = (TH1F*)hh->ProjectionY(("projy_bin_"+std::to_string(i+1)).c_str(),i+1,i+1);
        if(projy->GetEntries() < 1) continue;

        //Impact param for z0 > 0
        int start_bin = projy->FindBin(0.0);
        int end_bin = projy->FindLastBinAbove(0.0); 
        double refIntegral = projy->Integral(start_bin,end_bin);
        int cutz0_bin = start_bin; 
        double testIntegral = refIntegral;
        while(testIntegral > 0.90*refIntegral && cutz0_bin < end_bin-1){
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
        while(testIntegral > 0.90*refIntegral && cutz0_bin > end_bin+1){
            cutz0_bin = cutz0_bin - 1;           
            testIntegral = projy->Integral(end_bin, cutz0_bin);
        }
        
        double cutz0_down = projy->GetXaxis()->GetBinUpEdge(cutz0_bin);
        for(int j=0; j<(int)refIntegral; j++){
            down_h->Fill(hh->GetXaxis()->GetBinCenter(i+1),cutz0_down/refIntegral);
        }
    }

    //TF1* fitFunc = new TF1("linear_fit","[0] + [1]*x",5.0,70.0);
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
    double alpha = x;
    std::vector<double> params {m_p,a_p,m_d,a_d, beta, alpha};

    delete fitFunc;
    return params;
}

TF1* ZBiHistos::fitZTailWithExp(std::string cutname){
    TF1* fitFunc = new TF1("fitfunc","[0]*exp([1]*x)",10.0,100.0);

    std::string histoname = m_name+"_tritrig_zVtx_"+cutname+"_h";
    if(histos1d[histoname]->GetEntries() < 1){
        std::cout << "WARNING: Background Model is NULL: " << 
            cutname << " tritrig zVtx distribution was empty and could not be fit!" << std::endl;
        return nullptr;
    }
    TFitResultPtr fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "QLSIM", "", 10.0,100.0);
    double parm0 = fitResult->Parameter(0);
    double parm1 = fitResult->Parameter(1);

    double best_chi2 = 9999999.9;
    double best_parm0, best_parm1;

    int iteration = 20;
    for(int i=0; i < iteration; i++){
        fitFunc->SetParameters(parm0,parm1);
        fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "QLSIM", "", 10.0,100.0);
        if(fitResult->Ndf() <= 0)
            continue;
        if(fitResult->Chi2()/fitResult->Ndf() < best_chi2){
            best_chi2 = fitResult->Chi2()/fitResult->Ndf();
            best_parm0 = fitResult->Parameter(0);
            best_parm1 = fitResult->Parameter(1);
        }
    }

    fitFunc->SetParameters(best_parm0,best_parm1);
    fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "LSIM", "", 10.0, 100.0);
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

    /*
    int iteration = 50;
    for(int i=0; i < iteration; i++){
        tailZ = gaus1 + ran->Uniform(1.0,8.0)*gaus2;
        tail_l = ran->Uniform(0.0,80.0);
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
        }
    }
    */


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
    //gausResult = histos1d[histoname]->Fit("gaus","QS","",gaus1-2.5*gaus2, gaus1+10.0*gaus2);
    gausResult = histos1d[histoname]->Fit("gaus","QS","",gaus1-3.0*gaus2, gaus1+10.0*gaus2);
    gaus0 = gausResult->GetParams()[0];
    gaus1 = gausResult->GetParams()[1];
    gaus2 = gausResult->GetParams()[2];
    double xmin = gaus1 - 2.5*gaus2;
    double xmax = histos1d[histoname]->GetBinCenter(histos1d[histoname]->FindLastBinAbove(0.0)+1);
    xmax = 100.0;
    std::cout << "gaus0: " << gaus0 << std::endl;
    std::cout << "gaus1: " << gaus1 << std::endl;
    std::cout << "gaus2: " << gaus2 << std::endl;
    std::cout << "xmin: " << xmin << std::endl;

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
            //best_tailZ = tailZ;
            //best_tail_l = tail_l;
            best_gaus0 = fitResult->Parameter(0);
            best_gaus1 = fitResult->Parameter(1);
            best_gaus2 = fitResult->Parameter(2);
            best_tailZ = fitResult->Parameter(3);
            best_tail_l = fitResult->Parameter(4);
        }
    }

    fitFunc->SetParameters(best_gaus0, best_gaus1, best_gaus2, best_tailZ, best_tail_l);
    //fitFunc->SetParameters(gaus0, gaus1, gaus2, best_tailZ, best_tail_l);
    TFitResultPtr fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "LSIM", "", xmin, xmax);
    

    /** //works okay, but try just incrementing the tailZ value
    int iteration = 30;
    std::cout << "ITERATIVE GAUS FIT WITH RANDOM SEEDS" << std::endl;
    for(int i =0; i < iteration; i++){
        tailZ = gaus1 + ran->Uniform(3.0,8.0)*gaus2;
        tail_l = ran->Uniform(30.0,80.0);
        std::cout << "tailZ: " << tailZ << std::endl;
        std::cout << "tail_l: " << tail_l << std::endl;
        fitFunc->SetParameters(gaus0, gaus1, gaus2, tailZ, tail_l);
        std::cout << "set param tailZ: " << tailZ << std::endl;
        std::cout << "set param tail_l: " << tail_l << std::endl;
        TFitResultPtr fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "LSIM", "", xmin,xmax);
        std::cout << "fitresult tailZ: " << tailZ << std::endl;
        std::cout << "fitresult tail_l: " << tail_l << std::endl;
        std::cout << "fit result chi2: " << fitResult->Chi2() << std::endl;
        if(fitResult->Chi2() < best_chi2){
            std::cout << "if better chi2 tailZ: " << tailZ << std::endl;
            std::cout << "if better chi2 tail_l: " << tail_l << std::endl;
            std::cout << "if better Best tailZ " << best_tailZ << std::endl;
            std::cout << "if better Best tail_l " << best_tail_l << std::endl;
            best_chi2 = fitResult->Chi2();
            best_tailZ = tailZ;
            best_tail_l = tail_l;
            std::cout << "updted best tailZ " << best_tailZ << std::endl;
            std::cout << "updtaed Best tail_l " << best_tail_l << std::endl;
        }
        std::cout << "Best tailZ " << best_tailZ << std::endl;
        std::cout << "Best tail_l " << best_tail_l << std::endl;
    }
    std::cout << "FINAL FIT" << std::endl;
    std::cout << "Best tailZ " << best_tailZ << std::endl;
    std::cout << "Best tail_l " << best_tail_l << std::endl;
    fitFunc->SetParameters(gaus0, gaus1, gaus2, best_tailZ, best_tail_l);
    TFitResultPtr fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "LSIM", "", xmin, xmax);
    double ndf = fitResult->Ndf(); 

    std::cout << "MAKING GRAPH" << std::endl;
    //Graph the final fit chi2 for the test cut
    TGraph* g = graphs_[m_name+"_tritrig_zVtx_fit_chi2_"+cutname+"_g"];
    g->SetPoint(g->GetN(),(double)g->GetN(),best_chi2/ndf);
    */
    
    
    //fitFunc->SetParameters(gausResult->GetParams()[0], gaus1, gaus2, gaus1+3.0*gaus2, 50.0);
    //TFitResultPtr fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "LSIM", "", gaus1-3.0*gaus2, 100.0);
    
    std::cout << "Integrating fit" << std::endl;
    double zcut = -6.0;
    double testIntegral = fitFunc->Integral(zcut, 100.0);
    while(testIntegral > max_tail_events){
        zcut = zcut+0.1;
        testIntegral = fitFunc->Integral(zcut, 100.0);
    }
    fitFunc->Draw();
    std::cout << "Done integrating" << std::endl;
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
        if (!it->second){
            std::cout<<it->first<<" Null ptr in saving.."<<std::endl;
            continue;
        }
        it->second->Write();
    }
}

