/**
 * @file TSData.h
 * @brief Class used to decode TS words.
 * @author: Maurik Holtrop, University of New Hampsire
 * @author: Cameron Bravo, SLAC National Accelerator Laboratory
 */

#ifndef _TS_DATA_H_
#define _TS_DATA_H_

#include <vector>

//----------//
//   LCIO   //
//----------//
#include <EVENT/LCGenericObject.h>

//----------//
//   ROOT   //
//----------//
#include <TObject.h>

class TSData : public TObject { 

    public:
        // tsHeader.type meaning:
        //    0: Filler events
        //    1-32: GTP Physics trigger;
        //    33-64: Front panel physics trigger;
        //    250: multi-hit on GTP or Front panel physics trigger
        //    251: Multi-hit on GTP and Front panel physics trigger 
        //    253: VME trigger;
        //    254: VME random trigger;
        struct tsHeader
        {
            int wordCount; // 0-15 Event wordcount; Event header is excluded from the count
            int test     ; // 16-23 0000,0001, or 0x01
            int type     ; // 24-31 Trigger Type
        } header;

        struct tsBits
        {
            bool Single_0_Top  ; //  0   ( 150-8191) MeV (-31,31)   Low energy cluster
            bool Single_1_Top  ; //  1   ( 300-3000) MeV (  5,31)   e+
            bool Single_2_Top  ; //  2   ( 300-3000) MeV (  5,31)   e+ : Position dependent energy cut
            bool Single_3_Top  ; //  3   ( 300-3000) MeV (  5,31)   e+ : HODO L1*L2  Match with cluster
            bool Single_0_Bot  ; //  4   ( 150-8191) MeV (-31,31)   Low energy cluster
            bool Single_1_Bot  ; //  5   ( 300-3000) MeV (  5,31)   e+
            bool Single_2_Bot  ; //  6   ( 300-3000) MeV (  5,31)   e+ : Position dependent energy cut
            bool Single_3_Bot  ; //  7   ( 300-3000) MeV (  5,31)   e+ : HODO L1*L2  Match with cluster
            bool Pair_0        ; //  8    A'
            bool Pair_1        ; //  9    Moller
            bool Pair_2        ; // 10    pi0
            bool Pair_3        ; // 11    -
            bool LED           ; // 12    LED
            bool Cosmic        ; // 13    Cosmic
            bool Hodoscope     ; // 14    Hodoscope
            bool Pulser        ; // 15    Pulser
            bool Mult_0        ; // 16    Multiplicity-0 2 Cluster Trigger
            bool Mult_1        ; // 17    Multiplicity-1 3 Cluster trigger
            bool FEE_Top       ; // 18    FEE Top       ( 2600-5200)
            bool FEE_Bot       ; // 19    FEE Bot       ( 2600-5200)
            unsigned int NA    ; // 20-31 Not used
            unsigned int intval; // Full word
        };
        tsBits prescaled;
        tsBits ext;

        unsigned long EN; // Event Number in Run
        unsigned long T; // Trigger Time

    public:
        TSData();
        ~TSData();

        void print();

        void Clear(){
            TObject::Clear();
        };

        ClassDef(TSData, 1);

};

#endif // __TS_DATA_H__
