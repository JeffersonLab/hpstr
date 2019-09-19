#include "ClusterHistos.h"
#include <math.h>
#include "TCanvas.h"

ClusterHistos::ClusterHistos(const std::string& inputName):HistoManager(inputName) {
  m_name = inputName;
  mmapper_ = new ModuleMapper(2019);
}

ClusterHistos::~ClusterHistos() {
  
  std::cout<<"Cleaning ClusterHistos"<<std::endl;
  
  //TODO understand why this crashes
  /*
  if (mmapper_) {
    delete mmapper_; 
    mmapper_=nullptr;
  }
  */
  
  cluSizeMap.clear();
  chargeMap.clear();
  chargeCorrectedMap.clear();
  cluPositionMap.clear();
  for (std::map<std::string, TGraphErrors*>::iterator it = baselineGraphs.begin(); 
       it!=baselineGraphs.end(); ++it) {
    if (it->second) {
      delete (it->second);
      it->second = nullptr;
    }
  }
  baselineGraphs.clear();
}


void ClusterHistos::Define1DHistos() {

  //TODO improve this naming scheme
  std::string h_name = "";
 
  //Cluster position
  histos1d[m_name+"_gz"] = plot1D(m_name+"_gz","Global Z [mm]",20000,-1000,2000);
  
  mmapper_->getStrings(half_module_names);

  for (unsigned int ihm = 0; ihm<half_module_names.size(); ihm++) {
    h_name = m_name+"_"+half_module_names[ihm]+"_charge";
    histos1d[h_name] = plot1D(h_name,"charge",100,0,10000);
    h_name = m_name+"_"+half_module_names[ihm]+"_cluSize";
    histos1d[h_name] = plot1D(h_name,"cluSize",10,0,10);
    cluSizeMap[h_name]          = 0.;
    chargeMap[h_name]           = 0.;
    chargeCorrectedMap[h_name]  = 0.;
    cluPositionMap[h_name]      = 0.;
  }//half module plots
}

void ClusterHistos::Define2DHistos() {
  std::string h_name = "";
  histos2d[m_name+"_charge_L0_top_vs_gx"] = plot2D(m_name+"_charge_L0_top_vs_gx",
						   "Global X [mm] ",200,-100,100,
						   "edep",500,0,10000);
  int nbins = 1000;
  float pitch = 0.050;
  float startY = 0.700;
  
  histos2d[m_name+"_charge_L0T_vs_gy"] = plot2D(m_name+"_charge_L0T_vs_gy",
						"Global Y [mm]",nbins,startY,(nbins+1)*pitch,
						"edep",100,0,10000);
  
  
  histos2d[m_name+"_charge_L0T_vs_gx"] = plot2D(m_name+"_charge_L0T_vs_gx",
						"Global X [mm] ",200,-100,100,
						"edep",100,0,10000);
  
  
  histos2d[m_name+"_charge_L0B_vs_gy"] = plot2D(m_name+"_charge_L0B_vs_gy",
						"Global Y [mm]",nbins,startY,(nbins+1)*pitch,
						"edep",100,0,10000);
  
  
  histos2d[m_name+"_charge_L0B_vs_gx"] = plot2D(m_name+"_charge_L0B_vs_gx",
						"Global X [mm] ",200,-100,100,
						"edep",100,0,10000);
  

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


bool ClusterHistos::LoadBaselineHistos(const std::string& baselineRun) {
  
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
  
  while ( key = (TKey*)next()) {
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





void ClusterHistos::FillHistograms(TrackerHit* hit,float weight) {

  TRefArray* rawhits_ = hit->getRawHits();
  //int  iv      = -1;   // 0 top, 1 bottom
  //int  it      = -1;   // 0 axial, 1 stereo
  //int  ily     = -1;   // 0-6

  //TODO do this better
  
  //std::cout<<"Size:" <<rawhits_->GetEntries()<<std::endl;

  std::string swTag = "";
  
  for (unsigned int irh = 0; irh < rawhits_->GetEntries(); ++irh) {

    RawSvtHit * rawhit  = static_cast<RawSvtHit*>(rawhits_->At(irh));
    //rawhit layers go from 1 to 14. Example: RawHit->Layer1 is layer0 axial on top and layer0 stereo in bottom.
    
    swTag = "ly"+std::to_string(rawhit->getLayer())+"_m"+std::to_string(rawhit->getModule());
    
    std::string key = mmapper_->getStringFromSw(swTag);
    //std::cout<<"----"<<std::endl;
    //std::cout<<"From Mapper      "<<mmapper_->getStringFromSw(swTag)<<std::endl;
    //std::cout<<"----"<<std::endl;
    
    //2D cluster charge
    chargeMap     [m_name+"_"+key+"_charge"]  += rawhit->getAmp();
    
    double baseline = -999;
    double strip    = -999;
    
    //2D cluster corrected charge
    baselineGraphs[mmapper_->getHwFromString(key)]->GetPoint(rawhit->getStrip(),strip,baseline);
    
    float sample0 = baseline - rawhit->getADCs()[0];
    float sample1 = baseline - rawhit->getADCs()[1]; 
    
    chargeCorrectedMap[m_name+"_"+key+"_charge"]  += (rawhit->getAmp() + sample0);
    
    histos2d[m_name+"_"+key+"_sample0_vs_Amp"]->Fill(rawhit->getAmp(),sample0,weight);
    histos2d[m_name+"_"+key+"_sample1_vs_Amp"]->Fill(rawhit->getAmp(),sample1,weight);
    
    histos2d[m_name+"_"+key+"_sample0_vs_stripPos"]->Fill(rawhit->getStrip(),-sample0,weight);
    histos2d[m_name+"_"+key+"_sample1_vs_stripPos"]->Fill(rawhit->getStrip(),-sample1,weight);
    
    
    //2D cluster size1
    cluSizeMap    [m_name+"_"+key+"_cluSize"] ++;
    
    //2D Weighted position numerator
    cluPositionMap[m_name+"_"+key+"_charge"]  += rawhit->getAmp()*rawhit->getStrip();
    //std::cout<<"rawhit->getStrip()::"<<rawhit->getStrip()<<std::endl;
  }

  //TODO make this more efficient: useless to loop all over the possibilities

  for (std::map<std::string, int>::iterator it = cluSizeMap.begin(); it!=cluSizeMap.end(); ++it ) {
    if (it->second != 0) {
      //std::cout<<"Filling..."<<it->first<<" "<<histos1d[it->first]<<std::endl;
      histos1d[it->first]->Fill(it->second,weight);
      cluSizeMap[it->first]= 0;
    }
  }// fills the maps
  
  //TODO make this more efficient: useless to loop all over the possibilities
  for (std::map<std::string, double>::iterator it = chargeMap.begin(); it!=chargeMap.end(); ++it ) {
    //TODO make it better
    //Avoid comparing to 0.0 and check if there is a charge deposit on this 
    if (it->second > 1e-6) {
      
      
      std::string plotID = (it->first).substr(0,(it->first).find("_charge"));
      
      //it->second holds the charge
      //it->first =  charge histogram name.
      double charge = it->second;
      histos1d[it->first]->Fill(charge,weight);
      double weighted_pos = cluPositionMap[it->first] / (charge);
      double chargeCorrected = chargeCorrectedMap[it->first];
      //std::cout<<"weighted pos "<<weighted_pos<<std::endl;
      histos2d[it->first+"_vs_stripPos"]->Fill(weighted_pos,charge,weight);
      
      // Fill the baseline corrected charge
      histos2d[it->first+"_corrected_vs_stripPos"]->Fill(weighted_pos,chargeCorrected,weight);
      
      //Fill local vs global
      
      histos2d[plotID+"_stripPos_vs_gy"]->Fill(fabs(hit->getGlobalY()),weighted_pos,weight);
      
      double globRad = sqrt(hit->getGlobalX() * hit->getGlobalX() + hit->getGlobalY()+hit->getGlobalY());
      histos2d[it->first+"_vs_globRad"]->Fill(globRad,charge,weight);


      chargeMap[it->first]          = 0.0;
      chargeCorrectedMap[it->first] = 0.0;
      cluPositionMap[it->first]     = 0.0;
      
    } 
  }
  histos1d[m_name+"_gz"]->Fill(hit->getGlobalZ(),weight);
  //1D
  //histos1d[m_name+"_charge"]->Fill(hit->getCharge(),weight);
  //2D
  if (hit->getGlobalZ() < 50 && hit->getGlobalZ() > 40) {
    histos2d[m_name+"_charge_L0T_vs_gx"]->Fill(hit->getGlobalX(),hit->getCharge(),weight);
    histos2d[m_name+"_charge_L0T_vs_gy"]->Fill(fabs(hit->getGlobalY()),hit->getCharge(),weight);
  }
  
  if (hit->getGlobalZ() < 60 && hit->getGlobalZ() > 55) {
    histos2d[m_name+"_charge_L0B_vs_gx"]->Fill(hit->getGlobalX(),hit->getCharge(),weight);
    histos2d[m_name+"_charge_L0B_vs_gy"]->Fill(fabs(hit->getGlobalY()),hit->getCharge(),weight);
  }
}
