#include "Svt2DBlHistos.h"
#include <math.h>
#include "TCanvas.h"

Svt2DBlHistos::Svt2DBlHistos(const std::string& inputName) {
    m_name = inputName;
    mmapper_ = new ModuleMapper();
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

void Svt2DBlHistos::FillHistograms(std::vector<RawSvtHit*> *rawSvtHits_,float weight) {

    int nhits = rawSvtHits_->size();
    
    std::string histokey;
    if(Event_number%1000 == 0) std::cout << "Event: " << Event_number 
        << " Number of RawSvtHits: " << nhits << std::endl;

    //Following Block counts the total number of hits each hybrid records per event
    int svtHybMulti[4][15] = {0};
    for (int i = 0; i < nhits; i++)
    {
        RawSvtHit* rawSvtHit = rawSvtHits_->at(i);
        int mod = rawSvtHit->getModule();
        int lay = rawSvtHit->getLayer();
        //std::cout << "module: " << mod << std::endl;
        svtHybMulti[mod][lay]++;

    }
    for (int i =0; i < 4; i++)
    {
        for (int j = 1; j < 15; j++)
        {
            if (!(j<9 && i>1))
            {   
                std::string swTag = mmapper_->getStringFromSw("ly"+std::to_string(j)+"_m"+std::to_string(i));
                Fill1DHisto("hitN_"+swTag+"_h", svtHybMulti[i][j],weight);
            }
        }
    }

    Fill1DHisto("svtHitN_h", nhits,weight);
    //End of counting block

    //Populates histograms for each hybrid
    for (int i = 0; i < nhits; i++)
    {
        //std::cout << "hit: " << i << std::endl;
        RawSvtHit* rawSvtHit = rawSvtHits_->at(i);
        auto mod = std::to_string(rawSvtHit->getModule());
        auto lay = std::to_string(rawSvtHit->getLayer());
        //std::cout << "module: " << mod << std::endl;
        std::string swTag= mmapper_->getStringFromSw("ly"+lay+"_m"+mod);
        //std::cout << swTag << std::endl;
        //std::cout << histokey << std::endl;
        
        
        for(int ss = 0; ss < 6; ss++)
        {
            histokey = "baseline"+std::to_string(ss)+"_"+swTag+"_hh";
            Fill2DHisto(histokey, 
                    (float)rawSvtHit->getStrip(),
                    (float)rawSvtHit->getADCs()[ss], 
                    weight);
        }
    }
    Event_number++;

    

}      
