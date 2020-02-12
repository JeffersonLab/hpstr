#include "SvtBlFitHistoProcessor.h"
#include <string>
#include <algorithm>
SvtBlFitHistoProcessor::SvtBlFitHistoProcessor(const std::string& name, Process& process)
    : Processor(name, process) {
    }

SvtBlFitHistoProcessor::~SvtBlFitHistoProcessor() {
}

void SvtBlFitHistoProcessor::configure(const ParameterSet& parameters) {

    std::cout << "[SvtBlFitHistoProcessor] Configuring" << std::endl;
    try
    {
        outDir_ = parameters.getString("outDir");
        histCfgFilename_ = parameters.getString("histCfg");
        hybrid_ = parameters.getVString("hybrid");
        IterativeGaussFitCut_ = parameters.getInteger("nhitsFitCut");
        rebin_ = parameters.getInteger("rebin");
        nPointsDer_ = parameters.getInteger("nPoints");
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

}

void SvtBlFitHistoProcessor::initialize(std::string inFilename, std::string outFilename) {

    std::cout << "initializing SvtBlFitHistoProcessor" << std::endl;
    //InFile containing 2D histograms from the SvtCondAnaProcessor
    inF_ = new TFile(inFilename.c_str());
    outF_ = new TFile(outFilename.c_str(),"RECREATE");

    outputHistos_ = new BlFitHistos("raw_hits");
    inputHistos_ = new HistoManager("");
    std::cout << "[BlFitHistos] Loading 2D Histos" << std::endl;
    inputHistos_->GetHistosFromFile(inF_, hybrid_);
    std::cout << "[BlFitHistos] Loading json file" << std::endl;
    outputHistos_->loadHistoConfig(histCfgFilename_); 
    std::cout << "[BlFitHistos] Creating Histograms for Fit Parameters" << std::endl;
    outputHistos_->DefineHistos();

}


bool SvtBlFitHistoProcessor::process() { 

    //Given a list of names for the input 2d histograms, and a list of names for the 
    //output 1d fit parameter histograms, the 2d and 1d hisgorams are connected
    //based upon having matching substrings, starting from the last instance of "L" 
    //which SHOULD represent the start of the hybrid name. If the Hybrid names match
    //between the 2d and 1d histograms, they can be used in profileYwithIterativeGaussFit

    std::cout << "[SvtBlFitHistoProcessor] process running" << std::endl;
    for (vector<string>::iterator jj = inputHistos_->histos2dNamesfromTFile.begin();
            jj != inputHistos_->histos2dNamesfromTFile.end(); ++jj)
    {
        
        TH2F* histo_hh = inputHistos_->get2dHisto(*jj);
        std::string sensorname = histo_hh->GetName();
        int  nbins = histo_hh->GetXaxis()->GetNbins();
        TH1D* fit_histo_h = new  TH1D(Form("Max_2nd_Derivative_%s",histo_hh->GetName()),"Max 2nd Derivative of chi2 per channel",10000,0,10000);
        //TH1D* Max2D_total_h = new  TH1D(Form("Max_2nd_Derivative_%s",histo_hh->GetName()),"Max 2nd Derivative of chi2 per channel",50,0,50);

        histo_hh->RebinY(rebin_);
        for(int cc=0; cc < 640; ++cc) {

            std::cout << "Channel #" << cc << std::endl;
            TH1D* projy_h = histo_hh->ProjectionY(Form("%s_projection_%i",histo_hh->GetName(),cc),
                    cc+1,cc+1,"e");
            projy_h->SetTitle(Form("ProjectionY_%s_Channel%i",histo_hh->GetName(),cc));
            int iter=0;
            int firstbin=projy_h->FindFirstBinAbove(10,1);
            double xmin=projy_h->GetBinLowEdge(firstbin);
            double binwidth=1*projy_h->GetBinWidth(firstbin);
            double xmax=xmin+20*binwidth;
            std::vector<double> amp,mean,sigma,chi2,const_err,sigma_err,
                   fit_range_end,chi2_NDF,chi2_2D;
            std::vector<int> NDF;

            while(iter < projy_h->GetNbinsX()-firstbin && xmax < 10000){
            
                TF1* cc_fit = new TF1("cc_fit", "gaus", xmin, xmax);
                projy_h->Fit(cc_fit, "QRES");

                if(cc_fit->GetNDF() == 0){
                    xmax = xmax + 2*binwidth;
                    //iter = iter + 1;
                    continue;
                }
                                    
                amp.push_back(cc_fit->GetParameter(0));
                mean.push_back(cc_fit->GetParameter(1));
                sigma.push_back(cc_fit->GetParameter(2));
                chi2.push_back(cc_fit->GetChisquare());
                //std::cout << "get chi2:" << cc_fit->GetChisquare() << std::endl;
                fit_range_end.push_back(xmax);
                NDF.push_back(cc_fit->GetNDF());
                //std::cout << "get NDF:" << cc_fit->GetNDF() << std::endl;
                chi2_NDF.push_back(chi2.at(iter)/NDF.at(iter));

                xmax = xmax + 2*binwidth;
                iter = iter + 1;
                delete cc_fit;
            } 

            for(int i=0; i < chi2_NDF.size(); ++i) {
                if( i > nPointsDer_ && i < (chi2_NDF.size() - nPointsDer_)){
                chi2_2D.push_back((chi2_NDF.at(i+nPointsDer_)-chi2_NDF.at(i))/(nPointsDer_*binwidth) - (chi2_NDF.at(i)-chi2_NDF.at(iter-nPointsDer_))/(nPointsDer_*binwidth));
                }
            }

             //Remove any 'nan' entries from chi2_2D
             for( int i=0; i < chi2_2D.size(); ++i) {
                if(chi2_2D[i] != chi2_2D[i]) { chi2_2D[i]=0.0;}
            }

            double max_Element = *std::max_element(chi2_2D.begin(), chi2_2D.end());
            int maxElementIndex = std::max_element(chi2_2D.begin(), chi2_2D.end()) - chi2_2D.begin();
            //std::cout << "maxElementIndex" << maxElementIndex << std::endl;
            
            //Re-run fit with xmax = location where 2nd derivative of chi2/NDF is maximum
            xmax = fit_range_end.at(maxElementIndex);
            TF1* cc_fit = new TF1("cc_fit", "gaus", xmin, xmax);
            projy_h->Fit(cc_fit, "QRES");



            //Graphs and Histograms
            
            //fit_histo_h->SetBinContent(cc,max_Element);
            fit_histo_h->Fill(fit_range_end.at(maxElementIndex+nPointsDer_));
            int n=fit_range_end.size();
            //std::cout << "size of fit range end: "<< n << std::endl;
            //std::cout << "size of chi2_NDF: " << chi2_NDF.size() << std::endl;
            TGraph* chi2_NDF_gr = new TGraph(n,fit_range_end.data(),chi2_NDF.data());
            chi2_NDF_gr->SetName(Form("Chi2_NDF_gr_%s_Channel%i",histo_hh->GetName(),cc));
            chi2_NDF_gr->SetTitle(Form("Chi2_vs_FitRanageEnd_%s_Channel%i",histo_hh->GetName(),cc));

            TGraph* mean_gr = new TGraph(n,fit_range_end.data(),mean.data());
            mean_gr->SetName(Form("Mean_gr_%s_Channel%i",histo_hh->GetName(),cc));
            mean_gr->SetTitle(Form("Mean_vs_FitRanageEnd_%s_Channel%i",histo_hh->GetName(),cc));

            std::vector<double>::const_iterator first = fit_range_end.begin()+nPointsDer_;
            std::vector<double>::const_iterator last=fit_range_end.begin()+nPointsDer_+chi2_2D.size();
            std::vector<double> chi2_2D_range(first,last);

            //std::cout << "length of chi2_2D " << chi2_2D.size() << std::endl;
            //std::cout << "length of chi2_2D_range" << chi2_2D_range.size() << std::endl;
            //std::cout << "length of chi2_NDF" << chi2_NDF.size() << std::endl;

            TGraph* chi2_2D_gr = new TGraph(chi2_2D.size(),chi2_2D_range.data(),chi2_2D.data());
            chi2_2D_gr->SetName(Form("chi2_2D_gr_%s_Channel%i",histo_hh->GetName(),cc));
            chi2_2D_gr->SetTitle(Form("chi2_2D_vs_FitRanageEnd_%s_Channel%i",histo_hh->GetName(),cc));
            
            chi2_NDF_gr->Write();
            mean_gr->Write();
            projy_h->Write();
            chi2_2D_gr->Write();

            delete chi2_NDF_gr;
            delete mean_gr;
            delete projy_h;
            delete chi2_2D_gr;

            }

        fit_histo_h->Write();

        }
        
    /*
            TF1* baseline{nullptr};
            double ga=100000., gm=5200., gs=200., hc=5500., lm=6274., ls=144.5, la=2000000., lshift=0.;
            BlFitFunction baseline_func(3.0);
            baseline = new TF1("baseline", baseline_func, 1000., 20000., 7);
            baseline->SetParameters(ga,gm,gs,hc,lm,ls,la);
            baseline->SetParNames("gAmp","gMean","gStd","hCut","lMean","lSigma","lAmp", "lshift");
            //baseline->FixParameter(0, ga);
            //baseline->FixParameter(1, gm);
            //baseline->FixParameter(2, gs);
            //baseline->FixParameter(3, hc);
            baseline->FixParameter(4, lm);
            baseline->FixParameter(5, ls);
            //baseline->FixParameter(6, la);
            projy_h->Fit("baseline");
            baseline->Write();
            projy_h->Write();

        
        outFile.Close();
*/

    return true;


    
}

void SvtBlFitHistoProcessor::finalize() {

    std::cout << "finalizing SvtBlFitHistoProcessor" << std::endl;
    outputHistos_->saveHistos(outF_,"");
    outF_->Close();
    delete outputHistos_;
    outputHistos_ = nullptr;
    delete inputHistos_;
    inputHistos_ = nullptr;

}

DECLARE_PROCESSOR(SvtBlFitHistoProcessor);
