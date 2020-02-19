#include "RawSvtHitMCHistos.h"
#include <math.h>
#include "TCanvas.h"

RawSvtHitMCHistos::RawSvtHitMCHistos(const std::string& inputName) {
    m_name = inputName;
    mmapper_ = new ModuleMapper();
}

RawSvtHitMCHistos::~RawSvtHitMCHistos() {

}

void RawSvtHitMCHistos::FillHistograms(std::vector<RawSvtHit*> *rawSvtHits_,float weight) {

    int nhits = rawSvtHits_->size();
    int nBadHits = 0;
    
    std::string histokey;
    if(Event_number%1000 == 0) std::cout << "Event: " << Event_number 
        << " Number of RawSvtHits: " << nhits << std::endl;

    //Following Block counts the total number of hits each hybrid records per event
    int svtHybMulti[4][15] = {0};
    double simHitPosX = -999.9;
    double simHitPosY = -999.9;
    for (int i = 0; i < nhits; i++)
    {
        RawSvtHit* rawSvtHit = rawSvtHits_->at(i);
        int mod = rawSvtHit->getModule();
        int lay = rawSvtHit->getLayer();
        simHitPosX = rawSvtHit->getSimPos()[0];
        simHitPosY = rawSvtHit->getSimPos()[1];
        svtHybMulti[mod][lay]++;
        std::string swTag = mmapper_->getStringFromSw("ly"+std::to_string(lay)+"_m"+std::to_string(mod));
        Fill1DHisto("Edep_"+swTag+"_h", rawSvtHit->getSimEdep()*1000.0,weight);
        Fill2DHisto("simHitPos_"+swTag+"_hh", simHitPosX, simHitPosY, weight);
        if(rawSvtHit->getADCs()[0] < 0) 
        {
            Fill1DHisto("badEdep_"+swTag+"_h", rawSvtHit->getSimEdep()*1000.0,weight);
            Fill2DHisto("badSimHitPos_"+swTag+"_hh", simHitPosX, simHitPosY, weight);
            nBadHits++;
        }
    }

    Fill1DHisto("svtNbadHits_h", nBadHits, weight);
    //End of counting block

    Event_number++;

}      
