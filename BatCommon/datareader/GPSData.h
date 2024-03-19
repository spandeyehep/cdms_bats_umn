///////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Name: GPSData
//Author: L. Hsu
//Description:  This is a container class which stores data from the External (GPS) record in the raw data file.   
//The external data are filled directly by this class, which knows how to parse the raw record.
//
//File Import By: L. Hsu
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GPSDATA_H
#define GPSDATA_H

#include "zlib.h"
#include <map>
#include <vector>

#include "BatRootTypes.h"
#include "EndianHelper.h"

//!This class reads the external record and stores the info
class GPSData 
{
   public:

      GPSData();  //constructor

      ~GPSData(); //destructor

      //Get Data Members

      //For reading raw data
      void Reset();             //clears data members for next filling
      void ReadGPS(gzFile& localRawDataPtr, uint32_t recordLength, bool dispHeader);        
      
      //flipping bytes for opposite endianness
      void SetFlipBytes(bool flipCheck) { fFlipBytes = flipCheck; return; }

      //for querying RQ lists - provies read only access!
      void ConstructRQList();
      map<string, double> GetRQList() const { return fRQList; }
      
   private:
      
      //member data
      uint32_t fYear;
      uint32_t fDay;
      uint32_t fHour;
      uint32_t fMinute;
      uint32_t fSecond;
      uint32_t fTicks;
      uint32_t fStatus; //GPS lock status, added starting with series 170921_1511
  
      //RQ list
      map<string, double> fRQList;
      bool fStoreRQs;
  
      //endian reading and bitwise manipulations
      bool        fFlipBytes;              //initilized to false

};

#endif /* GPSDATA_H */
