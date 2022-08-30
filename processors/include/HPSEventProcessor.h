#ifndef _HPSEVENT_HEADER_PROCESSOR_H__
#define _HPSEVENT_HEADER_PROCESSOR_H__

#include "TClonesArray.h"

#include "Processor.h"
#include "HpsEvent.h"
#include "TTree.h"


/**
 * @brief Insert description here.
 * more details
 */
class HPSEventProcessor : public Processor {

    public:
        /**
         * @brief Constructor
         * 
         * @param name 
         * @param process 
         */
        HPSEventProcessor(const std::string& name, Process& process);
        ~HPSEventProcessor();

        /**
         * @brief description
         * 
         * @param ievent 
         * @return true 
         * @return false 
         */
        virtual bool process(IEvent* ievent);

        /**
         * @brief description
         * 
         * @param tree 
         */
        virtual void initialize(TTree* tree);

        /**
         * @brief description
         * 
         */
        virtual void finalize();

    private:
        TClonesArray* header_{nullptr}; //!< description
  
};

#endif
