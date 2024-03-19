///////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Name: HistoryData
//Author: L. Hsu
//Description:  This is a container class which stores data from the History record in the raw data file.   
//The history data are filled directly by this class, which knows how to parse the raw record.  This class
//also performs the mask compression routines. Note that MaskTimeCompressI and MaskTimeCompressII are
//attempts to port over the MaskTimeCompress routines from DarkPipe.
//
// ******************************************************************************************
// The final trigger bit structure after calling MaskCompressTrigger (for zip masks) is:
// Bit 0 = Z1Qhigh
// Bit 1 = Z1Qlow
// Bit 2 = Z1Phigh
// Bit 3 = Z1Plow
// Bit 4 = Z1Wisper
// Bit 5 = Z2Qhigh
// ...
// Bit 28 = Z6Plo
// Bit 29 = Z6Wisper
// Bit 30 = *unused*
// Bit 31 = *unused*
// Bit 32 = ST_TE
// Bit 33 = ISR
// Bit 34 = Global
// Bit 35 = Random
//
// ******************************************************************************************
// The final trigger bit structure after calling MaskCompressVeto (for veto masks) is:
// Bit 0 = Global Trigger
// Bit 1 = Veto OR of first 20 veto channels
// Bits 2-21 Veto Channels 1-20
// Bit 22 = Global Trigger
// Bit 23 = Veto OR of all 40 veto channels (? not sure if this is correct, copied from DarkPipe)
// Bit 24-43 Veto Channels 21-40
// Bit 44 Veto OR of first 20 veto channels 
//
// ******************************************************************************************
//
//File Import By: L. Hsu
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef HISTORYDATA_H
#define HISTORYDATA_H

#include "zlib.h"
#include <map>
#include <vector>

#include "BatRootTypes.h"
#include "EndianHelper.h"

//!This class reads the history buffer record and stores the info. It also performs mask compression routines (see header file for more info).
class HistoryData 
{
   public:

      HistoryData();  //constructor

      ~HistoryData(); //destructor

      // ===== Get Data Members =====

      uint64_t GetEventTowerMask(const int towerNum); //get mask for that tower at time t=0

      bool IsEventQLowOrHigh(const int detNum, const int eventWindow); //true if qlow/high is triggered in window around event
      bool IsEventPLowOrHigh(const int detNum, const int eventWindow); //true if qlow/high is triggered in window around event

      //old DarkPipe veto RQ's
      double GetVTPostTime();      //time (seconds) of first veto hit following trigger (i.e. VTTime21*10e-6)
      double GetVTPreTime();   //time (seconds) of last veto hit at or before trigger (i.e. VTTime20*10e-6)
      double GetVT2PreTime();  //time (seconds) of penultimate veto hit at or before trigger (i.e. VTTime19*10e-6)
      uint64_t GetVTPreMask(); //mask describing which veto panels were hit at or before trigger & any other veto hits within 2 us before

      uint32_t GetErrorMask();
      int      GetNTrigP(int detNum); //stores the number of plo triggers within [-100, 1000] us of the global trigger
      int      GetNTrigQ(int detNum); //stores the number of qlo triggers within [-100, 1000] us of the global trigger
      uint64_t GetORMasksFrom(double minTime, double maxTime, int tower); //OR's all masks between specified times on specifed tower, times given in microseconds

      // ===== For reading raw data =====

      void Reset();             //clears data members for next filling

      //contents of the history record will specify which of these is called
      void ReadSoudanHistoryRecord(gzFile& localRawDataPtr, uint32_t recordLength, bool dispHeader);        
      void ReadSUFHistoryRecord(gzFile& localRawDataPtr, uint32_t recordLength, bool dispHeader);        
      
      //flipping bytes for opposite endianness
      void SetFlipBytes(bool flipCheck) { fFlipBytes = flipCheck; return; }

      //for querying RQ lists - provies read only access!
      void ConstructRQList(const int nTowers);
      map<string, double> GetRQList() const { return fRQList; }
      double GetRQVal(const string& rqName) const;
      
   private:
            
      void MaskCompressTrigger(uint64_t& oddMask, uint64_t& evenMask); //rearranges information in bits 30, 31 between event and odd
      uint64_t MaskCompressVeto(vector<uint32_t>& vetoMasks); //combines two small masks into one large one, returns combined mask
      
      void MaskTimeCompressI(vector<double>& timeVector, vector<uint64_t>& maskVector);
      void MaskTimeCompressII(vector<double>& timeVector, vector<uint64_t>& maskVector);
      void MaskTimeCompressIII(vector<double>& timeVector, vector<uint64_t>& maskVector);
      
      bool CheckBit(const uint64_t& word, const int& bit); //checks specified bit in word
      void PrintBinary(const uint64_t& word);
       
      //member data
      double fEventTrigTime;
      double fEventTrigMask;

      //veto
      vector<double>             fBeforeEventVetoTimes; //also stores t=0 time if it exists 
      vector<uint64_t>           fBeforeEventVetoMasks; //also stores t=0 mask if it exists

      vector<double>             fAfterEventVetoTimes; 
      vector<uint64_t>           fAfterEventVetoMasks;
      
      bool                       fVetoBufferOverflow;   //check before MaskTimeCompress is performed

      //zip
      vector< vector<double> >   fBeforeEventTriggerTimes; 
      vector< vector<uint64_t> > fBeforeEventTriggerMasks; //vector of vectors (each tower/word has a vector of times)

      vector< vector<double> >   fAfterEventTriggerTimes; 
      vector< vector<uint64_t> > fAfterEventTriggerMasks; //vector of vectors (each tower/word has a vector of times)

      double                     fEventTriggerTime; 
      vector<uint64_t>           fEventTriggerMasks;

      //RQ list
      map<string, double> fRQList;
      bool fStoreRQs;
      int  fNTowers;
      int  fMaxZips;

      //endian reading and bitwise manipulations
      bool        fFlipBytes;              //initilized to false
      
};

#endif /* HISTORYDATA_H */
