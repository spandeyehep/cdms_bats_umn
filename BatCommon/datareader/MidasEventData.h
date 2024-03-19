///////////////////////////////////////////////////////////////////////
//
//  Class Name:         MidasEventData
// 
//  Author (s):         B. Serfass using 
//                           TMidasEvents (Konstantin Olchanski - TRIUMF)
//                           TTowerData (dcrcDisplay) (Thomas Lindner - TRIUMF)
//
//  Description:       C++ class representing one midas event. Data members are filled 
//                     by reading midas events from a file or by reading
//                     them from a midas shared memory buffer 
//
//
///////////////////////////////////////////////////////////////////////


#ifndef MIDASEVENTDATA_H
#define MIDASEVENTDATA_H


#include "zlib.h"
#include <vector>
#include <map>

#include "MidasStructs.h"
#include "MidasTriggerData.h"
#include "PulseData.h"

#ifdef HAVE_MIDAS
#include "TMidasControl.h"
#include "VirtualOdbRW.h"
#endif

class MidasEventData
{
 public:

  // Houskeeping functions

  MidasEventData(); ///< default constructor
  MidasEventData(const MidasEventData &); ///< copy constructor
  ~MidasEventData(); ///< destructor
  MidasEventData& operator=(const MidasEventData &); ///< assignement operator
  void Clear(); ///< clear event for reuse
  void Copy(const MidasEventData &); ///< copy helper
  void Print(const char* option = "") const; ///< show all event information

  //configure verbosity and diagnostic printing
  bool GetDiagnosticPrints(){return fdiagnosticPrints;}
  void SetDiagnosticPrints(bool value){fdiagnosticPrints=value;}
  int GetVerbosity(){return fverbosity;}
  void SetVerbosity(int value){fverbosity=value;}

  // Get event information

  uint16_t GetEventId() const {return fEventHeader.fEventId;} ;      //  Event id
  uint16_t GetTriggerMask() const {return fEventHeader.fTriggerMask;};     // Trigger mask
  uint32_t GetSerialNumber() const {return fEventHeader.fSerialNumber;} ; // Serial number (=Event Number)
  uint32_t GetTimeStamp() const {return fEventHeader.fTimeStamp;} ; // Time stamp (unix time in seconds)
  uint32_t GetDataSize() const { return fEventHeader.fDataSize;}; // Event size
  int GetEventCategory(int triggerNumber);
  
  int GetNbTriggers() const {return fNbTrigger;}; // Number of triggers
  vector<PulseData> GetPulseDataList(int triggerNumber);
  int IsStaleTrigger( int triggerNumber); //is this trigger stale, yuck!

  // Read event from file
  int ReadEventHeader(gzFile& localRawDataPtr);
  int ReadDataRecord(gzFile& localRawDataPtr);
  
  // Read Event Online (only if Midas library available)
#ifdef HAVE_MIDAS
  int ReadEventOnline(TMidasControl* midas);    
#endif
  
  
  // helpers for event creation
  TMidas_EVENT_HEADER* GetEventHeader(); ///< return pointer to the event header
  bool IsGoodSize() const; ///< validate the event length
   
  
 private:
  
 
   // Data Banks / Helpers for event creation 
  char* GetData(); ///< return pointer to the data buffer
  void AllocateData(); ///< allocate data buffer using the existing event header
  void SetData(uint32_t dataSize, char* dataBuffer); ///< set an externally allocated data buffer
  bool TriggerMatch(MidasTriggerData *triggerData,int trg_src_tower,int trg_src_dcrc,int trg_word);
  
  const char* GetBankList() const; ///< return a list of data banks
  int FindBank(const char* bankName, int* bankLength, int* bankType, void **bankPtr) const;
  int LocateBank(const void *unused, const char* bankName, void **bankPtr) const;
  
  bool IsBank32() const; ///< returns "true" if event uses 32-bit banks
  int IterateBank(TMidas_BANK **, char **pdata) const; ///< iterate through 16-bit data banks
  int IterateBank32(TMidas_BANK32 **, char **pdata) const; ///< iterate through 32-bit data banks
  
  int SetBankList(); ///< create the list of data banks, return number of banks

  
  // save pulse data into PulseData vector
  bool HandleBankData(); // send midas event buffer to RevC or RevD functions
  bool HandleBankDataRevCPreBackport(const uint32_t*); // save RevC pulse data into PulseData vector
  bool HandleBankData(const uint32_t*); // save RevD pulse data into PulseData vector


  // Byte swap (FIXME: use cdmsbats routines?)
  void SwapBytesEventHeader(); ///< convert event header between little-endian (Linux-x86) and big endian (MacOS-PPC) 
  int  SwapBytes(bool); ///< convert event data between little-endian (Linux-x86) and big endian (MacOS-PPC) 
  

  // Data member
  TMidas_EVENT_HEADER fEventHeader; ///< event header
  char* fData;     ///< event data buffer
  int  fBanksN;    ///< number of banks in this event
  char* fBankList; ///< list of bank names in this event
 
  //verbosity and printing
  bool fdiagnosticPrints;
  int fverbosity;
  
  // Number of Trigger
  int fNbTrigger;

  // Trigger Data 
  vector<MidasTriggerData*> fTriggerDataList;
  //vector<std::unique_ptr<MidasTriggerData>> fTriggerDataList;
  MidasTriggerData* fCurrentTriggerData;
  MidasTriggerData* fCurrentTriggerDataCopy;
  
  //Midas online
  int fRequestId;
  int fCurrentEventNumber;

  //Debugging
  bool fDebugOn;

};

#endif // MidasEventData.h
