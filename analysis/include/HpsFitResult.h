#ifndef __HPS_FIT_RESULT_H__
#define __HPS_FIT_RESULT_H__

#include <TFitResult.h>
#include <TFitResultPtr.h>

#include "ChebyshevFitFunction.h"
#include "LegendreFitFunction.h"
#include "las3pluslas6_FitFunction.h"
#include "ua23nolinpluslas1_FitFunction.h"
/**
 * @brief description
 * 
 * details
 */
class HpsFitResult { 

    public: 

        /** Default constructor */
        HpsFitResult(); 

        ~HpsFitResult(); 

        /**
         * @brief Get background fit result
         * 
         * @return TFitResultPtr 
         */
        TFitResultPtr getBkgFitResult() { return bkg_result_; }; 
        
        /**
         * @brief Get background toy model fit result
         * 
         * @return TFitResultPtr 
         */
        TFitResultPtr getBkgToysFitResult() { return bkg_toy_result_; }; 
        
        /**
         * @brief Get the complete fit result
         * 
         * @return TFitResultPtr 
         */
        TFitResultPtr getCompFitResult() { return comp_result_; }; 

        /**
         * @brief description
         * 
         * @param likelihood 
         */
        void addLikelihood(double likelihood) { _likelihoods.push_back(likelihood); }

        /**
         * @brief description
         * 
         * @param signal_yield 
         */
        void addSignalYield(double signal_yield) { _signal_yields.push_back(signal_yield); }

        /**
         * @brief Get the corrected mass
         * 
         * @return double 
         */
        double getCorrectedMass() const { return _cmass; }; 

        /**
         * @brief Get the integral within the fit window.
         * 
         * @return double _integral The integral within the fit window.
         */
        double getIntegral() { return _integral; };

        /**
         * @brief Get the mass hypothesis for the fit.
         * 
         * @return double mass_hypo_ The mass hypothesis used for this fit.
         */
        double getMass() const { return mass_hypo_; };

        /**
         * @brief description
         * 
         * @return double q0_ description
         */
        double getQ0() { return q0_; };

        /**
         * @brief description
         * 
         * @return double p_value_ description
         */
        double getPValue() { return p_value_; };

        /**
         * @brief Get the background rate obtained from the signal+background hit at the mass hypo.
         * 
         * @return double The bkg rate obtained from the sig+bkg fit at the mass hypo.
         */
        double getFullBkgRate();

        /**
         * @brief Get the background rate error from the signal+background fit at the mass hypo.
         * 
         * @return double The bkg rate error from the sig+bkg fit at the mass hypo.
         */
        double getFullBkgRateError();

        /**
         * @brief Get the signal yield obtained from the signal+background fit.
         * 
         * @return float The signal yield obtained from the sig+bkg fit.
         */
        float getSignalYield() { return comp_result_->Parameter(poly_order_ );  };

        /**
         * @brief Get the error of the signal yield.
         * 
         * @return float The error on the signal yield.
         */
        float getSignalYieldErr() { return comp_result_->ParError(poly_order_ ); }; 

        /**
         * @brief Get the upper fit limit.
         * 
         * @return double the upper limit
         */
        double getUpperLimit() { return upper_limit_; };

        /**
         * @brief Get the p-value at the upper limit.
         * 
         * @return double the p-value
         */
        double getUpperLimitPValue() { return _upper_limit_p_value; };

        /**
         * @brief Get the size of the fit window.
         * 
         * @return double The size of the fit window used.
         */
        double getWindowSize() { return window_size_; }; 

        /**
         * @brief Get the likelihoods
         * 
         * @return std::vector<double> 
         */
        std::vector<double> getLikelihoods() { return _likelihoods; }
        
        /**
         * @brief Get the signal yields.
         * 
         * @return std::vector<double> 
         */
        std::vector<double> getSignalYields() { return _signal_yields; }

        //-------------//
        //   Setters   //
        //-------------//

        /**
         * @brief Set the integral.
         * 
         * @param integral 
         */
        void setIntegral(double integral) { _integral = integral; };

        /**
         * @brief Set Q0.
         * 
         * @param q0 
         */
        void setQ0(double q0) { q0_ = q0; };

        /**
         * @brief Set the p-value.
         * 
         * @param p_value 
         */
        void setPValue(double p_value) { p_value_ = p_value; };  

        /**
         * @brief Set the result from the background only fit.
         * 
         * @param bkg_result Result from the background only fit.
         */
        void setBkgFitResult(TFitResultPtr bkg_result) { bkg_result_ = bkg_result; };

        /**
         * @brief Set the result from the background only fit for generating toys.
         * 
         * @param bkg_toy_result Result from the background only fit with toy model.
         */
        void setBkgToysFitResult(TFitResultPtr bkg_toy_result) { bkg_toy_result_ = bkg_toy_result; };

        /**
         * @brief Set the result from the signal+background only fit.
         * 
         * @param comp_result Result from the signal+background only fit.
         */
        void setCompFitResult(TFitResultPtr comp_result) { comp_result_ = comp_result; };

        /**
         * @brief Set mass hypothesis used for this fit.
         * 
         * @param mass The mass hypothesis.
         */
        void setMass(double mass) { mass_hypo_ = mass; };

        /**
         * @brief Set the corrected mass.
         * 
         * @param cmass corrected mass
         */
        void setCorrectedMass(double cmass) { _cmass = cmass; };

        /**
         * @brief Set the order polynomial used by the fitter.
         * 
         * @param poly_order Polynomial order used by the fitter.
         */
        void setPolyOrder(int poly_order) { poly_order_ = poly_order; };

        /**
         * @brief Sets the type of background fit function used by the fitter.
         * 
         * @param bkg_model The background fit model.
         */
        void setBkgModelType(FitFunction::BkgModel bkg_model) { bkg_model_ = bkg_model; };

        /**
         * @brief Set the 2 sigma upper limit.
         *
         * @param upper_limit The 2 sigma upper limit.
         */
        void setUpperLimit(double upper_limit) { upper_limit_ = upper_limit; };
        
        /**
         * @brief Set the p-value at the end of the upper limit calculation.
         * 
         * @param upper_limit_p_value The p-value at the end of the upper limit calculation
         */
        void setUpperLimitPValue(double upper_limit_p_value) { _upper_limit_p_value = upper_limit_p_value; }

        /**
         * @brief Set the size of the fit window used to get this results.
         *
         * @param window_size The size of the fit window.
         */
        void setWindowSize(double window_size) { window_size_ = window_size; }; 

        /**
         * @brief Set the size of the fit window used to get this results.
         *
         * @param bin_width The size of the fit window.
         */
        void setBinWidth(double bin_width) { bin_width_ = bin_width; }; 

    private: 
        /** Result from fit using a background model only. */
        TFitResultPtr bkg_result_{nullptr}; 

        /** Result from fit using a background model for generating toys. */
        TFitResultPtr bkg_toy_result_{nullptr}; 

        /** Result from fit using a signal+background model. */
        TFitResultPtr comp_result_{nullptr}; 

        /** description */
        std::vector<double> _likelihoods; 

        /** description */
        std::vector<double> _signal_yields; 

        /** Total number of events within the fit window. */
        double _integral{0};

        /** Mass hypothesis. */
        double mass_hypo_; 

        /** corrected mass */
        double _cmass{0}; 

        /** q0 value */
        double q0_;
        
        /** p-value. */
        double p_value_;

        /** Order polynomial used by the fitter. */
        double poly_order_{10};

        /** Type of background fit function to use. */
        FitFunction::BkgModel bkg_model_{FitFunction::BkgModel::UA23NOLINPLUSLAS1};

        /** 2 sigma upper limit on the signal. */
        double upper_limit_; 

        /** p-value at the upper limit. */
        double _upper_limit_p_value{-9999};

        /*** Size of the fit window. */
        double window_size_;

        /*** Size of the fit window. */
        double bin_width_;

}; // HpsFitResult

#endif // __HPS_FIT_RESULT_H__
