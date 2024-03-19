///////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Name: AdminData
//Author: L. Hsu
//Description:  This is a container class which stores data from the Admin record in the raw data file.   
//The admin data are filled directly by this class, which knows how to parse the raw record.
//
//File Import By: L. Hsu
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ADMINDATA_H
#define ADMINDATA_H

#include "zlib.h"
#include <map>

#include "EndianHelper.h"

//!This class reads the admin record and stores the info
class AdminData 
{
   public:

      AdminData();  //constructor

      ~AdminData(); //destructor

      //Get Data Members
      uint64_t GetSeries() const                 { return fSeries; }  
      uint32_t GetEvent() const                  { return fEvent;  }  
      uint32_t GetTimeBetween() const            { return fTimeBetween; }  
      uint32_t GetLiveTime() const               { return fLiveTime; }  
      uint32_t GetEventTime() const              { return fEventTime; }  

      //For reading raw data
      void Reset();             //clears data members for next filling
      void ReadRawDataRecord(gzFile& localRawDataPtr, uint32_t recordLength, uint32_t recordID, bool dispHeader);        
      
      //flipping bytes for opposite endianness
      void SetFlipBytes(bool flipCheck) { fFlipBytes = flipCheck; return; }
      

      // For setting data (only if data empty)
      void SetRawDataRecord(uint64_t series, uint32_t event, uint32_t timeBetween , uint32_t liveTime , uint32_t eventTime);


      //for querying RQ lists - provies read only access!
      void ConstructRQList();
      map<string, double> GetRQList() const { return fRQList; }

   private:
       
      //member data
      uint64_t    fSeries;
      uint32_t    fEvent;                  //within this series
      uint32_t    fTimeBetween;            //milliseconds
      uint32_t    fLiveTime;               //milliseconds
      uint32_t    fEventTime;              //seconds from epoch - truncated to nearest second

      //RQ list
      map<string, double> fRQList;
      bool fStoreRQs;

      //endian reading and bitwise manipulations
      bool        fFlipBytes;              //initilized to false
            
};

#endif /* ADMINDATA_H */
