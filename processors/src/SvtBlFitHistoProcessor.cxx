#include "SvtBlFitHistoProcessor.h"
#include <string>
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
        std::string inputname = *jj;
        std::string FitRangeLowerkey;
        std::string FitRangeUpperkey;
        std::string meankey;
        std::string widthkey;
        std::string normkey;

        for (vector<string>::iterator it = outputHistos_->histos1dNamesfromJson.begin();
                it != outputHistos_->histos1dNamesfromJson.end(); ++it) 
        {
            std::string fitparname = *it;
            size_t L = fitparname.find_last_of("L");
            std::string tag = fitparname.substr(L);
            if (inputname.find(tag) != std::string::npos) {
                //Substrings below must match the names in the JSON file
                if (fitparname.find("FitRangeLower") != std::string::npos) 
                    FitRangeLowerkey = fitparname;
                if (fitparname.find("FitRangeUpper") != std::string::npos) 
                    FitRangeUpperkey = fitparname;
                if (fitparname.find("mean") != std::string::npos) 
                    meankey = fitparname;
                if (fitparname.find("width") != std::string::npos) 
                    widthkey = fitparname;
                if (fitparname.find("norm") != std::string::npos) 
                    normkey = fitparname;

            }
        }
<<<<<<< HEAD
        std::cout << inputHistos_->get2dHisto(inputname)->GetName() << std::endl; 
        HistogramHelpers::profileYwithIterativeGaussFit(inputHistos_->get2dHisto(inputname),
                outputHistos_->get1dHisto(meankey),
                outputHistos_->get1dHisto(widthkey),
                outputHistos_->get1dHisto(normkey),
                outputHistos_->get1dHisto(FitRangeLowerkey),
                outputHistos_->get1dHisto(FitRangeUpperkey),
                IterativeGaussFitCut_,
                binning_, 0);
                
=======
    

        TH2F* histo_hh = inputHistos_->get2dHisto(inputname);
        for(int cc=0; cc < histo_hh->GetXaxis()->GetNbins(); ++cc) {
            TH1D* projy_h = histo_hh->ProjectionY(Form("%s_projection_%i",histo_hh->GetName(),cc),
                    cc+1,cc+1,"e");
        
    
            TF1* baseline{nullptr};
            double ga=1., gm=0., gs=1., hc=10., lm=1., ls=1., la=1.;
            BlFitFunction baseline_func(3.0);
            baseline = new TF1("baseline", baseline_func, 1, 1, 7);
            baseline->SetParameters(ga,gm,gs,hc,lm,ls,la);
            baseline->SetParNames("gAmp","gMean","gStd","hCut","lMean","lSigma","lAmp");
            projy_h->Fit("baseline");

        }



        /* std::cout << inputHistos_->get2dHisto(inputname)->GetName() << std::endl; 
           HistogramHelpers::profileYwithIterativeGaussFit(inputHistos_->get2dHisto(inputname),
           outputHistos_->get1dHisto(meankey),
           outputHistos_->get1dHisto(widthkey),
           outputHistos_->get1dHisto(normkey),
           outputHistos_->get1dHisto(FitRangeLowerkey),
           outputHistos_->get1dHisto(FitRangeUpperkey),
           binning_, 0);
           */
>>>>>>> 095347c5554f981ef2f222665ce689a3def2240e
    }

    /*// use PF's tweaked fit code to perform Gauss Fit on ADC counts for every channel of a sensor
      for (unsigned int ih2d = 0; ih2d<histos2dk.size();++ih2d) {     
    //for (unsigned int ih2d=0; ih2d<2; ++ih2d) {
    std::string histoname = histos2d[histos2dk[ih2d]]->GetName();
    histoname=histoname.substr(0,histoname.size()-1);
    graphname_m = "mean_"  + histoname;
    graphname_w = "width_" + histoname;
    graphname_n = "norm_" + histoname;
    graphname_l = "FitRangeLower_" + histoname;
    graphname_u = "FitRangeUpper_" + histoname;


    HistogramHelpers::profileYwithIterativeGaussFit(histos2d[histos2dk[ih2d]],histoMean[histoname],histoWidth[histoname],histoNorm[histoname],histoFitRangeLower[histoname],histoFitRangeUpper[histoname],binning,0);
    outF_->cd();
    histos2d[histos2dk[ih2d]]->Write();
    histoMean[histoname] ->Write(graphname_m.c_str());
    histoWidth[histoname]->Write(graphname_w.c_str());
    histoNorm[histoname]->Write(graphname_n.c_str());
    histoFitRangeLower[histoname]->Write(graphname_l.c_str());
    histoFitRangeUpper[histoname]->Write(graphname_u.c_str());
    }
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
