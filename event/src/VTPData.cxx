/**
 * @file VTPData.cxx
 * @brief Class used to decode VTP words.
 * @author: Cameron Bravo, SLAC National Accelerator Laboratory
 */

#include "VTPData.h"

ClassImp(VTPData)

VTPData::VTPData()
    : TObject() {
    }

//VTPData::VTPData(EVENT::LCGenericObject* vtp_data) : TObject() {
//    this->parseVTPData(vtp_data); 
//}

VTPData::~VTPData(){
    Clear();
}

void VTPData::print(){
    using namespace std;
    cout << "blockHeader.blocklevel: " << blockHeader.blocklevel << endl;
    cout << "blockHeader.blocknum: " << blockHeader.blocknum << endl;
    cout << "blockHeader.nothing: " << blockHeader.nothing << endl;
    cout << "blockHeader.slotid: " << blockHeader.slotid << endl;
    cout << "blockHeader.type: " << blockHeader.type << endl;
    cout << "blockHeader.istype: " << blockHeader.istype << endl;
    cout << "blockTail.nwords: " << blockTail.nwords << endl;
    cout << "blockTail.slotid: " << blockTail.slotid << endl;
    cout << "blockTail.type: " << blockTail.type << endl;
    cout << "blockTail.istype: " << blockTail.istype << endl;
    cout << "eventHeader.eventnum: " << eventHeader.eventnum << endl;
    cout << "eventHeader.type: " << eventHeader.type << endl;
    cout << "eventHeader.istype: " << eventHeader.istype << endl;
    cout << "trigTime: " << trigTime << std::endl;
    for(int i = 0; i < clusters.size(); i++)
    {
        cout << "Cluster " << i << endl;
        cout << "    X: " << clusters.at(i).X << endl;
        cout << "    Y: " << clusters.at(i).Y << endl;
        cout << "    E: " << clusters.at(i).E << endl;
        cout << "    subtype: " << clusters.at(i).subtype << endl;
        cout << "    type: " << clusters.at(i).type << endl;
        cout << "    istype: " << clusters.at(i).istype << endl;
        cout << "    T: " << clusters.at(i).T << endl;
        cout << "    N: " << clusters.at(i).N << endl;
        cout << "    nothing: " << clusters.at(i).nothing << endl;
    }
    for(int i = 0; i < singletrigs.size(); i++)
    {
        cout << "Single Trigger " << i << endl;
        cout << "    T: " << singletrigs.at(i).T << endl;
        cout << "    emin: " << singletrigs.at(i).emin << endl;
        cout << "    emax: " << singletrigs.at(i).emax << endl;
        cout << "    nmin: " << singletrigs.at(i).nmin << endl;
        cout << "    xmin: " << singletrigs.at(i).xmin << endl;
        cout << "    pose: " << singletrigs.at(i).pose << endl;
        cout << "    hodo1c: " << singletrigs.at(i).hodo1c << endl;
        cout << "    hodo2c: " << singletrigs.at(i).hodo2c << endl;
        cout << "    hodogeo: " << singletrigs.at(i).hodogeo << endl;
        cout << "    hodoecal: " << singletrigs.at(i).hodoecal << endl;
        cout << "    topnbot: " << singletrigs.at(i).topnbot << endl;
        cout << "    inst: " << singletrigs.at(i).inst << endl;
        cout << "    subtype: " << singletrigs.at(i).subtype << endl;
        cout << "    type: " << singletrigs.at(i).type << endl;
        cout << "    istype: " << singletrigs.at(i).istype << endl;
    }
    for(int i = 0; i < pairtrigs.size(); i++)
    {
        cout << "Pair Trigger " << i << endl;
        cout << "    T: " << pairtrigs.at(i).T << endl;
        cout << "    clusesum: " << pairtrigs.at(i).clusesum << endl;
        cout << "    clusediff: " << pairtrigs.at(i).clusedif << endl;
        cout << "    eslope: " << pairtrigs.at(i).eslope << endl;
        cout << "    coplane: " << pairtrigs.at(i).coplane << endl;
        cout << "    dummy: " << pairtrigs.at(i).dummy << endl;
        cout << "    topnbot: " << pairtrigs.at(i).topnbot << endl;
        cout << "    inst: " << pairtrigs.at(i).inst << endl;
        cout << "    subtype: " << pairtrigs.at(i).subtype << endl;
        cout << "    type: " << pairtrigs.at(i).type << endl;
        cout << "    istype: " << pairtrigs.at(i).istype << endl;
    }
    for(int i = 0; i < calibtrigs.size(); i++)
    {
        cout << "Calibration Trigger " << i << endl;
        cout << "    T: " << calibtrigs.at(i).T << endl;
        cout << "    reserved: " << calibtrigs.at(i).reserved << endl;
        cout << "    cosmicTrig: " << calibtrigs.at(i).cosmicTrig << endl;
        cout << "    LEDTrig: " << calibtrigs.at(i).LEDTrig << endl;
        cout << "    hodoTrig: " << calibtrigs.at(i).hodoTrig << endl;
        cout << "    pulserTrig: " << calibtrigs.at(i).pulserTrig << endl;
        cout << "    subtype: " << calibtrigs.at(i).subtype << endl;
        cout << "    type: " << calibtrigs.at(i).type << endl;
        cout << "    istype: " << calibtrigs.at(i).istype << endl;
    }
    for(int i = 0; i < clustermult.size(); i++)
    {
        cout << "Cluster Multiplicty Trigger " << i << endl;
        cout << "    T: " << clustermult.at(i).T << endl;
        cout << "    multtop: " << clustermult.at(i).multtop << endl;
        cout << "    multbot: " << clustermult.at(i).multbot << endl;
        cout << "    multtot: " << clustermult.at(i).multtot << endl;
        cout << "    bitinst: " << clustermult.at(i).bitinst << endl;
        cout << "    subtype: " << clustermult.at(i).subtype << endl;
        cout << "    type: " << clustermult.at(i).type << endl;
        cout << "    istype: " << clustermult.at(i).istype << endl;
    }
    for(int i = 0; i < feetrigger.size(); i++)
    {
        cout << "FEE Trigger " << i << endl;
        cout << "    T: " << feetrigger.at(i).T << endl;
        cout << "    region: " << feetrigger.at(i).region << endl;
        cout << "    reserved: " << feetrigger.at(i).reserved << endl;
        cout << "    subtype: " << feetrigger.at(i).subtype << endl;
        cout << "    type: " << feetrigger.at(i).type << endl;
        cout << "    istype: " << feetrigger.at(i).istype << endl;
    }

}

