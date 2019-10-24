#include "HistoManager.h"
#include <iostream>
#include "TKey.h"
#include "TClass.h"

HistoManager::HistoManager() {
    HistoManager("default");
}

HistoManager::HistoManager(const std::string& inputName) {
    m_name = inputName;
}

HistoManager::~HistoManager() {

    std::cout<<"Cleaning up HistoManager"<<std::endl;

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


void HistoManager::GetHistosFromFile(TFile* inFile, const std::string& name, const std::string& folder) {

    //Todo: use name as regular expression.
    //Todo: use folder to choose a folder. 
    TIter next(inFile->GetListOfKeys());
    TKey *key;
    while ((key = (TKey*)next())) {
        std::string classType = key->GetClassName();
        if (classType.find("TH1")!=std::string::npos)
            histos1d[key->GetName()] = (TH1F*) key->ReadObj();
        if (classType.find("TH2")!=std::string::npos)
            histos2d[key->GetName()] = (TH2F*) key->ReadObj();
        if (classType.find("TH3")!=std::string::npos)
            histos3d[key->GetName()] = (TH3F*) key->ReadObj();
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



void HistoManager::saveHistos(TFile* outF,std::string folder) {

    if (outF)
        outF->cd();
    TDirectoryFile* dir = 0;

    if (!folder.empty()) {
        dir = new TDirectoryFile(folder.c_str(),folder.c_str());
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

    if (dir) {delete dir; dir=0;}

}


