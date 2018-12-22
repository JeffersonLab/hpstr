/**
 * @file EventHeader.cxx
 * @brief Class used to encapsulate event information.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "EventHeader.h"

ClassImp(EventHeader)

EventHeader::EventHeader()
    : TObject() { 
    }

EventHeader::~EventHeader() {
    Clear();
}

void EventHeader::Clear(Option_t* /* option */) {
    TObject::Clear(); 
}

void EventHeader::Copy(TObject& obj) const { 

    EventHeader& header = static_cast<EventHeader&>(obj); 
    header.event_number_ = event_number_; 
    header.event_time_ = event_time_; 
    header.run_number_ = run_number_; 
    header.pair0_trigger_ = pair0_trigger_;
    header.pair1_trigger_ = pair1_trigger_;
    header.pulser_trigger_ = pulser_trigger_;
    header.single0_trigger_ = single0_trigger_;
    header.single1_trigger_ = single1_trigger_;
    header.svt_bias_state_ = svt_bias_state_;
    header.svt_burstmode_noise_ = svt_burstmode_noise_; 
    header.svt_event_header_state_ = svt_event_header_state_; 
    header.svt_latency_state_ = svt_latency_state_; 
    header.svt_position_state_ = svt_position_state_;
    header.rf_times_[0] = rf_times_[0];
    header.rf_times_[1] = rf_times_[1];
}

void EventHeader::Print() { 
    std::cout << "[ EventHeader ]:\n" 
              << "\tEvent number: " << event_number_ << "\n"
              << "\tRun number: " << run_number_ << "\n"
              << "\tEvent time: " << event_time_ << "\n" 
              << "\tRF time: 1) " << rf_times_[0] << " 2) " << rf_times_[1] 
              << std::endl;
}
