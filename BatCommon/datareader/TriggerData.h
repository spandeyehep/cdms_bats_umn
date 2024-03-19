///////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Name: TriggerData
//Author: L. Hsu
//Description:  This is a container class which stores data from the Trigger record in the raw data file.   
//The trigger data are filled directly by this class, which knows how to parse the raw record.
//
//File Import By: L. Hsu
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef TRIGGERDATA_H
#define TRIGGERDATA_H

#include "zlib.h"
#include <map>
#include <vector>

#include "BatRootTypes.h"
#include "EndianHelper.h"

//!This class reads the trigger record and stores the info
class TriggerData 
{
   public:

      TriggerData();  //constructor

      ~TriggerData(); //destructor

      //Get Data Members
      uint64_t GetTowerMask(const int towerNum); 
      
      size_t GetNTowerMasksInEvent() { return fTriggerMasks.size(); }
      
      std::string GetTriggerInfoTable();
      
      //For reading raw data
      void Reset();             //clears data members for next filling
      void ReadTrigger(gzFile& localRawDataPtr, uint32_t recordLength, bool dispHeader);        
      
      //flipping bytes for opposite endianness
      void SetFlipBytes(bool flipCheck) { fFlipBytes = flipCheck; return; }

      //for querying RQ lists - provies read only access!
      void ConstructRQList(const int nTowers);
      map<string, double> GetRQList() const { return fRQList; }
      
      ///Enum defining trigger types (based on HistoryData.h comments)
      enum TRIGGER_BITS{
	kQhigh               = 0,
	kQlow                = 1<<1,
	kPhigh               = 1<<2,
	kPlow                = 1<<3,
	kWisper              = 1<<4,
	kNTrigTypeBits       = 5,
	kMaxDibsPerTower     = 6,
	kST_TE               = (uint64_t)1<<32,
	kISR                 = (uint64_t)1<<33,
	kGlobal              = (uint64_t)1<<34,
	kRandom              = (uint64_t)1<<35,
	kNGlobalTrigTypeBits = 4,
      };
      
   private:
      
      bool CheckBit(const uint32_t& word, const int& bit); //checks specified bit in word
      void MaskCompress(uint64_t& oddMask, uint64_t& evenMask);

      void PrintBinary(const uint64_t& word); //only for debugging
       
      //member data
      vector<uint64_t> fTriggerMasks;

      //RQ list
      map<string, double> fRQList;
      bool fStoreRQs;
      int  fNTowers;

      //endian reading and bitwise manipulations
      bool        fFlipBytes;              //initilized to false
      
};

#endif /* TRIGGERDATA_H */
