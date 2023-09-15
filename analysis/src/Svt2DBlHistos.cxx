#include "Svt2DBlHistos.h"
#include <math.h>
#include "TCanvas.h"

Svt2DBlHistos::Svt2DBlHistos(const std::string& inputName, ModuleMapper* mmapper) {
    m_name = inputName;
    mmapper_ = mmapper;
}

Svt2DBlHistos::~Svt2DBlHistos() {
}

void Svt2DBlHistos::DefineHistos(){
    //Define vector of hybrid names using ModuleMapper 
    //Use this list to define multiple copies of histograms, one for each hybrid, from json file
    std::vector<std::string> hybridNames;
    mmapper_->getStrings(hybridNames);
    if(debug_ > 0){
        for(int i = 0; i< hybridNames.size(); i++) 
            std::cout << "Hybrid: " << hybridNames.at(i) << std::endl;
    }
    //Define histos
    //All histogram keys in the JSON file that contain special tag will have multiple copies of that histogram template made, one for each string
    std::string makeMultiplesTag = "SvtHybrids";
    HistoManager::DefineHistos(hybridNames, makeMultiplesTag );

}

void Svt2DBlHistos::FillHistograms(std::vector<RawSvtHit*> *rawSvtHits_,float weight) {

    int nhits = rawSvtHits_->size();
    std::vector<std::string> hybridStrings={};
    std::string histokey;
    if(Event_number%10000 == 0) std::cout << "Event: " << Event_number 
        << " Number of RawSvtHits: " << nhits << std::endl;

    //Following Block counts the total number of hits each hybrid records per event
    int svtHybMulti[4][15] = {0};
    for (int i = 0; i < nhits; i++)
    {
        RawSvtHit* rawSvtHit = rawSvtHits_->at(i);
        int mod = rawSvtHit->getModule();
        int lay = rawSvtHit->getLayer();
        svtHybMulti[mod][lay]++;

    }
    for (int i =0; i < 4; i++)
    {
        for (int j = 1; j < 15; j++)
        {
            if (!(j<9 && i>1))
            {   
                std::string swTag = mmapper_->getStringFromSw("ly"+std::to_string(j)+"_m"+std::to_string(i));
                hybridStrings.push_back(swTag);
                Fill1DHisto(swTag+ "_SvtHybridsHitN_h", svtHybMulti[i][j],weight);
            }
        }
    }

    Fill1DHisto("SvtHitMulti_h", nhits,weight);
    //End of counting block

    //Populates histograms for each hybrid
    for (int i = 0; i < nhits; i++)
    {
        RawSvtHit* rawSvtHit = rawSvtHits_->at(i);
        auto mod = std::to_string(rawSvtHit->getModule());
        auto lay = std::to_string(rawSvtHit->getLayer());
        std::string swTag= mmapper_->getStringFromSw("ly"+lay+"_m"+mod);
        
        //Manually select which baselines (0 - 6) are included. THIS MUST MATCH THE JSON FILE!
        int ss = 0;
        histokey = swTag + "_SvtHybrids_s"+std::to_string(ss)+"_hh";
                    Fill2DHisto(histokey, 
                (float)rawSvtHit->getStrip(),
                (float)rawSvtHit->getADCs()[ss], 
                weight);

        ss = 3;
        histokey = swTag + "_SvtHybrids_s"+std::to_string(ss)+"_hh";
                    Fill2DHisto(histokey, 
                (float)rawSvtHit->getStrip(),
                (float)rawSvtHit->getADCs()[ss], 
                weight);
        
    }

            Event_number++;
}      
