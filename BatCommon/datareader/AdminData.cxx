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

#include <iostream>
#include "zlib.h"

#include "BatRootTypes.h"
#include "AdminData.h"

using namespace std;

////////////////////////////////////////////////////////

AdminData::AdminData() :
  fStoreRQs(true),
  fFlipBytes(false)
{
   Reset();

//   cout <<"Hello from AdminData()" << endl;

}

AdminData::~AdminData()
{
//   cout <<"Goodbye from AdminData()" << endl;
}

void AdminData::Reset()
{
   //All data members to be filled from 
   //raw data records are reset here
   
//   cout <<"In AdminData::Reset()!" << endl;

   fSeries = 0;
   fEvent = 0;
   fTimeBetween = 0;
   fLiveTime = 0;
   fEventTime = 0;
   
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
void AdminData::ConstructRQList()
{
   double initVal = -999999.;

   fRQList.insert(pair<string,double>("EventNumber", initVal));
   fRQList.insert(pair<string,double>("SeriesNumber", initVal));
   fRQList.insert(pair<string,double>("TimeBetween", initVal));
   fRQList.insert(pair<string,double>("LiveTime", initVal));
   fRQList.insert(pair<string,double>("EventTime", initVal));

   return;
}

//it is the responsibility of the calling routine to ensure
//the file ptr is positioned at the start of the admin data block 
void AdminData::ReadRawDataRecord(gzFile& localRawDataPtr, uint32_t recordLength, uint32_t recordID,
				  bool dispflag)
{
   //Extract the information in the data block
   uint32_t bufferLength = recordLength/BatRootTypes::kWordSize; //this better be an integer
   uint32_t buffer[bufferLength]; 
   int readCheck = gzread(localRawDataPtr,(char*)buffer, bufferLength*sizeof(uint32_t));

   if(readCheck < 0)
   {
      cerr <<"AdminData::ERROR reading admin data block!" << endl;
      exit(1);
   }

   if(fFlipBytes)
   {
 
      //for data taken prior to supertower runs at Soudan (summer 2009)
      if(recordID == BatRootTypes::kAdminRecordID)
      {
	 fSeries = EndianHelper::Swap4ByteWord(buffer[0]);
	 fEvent = EndianHelper::Swap4ByteWord(buffer[1]);
	 fEventTime = EndianHelper::Swap4ByteWord(buffer[2]);
	 fTimeBetween = EndianHelper::Swap4ByteWord(buffer[3]);
	 fLiveTime = EndianHelper::Swap4ByteWord(buffer[4]);
      }

      //for data taken after start of supertower runs at Soudan (summer 2009)
      if(recordID == BatRootTypes::kAdminRecordID64)
      {
	 //for extra long series, splice together word 0 & word 1
	 fSeries = (uint64_t)EndianHelper::Swap4ByteWord(buffer[0])*10000 + (uint64_t)EndianHelper::Swap4ByteWord(buffer[1]); 

	 fEvent = EndianHelper::Swap4ByteWord(buffer[2]);
	 fEventTime = EndianHelper::Swap4ByteWord(buffer[3]);
	 fTimeBetween = EndianHelper::Swap4ByteWord(buffer[4]);
	 fLiveTime = EndianHelper::Swap4ByteWord(buffer[5]);
      }
   }
   else
   {
      //before supertowers
      if(recordID == BatRootTypes::kAdminRecordID)
      {
	 fSeries = buffer[0];  
	 fEvent = buffer[1];
	 fEventTime = buffer[2];
	 fTimeBetween = buffer[3];
	 fLiveTime = buffer[4];
      }

      //after supertowers
      if(recordID == BatRootTypes::kAdminRecordID64)
      {

	 //for extra long series, splice together word 0 & word 1
	 fSeries = (uint64_t)buffer[0]*10000 + (uint64_t)buffer[1];; 

	 fEvent = buffer[2];
	 fEventTime = buffer[3];
	 fTimeBetween = buffer[4];
	 fLiveTime = buffer[5];

      }
   }

   if(dispflag) 
      printf("series check %.16lld\n", (unsigned long long)fSeries);

//     cout <<"series = " << fSeries
//    	<<"\nevent = " << fEvent
//    	<<"\neventTime = " << fEventTime
//    	<<"\ntimeSinceLastEvent = " << fTimeBetween
//    	<<"\nlivetimeSinceLastEvent = " << fLiveTime
//  	<<"\nrecordLength = " << recordLength
//   	<< endl;

   //store values in RQ list
   if(fStoreRQs) {
     fRQList["EventNumber"]  = fEvent ;
     fRQList["SeriesNumber"] = fSeries;
     fRQList["TimeBetween"]  = fTimeBetween;
     fRQList["LiveTime"]     = fLiveTime;
     fRQList["EventTime"]    = fEventTime;
   }

   return;
}





/// set Admin data (only possible if everything empty)
void AdminData::SetRawDataRecord(uint64_t series, uint32_t event, uint32_t timeBetween , uint32_t liveTime , uint32_t eventTime)
{

  if (!(fSeries == 0 && fEvent == 0 && fTimeBetween == 0 && fLiveTime == 0 && fEventTime == 0))
    {
      cerr << "AdminData::SetRawDataRecord:  ERROR: No modification of existing AdminData object allowed. " << endl;
      exit(1);
    }
  

  fSeries = series;
  fEvent = event;
  fTimeBetween = timeBetween;
  fLiveTime = liveTime;
  fEventTime = eventTime;


  //store values in RQ list
  if(fStoreRQs) {
    fRQList["EventNumber"]  = fEvent ;
    fRQList["SeriesNumber"] = fSeries;
    fRQList["TimeBetween"]  = fTimeBetween;
    fRQList["LiveTime"]     = fLiveTime;
    fRQList["EventTime"]    = fEventTime;
  }
  
  return;
}
