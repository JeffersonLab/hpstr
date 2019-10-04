/**
 * @file VTPData.h
 * @brief Class used to decode VTP words.
 * @author: Maurik Holtrop, University of New Hampsire
 * @author: Cameron Bravo, SLAC National Accelerator Laboratory
 */

#ifndef _VTP_DATA_H_
#define _VTP_DATA_H_

#include <vector>

//----------//
//   LCIO   //
//----------//
#include <EVENT/LCGenericObject.h>

class VTPData { 

    public:
        struct {
            unsigned int blocklevel:  8; // bit 0-7
            unsigned int  blocknum:  10; // bit 8-17
            unsigned int  nothing:    4; // bit 18-21
            unsigned int  slotid:     5; // bit 22-26
            unsigned int  type:       4; // bit 27-30
            bool          istype:     1; // bit31
        } blockHeader;

        struct {
            unsigned int  nwords:    22; // bit 0-21
            unsigned int  slotid:     5; // bit 22-26
            unsigned int  type:       4; // bit 27-30
            bool          istype:     1; // bit 31
        } blockTail;

        struct {
            unsigned int eventnum:  27;  // bit  0-26
            unsigned int type:       4;  // bit 27-30
            bool         istype:     1;  // bit 31
        } eventHeader;

        unsigned long trigTime;

        struct hpsCluster {
            int X:    6;        // bit 0-5
            int Y:    4;        // bit 6-9
            unsigned int E: 13; // bit 10-22
            unsigned int subtype:    4;  // bit 23-26
            unsigned int type:       4;  // bit 27-30
            bool         istype:     1;  // bit 31
            unsigned int T:         10;  // bit 0-9
            unsigned int N:          4;  // bit 10-13   Second word
            unsigned int nothing:   18;  // bit 14-31   Second word not used.
        };

        std::vector<hpsCluster> clusters;

        struct hpsSingleTrig {
            unsigned int T:  10;  // bit 0-9 Trigger time in 4ns units referenced from the beginning of the readout window.
            struct {
                bool emin:   1;
                bool emax:   1;
                bool nmin:   1;
                bool xmin:   1;
                bool pose:   1; // position dependent energy thresh.
                bool hodo1c: 1; // hodoscope layer 1 coincidence.
                bool hodo2c: 1; // hodoscope layer 2 coincidence.
                bool hodogeo:1; // hodoscope layer 1 geometry matched to layer 2 geometry
                bool hodoecal:1;// hodoscope layer 1 and 2 geometry mached to ecal cluster x.
            } Pass;           // 9 bits: bit 10-18
            bool topnbot: 1;  //         bit 19
            unsigned int inst:3; // bit 20-22 = single cluster trigger bit instance.
            unsigned int subtype:    4;  // bit 23-26
            unsigned int type:       4;  // bit 27-30
            bool         istype:     1;  // bit 31
        };

        std::vector<hpsSingleTrig> singletrigs;

        struct hpsPairTrig {
            unsigned int T:  10;  // bit 0-9  Trigger time in 4ns units referenced from the beginning of the readout window.
            struct {
                bool clusesum:   1;
                bool clusedif:   1;
                bool eslope:     1;
                bool coplane:   1;
                unsigned int dummy: 5;
            } Pass;           // 9 bits: bit 10-18
            bool topnbot: 1;  //         bit 19 - dummy!
            unsigned int inst:3; // bit 20-22 = single cluster trigger bit instance.
            unsigned int subtype:    4;  // bit 23-26
            unsigned int type:       4;  // bit 27-30
            bool         istype:     1;  // bit 31
        };

        std::vector<hpsPairTrig> pairtrigs;

        struct hpsCalibTrig {
            unsigned int T:  10;  // bit 0-9  Trigger time in 4ns units referenced from the beginning of the readout window.
            unsigned int reserved: 9;
            struct {
                bool cosmic:    1;
                bool led:       1;
                bool hodoscope: 1;
                bool puler:     1;
            } trigtype;
            unsigned int subtype:    4;  // bit 23-26
            unsigned int type:       4;  // bit 27-30
            bool         istype:     1;  // bit 31
        };

        std::vector<hpsCalibTrig> calibtrigs;

        struct hpsClusterMult {
            unsigned int T:  10;  // bit 0-9  Trigger time in 4ns units referenced from the beginning of the readout window.
            unsigned int multtop:   4;
            unsigned int multbot:   4;
            unsigned int multtot:   4;
            bool         bitinst:   1;
            unsigned int subtype:    4;  // bit 23-26
            unsigned int type:       4;  // bit 27-30
            bool         istype:     1;  // bit 31
        };

        std::vector<hpsClusterMult> clustermult;  // Cluster multiplicity.

        struct hpsFEETrig {
            unsigned int T:  10;  // bit 0-9  Trigger time in 4ns units referenced from the beginning of the readout window.
            unsigned int region:   7;
            unsigned int reserved:   6;
            unsigned int subtype:    4;  // bit 23-26
            unsigned int type:       4;  // bit 27-30
            bool         istype:     1;  // bit 31
        };

        std::vector<hpsFEETrig> feetrigger;  // Cluster multiplicity.

    public:
        VTPData(){};
        VTPData(EVENT::LCGenericObject* vtp_data);
        ~VTPData(){};

        void Clear(){
            clusters.clear();
            singletrigs.clear();
            pairtrigs.clear();
            calibtrigs.clear();
            clustermult.clear();
            feetrigger.clear();
        };

        void parseVTPData(EVENT::LCGenericObject* vtp_data);

};

#endif // __VTP_DATA_H__
