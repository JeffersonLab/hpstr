#include "SvtCondHistos.h"
#include <math.h>
#include "TCanvas.h"

SvtCondHistos::SvtCondHistos(const std::string& inputName):HistoManager(inputName) {
    m_name = inputName;
    mmapper_ = new ModuleMapper(2019);
}

SvtCondHistos::~SvtCondHistos() {

    std::cout<<"Cleaning SvtCondHistos"<<std::endl;

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

    //TODO improve this naming scheme
    std::string h_name = "";
    histos1d[m_name+"_gz"] = plot1D(m_name+"_gz","Global Z [mm]",20000,-1000,2000);
    std::cout << "Module name " << m_name << std::endl;
    mmapper_->getStrings(half_module_names);
    for (unsigned int ihm = 0; ihm<half_module_names.size(); ihm++) {
        h_name = m_name+"_"+half_module_names[ihm]+"_charge";
        histos1d[h_name] = plot1D(h_name,"charge",100,0,10000);
	std::cout << "Hybrid: " << h_name << std::endl;
        h_name = m_name+"_"+half_module_names[ihm]+"_cluSize";
	std::cout << "h_name: " << h_name << std::endl;
        histos1d[h_name] = plot1D(h_name,"cluSize",10,0,10);

    }//half module plots
}

/*void SvtCondHistos::Define2DHistos() {
    f a container has volumeV′in its rest frameS′,  what is the volume measured by anobserver in frameSmoving at speedvrelative toS′?std::string h_name = "";
    int nbins = 1000;
    float pitch = 0.055;
    float startY = 0.700;

    histos2d[m_name+"_charge_L0T_vs_gy"] = plot2D(m_name+"_charge_L0T_vs_gy",
            "Global Y [mm]",nbins,startY,(nbins+1)*pitch,
            "edep",100,0,1e-5);


    histos2d[m_name+"_charge_L0T_vs_gx"] = plot2D(m_name+"_charge_L0T_vs_gx",
            "Global X [mm] ",80,-20,20,
            "edep",100,0,1e-5);


    histos2d[m_name+"_charge_L0B_vs_gy"] = plot2D(m_name+"_charge_L0B_vs_gy",
            "Global Y [mm]",nbins,startY,(nbins+1)*pitch,
            "edep",100,0,1e-5);


    histos2d[m_name+"_charge_L0B_vs_gx"] = plot2D(m_name+"_charge_L0B_vs_gx",
            "Global X [mm] ",80,-20,20,
            "edep",100,0,1e-5);


    // location of the hits

    histos2d[m_name+"_gy_L0T_vs_gx"] = plot2D(m_name+"_gy_L0T_vs_gx",
            "Global X [mm] ",400,-20,20,
            "Global Y [mm]",200,0,5);



    histos2d[m_name+"_gy_L0B_vs_gx"] = plot2D(m_name+"_gy_L0B_vs_gx",
            "Global X [mm] ",400,-20,20,
            "Global Y [mm]",200,0,5);





    //bin size must be multiple of 20 adc counts

    for (unsigned int ihm = 0; ihm<half_module_names.size(); ihm++) {

        h_name = m_name+"_"+half_module_names[ihm]+"_charge_vs_stripPos";
        histos2d[h_name] = plot2D(h_name,
                "Strip Position",640,0,640,
                "charge",100,0,10000);

        h_name = m_name+"_"+half_module_names[ihm]+"_charge_vs_globRad";

        histos2d[h_name] = plot2D(h_name,
                "#sqrt{x^{2} + y^{2}}",600,0,150,
                "charge",100,0,10000);

        //Charge with baseline substracted
        h_name = m_name+"_"+half_module_names[ihm]+"_charge_corrected_vs_stripPos";
        histos2d[h_name] = plot2D(h_name,
                "Strip Position",640,0,640,
                "corrected charge",100,0,10000);

        //sample 0 vs Strip
        h_name = m_name+"_"+half_module_names[ihm]+"_sample0_vs_stripPos";
        histos2d[h_name] = plot2D(h_name,
                "Strip Position",640,0,640,
                "sample0",200,-2000,2000);

        h_name = m_name+"_"+half_module_names[ihm]+"_sample1_vs_stripPos";
        histos2d[h_name] = plot2D(h_name,
                "Strip Position",640,0,640,
                "sample1",200,-2000,2000);


        //adc[0] (Sample 0) vs Amplitude
        h_name = m_name+"_"+half_module_names[ihm]+"_sample0_vs_Amp";
        histos2d[h_name] = plot2D(h_name,
                "Amp",    100,0,10000,
                "Sample0",200,-2000,2000);


        //adc[1] (Sample 1) vs Amplitude
        h_name = m_name+"_"+half_module_names[ihm]+"_sample1_vs_Amp";
        histos2d[h_name] = plot2D(h_name,
                "Amp",    100,0,10000,
                "Sample1",200,-2000,2000);


        h_name = m_name+"_"+half_module_names[ihm]+"_stripPos_vs_gy";
        histos2d[h_name] = plot2D(h_name,
                "Global Y [mm]", nbins, startY, (nbins+1)* pitch,
                "strip Pos", 640,0,640);

    }
}


bool SvtCondHistos::LoadBaselineHistos(const std::string& baselineRun) {

    baselineRun_ = baselineRun;


    TFile *baselinesFile = new TFile((baselineFits_+"/hpssvt_"+baselineRun+"_baselineFits.root").c_str());

    if (!baselinesFile) 
        return false;

    TDirectory* dir = baselinesFile->GetDirectory("baseline");

    TList* keyList = dir->GetListOfKeys();
    TIter next(keyList);
    TKey* key;

    //I assume that there are only TGraphErrors
    //TObject* obj;

    while ( (key = (TKey*)next())) {
        //obj = key->ReadObj();
        //if (strcmp(obj->IsA()->GetName(),"TGraphErrors") != 0 )
        //continue;


        //x values go from 0 to 512 (513 points) for L0-1. Last point is 0
        //x values go from 0 to 639 (640 points) for other layers
        std::string graph_key = key->GetName();
        graph_key = graph_key.substr(graph_key.find("F"),4);
        baselineGraphs[graph_key] = (TGraphErrors*) (dir->Get(key->GetName()));
    }

    //for (std::map<std::string,TGraphErrors*>::iterator it = baselineGraphs.begin(); it!=baselineGraphs.end(); ++it)
    //std::cout<<it->first<<" " <<it->second->GetN()<<std::endl;
    //
    //for (int point = 0 ; point < baselineGraphs["F5H1"]->GetN();point++) {
    //Double_t x=-999;
    //Double_t y=-999;
    //baselineGraphs["F5H1"]->GetPoint(point,x,y);
    //std::cout<<point<<" x "<<x<<" y "<<y<<std::endl;
    //}

    baselinesFile->Close();
    delete baselinesFile;
    baselinesFile = nullptr;

    return true;
}
*/


void SvtCondHistos::FillHistograms(RawSvtHit* rawSvtHit,float weight) {
	std::string swTag = "";
// for (unsigned int irh = 0; irh < rawSvtHit->GetEntries(); ++irh) {

	// swTag = "ly"+std::to_string(rawSvtHit->getLayer())+"_m"+std::to_string(rawSvtHit->getModule());
	 //std::string key = mmapper_->getStringFromSw(swTag);

	 float sample0 = rawSvtHit->getADCs()[0];
	 histos1d[hitOnTrack_2D_L0B_axial_cluSize]->Fill(sample0,weight);

	// histos1d[m_name+"_"+key+"_sample0_vs_Amp"]->Fill(sample0,weight);
	//RawSvtHit* rawhit = static_cast<RawSvtHit*>(rawhits_->At(irh));
	
	



// }
 }      
