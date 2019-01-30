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

        /** Set the adc values */
        void setADCs(int adcs[6]);

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

        ClassDef(RawSvtHit, 1);	
    
    private:

        /** The raw adcs of the hit. */
        int adcs_[6]{-999,-999,-999,-999,-999,-999}; 
        int layer_{-999}; 
        int module_{-999}; 
        int sensor_{-999}; 
        int side_{-999}; 
        int strip_{-999}; 

}; // RawSvtHit

#endif
