/**
 * @file RawSvtHit.h
 * @brief Class used to encapsulate raw svt hit information
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 */

#ifndef _RAW_SVT_HIT_H_
#define _RAW_SVT_HIT_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <iostream>

//----------//
//   ROOT   //
//----------//
#include <TObject.h>
#include <TClonesArray.h>

class RawSvtHit : public TObject { 

    public: 

        /** Constructor */
        RawSvtHit();

        /** Destructor */
        virtual ~RawSvtHit();

        /** Reset the Hit object. */
        void Clear();

        /** Set the fit multi */
        void setFitN(int fitN);

        /** Set the fit parameters */
        void setFit(double fit[5], int fitI);

        /** Set the adc values */
        void setADCs(int adcs[6]);

        /** Set the system */
        void setSystem(int system);

        /** Set the barrel */
        void setBarrel(int barrel);

        /** Set the layer */
        void setLayer(int layer);

        /** Set the module */
        void setModule(int module);

        /** Set the sensor */
        void setSensor(int sensor);

        /** Set the side */
        void setSide(int side);

        /** Set the strip */
        void setStrip(int strip);

        /** Get the fit multi */
        int getFitN() {return fitN_;}

        /** Get the fit paramters */
        double * getFit(int fitI);

        /** Get the adc values */
        int * getADCs();

        /** Get the system */
        int getSystem();

        /** Get the barrel */
        int getBarrel();

        /** Get the layer */
        int getLayer();

        /** Get the module */
        int getModule();

        /** Get the sensor */
        int getSensor();

        /** Get the side */
        int getSide();

        /** Get the strip */
        int getStrip();

        /** Get the t0 fit parameter */
        double getT0(int fitI) {return fit_[fitI][0];}

        /** Get the t0 err fit parameter */
        double getT0err(int fitI) {return fit_[fitI][1];}

        /** Get the amplitude fit parameter */
        double getAmp(int fitI) {return fit_[fitI][2];}

        /** Get the amplitude error fit parameter */
        double getAmpErr(int fitI) {return fit_[fitI][3];}

        /** Get the chiSq probability */
        double getChiSq(int fitI) {return fit_[fitI][4];}

        ClassDef(RawSvtHit, 1);

    private:


        /** The raw adcs of the hit. */
        int adcs_[6]{-999,-999,-999,-999,-999,-999}; 
        int system_{-999}; 
        int barrel_{-999}; 
        int layer_{-999}; 
        int module_{-999}; 
        int sensor_{-999}; 
        int side_{-999}; 
        int strip_{-999}; 
        /** The fit parameters of the hit. */
        int fitN_{0}; 
        //<<<<<<< HEAD
        // double fit_[2][5]={{-999.9,-999.9,-999.9,-999.9,-999.9},{-999.9,-999.9,-999.9,-999.9,-999.9}}; 
        //=======
        double fit_[2][5]{{-999.9,-999.9,-999.9,-999.9,-999.9},{-999.9,-999.9,-999.9,-999.9,-999.9}}; 
        //>>>>>>> master

}; // RawSvtHit

#endif
