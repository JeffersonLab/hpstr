#ifndef FITFUNCTION_H
#define FITFUNCTION_H
#include <TMath.h>
#include "FunctionMath.h"
#include <iostream>

class FitFunction {
    public:
        enum SignalFitModel {
            NONE         = 0,
            GAUSSIAN     = 1,
            CRYSTAL_BALL = 2
        };

        enum ModelOrder {
            FIRST   = 0,
            THIRD   = 1,
            FIFTH   = 2,
            SEVENTH = 3
        };

        enum BkgModel {
            CHEBYSHEV     = 0,
            EXP_CHEBYSHEV = 1,
            LEGENDRE      = 2,
            EXP_LEGENDRE  = 3
        };

        /** Constructor */
        FitFunction(double m_mass_hypothesis, double m_window_size, double m_bin_size, ModelOrder m_model_order,
                SignalFitModel m_sig_model = FitFunction::SignalFitModel::NONE,
                bool m_exp_background = true) {
            window_size = m_window_size;
            bin_size = m_bin_size;
            mass_hypothesis = m_mass_hypothesis;
            sig_model = m_sig_model;
            model_order = m_model_order;
            exp_background = m_exp_background;

            // The signal parameter is always one greater than the
            // polynomial order.
            if(model_order == FitFunction::ModelOrder::FIRST) {
                order = 1;
                sigParm = 2;
            } else if(model_order == FitFunction::ModelOrder::THIRD) {
                order = 3;
                sigParm = 4;
            } else if(model_order == FitFunction::ModelOrder::FIFTH) {
                order = 5;
                sigParm = 6;
            } else if(model_order == FitFunction::ModelOrder::SEVENTH) {
                order = 7;
                sigParm = 8;
            }
        }

        /** Calculates the value of the function at the specified x
         * and with the specified parameters.
         */
        double operator() (double *x, double *par) {
            if(exp_background) {
                return TMath::Power(10, calculateBackground(x, par)) + calculateSignal(x, par);
            } else {
                return calculateBackground(x, par) + calculateSignal(x, par);
            }
        }

    protected:
        /** Mass hypothesis */
        double mass_hypothesis = 0;

        /** Size of the search window. */
        double window_size = 0;

        /** Size of each bin in the histogram. */
        double bin_size = 0;

        /** The model order as an integer. */
        int order = 0;

        /** Signal fit function to be used. */
        SignalFitModel sig_model;

        /** Order of the model to be used. */
        ModelOrder model_order;

        /** Type of background fit to use. **/
        bool exp_background = true;

        /**
         * Calculates the value of the background function at the
         * specified x and with the specified parameters.
         */
        virtual double calculateBackground(double *x, double *par) = 0;

        /**
         * Calculates the value of the signal function at the
         * specified x and with the specified parameters.
         */
        double calculateSignal(double *x, double *par) {
            if(sig_model == FitFunction::SignalFitModel::GAUSSIAN) 
            {
                return bin_size * FunctionMath::Gaussian(x[0], par[sigParm], par[sigParm + 1], par[sigParm + 2]);
            } 
            else if(sig_model == FitFunction::SignalFitModel::CRYSTAL_BALL) 
            {
                return bin_size * FunctionMath::CrystalBall(x[0], par[sigParm], par[sigParm + 1], par[sigParm + 2], par[sigParm + 3], par[sigParm + 4]);
            } 
            else 
            {
                return 0.0;
            }
        }

        /**
         * Gets a value of x corrected for window size and the mass
         * hypothesis.
         */
        double getCorrectedX(double x) {
            return (x - mass_hypothesis) / (window_size * 2.0);
        }

    private:
        /** Specifies where the signal parameters begin. **/
        int sigParm = 0;
};

#endif
