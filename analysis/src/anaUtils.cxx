#include "anaUtils.h"
#include <utility>

Double_t utils::langaufun(Double_t *x, Double_t *par) {

  //Fit parameters:
  //par[0]=Width (scale) parameter of Landau density
  //par[1]=Most Probable (MP, location) parameter of Landau density
  //par[2]=Total area (integral -inf to inf, normalization constant)
  //par[3]=Width (sigma) of convoluted Gaussian function
  //
  //In the Landau distribution (represented by the CERNLIB approximation),
  //the maximum is located at x=-0.22278298 with the location parameter=0.
  //This shift is corrected within this function, so that the actual
  //maximum is identical to the MP parameter.

  // Numeric constants
  Double_t invsq2pi = 0.3989422804014;   // (2 pi)^(-1/2)
  Double_t mpshift  = -0.22278298;       // Landau maximum location

  // Control constants
  Double_t np = 100.0;      // number of convolution steps
  Double_t sc =   5.0;      // convolution extends to +-sc Gaussian sigmas

  // Variables
  Double_t xx;
  Double_t mpc;
  Double_t fland;
  Double_t sum = 0.0;
  Double_t xlow,xupp;
  Double_t step;
  Double_t i;


  // MP shift correction
  mpc = par[1] - mpshift * par[0];

  // Range of convolution integral
  xlow = x[0] - sc * par[3];
  xupp = x[0] + sc * par[3];

  step = (xupp-xlow) / np;
  // Convolution integral of Landau and Gaussian by sum
  for(i=1.0; i<=np/2; i++) {
    xx = xlow + (i-.5) * step;
    fland = TMath::Landau(xx,mpc,par[0]) / par[0];
    sum += fland * TMath::Gaus(x[0],xx,par[3]);

    xx = xupp - (i-.5) * step;
    fland = TMath::Landau(xx,mpc,par[0]) / par[0];
    sum += fland * TMath::Gaus(x[0],xx,par[3]);
  }

  return (par[2] * step * sum * invsq2pi / par[3]);
}


TF1* utils::langaufit(TH1F *his, Double_t *fitrange, Double_t *startvalues, Double_t *parlimitslo, Double_t *parlimitshi, Double_t *fitparams, Double_t *fiterrors, Double_t *ChiSqr, Int_t *NDF)
{
  // Once again, here are the Landau * Gaussian parameters:
  //   par[0]=Width (scale) parameter of Landau density
  //   par[1]=Most Probable (MP, location) parameter of Landau density
  //   par[2]=Total area (integral -inf to inf, normalization constant)
  //   par[3]=Width (sigma) of convoluted Gaussian function
  //
  // Variables for langaufit call:
  //   his             histogram to fit
  //   fitrange[2]     lo and hi boundaries of fit range
  //   startvalues[4]  reasonable start values for the fit
  //   parlimitslo[4]  lower parameter limits
  //   parlimitshi[4]  upper parameter limits
  //   fitparams[4]    returns the final fit parameters
  //   fiterrors[4]    returns the final fit errors
  //   ChiSqr          returns the chi square
  //   NDF             returns ndf

  Int_t i;
  Char_t FunName[100];

  sprintf(FunName,"Fitfcn_%s",his->GetName());

  TF1 *ffitold = (TF1*)gROOT->GetListOfFunctions()->FindObject(FunName);
  if (ffitold) delete ffitold;

  TF1 *ffit = new TF1(FunName,langaufun,fitrange[0],fitrange[1],4);
  ffit->SetParameters(startvalues);
  ffit->SetParNames("Width","MP","Area","GSigma");

  for (i=0; i<4; i++) {
    ffit->SetParLimits(i, parlimitslo[i], parlimitshi[i]);
  }

  his->Fit(FunName,"RB0Q");   // fit within specified range, use ParLimits, do not plot, quiet

  ffit->GetParameters(fitparams);    // obtain fit parameters
  for (i=0; i<4; i++) {
    fiterrors[i] = ffit->GetParError(i);     // obtain fit parameter errors
  }
  ChiSqr[0] = ffit->GetChisquare();  // obtain chi^2
  NDF[0] = ffit->GetNDF();           // obtain ndf

  return (ffit);              // return fit function

}

void utils::GetMean(TH1* hist, std::pair<float,float> &info, int method,TFile* outfile,float* fitRange) {

  std::string histname = hist->GetName();
  if(method==0) {
    hist->SetAxisRange(0,hist->GetBinCenter(hist->GetMaximumBin()), "X");
    info.first = hist->GetMean();
    info.second = hist->GetMeanError();
    return;
  }


  if(method==1) {
    double peak = hist->GetBinCenter(hist->GetMaximumBin());
    hist->SetAxisRange(0.50*peak,1.2*peak, "X");
    //    cout<< peak<<" "<<hist->GetMean()<<endl;
    info.first  = hist->GetMean();
    info.second = hist->GetMeanError();
    return;
  }

  if (method ==3) {
    if (hist->GetEntries() > 25) {
      info.first  = hist->GetBinCenter(hist->GetMaximumBin());
      info.second = hist->GetXaxis()->GetBinWidth(1) / sqrt(hist->GetEntries());
    }
    else {
      info.first = 0.;
      info.second = 0.;
    }
  }
  if(method==2) {

    // Fitting landau convoluted with gaussian
    //printf("Fitting...\n");

    // Setting fit range and start values
    double peak = hist->GetBinCenter(hist->GetMaximumBin());

    //Range of the histogram 0-10000
    double range     = 10000;
    double StartArea = range * hist->GetBinContent(hist->GetMaximumBin()) / 10.;


    //Double_t fr[2] = {300, 8000}; //fit range
    Double_t fr[2] = {fitRange[0], fitRange[1]}; //fit range
    //std::cout<<"Fit Range "<<fr[0]<<" "<<fr[1]<<std::endl;
    Double_t sv[4] = {250, peak, StartArea, 50}; //start values
    //std::cout<<"Start Values "<<sv[0]<<" "<<sv[1]<<" "<<sv[2]<<" "<<sv[3]<<std::endl;
    Double_t pllo[4] = {0,    0.5*peak, StartArea/5, 0}; //low  limits
    Double_t plhi[4] = {1000, 1.5*peak, StartArea*5, 1000}; //high limits
    Double_t fp[4], fpe[4];
    Double_t chisqr;
    Int_t    ndf;

    TF1* fit_langau = langaufit((TH1F*)hist,fr,sv,pllo,plhi,fp,fpe,&chisqr,&ndf);
    //cout<<peak<<" "<< hist->GetEntries()<<" "<<hist->GetEntries()/20<<endl;
    //cout<<chisqr<<" / "<<ndf<<" = "<<chisqr/ndf<<endl;
    if(chisqr/ndf < 10) {
      info.first  = fp[1];
      info.second = fpe[1];
    }


    bool storePlots = false;
    std::string dummy="";
 
    if (storePlots) {
      //std::cout<<"Saving.."<<std::endl;
      TCanvas* can = new TCanvas();
      can->cd();
      hist->Draw();
      fit_langau->Draw("same");
      outfile->cd();
      can->SetLogy();
      TLatex text;
      text.SetNDC();
      text.SetTextFont(42);
      text.SetTextSize(0.03);
      text.SetTextColor(kBlack);
      text.DrawLatexNDC(0.5,0.72,(dummy+"Chi2/ndf " + std::to_string(chisqr)+"/"+std::to_string(ndf)).c_str());
      text.DrawLatexNDC(0.5,0.,(dummy+"FitPars " + std::to_string(fp[0])+","+std::to_string(fp[1])).c_str());
      can->Write(histname.c_str());
      delete can;
    }

  }

  //return info;

}



void utils::GetMean(TH1F* hist, std::pair<float,float> &info, std::pair<float,float> &info_width, int method,TFile* outfile,std::string runNumber,std::string name){

  //name here is the runNumber in string format
  int runN = (int)::atof(runNumber.c_str());
  std::string histname = hist->GetName();


  if(!((TString)hist->GetName()).Contains("dEdx") ) {
    info.first  = (hist->GetMean());
    info.second = (hist->GetMeanError());
    return;
  }

  if(method==0) {
    std::cout<<"hist getMean method0"<<hist<<std::endl;
    hist->SetAxisRange(0,1.8, "X");
    info.first  = (hist->GetMean());
    info.second = (hist->GetMeanError());
    return;
  }


  if(method==1) {

    double peak = hist->GetBinCenter(hist->GetMaximumBin());
    hist->SetAxisRange(0,peak+0.7, "X");
    //    cout<< peak<<" "<<hist->GetMean()<<endl;
    info.first  = (hist->GetMean());
    info.second = (hist->GetMeanError());
    return;
  }if(method==2) {

    // Fitting landau convoluted with gaussian
    printf("Fitting...\n");

    // Setting fit range and start values
    double peak = hist->GetBinCenter(hist->GetMaximumBin());
    Double_t fr[2] = {peak-0.15, 5}; //fit range

    //For 2017 and B-Layer we need to cut, alternatively we can fit larger range
    if (runN > 341649 || name.find("Blayer")==std::string::npos) {
      fr[0] = 0.5;
    }

    Double_t sv[4] = {0.08, peak, -hist->GetEntries()/20, 0.15}; //start values
    Double_t pllo[4] = {0.05, peak/2, 0, 0.02}; //low  limits
    Double_t plhi[4] = {0.11, 1.2*peak, hist->GetEntries()/5, 0.3}; //high limits
    Double_t fp[4], fpe[4];
    Double_t chisqr;
    Int_t    ndf;
    TF1* fit_langau = langaufit(hist,fr,sv,pllo,plhi,fp,fpe,&chisqr,&ndf);

    bool storePlots = true;
    std::string dummy="";

    if (storePlots) {
      std::cout<<"Saving.."<<std::endl;
      TCanvas* can = new TCanvas();
      can->cd();
      hist->Draw();
      fit_langau->Draw("same");
      outfile->cd();
      can->SetLogy();
      TLatex text;
      text.SetNDC();
      text.SetTextFont(42);
      text.SetTextSize(0.03);
      text.SetTextColor(kBlack);
      //text.DrawLatexNDC(0.5,0.87,(dummy+"fitRange ["+std::to_string(fr[0])+","+std::to_string(fr[1])+"]").c_str());
      text.DrawLatexNDC(0.5,0.72,(dummy+"Chi2/ndf " + std::to_string(chisqr)+"/"+std::to_string(ndf)).c_str());
      text.DrawLatexNDC(0.5,0.,(dummy+"FitPars " + std::to_string(fp[0])+","+std::to_string(fp[1])).c_str());

      can->SaveAs(histname.c_str());
      delete can;
    }


    cout<<peak<<" "<< hist->GetEntries()<<" "<<hist->GetEntries()/20<<endl;
    cout<<chisqr<<" / "<<ndf<<" = "<<chisqr/ndf<<endl;
    if(chisqr/ndf < 10) {
      info.first  = fp[1];
      info.second = fpe[1];
      info_width.first = fp[0];
      info_width.second= fpe[0];
    }
  }
}

string utils::getSubString(const string& s, const string& start_delim,const string& stop_delim) {
  unsigned first_delim_pos = s.find(start_delim);
  unsigned end_pos_of_first_delim = first_delim_pos + start_delim.length();
  unsigned last_delim_pos  = s.find_first_of(stop_delim,end_pos_of_first_delim);

  return s.substr(end_pos_of_first_delim,
        last_delim_pos - end_pos_of_first_delim);

}

TH1F* utils::getHisto(const string &histoname,const  string &filename) {


  TFile f(filename.c_str());
  //std::cout<<"Getting "<<histoname.c_str()<<std::endl;
  TH1F* histo = (TH1F*) f.Get(histoname.c_str());
  histo->SetDirectory(0);
  f.Close();
  return histo;
}


std::pair<TGraphErrors*,TGraphErrors*> utils::FillPlot(TH2* histo2d, const std::string y_title,int method,TFile* outfile, int binning, float* fitRange) {

  int point = 0;
  std::pair <TGraphErrors*,TGraphErrors*> graphs;
  TGraphErrors* graph   = new TGraphErrors();
  TGraphErrors* graph_w = new TGraphErrors();
  std::pair<float,float> mean;
  mean.first  = 0.;
  mean.second = 0.;
  std::pair<float,float> width;
  width.first  = 0.;
  width.second = 0.;

  //General graphics settings
  graph->SetLineWidth(2);
  graph->SetMarkerColor(kBlack);
  graph->SetLineColor(kBlack);
  graph->GetXaxis()->SetLabelFont(42);
  graph->GetXaxis()->SetTitleSize(0.035);
  graph->GetXaxis()->SetTitleFont(42);
  graph->GetXaxis()->SetTitleOffset(1.2);
  graph->GetYaxis()->SetTitle(y_title.c_str());
  graph->GetYaxis()->SetLabelFont(42);
  graph->GetYaxis()->SetTitleSize(0.035);
  graph->GetYaxis()->SetTitleOffset(1.8);
  graph->GetYaxis()->SetTitleFont(42);
  graph->SetTitle(0);


  graph_w->SetLineWidth(2);
  graph_w->SetMarkerColor(kBlack);
  graph_w->SetLineColor(kBlack);
  graph_w->GetXaxis()->SetLabelFont(42);
  graph_w->GetXaxis()->SetTitleSize(0.035);
  graph_w->GetXaxis()->SetTitleFont(42);
  graph_w->GetXaxis()->SetTitleOffset(1.2);
  graph_w->GetYaxis()->SetTitle(y_title.c_str());
  graph_w->GetYaxis()->SetLabelFont(42);
  graph_w->GetYaxis()->SetTitleSize(0.035);
  graph_w->GetYaxis()->SetTitleOffset(1.8);
  graph_w->GetYaxis()->SetTitleFont(42);
  graph_w->SetTitle(0);
  std::string histotitle = histo2d->GetName();
  std::string name="";
  if (binning > 1 && (histo2d->GetNbinsX() % binning == 0))
    histo2d->RebinX(binning);
  for (int ibinx = 1; ibinx < histo2d->GetNbinsX()+1;++ibinx) {

    name = histotitle+"_proj_"+std::to_string(ibinx);
    TH1D* proj_y = histo2d->ProjectionY(name.c_str(),ibinx,ibinx);

    if (proj_y -> GetEntries() > 100) {
      GetMean(proj_y,mean,method,outfile,fitRange);

      graph->SetPoint(point,ibinx,mean.first);
      graph->SetPointError(point,0,mean.second);
      if (method==2){
    graph_w->SetPoint(point,ibinx,width.first);
    graph_w->SetPointError(point,0,width.second);
      }
      point++;
    }

    delete proj_y; proj_y = nullptr;
  }//loop over the bins
  graphs.first = graph;
  graphs.second = graph_w;
  return graphs;
}



std::pair<TGraphErrors*,TGraphErrors*> utils::FillPlot(const string & plotname, const string & layer, const vector<string> &sample, int colour, const string&  y_title, bool runNo,int method, TFile* outFile) {

  int point = 0;
  TGraphErrors * graph = new TGraphErrors();
  TGraphErrors * graph_w = new TGraphErrors();

  //General graphics settings
  graph->SetLineWidth(2);
  graph->SetMarkerColor(colour);
  graph->SetLineColor(colour);
  graph->GetXaxis()->SetLabelFont(42);
  graph->GetXaxis()->SetTitleSize(0.035);
  graph->GetXaxis()->SetTitleFont(42);
  graph->GetXaxis()->SetTitleOffset(1.2);
  graph->GetYaxis()->SetTitle(y_title.c_str());
  graph->GetYaxis()->SetLabelFont(42);
  graph->GetYaxis()->SetTitleSize(0.035);
  graph->GetYaxis()->SetTitleOffset(1.8);
  graph->GetYaxis()->SetTitleFont(42);
  graph->SetTitle(0);


  graph_w->SetLineWidth(2);
  graph_w->SetMarkerColor(colour);
  graph_w->SetLineColor(colour);
  graph_w->GetXaxis()->SetLabelFont(42);
  graph_w->GetXaxis()->SetTitleSize(0.035);
  graph_w->GetXaxis()->SetTitleFont(42);
  graph_w->GetXaxis()->SetTitleOffset(1.2);
  graph_w->GetYaxis()->SetTitle(y_title.c_str());
  graph_w->GetYaxis()->SetLabelFont(42);
  graph_w->GetYaxis()->SetTitleSize(0.035);
  graph_w->GetYaxis()->SetTitleOffset(1.8);
  graph_w->GetYaxis()->SetTitleFont(42);
  graph_w->SetTitle(0);

  for ( auto s : sample) {
    TH1F* tmp = getHisto(plotname,s);
    std::pair<float,float> mean_and_err;
    mean_and_err.first = 0.;
    mean_and_err.second = 0.;
    std::pair<float,float> width_and_err;
    width_and_err.first = 0.;
    width_and_err.second = 0.;


    //Convert string to float
    float runNumber;
    stringstream ss;
    ss << getSubString(s,".00",".");
    ss >> runNumber;

    if (tmp->GetEntries() == 0) {
      //std::cout<<"Skipping "<<tmp->GetName()<<" bacause of 0 entries for run "<<runNumber<<std::endl;
      continue;
    }

    GetMean(tmp,mean_and_err,width_and_err,method,outFile,ss.str(),layer);

    if (mean_and_err.first == 0) {
      std::cout<<"FillPlot::WARNING::Mean value = 0 for run number "<<runNumber<<std::endl;
      std::cout<<"Plot saved in order to check"<<std::endl;
      TCanvas c;
      c.cd();
      tmp->Draw();
      std::string c_name = "c_";
      c_name+=tmp->GetName();
      c_name+=("_"+std::to_string(runNumber));
      c.Print((c_name+".png").c_str());
    }

    if (mean_and_err.first!=0) {
      if (runNo) {
    graph->SetPoint(point,runNumber,mean_and_err.first);
    graph->SetPointError(point,0,mean_and_err.second);
    if (method ==2) {
      graph_w->SetPoint(point,runNumber,width_and_err.first);
      graph_w->SetPointError(point,0,width_and_err.second);
    }
      }
    }

      //Map lumi to runNumber not done yet.
      //Will use goodRunList or lumiCal output directly.

      //std::cout<<mean_and_err.first<<" "<<mean_and_err.second
      //<<" "<<runNumber<<std::endl;

      point++;
      delete tmp;
    }
  
  std::pair<TGraphErrors*,TGraphErrors*> graphs;
  graphs.first  = graph;
  graphs.second = graph_w;
  return graphs;
  }


void utils::Get2DHistosFromFile(std::map<std::string, TH2F*>& histos2d,std::vector<std::string>& histos2dk,TFile* inFile, std::string folder, std::string timesample) {
    TIter next(inFile->GetListOfKeys());
    TKey *key;
    typedef std::map<std::string, TH2F*>::iterator it2d;
    int i=0;
    while ((key = (TKey*)next())) {
        std::string classType = key->GetClassName();
        std::string s(key->GetName());
        if (classType.find("TH2")!=std::string::npos)
            if (s.find(Form("baseline0_%s", timesample.c_str())) != std::string::npos) {
               // if (i<1){ //!!!This is just to limit the number of histos processed, for dev purposes!!!
                    histos2dk.push_back(s);
                    histos2d[key->GetName()] = (TH2F*) key->ReadObj();
                    i++;
                
            }
    }
    
}

void utils::getFitParams(TFile* ParametersFile,std::map<std::string, double>& means, std::string sensor) {
     ParametersFile->cd();
    // TH1D* mean_h = (TH1D*)inFile->Get("mean_raw_hits_L0B_axial_timesample_0_hh");
     TH1D* mean_h = (TH1D*)ParametersFile->Get(Form("mean_%s", sensor.c_str()));
     for (int cc=0; cc<mean_h->GetNbinsX();cc++)
         means[std::to_string(cc)]=mean_h->GetBinContent(cc);
}

void utils::fitCheck(std::map<std::string, TH2F*> histos2d, std::string histo_key, std::map<std::string, TH1D*>& singleChannel_h, std::map<std::string, double> means) {
      int FitRangeLower=0;
      int FitRangeUpper=20000;
      for (int cc=2;cc<3;cc++){

        std::cout << means[std::to_string(cc)] << std::endl;
        TCanvas* c = new TCanvas();
        c->cd();
        TF1* fit_func = new TF1("fit_func", "gaus");
        fit_func->SetRange(FitRangeLower, FitRangeUpper);
        fit_func->SetParameter(0,500);
        fit_func->SetParameter(1,means[std::to_string(cc)]);
        fit_func->SetParameter(2,100);
        
        fit_func->Draw();

        singleChannel_h[std::to_string(cc)]= histos2d[histo_key]->ProjectionY(Form("%s_ch%i_h_check", histo_key.c_str(), cc), cc+1, cc+1, "e");
        singleChannel_h[std::to_string(cc)]->Draw("SAME");
        c->Write();
    }
}






