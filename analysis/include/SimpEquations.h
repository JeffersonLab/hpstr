#include <iostream>
#include <TEfficiency.h>
#include <TMath.h>
#include <TH1F.h>
#include "json.hpp"

using json = nlohmann::json;

class SimpEquations {

    public:
            

        SimpEquations(int year);

        SimpEquations(int year, const std::string paramsConfigFile);

        void loadParametersConfig(const std::string paramsConfigFile);

        double rate_2pi(double m_Ap, double m_pi, double m_V, double alpha_D);
        
        double rate_Vpi(double m_Ap, double m_pi, double m_V, double alpha_D, double f_pi, bool rho, bool phi);
        
        double br_Vpi(double m_Ap, double m_pi, double m_V, double alpha_D, double f_pi, bool rho, bool phi);
        
        double br_2V(double m_Ap,double m_pi,double m_V,double alpha_D,double f_pi,double rho,double phi);
        
        double Tv(bool rho,bool phi);
        
        double Beta(double x,double y);
        
        double rate_2V(double m_Ap,double m_V,double alpha_D);
        
        double f(double r);
        
        double rate_2l(double m_Ap,double m_pi,double m_V,double eps,double alpha_D,double f_pi,double m_l,bool rho);
        
        double getCtau(double m_Ap,double m_pi,double m_V,double eps,double alpha_D,double f_pi,double m_l,bool rho);
        
        double gamma(double m_V,double E_V);

        double radiativeFraction(double m_Ap);

        double radiativeAcceptance(double m_Ap);

        double massResolution(double m_V);

        double controlRegionBackgroundRate(double m_Ap);

        double expectedSignalCalculation(double m_V, double eps, bool rho, bool phi,
                        double E_V, TEfficiency* effCalc_h, double target_pos, double zcut);

        double expectedSignalCalculation(double m_Ap, double m_pi, double m_V, double eps, double alpha_D,
            double f_pi, double m_l, bool rho, bool phi, double E_V, TEfficiency* effCalc_h, double target_pos, 
            double zcut);

    private:

        int year_ = 2016;
        json params_config_;
        double mass_ratio_Ap_to_Pid_ = 3.0;
        double mass_ratio_Ap_to_Vd_ = 3.0/1.8;
        double ratio_mPi_to_fPi_ = 4.*M_PI;
        double m_l_ = 0.511;
        double alpha_D_ = 0.01;

};
