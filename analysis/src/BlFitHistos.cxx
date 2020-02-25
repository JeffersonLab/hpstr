#include "BlFitHistos.h"

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


    //Loop over all channels to find location of maximum chi2 2nd derivative
    for(int cc=43; cc < 50; ++cc) {

        //Set Channel and Hybrid information in the flat tuple
        flat_tuple_->setVariableValue("SvtAna2DHisto_key", SvtAna2DHisto_key);
        flat_tuple_->setVariableValue("channel", cc);
        flat_tuple_->setVariableValue("minbinThresh",(double)xmin_);
        flat_tuple_->setVariableValue("minStats", (double)minStats_);
        flat_tuple_->setVariableValue("rebin", (double)rebin_);
     


       // if(cc%20 == 0){
         //   std::cout <<"Hybrid: "<< SvtAna2DHisto_key << " channel #" << cc << std::endl;
       // }
        std::cout <<"Hybrid: "<< SvtAna2DHisto_key << " channel #" << cc << std::endl;
        TH1D* projy_h = histo_hh->ProjectionY(Form("%s_projectionY_channel_%i",SvtAna2DHisto_key.c_str(),cc),cc+1,cc+1,"e");

        projy_h->SetTitle(Form("%s_projectionY_channel_%i",SvtAna2DHisto_key.c_str(),cc));
        double meanError_h = projy_h->GetMeanError();

        flat_tuple_->setVariableValue("meanError_ch", meanError_h);
        if(meanError_h > 400) {
            flat_tuple_->setVariableValue("noisey_ch", 1.0);
        }
        else {flat_tuple_->setVariableValue("noisey_ch", 0.0);}

        flat_tuple_->setVariableValue("n_entries", projy_h->GetEntries());

        //Minimum Entry Requirement NOT ROBUST!!!
        if(projy_h->GetEntries() < minStats_){
            
            flat_tuple_->setVariableValue("meanError_h", -9999.9);
            flat_tuple_->setVariableValue("BlFitMean", -9999.9);
            flat_tuple_->setVariableValue("BlFitSigma", -9999.9);
            flat_tuple_->setVariableValue("BlFitNorm", -9999.9);
            flat_tuple_->setVariableValue("BlFitRangeLower", -9999.9);
            flat_tuple_->setVariableValue("BlFitRangeUpper", -9999.9);
                
            flat_tuple_->addToVector("iterMean", -9999.9);
            flat_tuple_->addToVector("iterChi2NDF_2der",-9999.9);
            flat_tuple_->addToVector("iterChi2NDF_1der",-9999.9);
            flat_tuple_->addToVector("iterChi2NDF_derRange",-9999.9);
            flat_tuple_->addToVector("iterChi2NDF",-9999.9);
            flat_tuple_->addToVector("ratio2derChi2", -9999.9);
            flat_tuple_->addToVector("iterFitRangeEnd", -9999.9);
            //If channel did not have enough entries to perform a fit, set minStats_dead_channel status to 1
            flat_tuple_->setVariableValue("statsTooLow",1.0);
            std::cout << "Not enough stats in channel " << cc << std::endl;
            continue;
        }

        //If minimum entry requirement is passed, set 'dead_channel' variable to 0
        flat_tuple_->setVariableValue("statsTooLow",0.0);

    int iter=0;
    int firstbin=projy_h->FindFirstBinAbove(xmin_,1);
    double xmin=projy_h->GetBinLowEdge(firstbin);
    double binwidth=1*projy_h->GetBinWidth(firstbin);
    double xmax=xmin+20*binwidth;
    std::vector<double> amp,mean,sigma,chi2,const_err,sigma_err,
           fit_range_end,chi2_NDF,chi2_2D,chi2_1D;
    std::vector<int> NDF;
    //std::cout << "Getting starting fit parameters for channel " << cc << std::endl;


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
        flat_tuple_->addToVector("iterMean", cc_fit->GetParameter(1));

        chi2.push_back(cc_fit->GetChisquare());
        NDF.push_back(cc_fit->GetNDF());
        chi2_NDF.push_back(chi2.at(iter)/NDF.at(iter));
        flat_tuple_->addToVector("iterChi2NDF", chi2_NDF.at(iter));

        fit_range_end.push_back(xmax);
        flat_tuple_->addToVector("iterFitRangeEnd", xmax);

        //Increase fit range window and iterate the fit
        xmax = xmax + 2*binwidth;
        iter = iter + 1;
        delete cc_fit;
    }
    //std::cout << "First fit compelete for channel " << cc << std::endl;
    //Calculate the 2nd derivative of chi2 by taking slope on either side of one point
    //and taking the difference between the two.
    for(int i=0; i < chi2_NDF.size(); ++i) {

        if( i > nPointsDer_ && i < (chi2_NDF.size() - nPointsDer_)){
            double slope2 = (chi2_NDF.at(i+nPointsDer_)-chi2_NDF.at(i))/nPointsDer_*binwidth;
            double slope1 = (chi2_NDF.at(i)-chi2_NDF.at(i-nPointsDer_))/nPointsDer_*binwidth;
            double slopeDiff = slope2 - slope1;
            chi2_2D.push_back((slopeDiff));
            chi2_1D.push_back((slope1));
        }
    }
    //std::cout << "chi2_2D size for channel " << cc << " is "  << chi2_2D.size() << std::endl;
     //Remove any 'nan' entries from chi2_2D
     for( int i=0; i < chi2_2D.size(); ++i) {

        if(chi2_2D[i] != chi2_2D[i]) { 
            chi2_2D[i]=0.0;
            //add chi2_2Der to flat tuple vector
        }

        if(chi2_1D[i] != chi2_1D[i]) { 
            chi2_1D[i]=0.0;
            //add chi2_2Der to flat tuple vector
        }
        flat_tuple_->addToVector("iterChi2NDF_2der",chi2_2D.at(i));
        flat_tuple_->addToVector("iterChi2NDF_1der",chi2_1D.at(i));
    }

    //Take ratio of chi2/NDF 2nd derivative and chi2/NDF. Where this is maximized is the cut.
    std::vector<double> ratio2derChi2{};
    for(int i=0; i < chi2_2D.size(); ++i){
        ratio2derChi2.push_back(chi2_2D.at(i)/chi2_NDF.at(i+nPointsDer_));
        flat_tuple_->addToVector("ratio2derChi2",ratio2derChi2.at(i));
    }

    //Find Maximum Chi2/NDF second derivative
    double chi2_2D_max = *std::max_element(chi2_2D.begin(), chi2_2D.end());
    int chi2_2D_maxIndex = std::max_element(chi2_2D.begin(), chi2_2D.end()) - chi2_2D.begin();

    double chi2_2D_min = *std::min_element(chi2_2D.begin(), chi2_2D.end());
    int chi2_2D_minIndex = std::min_element(chi2_2D.begin(), chi2_2D.end()) - chi2_2D.begin();
    
    int back_off=0;
    double minimum_thresh = 1.4;
    std::vector<double>::const_iterator first = fit_range_end.begin()+nPointsDer_;
    std::vector<double>::const_iterator last=fit_range_end.begin()+nPointsDer_+chi2_2D.size();
    std::vector<double> chi2_2D_range(first,last);

    std::cout << "first max " << chi2_2D_max << "at pos " << chi2_2D_range.at(chi2_2D_maxIndex) << std::endl;
    std::cout << "min " << chi2_2D_min << std::endl;

    if(std::abs(chi2_2D_min) > chi2_2D_max * minimum_thresh and std::abs(chi2_2D_maxIndex - chi2_2D_minIndex) < 10) {
        //std::cout << "chi2_2D size " << chi2_2D.size() << std::endl;
        //std::cout << "chi2_2D_range size " << chi2_2D_range.size() << std::endl;
        //std::cout << "maxIndex " << chi2_2D_maxIndex << std::endl;
        //std::cout << "minIndex " << chi2_2D_minIndex << std::endl;
        int subtr;
        if( chi2_2D_maxIndex > 3) {subtr = 3;}
        else if(chi2_2D_maxIndex == 0) {subtr = 0;}
        else if(chi2_2D_maxIndex == 1) {subtr =1;}
        else if(chi2_2D_maxIndex == 2) {subtr = 2;}
        chi2_2D.erase(chi2_2D.begin() + chi2_2D_maxIndex - subtr, chi2_2D.begin() + chi2_2D_minIndex);

        chi2_2D_range.erase(chi2_2D_range.begin() + chi2_2D_maxIndex - subtr, chi2_2D_range.begin() + chi2_2D_minIndex);

        chi2_2D_max = *std::max_element(chi2_2D.begin(), chi2_2D.end());
        chi2_2D_maxIndex = std::max_element(chi2_2D.begin(), chi2_2D.end()) - chi2_2D.begin();
        
        std::cout << "Filtered max " << chi2_2D_max << "at pos " << chi2_2D_range.at(chi2_2D_maxIndex) << std::endl;
    }   




    std::cout << "chi2_2D_range Size" << chi2_2D_range.size() << std::endl;
    std::cout << "chi2_2D_maxIndex" << chi2_2D_maxIndex << std::endl;
    std::cout << "chi2_2D xmax" << chi2_2D_range.at(chi2_2D_maxIndex-back_off) << std::endl;
    double chi2_2D_xmax = chi2_2D_range.at(chi2_2D_maxIndex-back_off);
    xmax = chi2_2D_xmax;
    std::cout << "xmax " << xmax << std::endl;
    std::cout << "new max " << chi2_2D_max << std::endl;
    //TF1 cc_fit = new TF1("cc_fit", "gaus", xmin, xmax);
    TFitResultPtr fit = projy_h->Fit("gaus", "QRESL","",xmin,xmax);
    std::cout << "crash?" << std::endl;
    const double* chi2params = fit->GetParams();
    double sigma1= chi2params[2];
    std::cout << "crash1?" << std::endl;
    //Find Maximum Chi2/NDF 2nd derivative divided by Chi2/NDF
    double ratio2derChi2_max = *std::max_element(ratio2derChi2.begin(), ratio2derChi2.end());
    std::cout << "crash2?" << std::endl;
    int ratio2derChi2_maxIndex = std::max_element(ratio2derChi2.begin(), ratio2derChi2.end()) - ratio2derChi2.begin();
    double ratio_xmax = chi2_2D_range.at(ratio2derChi2_maxIndex-back_off);
    std::cout << "ratio max " << ratio2derChi2_max << "at pos " << ratio_xmax << std::endl;

    fit = projy_h->Fit("gaus","QRESL","",xmin,ratio_xmax);
    const double* ratioParams = fit->GetParams();
    double sigma2= ratioParams[2];
    std::cout << "crash c ?" << std::endl;
    std::cout << "crash e?" << std::endl;

    std::cout << "sigma 1: " << sigma1 << "; sigma 2: " << sigma2 << std::endl;
    if (sigma1 < sigma2){xmax = chi2_2D_xmax;}
    else {xmax = ratio_xmax;}
    
    //std::cout << "xmax " << xmax << std::endl;
    //std::cout << "new max " << chi2_2D_max << std::endl;
    fit = projy_h->Fit("gaus", "QRESL","",xmin,xmax);
    const double* fitparams = fit->GetParams();

    //Set Fit Parameters in the flat tuple
    flat_tuple_->setVariableValue("BlFitMean", fitparams[1]);
    flat_tuple_->setVariableValue("BlFitSigma", fitparams[2]);
    flat_tuple_->setVariableValue("BlFitNorm", fitparams[0]);
    flat_tuple_->setVariableValue("BlFitRangeLower", (double)xmin);
    flat_tuple_->setVariableValue("BlFitRangeUpper", (double)xmax);

    for(int i=0; i < chi2_2D_range.size(); ++i) {
        flat_tuple_->addToVector("iterChi2NDF_derRange",chi2_2D_range.at(i));
    } 
    //projy_h->Write();
    flat_tuple_->fill();
    delete projy_h;
    }

    delete histo_hh;
    }

}

