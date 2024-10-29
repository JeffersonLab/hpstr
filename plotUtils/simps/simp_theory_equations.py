import math
import ROOT as r
import numpy as np
class SimpEquations:

    def __init__(self, year = 2016, alpha_dark = 0.01, mass_ratio_Ap_to_Vd = 1.66, mass_ratio_Ap_to_Pid = 3.0, 
            ratio_mPi_to_fPi = 12.566, lepton_mass = 0.511):
        self.year = year
        self.alpha_dark = alpha_dark
        self.mass_ratio_Ap_to_Vd = mass_ratio_Ap_to_Vd
        self.mass_ratio_Ap_to_Pid = mass_ratio_Ap_to_Pid
        self.ratio_mPi_to_fPi = ratio_mPi_to_fPi
        self.lepton_mass = lepton_mass

    @staticmethod
    def rate_Ap_ee(m_Ap, eps):
        ml = 0.511
        r = ml/m_Ap
        coeff1 = ((1.0/137.0)*eps**2)/3.0
        coeff2 = (1.0 - 4.0*(r**2))**(0.5)
        coeff3 = (1.0 + 2.0*(r**2))*m_Ap
        return coeff1*coeff2*coeff3

    @staticmethod
    def rate_2pi(m_Ap, m_pi, m_V, alpha_dark):
        coeff = (2.0 * alpha_dark / 3.0) * m_Ap
        pow1 = np.power((1 - (4 * m_pi * m_pi / (m_Ap * m_Ap))), 3 / 2.0)
        pow2 = np.power(((m_V * m_V) / ((m_Ap * m_Ap) - (m_V * m_V))), 2)
        return coeff * pow1 * pow2

    @staticmethod
    def rate_Vrho_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi):
        x = m_pi / m_Ap
        y = m_V / m_Ap
        Tv = 3.0/4.0
        coeff = alpha_dark * Tv / (192.0 * np.power(math.pi, 4))
        return coeff * np.power((m_Ap / m_pi), 2) * np.power(m_V / m_pi, 2) * np.power((m_pi / f_pi), 4) * m_Ap * np.power(SimpEquations.Beta(x, y), 3 / 2.0)

    def rate_Vphi_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi):
        x = m_pi / m_Ap
        y = m_V / m_Ap
        Tv = 3.0/2.0
        coeff = alpha_dark * Tv / (192.0 * np.power(math.pi, 4))
        return coeff * np.power((m_Ap / m_pi), 2) * np.power(m_V / m_pi, 2) * np.power((m_pi / f_pi), 4) * m_Ap * np.power(SimpEquations.Beta(x, y), 3 / 2.0)

    def rate_Vcharged_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi):
        x = m_pi / m_Ap
        y = m_V / m_Ap
        Tv = 18.0 - ((3.0/2.0)+(3.0/4.0))
        coeff = alpha_dark * Tv / (192.0 * np.power(math.pi, 4))
        return coeff * np.power((m_Ap / m_pi), 2) * np.power(m_V / m_pi, 2) * np.power((m_pi / f_pi), 4) * m_Ap * np.power(SimpEquations.Beta(x, y), 3 / 2.0)

    @staticmethod
    def br_2pi(m_Ap, m_pi, m_V, alpha_dark, f_pi):
        total_rate = (SimpEquations.rate_Vrho_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi) + 
                SimpEquations.rate_Vphi_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi) + 
                SimpEquations.rate_Vcharged_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi) + 
                SimpEquations.rate_2pi(m_Ap, m_pi, m_V, alpha_dark))

        # Add the rate_2V contribution element-wise where m_Ap > 2.0 * m_V
        total_rate += np.where(m_Ap > 2.0 * m_V, SimpEquations.rate_2V(m_Ap, m_V, alpha_dark), 0.0)

        return SimpEquations.rate_2pi(m_Ap, m_pi, m_V, alpha_dark) / total_rate

    @staticmethod
    def br_Vrho_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi):
        total_rate = (SimpEquations.rate_Vrho_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi) + 
                SimpEquations.rate_Vphi_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi) + 
                SimpEquations.rate_Vcharged_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi) + 
                SimpEquations.rate_2pi(m_Ap, m_pi, m_V, alpha_dark))

        # Add the rate_2V contribution element-wise where m_Ap > 2.0 * m_V
        total_rate += np.where(m_Ap > 2.0 * m_V, SimpEquations.rate_2V(m_Ap, m_V, alpha_dark), 0.0)

        return SimpEquations.rate_Vrho_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi) / total_rate

    @staticmethod
    def br_Vphi_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi):
        total_rate = (SimpEquations.rate_Vrho_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi) + 
                SimpEquations.rate_Vphi_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi) + 
                SimpEquations.rate_Vcharged_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi) + 
                SimpEquations.rate_2pi(m_Ap, m_pi, m_V, alpha_dark))

        # Add the rate_2V contribution element-wise where m_Ap > 2.0 * m_V
        total_rate += np.where(m_Ap > 2.0 * m_V, SimpEquations.rate_2V(m_Ap, m_V, alpha_dark), 0.0)

        return SimpEquations.rate_Vphi_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi) / total_rate

    @staticmethod
    def br_Vcharged_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi):
        total_rate = (SimpEquations.rate_Vrho_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi) + 
                SimpEquations.rate_Vphi_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi) + 
                SimpEquations.rate_Vcharged_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi) + 
                SimpEquations.rate_2pi(m_Ap, m_pi, m_V, alpha_dark))

        # Add the rate_2V contribution element-wise where m_Ap > 2.0 * m_V
        total_rate += np.where(m_Ap > 2.0 * m_V, SimpEquations.rate_2V(m_Ap, m_V, alpha_dark), 0.0)

        return SimpEquations.rate_Vcharged_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi) / total_rate

    @staticmethod
    def br_2V(m_Ap, m_pi, m_V, alpha_dark, f_pi):
        total_rate = (SimpEquations.rate_Vrho_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi) + 
                SimpEquations.rate_Vphi_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi) + 
                SimpEquations.rate_Vcharged_pi(m_Ap, m_pi, m_V, alpha_dark, f_pi) + 
                SimpEquations.rate_2pi(m_Ap, m_pi, m_V, alpha_dark))

        # Calculate the rate_2V contribution only where m_Ap > 2.0 * m_V and add it to total_rate
        total_rate += np.where(m_Ap > 2.0 * m_V, SimpEquations.rate_2V(m_Ap, m_V, alpha_dark), 0.0)

        # Return 0.0 where 2 * m_V >= m_Ap
        rate_2V_contrib = np.where((2.0 * m_V >= m_Ap) | (m_Ap <= 2.9 * m_V), 0.0, SimpEquations.rate_2V(m_Ap, m_V, alpha_dark))

        return rate_2V_contrib / total_rate

    @staticmethod
    def Tv(rho, phi):
        if rho:
            return 3.0 / 4.0
        elif phi:
            return 3.0 / 2.0
        else:
            return 18.0

    @staticmethod
    def Beta(x, y):
        return (1 + np.power(y, 2) - np.power(x, 2) - 2 * y) * (1 + np.power(y, 2) - np.power(x, 2) + 2 * y)

    @staticmethod
    def rate_2V(m_Ap, m_V, alpha_dark):
        r = m_V / m_Ap
        return alpha_dark / 6.0 * m_Ap * SimpEquations.f(r)

    @staticmethod
    def f(r):
        # Define your function f(r) here
        # Example: return some_expression
        return -1.

    @staticmethod
    def rate_2l(m_Ap, m_pi, m_V, eps, alpha_dark, f_pi, m_l, rho):
        alpha = 1.0 / 137.0
        coeff = (16 * math.pi * alpha_dark * alpha * eps**2 * f_pi**2) / (3 * m_V**2)
        term1 = (m_V**2 / (m_Ap**2 - m_V**2))**2
        term2 = (1 - (4 * m_l**2 / m_V**2))**0.5
        term3 = 1 + (2 * m_l**2 / m_V**2)
        constant = 1 if not rho else 2
        return coeff * term1 * term2 * term3 * m_V * constant

    @staticmethod
    def getCtau(m_Ap, m_pi, m_V, eps, alpha_dark, f_pi, m_l, rho):
        #c = 3.00e10  # cm/s <! -- BUG
        c = 3.00e11 #mm/s
        hbar = 6.58e-22  # MeV*sec
        rate = SimpEquations.rate_2l(m_Ap, m_pi, m_V, eps, alpha_dark, f_pi, m_l, rho)  # MeV
        tau = hbar / rate
        ctau = c * tau
        return ctau

    @staticmethod
    def gamma(m_V, E_V):
        gamma = E_V / m_V
        return gamma

    @staticmethod
    def totalApProductionRate(m_Ap, eps, radFrac, radAcc, dNdm):
        # Total A' Production Rate
        apProduction = (3. * 137 / 2.) * 3.14159 * (m_Ap * eps * eps * radFrac * dNdm)/radAcc
        return apProduction



