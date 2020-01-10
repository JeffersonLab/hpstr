#include "Svt2DBlHistos.h"
#include <math.h>
#include "TCanvas.h"

Svt2DBlHistos::Svt2DBlHistos(const std::string& inputName) {
    std::cout << "[Svt2DBlHistos] Constructor" << std::endl;
    m_name = inputName;
    HistoManager();
}

Svt2DBlHistos::~Svt2DBlHistos() {

    for (std::map<std::string, TGraphErrors*>::iterator it = baselineGraphs.begin(); 
            it!=baselineGraphs.end(); ++it) {
        if (it->second) {
            delete (it->second);
            it->second = nullptr;
        }
    }
    baselineGraphs.clear();
}

void Svt2DBlHistos::FillHistograms(RawSvtHit* rawSvtHit,float weight) {
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
