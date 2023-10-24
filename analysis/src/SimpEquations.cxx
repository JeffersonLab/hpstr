#include "SimpEquations.h"
#include <fstream>
#include <iostream>
#include <cmath>

SimpEquations::SimpEquations(int year){
    year_ = year;

    std::cout << "[SimpEquations]::Parameters " << std::endl;
    std::cout << "mass ratio Ap:Pi_D = " << mass_ratio_Ap_to_Pid_ << std::endl;
    std::cout << "mass ratio Ap:V_D = " << mass_ratio_Ap_to_Vd_ << std::endl;
    std::cout << "ratio of dark pion mass to dark pion decay constant: " << ratio_mPi_to_fPi_ << std::endl;
    std::cout << "Dark Vector decay lepton mass: " << m_l_ << std::endl;
    std::cout << "Alpha Dark: " << alpha_dark_ << std::endl;

}

SimpEquations::SimpEquations(int year, const std::string paramsConfigFile){
    year_ = year;
    loadParametersConfig(paramsConfigFile);

    std::cout << "[SimpEquations]::Parameters Loaded From Json Config " << std::endl;
    std::cout << "mass ratio Ap:Pi_D = " << mass_ratio_Ap_to_Pid_ << std::endl;
    std::cout << "mass ratio Ap:V_D = " << mass_ratio_Ap_to_Vd_ << std::endl;
    std::cout << "ratio of dark pion mass to dark pion decay constant: " << ratio_mPi_to_fPi_ << std::endl;
    std::cout << "Dark Vector decay lepton mass: " << m_l_ << std::endl;
    std::cout << "Alpha Dark: " << alpha_dark_ << std::endl;

}

void SimpEquations::loadParametersConfig(const std::string paramsConfigFile){

    //Read SIMP Model parameters from json
    std::ifstream i_file(paramsConfigFile);
    i_file >> params_config_;
    for(auto& el : params_config_.items())
        std::cout << el.key() << " : " << el.value() << "\n";
    i_file.close();

    //Set parameter values. All parameters must be specificed in json config
    try
    {
        for(auto param : params_config_.items()){
            mass_ratio_Ap_to_Vd_ = param.value().at("mass_ratio_Ap_to_Vd");
            mass_ratio_Ap_to_Pid_ = param.value().at("mass_ratio_Ap_to_Pid");
            ratio_mPi_to_fPi_ = param.value().at("ratio_mPi_to_fPi");
            m_l_ = param.value().at("lepton_mass");
            alpha_dark_ = param.value().at("alpha_dark");
        }
    }
    catch(...)
    {
        std::cout << "[SimpEquations]::ERROR::REQUIRED PARAMETER CONFIGURATIONS NOT FOUND IN " 
            << paramsConfigFile << std::endl;
    }
}

double SimpEquations::rate_2pi(double m_Ap, double m_pi, double m_V, double alpha_dark){
    double coeff = (2.0*alpha_dark/3.0) * m_Ap;
    double pow1 = std::pow((1-(4*m_pi*m_pi/(m_Ap*m_Ap))),3/2.);
    double pow2 = std::pow(((m_V*m_V)/((m_Ap*m_Ap)-(m_V*m_V))),2);
    return coeff*pow1*pow2;
}

double SimpEquations::rate_Vrho_pi(double m_Ap, double m_pi, double m_V, 
        double alpha_dark, double f_pi){
    double x = m_pi / m_Ap;
    double y = m_V / m_Ap;
    double Tv = 3.0/4.0;
    double coeff = alpha_dark*Tv/(192.*std::pow(M_PI,4));
    return coeff * std::pow((m_Ap/m_pi),2) * std::pow(m_V/m_pi,2) * std::pow((m_pi/f_pi),4) * m_Ap*std::pow(Beta(x,y),3./2.);
}

double SimpEquations::rate_Vphi_pi(double m_Ap, double m_pi, double m_V, 
        double alpha_dark, double f_pi){
    double x = m_pi / m_Ap;
    double y = m_V / m_Ap;
    double Tv = 3.0/2.0;
    double coeff = alpha_dark*Tv/(192.*std::pow(M_PI,4));
    return coeff * std::pow((m_Ap/m_pi),2) * std::pow(m_V/m_pi,2) * std::pow((m_pi/f_pi),4) * m_Ap*std::pow(Beta(x,y),3./2.);
}
double SimpEquations::rate_Vcharged_pi(double m_Ap, double m_pi, double m_V, 
        double alpha_dark, double f_pi){
    double x = m_pi / m_Ap;
    double y = m_V / m_Ap;
    double Tv = 18.0;
    double coeff = alpha_dark*Tv/(192.*std::pow(M_PI,4));
    return coeff * std::pow((m_Ap/m_pi),2) * std::pow(m_V/m_pi,2) * std::pow((m_pi/f_pi),4) * m_Ap*std::pow(Beta(x,y),3./2.);
}

double SimpEquations::br_2pi(double m_Ap, double m_pi, double m_V, double alpha_dark, 
        double f_pi){
    double total_rate = rate_Vrho_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi) +
        rate_Vphi_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi) +
        rate_Vcharged_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi) +
        rate_2pi(m_Ap, m_pi, m_V, alpha_dark);
    if(m_Ap > 2.0*m_V)
        total_rate + rate_2V(m_Ap, m_V, alpha_dark);
    return rate_2pi(m_Ap, m_pi, m_V, alpha_dark)/total_rate;
}

double SimpEquations::br_Vrho_pi(double m_Ap, double m_pi, double m_V, double alpha_dark, 
        double f_pi){
    double total_rate = rate_Vrho_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi) +
        rate_Vphi_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi) +
        rate_Vcharged_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi) +
        rate_2pi(m_Ap, m_pi, m_V, alpha_dark);
    if(m_Ap > 2.0*m_V)
        total_rate + rate_2V(m_Ap, m_V, alpha_dark);
    return rate_Vrho_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi)/total_rate;
}

double SimpEquations::br_Vphi_pi(double m_Ap, double m_pi, double m_V, double alpha_dark, 
        double f_pi){
    double total_rate = rate_Vrho_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi) +
        rate_Vphi_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi) +
        rate_Vcharged_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi) +
        rate_2pi(m_Ap, m_pi, m_V, alpha_dark);
    if(m_Ap > 2.0*m_V)
        total_rate + rate_2V(m_Ap, m_V, alpha_dark);
    return rate_Vphi_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi)/total_rate;
}

double SimpEquations::br_Vcharged_pi(double m_Ap, double m_pi, double m_V, 
        double alpha_dark, double f_pi){
    double total_rate = rate_Vrho_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi) +
        rate_Vphi_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi) +
        rate_Vcharged_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi) +
        rate_2pi(m_Ap, m_pi, m_V, alpha_dark);
    if(m_Ap > 2.0*m_V)
        total_rate + rate_2V(m_Ap, m_V, alpha_dark);
    return rate_Vcharged_pi(m_Ap, m_pi, m_V, alpha_dark,f_pi)/total_rate;
}

double SimpEquations::br_2V(double m_Ap,double m_pi,double m_V,double alpha_dark,double f_pi,double rho,double phi){
    double total_rate = rate_Vrho_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi) +
        rate_Vphi_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi) +
        rate_Vcharged_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi) +
        rate_2pi(m_Ap, m_pi, m_V, alpha_dark);
    if(m_Ap > 2.0*m_V){
        total_rate + rate_2V(m_Ap, m_V, alpha_dark);
        return 0.0;
    }
    return rate_2V(m_Ap,m_V,alpha_dark)/total_rate;
}

double SimpEquations::Tv(bool rho,bool phi){
    if(rho) return 3./4.;
    else if(phi) return 3./2.;
    else return 18.;
}

double SimpEquations::Beta(double x,double y){
    return (1+std::pow(y,2)-std::pow(x,2)-2*y)*(1+std::pow(y,2)-std::pow(x,2)+2*y);
}

double SimpEquations::rate_2V(double m_Ap,double m_V,double alpha_dark){
    double r = m_V/m_Ap;
    return alpha_dark/6. * m_Ap * f(r);
}

double SimpEquations::f(double r){
    double num = 1 + 16*std::pow(r,2) - 68*std::pow(r,4) - 48*std::pow(r,6);
    double den = std::pow(1-std::pow(r,2),2);
    return num/den * std::pow((1-4*std::pow(r,2)),0.5);
}

double SimpEquations::rate_2l(double m_Ap,double m_pi,double m_V,double eps,double alpha_dark,double f_pi,double m_l,bool rho){
    double alpha = 1./137.0;
    double coeff = 16*M_PI*alpha_dark*alpha*std::pow(eps,2)*std::pow(f_pi,2)/(3*std::pow(m_V,2));
    double term1 = std::pow((std::pow(m_V,2)/(std::pow(m_Ap,2) - std::pow(m_V,2))),2);
    double term2 = std::pow((1-(4*std::pow(m_l,2)/std::pow(m_V,2))),0.5);
    double term3 = 1+(2*std::pow(m_l,2)/std::pow(m_V,2));
    int constant = 1;
    if(rho) constant = 2;
    return coeff * term1 * term2 * term3 * m_V * constant;
}

double SimpEquations::getCtau(double m_Ap,double m_pi,double m_V,double eps,double alpha_dark,double f_pi,double m_l,bool rho){
    double c = 3.00e11; //mm/s
    double hbar = 6.58e-22; //MeV*sec
    double rate = rate_2l(m_Ap,m_pi,m_V,eps,alpha_dark,f_pi,m_l,rho); //MeV
    double tau = hbar/rate;
    double ctau = c*tau;
    return ctau;
}

double SimpEquations::gamma(double m_V,double E_V){
    double gamma = E_V/m_V;
    return gamma;
}

//Calculate expected signal by passing radFrac, radAcc, and dNdm
double SimpEquations::expectedSignalCalculation(double m_V, double eps, bool rho, 
        double E_V, TEfficiency* effCalc_h, double dNdm, double radFrac, double radAcc, double target_pos, double zcut){

    //Signal mass dependent SIMP parameters
    double m_Ap = m_V*(mass_ratio_Ap_to_Vd_);
    double m_pi = m_Ap/mass_ratio_Ap_to_Pid_;
    double f_pi = m_pi/ratio_mPi_to_fPi_;

    //Mass in MeV
    double ctau = getCtau(m_Ap,m_pi, m_V,eps,alpha_dark_,f_pi,m_l_,rho);
    double gcTau = ctau * gamma(m_V/1000.0, E_V); //E_V in GeV
    std::cout << "gcTau: " << gcTau << std::endl;
    std::cout << "radFrac: " << radFrac << std::endl;
    std::cout << "radAcc: " << radAcc << std::endl;
    std::cout << "dNdm:" << dNdm << std::endl;

    //Calculate the Efficiency Vertex (Displaced VD Acceptance)
    double effVtx = 0.0;
    for(int zbin = 0; zbin < (effCalc_h->GetTotalHistogram()->GetNbinsX())+1; zbin++){
        double zz = effCalc_h->GetTotalHistogram()->GetBinLowEdge(zbin);
        if(zz < zcut) continue;
        effVtx += (TMath::Exp((target_pos-zz)/gcTau)/gcTau)*
            (effCalc_h->GetEfficiency(zbin) - effCalc_h->GetEfficiencyErrorLow(zbin))*
            effCalc_h->GetTotalHistogram()->GetBinWidth(zbin);
    }

    //Total A' Production Rate
    double apProduction = (3.*137/2.)*3.14159*(m_Ap*eps*eps*radFrac*dNdm)/radAcc;

    //A' -> V+Pi Branching Ratio
    double br_VPi = 0.0;
    if(rho)
        br_VPi = br_Vrho_pi(m_Ap, m_pi, m_V, alpha_dark_, f_pi);
    else
        br_VPi = br_Vphi_pi(m_Ap, m_pi, m_V, alpha_dark_, f_pi);

    std::cout << "Branching ratio is " << br_VPi << std::endl;
    //Vector to e+e- BR = 1
    double br_V_ee = 1.0;

    //Expected Signal
    double expSignal = apProduction * effVtx * br_VPi * br_V_ee;

    return expSignal;
}

double SimpEquations::expectedSignalCalculation(double m_V, double eps, bool rho, 
        double E_V, TEfficiency* effCalc_h, double target_pos, double zcut){

    //Signal mass dependent SIMP parameters
    double m_Ap = m_V*(mass_ratio_Ap_to_Vd_);
    double m_pi = m_Ap/mass_ratio_Ap_to_Pid_;
    double f_pi = m_pi/ratio_mPi_to_fPi_;

    //Mass in MeV
    double ctau = getCtau(m_Ap,m_pi, m_V,eps,alpha_dark_,f_pi,m_l_,rho);
    double gcTau = ctau * gamma(m_V/1000.0, E_V); //E_V in GeV

    //Calculate the Efficiency Vertex (Displaced VD Acceptance)
    double effVtx = 0.0;
    for(int zbin = 0; zbin < (effCalc_h->GetTotalHistogram()->GetNbinsX())+1; zbin++){
        double zz = effCalc_h->GetTotalHistogram()->GetBinLowEdge(zbin);
        if(zz < zcut) continue;
        effVtx += (TMath::Exp((target_pos-zz)/gcTau)/gcTau)*
            (effCalc_h->GetEfficiency(zbin) - effCalc_h->GetEfficiencyErrorLow(zbin))*
            effCalc_h->GetTotalHistogram()->GetBinWidth(zbin);
    }


    //Total A' Production Rate
    double apProduction = (3.*137/2.)*3.14159*(m_Ap*eps*eps*radiativeFraction(m_Ap)*controlRegionBackgroundRate(m_Ap))
        /radiativeAcceptance(m_Ap);

    //A' -> V+Pi Branching Ratio
    double br_VPi;
    if(rho)
        br_VPi = br_Vrho_pi(m_Ap, m_pi, m_V, alpha_dark_, f_pi);
    else
        br_VPi = br_Vphi_pi(m_Ap, m_pi, m_V, alpha_dark_, f_pi);

    //Vector to e+e- BR = 1
    double br_V_ee = 1.0;

    //Expected Signal
    double expSignal = apProduction * effVtx * br_VPi * br_V_ee;

    return expSignal;
}

double SimpEquations::expectedSignalCalculation(double m_Ap, double m_pi, double m_V, double eps, double alpha_dark, 
        double f_pi, double m_l, bool rho, double E_V, TEfficiency* effCalc_h, double target_pos, double zcut){

    //Mass in MeV
    double ctau = getCtau(m_Ap,m_pi, m_V,eps,alpha_dark,f_pi,m_l,rho);
    double gcTau = ctau * gamma(m_V/1000.0, E_V); //E_V in GeV

    //Calculate the Efficiency Vertex (Displaced VD Acceptance)
    double effVtx = 0.0;
    for(int zbin = 0; zbin < (effCalc_h->GetTotalHistogram()->GetNbinsX())+1; zbin++){
        double zz = effCalc_h->GetTotalHistogram()->GetBinLowEdge(zbin);
        if(zz < zcut) continue;
        effVtx += (TMath::Exp((target_pos-zz)/gcTau)/gcTau)*
            (effCalc_h->GetEfficiency(zbin) - effCalc_h->GetEfficiencyErrorLow(zbin))*
            effCalc_h->GetTotalHistogram()->GetBinWidth(zbin);
    }


    //Total A' Production Rate
    double apProduction = (3.*137/2.)*3.14159*(m_Ap*eps*eps*radiativeFraction(m_Ap)*controlRegionBackgroundRate(m_Ap))
        /radiativeAcceptance(m_Ap);

    //A' -> V+Pi Branching Ratio
    double br_VPi;
    if(rho)
        br_VPi = br_Vrho_pi(m_Ap, m_pi, m_V, alpha_dark_, f_pi);
    else
        br_VPi = br_Vphi_pi(m_Ap, m_pi, m_V, alpha_dark_, f_pi);

    //Vector to e+e- BR = 1
    double br_V_ee = 1.0;

    //Expected Signal
    double expSignal = apProduction * effVtx * br_VPi * br_V_ee;

    return expSignal;
}

double SimpEquations::radiativeFraction(double m_Ap){
    double radFrac = 0.0;
    if(year_ == 2016){
        radFrac = ( -1.92497e-01 + 1.47144e-02*m_Ap + -2.91966e-04*std::pow(m_Ap,2) + 2.65603e-06*std::pow(m_Ap,3) + -1.12471e-8*std::pow(m_Ap,4) + 1.74765e-11*std::pow(m_Ap,5) + 2.235718e-15*std::pow(m_Ap,6)); //alic 2016 simp reach estimate
    }

    return radFrac;
}

double SimpEquations::radiativeAcceptance(double m_Ap){
    double radAcc = 0.0;
    if(year_ == 2016){
        radAcc = ( -7.93151e-01 + 1.04324e-01*m_Ap + -5.55225e-03*std::pow(m_Ap,2) + 1.55480e-04*std::pow(m_Ap,3) + -2.53281e-06*std::pow(m_Ap,4) + 2.54558e-08*std::pow(m_Ap,5) + -1.60877e-10*std::pow(m_Ap,6) + 6.24627e-13*std::pow(m_Ap,7) + -1.36375e-15*std::pow(m_Ap,8) + 1.28312e-18*std::pow(m_Ap,9) ); //alic 2016 simp reach est
    }

    return radAcc;
}

double SimpEquations::massResolution(double m_V){
    double massRes = 0.0;
    if(year_ == 2016){
        massRes = 9.73217e-01 + 3.63659e-02*m_V + -7.32046e-05*m_V*m_V; //2016 simps alic
    }

    return massRes;
}

double SimpEquations::controlRegionBackgroundRate(double m_Ap){
    double dNdm = 0.0;
    m_Ap = m_Ap/1000.0;
    /*
    if(year_ == 2016){
        std::cout << "[SimpEquations]::WARNING! BACKGROUND NOT DEFINED FOR MASS SPECTRUM!" << std::endl;
        dNdm = 513800.0;
    }*/

    if(year_ == 2016){
        std::cout << "[SimpEquations]::WARNING! USING TEMPORARY BACKGROUND MODEL" << std::endl;
        //dNdm = 100000 + -1.02011e07*std::pow(m_Ap,1) + 4.02136e08*std::pow(m_Ap,2) + -8.02288e09*std::pow(m_Ap,3) 
        //   + 9.16485e10*std::pow(m_Ap,4) + -6.2886e11*std::pow(m_Ap,5) + 2.57095e12*std::pow(m_Ap,6) 
        //    + -5.78477e12*std::pow(m_Ap,7) + 5.52322e12*std::pow(m_Ap, 8);
        dNdm = 204656 + -2.08037e7*std::pow(m_Ap,1) + 8.17706e8*std::pow(m_Ap,2) + -1.62741e10*std::pow(m_Ap,3) 
           + 1.85484e11*std::pow(m_Ap,4) + -1.26987e12*std::pow(m_Ap,5) + 5.17974e12*std::pow(m_Ap,6) 
            + -1.16277e13*std::pow(m_Ap,7) + 1.10758e13*std::pow(m_Ap, 8);
    }

    std::cout << "LOOK: Background Rate taken from Control Region InvMass is " << dNdm << std::endl;
    return dNdm;
}


