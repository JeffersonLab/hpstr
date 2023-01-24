#include "SimpEquations.h"
#include <cmath>

SimpEquations::SimpEquations(){

}

double SimpEquations::rate_2pi(double m_Ap, double m_pi, double m_V, double alpha_D){
    double coeff = (2.0*alpha_D/3.0) * m_Ap;
    double pow1 = std::pow((1-(4*m_pi*m_pi/(m_Ap*m_Ap))),3/2.);
    double pow2 = std::pow(((m_V*m_V)/((m_Ap*m_Ap)-(m_V*m_V))),2);
    return coeff*pow1*pow2;
}

double SimpEquations::rate_Vpi(double m_Ap, double m_pi, double m_V, double alpha_D, double f_pi, bool rho, bool phi){
    double x = m_pi/m_Ap;
    double y = m_V/m_Ap;
    double coeff = alpha_D*Tv(rho, phi)/(192.*std::pow(M_PI,4));
    return coeff * std::pow((m_Ap/m_pi),2) * std::pow(m_V/m_pi,2) * std::pow((m_pi/f_pi),4) * m_Ap*std::pow(Beta(x,y),3./2.);
}

double SimpEquations::br_Vpi(double m_Ap, double m_pi, double m_V, double alpha_D, double f_pi, bool rho, bool phi){
    double rate = rate_Vpi(m_Ap,m_pi,m_V,alpha_D,f_pi,rho,phi) + rate_2pi(m_Ap,m_pi,m_V,alpha_D);
    if(2*m_V < m_Ap) rate = rate_Vpi(m_Ap, m_pi, m_V, alpha_D, f_pi, rho,phi) + rate_2pi(m_Ap,m_pi,m_V,alpha_D) + rate_2V(m_Ap,m_V,alpha_D);
    return rate_Vpi(m_Ap,m_pi,m_V,alpha_D,f_pi,rho,phi)/rate;
}

double SimpEquations::br_2V(double m_Ap,double m_pi,double m_V,double alpha_D,double f_pi,double rho,double phi){
    if(2*m_V >= m_Ap) return 0.;
    double rate = rate_Vpi(m_Ap,m_pi,m_V,alpha_D,f_pi,rho,phi) + rate_2pi(m_Ap,m_pi,m_V,alpha_D) + rate_2V(m_Ap,m_V,alpha_D);
    return rate_2V(m_Ap,m_V,alpha_D)/rate;
}

double SimpEquations::Tv(bool rho,bool phi){
    if(rho) return 3./4.;
    else if(phi) return 3./2.;
    else return 18.;
}

double SimpEquations::Beta(double x,double y){
    return (1+std::pow(y,2)-std::pow(x,2)-2*y)*(1+std::pow(y,2)-std::pow(x,2)+2*y);
}

double SimpEquations::rate_2V(double m_Ap,double m_V,double alpha_D){
    double r = m_V/m_Ap;
    return alpha_D/6. * m_Ap * f(r);
}

double SimpEquations::f(double r){
    double num = 1 + 16*std::pow(r,2) - 68*std::pow(r,4) - 48*std::pow(r,6);
    double den = std::pow(1-std::pow(r,2),2);
    return num/den * std::pow((1-4*std::pow(r,2)),0.5);
}

double SimpEquations::rate_2l(double m_Ap,double m_pi,double m_V,double eps,double alpha_D,double f_pi,double m_l,bool rho){
    double alpha = 1./137.0;
    double coeff = 16*M_PI*alpha_D*alpha*std::pow(eps,2)*std::pow(f_pi,2)/(3*std::pow(m_V,2));
    double term1 = std::pow((std::pow(m_V,2)/(std::pow(m_Ap,2) - std::pow(m_V,2))),2);
    double term2 = std::pow((1-(4*std::pow(m_l,2)/std::pow(m_V,2))),0.5);
    double term3 = 1+(2*std::pow(m_l,2)/std::pow(m_V,2));
    int constant = 1;
    if(rho) constant = 2;
    return coeff * term1 * term2 * term3 * m_V * constant;
}

double SimpEquations::getCtau(double m_Ap,double m_pi,double m_V,double eps,double alpha_D,double f_pi,double m_l,bool rho){
    double c = 3.00e10; //cm/s
    double hbar = 6.58e-22; //MeV*sec
    double rate = rate_2l(m_Ap,m_pi,m_V,eps,alpha_D,f_pi,m_l,rho); //MeV
    double tau = hbar/rate;
    double ctau = c*tau;
    return ctau;
}

double SimpEquations::gamma(double m_V,double E_V){
    double gamma = E_V/m_V;
    return gamma;
}

