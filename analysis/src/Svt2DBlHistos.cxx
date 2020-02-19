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

void Svt2DBlHistos::get2DHistoOccupancy(std::vector<std::string> histos2dNames) {
    /*
    //This block extracts the number of entries per channel, per event, and fills a 1D histogram
    //that will give the total number of hits/channel on the x axis, & number of events on the Y axis
    std::cout << "Running Occupancy method" << std::endl;
    for(std::vector<std::string>::iterator t = histos2dNames.begin(); t != histos2dNames.end(); ++t){
       std::string hybridKey_hh = *t;
       TH2F* histo_hh = get2dHisto(hybridKey_hh);

        for(int i = 0; i < histo_hh->GetXaxis()->GetNbins(); ++i) {
            int entries= histo_hh->ProjectionY(Form("%s_projection_%i",histo_hh->GetName(),i),
            i+1,i+1,"e")->GetEntries();
            Fill1DHisto("Events_per_Channel_h", entries, 1.);
       }
       
    }
    TH1F* co_h = get1dHisto("raw_hits_Events_per_Channel_h");
    int xmax=co_h->FindLastBinAbove(0,1);
    //double ymax=co_h->GetBinContent(co_h->GetMaximumBin());
    co_h->GetXaxis()->SetRangeUser(0,xmax);
    //co_h->GetYaxis()->SetRange(0,ymax);
    */
}

void Svt2DBlHistos::FillHistograms(std::vector<RawSvtHit*> *rawSvtHits_,float weight) {

    int nhits = rawSvtHits_->size();
    std::vector<std::string> hybridStrings={};
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
                Fill1DHisto("hitN_"+swTag+"_h", svtHybMulti[i][j],weight);
            }
        }
    }

    Fill1DHisto("svtHitN_h", nhits,weight);
    //End of counting block

    //Populates histograms for each hybrid
    for (int i = 0; i < nhits; i++)
    {
        RawSvtHit* rawSvtHit = rawSvtHits_->at(i);
        auto mod = std::to_string(rawSvtHit->getModule());
        auto lay = std::to_string(rawSvtHit->getLayer());
        std::string swTag= mmapper_->getStringFromSw("ly"+lay+"_m"+mod);
        
        
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
