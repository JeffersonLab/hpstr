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



void BlFitHistos::Chi2GausFit( HistoManager* inputHistos_, int nPointsDer_,int rebin_,int xmin_, int minStats_, FlatTupleMaker* flat_tuple_) {
     

    //Loop over all 2D histogram keys found in input ROOT File
    for (std::vector<std::string>::iterator jj = inputHistos_->histos2dNamesfromTFile.begin();
            jj != inputHistos_->histos2dNamesfromTFile.end(); ++jj)
    {
        //Get 2D Histograms from input ROOT File using keys, and rebin based on configuration setting
        TH2F* histo_hh = inputHistos_->get2dHisto(*jj);
        histo_hh->RebinY(rebin_);
        //Save 2D histogram to output ROOT File
        histo_hh->Write();
        std::string SvtAna2DHisto_key = histo_hh->GetName();
        SvtAna2DHisto_key.erase(SvtAna2DHisto_key.end()-3,SvtAna2DHisto_key.end());

        int  nbins = histo_hh->GetXaxis()->GetNbins();


        //Perform fitting procedure over all channels on a sensor
        for(int cc=0; cc < 30 ; ++cc) 
        {
            
            //Set Channel and Hybrid information and paramaters in the flat tuple
            flat_tuple_->setVariableValue("SvtAna2DHisto_key", SvtAna2DHisto_key);
            flat_tuple_->setVariableValue("channel", cc);
            flat_tuple_->setVariableValue("minbinThresh",(double)xmin_);
            flat_tuple_->setVariableValue("minStats", (double)minStats_);
            flat_tuple_->setVariableValue("rebin", (double)rebin_);


            //Update display with sensor and channel information periodically 
            if(cc%20 == 0)
            {
                std::cout <<"Hybrid: "<< SvtAna2DHisto_key << " channel #" << cc << std::endl;
            }

            //Get YProjection (1D Channel Histo) from 2D Histogram 
            TH1D* projy_h = histo_hh->ProjectionY(Form("%s_proY_ch%i",SvtAna2DHisto_key.c_str(),cc),
                    cc+1,cc+1,"e");
            projy_h->SetTitle(Form("%s_proY_ch%i",SvtAna2DHisto_key.c_str(),cc));

            //Check number of entries and RMS of channel
            flat_tuple_->setVariableValue("n_entries", projy_h->GetEntries());
            double chRMS = projy_h->GetRMS();

            //If rms is > 400, set variable "noisy" to 1 for true
            flat_tuple_->setVariableValue("rms", chRMS);
            if(chRMS > 400) 
            {
                flat_tuple_->setVariableValue("noisy", 1.0);
            }
            else 
            {
                flat_tuple_->setVariableValue("noisy", 0.0);
            }


            //Set minimum of fit range window by finding first bin with fraction of maximum bin value
            double maxbin = projy_h->GetBinContent(projy_h->GetMaximumBin());
            //std::cout << "[BlFitHistos] Maximum Bin Value Is " << maxbin << std::endl;
            //Fraction of maximum bin value required to start fit
            double frac = 0.05;
            int firstbin = projy_h->FindFirstBinAbove((double)frac*maxbin,1);
            
            //If channel does not have the minimum statistics required, set all variables to -9999.9
            //and skip the fit procedure on this channel
            if (firstbin == -1 or projy_h->GetEntries() < minStats_ ) 
            {
                flat_tuple_->setVariableValue("BlFitMean", -9999.9);
                flat_tuple_->setVariableValue("BlFitSigma", -9999.9);
                flat_tuple_->setVariableValue("BlFitNorm", -9999.9);
                flat_tuple_->setVariableValue("BlFitRangeLower", -9999.9);
                flat_tuple_->setVariableValue("BlFitRangeUpper", -9999.9);
                flat_tuple_->setVariableValue("BlFitChi2", -9999.9);
                flat_tuple_->setVariableValue("BlFitNdf", -9999.9);

                flat_tuple_->setVariableValue("ogxmax", -9999.9);
                flat_tuple_->setVariableValue("ogxmin", -9999.9);

                flat_tuple_->addToVector("iterMean", -9999.9);
                flat_tuple_->addToVector("iterChi2NDF_2der",-9999.9);
                flat_tuple_->addToVector("iterChi2NDF_1der",-9999.9);
                flat_tuple_->addToVector("iterChi2NDF_derRange",-9999.9);
                flat_tuple_->addToVector("iterChi2NDF",-9999.9);
                flat_tuple_->addToVector("ratio2derChi2", -9999.9);
                flat_tuple_->addToVector("iterFitRangeEnd", -9999.9);

                flat_tuple_->setVariableValue("lowdaq", -9999.9);

                flat_tuple_->setVariableValue("minbinFail",1.0);
                std::cout << "Not enough stats in channel " << cc << std::endl;
                continue;
            }

            //If minimum entry requirement is passed, set 'dead_channel' variable to 0
            flat_tuple_->setVariableValue("minbinFail",0.0);

            //Initialize the minimum and maximum fit range positions
            int iter = 0;
            double xmin = projy_h->GetBinLowEdge(firstbin);
            //std::cout << "[BlFitHistos] Initial xmin is " << xmin << std::endl;
            double binwidth = projy_h->GetBinWidth(firstbin);
            double xmax = xmin + 20.0*binwidth;
            //std::cout << "[BlFitHistos] Initial xmax is " << xmax << std::endl;
            
            //Define all iterative and final fit parameters
            std::vector<double> amp,mean,sigma,chi2,const_err,sigma_err,
                fit_range_end,chi2_NDF,chi2_2D,chi2_1D,der2chi2,der2chi2R;
            std::vector<int> NDF;


            //Iterative Fitting Procedure. Used to lcoate  maximum chi2 2nd derivative along x-axis
            //This appears to correlate directly to the location where the baseline gaussian signal
            //ends, and the pileup threshold begins. This x-axis location is then used as the 
            //end of the iterative fit range window, fiting JUST the gaussian.
            //std::cout << "[BlFitHistos] Starting Iterative Fit Procedure" << std::endl;
            double currentChi2 = 0.0;
            while(xmax < 6800.0 && currentChi2 < 100.0 || iter < 10)
            {
                //std::cout << "current xmax is " << xmax << std::endl;
                TFitResultPtr cc_fit = projy_h->Fit("gaus", "QRES", "", xmin, xmax);
                if(cc_fit->Ndf() == 0){

                    //std::cout << "[BlFitHistos] WARNING: Ndf = 0. SKIP CH" << firstbin << std::endl;
                    xmax = xmax + binwidth;
                    continue;
                }

                //Return iterative fit parameters and add to flat tuple
                flat_tuple_->addToVector("iterMean", cc_fit->GetParams()[1]);
                chi2.push_back(cc_fit->Chi2());
                NDF.push_back(cc_fit->Ndf());
                chi2_NDF.push_back(chi2.at(iter)/NDF.at(iter));
                flat_tuple_->addToVector("iterChi2NDF", chi2_NDF.at(iter));
                //Xmax for each fit iteration
                fit_range_end.push_back(xmax);
                flat_tuple_->addToVector("iterFitRangeEnd", xmax);

                //Increase fit range window and iterate the fit
                xmax = xmax + binwidth;
                iter++;
                currentChi2 = cc_fit->Chi2()/cc_fit->Ndf();
            }

            //Calculate the 2nd derivative of chi2 by taking slope on either side of one point
            //and taking the difference between the two. The xposition of the maximum Chi2 2nd 
            //derivative is used to define the maximum fit range x-position

            for(int i = nPointsDer_; i < chi2_NDF.size() - nPointsDer_; i++) 
            {
                double derForward=(chi2_NDF.at(i+nPointsDer_)-chi2_NDF.at(i))/(nPointsDer_*binwidth);
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

            //Take ratio of chi2/NDF 2nd derivative and chi2/NDF. ADC value of this maximum will be
            //the maximum fit range.
            std::vector<double> ratio2derChi2;
            for(int i=0; i < chi2_2D.size(); ++i)
            {
                ratio2derChi2.push_back(chi2_2D.at(i)/chi2_NDF.at(i+nPointsDer_));
                flat_tuple_->addToVector("ratio2derChi2",ratio2derChi2.at(i));
            }

            //Create subrange for chi2_2D that accounts for derivative requiring <n> points prior  
            //and post point of interest
            double minimum_thresh = 1.;
            std::vector<double>::const_iterator first = fit_range_end.begin()+nPointsDer_;
            std::vector<double>::const_iterator last=fit_range_end.begin()+nPointsDer_+chi2_2D.size();
            std::vector<double> chi2_2D_range(first,last);
            
            //Find maximum Chi2/Ndf
            first = chi2_NDF.begin()+nPointsDer_;
            last = chi2_NDF.begin() + nPointsDer_ + chi2_2D.size();
            std::vector<double> chi2SubRange(first,last);
            double chi2max = *std::max_element(chi2SubRange.begin(), chi2SubRange.end());
            int chi2maxIndex = std::max_element(chi2SubRange.begin(), chi2SubRange.end()) - chi2SubRange.begin();

            //Find maximum chi2 second derivative
            double chi2_2D_max = *std::max_element(chi2_2D.begin(), chi2_2D.end());
            int chi2_2D_maxIndex = std::max_element(chi2_2D.begin(), chi2_2D.end()) - chi2_2D.begin();
            double chi2_2D_xmax = chi2_2D_range.at(chi2_2D_maxIndex);
            xmax = chi2_2D_xmax;
            
            
            //subrange of chi2_2D values using chi2max as a maximum cut
            //Apply cut on chi2 2nd derivative values by only considering x-positions that occur
            //before the x-position of maximum Chi2
            std::vector<double>::const_iterator begin = chi2_2D.begin();
            std::vector<double>::const_iterator end  = chi2_2D.begin()+(chi2maxIndex);
            std::vector<double> cutrange(begin,end);
            double cut_maxIndex = std::max_element(cutrange.begin(),cutrange.end())-cutrange.begin();
            double cut_xmax = chi2_2D_range.at(cut_maxIndex);
            double cut1xmax = cut_xmax;
            //std::cout << "[BlFitHistos] xmax before Chi2Max cut: " << chi2_2D_xmax << std::endl;
            //std::cout << "[BlFitHistos] xmax after Chi2Max cut: " << cut_xmax << std::endl;


            //Perform fit regardless if chi2 max cut made any difference
            TFitResultPtr fit = projy_h->Fit("gaus", "QRES", "", xmin, cut1xmax);
            double chi21 = fit->Chi2()/fit->Ndf();
            
            //Check if chi2 max cut was applied. If not, perform additional cut
            double cut2xmax;
            double chi22;
            bool cut2=false;
            //If original xmax was not decreased by chi2max cut, move chi2max down until a new
            //maximum chi2 2nd derivative is found
            if(cut_xmax >= chi2_2D_xmax) 
            {
                cut2 = true;
                while (cut_xmax >= chi2_2D_xmax && chi2maxIndex > 0)
                {
                    begin = chi2_2D.begin();
                    end  = chi2_2D.begin()+(chi2maxIndex);
                    std::vector<double> cutrange(begin,end);
                    cut_maxIndex = std::max_element(cutrange.begin(),cutrange.end())-cutrange.begin();
                    cut_xmax = chi2_2D_range.at(cut_maxIndex);
                    cut2xmax = cut_xmax;
                    chi2maxIndex = chi2maxIndex - 1;
                }

                //Fit at next lowest xmax position of chi2 second derivative
                fit = projy_h->Fit("gaus", "QRES", "", xmin, cut2xmax);
                chi22 = fit->Chi2()/fit->Ndf();
            }
            
            //If second cut was applied, keep whichever fit has smallest Chi2/Ndf
            if (cut2 == true) 
            {
                if (chi21 < chi22)
                {
                    xmax = cut1xmax;
                }
                else 
                {
                    xmax = cut2xmax;
                }
            }

            //If initial chi2max cut did initially change xmax, just keep that fit
            else 
            {
                xmax = cut1xmax;
            }
        
            //std::cout << "[BlFitHistos] xmax after cuts is " << xmax << std::endl;
            //Variables for examining fit window distributions in event of issues
            flat_tuple_->setVariableValue("ogxmax", xmax);
            flat_tuple_->setVariableValue("ogxmin", xmin);

            //Again fit the baseline using xmax as predetermined above
            fit = projy_h->Fit("gaus", "QRES", "", xmin, xmax);


            //Collection of variables/parameters used in improving fit through iteration
            double ogChi2 = fit->Chi2();
            double ogNdf = fit->Ndf();
            double ogMean = fit->GetParams()[1];
            double ogSigma = fit->GetParams()[2];
            double tempChi2 = ogChi2;
            double tempNdf = ogNdf;
            double improve = 0.1;
            bool xminworse = false;
            bool addxmaxworse = false;
            double stopxmax = ogMean + 3*ogSigma; 
            bool subxmaxworse = false;
            double ogxmax = xmax;
            bool xmaxreset = true;

                       
            //Iteratively decrease xmax, refit, and calculate Chi2/Ndf
            //If Chi2/Ndf improves after an iteration by set amount, continue iterating
            while (subxmaxworse != true && xmin > 0 && xmax - xmin > 50.0 ) 
            {
                //std::cout << "[BlFitHistos] Original xmax is  " << xmax << std::endl;
                //std::cout << "[BlFitHistos] Original Chi2/Ndf is " << fit->Chi2()/fit->Ndf() << std::endl;
                xmax = xmax - 5*binwidth;
                //std::cout << "[BlFitHistos] xmax decreased to  " << xmax << std::endl;

                fit = projy_h->Fit("gaus", "QRES", "", xmin, xmax);
                //std::cout << "[BlFitHistos] New Chi2/Ndf is " << fit->Chi2()/fit->Ndf() << std::endl;

                //If Chi2/Ndf of new fit doesn't imrpove by some amount, stop
                if ( (fit->Chi2()/fit->Ndf())/(tempChi2/tempNdf) > (1.0-improve) )
                {
                    xmax = xmax + 5*binwidth;
                    subxmaxworse = true;
                }
                tempChi2 = fit->Chi2();
                tempNdf = fit->Ndf();
            }


            //XMIN
            //Iteratively increase xmin, refit, calculate Chi2/Ndf
            //Iterate unless or until Chi2/Ndf starts increasing
            tempChi2 = ogChi2;
            tempNdf = ogNdf;
            std::cout << "original xmin " << xmin << std::endl;
            std::cout << "original Chi2/Ndf " << tempChi2/tempNdf << std::endl;
            while (xminworse != true && xmax - xmin > 50.0) 
            {
                //std::cout  << "xmax - xmin is " << abs(xmax-xmin) << std::endl;
                //if (abs(xmax-xmin) < 50.0) { break;}
                xmin = xmin + 5*binwidth;
                fit = projy_h->Fit("gaus", "QRES", "", xmin, xmax);
                //std::cout << "Ndf is " << fit->Ndf() << std::endl;
                //std::cout << "New Chi2/Ndf " << fit->Chi2()/fit->Ndf() << std::endl;
                //If Chi2/Ndf of new fit doesn't imrpove by some amount, stop
                if ( (fit->Chi2()/fit->Ndf())/(tempChi2/tempNdf) > (1.0-improve) )
                {
                    xmin = xmin - 5*binwidth;
                    xminworse = true;
                }
                //std::cout << "New xmin found at " << xmin << std::endl; 
                tempChi2 = fit->Chi2();
                tempNdf = fit->Ndf();
            }


            //Now that xmax and xmin have both been separately incremented to find the fit range 
            //that returns the smallest Chi2/Ndf, redo the fit using this new range
            //std::cout << "[BlFitHistos] Final Fit" << std::endl;

            fit = projy_h->Fit("gaus", "QRES", "", xmin, xmax);
            const double* fitparams = fit->GetParams();

            //LOWDAQ THRESHOLD FLAG
            //If fit mean + N*Sigma > xmax, channel has low daq threshold *or* bad fit
            double N = 0.2;
            if ( fitparams[1] + N*fitparams[2] > xmax )
            {
                flat_tuple_->setVariableValue("lowdaq", 1.0);
            }
            else 
            {
                flat_tuple_->setVariableValue("lowdaq", 0.0);
            }

            //Store Final Fit Parameters in the flat tuple
            //std::cout << "New Xmax-Xmin = " << xmax - xmin << std::endl;
            flat_tuple_->setVariableValue("BlFitMean", fitparams[1]);
            flat_tuple_->setVariableValue("BlFitSigma", fitparams[2]);
            flat_tuple_->setVariableValue("BlFitNorm", fitparams[0]);
            flat_tuple_->setVariableValue("BlFitRangeLower", (double)xmin);
            flat_tuple_->setVariableValue("BlFitRangeUpper", (double)xmax);
            flat_tuple_->setVariableValue("BlFitChi2", fit->Chi2());
            flat_tuple_->setVariableValue("BlFitNdf", fit->Ndf());
            //std::cout << "Final Chi2/Ndf is " << fit->Chi2()/fit->Ndf();
            for(int i=0; i < chi2_2D_range.size(); ++i) {
                flat_tuple_->addToVector("iterChi2NDF_derRange",chi2_2D_range.at(i));
            } 

            flat_tuple_->fill();
            delete projy_h;
        }

    }

}

