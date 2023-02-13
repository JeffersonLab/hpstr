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
    histo->ClearUnderflowAndOverflow();
    int xmax = histo->FindLastBinAbove(0.1);
    int xmin = histo->FindFirstBinAbove(0.1);
    if(debug_){
        std::cout << "[ZBiHistos] Initial integral for " << histoname << ": " << initialIntegral << std::endl;
    }
    
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

    if(debug_){
        std::cout << "[ZBiHistos] Initial integral for " << histoname << ": " << initialIntegral << std::endl;
        std::cout << "[ZBiHistos] (New integral/Initial integral) = " << histo->Integral(xmin,xmax)/initialIntegral << 
            std::endl;
        std::cout << "[ZBiHistos] cut value = " << cutvalue << std::endl;
    }
    return cutvalue;
}

void ZBiHistos::defineCutlistHistos(std::map<std::string,std::pair<double,int>> cutmap){
    for(std::map<std::string,std::pair<double,int>>::iterator it=cutmap.begin(); it != cutmap.end(); it++){
        std::string cutname = it->first;


        //vdSelZ
        addHisto1d("signal_vdSelZ_"+cutname+"_h","true z_{vtx} [mm]",200, -50.3, 149.7);
        //tritrig zVtx
        addHisto1d("tritrig_zVtx_"+cutname+"_h","unc z_{vtx} [mm]",150, -50.0, 100.0);
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

    TF1* fitFunc = new TF1("linear_fit","[0] + [1]*x",5.0,70.0);
    TFitResultPtr fitResult = (TFitResultPtr)up_h->Fit("linear_fit","QS","",5.0,70.0);
    fitFunc->Draw();
    double a_p = fitResult->GetParams()[0];
    double b_p = fitResult->GetParams()[1];

    fitResult = (TFitResultPtr)down_h->Fit("linear_fit","QS","",5.0,70.0);
    fitFunc->Draw();
    double a_d = fitResult->GetParams()[0];
    double b_d = fitResult->GetParams()[1];

    //Find the location in z0 where two lines meet
    double diff = 999.9;
    double x = 10.0;
    while( std::abs((a_p+b_p*x) - (a_d+b_d*x)) < diff ){
        diff = std::abs((a_p+b_p*x) - (a_d+b_d*x));    
        x = x - 0.01;
    }
    double beta = .5*((a_p+b_p*x) + (a_d+b_d*x));
    double alpha = x;
    std::vector<double> params {a_p,b_p,a_d,b_d, beta, alpha};

    delete fitFunc;
    return params;
}


double ZBiHistos::shosFitZTail(std::string cutname, double max_tail_events){
    TF1* fitFunc = new TF1("fitfunc", "[0]* exp( (x<=([1]+[3]))*(-0.5*((x-[1])^2)/([2]^2)) + (x>=[1]+[3])*(-0.5*([3]^2/[2]^2)-(x-[1]-[3])/[4])  )", -100.0, 100.0);

    std::string histoname = m_name+"_tritrig_zVtx_"+cutname+"_h";
    if(histos1d[histoname]->GetEntries() < 1)
        return -4.3;

    TFitResultPtr gausResult = (TFitResultPtr)histos1d[histoname]->Fit("gaus","QS"); 
    double gaus1 = gausResult->GetParams()[1];
    double gaus2 = gausResult->GetParams()[2];
    std::cout << "Gaus param 1: " << gaus1 << std::endl;
    std::cout << "Gaus param 2: " << gaus2 << std::endl;
    gausResult = histos1d[histoname]->Fit("gaus","QS","",gaus1-3.0*gaus2, gaus1+10.0*gaus2);
    gaus1 = gausResult->GetParams()[1];
    gaus2 = gausResult->GetParams()[2];
    //double b = gausResult->GetParams()[3];
    //double l = gausResult->GetParams()[4];
    double tailZ = gaus1 + 3.0*gaus2;
    //fitFunc->SetParameters(gausResult->GetParams()[0], gaus1, gaus2, gausResult->GetParams()[3],gausResult->GetParams()[4]);
    fitFunc->SetParameters(gausResult->GetParams()[0], gaus1, gaus2, gaus1+5.0*gaus2, 100.0);
    //TFitResultPtr fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "LSIM", "", gaus1-2.0*gaus2, gaus1+10.0*gaus2);
    TFitResultPtr fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "LSIM", "", gaus1-3.0*gaus2, 150.0);
    
    double zcut = -6.0;
    double testIntegral = fitFunc->Integral(zcut, 90.0);
    while(testIntegral > max_tail_events){
        zcut = zcut+0.1;
        testIntegral = fitFunc->Integral(zcut, 90.0);
    }
    fitFunc->Draw();
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
void ZBiHistos::writeHistos(TFile* outF, std::string folder) {
    if (outF) outF->cd();
    TDirectory* dir{nullptr};
    std::cout<<folder.c_str()<<std::endl;
    if (!folder.empty()) {
        dir = outF->mkdir(folder.c_str());
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

