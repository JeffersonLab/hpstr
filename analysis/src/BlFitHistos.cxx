#include "BlFitHistos.h"

BlFitHistos::BlFitHistos() {
    //ModuleMapper used to translate between hw and sw names
    mmapper_ = new ModuleMapper();

    //Global svtIDMap built in ModuleMapper. Required to output baselines in database format
    svtIDMap = mmapper_->buildChannelSvtIDMap();
}

BlFitHistos::~BlFitHistos() {
}

void BlFitHistos::getHistosFromFile(TFile* inFile, std::string layer){
    
    for (auto hist: _h_configs.items()) {
        std::string h_name = hist.key();
        if (h_name.find(layer) == std::string::npos)
            continue;
        std::size_t found = (hist.key()).find_last_of("_");
        std::string extension = hist.key().substr(found+1);

        TIter next(inFile->GetListOfKeys());
        TKey *key;
        bool got = false;
        std::string histoname;
        while ((key = (TKey*)next())) {
            std::string name(key->GetName());
            if (name.find(h_name) != std::string::npos){
                TH2F *hh = (TH2F*) inFile-> Get(key->GetName());
                histos2d[key->GetName()] = hh;
            }
        }
    }
}

double fitmin;
double fitmax;

TF1* BlFitHistos::singleGausIterative(TH1D* hist, double sigmaRange, double min = -1., double max = -1., bool iterchi2 = false) {
    //perform single Gaus fit across full range of histo
    //projy_h->GetBinLowEdge(projy_h->FindFirstBinAbove(0.0,1)), projy_h->GetBinLowEdge(projy_h->FindLastBinAbove(0.0,1)))
    if (min < 0.0 )
        min = hist->GetBinLowEdge(hist->FindFirstBinAbove(0.0,1));

    if (max < 0.0 )
        max = hist->GetBinLowEdge(hist->FindLastBinAbove(0.0,1));

    TF1 *fitA = new TF1("fitA", "gaus", min, max);
    hist->Fit("fitA","ORQN","");
    double fitAMean = fitA->GetParameter(1);
    double fitASig = fitA->GetParameter(2);
    std::cout << "fitA mean " << fitAMean << std::endl;
    std::cout << "fitA sigma " << fitASig << std::endl;
    delete fitA;

    //if fitAMean is > max, increment xmin up and refit
    int iter = 0;
    if( fitAMean > max ){
        std::cout << "initial fit mean > max. Increment xmin up" << std::endl;
        double binwidth = hist->GetBinWidth(hist->FindFirstBinAbove(0.0,1));
        double newxmin = min;
        while (fitAMean > max || iter < 10){
            newxmin = newxmin + 20*binwidth;
            std::cout << "new xmin is " << newxmin << std::endl;
            TF1 *fitA = new TF1("fitA", "gaus", newxmin, max);
            hist->Fit("fitA","ORQN","");
            fitAMean = fitA->GetParameter(1);
            fitASig = fitA->GetParameter(2);
            std::cout << "fitA mean iter " << iter << " is " << fitAMean << std::endl;
            std::cout << "fitA sigma iter" << iter << " is " << fitASig << std::endl;
            iter = iter + 1;
        }
    }

    std::cout << "fitA mean after iter " << fitAMean << std::endl;
    std::cout << "fitA sigma after iter " << fitASig << std::endl;

    //perform second fit with range determined by first fit
    if((fitAMean + (fitASig*sigmaRange)) <= max)
        max = fitAMean + (fitASig*sigmaRange);
    //else
    //    max = fitAMean + (fitASig*sigmaRange);

    if((fitAMean - (fitASig*sigmaRange)) >= min && (fitAMean - (fitASig*sigmaRange) < max))
        min = fitAMean - (fitASig*sigmaRange);
    //else
    //    min = fitAMean - (fitASig*sigmaRange);

    std::cout << "fitA max " << max << std::endl;
    std::cout << "fitA min " << min << std::endl;

    TF1 *fitB = new TF1("fitB", "gaus", min, max);
    hist->Fit("fitB","ORQN","");
    double fitMean = fitB->GetParameter(1);
    double fitSig = fitB->GetParameter(2);

    double newFitSig = 99999;
    double newFitMean = 99999;
    int i = 0;

    if((fitMean + (fitSig*sigmaRange)) <= max)
        max = fitMean + (fitSig*sigmaRange);
    //else
    //    max = fitMean + (fitSig*sigmaRange);

    if((fitMean - (fitSig*sigmaRange)) >= min && (fitMean - (fitSig*sigmaRange) < max))
        min = fitMean - (fitSig*sigmaRange);
    //else
    //    min = fitMean - (fitSig*sigmaRange);

    /*
    max = fitMean + (fitSig*sigmaRange);
    if((fitMean - (fitSig*sigmaRange)) >= min)
        min = fitMean - (fitSig*sigmaRange);
        */

    std::cout << "fit2 max " << max << std::endl;
    std::cout << "fit2 min " << min << std::endl;
    //TFitResultPtr fit = hist->Fit("gaus", "QRES", "same", min, max);
    TF1 *fit = new TF1("fit", "gaus", min, max);
    hist->Fit("fit","ORQN","");

    while ( std::abs(fitSig - newFitSig) > 0.0005 or std::abs(fitMean - newFitMean) > 0.0005) {

        fitMean = newFitMean;
        fitSig = newFitSig;

        if((fitMean + (fitSig*sigmaRange)) <= max)
            max = fitMean + (fitSig*sigmaRange);
        //else
        //    max = fitMean + (fitSig*sigmaRange);

        if((fitMean - (fitSig*sigmaRange)) >= min && (fitMean - (fitSig*sigmaRange) < max))
            min = fitMean - (fitSig*sigmaRange);
        //else
        //    min = fitMean - (fitSig*sigmaRange);

        //max = fitMean + (fitSig*sigmaRange);
        //if((fitMean - (fitSig*sigmaRange)) >= min)
        //    min = fitMean - (fitSig*sigmaRange);
        fit->SetRange(min,max);
        hist->Fit("fit","ORQN","");
        newFitMean = fit->GetParameter(1);
        newFitSig = fit->GetParameter(2);
        if(i > 50)
            break;
        i = i + 1;

    }

    /*

    //BELOW DIDNT WORK. FIT NORM LOOKS WRONG, BUT MEAN IS RIGHT...?
    std::cout << "[fit] max " << max << std::endl;
    std::cout << "[fit] min " << min << std::endl;
    double chi2 = fit->GetChisquare();
    double ndf = fit->GetNDF();

    double iter = min;
    while (iter < fitMean-fitSig){
        iter = iter + hist->GetBinWidth(hist->FindFirstBinAbove(0));
        fit->SetRange(iter, max);
        hist->Fit("fit","ORQN","");
        if ( fit->GetChisquare()/fit->GetNDF() < 0.8*chi2/ndf){
            chi2 = fit->GetChisquare();
            ndf = fit->GetNDF();
            min = iter;
        }
        else
            break;
    }

    fit->SetRange(min, max);
    hist->Fit("fit","ORQN","");
    */

    fitmin = min;
    fitmax = max;

    std::cout << "end fit min: " << fitmin << std::endl;
    std::cout << "end fit max: " << fitmax << std::endl;
    return fit;

}

void BlFitHistos::iterativeChi2GausFit(TH1D* hist, TF1* fit, double min, double max){
    double fitAMean = fit->GetParameter(1);
    double fitASig = fit->GetParameter(2);
    double chi2A = fit->GetChisquare();
    double ndfA = fit->GetNDF();

    double maxA = max;

    double iter = hist->GetBinWidth(hist->FindFirstBinAbove(0));
    std::cout << "initial mean: " << fitAMean << std::endl;
    std::cout << "max = " << max << std::endl;
    while (max > fitAMean){
        max = max - (20*iter);
        fit->SetRange(min,max);
        hist->Fit("fit","ORQN","same");
        double chi2 = fit->GetChisquare();
        double ndf = fit->GetNDF();
        if(ndf == 0)
            continue;
        std::cout << "ogchi2ndf " << chi2A/ndfA << std::endl;
        std::cout << "chi2/ndf " << chi2/ndf << std::endl;
        if(chi2/ndf < 0.8*(chi2A/ndfA)){
            maxA = max;
            chi2A = chi2;
            ndfA = ndf;
            std::cout << "new max = " << max << std::endl;
        }
        else{
            fit->SetRange(min,maxA);
            hist->Fit("fit","ORQN","");
            break;
        }
    }
}
    

void BlFitHistos::Chi2GausFit(std::map<std::string,TH2F*> histos2d, int nPointsDer_,int rebin_,int xmin_, int minStats_, int noisyRMS_, int deadRMS_, FlatTupleMaker* flat_tuple_) {
     
    //Get half module string names 
    std::vector<std::string> halfmodule_strings;
    mmapper_->getStrings(halfmodule_strings);

    //Loop over rawsvthit 2D histograms, one for each selected halfmodule
    for(std::map<std::string, TH2F*>::iterator it = histos2d.begin(); it != histos2d.end(); ++it)
    {
        TH2F* halfmodule_hh = it->second; 
        halfmodule_hh->RebinY(rebin_);
        halfmodule_hh->Write();
        std::string hh_name = it->first;

        //get the hardware tag for this F<n>H<M>. Required for svtid mapping
        std::string hwTag;

        for(std::vector<std::string>::iterator it = halfmodule_strings.begin(); it != halfmodule_strings.end(); ++it){
            if(hh_name.find(*it) != std::string::npos){
                hwTag = mmapper_->getHwFromString(*it);
                std::cout << "hwTag for " << hh_name << " is " << hwTag << std::endl;
                break;
            }
        }
        
        //Perform fitting procedure over all channels on a sensor
        for(int cc=390; cc < 400 ; ++cc) 
        {
            std::cout << hh_name << " " << cc << std::endl;

            //get the global svt_id for channel
            int svt_id = mmapper_->getSvtIDFromHWChannel(cc, hwTag, svtIDMap);
            if(svt_id == 99999) //Feb 0-1 have max_channel = 512. svt_id = 99999 means max_channel reached. Skip cc > 512 
                continue;

            //Set Channel and Hybrid information and paramaters in the flat tuple
            flat_tuple_->setVariableValue("halfmodule_hh", hh_name);
            flat_tuple_->setVariableValue("channel", cc);
            flat_tuple_->setVariableValue("svt_id", svt_id);
            flat_tuple_->setVariableValue("minbinThresh",(double)xmin_);
            flat_tuple_->setVariableValue("minStats", (double)minStats_);
            flat_tuple_->setVariableValue("rebin", (double)rebin_);


            //Get YProjection (1D Channel Histo) from 2D Histogram 
            TH1D* projy_h = halfmodule_hh->ProjectionY(Form("%s_proY_ch%i",hh_name.c_str(),cc),
                    cc+1,cc+1,"e");
            projy_h->SetTitle(Form("%s_proY_ch%i",hh_name.c_str(),cc));

            //Check number of entries and RMS of channel
            flat_tuple_->setVariableValue("n_entries", projy_h->GetEntries());
            double chRMS = projy_h->GetRMS();
            flat_tuple_->setVariableValue("rms", chRMS);

            //If channel is "dead" set flag
            if(chRMS < deadRMS_ || projy_h->GetEntries() == 0)
                flat_tuple_->setVariableValue("dead",1.0);

            //xmin is the start of the fit window. iterxmax will initially be iteratively fit and
            //then increased until some maximum allowed value, or until the chi2/Ndf > 100
            double maxbin = projy_h->GetBinContent(projy_h->GetMaximumBin());
            double frac = 0.20;
            int firstbin = projy_h->FindFirstBinAbove((double)frac*maxbin,1);
            double xmin = projy_h->GetBinLowEdge(firstbin);
            double binwidth = projy_h->GetBinWidth(firstbin);

            //If channel does not have the minimum statistics required, set all variables to -9999.9
            //and skip the fit procedure on this channel
            if (firstbin == -1 || projy_h->GetEntries() < minStats_ ) 
            {
                std::cout << "FAILED TO FIT CHANNEL" << std::endl;
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
                flat_tuple_->setVariableValue("TFitResultError", -9999.9);

                flat_tuple_->setVariableValue("minStatsFailure",1.0);

                flat_tuple_->fill();
                continue;
            }
            flat_tuple_->setVariableValue("minStatsFailure",0.0);

            //Define all iterative and final fit parameters
            std::vector<double> amp,mean,sigma,chi2,const_err,sigma_err,
                fit_range_end,chi2_NDF,chi2_2D,chi2_1D,der2chi2,der2chi2R;
            std::vector<int> NDF;

            //Baseline signals are composed of a gaussian baseline, followed by a landau pile-up
            //It is found that at the boundary of these two distributions, the
            //second derivative of the Chi2/Ndf of a fit is maximized. 
            //Therefore, in order to determine the correct fit window, i.e. xmin and xmax, that
            //will fit a gaussian over just the baseline distribution, the fit works as follows:
            //Iterate xmax across the ADC distribution. At each new value of iterxmax, perform
            //a fit on the histogram. Add Chi2/Ndf for the iterxmax of each fit to a vector.
            //Locate the iterxmax value that corresponds to the maximum Chi2/Ndf 2nd derivative. This
            //is where the fit window should end (xmax) 

            
            //If baseline fitting an online baseline, must set simpleGauseFit_ to true!
            if(simpleGausFit_ == true){
                TF1* simpleFit = singleGausIterative(projy_h, 2.0,xmin);
                const double* parameters;
                parameters = simpleFit->GetParameters();
                flat_tuple_->setVariableValue("BlFitMean", parameters[1]);
                flat_tuple_->setVariableValue("BlFitSigma", parameters[2]);
                flat_tuple_->setVariableValue("BlFitNorm", parameters[0]);
                flat_tuple_->setVariableValue("BlFitChi2", simpleFit->GetChisquare());
                flat_tuple_->setVariableValue("BlFitNdf", simpleFit->GetNDF());
                flat_tuple_->setVariableValue("BlFitRangeLower", fitmin);
                flat_tuple_->setVariableValue("BlFitRangeUpper", fitmax);

                flat_tuple_->fill();
                delete simpleFit;
                delete projy_h;
                continue;
            }

            //iteratively fit distribution with gaussian until Chi2 blows up larger than 100
            int iter = 0;
            double iterxmax = xmin + 20.0*binwidth;
            double currentChi2 = 0.0;
            while(iterxmax < projy_h->GetBinLowEdge(projy_h->FindLastBinAbove(0.10*maxbin)) && currentChi2 < 50.0 || iter < 20)
            {
                TFitResultPtr cc_fit = projy_h->Fit("gaus", "QRES", "", xmin, iterxmax);
                if(cc_fit->Ndf() == 0)
                {
                    iterxmax = iterxmax + binwidth;
                    continue;
                }

                //Return iterative fit parameters and add to flat tuple
                flat_tuple_->addToVector("iterMean", cc_fit->GetParams()[1]);
                chi2.push_back(cc_fit->Chi2());
                NDF.push_back(cc_fit->Ndf());
                chi2_NDF.push_back(chi2.at(iter)/NDF.at(iter));
                flat_tuple_->addToVector("iterChi2NDF", chi2_NDF.at(iter));
                fit_range_end.push_back(iterxmax);
                flat_tuple_->addToVector("iterFitRangeEnd", iterxmax);

                //Increase fit range window and iterate the fit
                iterxmax = iterxmax + binwidth;
                iter++;
                currentChi2 = cc_fit->Chi2()/cc_fit->Ndf();
            }

            //Calculate the 2nd derivative of chi2 by taking slope on either side of one point
            //and taking the difference between the two. The xposition of the maximum Chi2 2nd 
            //derivative is used to define the maximum fit range x-position

            int nPointsDer_ = 3;
            for(int i = nPointsDer_; i < chi2_NDF.size() - nPointsDer_; i++) 
            {
                double derForward=(chi2_NDF.at(i+nPointsDer_)-chi2_NDF.at(i))/(nPointsDer_*binwidth);
                double derBack = (chi2_NDF.at(i)-chi2_NDF.at(i-nPointsDer_))/(nPointsDer_*binwidth);
                double der = (derForward+derBack)/2.0;

                if(der == der) 
                {
                    chi2_1D.push_back(der);
                }
                else
                {
                    chi2_1D.push_back(-9999.9);
                }
                flat_tuple_->addToVector("iterChi2NDF_1der",der);
            }

            for(int i = 3; i < chi2_1D.size() - 3; i++)
            {
                double derForward=(chi2_1D.at(i+3)-chi2_1D.at(i))/(3*binwidth);
                double derBack = (chi2_1D.at(i)-chi2_1D.at(i-3))/(3*binwidth);
                double der = (derForward+derBack)/2.0;

                if(der == der) 
                {
                    chi2_2D.push_back(der);
                }
                else
                {
                    chi2_2D.push_back(-9999.9);
                }
                flat_tuple_->addToVector("iterChi2NDF_2der",der);
            }

            //Create subrange for chi2_2D that accounts for derivative requiring <n> points prior  
            //and post point of interest
            std::vector<double>::const_iterator first = fit_range_end.begin()+(3+nPointsDer_);
            std::vector<double>::const_iterator last=fit_range_end.begin()+3+nPointsDer_+chi2_2D.size();
            std::vector<double> chi2_2D_range(first,last);
            
            //Find maximum chi2 second derivative
            double chi2_2D_max = *std::max_element(chi2_2D.begin(), chi2_2D.end());
            int chi2_2D_maxIndex = std::max_element(chi2_2D.begin(), chi2_2D.end()) - chi2_2D.begin();
            double chi2_2D_xmax = chi2_2D_range.at(chi2_2D_maxIndex);
            //xmax defines the end of the fit window over the gaussian distribution. Ideally this
            //position occurs at the boundary of the baseline and landau pile-up shapes
            double xmax = chi2_2D_xmax;

            //Refit the baseline using the new fit window that ends where the iterative fit chi2 has the largest second derivative
            std::cout << "performing initial fit" << std::endl;
            std::cout << "xmax = " << xmax << std::endl;
            std::cout << "xmin = " << xmin << std::endl;

            TF1* fit = singleGausIterative(projy_h, 1.0,xmin,xmax, false);
            const double* parameters;
            parameters = fit->GetParameters();
            double fitmean = parameters[1];
            double fitsigma = parameters[2];
            double fitnorm = parameters[0];
            double fitchi2 = fit->GetChisquare();
            double fitndf = fit->GetNDF();

            std::cout << "post initial fitmean = " << fitmean << std::endl;
            std::cout << "post initial fitmax = " << fitmax << std::endl;
            std::cout << "post initial fitmin = " << fitmin << std::endl;
            delete fit;


            //If a channel matches any of the below criteria:
            //1. If xmax or xmin occur within N*Sigma of the fit, indicative of a partial fit or a
            //lowDaq-threshold channel

            if (fitmean > xmax){
                std::cout << "fitmean > xmax. Refit" << std::endl;
                double newmax = projy_h->GetBinLowEdge(projy_h->GetMaximumBin()) - 2*binwidth;
                std::cout << "old max: " << xmax << "| new max = " << newmax << std::endl;
                TF1* lowdaqfit = singleGausIterative(projy_h, 2.0, xmin, newmax, true);

                parameters = lowdaqfit->GetParameters();
                fitmean = parameters[1];
                fitsigma = parameters[2];
                fitnorm = parameters[0];
                fitchi2 = lowdaqfit->GetChisquare();
                fitndf = lowdaqfit->GetNDF();

                std::cout << "post final fitmean = " << fitmean << std::endl;
                std::cout << "post final fitmax = " << fitmax << std::endl;
                std::cout << "post final fitmin = " << fitmin << std::endl;

                delete lowdaqfit;
            }

            //Check if channel is lowdaq
            //If the maximum bin in a distribution occurs outside of the fit by some level of 
            //sigma, AND the average around the maximum bin is greater than the mean of the fit, the
            //channel is lowDaq but fitting an improper gaussian over a messy distribution. 
            //(VISUAL EXAMPLES OF DISTRIBUTIONS NEEDED TO UNDERSTAND)
            
            //LOWDAQ THRESHOLD FLAG
            //If fit mean + N*Sigma > xmax, channel has low daq threshold *or* bad fit
            double ymax = 0;
            for (int i = 0; i < 10; i++){
                ymax = ymax + projy_h->GetBinContent(projy_h->GetMaximumBin() + i);
            }
            ymax = ymax/10.0;

            double maxbinx = projy_h->GetBinLowEdge(projy_h->GetMaximumBin());
            double maxavg = 0;
            for (int i = 0; i < 3; i++){
                maxavg = maxavg + projy_h->GetBinContent(projy_h->GetMaximumBin() + i); 
            }
            maxavg = maxavg/3;

            //if (maxbinx > fitmean + fitsigma && (maxavg >  

            //Fill fit values
            flat_tuple_->setVariableValue("BlFitMean", fitmean);
            flat_tuple_->setVariableValue("BlFitSigma", fitsigma);
            flat_tuple_->setVariableValue("BlFitNorm", fitnorm);
            flat_tuple_->setVariableValue("BlFitChi2", fitchi2);
            flat_tuple_->setVariableValue("BlFitNdf", fitndf);
            flat_tuple_->setVariableValue("BlFitRangeLower", fitmin);
            flat_tuple_->setVariableValue("BlFitRangeUpper", fitmax);
            for(int i=0; i < chi2_2D_range.size(); ++i) 
            {
                flat_tuple_->addToVector("iterChi2NDF_derRange",chi2_2D_range.at(i));
            }

            flat_tuple_->fill();

            delete projy_h;
            continue;
                        
            /*
            if ((fitparams[1] + N*fitparams[2] > xmax) || (fitparams[1] - N*fitparams[2] < xmin) || ( (projy_h->GetBinLowEdge(projy_h->GetMaximumBin()) > fitparams[1] + 0.1*fitparams[2]) && (maxvalavg > fitparams[0]))) 
            {
                //Fix xmax to the location of the maximum bin value, which should generally
                //be the required end of the fit window for the baseline distribution.
                //Iterate xmin up until the fit conforms to the baseline.
                double tempxmin = xmin;
                xmax = projy_h->GetBinLowEdge(projy_h->GetMaximumBin()) - 2*binwidth;
                double ogChi2Ndf = fit->Chi2()/fit->Ndf();
                if (xmax-xmin < 300)
                {
                    tempxmin = xmax - 300;
                    
                }
                //Minimum Fit window size seems to be necessary to avoid empty TFitResultPtr...?
                //200 chosen based on distribution of window sizes, but may need adjusting...
                while(xmin < (xmax - 200.0))
                {
                    tempxmin = tempxmin + binwidth;
                    TFitResultPtr fit = projy_h->Fit("gaus", "QRES", "", tempxmin, xmax);
                    if ( fit == -1) 
                    {
                        TFRE = TFRE*11.0;
                        flat_tuple_->setVariableValue("TFitResultError",TFRE);
                        continue;
                    }
                    else
                    {
                        if ( fit->Chi2()/fit->Ndf() < 0.8*ogChi2Ndf )
                        {
                            ogChi2Ndf = fit->Chi2()/fit->Ndf();
                            xmin = tempxmin;
                        }
                       else 
                       {
                            break;
                       }
                    }

                }
            }


            //After attempting to refit baseline above, if lowDaq criteria is still met, label
            //channel as lowDaq and stop trying to refit.
            if ( fitparams[1] + N*fitparams[2] > xmax || fitparams[1] - N*fitparams[2] < xmin  )
            {
                flat_tuple_->setVariableValue("lowdaq", 1.0);
            }
            else 
            {
                double ogChi2Ndf = fit->Chi2()/fit->Ndf();
                while ( xmax > (xmin + 100))
                {
                   double tempxmax = xmax;
                   tempxmax = tempxmax - 5*binwidth; 
                   TFitResultPtr newfit = projy_h->Fit("gaus", "QRES", "", xmin, tempxmax);
                    if ( newfit == -1) 
                    {
                        TFRE = TFRE*13.0;
                        flat_tuple_->setVariableValue("TFitResultError",TFRE);
                        continue;
                    }
                   else 
                   {
                       if ( newfit->Chi2()/newfit->Ndf() < 0.8*ogChi2Ndf )
                       {
                            ogChi2Ndf = newfit->Chi2()/newfit->Ndf();
                            xmax = tempxmax;
                       }
                       else 
                       {
                            break;
                       }
                   }
                }

                flat_tuple_->setVariableValue("lowdaq", 0.0);
            }

            fit = projy_h->Fit("gaus", "QRES", "", xmin, xmax);
            if ( fit == -1) 
            {
                TFRE = TFRE*17.0;
                flat_tuple_->setVariableValue("TFitResultError",TFRE);
                continue;
            }
            else
            {
                flat_tuple_->setVariableValue("TFitResultError",TFRE);
                fitparams = fit->GetParams();
            }

            //Store Final Fit Parameters in the flat tuple
            flat_tuple_->setVariableValue("BlFitMean", fitparams[1]);
            flat_tuple_->setVariableValue("BlFitSigma", fitparams[2]);
            flat_tuple_->setVariableValue("BlFitNorm", fitparams[0]);
            flat_tuple_->setVariableValue("BlFitRangeLower", (double)xmin);
            flat_tuple_->setVariableValue("BlFitRangeUpper", (double)xmax);
            flat_tuple_->setVariableValue("BlFitChi2", fit->Chi2());
            flat_tuple_->setVariableValue("BlFitNdf", fit->Ndf());
            for(int i=0; i < chi2_2D_range.size(); ++i) 
            {
                flat_tuple_->addToVector("iterChi2NDF_derRange",chi2_2D_range.at(i));
            }

        flat_tuple_->fill();
        delete projy_h;
        */
        }
    }

}

