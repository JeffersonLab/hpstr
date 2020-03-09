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
        for(int cc=498; cc < 600 ; ++cc) 
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
            //Set minimum bin threshold for fit range begin
            int ntries = projy_h->GetEntries();
            /*
            if ( ntries < 5000 ){xmin_ = 10;}
            else if ( 5000 < ntries && ntries < 10000) {xmin_ = 50;}
            else if ( 10000 < ntries && ntries < 20000) {xmin_ = 70;}
            else if ( 20000 < ntries && ntries < 30000) {xmin_ = 100;}
            else if ( 30000 < ntries && ntries < 100000) {xmin_ = 100;}
            else if ( 100000 < ntries) {xmin_ = 100;}
            */

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
            if (firstbin == -1 or projy_h->GetEntries() < minStats_ ) 
            {
                flat_tuple_->setVariableValue("BlFitMean", -9999.9);
                flat_tuple_->setVariableValue("BlFitSigma", -9999.9);
                flat_tuple_->setVariableValue("BlFitNorm", -9999.9);
                flat_tuple_->setVariableValue("BlFitRangeLower", -9999.9);
                flat_tuple_->setVariableValue("BlFitRangeUpper", -9999.9);
                flat_tuple_->setVariableValue("BlFitChi2", -9999.9);
                flat_tuple_->setVariableValue("BlFitNdf", -9999.9);

                flat_tuple_->addToVector("iterMean", -9999.9);
                flat_tuple_->addToVector("iterChi2NDF_2der",-9999.9);
                flat_tuple_->addToVector("iterChi2NDF_1der",-9999.9);
                flat_tuple_->addToVector("iterChi2NDF_derRange",-9999.9);
                flat_tuple_->addToVector("iterChi2NDF",-9999.9);
                flat_tuple_->addToVector("ratio2derChi2", -9999.9);
                flat_tuple_->addToVector("iterFitRangeEnd", -9999.9);

                flat_tuple_->setVariableValue("lowdaq", -9999.9);

      //If channel not have enough entries to fit, set minStats_dead_channel status to 1
                flat_tuple_->setVariableValue("minbinFail",1.0);
                std::cout << "Not enough stats in channel " << cc << std::endl;
                continue;
            }

            //If minimum entry requirement is passed, set 'dead_channel' variable to 0
            flat_tuple_->setVariableValue("minbinFail",0.0);

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
            //std::cout << "Chi2_2D_range size: " << chi2_2D_range.size() << std::endl;
            //std::cout << "Chi2_2D size: " << chi2_2D.size() << std::endl;
            
            first = chi2_NDF.begin()+nPointsDer_;
            last = chi2_NDF.begin() + nPointsDer_ + chi2_2D.size();
            std::vector<double> chi2SubRange(first,last);


            double chi2max = *std::max_element(chi2SubRange.begin(), chi2SubRange.end());
            int chi2maxIndex = std::max_element(chi2SubRange.begin(), chi2SubRange.end()) - chi2SubRange.begin();
            //std::cout << " Original Chi2 xmax: " << fit_range_end.at(chi2maxIndex) << std::endl;
            //std::cout << "Chi2 max index " << chi2maxIndex << std::endl;

            double chi2_2D_max = *std::max_element(chi2_2D.begin(), chi2_2D.end());
            int chi2_2D_maxIndex = std::max_element(chi2_2D.begin(), chi2_2D.end()) - chi2_2D.begin();
            double chi2_2D_xmax = chi2_2D_range.at(chi2_2D_maxIndex);
            //std::cout << "Pre-cut Chi2_2D xmax: " << chi2_2D_xmax << std::endl;

            
            //subrange of chi2_2D values using chi2max as a maximum cut
            std::vector<double>::const_iterator begin = chi2_2D.begin();
            std::vector<double>::const_iterator end  = chi2_2D.begin()+(chi2maxIndex);
            std::vector<double> cutrange(begin,end);
            double cut_maxIndex = std::max_element(cutrange.begin(), cutrange.end()) - cutrange.begin();
            double cut_xmax = chi2_2D_range.at(cut_maxIndex);
            //std::cout << "Post-cut Chi2_2D xmax: " << cut_xmax << std::endl;


            double cut1xmax = cut_xmax;

            //Fit regardless if cut was applied or not
            TFitResultPtr fit = projy_h->Fit("gaus", "QRES", "", xmin, cut1xmax);
            double sigma1 = fit->GetParams()[2];

            
            double cut2xmax;
            double sigma2;
            bool cut2;
            if(cut_xmax >= chi2_2D_xmax) 
            {
                cut2 = true;
                while (cut_xmax >= chi2_2D_xmax && chi2maxIndex > 0)
                {
                    //std::cout << "chi2maxIndex" << chi2maxIndex << std::endl;
                    //std::cout << "chi2_2D size: " << chi2_2D.size() << std::endl;
                    begin = chi2_2D.begin();
                    end  = chi2_2D.begin()+(chi2maxIndex);
                    std::vector<double> cutrange(begin,end);

                    cut_maxIndex = std::max_element(cutrange.begin(), cutrange.end()) - cutrange.begin();
                    //std::cout << "cut_maxIndex: " << cut_maxIndex << std::endl;
                    cut_xmax = chi2_2D_range.at(cut_maxIndex);
                    //std::cout << "Post-cut Chi2_2D xmax: " << cut_xmax << std::endl;
                    cut2xmax = cut_xmax;
                    chi2maxIndex = chi2maxIndex - 1;
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

            
            

            fit = projy_h->Fit("gaus", "QRES", "", xmin, xmax);
            double ogChi2 = fit->Chi2();
            double ogNdf = fit->Ndf();
            double ogMean = fit->GetParams()[1];
            double ogSigma = fit->GetParams()[2];
            double tempChi2 = ogChi2;
            double tempNdf = ogNdf;
            double improve = 0.01;
            bool xminworse = false;
            bool addxmaxworse = false;
            double stopxmax = ogMean + 3*ogSigma; 
            bool subxmaxworse = false;
            double ogxmax = xmax;
            bool xmaxreset = true;

            

            

            //XMAX
            //std::cout << "original xmax " << xmax << std::endl;
            //std::cout << "original Chi2/Ndf " << ogChi2/ogNdf << std::endl;
            
            /*while (addxmaxworse != true ) 
            {
                xmax = xmax + 5*binwidth;
                std::cout << "xmax = " << xmax << std::endl;
                fit = projy_h->Fit("gaus", "QRES", "", xmin, xmax);
                std::cout << "New Chi2/Ndf " << fit->Chi2()/fit->Ndf() << std::endl;
                if ( (fit->Chi2()/fit->Ndf())/(tempChi2/tempNdf) > 1.1 )
                {
                    xmax = xmax - 5*binwidth;
                    addxmaxworse = true;
                }
                std::cout << "New xmax found at " << xmax << std::endl; 
                tempChi2 = fit->Chi2();
                tempNdf = fit->Ndf();
            }
            */
            /*
                std::cout << "mean + Nsigma = " << ogMean + 3*ogSigma << std::endl;
                if ( xmax < ogMean + 0.3*ogSigma) 
                {
                    std::vector<double> xmaxpos;
                    std::vector<double> minChi2;
                    while (xmax < stopxmax)
                    {
                        xmax = xmax + 1*binwidth;
                        xmaxpos.push_back(xmax);
                        std::cout << "xmax increased to " << xmax << std::endl;
                        tempChi2 = fit->Chi2();
                        tempNdf = fit->Ndf();
                        fit = projy_h->Fit("gaus", "QRES", "", xmin, xmax);
                        std::cout << "New Chi2/Ndf " << fit->Chi2()/fit->Ndf() << std::endl;
                        minChi2.push_back(fit->Chi2()/fit->Ndf());
                        if ( fit->Chi2()/fit->Ndf() < tempChi2/tempNdf)
                        {
                            while ( xmax < stopxmax) 
                            {
                                xmax = xmax + 1*binwidth;
                                std::cout << "xmax increased to " << xmax << std::endl;
                                tempChi2 = fit->Chi2();
                                tempNdf = fit->Ndf();
                                fit = projy_h->Fit("gaus", "QRES", "", xmin, xmax);
                                std::cout << "New Chi2/Ndf " << fit->Chi2()/fit->Ndf() << std::endl;
                                if ( fit->Chi2()/fit->Ndf() > tempChi2/tempNdf) 
                                {
                                    xmax = xmax - 1*binwidth;
                                    subxmaxworse = true;
                                    xmaxreset = false;
                                    break;
                                }
                            }
                            break;
                        }
                    }
        */

               // int chi2mindex = std::min_element(minChi2.begin(), minChi2.end()) - minChi2.begin();
                //xmax = xmaxpos.at(chi2mindex);
                //std::cout << "Xmax using min chi2 " << xmax << std::endl;
               // }

            
            while (subxmaxworse != true && xmin > 0 ) 
            {
                 
                std::cout << "original xmax  " << xmax << std::endl;
                std::cout << "original Chi2/Ndf " << fit->Chi2()/fit->Ndf() << std::endl;
                xmax = xmax - 5*binwidth;
                std::cout << "xmax decreased to  " << xmax << std::endl;
                fit = projy_h->Fit("gaus", "QRES", "", xmin, xmax);
                std::cout << "New Chi2/Ndf " << fit->Chi2()/fit->Ndf() << std::endl;
                if ( (fit->Chi2()/fit->Ndf())/(tempChi2/tempNdf) > (1.0-improve) )
                {
                    xmax = xmax + 5*binwidth;
                    subxmaxworse = true;
                }
                tempChi2 = fit->Chi2();
                tempNdf = fit->Ndf();
            }

            tempChi2 = ogChi2;
            tempNdf = ogNdf;

            //XMIN
            std::cout << "original xmin " << xmin << std::endl;
            std::cout << "original Chi2/Ndf " << tempChi2/tempNdf << std::endl;
            while (xminworse != true) 
            {
                xmin = xmin + 5*binwidth;
                if (xmax-xmin < 80) { break;}
                std::cout << "xmin = " << xmin << std::endl;
                std::cout << "xmax = " << xmax << std::endl;
                fit = projy_h->Fit("gaus", "QRES", "", xmin, xmax);
                std::cout << "Ndf is " << fit->Ndf() << std::endl;
                std::cout << "New Chi2/Ndf " << fit->Chi2()/fit->Ndf() << std::endl;
                if ( (fit->Chi2()/fit->Ndf())/(tempChi2/tempNdf) > (1.0-improve) )
                {
                    xmin = xmin - 5*binwidth;
                    xminworse = true;
                }
                std::cout << "New xmin found at " << xmin << std::endl; 
                tempChi2 = fit->Chi2();
                tempNdf = fit->Ndf();
            }

            fit = projy_h->Fit("gaus", "QRES", "", xmin, xmax);
            const double* fitparams = fit->GetParams();
            //std::cout << "New Chi2/Ndf " << fit->Chi2()/fit->Ndf() << std::endl; 

            //If fit mean > xmax, low daq threshold or bad fit
            double N = 0.5;
            if ( fitparams[1] + N*fitparams[2] > xmax )
            {
                flat_tuple_->setVariableValue("lowdaq", 1.0);
            }
            else 
            {
                flat_tuple_->setVariableValue("lowdaq", 0.0);
            }
            //Set Fit Parameters in the flat tuple
            flat_tuple_->setVariableValue("BlFitMean", fitparams[1]);
            flat_tuple_->setVariableValue("BlFitSigma", fitparams[2]);
            flat_tuple_->setVariableValue("BlFitNorm", fitparams[0]);
            flat_tuple_->setVariableValue("BlFitRangeLower", (double)xmin);
            flat_tuple_->setVariableValue("BlFitRangeUpper", (double)xmax);
            flat_tuple_->setVariableValue("BlFitChi2", fit->Chi2());
            flat_tuple_->setVariableValue("BlFitNdf", fit->Ndf());

            for(int i=0; i < chi2_2D_range.size(); ++i) {
                flat_tuple_->addToVector("iterChi2NDF_derRange",chi2_2D_range.at(i));
            } 
            //projy_h->Write();
            flat_tuple_->fill();
            delete projy_h;
        }

    }

}

