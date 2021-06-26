/**
 * @file SvtBl2DEvioProcessor.cxx
 * @author Emrys Peets, Stanford, SLAC National Accelerator Laboratory
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 */

#include "SvtBl2DEvioProcessor.h"

SvtBl2DEvioProcessor::SvtBl2DEvioProcessor(const std::string& name, Process& process)
    : Processor(name, process) { 
        mmapper_ = new ModuleMapper();
    }

SvtBl2DEvioProcessor::~SvtBl2DEvioProcessor() { 
}

void SvtBl2DEvioProcessor::configure(const ParameterSet& parameters) {

    std::cout << "Configuring SvtBl2DEvioProcessor" << std::endl;
    try
    {
        debug_          = parameters.getInteger("debug");
        trigFilename_   = parameters.getString("trigConf");
        histCfgFilename_  = parameters.getString("histCfg");
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }
}

void SvtBl2DEvioProcessor::initialize(std::string inFilename, std::string outFilename) {
    std::cout << "SvtBl2DEvioProcessor::initialize" << std::endl;
    inFilename_ = inFilename;
    outF_ = new TFile(outFilename.c_str(),"RECREATE");
    etool = new HPSEvioReader();
    etool->Open(inFilename.c_str());
    etool->SVT->fSaveHeaders = true;

    if(trigFilename_.size()>2){
        etool->TrigConf->Parse_trigger_file(trigFilename_); // If requested, parse the Trigger config file supplied.
        etool->ECAL->Config();
        std::cout << "Parsed trigger file: " << trigFilename_ << std::endl;
    }

    //DEBUGGING  
    if(debug_==0){
        etool->fDebug = 0b000000;
    }else if(debug_==1){
        etool->fDebug = 0b000001;
    }else if(debug_==2){
        etool->fDebug = 0b000011;
    } else if(debug_ == 3){
        etool->fDebug = 0b000111;
    } else{
        etool->fDebug = 0xFF;
    }


    bool found = false;
    while( !found &&   etool->Next()== S_SUCCESS){
        if( (etool->this_tag & 128) == 128){
            found = true;
            run_number_ = etool->GetRunNumber();
            trigtime_start_ = etool->GetTrigTime();
            break;
        }
    }
    if( found == false) std::cout << "WARNING -- Not able to find a bank with a runnumber! \n";
    etool->Close();

    if(etool->SVT){
        if(run_number_ < 8000){ // This is 2015 or 2016 data.
            etool->Set2016Data();
        }else{
            etool->Set2019Data();
        }
    }else{
        std::cout << "NO SVT initialized \n";
    }

    etool->fAutoAdd = false;

    std::cout << "Run Number: " << run_number_ <<"  Start trigger time:  " << trigtime_start_ << std::endl;

    if(debug_>1) etool->PrintBank(5);

    //Init histos
    svtCondHistos = new Svt2DBlHistos("raw_hits", mmapper_);

    std::cout << "[SvtBl2DEvioProcessor] Load JSON" << std::endl;
    svtCondHistos->loadHistoConfig(histCfgFilename_);
    if (debug_ > 0) std::cout << "[SvtBl2DAnaProcessor] Define 2DHistos" << std::endl;
    svtCondHistos->Svt2DBlHistos::DefineHistos();
    if (debug_ > 0) std::cout << "[SvtBl2DAnaProcessor] Defined 2DHistos" << std::endl;

}

bool SvtBl2DEvioProcessor::process() {

    std::cout << "SvtBl2DEvioProcessor::process" << std::endl;
    unsigned long evt_count=0;
    unsigned long totalCount=0;
    int l0APVmap[4] = {1, 0, 2, 3};
    int APVmap[5] = {4, 3, 2, 1, 0};

    std::chrono::microseconds totalTime(0);

    auto start = std::chrono::system_clock::now();
    auto time1 = start;

    unsigned long time_of_last_event = 0;
    int last_event_trigger_bits = 0;

    etool->Open(inFilename_.c_str());
    std::cout << "SvtBl2DEvioProcessor::opened file" << std::endl;
    while(etool->Next() == S_SUCCESS){
        if( (etool->this_tag & 128) != 128) continue;
        if(debug_) cout<<"EVIO Event " << evt_count << endl;
        for(int i = 0; i < rawSvtHits_.size(); i++)
        {
            delete rawSvtHits_[i];
        }
        rawSvtHits_.clear();
        //      etool->VtpTop->ParseBank();
        //      etool->VtpBot->ParseBank();
        evt_count++;
        //      cout << "Event:  " << etool->head->GetEventNumber() << "  seq: " << evt_count << endl;

        if(debug_>0) {
            etool->PrintBank(10);
        }
        if( evt_count%50000 ==0 ){
            //      statistics
            auto time2 = std::chrono::system_clock::now();
            std::chrono::microseconds delta_t = std::chrono::duration_cast<std::chrono::microseconds>(time2-time1);
            totalTime += delta_t;
            double rate = 1000000.0 * ((double) evt_count) / delta_t.count();
            totalCount += evt_count;
            double avgRate = 1000000.0 * ((double) totalCount) / totalTime.count();
            evt_count = 0;
            time1 = std::chrono::system_clock::now();
        }

        TSBank::TriggerBits tstrig = etool->Trigger->GetTriggerBits();
        if(etool->SVT){
            //etool->PrintBank(0);
            //std::cout << etool->SVT->svt_data.size() << " raw svt hits" << endl;
            for(int i = 0; i < etool->SVT->svt_data.size(); i++)
            {
                RawSvtHit * rawHit = new RawSvtHit();
                std::string hwTag = "F"+std::to_string(int(etool->SVT->svt_data[i].head.feb_id))+"H"+std::to_string(int(etool->SVT->svt_data[i].head.hyb_id));
                std::string swTag = mmapper_->getSwFromHw(hwTag);
                int layer = std::stoi(swTag.substr(2, swTag.find("_")));
                int module = std::stoi(swTag.substr(swTag.find("m")+1, swTag.length()));
                rawHit->setLayer(layer);
                rawHit->setModule(module);
                int apv = int(etool->SVT->svt_data[i].head.apv);
                int strip = l0APVmap[apv]*128;
                if ( int(etool->SVT->svt_data[i].head.feb_id) > 1 ) strip = APVmap[apv]*128;
                strip += int(etool->SVT->svt_data[i].head.chan);
                rawHit->setStrip(strip);
                int adcs[6];
                for(int adcI = 0; adcI < etool->SVT->svt_data.size(); adcI++)
                {
                    adcs[adcI] = int(etool->SVT->svt_data[i].samples[adcI]);
                }
                rawHit->setADCs(adcs);
                rawSvtHits_.push_back(rawHit);
            }
            svtCondHistos->FillHistograms(&rawSvtHits_,1.);
        }

        time_of_last_event= etool->GetTrigTime();
        last_event_trigger_bits = etool->Trigger->GetTriggerInt();
    }
    totalCount += evt_count;
    totalCount += evt_count;
    double avgRate = 1000000.0 * ((double) totalCount) / totalTime.count();
    printf("Last event: %6d\n",etool->Head->GetEventNumber());
    printf("Total events: %6ld \n",totalCount);
    printf("Final: %3.4g kHz \n", avgRate/1000.);

    return true;
}

void SvtBl2DEvioProcessor::finalize() { 

    std::cout << "SvtBl2DEvioProcessor::finalize" << std::endl;
    outF_->cd();
    std::cout << "SvtBl2DEvioProcessor::write histos" << std::endl;
    svtCondHistos->saveHistos(outF_,"");
    outF_->Close();
    delete svtCondHistos;
    svtCondHistos = nullptr;
    delete mmapper_;
    mmapper_ = nullptr;

}

DECLARE_PROCESSOR(SvtBl2DEvioProcessor); 
