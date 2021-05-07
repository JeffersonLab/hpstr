#include "TSProcessor.h"


TSProcessor::TSProcessor(const std::string& name, Process& process)
    : Processor(name, process) {
    }

TSProcessor::~TSProcessor() {
}

void TSProcessor::configure(const ParameterSet& parameters) {

    std::cout << "Configuring EventProcessor" << std::endl;
    try
    {
        debug_         = parameters.getInteger("debug", debug_);
        tsCollLcio_  = parameters.getString("tsCollLcio", tsCollLcio_);
        tsCollRoot_  = parameters.getString("tsCollRoot", tsCollRoot_);
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }
}

void TSProcessor::initialize(TTree* tree) {
    tsData = new TSData();
    tree->Branch(tsCollRoot_.c_str(),  &tsData);
}

bool TSProcessor::process(IEvent* ievent) {

    Event* event = static_cast<Event*> (ievent);

    EVENT::LCCollection* ts_data
            = static_cast<EVENT::LCCollection*>(event->getLCCollection(tsCollLcio_.c_str()));

    EVENT::LCGenericObject* ts_datum
        = static_cast<EVENT::LCGenericObject*>(ts_data->getElementAt(0));

    parseTSData(ts_datum);

    return true;

}

void TSProcessor::parseTSData(EVENT::LCGenericObject* ts_data_lcio)
{
    // Parse out TS header
    unsigned int headerWord = ts_data_lcio->getIntVal(1);
    tsData->header.wordCount = (headerWord      )&0xFFFF; //  0-15 Word Count
    tsData->header.test      = (headerWord >> 16)&0x00FF; // 16-23 Test Word
    tsData->header.type      = (headerWord >> 24)&0x00FF; // 24-31 Trigger Type
    // Parse out trigger time and Event Number
    tsData->T = static_cast<unsigned long>(ts_data_lcio->getIntVal(3)) + ( (static_cast<unsigned long>(ts_data_lcio->getIntVal(4)&0xFFFF)<<32));
    tsData->EN = static_cast<unsigned long>(ts_data_lcio->getIntVal(2)) + ( (static_cast<unsigned long>(ts_data_lcio->getIntVal(4)&0xFFFF0000)<<16));
    // Parse out prescaled word
    tsData->prescaled.intval = ts_data_lcio->getIntVal(5); // Full word
    tsData->prescaled.Single_0_Top = (tsData->prescaled.intval      )&0x001; //  0 Low energy cluster
    tsData->prescaled.Single_1_Top = (tsData->prescaled.intval >>  1)&0x001; //  1 e+
    tsData->prescaled.Single_2_Top = (tsData->prescaled.intval >>  2)&0x001; //  2 e+ : Position dependent energy cut
    tsData->prescaled.Single_3_Top = (tsData->prescaled.intval >>  3)&0x001; //  3 e+ : HODO L1*L2  Match with cluster
    tsData->prescaled.Single_0_Bot = (tsData->prescaled.intval >>  4)&0x001; //  4 Low energy cluster
    tsData->prescaled.Single_1_Bot = (tsData->prescaled.intval >>  5)&0x001; //  5 e+
    tsData->prescaled.Single_2_Bot = (tsData->prescaled.intval >>  6)&0x001; //  6 e+ : Position dependent energy cut
    tsData->prescaled.Single_3_Bot = (tsData->prescaled.intval >>  7)&0x001; //  7 e+ : HODO L1*L2  Match with cluster
    tsData->prescaled.Pair_0       = (tsData->prescaled.intval >>  8)&0x001; //  8 A'
    tsData->prescaled.Pair_1       = (tsData->prescaled.intval >>  9)&0x001; //  9 Moller
    tsData->prescaled.Pair_2       = (tsData->prescaled.intval >> 10)&0x001; // 10 pi0
    tsData->prescaled.Pair_3       = (tsData->prescaled.intval >> 11)&0x001; // 11 -
    tsData->prescaled.LED          = (tsData->prescaled.intval >> 12)&0x001; // 12 LED
    tsData->prescaled.Cosmic       = (tsData->prescaled.intval >> 13)&0x001; // 13 Cosmic
    tsData->prescaled.Hodoscope    = (tsData->prescaled.intval >> 14)&0x001; // 14 Hodoscope
    tsData->prescaled.Pulser       = (tsData->prescaled.intval >> 15)&0x001; // 15 Pulser
    tsData->prescaled.Mult_0       = (tsData->prescaled.intval >> 16)&0x001; // 16 Multiplicity-0 2 Cluster Trigger
    tsData->prescaled.Mult_1       = (tsData->prescaled.intval >> 17)&0x001; // 17 Multiplicity-1 3 Cluster trigger
    tsData->prescaled.FEE_Top      = (tsData->prescaled.intval >> 18)&0x001; // 18 FEE Top       ( 2600-5200)
    tsData->prescaled.FEE_Bot      = (tsData->prescaled.intval >> 19)&0x001; // 19 FEE Bot       ( 2600-5200)
    tsData->prescaled.NA           = (tsData->prescaled.intval >> 20)&0xFFF; // 20-31 Not used
    // Parse out ext word
    tsData->ext.intval = ts_data_lcio->getIntVal(6); // Full word
    tsData->ext.Single_0_Top = (tsData->ext.intval      )&0x001; //  0 Low energy cluster
    tsData->ext.Single_1_Top = (tsData->ext.intval >>  1)&0x001; //  1 e+
    tsData->ext.Single_2_Top = (tsData->ext.intval >>  2)&0x001; //  2 e+ : Position dependent energy cut
    tsData->ext.Single_3_Top = (tsData->ext.intval >>  3)&0x001; //  3 e+ : HODO L1*L2  Match with cluster
    tsData->ext.Single_0_Bot = (tsData->ext.intval >>  4)&0x001; //  4 Low energy cluster
    tsData->ext.Single_1_Bot = (tsData->ext.intval >>  5)&0x001; //  5 e+
    tsData->ext.Single_2_Bot = (tsData->ext.intval >>  6)&0x001; //  6 e+ : Position dependent energy cut
    tsData->ext.Single_3_Bot = (tsData->ext.intval >>  7)&0x001; //  7 e+ : HODO L1*L2  Match with cluster
    tsData->ext.Pair_0       = (tsData->ext.intval >>  8)&0x001; //  8 A'
    tsData->ext.Pair_1       = (tsData->ext.intval >>  9)&0x001; //  9 Moller
    tsData->ext.Pair_2       = (tsData->ext.intval >> 10)&0x001; // 10 pi0
    tsData->ext.Pair_3       = (tsData->ext.intval >> 11)&0x001; // 11 -
    tsData->ext.LED          = (tsData->ext.intval >> 12)&0x001; // 12 LED
    tsData->ext.Cosmic       = (tsData->ext.intval >> 13)&0x001; // 13 Cosmic
    tsData->ext.Hodoscope    = (tsData->ext.intval >> 14)&0x001; // 14 Hodoscope
    tsData->ext.Pulser       = (tsData->ext.intval >> 15)&0x001; // 15 Pulser
    tsData->ext.Mult_0       = (tsData->ext.intval >> 16)&0x001; // 16 Multiplicity-0 2 Cluster Trigger
    tsData->ext.Mult_1       = (tsData->ext.intval >> 17)&0x001; // 17 Multiplicity-1 3 Cluster trigger
    tsData->ext.FEE_Top      = (tsData->ext.intval >> 18)&0x001; // 18 FEE Top       ( 2600-5200)
    tsData->ext.FEE_Bot      = (tsData->ext.intval >> 19)&0x001; // 19 FEE Bot       ( 2600-5200)
    tsData->ext.NA           = (tsData->ext.intval >> 20)&0xFFF; // 20-31 Not used
} //EventProcessor::parstsData->eTSData(LCGenericObject* ts_data_lcio)

void TSProcessor::finalize() {
}

DECLARE_PROCESSOR(TSProcessor);
