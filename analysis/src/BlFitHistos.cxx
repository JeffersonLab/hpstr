#include "BlFitHistos.h"
#include <math.h>
#include "TCanvas.h"
#include <vector>
#include "ModuleMapper.h"
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
}

void BlFitHistos::FillHistograms() {

}


void BlFitHistos::Chi2GausFit( HistoManager* inputHistos_, int nPointsDer_,int rebin_,int xmin_, int minStats_, FlatTupleMaker* flat_tuple_) {


      //Loop over all 2D histogram names from the input TFile
    for (std::vector<std::string>::iterator jj = inputHistos_->histos2dNamesfromTFile.begin();
        jj != inputHistos_->histos2dNamesfromTFile.end(); ++jj)
    {
        //Get input 2D histogram and rebin based on configuration setting
        TH2F* histo_hh = inputHistos_->get2dHisto(*jj);
        histo_hh->RebinY(rebin_);
        std::string SvtAna2DHisto_key = histo_hh->GetName();
        histo_hh->Write();
        SvtAna2DHisto_key.erase(SvtAna2DHisto_key.end()-3,SvtAna2DHisto_key.end());

    int  nbins = histo_hh->GetXaxis()->GetNbins();

    //TH1D* fit_histo_h = new  TH1D(Form("Max_Chi2_2nd_Derivative_for_Sensor%s",histo_hh->GetName()),"Max_Chi2_2nd_Derivative_per_Channel",10000,0,10000);

    //Loop over all channels to find location of maximum chi2 2nd derivative
    for(int cc=0; cc < 640; ++cc) {

        //Set Channel and Hybrid information in the flat tuple
        flat_tuple_->setVariableValue("SvtAna2DHisto_key", SvtAna2DHisto_key);
        flat_tuple_->setVariableValue("channel", cc);
        flat_tuple_->setVariableValue("minimum_bin_threshold",(double)xmin_);
        flat_tuple_->setVariableValue("minimum_entry_requirement_per_channel", (double)minStats_);
        flat_tuple_->setVariableValue("rebin_factor", (double)rebin_);
     


        if(cc%20 == 0){
            std::cout <<"Hybrid: "<< SvtAna2DHisto_key << " channel #" << cc << std::endl;
        }
        TH1D* projy_h = histo_hh->ProjectionY(Form("%s_projectionY_channel_%i",SvtAna2DHisto_key.c_str(),cc),
                cc+1,cc+1,"e");
        projy_h->SetTitle(Form("%s_projectionY_channel_%i",SvtAna2DHisto_key.c_str(),cc));

        //Minimum Entry Requirement NOT ROBUST!!!
        if(projy_h->GetEntries() < minStats_){
            
            flat_tuple_->setVariableValue("baseline_gausFit_mean", -9999.9);
            flat_tuple_->setVariableValue("baseline_gausFit_sigma", -9999.9);
            flat_tuple_->setVariableValue("baseline_gausFit_norm", -9999.9);
            flat_tuple_->setVariableValue("baseline_gausFit_range_lower", -9999.9);
            flat_tuple_->setVariableValue("baseline_gausFit_range_upper", -9999.9);
                
            flat_tuple_->addToVector("iterativeFit_mean", -9999.9);
            flat_tuple_->addToVector("iterativeFit_chi2_2ndDerivative",-9999.9);
            flat_tuple_->addToVector("iterativeFit_chi2_2Der_range",-9999.9);
            flat_tuple_->addToVector("iterativeFit_chi2_NDF",-9999.9);
            flat_tuple_->addToVector("iterativeFit_range_end", -9999.9);
            //If channel did not have enough entries to perform a fit, set minStats_dead_channel status to 1
            flat_tuple_->setVariableValue("minStats_dead_channel",1.0);
            continue;
        }

        //If minimum entry requirement is passed, set 'dead_channel' variable to 0
        flat_tuple_->setVariableValue("minStats_dead_channel",0.0);

        int iter=0;
        int firstbin=projy_h->FindFirstBinAbove(xmin_,1);
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

    while(iter < projy_h->GetNbinsX()-firstbin && xmax < 8000){

        TF1* cc_fit = new TF1("cc_fit", "gaus", xmin, xmax);
        projy_h->Fit(cc_fit, "QRES");

        if(cc_fit->GetNDF() == 0){
            xmax = xmax + 2*binwidth;
            continue;
        }

        //Get Iterative Fit Parameters and add to flat tuple vector                                
        flat_tuple_->addToVector("iterativeFit_mean", cc_fit->GetParameter(1));

        chi2.push_back(cc_fit->GetChisquare());
        NDF.push_back(cc_fit->GetNDF());
        chi2_NDF.push_back(chi2.at(iter)/NDF.at(iter));
        flat_tuple_->addToVector("iterativeFit_chi2_NDF", chi2_NDF.at(iter));

        fit_range_end.push_back(xmax);
        flat_tuple_->addToVector("iterativeFit_range_end", xmax);

        //Increase fit range window and iterate the fit
        xmax = xmax + 2*binwidth;
        iter = iter + 1;
        delete cc_fit;
    }

    //Calculate the 2nd derivative of chi2 by taking slope on either side of one point
    //and taking the difference between the two.
    for(int i=0; i < chi2_NDF.size(); ++i) {

        if( i > nPointsDer_ && i < (chi2_NDF.size() - nPointsDer_)){
            double slope2 = (chi2_NDF.at(i+nPointsDer_)-chi2_NDF.at(i))/nPointsDer_*binwidth;
            double slope1 = (chi2_NDF.at(i)-chi2_NDF.at(i-nPointsDer_))/nPointsDer_*binwidth;
            double slopeDiff = slope2 - slope1;
            chi2_2D.push_back(std::abs(slopeDiff));
        }
    }

     //Remove any 'nan' entries from chi2_2D
     for( int i=0; i < chi2_2D.size(); ++i) {

        if(chi2_2D[i] != chi2_2D[i]) { 
            chi2_2D[i]=0.0;
            //add chi2_2Der to flat tuple vector
        }
        flat_tuple_->addToVector("iterativeFit_chi2_2ndDerivative",chi2_2D.at(i));
    }


    //Re-run fit with xmax = location where 2nd derivative of chi2/NDF is maximum
    double max_Element = *std::max_element(chi2_2D.begin(), chi2_2D.end());
    int maxElementIndex = std::max_element(chi2_2D.begin(), chi2_2D.end()) - chi2_2D.begin();
    int back_off=0*nPointsDer_;
    xmax = fit_range_end.at(maxElementIndex-back_off);
    TF1* cc_fit = new TF1("cc_fit", "gaus", xmin, xmax);
    projy_h->Fit(cc_fit, "QRESL");

    //Set Fit Parameters in the flat tuple
    flat_tuple_->setVariableValue("baseline_gausFit_mean", cc_fit->GetParameter(1));
    flat_tuple_->setVariableValue("baseline_gausFit_sigma", cc_fit->GetParameter(2));
    flat_tuple_->setVariableValue("baseline_gausFit_norm", cc_fit->GetParameter(0));
    flat_tuple_->setVariableValue("baseline_gausFit_range_lower", (double)xmin);
    flat_tuple_->setVariableValue("baseline_gausFit_range_upper", (double)xmax);

    std::vector<double>::const_iterator first = fit_range_end.begin()+nPointsDer_;
    std::vector<double>::const_iterator last=fit_range_end.begin()+nPointsDer_+chi2_2D.size();
    std::vector<double> chi2_2D_range(first,last);
    for(int i=0; i < chi2_2D_range.size(); ++i) {
        flat_tuple_->addToVector("iterativeFit_chi2_2Der_range",chi2_2D_range.at(i));
    } 
    //projy_h->Write();
    flat_tuple_->fill();
    delete projy_h;
    delete cc_fit;
    }

    delete histo_hh;
    }

}

