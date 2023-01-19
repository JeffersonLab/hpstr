#include <iostream>

class SimpEquations {

    public:
            
        double pi = 3.14159;

        SimpEquations();

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
};
