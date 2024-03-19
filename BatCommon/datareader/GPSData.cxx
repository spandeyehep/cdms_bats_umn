///////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Name: GPSData
//Author: L. Hsu
//Description:  This is a container class which stores data from the GPS record in the raw data file.   
//The external data are filled directly by this class, which knows how to parse the raw record.
//
// The following information is written in an elog note by Long on the meaning of the status (S) bits
// S-bit cluster = 0 b3 b2 b1
//  -------------------------------------------------------
//  upper most bit 4 = 0 always
//  b3 (bit 3) = 1 if frequency offset > 5E7 (0 otherwise)
//  b2 (bit 2) = 1 if time offset > 5 us     (0 otherwise)
//  b1 (bit 1) = 1 if in flywheeling mode    (0 if IRIG-B present)
//
//  if b1 is set, this means the unit has lost its lock
//
//File Import By: L. Hsu
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
//Feb. 2011 - This class was renamed from ExternalData to GPSData to avoid confusion 
//            with the classes in the extfile_management directory. [LLH]
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include "zlib.h"

#include "BatRootTypes.h"
#include "GPSData.h"

using namespace std;

////////////////////////////////////////////////////////

GPSData::GPSData() :
  fStoreRQs(true),
  fFlipBytes(false)
{
   Reset();
}

GPSData::~GPSData()
{
//   cout <<"Goodbye from GPSData()" << endl;
}

void GPSData::Reset()
{
   //All data members to be filled from 
   //raw data records are reset here

   //loop through RQ list and reset to initial values - we DO NOT clear out this map between events!
   double initVal = -999999.; 
   map<string,double>::iterator rqListItr = fRQList.begin();
   for( ; rqListItr!=fRQList.end(); rqListItr++)
   {
      rqListItr->second = initVal;
   }

   return;
}

//This method constructs the RQ list that is handed off to BatOutputManager
//It also sets the default value for the RQ to initVal.
void GPSData::ConstructRQList()
{
   double initVal = -999999.;

   fRQList.insert(pair<string,double>("GPS_y", initVal));
   fRQList.insert(pair<string,double>("GPS_d", initVal));
   fRQList.insert(pair<string,double>("GPS_h", initVal));
   fRQList.insert(pair<string,double>("GPS_m", initVal));
   fRQList.insert(pair<string,double>("GPS_s", initVal));
   fRQList.insert(pair<string,double>("GPS_ticks", initVal));
   fRQList.insert(pair<string,double>("GPS_status", initVal));

   return;
}

// ========================= Raw Data Read ======================================

//it is the responsibility of the calling routine to ensure
//the file ptr is positioned at the start of the GPS data block 
void GPSData::ReadGPS(gzFile& localRawDataPtr, uint32_t recordLength, bool debugOn)
{

   //Extract the information in the data block
   uint32_t bufferLength = recordLength/BatRootTypes::kWordSize; //this better be an integer
   uint32_t buffer[bufferLength]; 
   int readCheck = gzread(localRawDataPtr,(char*)buffer, bufferLength*sizeof(uint32_t));

   if(readCheck < 0)
   {
      cerr <<"GPSData::ERROR reading GPS data block!" << endl;
      exit(1);
   }
   
   if(debugOn)
   {
      cout <<"External Data buffer length = " << bufferLength << endl;
      printf("word1 = %#x\n", buffer[0]);
      printf("word2 = %#x\n", buffer[1]);
      printf("word3 = %#x\n", buffer[2]);
   }   

   //Extract 
   fYear = (buffer[0] & 0xffff0000) >> 16;
   fYear = ((fYear & 0xf000) >> (4*3))*1000 + ((fYear & 0xf00) >> (4*2))*100 + ((fYear & 0xf0) >> (4*1))*10 + (fYear & 0xf);

   fDay  = buffer[0] & 0xffff;
   fDay = ((fDay & 0xf000) >> (4*3))*1000 + ((fDay & 0xf00) >> (4*2))*100 + ((fDay & 0xf0) >> (4*1))*10 + (fDay & 0xf);

   fStatus = (buffer[1] & 0xf0000000) >> 28; 

   fHour = (buffer[1] & 0xff0000) >> 16;
   fHour = ((fHour & 0xf0) >> (4*1))*10 + (fHour & 0xf);

   fMinute = (buffer[1] & 0xff00) >> 8;
   fMinute = ((fMinute & 0xf0) >> (4*1))*10 + (fMinute & 0xf);

   fSecond = buffer[1] & 0xff;
   fSecond = ((fSecond & 0xf0) >> (4*1))*10 + (fSecond & 0xf);

   fTicks = buffer[2];
   fTicks = ((fTicks & 0xf0000000) >> (4*7))*10000000 + ((fTicks & 0xf000000) >> (4*6))*1000000 
      + ((fTicks & 0xf00000) >> (4*5))*100000 + ((fTicks & 0xf0000) >> (4*4))*10000 
      + ((fTicks & 0xf000) >> (4*3))*1000 + ((fTicks & 0xf00) >> (4*2))*100 + ((fTicks & 0xf0) >> (4*1))*10 + (fTicks & 0xf);

   if(debugOn)
   {
      printf("year = %d\n", fYear);
      printf("day = %d\n", fDay);
      printf("hour = %d\n", fHour);
      printf("minute = %d\n", fMinute);
      printf("second = %d\n", fSecond);
      printf("ticks = %d\n", fTicks);
   }

   //store values in RQ list
   if(fStoreRQs) {
      fRQList["GPS_y"]  = fYear;
      fRQList["GPS_d"]  = fDay;
      fRQList["GPS_h"]  = fHour;
      fRQList["GPS_m"]     = fMinute;
      fRQList["GPS_s"]    = fSecond;
      fRQList["GPS_ticks"]    = fTicks;
      fRQList["GPS_status"]    = fStatus;
   }

   return;
}

