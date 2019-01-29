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

        ClassDef(RawSvtHit, 1);	
    
    private:

        /** The raw adcs of the hit. */
        int adcs_[6]{-999,-999,-999,-999,-999,-999}; 

}; // RawSvtHit

#endif
