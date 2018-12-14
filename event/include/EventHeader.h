/**
 * @file EventHeader.h
 * @brief Class used to encapsulate event information.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef _EVENT_HEADER_H_
#define _EVENT_HEADER_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <vector>

//----------//
//   ROOT   //
//----------//
#include <TObject.h>

class EventHeader : public TObject { 

    public: 
        
        /** Constructor */
        EventHeader();

        /** Destructor */
        ~EventHeader();

        /** Reset the EventHeader object */ 
        void Clear(Option_t *option="");

        /** 
         * Set the event number. 
         *
         * @param event_number The event number.
         */
        void setEventNumber(const int event_number) { event_number_ = event_number; };

        /** @return The event number. */
        int getEventNumber() const { return event_number_; }; 

        /**
         * Set the event time stamp. The event time is currently the Unix time 
         * stamp associated with the event.
         *
         * @param event_time The Unix time stamp of the event.
         */
        void setEventTime(const int event_time) { event_time_ = event_time; };

        /** @return The event time. */
        int getEventTime() const { return event_time_; };

        /** 
         * @param event_time The run number.
         */
        void setRunNumber(const int run_number) { run_number_ = run_number; };

        /** @return The run number. */
        int getRunNumber() const { return run_number_; };

        /**
         * @param pair0_trigger Set Flag indicating whether this event was due
         *        to a pair0 trigger.
         */
        void setPair0Trigger(const int pair0_trigger) { 
            pair0_trigger_ = pair0_trigger; 
        };
        
        /**
         * Indicate whether a pair0 trigger was registered for the event.
         *
         * @return Returns true if a pair0 trigger was registered for the,
         *         false otherwise.
         */
        bool isPair0Trigger() const { return pair0_trigger_ == 1; };
        
        /**
         * @param pair1_trigger Set Flag indicating whether this event was due
         *        to a pair1 trigger.
         */
        void setPair1Trigger(const int pair1_trigger) {
            pair1_trigger_ = pair1_trigger;
        };
        
        /**
         * Indicate whether a pair1 trigger was registered for the event.
         *
         * @return Returns true if a pair1 trigger was registered for the,
         *         false otherwise.
         */      
        bool isPair1Trigger() const { return pair1_trigger_ == 1; };
        
        /**
         * @param pulser_trigger Set Flag indicating whether this event was due
         *        to a pulser trigger.
         */
        void setPulserTrigger(const int pulser_trigger) { 
            pulser_trigger_ = pulser_trigger; 
        };

        /**
         * Indicate whether a pulser (random) trigger was registered for the 
         * event.
         *
         * @return Returns true if a pulser trigger was registered for the,
         *         false otherwise.
         */
        bool isPulserTrigger() const { return pulser_trigger_ == 1; };
        
        /**
         * @param single0_trigger Set Flag indicating whether this event was due
         *        to a single0 trigger.
         */
        void setSingle0Trigger(const int single0_trigger) { 
            single0_trigger_ = single0_trigger; 
        };

        /**
         * Indicate whether a single0 trigger was registered for the event.
         *
         * @return Returns true if a single0 trigger was registered for the,
         *         false otherwise.
         */
        bool isSingle0Trigger() const { return single0_trigger_ == 1; };
        
        /**
         * @param single1_trigger Set Flag indicating whether this event was due
         *        to a single1 trigger.
         */
        void setSingle1Trigger(const int single1_trigger) { 
            single1_trigger_ = single1_trigger; 
        };
        
        /**
         * Indicate whether a single1 trigger was registered for the event.
         *
         * @return Returns true if a single1 trigger was registered for the,
         *         false otherwise.
         */
        bool isSingle1Trigger() const { return single1_trigger_ == 1; };
       
        /**
         * Set the state of the SVT bias during the event i.e. was it on or 
         * off? 
         *
         * @param svt_bias_state The state of the SVT bias. It's set to 0 if 
         *                       the bias was off or 1 if it was on.
         */ 
        void setSvtBiasState(const int svt_bias_state) { 
            svt_bias_state_ = svt_bias_state; 
        }; 
        
        /**
         * Indicate whether the SVT bias was on during the event.
         *
         * @return Returns true if the bias was one, false otherwise.
         */
        bool isSvtBiasOn() const { return svt_bias_state_ == 1; };

        /**
         * Set the flag indicating whether the event was affected by SVT burst
         * noise.
         *
         * @param svt_burstmode_noise Flag indicating whether an event was affected
         *                        by SVT burst noise.  It's set to 0 if it was
         *                        or 1 if it wasn't.
         */
        void setSvtBurstModeNoise(const int svt_burstmode_noise) { 
            svt_burstmode_noise_ = svt_burstmode_noise; 
        };

        /**
         * Indicates whether the event was affected by SVT burst noise.
         *
         * @return Returns true if the event has SVT burst noise, false 
         *         otherwise. 
         */
        bool hasSvtBurstModeNoise() const { return svt_burstmode_noise_ == 0; };
        
        /**
         * Set the flag indicating whether the SVT headers had errors.
         *
         * @param svt_event_header_state Flag indicating whether the SVT event
         *                               headers had errors.
         *
         */
        void setSvtEventHeaderState(const int svt_event_header_state) {
            svt_event_header_state_ = svt_event_header_state; 
        };
        
        /**
         * Indicates whether the SVT event headers had errors.
         *
         * @return Returns true if the SVT event headers had an error, 
         *         false otherwise.
         */
        bool hasSvtEventHeaderErrors() const { return svt_event_header_state_ == 0; }; 
        
        /**
         * Set the flag indicating whether the SVT latency was correct
         * during an event.
         *
         * @param svt_latency_state Flag indicating whether the SVT latency
         *                          was correct during an event.
         */
        void setSvtLatencyState(const int svt_latency_state) { 
            svt_latency_state_ = svt_latency_state; 
        }; 

        /**
         * Indicate whether the SVT latency was correct during an event.
         *
         * @return Returns true if the SVT latency was correct, false 
         *         otherwise.
         */
        bool isSvtLatencyGood() const { return svt_latency_state_ == 1; };

        /**
         * Set the state of indicating whether the SVT was open or closed 
         * during an event. 
         *
         * @param svt_position_state The state indicating whether the SVT was 
         *                           open or closed. It's set to 0 if the SVT 
         *                           was open or 1 if it was closed. 
         */ 
        void setSvtPositionState(const int svt_position_state) { 
            svt_position_state_ = svt_position_state; 
        }; 

        /**
         * Indicates whether the SVT was open or closed during an event.
         *
         * @return Returns true if the SVT was closed, false otherwise.
         */ 
        bool isSvtClosed() const { return svt_position_state_ == 1; }; 

        
        /**
         * Set the event RF time.
         *
         * @param channel The channel from which the RF time was retrieved.
         * @param rf_time The event RF time. 
         */
        void setRfTime(const int channel, const double rf_time) {
            rf_times_[channel] = rf_time; 
        }; 
        
        /**
         * Get the RF time.
         *
         * @param channel The channel associated with the RF time.
         * @return The RF time. 
         */ 
        double getRfTime(const int channel) const { return rf_times_[channel]; }; 
    
    private: 

        /** Event number */
        int event_number_{-9999}; 

        /**  Event time */
        double event_time_{-9999}; 

        /** Run number */
        int run_number_{-9999}; 

        /** 
         * Flag indicating that a pair0 trigger was registered. It's 
         * set to 1 if it was registered or 0 if it wasn't.
         */
        int pair0_trigger_{0};

        /** 
         * Flag indicating that a pair1 trigger was registered. It's 
         * set to 1 if it was registered or 0 if it wasn't.
         */
        int pair1_trigger_{0};

        /** 
         * Flag indicating that a pulser (random) trigger was registered. It's 
         * set to 1 if it was registered or 0 if it wasn't.
         */
        int pulser_trigger_{0};

        /** 
         * Flag indicating that a singles0 trigger was registered. It's 
         * set to 1 if it was registered or 0 if it wasn't.
         */
        int single0_trigger_{0};

        /** 
         * Flag indicating that a singles1 trigger was registered. It's 
         * set to 1 if it was registered or 0 if it wasn't.
         */
        int single1_trigger_{0};
       
        /** 
         * Flag indicating the state of the SVT bias. It's set to 0 if the bias
         * was off or 1 if it was on.
         */
        int svt_bias_state_{0};

        /**
         * Flag indicating whether the event was affected by SVT burst noise. 
         * It's set to 0 if the event saw burst noise or 1 if it was fine.
         */ 
        int svt_burstmode_noise_{0}; 

        /**
         * Flag indicating whether the SVT event headers had an error. It's 
         * set to 0 if the event headers had and error, of 1 if it was errorless. 
         */
        int svt_event_header_state_{0}; 

        /**
         * Flag indicating whether the SVT latency was correct during an event. 
         * It's set to 0 if the latency was incorrect, 1 if it was fine.
         */
        int svt_latency_state_{0}; 

        /** 
         * Flag indicating whether the SVT was open or closed.  It's set to 0 if 
         * the SVT was open or 1 if it was closed.
         */ 
        int svt_position_state_{0}; 

        /** The RF time */
        double rf_times_[2]; 

        ClassDef(EventHeader, 1);

}; // EventHeader

#endif // _EVENT_HEADER_H_

