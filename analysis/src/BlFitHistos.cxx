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
        SvtAna2DHisto_key.erase(SvtAna2DHisto_key.end()-3,SvtAna2DHisto_key.end());

        int  nbins = histo_hh->GetXaxis()->GetNbins();


        //Loop over all channels to find location of maximum chi2 2nd derivative
        for(int cc=43; cc < 50; ++cc) 
        {

            //Set Channel and Hybrid information in the flat tuple
            flat_tuple_->setVariableValue("SvtAna2DHisto_key", SvtAna2DHisto_key);
            flat_tuple_->setVariableValue("channel", cc);
            flat_tuple_->setVariableValue("minbinThresh",(double)xmin_);
            flat_tuple_->setVariableValue("minStats", (double)minStats_);
            flat_tuple_->setVariableValue("rebin", (double)rebin_);



            if(cc%20 == 0)
            {
                std::cout <<"Hybrid: "<< SvtAna2DHisto_key << " channel #" << cc << std::endl;
            }
            TH1D* projy_h = histo_hh->ProjectionY(Form("%s_proY_ch%i",SvtAna2DHisto_key.c_str(),cc),cc+1,cc+1,"e");

            projy_h->SetTitle(Form("%s_proY_ch%i",SvtAna2DHisto_key.c_str(),cc));
            double chRMS = projy_h->GetRMS();

            flat_tuple_->setVariableValue("rms", chRMS);
            if(chRMS > 400) 
            {
                flat_tuple_->setVariableValue("noisy", 1.0);
            }
            else 
            {
                flat_tuple_->setVariableValue("noisy", 0.0);
            }

            flat_tuple_->setVariableValue("n_entries", projy_h->GetEntries());

            //Minimum Entry Requirement NOT ROBUST!!!
            if(projy_h->GetEntries() < minStats_)
            {

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

            int iter = 0;
            int firstbin = projy_h->FindFirstBinAbove(xmin_,1);
            double xmin = projy_h->GetBinLowEdge(firstbin);
            double binwidth = projy_h->GetBinWidth(firstbin);
            double xmax = xmin + 20.0*binwidth;
            std::vector<double> amp,mean,sigma,chi2,const_err,sigma_err,
                fit_range_end,chi2_NDF,chi2_2D,chi2_1D;
            std::vector<int> NDF;
            //std::cout << "Getting starting fit parameters for channel " << cc << std::endl;


            //Iterative Fitting Procedure. Used to lcoate  maximum chi2 2nd derivative along x-axis
            //This appears to correlate directly to the location where the baseline gaussian signal
            //ends, and the pileup threshold begins. This x-axis location is then used as the 
            //end of the iterative fit range window, fiting JUST the gaussian.

            while(xmax < 6800.0)
            {

                TFitResultPtr cc_fit = projy_h->Fit("gaus", "QRES", "", xmin, xmax);

                if(cc_fit->Ndf() == 0){
                    xmax = xmax + binwidth;
                    continue;
                }

                //Get Iterative Fit Parameters and add to flat tuple vector                                
                flat_tuple_->addToVector("iterMean", cc_fit->GetParams()[1]);

                chi2.push_back(cc_fit->Chi2());
                NDF.push_back(cc_fit->Ndf());
                chi2_NDF.push_back(chi2.at(iter)/NDF.at(iter));
                flat_tuple_->addToVector("iterChi2NDF", chi2_NDF.at(iter));

                fit_range_end.push_back(xmax);
                flat_tuple_->addToVector("iterFitRangeEnd", xmax);

                //Increase fit range window and iterate the fit
                xmax = xmax + binwidth;
                iter++;
            }

            //std::cout << "First fit compelete for channel " << cc << std::endl;
            //Calculate the 2nd derivative of chi2 by taking slope on either side of one point
            //and taking the difference between the two.
            for(int i = nPointsDer_; i < chi2_NDF.size() - nPointsDer_; i++) 
            {
                double derForward = (chi2_NDF.at(i+nPointsDer_)-chi2_NDF.at(i))/(nPointsDer_*binwidth);
                double derBack = (chi2_NDF.at(i)-chi2_NDF.at(i-nPointsDer_))/(nPointsDer_*binwidth);
                double der = (derForward+derBack)/2.0;
                double der2 = (derForward - derBack)/(nPointsDer_*binwidth);
                if(der2 == der2) 
                {
                    chi2_2D.push_back(der2);
                }
                else
                {
                    chi2_2D.push_back(-9999.9);
                }

                if(der == der) 
                {
                    chi2_1D.push_back(der);
                }
                else
                {
                    chi2_1D.push_back(-9999.9);
                }
                flat_tuple_->addToVector("iterChi2NDF_2der",der2);
                flat_tuple_->addToVector("iterChi2NDF_1der",der);
            }

            //Take ratio of chi2/NDF 2nd derivative and chi2/NDF. Where this is maximized is the cut.
            std::vector<double> ratio2derChi2;
            for(int i=0; i < chi2_2D.size(); ++i)
            {
                ratio2derChi2.push_back(chi2_2D.at(i)/chi2_NDF.at(i+nPointsDer_));
                flat_tuple_->addToVector("ratio2derChi2",ratio2derChi2.at(i));
            }

            //Find Maximum Chi2/NDF second derivative
            double chi2_2D_max = *std::max_element(chi2_2D.begin(), chi2_2D.end());
            int chi2_2D_maxIndex = std::max_element(chi2_2D.begin(), chi2_2D.end()) - chi2_2D.begin();

            double chi2_2D_min = *std::min_element(chi2_2D.begin(), chi2_2D.end());
            int chi2_2D_minIndex = std::min_element(chi2_2D.begin(), chi2_2D.end()) - chi2_2D.begin();

            int back_off = 0;
            double minimum_thresh = 1.4;
            std::vector<double>::const_iterator first = fit_range_end.begin()+nPointsDer_;
            std::vector<double>::const_iterator last  = fit_range_end.begin()+nPointsDer_+chi2_2D.size();
            std::vector<double> chi2_2D_range(first,last);

            if(std::abs(chi2_2D_min) > chi2_2D_max * minimum_thresh and std::abs(chi2_2D_maxIndex - chi2_2D_minIndex) < 10) {
                int subtr;
                if( chi2_2D_maxIndex > 3) {subtr = 3;}
                else if(chi2_2D_maxIndex == 0) {subtr = 0;}
                else if(chi2_2D_maxIndex == 1) {subtr = 1;}
                else if(chi2_2D_maxIndex == 2) {subtr = 2;}
                chi2_2D.erase(chi2_2D.begin() + chi2_2D_maxIndex - subtr, chi2_2D.begin() + chi2_2D_minIndex);

                chi2_2D_range.erase(chi2_2D_range.begin() + chi2_2D_maxIndex - subtr, chi2_2D_range.begin() + chi2_2D_minIndex);

                chi2_2D_max = *std::max_element(chi2_2D.begin(), chi2_2D.end());
                chi2_2D_maxIndex = std::max_element(chi2_2D.begin(), chi2_2D.end()) - chi2_2D.begin();

            }   




            double chi2_2D_xmax = chi2_2D_range.at(chi2_2D_maxIndex-back_off);
            xmax = chi2_2D_xmax;
            //TF1 cc_fit = new TF1("cc_fit", "gaus", xmin, xmax);
            TFitResultPtr fit = projy_h->Fit("gaus", "QRESL","",xmin,xmax);
            const double* chi2params = fit->GetParams();
            double sigma1= chi2params[2];
            //Find Maximum Chi2/NDF 2nd derivative divided by Chi2/NDF
            double ratio2derChi2_max = *std::max_element(ratio2derChi2.begin(), ratio2derChi2.end());
            int ratio2derChi2_maxIndex = std::max_element(ratio2derChi2.begin(), ratio2derChi2.end()) - ratio2derChi2.begin();
            double ratio_xmax = chi2_2D_range.at(ratio2derChi2_maxIndex-back_off);

            fit = projy_h->Fit("gaus","QRESL","",xmin,ratio_xmax);
            const double* ratioParams = fit->GetParams();
            double sigma2= ratioParams[2];

            if (sigma1 < sigma2){xmax = chi2_2D_xmax;}
            else {xmax = ratio_xmax;}

            fit = projy_h->Fit("gaus", "QRESL", "", xmin, xmax);
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

    }

}

