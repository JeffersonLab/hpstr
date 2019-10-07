/**
 * @file VTPData.h
 * @brief Class used to decode VTP words.
 * @author: Maurik Holtrop, University of New Hampsire
 * @author: Cameron Bravo, SLAC National Accelerator Laboratory
 */

#ifndef _VTP_DATA_H_
#define _VTP_DATA_H_

#include <vector>
#include <iostream>

//----------//
//   ROOT   //
//----------//
#include <TObject.h>

class VTPData : public TObject { 

    public:
        struct bHeader {
            unsigned int  blocklevel; // 8  bits 0-7
            unsigned int  blocknum;   // 10 bits 8-17
            unsigned int  nothing;    // 4  bits 18-21
            unsigned int  slotid;     // 5  bits 22-26
            unsigned int  type;       // 4  bits 27-30
            bool          istype;     // 1  bit  31
        };
        bHeader blockHeader;

        struct bTail {
            unsigned int  nwords; // 22 bits 0-21
            unsigned int  slotid; // 5  bits 22-26
            unsigned int  type;   // 4  bits 27-30
            bool          istype; // 1  bit  31
        };
        bTail blockTail;

        struct eHeader {
            unsigned int eventnum; // 27 bits 0-26
            unsigned int type;     //  4 bits 27-30
            bool         istype;   //  1 bit  31
        };
        eHeader eventHeader;

        unsigned long trigTime;

        struct hpsCluster {
            int X;                // 6  bits 0-5
            int Y;                // 4  bits 6-9
            unsigned int E;       // 13 bits 10-22
            unsigned int subtype; // 4  bits 23-26
            unsigned int type;    // 4  bits 27-30
            bool         istype;  // 1  bit  31
            unsigned int T;       // 10 bits 0-9     Second word
            unsigned int N;       // 4  bits 10-13   Second word
            unsigned int nothing; // 18 bits 14-31   Second word not used.
        };

        std::vector<hpsCluster> clusters;

        struct hpsSingleTrig {
            unsigned int T;       // 10 bits 0-9 Trigger time in 4ns units from the beginning of the readout window.
            bool emin;            //  1 bit  10
            bool emax;            //  1 bit  11
            bool nmin;            //  1 bit  12
            bool xmin;            //  1 bit  13
            bool pose;            //  1 bit  14 position dependent energy thresh.
            bool hodo1c;          //  1 bit  15 hodoscope layer 1 coincidence.
            bool hodo2c;          //  1 bit  16 hodoscope layer 2 coincidence.
            bool hodogeo;         //  1 bit  17 hodoscope layer 1 geometry matched to layer 2 geometry
            bool hodoecal;        //  1 bit  18 hodoscope layer 1 and 2 geometry mached to ecal cluster x.
            bool topnbot;         //  1 bit  19
            unsigned int inst;    //  3 bits 20-22 = single cluster trigger bit instance.
            unsigned int subtype; //  4 bits 23-26
            unsigned int type;    //  4 bits 27-30
            bool         istype;  //  1 bit  31
        };

        std::vector<hpsSingleTrig> singletrigs;

        struct hpsPairTrig {
            unsigned int T;       // bit 0-9  Trigger time in 4ns units from the beginning of the readout window.
            bool clusesum;        // 1 bit  10
            bool clusedif;        // 1 bit  11
            bool eslope;          // 1 bit  12
            bool coplane;         // 1 bit  13
            unsigned int dummy;   // 5 bits 14-18
            bool topnbot;         // 1 bit  19 - dummy!
            unsigned int inst;    // 3 bits 20-22 = single cluster trigger bit instance.
            unsigned int subtype; // 4 bits 23-26
            unsigned int type;    // 4 bits 27-30
            bool         istype;  // 1 bit  31
        };

        std::vector<hpsPairTrig> pairtrigs;

        struct hpsCalibTrig {
            unsigned int T;          // 10 bits 0-9  Time in 4ns units from the beginning of the readout window.
            unsigned int reserved;   // 9  bits 10-18 
            bool         cosmicTrig; // 1  bit  19
            bool         LEDTrig;    // 1  bit  20
            bool         hodoTrig;   // 1  bit  21
            bool         pulserTrig; // 1  bit  22
            unsigned int subtype;    // 4  bits 23-26
            unsigned int type;       // 4  bits 27-30
            bool         istype;     // 1  bit  31
        };

        std::vector<hpsCalibTrig> calibtrigs;

        struct hpsClusterMult {
            unsigned int T;        // 10 bits 0-9  Time in 4ns units from the beginning of the readout window.
            unsigned int multtop;  // 4  bits 10-13
            unsigned int multbot;  // 4  bits 14-17
            unsigned int multtot;  // 4  bits 18-21
            bool         bitinst;  // 1  bit  22
            unsigned int subtype;  // 4  bits 23-26
            unsigned int type;     // 4  bits 27-30
            bool         istype;   // 1  bit  31
        };

        std::vector<hpsClusterMult> clustermult;  // Cluster multiplicity.

        struct hpsFEETrig {
            unsigned int T;         // 10 bits 0-9  Time in 4ns units from the beginning of the readout window.
            unsigned int region;    // 7  bits 10-16
            unsigned int reserved;  // 6  bits 17-22
            unsigned int subtype;   // 4  bits 23-26
            unsigned int type;      // 4  bits 27-30
            bool         istype;    // 1  bit  31
        };

        std::vector<hpsFEETrig> feetrigger;  // Cluster multiplicity.

    public:
        VTPData();
        ~VTPData();

        void print();

        void Clear(){
            TObject::Clear();
            clusters.clear();
            singletrigs.clear();
            pairtrigs.clear();
            calibtrigs.clear();
            clustermult.clear();
            feetrigger.clear();
        };

        ClassDef(VTPData, 1);

};

#endif // __VTP_DATA_H__
