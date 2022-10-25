#ifndef __IEVENT_FILE_H__
#define __IEVENT_FILE_H__

#include "IEvent.h"

/**
 * @brief description
 * 
 */
class IEventFile {

    public:
        /** Destructor */
        virtual ~IEventFile() {};
        
        /**
         * @brief description
         * 
         * @return true 
         * @return false 
         */
        virtual bool nextEvent() = 0;

        /**
         * @brief description
         * 
         * @param ievent 
         */
        virtual void setupEvent(IEvent* ievent) = 0;

        /**
         * @brief description
         * 
         */
        virtual void close() = 0;

        /**
         * @brief description
         * 
         */
        virtual void resetOutputFileDir() = 0;
  
};

#endif
