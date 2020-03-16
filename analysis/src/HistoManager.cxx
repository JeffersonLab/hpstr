#include "HistoManager.h"
#include <iostream>
#include "TKey.h"
#include "TClass.h"
#include <fstream>
#include <iomanip>
#include <vector>

HistoManager::HistoManager() {
    mmapper_ = new ModuleMapper();
    m_name = "default";
}

HistoManager::HistoManager(const std::string& inputName) {
    m_name = inputName;
    mmapper_ = new ModuleMapper();
}

void HistoManager::Clear() {

    for (it1d it = histos1d.begin(); it!=histos1d.end(); ++it) {
        if (it->second) {
            delete (it->second);
            (it->second) = nullptr;
        }
    }

    histos1d.clear();

    for (it2d it = histos2d.begin(); it!=histos2d.end(); ++it) {
        if (it->second) {
            delete (it->second);
            (it->second) = nullptr;
        }
    }

    histos2d.clear();

    for (it3d it = histos3d.begin(); it!=histos3d.end(); ++it) {
        if (it->second) {
            delete (it->second);
            (it->second) = nullptr;
        }
    }

    histos3d.clear();

}

HistoManager::~HistoManager() { delete mmapper_;}

void HistoManager::DefineHistos(){
    std::cout << "HELLO" << std::endl;
    if (debug_ > 0) std::cout << "[HistoManager] DefineHistos" << std::endl;
    std::string h_name = "";
    std::vector<std::string> hybNames;
    mmapper_->getStrings(hybNames);
    std::cout << "mmapper defined" << std::endl;
    if (debug_ > 0) std::cout << "[HistoManager] hybrids names retrieved" << std::endl;
    for (auto hist : _h_configs.items()) {


        //Get the extension of the name to decide the histogram to create
        //i.e. _h = TH1D, _hh = TH2D, _ge = TGraphErrors, _p = TProfile ...
        if (std::string(hist.key()).find("SvtHybrids") != std::string::npos) {
            if (debug_ > 0) std::cout << "[HistoManager] Hyb size" <<  hybNames.size() << std::endl;
            for(std::vector<std::string>::iterator it = hybNames.begin(); it != hybNames.end(); ++it) {
                if (hist.value().at("type") == "h") 
                {
                    h_name = m_name+"_"+std::string(hist.value().at("prefix"))+"_"+*it+"_h";
                    histos1d[h_name] = plot1D(h_name,
                            hist.value().at("xtitle"),
                            hist.value().at("bins"),
                            hist.value().at("minX"),
                            hist.value().at("maxX"));
                            
                    std::string ytitle = hist.value().at("ytitle");
                    histos1d[h_name]->GetYaxis()->SetTitle(ytitle.c_str());
                   // std::cout << histos1d[h_name]->GetName() << std::endl;
                    histos1dNamesfromJson.push_back(h_name);
                }
                if (hist.value().at("type") == "hh") {
                    h_name = m_name+"_"+std::string(hist.value().at("prefix"))+"_"+*it+"_hh";
                    histos2d[h_name] = plot2D(h_name,
                            hist.value().at("xtitle"),
                            hist.value().at("binsX"),
                            hist.value().at("minX"),
                            hist.value().at("maxX"),
                            hist.value().at("ytitle"),
                            hist.value().at("binsY"),
                            hist.value().at("minY"),
                            hist.value().at("maxY"));
                   if (debug_ > 0) std::cout << histos2d[h_name]->GetName() << std::endl;
                    histos2dNamesfromJson.push_back(h_name);
                }  
            }
        }
        else 
        {
            h_name = m_name+"_"+hist.key();
            std::size_t found = (hist.key()).find_last_of("_");
            std::string extension = hist.key().substr(found+1);

            if (extension == "h") {
                histos1d[h_name] = plot1D(h_name,hist.value().at("xtitle"),
                        hist.value().at("bins"),
                        hist.value().at("minX"),
                        hist.value().at("maxX"));

                std::string ytitle = hist.value().at("ytitle");

                histos1d[h_name]->GetYaxis()->SetTitle(ytitle.c_str());

                if (hist.value().contains("labels")) {
                    std::vector<std::string> labels = hist.value().at("labels").get<std::vector<std::string> >();

                    if (labels.size() < hist.value().at("bins")) {
                        std::cout<<"Cannot apply labels to histogram:"<<h_name<<std::endl;
                    }
                    else {
                        for (int i = 1; i<=hist.value().at("bins");++i)
                            histos1d[h_name]->GetXaxis()->SetBinLabel(i,labels[i-1].c_str());
                    }//bins
                }//labels
            }//1D histo

            else if (extension == "hh") {
                histos2d[h_name] = plot2D(h_name,
                        hist.value().at("xtitle"),hist.value().at("binsX"),hist.value().at("minX"),hist.value().at("maxX"),
                        hist.value().at("ytitle"),hist.value().at("binsY"),hist.value().at("minY"),hist.value().at("maxY"));
            }


        }//loop on config
    }
}

void HistoManager::GetHistosFromFile(TFile* inFile, const std::vector<std::string>& name, const std::string& folder) {

    //Todo: use folder to choose a folder. 
    TIter next(inFile->GetListOfKeys());
    TKey *key;
    while ((key = (TKey*)next())) {
        std::string classType = key->GetClassName();
        std::string s(key->GetName());
    for(std::vector<std::string>::const_iterator i = name.begin(); i != name.end(); i++) {
        if (s.find(*i) == std::string::npos) continue;
        if (classType.find("TH1")!=std::string::npos) {
            histos1d[key->GetName()] = (TH1F*) key->ReadObj();
            histos1dNamesfromTFile.push_back(key->GetName());
        }
        if (classType.find("TH2")!=std::string::npos) {
            histos2d[key->GetName()] = (TH2F*) key->ReadObj();
            histos2dNamesfromTFile.push_back(key->GetName());
            std::cout << histos2d[key->GetName()]->GetName() << std::endl;
        }
        if (classType.find("TH3")!=std::string::npos)
            histos3d[key->GetName()] = (TH3F*) key->ReadObj();
    }
    }
}



TH1F* HistoManager::plot1D(const std::string& name,const std::string& xtitle, int nbinsX, float xmin, float xmax) {
    TH1F* h= new TH1F(name.c_str(),name.c_str(),nbinsX,xmin,xmax);
    h->GetXaxis()->SetTitle(xtitle.c_str());
    h->Sumw2();
    return h;
}

TH1F* HistoManager::plot1D(const std::string& name,const std::string& xtitle, int nbinsX, double* axisX) {
    TH1F* h= new TH1F(name.c_str(),name.c_str(),nbinsX,axisX);
    h->GetXaxis()->SetTitle(xtitle.c_str());
    h->Sumw2();
    return h;
}

TH2F* HistoManager::plot2D(std::string name,
        std::string xtitle, int nbinsX, float xmin, float xmax,
        std::string ytitle, int nbinsY, float ymin, float ymax) {

    TH2F* h = new TH2F(name.c_str(),name.c_str(),
            nbinsX,xmin,xmax,
            nbinsY,ymin,ymax);
    h->GetXaxis()->SetTitle(xtitle.c_str());
    h->GetYaxis()->SetTitle(ytitle.c_str());
    h->Sumw2();
    return h;
}

TH2F* HistoManager::plot2D(std::string name,
        std::string xtitle, int nbinsX, double* axisX,
        std::string ytitle, int nbinsY, double* axisY) {

    TH2F * h = new TH2F(name.c_str(),name.c_str(),
            nbinsX,axisX,
            nbinsY,axisY);
    h->GetXaxis()->SetTitle(xtitle.c_str());
    h->GetYaxis()->SetTitle(ytitle.c_str());
    h->Sumw2();
    return h;
}

TH2F* HistoManager::plot2D(std::string name,
        std::string xtitle, int nbinsX, const double* axisX,
        std::string ytitle, int nbinsY, const double* axisY) {

    TH2F * h = new TH2F(name.c_str(),name.c_str(),
            nbinsX,axisX,
            nbinsY,axisY);
    h->GetXaxis()->SetTitle(xtitle.c_str());
    h->GetYaxis()->SetTitle(ytitle.c_str());
    h->Sumw2();
    return h;
}



TH2F* HistoManager::plot2D(std::string name,
        std::string xtitle, int nbinsX, double* axisX,
        std::string ytitle, int nbinsY, float  ymin, float ymax) {

    TH2F * h = new TH2F(name.c_str(),name.c_str(),
            nbinsX,axisX,
            nbinsY,ymin,ymax);
    h->GetXaxis()->SetTitle(xtitle.c_str());
    h->GetYaxis()->SetTitle(ytitle.c_str());
    h->Sumw2();
    return h;
}


TH3F*  HistoManager::plot3D(std::string name,
        std::string xtitle, int nbinsX, double* axisX,
        std::string ytitle, int nbinsY, double* axisY,
        std::string ztitle, int nbinsZ, double* axisZ) {


    TH3F* h = new TH3F(name.c_str(),name.c_str(),
            nbinsX,axisX,
            nbinsY,axisY,
            nbinsZ,axisZ);

    h->GetXaxis()->SetTitle(xtitle.c_str());
    h->GetYaxis()->SetTitle(ytitle.c_str());
    h->GetZaxis()->SetTitle(ztitle.c_str());
    h->Sumw2();
    return h;
}




TH3F*  HistoManager::plot3D(std::string name,
        std::string xtitle, int nbinsX, float xmin, float xmax,
        std::string ytitle, int nbinsY, float ymin, float ymax,
        std::string ztitle, int nbinsZ, float zmin, float zmax) {


    TH3F* h = new TH3F(name.c_str(),name.c_str(),
            nbinsX,xmin,xmax,
            nbinsY,ymin,ymax,
            nbinsZ,zmin,zmax);

    h->GetXaxis()->SetTitle(xtitle.c_str());
    h->GetYaxis()->SetTitle(ytitle.c_str());
    h->GetZaxis()->SetTitle(ztitle.c_str());
    h->Sumw2();
    return h;
}



void HistoManager::sumw2() {

    for (it3d it = histos3d.begin(); it!=histos3d.end(); ++it) {
        if (!it->second){
            std::cout<<it->first<<" Null ptr in saving.."<<std::endl;
            continue;
        }
        it->second->Sumw2();
    }

    for (it2d it = histos2d.begin(); it!=histos2d.end(); ++it) {
        if (!(it->second)) {
            std::cout<<it->first<<" Null ptr in saving.."<<std::endl;
            continue;
        }
        it->second->Sumw2();
    }

    for (it1d it = histos1d.begin(); it!=histos1d.end(); ++it) {
        if (!it->second){
            std::cout<<it->first<<" Null ptr in saving.."<<std::endl;
            continue;
        }
        it->second->Sumw2();
    }

}

void HistoManager::Fill2DHisto(const std::string& histoName,float valuex, float valuey, float weight) {
    if (histos2d[m_name+"_"+histoName])
        histos2d[m_name+"_"+histoName]->Fill(valuex,valuey,weight);
    else
        std::cout<<"ERROR::Fill2DHisto Histogram not found! "<<m_name+"_"+histoName<<std::endl;
}


void HistoManager::Fill1DHisto(const std::string& histoName,float value, float weight) {
    if (histos1d[m_name+"_"+histoName])
        histos1d[m_name+"_"+histoName]->Fill(value,weight);
    else
        std::cout<<"ERROR::Fill1DHisto Histogram not found! "<<m_name+"_"+histoName<<std::endl;
}


void HistoManager::loadHistoConfig(const std::string histoConfigFile) {

    std::ifstream i_file(histoConfigFile);
    i_file >> _h_configs;
    if (debug_) {
        for (auto& el : _h_configs.items()) 
            std::cout << el.key() << " : " << el.value() << "\n";
    }

}




void HistoManager::saveHistos(TFile* outF,std::string folder) {

    if (outF) outF->cd();
    TDirectory* dir{nullptr};

    if (!folder.empty()) {
        dir = outF->mkdir(folder.c_str());
        dir->cd();
    }

    for (it3d it = histos3d.begin(); it!=histos3d.end(); ++it) {
        if (!it->second){
            std::cout<<it->first<<" Null ptr in saving.."<<std::endl;
            continue;
        }
        it->second->Write();
    }

    for (it2d it = histos2d.begin(); it!=histos2d.end(); ++it) {
        if (!(it->second)) {
            std::cout<<it->first<<" Null ptr in saving.."<<std::endl;
            continue;
        }
        it->second->Write();
    }

    for (it1d it = histos1d.begin(); it!=histos1d.end(); ++it) {
        if (!it->second){
            std::cout<<it->first<<" Null ptr in saving.."<<std::endl;
            continue;
        }
        it->second->Write();
    }

    //dir->Write();
    //if (dir) {delete dir; dir=0;}

    Clear();

}


