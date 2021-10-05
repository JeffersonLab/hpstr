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
        std::cout << "h_name: " << h_name << std::endl;
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

//Globally used fit window
double fitmin;
double fitmax;

TF1* BlFitHistos::singleGausIterative(TH1D* hist, double sigmaRange, double min = -1., double max = -1.) {
    //perform single Gaus fit across full range of histo
    if (min < 0.0 )
        min = hist->GetBinLowEdge(hist->FindFirstBinAbove(0.0,1));

    if (max < 0.0 )
        max = hist->GetBinLowEdge(hist->FindLastBinAbove(0.0,1));

    //Initial fit to establish rough mean and sigma
    TF1 *fitA = new TF1("fitA", "gaus", min, max);
    hist->Fit("fitA","ORQN","");
    double fitAMean = fitA->GetParameter(1);
    double fitASig = fitA->GetParameter(2);
    delete fitA;

    //perform second fit with range determined by first fit
    if((fitAMean + (fitASig*sigmaRange)) <= max)
        max = fitAMean + (fitASig*sigmaRange);

    if((fitAMean - (fitASig*sigmaRange) < max) && (fitAMean - (fitASig*sigmaRange) > hist->GetBinLowEdge(hist->FindFirstBinAbove(0.0,1))))
        min = fitAMean - (fitASig*sigmaRange);

    //Fit second time using updated min and max
    TF1 *fitB = new TF1("fitB", "gaus", min, max);
    hist->Fit("fitB","ORQN","");
    double fitMean = fitB->GetParameter(1);
    double fitSig = fitB->GetParameter(2);

    double newFitSig = 99999;
    double newFitMean = 99999;
    int i = 0;

    if((fitMean + (fitSig*sigmaRange)) <= max)
        max = fitMean + (fitSig*sigmaRange);
    if((fitMean - (fitSig*sigmaRange) < max) && (fitMean - (fitSig*sigmaRange) > hist->GetBinLowEdge(hist->FindFirstBinAbove(0.0,1))))
        min = fitAMean - (fitASig*sigmaRange);

    TF1 *fit = new TF1("fit", "gaus", min, max);
    hist->Fit("fit","ORQN","");

    while ( std::abs(fitSig - newFitSig) > 0.0005 or std::abs(fitMean - newFitMean) > 0.0005) {

        fitMean = newFitMean;
        fitSig = newFitSig;

        if((fitMean + (fitSig*sigmaRange)) <= max)
            max = fitMean + (fitSig*sigmaRange);
        if((fitMean - (fitSig*sigmaRange) < max) && (fitMean - (fitSig*sigmaRange) > hist->GetBinLowEdge(hist->FindFirstBinAbove(0.0,1))))
            min = fitAMean - (fitASig*sigmaRange);
        fit->SetRange(min,max);
        hist->Fit("fit","ORQN","");
        newFitMean = fit->GetParameter(1);
        newFitSig = fit->GetParameter(2);
        if(i > 50)
            break;
        i = i + 1;

    }

    fitmin = min;
    fitmax = max;

    return fit;
}

TF1* BlFitHistos::backwardsIterativeChi2Fit(TH1D* hist, double min, double max){

    //Initial fit using fit window
    TF1 *fit = new TF1("fit", "gaus", min, max);
    hist->Fit("fit","ORQN","");
    double fitAMean = fit->GetParameter(1);
    double fitASig = fit->GetParameter(2);
    double chi2A = fit->GetChisquare();
    double ndfA = fit->GetNDF();
    double fitAchi2 = chi2A/ndfA;
    double maxA = max;
    double minA = min;

    int iterator = 0;
    double fitBMean = fitAMean;
    double fitBSig = fitASig;
    double chi2B;
    double ndfB;
    double newchi2ndf = 999999;

    while (true && iterator < 10){
        std::vector<double> chi2_ndf_max;
        std::vector<double> xmaxvals;
        //Iterate fit max backwards and save chi2/ndf for each value
        //Restrict value of fitmax to be greater than initial fit mean + 0.5sigma, to prevent iterating backwards too far
        while (max > (fitBMean + 0.5*fitBSig) ){
            fit->SetRange(min,max);
            hist->Fit("fit","ORQN","");
            fitBMean = fit->GetParameter(1);
            fitBSig = fit->GetParameter(2);
            chi2B = fit->GetChisquare();
            ndfB = fit->GetNDF();
            if (ndfB && ndfB != 0){
                chi2_ndf_max.push_back(chi2B/ndfB);
                xmaxvals.push_back(max);
            }
            max = max - 10*hist->GetBinWidth(hist->FindFirstBinAbove(0.0,1));
        }

        //Find fitmax value corresponding to smallest chi2/ndf
        if(chi2_ndf_max.size() > 0){
            int bestindex = std::min_element(chi2_ndf_max.begin(), chi2_ndf_max.end()) - chi2_ndf_max.begin();
            max = xmaxvals.at(bestindex);
            //Fit with new max value, corresponding to smallest chi2/ndf
            fit->SetRange(min,max);
            hist->Fit("fit","ORQN","");
            fitBMean = fit->GetParameter(1);
            fitBSig = fit->GetParameter(2);
            chi2B = fit->GetChisquare();
            ndfB = fit->GetNDF();
            //If new fit has better chi2/ndf than original fit, set fitmax globally
            if(chi2B/ndfB < fitAchi2){
                newchi2ndf = chi2B/ndfB;
                fitmax = max;
            }
            //else return to original fit
            else{
                fitBMean = fitAMean;
                fitBSig = fitASig;
                chi2B = chi2A;
                ndfB = ndfA;
                max = maxA;
            }
        }
        //If no iteration, return to original fit
        else{
            fitBMean = fitAMean;
            fitBSig = fitASig;
            chi2B = chi2A;
            ndfB = ndfA;
            max = maxA;
        }

        //iterate fitmin backwards
        std::vector<double> chi2_ndf_min;
        std::vector<double> xminvals;
        while(min > hist->GetBinLowEdge(hist->FindFirstBinAbove(0.0,1))){ 
            //Refit now that max may have been updated
            fit->SetRange(min,max);
            hist->Fit("fit","ORQN","");
            fitBMean = fit->GetParameter(1);
            fitBSig = fit->GetParameter(2);
            chi2B = fit->GetChisquare();
            ndfB = fit->GetNDF();
            if (ndfB && ndfB != 0){
                chi2_ndf_min.push_back(chi2B/ndfB);
                xminvals.push_back(min);
            }
            //iterate fitmin backwards
            min = min - 10*hist->GetBinWidth(hist->FindFirstBinAbove(0.0,1));
        }

        //Refit with fitmin that corresponds to smallest chi2/ndf
        if(chi2_ndf_min.size() > 0){
            int bestindex = std::min_element(chi2_ndf_min.begin(), chi2_ndf_min.end()) - chi2_ndf_min.begin();
            min = xminvals.at(bestindex);
            fit->SetRange(min,max);
            hist->Fit("fit","ORQN","");
            fitBMean = fit->GetParameter(1);
            fitBSig = fit->GetParameter(2);
            chi2B = fit->GetChisquare();
            ndfB = fit->GetNDF();
            //If chi2/ndf improved, update global fit window
            if(chi2B/ndfB < newchi2ndf && chi2B/ndfB < fitAchi2){
                fitmin = min;
                newchi2ndf = chi2B/ndfB;
            }
            else{
                break;
            }
        }
        else{
            break;
        }
        iterator = iterator + 1;
    }

    return fit;
}


void BlFitHistos::GausFitHistos2D(std::map<std::string,TH2F*> histos2d, int rebin_, int minStats_, int deadRMS_, FlatTupleMaker* flat_tuple_) {
     
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
                if(debug_)
                    std::cout << "hwTag for " << hh_name << " is " << hwTag << std::endl;
                break;
            }
        }
        //For online baselines we must use simple gaussian fit, as online and offline distributions differ dramatically
        //and the algorithm developed for offline fits does not work for online
        if(simpleGausFit_ == true){
            std::cout << "WARNING! PERFORMING ***ONLINE*** BASELINE FITS USING SIMPLE GAUSSIAN FIT!" << std::endl;
            std::cout << "If attempting to fit ***offline*** baseline, set 'simpleGausFit' to False" << std::endl;
        }
        else{
            std::cout << "WARNING! PERFORMING ***OFFLINE*** BASELINE FITS!" << std::endl;
            std::cout << "If attempting to fit ***online*** baseline, set 'simpleGausFit' to True" << std::endl;
        }
        
        //Perform fitting procedure over all channels on a sensor
        for(int cc=0; cc < 640 ; ++cc) 
        {
            if(debug_)
                std::cout << hh_name << " " << cc << std::endl;

            //get the global svt_id for channel
            int svt_id = mmapper_->getSvtIDFromHWChannel(cc, hwTag, svtIDMap);
            if(debug_)
                std::cout << "Global SVT ID: " << svt_id << std::endl;
            if(svt_id == 99999) //Feb 0-1 have max_channel = 512. svt_id = 99999 means max_channel reached. Skip cc > 512 
                continue;

            //Set Channel and Hybrid information and paramaters in the flat tuple
            flat_tuple_->setVariableValue("halfmodule_hh", hh_name);
            flat_tuple_->setVariableValue("channel", cc);
            flat_tuple_->setVariableValue("svt_id", svt_id);
            flat_tuple_->setVariableValue("minStats", (double)minStats_);
            flat_tuple_->setVariableValue("rebin", (double)rebin_);


            //Get YProjection (1D Channel Histo) from 2D Histogram 
            TH1D* projy_h = halfmodule_hh->ProjectionY(Form("%s_proY_ch%i",hh_name.c_str(),cc),
                    cc+1,cc+1,"e");
            projy_h->Smooth(1);
            projy_h->SetTitle(Form("%s_proY_ch%i",hh_name.c_str(),cc));

            //Check number of entries and RMS of channel
            flat_tuple_->setVariableValue("n_entries", projy_h->GetEntries());
            double chRMS = projy_h->GetRMS();
            flat_tuple_->setVariableValue("rms", chRMS);

            //If the channel RMS is under some threshold, flag it as "dead"
            if(chRMS < deadRMS_ || projy_h->GetEntries() == 0)
                flat_tuple_->setVariableValue("dead",1.0);

            //xmin is the start of the fit window. iterxmax will initially be iteratively fit and
            //then increased until some maximum allowed value, or until the chi2/Ndf > 100
            double maxbin = projy_h->GetBinContent(projy_h->GetMaximumBin());
            //firstbin is first bin that contains some fraction of bin with most entries
            double frac = 0.10;
            int firstbin = projy_h->FindFirstBinAbove((double)frac*maxbin,1);
            double xmin = projy_h->GetBinLowEdge(firstbin);
            double binwidth = projy_h->GetBinWidth(firstbin);

            //If channel does not have the minimum statistics required, set all variables to -9999.9
            //and skip the fit procedure on this channel
            if (firstbin == -1 || projy_h->GetEntries() < minStats_ ) 
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
                flat_tuple_->addToVector("iterFitRangeEnd", -9999.9);
                flat_tuple_->setVariableValue("lowdaq", -9999.9);
                flat_tuple_->setVariableValue("lowStats",1.0);
                flat_tuple_->setVariableValue("badfit",0.0);
                flat_tuple_->fill();
                continue;
            }
            flat_tuple_->setVariableValue("lowStats",0.0);

            //Define all iterative and final fit parameters
            std::vector<double> amp,mean,sigma,chi2,const_err,sigma_err,
                fit_range_end,chi2_NDF,chi2_2D,chi2_1D,der2chi2,der2chi2R;
            std::vector<int> NDF;

            //Baseline signals are composed of a gaussian baseline, followed by a landau pile-up
            //It is found that at the boundary of these two distributions, the
            //second derivative of the Chi2/Ndf of the iterative fit is maximized. 
            //Therefore, in order to determine the correct fit window, i.e. xmin and xmax, that
            //will fit a gaussian over just the baseline, the fit works as follows:
            //Iterate xmax across the ADC distribution. At each new value of iterxmax, perform
            //a fit on the histogram. Add Chi2/Ndf for the iterxmax of each fit to a vector.
            //Locate the iterxmax value that corresponds to the maximum Chi2/Ndf 2nd derivative. This
            //is where the fit window should end (xmax) 
            
            //If baseline fitting an online baseline, must set simpleGauseFit_ to true!
            if(simpleGausFit_ == true){
                TF1* simpleFit = singleGausIterative(projy_h, 1.5,xmin);
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

            //If fitting an online baseline (which has gaussian and landau)
            //iteratively fit distribution with gaussian until Chi2 blows up larger than 100
            int iter = 0;
            double iterxmax = xmin + 20.0*binwidth;
            double currentChi2 = 0.0;
            //Require at least 20 iterations regardless of chi2
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
            TF1* fit = singleGausIterative(projy_h, 1.5,xmin,xmax);
            const double* parameters;
            parameters = fit->GetParameters();
            double fitmean = parameters[1];
            double fitsigma = parameters[2];
            double fitnorm = parameters[0];
            double fitchi2 = fit->GetChisquare();
            double fitndf = fit->GetNDF();

            delete fit;

            //Some fits extend beyond the optimal fitmax position. 
            //After getting the base fit, iteratite fitmax backwards and check for improvements in chi2/ndf of the fit
            //If chi2/ndf improves, retain fitmax, and then iterate xmin backwards similarly
            //Walking the fit window backwards this way helps fit converge in cases where shape is complicatd by low daq threshold
            fit = backwardsIterativeChi2Fit(projy_h, fitmin, fitmax);
            parameters = fit->GetParameters();
            fitmean = parameters[1];
            fitsigma = parameters[2];
            fitnorm = parameters[0];
            fitchi2 = fit->GetChisquare();
            fitndf = fit->GetNDF();

            //Check if fit is grossly bad
            bool badfit = false;
            //If fitmean > fitmax or fitmean < fitmin...flag
            if(fitmean > fitmax || fitmean < fitmin)
                badfit = true;

            bool lowdaq = false;
            if(!badfit){
                //Check channel to see if it has a low DAQ threshold, where landau shape interferes with baseline
                //If maxbin occurs outside of fit mean by NSigma...flag
                double maxbinx = projy_h->GetBinLowEdge(projy_h->GetMaximumBin());
                if ((std::abs(maxbinx - fitmean) > fitsigma)){
                    lowdaq = true;
                }
                //If fitmean > fitmax or fitmean < fitmin...flag
                if(fitmean > fitmax || fitmean < fitmin)
                    lowdaq = true;

                //If bins after fitmax averaged to the right are greater than the fitmean...flag
                double maxavg = 0;
                int fitmaxbin = projy_h->FindBin(fitmax);
                for (int i = 1; i < 6; i++){
                    maxavg = maxavg + projy_h->GetBinContent(fitmaxbin + i); 
                }
                maxavg = maxavg/5;
                if(maxavg > fitnorm)
                    lowdaq = true;
            }

            //Fill fit values
            flat_tuple_->setVariableValue("lowdaq", (double)int(lowdaq));
            flat_tuple_->setVariableValue("badfit", (double)int(badfit));
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
                        
        }
    }
}

