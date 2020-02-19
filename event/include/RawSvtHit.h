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

        /** Set the fit parameters */
        void setFit(double fit[5]);

        /** Set the fit parameters */
        void setSimPos(double hitPos[3]);

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

        /** Set the sim hit energy deposition */
        void setSimEdep(double hitEdep);

        /** Get the fit paramters */
        double * getFit();

        /** Get the sim hit position */
        double * getSimPos();

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
        double getT0() {return fit_[0];}

        /** Get the t0 err fit parameter */
        double getT0err() {return fit_[1];}

        /** Get the amplitude fit parameter */
        double getAmp() {return fit_[2];}

        /** Get the amplitude error fit parameter */
        double getAmpErr() {return fit_[3];}

        /** Get the chiSq probability */
        double getChiSq() {return fit_[4];}

        /** Get the sim hit energy deposition */
        double getSimEdep() {return simEdep_;}

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
        double fit_[5]{-999.9,-999.9,-999.9,-999.9,-999.9}; 
        /** The position and energy deposit of the associated sim hit. */
        double simPos_[3]{-999.9, -999.9, -999.9}; 
        double simEdep_{-999.9}; 

}; // RawSvtHit

#endif
