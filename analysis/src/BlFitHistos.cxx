#include "BlFitHistos.h"
#include <math.h>
#include "TCanvas.h"
#include <vector>
BlFitHistos::BlFitHistos(const std::string& inputName) {
    m_name = inputName;
    mmapper_ = new ModuleMapper();
}

BlFitHistos::~BlFitHistos() {
    for (std::map<std::string, TGraphErrors*>::iterator it = baselineGraphs.begin();
                it!=baselineGraphs.end(); ++it) {
            if (it->second) {
                delete (it->second);
                it->second = nullptr;
            }
        }
        baselineGraphs.clear();
}

void BlFitHistos::Mean2DHistoOverlay(HistoManager* inputHistos_, BlFitHistos* outputHistos_) {
//Loop over all 2D histogram names from the input TFile
    for (std::vector<std::string>::iterator jj = inputHistos_->histos2dNamesfromTFile.begin();
        jj != inputHistos_->histos2dNamesfromTFile.end(); ++jj)
    {

    std::string inputname = *jj;
    std::string FitRangeLowerkey;
    std::string FitRangeUpperkey;
    std::string meankey;
    std::string widthkey;
    std::string normkey;
    std::cout << "Sensor: " << inputname << std::endl;

    for (std::vector<std::string>::iterator it = outputHistos_->histos1dNamesfromJson.begin();
                it != outputHistos_->histos1dNamesfromJson.end(); ++it)
    {
        std::string fitparname = *it;
        size_t L = fitparname.find_last_of("L");
        std::string tag = fitparname.substr(L);
        if (inputname.find(tag) != std::string::npos) {
            //Substrings below must match the names in the JSON file
            if (fitparname.find("FitRangeLower") != std::string::npos)
                FitRangeLowerkey = fitparname;
            if (fitparname.find("FitRangeUpper") != std::string::npos)
                FitRangeUpperkey = fitparname;
            if (fitparname.find("mean") != std::string::npos)
                meankey = fitparname;
            if (fitparname.find("width") != std::string::npos)
                widthkey = fitparname;
            if (fitparname.find("norm") != std::string::npos)
                normkey = fitparname;

        }
    }

    TH2F* histo_hh = inputHistos_->get2dHisto(*jj);
    TCanvas canvas(Form("Mean_Fit_Overlay_%s",histo_hh->GetName()),"c",1800,800);
    //TCanvas canvas(projy_h->GetName(),"c",1800,800);

    histo_hh->Draw("colz");
    outputHistos_->get1dHisto(meankey)->SetLineColor(2);
    outputHistos_->get1dHisto(meankey)->Draw("SAME");
    canvas.Write();
    histo_hh->Write();
    outputHistos_->get1dHisto(meankey)->Write();

 



    }
}

void BlFitHistos::FillHistograms() {

}


void BlFitHistos::Chi2GausFit( HistoManager* inputHistos_, BlFitHistos* outputHistos_, int nPointsDer_,int rebin_) {
//Loop over all 2D histogram names from the input TFile
    for (std::vector<std::string>::iterator jj = inputHistos_->histos2dNamesfromTFile.begin();
        jj != inputHistos_->histos2dNamesfromTFile.end(); ++jj)
    {

     //Create histograms for fit parameters to fill
    std::string inputname = *jj;
    std::string FitRangeLowerkey;
    std::string FitRangeUpperkey;
    std::string meankey;
    std::string widthkey;
    std::string normkey;
    std::cout << "Sensor: " << inputname << std::endl;

    for (std::vector<std::string>::iterator it = outputHistos_->histos1dNamesfromJson.begin();
                it != outputHistos_->histos1dNamesfromJson.end(); ++it)
    {
        std::string fitparname = *it;
        size_t L = fitparname.find_last_of("L");
        std::string tag = fitparname.substr(L);
        if (inputname.find(tag) != std::string::npos) {
            //Substrings below must match the names in the JSON file
            if (fitparname.find("FitRangeLower") != std::string::npos)
                FitRangeLowerkey = fitparname;
            if (fitparname.find("FitRangeUpper") != std::string::npos)
                FitRangeUpperkey = fitparname;
            if (fitparname.find("mean") != std::string::npos)
                meankey = fitparname;
            if (fitparname.find("width") != std::string::npos)
                widthkey = fitparname;
            if (fitparname.find("norm") != std::string::npos)
                normkey = fitparname;

        }
    }


    TH2F* histo_hh = inputHistos_->get2dHisto(*jj);
    std::string sensorname = histo_hh->GetName();
    histo_hh->RebinY(rebin_);
    int  nbins = histo_hh->GetXaxis()->GetNbins();

    TH1D* fit_histo_h = new  TH1D(Form("Max_Chi2_2nd_Derivative_for_Sensor%s",histo_hh->GetName()),"Max_Chi2_2nd_Derivative_per_Channel",10000,0,10000);

    //Loop over all channels to find location of maximum chi2 2nd derivative
    for(int cc=0; cc < 640; ++cc) {
        
        std::cout << "Channel #" << cc << std::endl;
        TH1D* projy_h = histo_hh->ProjectionY(Form("%s_projection_%i",histo_hh->GetName(),cc),
                cc+1,cc+1,"e");
        projy_h->SetTitle(Form("ProjectionY_%s_Channel%i",histo_hh->GetName(),cc));
        if(projy_h->GetEntries() < 1000){continue;}

        int iter=0;
        int firstbin=projy_h->FindFirstBinAbove(10,1);
        double xmin=projy_h->GetBinLowEdge(firstbin);
        double binwidth=1*projy_h->GetBinWidth(firstbin);
        double xmax=xmin+20*binwidth;
        std::vector<double> amp,mean,sigma,chi2,const_err,sigma_err,
               fit_range_end,chi2_NDF,chi2_2D;
        std::vector<int> NDF;


    //Iterative Fitting Procedure. Used to lcoate  maximum chi2 2nd derivative along x-axis
    //This appears to correlate directly to the location where the baseline gaussian signal
    //ends, and the pileup threshold begins. This x-axis location is then used as the 
    //end of the iterative fit range window, fiting JUST the gaussian.

    while(iter < projy_h->GetNbinsX()-firstbin && xmax < 10000){
        TF1* cc_fit = new TF1("cc_fit", "gaus", xmin, xmax);
        projy_h->Fit(cc_fit, "QRES");

        if(cc_fit->GetNDF() == 0){
            xmax = xmax + 2*binwidth;
            continue;
        }

        //Collect fit parameters                                 
        amp.push_back(cc_fit->GetParameter(0));
        mean.push_back(cc_fit->GetParameter(1));
        sigma.push_back(cc_fit->GetParameter(2));
        chi2.push_back(cc_fit->GetChisquare());
        fit_range_end.push_back(xmax);
        NDF.push_back(cc_fit->GetNDF());
        chi2_NDF.push_back(chi2.at(iter)/NDF.at(iter));
        //Increase fit range window and iterate the fit
        xmax = xmax + 2*binwidth;
        iter = iter + 1;
        delete cc_fit;
    }

    //Calculate the 2nd derivative of chi2 by taking slope on either side of one point
    //and taking the difference between the two.
    for(int i=0; i < chi2_NDF.size(); ++i) {
        if( i > nPointsDer_ && i < (chi2_NDF.size() - nPointsDer_)){
            //std::cout << "chi2+n: " << chi2_NDF.at(i+nPointsDer_) << std::endl;
            //std::cout << "chi2: " << chi2_NDF.at(i) << std::endl;
            //std::cout << "chi2-n: " << chi2_NDF.at(i-nPointsDer_) << std::endl;
            double slope2 = (chi2_NDF.at(i+nPointsDer_)-chi2_NDF.at(i))/nPointsDer_*binwidth;
            double slope1 = (chi2_NDF.at(i)-chi2_NDF.at(i-nPointsDer_))/nPointsDer_*binwidth;
            double slopeDiff = slope2 - slope1;
            //std::cout << "xposition: " << fit_range_end.at(i) << "; slopeDiff: " << slopeDiff << std::endl;
            chi2_2D.push_back(slopeDiff);
            //chi2_2D.push_back((chi2_NDF.at(i+nPointsDer_)-chi2_NDF.at(i))/(nPointsDer_*binwidth) - ((chi2_NDF.at(i)-chi2_NDF.at(iter-nPointsDer_))/(nPointsDer_*binwidth)));
        }
    }

     //Remove any 'nan' entries from chi2_2D
     for( int i=0; i < chi2_2D.size(); ++i) {
        if(chi2_2D[i] != chi2_2D[i]) { chi2_2D[i]=0.0;}
    }

    double max_Element = *std::max_element(chi2_2D.begin(), chi2_2D.end());
    int maxElementIndex = std::max_element(chi2_2D.begin(), chi2_2D.end()) - chi2_2D.begin();
    //for(int i=0; i < chi2_2D.size();++i) {std::cout << "2nd derivatives" << chi2_2D.at(i) << std::endl;}
    //Re-run fit with xmax = location where 2nd derivative of chi2/NDF is maximum
    int back_off=0*nPointsDer_;
    xmax = fit_range_end.at(maxElementIndex-back_off);
    TF1* cc_fit = new TF1("cc_fit", "gaus", xmin, xmax);
    projy_h->Fit(cc_fit, "QRES");

    outputHistos_->get1dHisto(meankey)->SetBinContent(cc+1,(float)cc_fit->GetParameter(1));
    outputHistos_->get1dHisto(widthkey)->SetBinContent(cc+1,(float)cc_fit->GetParameter(2));
    outputHistos_->get1dHisto(normkey)->SetBinContent(cc+1,(float)cc_fit->GetParameter(0));
    outputHistos_->get1dHisto(FitRangeLowerkey)->SetBinContent(cc+1,(float)xmin);
    outputHistos_->get1dHisto(FitRangeUpperkey)->SetBinContent(cc+1,(float)xmax);

//Graphs and Histograms

    fit_histo_h->Fill(fit_range_end.at(maxElementIndex+nPointsDer_));

    int n=fit_range_end.size();
    TGraph* chi2_NDF_gr = new TGraph(n,fit_range_end.data(),chi2_NDF.data());
    chi2_NDF_gr->SetName(Form("Chi2_NDF_gr_%s_Channel%i",histo_hh->GetName(),cc));
    chi2_NDF_gr->SetTitle(Form("Chi2_vs_FitRanageEnd_%s_Channel%i",histo_hh->GetName(),cc));

    TGraph* mean_gr = new TGraph(n,fit_range_end.data(),mean.data());
    mean_gr->SetName(Form("Mean_gr_%s_Channel%i",histo_hh->GetName(),cc));
    mean_gr->SetTitle(Form("Mean_vs_FitRanageEnd_%s_Channel%i",histo_hh->GetName(),cc));

    std::vector<double>::const_iterator first = fit_range_end.begin()+nPointsDer_;
    std::vector<double>::const_iterator last=fit_range_end.begin()+nPointsDer_+chi2_2D.size();
    std::vector<double> chi2_2D_range(first,last);
    TGraph* chi2_2D_gr = new TGraph(chi2_2D.size(),chi2_2D_range.data(),chi2_2D.data());
    chi2_2D_gr->SetName(Form("chi2_2ndDerivative_gr_%s_Channel%i",histo_hh->GetName(),cc));
    chi2_2D_gr->SetTitle(Form("chi2_2ndDerivative_vs_FitRanageEnd_%s_Channel%i",
        histo_hh->GetName(),cc));

    //outF_chi2->cd();
    TCanvas canvas(projy_h->GetName(),"c",1800,800);
    projy_h->Draw();
    //canvas.Write();
    canvas.SaveAs(Form("run/fit_images/%s_fit.png",projy_h->GetName()));

    chi2_NDF_gr->Draw();
    //canvas.Write();
    canvas.SaveAs(Form("run/fit_images/%s_chi2.png",projy_h->GetName()));

    mean_gr->Draw();
    //canvas.Write();
    canvas.SaveAs(Form("run/fit_images/%s_mean.png",projy_h->GetName()));

    chi2_2D_gr->Draw();
    //canvas.Write();
    canvas.SaveAs(Form("run/fit_images/%s_chi2_2nd_derivative.png",projy_h->GetName()));

    //projy_h->Write();
    //chi2_NDF_gr->Write();
    //mean_gr->Write();
    //chi2_2D_gr->Write();

    delete chi2_NDF_gr;
    delete mean_gr;
    delete projy_h;
    delete chi2_2D_gr;
    delete cc_fit;
    }

    fit_histo_h->Write();

    }
}
