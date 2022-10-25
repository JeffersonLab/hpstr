#ifndef __HPSEVENT_FILE_H
#define __HPSEVENT_FILE_H

#include "IEventFile.h"
#include "HpsEvent.h"
#include "TTreeReader.h"
#include "TFile.h"
#include "TTree.h"

/**
 * @brief description
 * 
 */
class HpsEventFile : public IEventFile {

<<<<<<< HEAD
 public:

  virtual ~HpsEventFile();
  HpsEventFile(const std::string ifilename, const std::string& ofilename);
  virtual bool nextEvent();
  virtual bool getEvent(int evN);
  void setupEvent(IEvent* ievent);
  void resetOutputFileDir() { ofile_->cd();}
  TFile* getOutputFile() { return ofile_;}
  void close();


 private:

  HpsEvent* event_{nullptr};
  int entry_{0};
  int maxEntries_{0};
  TFile* ofile_{nullptr};
  TFile* rootfile_{nullptr};
  TTree* intree_{nullptr};
  
  
  //TTreeReader* ttree_reader;
  
=======
    public:
        /**
         * @brief Constructor
         * 
         * @param ifilename 
         * @param ofilename 
         */
        HpsEventFile(const std::string ifilename, const std::string& ofilename);

        /** Destructor */
        virtual ~HpsEventFile();

        /**
         * @brief description
         * 
         * @return true 
         * @return false 
         */
        virtual bool nextEvent();

        /**
         * @brief description
         * 
         * @param ievent 
         */
        void setupEvent(IEvent* ievent);

        /**
         * @brief description
         * 
         */
        void resetOutputFileDir() { ofile_->cd();}

        /**
         * @brief Get the output file.
         * 
         * @return TFile* 
         */
        TFile* getOutputFile() { return ofile_;}

        /**
         * @brief description
         * 
         */
        void close();


    private:
        HpsEvent* event_{nullptr}; //!< description
        int entry_{0}; //!< description
        int maxEntries_{0}; //!< description
        TFile* ofile_{nullptr}; //!< description
        TFile* rootfile_{nullptr}; //!< description
        TTree* intree_{nullptr}; //!< description

        //TTreeReader* ttree_reader;
>>>>>>> 30ff1b41b3b87982b2760e50d42d036b6af48ce1
};


#endif
