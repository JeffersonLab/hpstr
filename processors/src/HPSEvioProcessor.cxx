/**
 * @file HPSEvioProcessor.cxx
 * @author Emrys Peets, Stanford, SLAC National Accelerator Laboratory
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 */

#include "HPSEvioProcessor.h"

HPSEvioProcessor::HPSEvioProcessor(const std::string& name, Process& process)
    : Processor(name, process) { 
    }

HPSEvioProcessor::~HPSEvioProcessor() { 
}

void HPSEvioProcessor::configure(const ParameterSet& parameters) {

    std::cout << "Configuring HPSEvioProcessor" << std::endl;
    try
    {
        debug_          = parameters.getInteger("debug");
        trigFilename_   = parameters.getString("trigConf");
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }
}

void HPSEvioProcessor::initialize(std::string inFilename, std::string outFilename) {
    std::cout << "HPSEvioProcessor::initialize" << std::endl;
    inFilename_ = inFilename;
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

    outF_ = new TFile(outFilename.c_str(),"RECREATE");
    etool->fAutoAdd = false;

    std::cout << "Run Number: " << run_number_ <<"  Start trigger time:  " << trigtime_start_ << std::endl;

    if(debug_>1) etool->PrintBank(5);
}

bool HPSEvioProcessor::process() {

    std::cout << "HPSEvioProcessor::process" << std::endl;
    unsigned long evt_count=0;
    unsigned long totalCount=0;

    std::chrono::microseconds totalTime(0);

    auto start = std::chrono::system_clock::now();
    auto time1 = start;

    unsigned long time_of_last_event = 0;
    int last_event_trigger_bits = 0;

    //init Histograms
    TH1D * event_dt = new TH1D("event_dt", "event_dt", 1000, 0., 100000.); 
    etool->Open(inFilename_.c_str());
    while(etool->Next() == S_SUCCESS){
        if( (etool->this_tag & 128) != 128) continue;
        if(debug_) cout<<"EVIO Event " << evt_count << endl;
        //      etool->VtpTop->ParseBank();
        //      etool->VtpBot->ParseBank();
        evt_count++;
        //      cout << "Event:  " << etool->head->GetEventNumber() << "  seq: " << evt_count << endl;

        if(debug_>0) {
            etool->PrintBank(10);
        }
        if( evt_count%50000 ==0 ){
            //      /* statistics */
            auto time2 = std::chrono::system_clock::now();
            std::chrono::microseconds delta_t = std::chrono::duration_cast<std::chrono::microseconds>(time2-time1);
            totalTime += delta_t;
            double rate = 1000000.0 * ((double) evt_count) / delta_t.count();
            totalCount += evt_count;
            double avgRate = 1000000.0 * ((double) totalCount) / totalTime.count();
            evt_count = 0;
            time1 = std::chrono::system_clock::now();
        }

        for(auto ecal_hit: etool->ECAL->hitmap){
            for(int i=0;i<ecal_hit.second.hits.size();++i){
                if(ecal_hit.second.hits[i].energy>100.){
                    continue;//one day an individual could put plots here
                }
            }

        }
        for(auto cluster: etool->ECAL->GTPClusters ){
            // find the seed hit.
            auto seed_hit=etool->ECAL->hitmap.find(cluster.seed_ixy);
            if(seed_hit == etool->ECAL->hitmap.end() ) std::cout << "Problem! Seed hit not in map. \n";
        }

        TSBank::TriggerBits tstrig = etool->Trigger->GetTriggerBits();
        if(etool->SVT && debug_>0){
            etool->PrintBank(0);
        }

        //     printf("Event : %9d  Time: %16ld ----------------------------------- \n",etool->GetEventNumber(),etool->GetTrigTime()-trigtime_start);
        if(etool->SVT->fSaveHeaders){
            for(int i=0;i<etool->SVT->svt_tails.size(); ++i){
                if(etool->SVT->svt_tails[i].apv_sync_error ) std::cout << "APV Sync ERROR \n";
                if(etool->SVT->svt_tails[i].fifo_backup_error ) std::cout << "FIFO Backup ERROR \n";
                if(etool->SVT->svt_tails[i].skip_count >0 ) std::cout << "Skip count is set. \n";
                //          printf("Tail: %2d - Multi: %6d  Skip: %3d \n",i,etool->SVT->svt_tails[i].num_multisamples,etool->SVT->svt_tails[i].skip_count);
            }
            for(int i=0;i<etool->SVT->svt_headers.size();++i){
                std::cout << "Header " << i << " time: " << etool->SVT->svt_headers[i].GetTimeStamp() << " trig: "<< etool->GetTrigTime()<< "  DT: " << ((long)etool->GetTrigTime() - (long)etool->SVT->svt_headers[i].GetTimeStamp() - (long)trigtime_start_) << std::endl;
            }
        }
        event_dt->Fill((double)(etool->GetTrigTime()-time_of_last_event));
        time_of_last_event= etool->GetTrigTime();
        last_event_trigger_bits = etool->Trigger->GetTriggerInt();
    }
    totalCount += evt_count;
    totalCount += evt_count;
    double avgRate = 1000000.0 * ((double) totalCount) / totalTime.count();
    printf("Last event: %6d\n",etool->Head->GetEventNumber());
    printf("Total events: %6ld \n",totalCount);
    printf("Final: %3.4g kHz \n", avgRate/1000.);
    outF_->cd();
    event_dt->Write();
    outF_->Close();


    return true;
}

void HPSEvioProcessor::finalize() { 

    std::cout << "HPSEvioProcessor::finalize" << std::endl;
}

DECLARE_PROCESSOR(HPSEvioProcessor); 
