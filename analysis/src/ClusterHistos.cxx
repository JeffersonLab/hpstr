#include "ClusterHistos.h"
#include <math.h>

void ClusterHistos::Define1DHistos() {

  //TODO improve this naming scheme
  std::string h_name = "";

  //Cluster position
  histos1d[m_name+"_gz"] = plot1D(m_name+"_gz","Global Z [mm]",20000,-1000,2000);
  
  //2D hits plots
  for (unsigned int iv = 0 ; iv < volumes.size(); ++iv) {
    for (unsigned int ily = 0; ily < layers.size(); ++ily) {
      for (unsigned int is = 0; is < sides.size(); ++is) {

	h_name = m_name+"_"+volumes[iv]+"_"+layers[ily]+"_"+sides[is]+"_charge";
	histos1d[h_name] = plot1D(h_name,"charge",100,0,10000);
	
	h_name = m_name+"_"+volumes[iv]+"_"+layers[ily]+"_"+sides[is]+"_cluSize";
	histos1d[h_name] = plot1D(h_name,"cluSize",10,0,10);
	cluSizeMap[h_name]      = 0;
	chargeMap[h_name]       = 0.;
	cluPositionMap[h_name]  = 0.;
      }
    }
  }
}

void ClusterHistos::Define2DHistos() {
  std::string h_name = "";
  histos2d[m_name+"_charge_vs_gx"] = plot2D(m_name+"_charge_vs_gx",
					      "Global X [mm] ",100,0,100,
					      "edep",100,0,10000);
  int nbins = 1000;
  float pitch = 0.055;
  
  histos2d[m_name+"_charge_vs_gy"] = plot2D(m_name+"_charge_vs_gy",
					    "Gloabl Y [mm]",nbins,0.750,(nbins+1)*0.055,
					    "edep",100,0,10000);
  
  for (unsigned int iv = 0 ; iv < volumes.size(); ++iv) {
    for (unsigned int ily = 0; ily < layers.size(); ++ily) {
      for (unsigned int is = 0; is < sides.size(); ++is) {
	
	h_name = m_name+"_"+volumes[iv]+"_"+layers[ily]+"_"+sides[is]+"_charge_vs_stripPos";
	histos2d[h_name] = plot2D(h_name,
				  "Strip Position",640,0,640,
				  "charge",100,0,10000);
	
	h_name = m_name+"_"+volumes[iv]+"_"+layers[ily]+"_"+sides[is]+"_charge_vs_globRad";
	histos2d[h_name] = plot2D(h_name,
				  "#sqrt{x^{2} + y^{2}}",600,0,150,
				  "charge",100,0,10000);
      }
    }
  }
}

void ClusterHistos::FillHistograms(TrackerHit* hit,float weight) {

  TRefArray* rawhits_ = hit->getRawHits();
  int  iv      = -1;   // 0 top, 1 bottom
  int  is      = -1;   // 0 axial, 1 stereo
  int  ily     = -1;   // 0-6

  //TODO do this better
  //std::cout<<"Size:" <<rawhits_->GetEntries()<<std::endl;
  for (unsigned int irh = 0; irh < rawhits_->GetEntries(); ++irh) {

    RawSvtHit * rawhit  = static_cast<RawSvtHit*>(rawhits_->At(irh));
    //rawhit layers go from 1 to 14. Example: RawHit->Layer1 is layer0 axial on top and layer0 stereo in bottom.
    ily = (rawhit->getLayer() - 1) / 2;

    //Get the volume: even = top, odd = bottom
    if (rawhit->getModule() % 2 == 0)
      iv = 0;
    else
      iv = 1;
    //top volume
    if (!iv) {
      //layer odd => axial / layer even => stereo
      if (rawhit->getLayer() % 2 != 0)
	is = 0;
      else
	is = 1;
    }
    //bottom volume
    else {
      //layer even => axial / layer odd => stereo
      if (rawhit->getLayer() % 2 == 0)
	is = 0;
      else
	is = 1;
    }
    //std::cout<<"ily:"<<ily<<std::endl;
    //std::cout<<"iv:"<<iv<<std::endl;
    //std::cout<<"is:"<<is<<std::endl;
    //histos1d[m_name+"_"+volumes[iv]+"_"+layers[ily]+"_"+sides[is]+"_charge"]->Fill(1e-5,weight);

    //2D cluster charge
    chargeMap     [m_name+"_"+volumes[iv]+"_"+layers[ily]+"_"+sides[is]+"_charge"]  += rawhit->getAmp();

    //2D cluster size
    cluSizeMap    [m_name+"_"+volumes[iv]+"_"+layers[ily]+"_"+sides[is]+"_cluSize"] ++;
    
    //2D Weighted position numerator
    cluPositionMap[m_name+"_"+volumes[iv]+"_"+layers[ily]+"_"+sides[is]+"_charge"]  += rawhit->getAmp()*rawhit->getStrip();
    //std::cout<<"rawhit->getStrip()::"<<rawhit->getStrip()<<std::endl;
  }
  for (std::map<std::string, int>::iterator it = cluSizeMap.begin(); it!=cluSizeMap.end(); ++it ) {
    if (it->second != 0) {
      //std::cout<<"Filling..."<<it->first<<" "<<histos1d[it->first]<<std::endl;
      histos1d[it->first]->Fill(it->second,weight);
      cluSizeMap[it->first]= 0;
    }
  }
  for (std::map<std::string, double>::iterator it = chargeMap.begin(); it!=chargeMap.end(); ++it ) {
    //Avoid comparing to 0.0
    if (it->second > 1e-6) {
      //std::cout<<"Filling..."<<it->first<<" "<<histos1d[it->first]<<std::endl;
      //it->second holds the charge
      //it->first =  charge histogram name.
      double charge = it->second;
      histos1d[it->first]->Fill(charge,weight);
      double weighted_pos = cluPositionMap[it->first] / (charge);
      //std::cout<<"weighted pos "<<weighted_pos<<std::endl;
      histos2d[it->first+"_vs_stripPos"]->Fill(weighted_pos,charge,weight);
      double globRad = sqrt(hit->getGlobalX() * hit->getGlobalX() + hit->getGlobalY()+hit->getGlobalY());
      histos2d[it->first+"_vs_globRad"]->Fill(globRad,charge,weight);
      chargeMap[it->first]      = 0.0;
      cluPositionMap[it->first] = 0.0;
    }
  }
  histos1d[m_name+"_gz"]->Fill(hit->getGlobalZ(),weight);
  //1D
  //histos1d[m_name+"_charge"]->Fill(hit->getCharge(),weight);
  //2D
  //histos2d[m_name+"_charge_vs_gx"]->Fill(hit->getGlobalX(),hit->getCharge(),weight);
  //histos2d[m_name+"_charge_vs_gy"]->Fill(hit->getGlobalY(),hit->getCharge(),weight);
}
