#include "SvtCondHistos.h"
#include <math.h>
#include "TCanvas.h"
#include "TString.h"

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
            h_name = Form("%s_%s_timesample_%i_hh", m_name.c_str(), half_module_names[ihm].c_str(), ihn);
            histos2d[h_name] = plot2D(h_name, "Channel", 640, 0, 640, "ADC Value", 5000, 0, 20000);
        }
    }
}

void SvtCondHistos::FillHistograms(RawSvtHit* rawSvtHit,float weight) {
    std::string histo_key;
    int mod = rawSvtHit->getModule();
    int lay = rawSvtHit->getLayer();
    std::string swTag = mmapper_->getStringFromSw(Form("ly%i_m%i", lay, mod));

    for (int i=0; i<6; i++){
        histo_key = Form("%s_%s_timesample_%i_hh", m_name.c_str(), swTag.c_str(), i);
        histos2d[histo_key]->Fill((float)rawSvtHit->getStrip(),(float)rawSvtHit->getADCs()[i], weight); 
    }

}      
