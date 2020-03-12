//Authors: F.Sforza, PF, Alignment Team

#include "HistogramHelpers.h"
#include "TMath.h" 

double HistogramHelpers::GaussExpTails_f(double* x, double *par) {
  //core Gaussian with exponential tails starting K
  const double norm    = par[0];
  const double fracT   = par[1];
  //const double fracTL   = par[1];
  //const double fracTH   = par[2];
  const double meanG    = par[2];
  const double sigmaG   = par[3];
  const double K        = par[4];
  //const double KL      = par[5];
  //const double KH      = par[6];
  

  const double GFact = TMath::Sqrt(2*TMath::Pi());
  const double GausExpNorm =  (GFact*sigmaG) *
    ( TMath::Erf( 0.70710678*K ) + exp( -K*K/2 ) * sqrt( 2/TMath::Pi() )/K);
  
  //const double GausExpNormL =  (GFact*sigmaG) *
  //( TMath::Erf( 0.70710678*KL ) + exp( -KL*KL/2 ) * sqrt( 2/TMath::Pi() )/KL);
  
  //const double GausExpNormH =  (GFact*sigmaG) *
  //( TMath::Erf( 0.70710678*KH ) + exp( -KH*KH/2 ) * sqrt( 2/TMath::Pi() )/KH);
  
  //return norm*fracT/GausExpNorm*exp( -K *fabs(x[0]-meanG)/sigmaG + K*K/2) +
  //norm*(1-fracT)/(GFact*sigmaG)*exp(-pow( (x[0]-meanG),2)/(2*pow(sigmaG,2)));
  
  //if(fabs(x[0]-meanG)> sigmaG*K) {
  return norm*fracT/GausExpNorm*exp( -K *fabs(x[0]-meanG)/sigmaG + K*K/2) +
    norm*(1-fracT)/(GFact*sigmaG)*exp(-pow( (x[0]-meanG),2)/(2*pow(sigmaG,2)));
    //} 
    // return norm/(GFact*sigmaG)*exp(-pow( (x[0]-meanG),2)/(2*pow(sigmaG,2)));

    
  
}

double HistogramHelpers::twoGaussExp_f(double *x, double *par){
  //2 core Gaussian with exponential tails starting  K
  const double norm = par[0];
  const double fracT = par[1];
  const double meanG = par[2];
  const double sigmaGT = par[3];
  const double sigmaGC = par[4]; 
  const double K = par[5];
  
  const double GFact = TMath::Sqrt(2*TMath::Pi());
  const double GausExpNorm =  (GFact*sigmaGT) *
    ( TMath::Erf( 0.70710678*K ) + exp( -K*K/2 ) * sqrt( 2/TMath::Pi() )/K);
  
  if(fabs(x[0]-meanG)> sigmaGT*K) {
    return norm*fracT/GausExpNorm*exp( -K *fabs(x[0]-meanG)/sigmaGT + K*K/2) +
      norm*(1-fracT)/(GFact*sigmaGC)*exp(-pow( (x[0]-meanG),2)/(2*pow(sigmaGC,2)));
  } 
  return norm*fracT/GausExpNorm*exp(-pow( (x[0]-meanG), 2)/(2*pow(sigmaGT,2))) +
    norm*(1-fracT)/(GFact*sigmaGC)*exp(-pow( (x[0]-meanG),2)/(2*pow(sigmaGC,2)));
}

double HistogramHelpers::twoGauss_f(double *x, double *par){
  //two Gaussians, Tail and Core, normalized to a total integral par[0] and of tail fraction par[1]
  const double norm = par[0];
  const double fracT = par[1];
  const double meanG = par[2];
  const double sigmaGT = par[3];
  const double sigmaGC = par[4];
  const double GFact = TMath::Sqrt(2*TMath::Pi());
  return
    norm*(1-fracT)/(GFact*sigmaGC)*exp(-pow( (x[0]-meanG),2)/(2*pow(sigmaGC,2))) +
    norm*fracT/(GFact*sigmaGT)*exp(-pow( (x[0]-meanG), 2)/(2*pow(sigmaGT,2)));
}

  
//-------------------------------------------------------------
void HistogramHelpers::profileZwithIterativeGaussFit(TH3* hist, TH2* mu_graph, TH2* sigma_graph, int num_bins, TH2* mu_err_graph, TH2* sigma_err_graph)
{
  if (!hist) {
    cout<< "ProfileZwithIterativeGaussFit(): No histogram supplied!"<<endl;
    return;
  }

  
  int num_bins_x = hist->GetXaxis()->GetNbins();
  int num_bins_y = hist->GetYaxis()->GetNbins();
        

  int minEntries = 50;
  int fDebug = 0;

  
  double num_not_converged = 0;
  int num_skipped = 0;

  double max_sigma = 0;
  double min_sigma = 0;

  double max_mu = 0;
  double min_mu = 0;

  TH1D* current_proj;
  if (fDebug || false) std::cout << " ** profileZwithIterativeGaussFit ** target: " << mu_graph->GetName() << " ** " << std::endl; 

  for (int i = 1; i < num_bins_x+(num_bins==1); i+=num_bins) {

    for (int j = 1; j < num_bins_y+(num_bins==1); j+=num_bins) {
      
      int index = i/num_bins;
      int index_y = j/num_bins;

      current_proj = hist->ProjectionZ(Form("%s_GaussProjection_%i_%i",hist->GetName(),index, index_y),i,i+num_bins-1,j,j+num_bins-1);
      current_proj->SetTitle(Form("%s - Bin %i x %i",hist->GetName(), index,index_y));

      double current_mu,current_err_mu, current_sigma, current_err_sigma;

      if(current_proj->GetEntries() < minEntries) {
	//if (m_PrintLevel >= 3) cout << " ** profileZwithIterativeGaussFit ** fitting " << hist->GetName() << "  bin (" << index << ", " << index_y << ") "
	//			    << " Not enough entries " << current_proj->GetEntries() << " < " << minEntries << endl;
	//current_mu = -999;
	current_mu = 0;
	current_sigma = 0;
	current_err_mu = 1;
	current_err_sigma = 1;
	
	if (fDebug) std::cout<<"WARNING: Only "<<current_proj->GetEntries()<<" entries in bin "<<index<<","<<index_y<< " in histogram " <<hist->GetName()<< std::endl;
	num_skipped++;

      } 
      else {
	//if (m_PrintLevel >= 3) cout << " ** profileZwithIterativeGaussFit ** fitting " << hist->GetName() 
	//			    << "  bin (" << index << ", " << index_y << ") "
	//			    << "  entries: "<< current_proj->GetEntries()
	//			    << endl;
	
	IterativeGaussFit(current_proj, current_mu, current_err_mu, current_sigma, current_err_sigma);
	
	if (current_sigma > max_sigma || max_sigma == 0) max_sigma = current_sigma;
	if (current_sigma < min_sigma || min_sigma == 0) min_sigma = current_sigma;
	if (current_mu > max_mu || max_mu == 0) max_mu = current_mu;
	if (current_mu < min_mu || min_mu == 0) min_mu = current_mu;
	
      }//end if entries < minEntries
      
      float x_coord = (hist->GetXaxis()->GetBinLowEdge(i) + hist->GetXaxis()->GetBinUpEdge(i+num_bins-1))/2;
      float y_coord = (hist->GetYaxis()->GetBinLowEdge(j) + hist->GetYaxis()->GetBinUpEdge(j+num_bins-1))/2;
      
      int binx = mu_graph->GetXaxis()->FindBin(x_coord);
      int biny = mu_graph->GetYaxis()->FindBin(y_coord);
      if (mu_graph) {
	mu_graph->Fill(x_coord,y_coord,current_mu);
	mu_graph->SetBinError(binx,biny, current_err_mu);
	if (fDebug || false) std::cout << " ** profileZwithIterativeGaussFit ** target: " << mu_graph->GetName()
				       << " bin(" << binx << ", " << biny << ") = " << current_mu
				       << " +- " << current_err_mu << std::endl;
      }
      //should probably be replace bin content, not fill?
      if (sigma_graph)         sigma_graph->SetBinContent(binx, biny, current_sigma);
      if (sigma_err_graph) sigma_err_graph->SetBinContent(binx, biny, current_err_sigma);
      if (mu_err_graph)       mu_err_graph->SetBinContent(binx, biny,current_err_mu);
      
      delete current_proj;
      
    } //end loop on j (y)
  } //end loop on i (x)
  
  if (mu_graph) {
    mu_graph->GetXaxis()->SetTitle(hist->GetXaxis()->GetTitle());
    mu_graph->GetYaxis()->SetTitle(hist->GetYaxis()->GetTitle());
    mu_graph->GetYaxis()->SetTitleOffset(1);
    mu_graph->GetZaxis()->SetTitle(hist->GetZaxis()->GetTitle());
    mu_graph->GetZaxis()->SetTitleOffset(1.2);
    mu_graph->SetTitle(hist->GetTitle() );
    //mu_graph->SetMaximum(max_mu + 0.1* (max_mu-min_mu));
    //mu_graph->SetMinimum(min_mu - 0.1* (max_mu-min_mu));
  }
  
  if (sigma_graph) {
    sigma_graph->GetXaxis()->SetTitle(hist->GetXaxis()->GetTitle());
    sigma_graph->GetYaxis()->SetTitle(hist->GetYaxis()->GetTitle());
    sigma_graph->GetYaxis()->SetTitleOffset(1);
    sigma_graph->GetZaxis()->SetTitle(hist->GetZaxis()->GetTitle());
    sigma_graph->GetZaxis()->SetTitleOffset(1.2);
    sigma_graph->SetTitle( hist->GetTitle() );
    //sigma_graph->SetMaximum(max_sigma + 0.1* (max_sigma-min_sigma));
    //sigma_graph->SetMinimum(min_sigma - 0.1* (max_sigma-min_sigma));
  }
  
  if (mu_err_graph) {
    mu_err_graph->GetXaxis()->SetTitle(hist->GetXaxis()->GetTitle());
    mu_err_graph->GetYaxis()->SetTitle(hist->GetYaxis()->GetTitle());
    mu_err_graph->GetYaxis()->SetTitleOffset(1);
    mu_err_graph->GetZaxis()->SetTitle(Form("Error of fit #mu: %s",hist->GetZaxis()->GetTitle()));
    mu_err_graph->GetZaxis()->SetTitleOffset(1.2);
    mu_err_graph->SetTitle(hist->GetTitle());
    //mu_err_graph->SetMaximum(max_mu + 0.1* (max_mu-min_mu));
    //mu_err_graph->SetMinimum(min_mu - 0.1* (max_mu-min_mu));
  }
  
  if (sigma_err_graph) {
    sigma_err_graph->GetXaxis()->SetTitle(hist->GetXaxis()->GetTitle());
    sigma_err_graph->GetYaxis()->SetTitle(hist->GetYaxis()->GetTitle());
    sigma_err_graph->GetYaxis()->SetTitleOffset(1);
    sigma_err_graph->GetZaxis()->SetTitle(Form("Error of fit #sigma: %s",hist->GetZaxis()->GetTitle()));
    sigma_err_graph->GetZaxis()->SetTitleOffset(1.2);
    sigma_err_graph->SetTitle(hist->GetTitle());
    //sigma_err_graph->SetMaximum(max_mu + 0.1* (max_mu-min_mu));
    //sigma_err_graph->SetMinimum(min_mu - 0.1* (max_mu-min_mu));
  }
  
  
  if (num_not_converged || num_skipped) std::cout<<"Fit Results for histogram: "<< hist->GetName()<<std::endl;
  if (num_not_converged) std::cout<<"Non Convergent Bin Fraction: "<<num_not_converged<< " / " <<num_bins_x*num_bins_y - num_skipped<<std::endl;
  if (num_skipped) std::cout<<"Number skipped bins: "<<num_skipped<< " / " <<num_bins_x*num_bins_y<<std::endl;
  
  return;
}





//-----------------------------------------------------------------------------
void HistogramHelpers::profileYwithIterativeGaussFit(TH2* hist, TH1* mu_graph, TH1* sigma_graph, int num_bins,int m_PrintLevel)
{
  
  if (!hist) {
    std::cout << "Error in ProfileYwithIterativeGaussFit(): Histogram not found" <<std::endl;
    return;
  }
  
  if (num_bins < 1 ) {
    std::cout << "Error in ProfileYwithIterativeGaussFit(): Invalid number of bins to integrate over." <<std::endl;
    return;
  }

  const int minEntries = 50;
  const int fDebug = 0;

  int num_bins_x = hist->GetXaxis()->GetNbins();

  if (mu_graph) mu_graph->Rebin(num_bins);
  if (sigma_graph) sigma_graph->Rebin(num_bins);

  double* errs_mu = new double[num_bins_x/num_bins + 2]; // +2 for overflow!!
  double* errs_sigma = new double[num_bins_x/num_bins + 2];

  errs_mu[0] = 0;
  errs_mu[num_bins_x/num_bins + 1] = 0;

  errs_sigma[0] = 0;
  errs_sigma[num_bins_x/num_bins + 1] = 0;

  double min_sigma = 0;
  double max_sigma = 0;
  double min_mu = 0;
  double max_mu = 0;

  int num_skipped = 0;

  TH1D* current_proj;

  for (int i = 1; i < (num_bins_x + (num_bins == 1)); i+=num_bins) {

    int index = i/num_bins;
    if (num_bins == 1) index--;

    current_proj = hist->ProjectionY(Form("%s_projection_%i",hist->GetName(),index),i,i+num_bins-1);

    double mu, mu_err, sigma, sigma_err;

    if(current_proj->GetEntries() < minEntries) {
      mu = 0;
      mu_err = 0;
      sigma = 0;
      sigma_err = 0;
      num_skipped++;
      if ( fDebug ) std::cout<<"WARNING: Not enough entries in bin "<<index<<std::endl;
    } else {

      IterativeGaussFit(current_proj, mu, mu_err, sigma, sigma_err,m_PrintLevel);

      if (sigma > max_sigma || max_sigma == 0) max_sigma = sigma;
      if (sigma < min_sigma || min_sigma == 0) min_sigma = sigma;
      if (mu > max_mu || max_mu == 0) max_mu = mu;
      if (mu < min_mu || min_mu == 0) min_mu = mu;

    }

    double value_x = (hist->GetXaxis()->GetBinLowEdge(i) + hist->GetXaxis()->GetBinUpEdge(i+num_bins-1))/2;

    //Important!! Use Fill to increment the graph with each iteration, or SetBinContent to replace contents...

    if (sigma_graph) sigma_graph->Fill(value_x, sigma);
    if (mu_graph) mu_graph->Fill(value_x, mu);
        
    errs_mu[index + 1] = mu_err;
    errs_sigma[index + 1] = sigma_err;
    
    /*
    std::cout<<hist->GetName()<<" "<<index+1<<"  mu = " <<mu<<"+/-"<<mu_err<<std::endl;
    std::cout<<hist->GetName()<<" "<<index+1<<"  sigma = " <<sigma<<"+/-"<<sigma_err<<std::endl;
    */

    delete current_proj;
  }

  if (sigma_graph) {
     sigma_graph->SetError(errs_sigma);
    //sigma_graph->SetMaximum(max_sigma+0.15*(max_sigma - min_sigma));
    //sigma_graph->SetMinimum(min_sigma-0.15*(max_sigma - min_sigma));
    sigma_graph->GetYaxis()->SetTitleOffset(1.5);
    //sigma_graph->GetYaxis()->SetTitle(hist->GetYaxis()->GetTitle());
    //sigma_graph->GetXaxis()->SetTitle(hist->GetXaxis()->GetTitle());
    sigma_graph->GetYaxis()->SetTitle(sigma_graph->GetYaxis()->GetTitle());
    sigma_graph->GetXaxis()->SetTitle(sigma_graph->GetXaxis()->GetTitle());
    sigma_graph->SetTitle("");
  }

  if (mu_graph) {
      mu_graph->SetError(errs_mu);
    //mu_graph->SetMaximum(max_mu+0.15*(max_mu - min_mu));
    //mu_graph->SetMinimum(min_mu-0.15*(max_mu - min_mu));
    mu_graph->GetYaxis()->SetTitleOffset(1.5);
    //    mu_graph->GetYaxis()->SetTitle(hist->GetYaxis()->GetTitle());
    // mu_graph->GetXaxis()->SetTitle(hist->GetXaxis()->GetTitle());
    mu_graph->GetYaxis()->SetTitle(mu_graph->GetYaxis()->GetTitle());
    mu_graph->GetXaxis()->SetTitle(mu_graph->GetXaxis()->GetTitle());
    mu_graph->SetTitle("");
  }

  if (fDebug && num_skipped)  std::cout<<" Number of skipped bins: "<<num_skipped<<std::endl;

  return;
  
}


//@par1 histogram to fit
//@par2 pointer to function (which needs to be defined as double fcn(double*,double*)
//@par3 initialParams
//@par4 fitRange
//@par5 FitResultPtr
//@return Pointer to the fit function
TF1* HistogramHelpers::IterativeGeneralFit(TH1* hist, double(*fcn)(double *, double *),const std::vector<float>& initial, const std::vector<float>& fitRange, TFitResultPtr *fitr_p, int m_PrintLevel) {
  
  const int iteration_limit = 20;
  const float percent_limit = 0.01;
  const int fDebug = 0;

  int minEntries = 50;
  
  if (!hist) {
    if (fDebug) std::cout<<"IterativeGeneralFit:: Histogram to be fit is missing. Return nullptr"<<std::endl;
    return nullptr;
  }

  if (hist->GetEntries() < minEntries) {
    if (fDebug) std::cout<<"IterativeGeneralFit::Histogram to be fit is too low-stat. Return nullptr"<<std::endl;
    return nullptr;
  }

  if (fitRange.size() < 2) {
    if (fDebug) std::cout<<"IterativeGeneralFit::Provide the fit range. Return nullptr"<<std::endl;
    return nullptr;
  }
    
  if (initial.size() < 1) {
    if (fDebug) std::cout<<"IterativeGeneralFit. At least a free parameter is necessary. Return nullptr"<<std::endl;
    return nullptr;
  }
  
  
  HistogramConditioning(hist);
  
  TF1* fit_func = new TF1("fit_func",fcn,fitRange[0],fitRange[1],initial.size());
  //Setting the initial parameters
  for (unsigned int i=0; i<initial.size();++i) { 
    fit_func->SetParameter(i,initial[i]);
  }
  return fit_func;
}


int HistogramHelpers::IterativeGaussFit(TH1* hist, double &mu, double &mu_err, double &sigma, double &sigma_err, int m_PrintLevel)
{
  
  //constants for fitting algorithm
  const int iteration_limit = 20;
  const float percent_limit = 0.01;
  const float fit_range_multiplier = 1.5;
  const int fDebug = 0;
  
  double last_mu;
  double last_sigma;
  double current_mu = 0;
  double current_sigma = 0;
  double mu_percent_diff;
  double sigma_percent_diff;
  
  int minEntries = 25;
  
  if (!hist) {
    if (fDebug) std::cout<< "Error in  Anp::IterativeGaussFit(): Histogram to be fit is missing" <<std::endl;
    return 1;
  }
  if (hist->GetEntries() < minEntries) {
    if (fDebug) std::cout<< "Error in  Anp::IterativeGaussFit(): Histogram has too few entries " << hist->GetEntries() << std::endl;
    return 1;
  }
  
  HistogramConditioning(hist);
  
  TF1* fit_func = new TF1("fit_func","gaus");
  
  int bad_fit = hist->Fit(fit_func,"QN");

  if (fDebug && bad_fit) std::cout <<"BAD INITIAL FIT: "<< hist->GetTitle() << std::endl;
  
  last_mu = fit_func->GetParameter(1);
  last_sigma = fit_func->GetParameter(2);
  
  if (bad_fit) last_mu = hist->GetMean();
  
  // check as well that the value of last_mu is reasonable
  if (fabs(last_mu - hist->GetMean()) > 3*hist->GetBinWidth(1)) {
    last_mu =  hist->GetMean();
    last_sigma = hist->GetRMS();
  }
  
  // intial fit range
  // fit_func->SetRange(last_mu-fit_range_multiplier*last_sigma,last_mu+fit_range_multiplier*last_sigma);
  // superseeded below
  
  int iteration = 0;
  while ( iteration < iteration_limit ) {
    
    iteration++;
    
    double FitRangeLower = last_mu-fit_range_multiplier*last_sigma;
    double FitRangeUpper = last_mu+fit_range_multiplier*last_sigma;
    
    // if range is to narrow --> broaden it
    if ((FitRangeUpper-FitRangeLower)/hist->GetBinWidth(1) < 5) {
      FitRangeLower -= hist->GetBinWidth(1);
      FitRangeUpper += hist->GetBinWidth(1);
    }
    
    fit_func->SetRange(FitRangeLower, FitRangeUpper);
    if (m_PrintLevel >= 3) cout << " ** IterativeGaussFit ** fit iter # " << iteration
				<< "   new fit range: " << FitRangeLower << " --> " << FitRangeUpper << endl;
    
    bad_fit = hist->Fit(fit_func, "RQN");
    // check that fit result is sound
    // 1) the mean must be within the histogram range
    if (fit_func->GetParameter(1) < hist->GetXaxis()->GetXmin() || fit_func->GetParameter(1) > hist->GetXaxis()->GetXmax()) bad_fit = 1;
    // 2) the sigma can not be broader than the histogram range
    if (fit_func->GetParameter(2) > hist->GetXaxis()->GetXmax()-hist->GetXaxis()->GetXmin()) bad_fit = 1;
    
    if (fDebug && bad_fit) std::cout<<" ** BAD FIT ** : bin "<< hist->GetTitle() <<"  iteration "<<iteration<<std::endl;
    
    if (bad_fit) { // in case the fit looks odd, then rebin the histogram and refit with smallest divisor.
      int rebinFactor = 1;
      for (int i_div = 2; i_div < (hist->GetXaxis()->GetNbins()/2)+1; ++i_div)
	if ((hist->GetXaxis()->GetNbins() % i_div) == 0) {
	  rebinFactor = i_div;
	  break;
	}
      //std::cout<<"Rebinning by factor:"<<rebinFactor<<std::endl;
      hist->Rebin(rebinFactor);
      hist->Fit(fit_func, "RQN");
    }
    
    // extract the correction for this iteration
    current_mu = fit_func->GetParameter(1);
    current_sigma = fit_func->GetParameter(2);
      
    float average_mu = (last_mu+current_mu)/2;
    float average_sigma = (last_sigma+current_sigma)/2;
    
    if (average_mu == 0) {
      if ( fDebug ) std::cout<<" Average mu = 0 in bin "<< hist->GetTitle() <<std::endl;
      average_mu = current_mu;
    }
    
    if (average_sigma == 0) {
      if ( fDebug ) std::cout<<"Average sigma = 0; Fit Problem in "<< hist->GetTitle() <<". "<<last_sigma<<" "<<current_sigma<<std::endl;
      average_sigma = current_sigma;
    }
    
    mu_percent_diff = fabs((last_mu-current_mu)/average_mu);
    sigma_percent_diff = fabs((last_sigma-current_sigma)/average_sigma);
    
    if ( mu_percent_diff < percent_limit && sigma_percent_diff < percent_limit ) break;
    
    if (iteration != iteration_limit) { //necessary?
      last_mu = current_mu;
      last_sigma = current_sigma;
    }
    // check as well that the last_mu is reasonable
    if (fabs(last_mu - hist->GetMean()) > 5*hist->GetBinWidth(1)) {
      if (m_PrintLevel >= 3) cout << " ** IterativeGaussFit ** fit iter # " << iteration
				  << " ** WARNING ** last_mu looks bad: " << last_mu
				  << "    this iter mu: " << fit_func->GetParameter(1)
				  << "    proposed mu: " << hist->GetMean()
				  << endl;
      last_mu =  hist->GetMean();
    }
  }
  
  if (iteration == iteration_limit) {
    if (fDebug ) std::cout<<"WARNING: Fit did not converge to < "<<percent_limit*100<<"% in "<< hist->GetTitle() <<" in "<<iteration_limit<<" iterations. Percent Diffs: "<<mu_percent_diff*100<<"% (Mu),"<<" "<<sigma_percent_diff*100<<"% (Sigma)"<<std::endl;
    //possibly return a value other than 0 to indicate not converged?
  }
  
  mu = current_mu;
  mu_err = fit_func->GetParError(1);
  sigma = current_sigma;
  sigma_err = fit_func->GetParError(2);
  
  hist->GetListOfFunctions()->Add(fit_func);
  
  if (m_PrintLevel >= 1 ) {
    cout << " ** IterativeGaussFit ** fit result: histo name " << hist->GetName() << "    title: " << hist->GetTitle()  << endl
	 << "    mu = " << mu << " +- " << mu_err << endl
	 << " sigma = " << sigma << " +- " << sigma_err
	 << endl;
    //if (TempCanvasIterGaussFit == NULL) {
    //TempCanvasIterGaussFit = new TCanvas ("TempCanvasIterGaussFit","Iter Gauss fit", 400, 400);
    //}
    //hist->DrawCopy();
    //TempCanvasIterGaussFit->Update();
    //hist->Print();
    string input = "";
    cout << " ** IterativeGaussFit ** Please type RETURN to continue:\n>";
    getline(cin, input);
  }
  
  
  if (outFile_for_projections->IsOpen()) {
      TCanvas q; 
      q.cd();
      hist->Draw();
      fit_func->Draw("same");
      std::string str = "";
      //q.SaveAs((str+"_"+hist->GetName()+".pdf").c_str());
      outFile_for_projections->cd();
      q.Write((str+"_"+hist->GetName()).c_str()); 
  }
      
  return 0;
}

void HistogramHelpers::OpenProjectionFile() {
    outFile_for_projections = new TFile("debug_projections_histogramHelpers.root","RECREATE");
}

void HistogramHelpers::CloseProjectionFile() {
    
    if (outFile_for_projections->IsOpen())  
        outFile_for_projections->Close();
}


void HistogramHelpers::HistogramConditioning (TH1* hist,int m_PrintLevel)
{
  if (m_PrintLevel>=3) cout << " ** HistogramConditioning ** START ** hist = " << hist->GetName() << endl;

  double MinEntriesMPB = 15;
  Int_t NGroupBins = 2;

  // goal:
  // make sure that the most populated bin has a minimum number of entries
  Int_t  MostPopulatedBin = (hist->GetMaximumBin());
  double EntriesMPB = hist->GetBinContent(MostPopulatedBin);
  if (EntriesMPB < MinEntriesMPB) {
    // check the entries of the neighbour channels
    if ((EntriesMPB + hist->GetBinContent(MostPopulatedBin+1) + hist->GetBinContent(MostPopulatedBin-1)) > MinEntriesMPB) {
      NGroupBins = 2;
    }
    else {
      NGroupBins = 3;
    }

    // now find the first divisor (factor of ) the original number of bins
    Bool_t DivisorFound = false;
    while (!DivisorFound) {
      if ( hist->GetNbinsX() % NGroupBins == 0) {
	DivisorFound = true;
      }
      else {
	DivisorFound = false;
	NGroupBins++;
      }
    }
    Int_t NBinsWas = hist->GetNbinsX();
    hist = hist->Rebin(NGroupBins);
    if (m_PrintLevel>=1) cout << " ** HistogramConditioning ** histogram had to be rebinned by: " << NGroupBins
			      << " NBins was: " << NBinsWas << " and now is: " << hist->GetNbinsX() << endl;

  }


  return;
}

