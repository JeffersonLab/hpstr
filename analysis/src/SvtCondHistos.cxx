#include "SvtCondHistos.h"
#include <math.h>
#include "TCanvas.h"

SvtCondHistos::SvtCondHistos(const std::string& inputName):HistoManager(inputName) {
    m_name = inputName;
    mmapper_ = new ModuleMapper(2019);
}

SvtCondHistos::~SvtCondHistos() {

    for (std::map<std::string, TGraphErrors*>::iterator it = baselineGraphs.begin(); 
            it!=baselineGraphs.end(); ++it) {
        if (it->second) {
            delete (it->second);
            it->second = nullptr;
        }
    }
    baselineGraphs.clear();
}


void SvtCondHistos::Define1DHistos() {
}

void SvtCondHistos::Define2DHistos() {
    std::string h_name = "";
    mmapper_->getStrings(half_module_names);
    std::cout << "half_module_names length: " << half_module_names.size() << std::endl;
    for (unsigned int ihm = 0; ihm<half_module_names.size(); ihm++) {
        for (unsigned int ihn = 0; ihn<6; ihn++) {
            auto n = std::to_string(ihn);
            h_name = m_name+"_"+half_module_names[ihm]+"_timesample_"+n;
            histos2d[h_name] = plot2D(h_name, "Channel", 640, 0, 640, "ADC Value", 5000, 0, 20000);
}
}
}


void SvtCondHistos::FillHistograms(RawSvtHit* rawSvtHit,float weight) {
    std::string swTag="";
    std::string build_key = "";
    auto mod = std::to_string(rawSvtHit->getModule());
    auto lay = std::to_string(rawSvtHit->getLayer());
    swTag= mmapper_->getStringFromSw("ly"+lay+"_m"+mod);

    for (int i=0; i<6; i++){
        build_key= m_name+"_"+swTag+"_timesample_"+std::to_string(i);
        histos2d[build_key]->Fill((float)rawSvtHit->getStrip(),(float)rawSvtHit->getADCs()[i], weight); 
    }



}      
