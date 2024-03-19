///////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Name: RawDataReader
//Author: L. Hsu 
//Description:  This class manages the navigation through the raw data file.   It provides wrapper functions
//(of zlib routines) that allow the user to open, close, rewind and step through the various records of the 
//raw data file.  It manages the filling of the various data storage classes and hands them off to the rest 
//of the processing routine for analysis.  Note, some snippets were originally ported from DarkPipe and 
//PipeFitter raw data reading routines.
//
//File Import By: L. Hsu
//Creation Date: Nov. 17, 2008
//
//Modifications:
//Nov. 22, 2010 - Adding DetectorConfigData 
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef RAWDATAREADER_H
#define RAWDATAREADER_H

#include "zlib.h"

#include "BatRootTypes.h"
#include "DetectorConfigData.h"
#include "AdminData.h"
#include "HistoryData.h"
#include "TriggerData.h"
#include "GPSData.h"
#include "PulseData.h"


// MIDAS
#include "MidasEventData.h"
#include "TSystem.h"

// MIDAS online
#ifdef HAVE_MIDAS
#include "TMidasControl.h"
#include "VirtualOdbRW.h"
#endif


using namespace std;

//!Class for navigation of the raw data file (see header file for more info).
class RawDataReader 
{
   public:

      RawDataReader(); //constructor
      ~RawDataReader(); //destructor

      //Register which data types are to be read from raw file
      void RegisterDetectorConfigData(DetectorConfigData* dataPtr);
      void RegisterAdminData(AdminData* dataPtr);
      void RegisterHistoryData(HistoryData* dataPtr, const int nTowers);
      void RegisterTriggerData(TriggerData* dataPtr, const int nTowers);
      void RegisterGPSData(GPSData* dataPtr);
      void RegisterZipPulseMap(map< int, vector<PulseData> >* mapOfZipPulses);
      void RegisterVetoPulseVector(vector<PulseData>* vectorOfVetoPulses);
      void RegisterNoiseMonitorPulseVector(vector<PulseData>* vectorOfNoiseMonitorPulses);   


      //Read raw data
      void OpenRawDataFile(const string& inputRawDataDir, const string& inputRawDataFile);
      void CloseRawDataFile();
      void ResetRawDataFile();
      void ReadFileHeader(bool dispHeader);      
      int  ReadRawDataRecord();  
      int  SkipRawEvents(const int nEventsToSkip);
      int  ReadRawDataRecord(const int nEventsToSkip);  
      void Clear();
      void ModifyRawData(const map<int,string>& modificationMap);

      //configure verbosity and diagnostic printing
      bool GetDiagnosticPrints(){return fdiagnosticPrints;}
      void SetDiagnosticPrints(bool value){fdiagnosticPrints=value;}
      int GetVerbosity(){return fverbosity;}
      void SetVerbosity(int value){fverbosity=value;}

      //Get file info
      uint64_t GetSeriesInt(){return fMidasSeriesNumber;}
      uint32_t GetDumpNum(){return fMidasDumpNumber;}

      //Get event info
      uint32_t GetEventCategory() const { return fEventCategory; }
      uint32_t GetEventType()     const { return fEventType; }
      
      //Read raw data online (Midas)
      void RegisterGUITSystem(TSystem* gSystem);
#ifdef HAVE_MIDAS
      void RegisterMidasInstance(TMidasControl* midas);
      void RegisterMidasInstanceOffline(TMidasControl* midas);
#endif
  
      //Access to the underlying file pointer for mapping purposes
      z_off_t GetCurrentFilePosition() { return gztell(fgzRawDataPtr); }
     
   private:

      //Data objects
      AdminData*   fAdminPtr;
      HistoryData* fHistoryPtr;
      TriggerData* fTriggerPtr;
      GPSData*     fGPSPtr;
      DetectorConfigData* fDetectorConfigPtr;
      vector<PulseData>* fListOfVetoPulsesPtr;
      vector<PulseData>* fListOfNoiseMonitorPulsesPtr;
      map< int, vector<PulseData> >* fMapOfZipPulsesPtr; 
      vector<PulseData>  fListOfZipPulses;  //for RawDataReader use only

      //verbosity and printing
      bool fdiagnosticPrints;
      int fverbosity;

      //data flags - initialized to false
      bool fReadDetectorConfig;
      bool fReadAdmin; 
      bool fReadHistory; 
      bool fReadTrigger; 
      bool fReadGPS; 
      bool fReadZipPulses;
      bool fReadVetoPulses;
      bool fReadNoiseMonitorPulses;
      
      // flag for raw data modification
      bool fIsZipPulsesModified;
  

      //for i/o manipulation
      gzFile  fgzRawDataPtr;
      string  fRawDataPath;
      bool    fByteCheckDone; //initialized to false
      bool    fFlipBytes;     //initialized to false
       
      //all in bytes
      uint32_t fCurrentEventPosition;
      uint32_t fNextEventPosition;
      uint32_t fEventLength; 
      uint32_t fCurrentRecordPosition;    //after record header read 
      uint32_t fNextRecordHeaderPosition; //before record header read
      
      //event header info
      uint32_t fEventCategory;
      uint32_t fEventType;
  
  
      // MIDAS files
      bool fIsMidasData;
      bool fIsMidasOnline;
      bool fGotSeriesInfoFromFilename;
      MidasEventData fMidasEvent;
      int fNbMidasEventTriggers;
      uint32_t fCurrentMidasEventTrigger;
      uint32_t fMidasEventNumber;
      uint32_t fCurrentEventNumber;
      uint64_t fMidasSeriesNumber;
      uint32_t fMidasDumpNumber;
     
      //  GUI 
      TSystem *fgSystem;
    
      // Midas online
#ifdef HAVE_MIDAS
      TMidasControl* fMidas;
      VirtualOdbRW* fODB;
#endif 


      //member functions
      //make public so event mapping can use it
 public:
      int  ReadEventHeader(bool dispflag);
      int  ReadMidasEventHeader(bool dispflag);

      uint32_t GetNextRecordID(bool dispflag);
      //others necessary for event mapping
      uint32_t GetCurrentEventPosition(){ return fCurrentEventPosition; }
      uint32_t GetNextEventPosition(){ return fNextEventPosition; }
      uint32_t GetEventLength(){ return fEventLength; }
      uint32_t GetCurrentRecordPosition(){ return fCurrentRecordPosition; }   //after record header read 
      uint32_t GetNextRecordHeaderPosition(){ return fNextRecordHeaderPosition; }//before record header read
      gzFile&  GetRawDataPtr(){ return fgzRawDataPtr; }

 private:
      void StorePulsesByDetCode(PulseData& tempPulseData, int& zipEndCode, int &vetoEndCode);
      void FillMapOfZipPulses();

      void SetEndianParam(); //sets flipBytes parameter for all readers

      //side computations
      bool ParseSeriesAndDumpFromRawPath();
       
};

#endif /* RAWDATAREADER_H */
