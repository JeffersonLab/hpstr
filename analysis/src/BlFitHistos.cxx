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
        histo_hh->Write();
        std::string SvtAna2DHisto_key = histo_hh->GetName();
        SvtAna2DHisto_key.erase(SvtAna2DHisto_key.end()-3,SvtAna2DHisto_key.end());

        int  nbins = histo_hh->GetXaxis()->GetNbins();


        //Loop over all channels to find location of maximum chi2 2nd derivative
        for(int cc=0; cc < 640 ; ++cc) 
        {

            //Set Channel and Hybrid information in the flat tuple
            flat_tuple_->setVariableValue("SvtAna2DHisto_key", SvtAna2DHisto_key);
            flat_tuple_->setVariableValue("channel", cc);
            flat_tuple_->setVariableValue("minbinThresh",(double)xmin_);
            flat_tuple_->setVariableValue("minStats", (double)minStats_);
            flat_tuple_->setVariableValue("rebin", (double)rebin_);



            //if(cc%20 == 0)
           // {
                std::cout <<"Hybrid: "<< SvtAna2DHisto_key << " channel #" << cc << std::endl;
            //}
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
            int firstbin = projy_h->FindFirstBinAbove(xmin_,1);
            if (firstbin == -1) 
            {
                
                flat_tuple_->setVariableValue("statsTooLow",1.0);
                std::cout << "Bin threshold too low" << cc << std::endl;
                continue;
            }
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
            
           // std::cout << "max bin count" << projy_h->GetMaximumBin() << std::endl;
            double xmin = projy_h->GetBinLowEdge(firstbin);
            double binwidth = projy_h->GetBinWidth(firstbin);
            double xmax = xmin + 20.0*binwidth;
            std::vector<double> amp,mean,sigma,chi2,const_err,sigma_err,
                fit_range_end,chi2_NDF,chi2_2D,chi2_1D,der2chi2,der2chi2R;
            std::vector<int> NDF;


            //Iterative Fitting Procedure. Used to lcoate  maximum chi2 2nd derivative along x-axis
            //This appears to correlate directly to the location where the baseline gaussian signal
            //ends, and the pileup threshold begins. This x-axis location is then used as the 
            //end of the iterative fit range window, fiting JUST the gaussian.

            while(xmax < 6800.0)
            {

                TFitResultPtr cc_fit = projy_h->Fit("gaus", "QRES", "", xmin, xmax);

                if(cc_fit->Ndf() == 0){

                    std::cout << "Ndf = 0" << firstbin << std::endl;
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

            std::cout << "First fit compelete for channel " << cc << std::endl;
            //Calculate the 2nd derivative of chi2 by taking slope on either side of one point
            //and taking the difference between the two.

            for(int i = nPointsDer_; i < chi2_NDF.size() - nPointsDer_; i++) 
            {
                double derForward = (chi2_NDF.at(i+nPointsDer_)-chi2_NDF.at(i))/(nPointsDer_*binwidth);
                double derBack = (chi2_NDF.at(i)-chi2_NDF.at(i-nPointsDer_))/(nPointsDer_*binwidth);
                double der = (derForward+derBack)/2.0;
                double der2 = (derForward - derBack); //(nPointsDer_*binwidth);
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
            double minimum_thresh = 1.;
            std::vector<double>::const_iterator first = fit_range_end.begin()+nPointsDer_;
            std::vector<double>::const_iterator last  = fit_range_end.begin()+nPointsDer_+chi2_2D.size();
            std::vector<double> chi2_2D_range(first,last);

            double chi2max = *std::max_element(chi2_NDF.begin(), chi2_NDF.end());
            int chi2maxIndex = std::max_element(chi2_NDF.begin(), chi2_NDF.end()) - chi2_NDF.begin();
            //std::cout << " Original Chi2 xmax: " << fit_range_end.at(chi2maxIndex) << std::endl;
            //std::cout << "Chi2 max index " << chi2maxIndex << std::endl;

            double chi2_2D_max = *std::max_element(chi2_2D.begin(), chi2_2D.end());
            int chi2_2D_maxIndex = std::max_element(chi2_2D.begin(), chi2_2D.end()) - chi2_2D.begin();
            double chi2_2D_xmax = chi2_2D_range.at(chi2_2D_maxIndex);
            //std::cout << "Pre-cut Chi2_2D xmax: " << chi2_2D_xmax << std::endl;

            if (chi2maxIndex > nPointsDer_) 
                {
                //subrange of chi2_2D values using chi2max as a maximum cut
                std::vector<double>::const_iterator begin = chi2_2D.begin();
                std::vector<double>::const_iterator end  = chi2_2D.begin()+(chi2maxIndex-nPointsDer_);
                std::vector<double> cutrange(begin,end);
                double cut_maxIndex = std::max_element(cutrange.begin(), cutrange.end()) - cutrange.begin();
                double cut_xmax = chi2_2D_range.at(cut_maxIndex);
                //std::cout << "Post-cut Chi2_2D xmax: " << cut_xmax << std::endl;


                double cut1xmax = cut_xmax;

                //Fit regardless if cut was applied or not
                TFitResultPtr fit = projy_h->Fit("gaus", "QRES", "", xmin, cut1xmax);
                double sigma1 = fit->GetParams()[2];

                
                int back_off = 0;
                double cut2xmax;
                double sigma2;
                bool cut2;
                if(cut_xmax >= chi2_2D_xmax && chi2maxIndex > nPointsDer_) 
                {
                    cut2 = true;
                    while ( back_off < chi2maxIndex && cut_xmax >= chi2_2D_xmax && chi2maxIndex > nPointsDer_)
                    {
                        chi2maxIndex = chi2maxIndex - 1;
                        //std::cout << "chi2maxIndex" << chi2maxIndex << std::endl;
                        //std::cout << "chi2_2D size: " << chi2_2D.size() << std::endl;
                        //std::cout << "range subtr" << chi2maxIndex - nPointsDer_ << std::endl;
                        begin = chi2_2D.begin();
                        end  = chi2_2D.begin()+(chi2maxIndex-nPointsDer_);
                        std::vector<double> cutrange(begin,end);

                        cut_maxIndex = std::max_element(cutrange.begin(), cutrange.end()) - cutrange.begin();
                        cut_xmax = chi2_2D_range.at(cut_maxIndex);
                        //std::cout << "Post-cut Chi2_2D xmax: " << cut_xmax << std::endl;
                        cut2xmax = cut_xmax;
                        back_off ++;
                    }
                    //std::cout << "exit while loop" << std::endl;
                    //Fit at next lowest xmax position of chi2 second derivative
                    fit = projy_h->Fit("gaus", "QRES", "", xmin, cut2xmax);
                    sigma2 = fit->GetParams()[2];

                }
                
                if (cut2 == true) 
                {
                    if (sigma1 < sigma2)
                    {
                        xmax = cut1xmax;
                    }
                    else 
                    {
                        xmax = cut2xmax;
                    }
                }
                else 
                {
                    xmax = cut1xmax;
                }

            }

            else 
            {
                xmax = chi2_2D_xmax;
            }

            TFitResultPtr fit = projy_h->Fit("gaus", "QRES", "", xmin, xmax);
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

