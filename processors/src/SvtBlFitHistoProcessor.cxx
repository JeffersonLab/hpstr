#include "SvtBlFitHistoProcessor.h"
#include "anaUtils.h"

SvtBlFitHistoProcessor::SvtBlFitHistoProcessor(const std::string& name, Process& process)
    : Processor(name, process) {
    }

SvtBlFitHistoProcessor::~SvtBlFitHistoProcessor() {
}

void SvtBlFitHistoProcessor::configure(const ParameterSet& parameters) {

    std::cout << "Configuring SvtBlFitHistoProcessor" << std::endl;
    try
    {
        folder = parameters.getString("folder");
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

     

}

void SvtBlFitHistoProcessor::initialize(std::string inFilename, std::string outFilename) {

    std::cout << "initializing SvtBlFitHistoProcessor" << std::endl;
    //InFile containing 2D histograms from the SvtCondAnaProcessor
    inFile = new TFile(inFilename.c_str());
    outFile = new TFile(outFilename.c_str(),"RECREATE");
   
    utils::Get2DHistosFromFile(histos2d, histos2dk,inFile, folder, "0");
    int binning =1;

    for (unsigned int ih2d = 0; ih2d<histos2dk.size();++ih2d) {
    //for (unsigned int ih2d = 0; ih2d<1;++ih2d) {
    std::string histoname = histos2d[histos2dk[ih2d]]->GetName();
    histoname=histoname.substr(0,histoname.size()-1);
    std::cout << histoname << std::endl;
    std::string graphname_m = "mean_"  + histoname;
    std::string graphname_w = "width_" + histoname;
    std::string graphname_n = "norm_" + histoname;
    std::string graphname_l = "FitRangeLower_" + histoname;
    std::string graphname_u = "FitRangeUpper_" + histoname;
    
//Create 1D histograms to store fit values for each channel on each sensor. These fit values determined in process method, using profileYwithIterativeGaussFit function 

    histoMean  = new TH1D(graphname_m.c_str(),graphname_m.c_str(), histos2d[histos2dk[ih2d]]->GetNbinsX(),histos2d[histos2dk[ih2d]]->GetXaxis()->GetXmin(),histos2d[histos2dk[ih2d]]->GetXaxis()->GetXmax());
    outFile->cd();

    histoWidth = new TH1D(graphname_w.c_str(),graphname_w.c_str(), histos2d[histos2dk[ih2d]]->GetNbinsX(),histos2d[histos2dk[ih2d]]->GetXaxis()->GetXmin(),histos2d[histos2dk[ih2d]]->GetXaxis()->GetXmax());
    histoNorm = new TH1D(graphname_n.c_str(),graphname_n.c_str(), histos2d[histos2dk[ih2d]]->GetNbinsX(),histos2d[histos2dk[ih2d]]->GetXaxis()->GetXmin(),histos2d[histos2dk[ih2d]]->GetXaxis()->GetXmax());

    histoFitRangeLower = new TH1D(graphname_l.c_str(),graphname_l.c_str(), histos2d[histos2dk[ih2d]]->GetNbinsX(),histos2d[histos2dk[ih2d]]->GetXaxis()->GetXmin(),histos2d[histos2dk[ih2d]]->GetXaxis()->GetXmax());

    histoFitRangeUpper = new TH1D(graphname_u.c_str(),graphname_u.c_str(), histos2d[histos2dk[ih2d]]->GetNbinsX(),histos2d[histos2dk[ih2d]]->GetXaxis()->GetXmin(),histos2d[histos2dk[ih2d]]->GetXaxis()->GetXmax());



    }
}


bool SvtBlFitHistoProcessor::process() { 

    std::cout << "processing SvtBlFitHistoProcessor" << std::endl;
    
// use PF's tweaked fit code to perform Gauss Fit on ADC counts for every channel of a sensor
for (unsigned int ih2d = 0; ih2d<histos2dk.size();++ih2d) {     

    HistogramHelpers::profileYwithIterativeGaussFit(histos2d[histos2dk[ih2d]],histoMean,histoWidth,histoNorm,histoFitRangeLower,histoFitRangeUpper,binning,0);
    outfile->cd();
    histos2d[histos2dk[ih2d]]->Write();
    histoMean ->Write(graphname_m.c_str());
    histoWidth->Write(graphname_w.c_str());
    histoNorm->Write(graphname_n.c_str());
    histoFitRangeLower->Write(graphname_l.c_str());
    histoFitRangeUpper->Write(graphname_u.c_str());
    }

}

void SvtBlFitHistoProcessor::finalize() {

    std::cout << "finalizing SvtBlFitHistoProcessor" << std::endl;

}

DECLARE_PROCESSOR(SvtBlFitHistoProcessor)
