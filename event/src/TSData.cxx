/**
 * @file TSData.cxx
 * @brief Class used to decode VTP words.
 * @author: Cameron Bravo, SLAC National Accelerator Laboratory
 */

#include "TSData.h"

ClassImp(TSData)

TSData::TSData() 
    : TObject() {
    }

TSData::~TSData(){
    Clear();
}

// TODO: Finish writing print method
void TSData::print(){
    using namespace std;
    cout << "TSData::print()" << endl;
    cout << "Event Number: " << EN << endl;
    cout << "Trigger Time: " << T << endl;
}

