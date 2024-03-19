///////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Name: HistoryData
//Author: L. Hsu
//Description:  This is a container class which stores data from the History record in the raw data file.   
//The history data are filled directly by this class, which knows how to parse the raw record. This class
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
// Bit 23 = Veto OR of all 40 veto channels 
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

#include <iostream>
#include "zlib.h"
#include <iomanip>
#include <cmath>
#include <sstream>

#include "BatRootTypes.h"
#include "HistoryData.h"

using namespace std;

////////////////////////////////////////////////////////

//this variable controls where the noise monitor triggers are plugged in
//If it is changed, additional RQs will be generated
//VT masks in raw data are stored in 2 separate 32-bit words;
//first 32bits of fNoiseMonBits are the first board, last are second board
static const uint64_t fNoiseMonBits = (1ULL<<55); 
//^bit 23 on second board

HistoryData::HistoryData() :
  fVetoBufferOverflow(false),
  fStoreRQs(true),
  fNTowers(BatRootTypes::kEmptyVariable),
  fFlipBytes(false)
{
   Reset();
}

HistoryData::~HistoryData()
{
//   cout <<"Goodbye from HistoryData()" << endl;
}

void HistoryData::Reset()
{
   //All data members to be filled from 
   //raw data records are reset here

   // ========== veto ===========
   fBeforeEventVetoTimes.clear();
   fBeforeEventVetoMasks.clear();
   fAfterEventVetoTimes.clear();
   fAfterEventVetoMasks.clear();

   fVetoBufferOverflow = false;
   // ========= zip  ==========
   fBeforeEventTriggerTimes.clear();
   fBeforeEventTriggerMasks.clear();
   fAfterEventTriggerTimes.clear();
   fAfterEventTriggerMasks.clear();

   fEventTriggerTime = BatRootTypes::kEmptyVariable;
   fEventTriggerMasks.clear();

   //loop through RQ list and reset to initial values - we DO NOT clear out this map between events!
   double initVal = BatRootTypes::kEmptyVariable; 
   map<string,double>::iterator rqListItr = fRQList.begin();
   for( ; rqListItr!=fRQList.end(); rqListItr++)
   {
     rqListItr->second = initVal;
   }

   return;
}

//This method constructs the RQ list that is handed off to BatOutputManager
//It also sets the default value for the RQ to initVal.
void HistoryData::ConstructRQList(const int nTowers)
{
   fNTowers = nTowers;
   double initVal = BatRootTypes::kEmptyVariable;

   //construct the RQ list here

   //veto RQ's
   for(int nth = 14; nth < 24; nth++)
   {

     //FIXME check this once the history buffer is re-enabled!
     stringstream tempRQName1;
     stringstream tempRQName2;

     tempRQName1 <<"VTTime" << nth; 
     tempRQName2 <<"VTMask" << nth; 
      
     fRQList.insert(pair<string,double>(tempRQName1.str(), initVal));
     fRQList.insert(pair<string,double>(tempRQName2.str(), initVal));
   }
   
   //noise monitor RQs
   //for each active bit in the noise monitor mask, store the nearest
   //pre and post times
   for(size_t bit=0; bit<64; ++bit){
     if(fNoiseMonBits & (1ULL<<bit) ){
       stringstream s;
       s<<"NM"<<bit<<"PreTime";
       fRQList.insert(pair<string,double>(s.str(),initVal));
       s.str("");
       s<<"NM"<<bit<<"PostTime";
       fRQList.insert(pair<string,double>(s.str(),initVal));
     }
   }
   
   //zip trigger RQ's
   for(int towerCtr = 1; towerCtr <= fNTowers; towerCtr++)
   {
     for(int nth = 16; nth < 26; nth++)
     {
       stringstream tempRQName1;
       stringstream tempRQName2;

       tempRQName1 <<"T" << towerCtr <<"TGTime" << nth;
       tempRQName2 <<"T" << towerCtr <<"TGMask" << nth;

       fRQList.insert(pair<string,double>(tempRQName1.str(), initVal));
       fRQList.insert(pair<string,double>(tempRQName2.str(), initVal));
     }

     //convenience RQ's for glitch cut
     stringstream tempRQName3;
     stringstream tempRQName4;

     tempRQName3 <<"T" << towerCtr <<"NTrigP";
     tempRQName4 <<"T" << towerCtr <<"NTrigQ";

     fRQList.insert(pair<string,double>(tempRQName3.str(), initVal));
     fRQList.insert(pair<string,double>(tempRQName4.str(), initVal));


     //OR'd masks over specified time ranges
     stringstream tempRQName5;
     stringstream tempRQName6;
     stringstream tempRQName7;

     stringstream tempRQName8; //for -300,200 OR'd
     stringstream tempRQName9; //for -200,-100 OR'd
     stringstream tempRQName10; //for -100,000 OR'd
     stringstream tempRQName13;  //for 200,300 OR'd
     stringstream tempRQName12; //for 100,200 OR'd
     stringstream tempRQName11; //for 000,100 OR'd


     tempRQName5 <<"T" << towerCtr <<"TGMask256to10";
     tempRQName6 <<"T" << towerCtr <<"TGMask50to50";
     tempRQName7 <<"T" << towerCtr <<"TGMask100to200";
     
     tempRQName8 <<"T" << towerCtr <<"TGMaskminus300to200";
     tempRQName9 <<"T" << towerCtr <<"TGMaskminus200to100";
     tempRQName10 <<"T" << towerCtr <<"TGMaskminus100to0";
     tempRQName13 <<"T" << towerCtr <<"TGMaskplus200to300";
     tempRQName12 <<"T" << towerCtr <<"TGMaskplus100to200";
     tempRQName11 <<"T" << towerCtr <<"TGMaskplus0to100";

     fRQList.insert(pair<string,double>(tempRQName5.str(), initVal));
     fRQList.insert(pair<string,double>(tempRQName6.str(), initVal));
     fRQList.insert(pair<string,double>(tempRQName7.str(), initVal));

     fRQList.insert(pair<string,double>(tempRQName8.str(), initVal));
     fRQList.insert(pair<string,double>(tempRQName9.str(), initVal));
     fRQList.insert(pair<string,double>(tempRQName10.str(), initVal));
     fRQList.insert(pair<string,double>(tempRQName11.str(), initVal));
     fRQList.insert(pair<string,double>(tempRQName12.str(), initVal));
     fRQList.insert(pair<string,double>(tempRQName13.str(), initVal));
   }

   //general history buffer RQ's
   fRQList.insert(pair<string,double>("ErrorMask", initVal));


   //Any RQ that is included in the above list will be written out by BatRoot.  Add to this as you please.

   return;
}

double HistoryData::GetRQVal(const string& rqName) const
{
   double val = BatRootTypes::kEmptyVariable;

   if(fRQList.count(rqName) == 0)
   {
      cout <<"HistoryData::GetRQVal ERROR!  Requested RQ, " << rqName
	   <<", is not found, please check your code" << endl;
      exit(1);
   }

   map<string, double>::const_iterator rqItr = fRQList.begin();
   for( ; rqItr != fRQList.end(); rqItr++)
   {
      if(rqItr->first == rqName)
	 return rqItr->second;
   }

   return val;
}

// ========================= Get Functions ======================================

uint64_t HistoryData::GetEventTowerMask(const int towerNum)
{
  if(fEventTriggerMasks.size() == 0)
  {
    cout <<"HistoryData::GetEventTowerMask ERROR! Attempting to get tower mask, but it appears event has not been read yet.  Check your code." << endl;
    exit(1);
  }  

  return fEventTriggerMasks[towerNum-1];
}

//a routine to simplify calculation of glitch cut
//counts number of plo bits within preset window around global trigger
//WARNING - this routine expects 6 zips per tower
int HistoryData::GetNTrigP(int detNum)
{ 
   int nTrigP = 0;
   int nZipsPerTower = 6;
   double minTime = -100.; //microseconds
   double maxTime = 1000.; //microseconds

   if(fEventTriggerMasks.size() == 0)
   {
      cout <<"HistoryData::GetNTrigP ERROR! It appears event has not been read yet.  Check your code." 
	   << endl;
      exit(1);
   }  

   //convert detector number into tower and zip number
   int towerNum = (int)ceil((double)detNum/(double)nZipsPerTower); 
   int zipNum = (detNum % nZipsPerTower);

   if(zipNum == 0) zipNum = nZipsPerTower;

   //loop over before trigger times 
   for(uint timeCtr=0; timeCtr < (fBeforeEventTriggerTimes[towerNum-1]).size(); timeCtr++)
   {
//      cout <<"Made it into before time loop!" << endl;

      if( (fBeforeEventTriggerTimes[towerNum-1])[timeCtr] <= maxTime &&
	  (fBeforeEventTriggerTimes[towerNum-1])[timeCtr] >= minTime)
      {
	 if(CheckBit(fBeforeEventTriggerMasks[towerNum-1][timeCtr], 3 + 5*(zipNum-1)))
	 {
	    nTrigP++;
	 }
      }
   }
   
   //loop over after trigger times 
   for(uint timeCtr=0; timeCtr < (fAfterEventTriggerTimes[towerNum-1]).size(); timeCtr++)
   {
//      cout <<"Made it into after time loop!" << endl;

      if( (fAfterEventTriggerTimes[towerNum-1])[timeCtr] <= maxTime &&
	  (fAfterEventTriggerTimes[towerNum-1])[timeCtr] >= minTime )
      {
	 if(CheckBit(fAfterEventTriggerMasks[towerNum-1][timeCtr], 3 + 5*(zipNum-1)))
	 {
	    nTrigP++;
	 }
      }
   }

   //check event time
   if( fEventTriggerTime <= maxTime && fEventTriggerTime >= minTime )
   {
//      cout <<"Made it into event time loop!" << endl;
      if(CheckBit(fEventTriggerMasks[towerNum-1], 3 + 5*(zipNum-1)))
      {
	 nTrigP++;
      }
   }

   return nTrigP;
}

//a routine to simplify calculation of glitch cut
//counts number of qlo bits within preset window around global trigger
//WARNING - this routine expect 6 zips (or dibs) per tower
int HistoryData::GetNTrigQ(int detNum)
{
   int nTrigQ = 0;
   int nZipsPerTower = 6;
   double minTime = -100.; //microseconds
   double maxTime = 1000.; //microseconds

   if(fEventTriggerMasks.size() == 0)
   {
      cout <<"HistoryData::GetNTrigQ ERROR! It appears event has not been read yet.  Check your code." 
	   << endl;
      exit(1);
   }  

   //convert detector number into tower and zip number
   int towerNum = (int)ceil((double)detNum/(double)nZipsPerTower); 
   int zipNum = (detNum % nZipsPerTower);

   if(zipNum == 0) zipNum = nZipsPerTower;

   //loop over before trigger times 
   for(uint timeCtr=0; timeCtr < (fBeforeEventTriggerTimes[towerNum-1]).size(); timeCtr++)
   {
//      cout <<"Made it into before time loop!" << endl;

      if( (fBeforeEventTriggerTimes[towerNum-1])[timeCtr] <= maxTime &&
	  (fBeforeEventTriggerTimes[towerNum-1])[timeCtr] >= minTime)
      {
	 if(CheckBit(fBeforeEventTriggerMasks[towerNum-1][timeCtr], 1 + 5*(zipNum-1)))
	    nTrigQ++;
      }
   }
   
   //loop over after trigger times 
   for(uint timeCtr=0; timeCtr < (fAfterEventTriggerTimes[towerNum-1]).size(); timeCtr++)
   {
//      cout <<"Made it into after time loop!" << endl;

      if( (fAfterEventTriggerTimes[towerNum-1])[timeCtr] <= maxTime &&
	  (fAfterEventTriggerTimes[towerNum-1])[timeCtr] >= minTime )
      {
	 if(CheckBit(fAfterEventTriggerMasks[towerNum-1][timeCtr], 1 + 5*(zipNum-1)))
	    nTrigQ++;
      }
   }

   //check event time
   if( fEventTriggerTime <= maxTime && fEventTriggerTime >= minTime )
   {
//      cout <<"Made it into event time loop!" << endl;
      if(CheckBit(fEventTriggerMasks[towerNum-1], 1 + 5*(zipNum-1)))
	 nTrigQ++;
   }

   return nTrigQ;

}

//no longer stored as an rq
uint64_t HistoryData::GetORMasksFrom(double minTime, double maxTime, int towerNum)
{
  //cout <<"\nHello from GetORMasksFrom, checking tower: " << towerNum << endl;

   uint64_t tempORmask = 0x0;

   if(fEventTriggerMasks.size() == 0)
   {
      cout <<"HistoryData::GetORMasksFrom ERROR! It appears event has not been read yet.  Check your code." 
	   << endl;
      exit(1);
   }  


   //loop over before trigger times 
   for(uint timeCtr=0; timeCtr < (fBeforeEventTriggerTimes[towerNum-1]).size(); timeCtr++)
   {
      if( (fBeforeEventTriggerTimes[towerNum-1])[timeCtr] <= maxTime &&
	  (fBeforeEventTriggerTimes[towerNum-1])[timeCtr] >= minTime)
      {
	tempORmask = tempORmask | fBeforeEventTriggerMasks[towerNum-1][timeCtr];

// 	cout <<"\nMask for time " << (fBeforeEventTriggerTimes[towerNum-1])[timeCtr] <<" ";
// 	PrintBinary(fBeforeEventTriggerMasks[towerNum-1][timeCtr]);
      }
   }
   
   //loop over after trigger times 
   for(uint timeCtr=0; timeCtr < (fAfterEventTriggerTimes[towerNum-1]).size(); timeCtr++)
   {
      if( (fAfterEventTriggerTimes[towerNum-1])[timeCtr] <= maxTime &&
	  (fAfterEventTriggerTimes[towerNum-1])[timeCtr] >= minTime )
      {
	tempORmask = tempORmask | fAfterEventTriggerMasks[towerNum-1][timeCtr];

// 	cout <<"\nMask for time " << (fAfterEventTriggerTimes[towerNum-1])[timeCtr] <<" ";
// 	PrintBinary(fAfterEventTriggerMasks[towerNum-1][timeCtr]);
      }
   }

   //check event time
   if( fEventTriggerTime <= maxTime && fEventTriggerTime >= minTime )
   {
	tempORmask = tempORmask | fEventTriggerMasks[towerNum-1];

// 	cout <<"\nMask for event time ";
// 	PrintBinary(fEventTriggerMasks[towerNum-1]);
   }

//    cout <<"\nIn GetORMasksFrom, after oring trigger masks: " <<endl;
//    PrintBinary(tempORmask);

   return tempORmask;
}


//this routine works if you have 6 zips per tower or 1 tower w/ less than 6 zips
//looks in symmetric window around event time for qlow and qhigh trigger bits
bool HistoryData::IsEventQLowOrHigh(const int detNum, const int eventWindow)
{
  int nZipsPerTower = 6;
  uint checkBits = 0;

  if(fEventTriggerMasks.size() == 0)
  {
    cout <<"HistoryData::IsEventQLowOrHigh ERROR! Attempting to get tower mask, but it appears event has not been read yet.  Check your code." << endl;
    exit(1);
  }  

  //convert detector number into tower and zip number
  int towerNum = (int)ceil((double)detNum/(double)nZipsPerTower); 
  int zipNum = (detNum % nZipsPerTower);

  if(zipNum == 0) zipNum = nZipsPerTower;

  //  printf("\nthe mask = %#x\n", fEventTriggerMasks[towerNum-1]);

  //loop over before trigger times 
  for(uint timeCtr=0; timeCtr < (fBeforeEventTriggerTimes[towerNum-1]).size(); timeCtr++)
  {
     if(abs((fBeforeEventTriggerTimes[towerNum-1])[timeCtr]) <= (double)eventWindow)
     {
	checkBits = ((fBeforeEventTriggerMasks[towerNum-1])[timeCtr] >> (zipNum-1)*5) & 0x3;

	//exit and return true if qlow or qhigh is on
	if(checkBits != 0)
	   return true;
     }
  }

  //loop over after trigger times 
  for(uint timeCtr=0; timeCtr < (fAfterEventTriggerTimes[towerNum-1]).size(); timeCtr++)
  {
     if(abs((fAfterEventTriggerTimes[towerNum-1])[timeCtr]) <= (double)eventWindow)
     {
	checkBits = ((fAfterEventTriggerMasks[towerNum-1])[timeCtr] >> (zipNum-1)*5) & 0x3;

	//exit and return true if qlow or qhigh is on
	if(checkBits != 0)
	   return true;
     }
  }

  //check event time
  checkBits = ( fEventTriggerMasks[towerNum-1] >> (zipNum-1)*5 ) & 0x3;

  if(checkBits != 0)
    return true;

  return false;
}

//this routine works if you have 6 zips per tower or 1 tower w/ less than 6 zips
//looks in symmetric window around event time for plow and phigh trigger bits
bool HistoryData::IsEventPLowOrHigh(const int detNum, const int eventWindow)
{
  int nZipsPerTower = 6;
  uint checkBits = 0;

  if(fEventTriggerMasks.size() == 0)
  {
    cout <<"HistoryData::IsEventPLowOrHigh ERROR! Attempting to get tower mask, but it appears event has not been read yet.  Check your code." << endl;
    exit(1);
  }  

//  cout <<"eventWindow = " << eventWindow << endl;

  //convert detector number into tower and zip number
  int towerNum = (int)ceil((double)detNum/(double)nZipsPerTower); 
  int zipNum = (detNum % nZipsPerTower);

  if(zipNum == 0) zipNum = nZipsPerTower;

  //loop over before trigger times 
  for(uint timeCtr=0; timeCtr < (fBeforeEventTriggerTimes[towerNum-1]).size(); timeCtr++)
  {
     if(abs((fBeforeEventTriggerTimes[towerNum-1])[timeCtr]) <= (double)eventWindow)
     {
	checkBits = ( (fBeforeEventTriggerMasks[towerNum-1])[timeCtr] >> ((zipNum-1)*5+2) ) & 0x3;

//	printf("\nthe time, %e, the mask = %d\n", (fBeforeEventTriggerTimes[towerNum-1])[timeCtr], (fBeforeEventTriggerMasks[towerNum-1])[timeCtr]);

	//exit and return true if qlow or qhigh is on
	if(checkBits != 0)
	   return true;
     }
  }

  //loop over after trigger times 
  for(uint timeCtr=0; timeCtr < (fAfterEventTriggerTimes[towerNum-1]).size(); timeCtr++)
  {
     if(abs((fAfterEventTriggerTimes[towerNum-1])[timeCtr]) <= (double)eventWindow)
     {
	checkBits = ( (fAfterEventTriggerMasks[towerNum-1])[timeCtr] >> ((zipNum-1)*5+2) ) & 0x3;

	//exit and return true if qlow or qhigh is on
	if(checkBits != 0)
	   return true;
     }
  }

  //check event time
  checkBits = ( fEventTriggerMasks[towerNum-1] >> ((zipNum-1)*5+2) ) & 0x3;
  //printf("\nthe event mask = %d\n", (fEventTriggerMasks[towerNum-1]));

  if(checkBits != 0)
    return true;

  return false;

}

// We redefined the error masks from DarkPipe because
// parts of the old DarkPipe code are obsolete.
// All mask bits, except for the veto buffer overflow,
// are set based on history buffer masks and times 
// *after* running the MaskTimeCompress routine
//
// New Error Codes Definition:
// bit0(1) = all trigger (zip) masks at time t=0 are empty
// bit1(2) = no global trigger bit in the trigger (zip) masks at time=0
// bit2(4) = no zip or random trigger or veto multiplicity trigger at time t=0 
// bit3(8) = no global trigger bit in the veto masks at time=0
// bit4(16) = veto OR of all 40 panels is 0 when event was triggered by veto 
// bit5(32) = first veto OR of first 20 panels is not consistent with second OR of *first* 20 panels - only checked for VTMask20
// bit6(64) = veto buffer overflow occurred

uint32_t HistoryData::GetErrorMask()
{
   uint32_t errorMask=0x0;

   // === bit0(1) all trigger (zip) masks at t=0 are empty ===
   uint64_t triggerZeroCheck= 0x0;
   for(int maskItr=0; (maskItr < fNTowers && maskItr < (int)fEventTriggerMasks.size()); maskItr++)
   {
      triggerZeroCheck += fEventTriggerMasks[maskItr];
   }
   if(triggerZeroCheck == 0x0) errorMask += 1;


   // === bit1(2) no global trigger bit in the trigger masks ===
   bool isGlobalBit = false;
   for(int maskItr=0; (maskItr < fNTowers && maskItr < (int)fEventTriggerMasks.size()); maskItr++)
   {
      if(CheckBit(fEventTriggerMasks[maskItr], 34)) isGlobalBit = true;
   }   
   if(!isGlobalBit) errorMask += 2;


   // === bit2(4) no zip or random or veto multiplicity trigger at time t=0 ===
   bool isRandom = false;
   bool isVetoMultiplicity = false;
   uint64_t zipBitSum = 0x0;
   for(int maskItr=0; (maskItr < fNTowers && maskItr < (int)fEventTriggerMasks.size()); maskItr++)
   {
      if(CheckBit(fEventTriggerMasks[maskItr], 35)) isRandom = true;
      if(CheckBit(fEventTriggerMasks[maskItr], 33)) isVetoMultiplicity = true;

      //checking to see if any of the 5 thresholds were crossed in the 6 zips per tower
      zipBitSum += (fEventTriggerMasks[maskItr] & 0x3FFFFFFFUL);  
   }   
   if(zipBitSum == 0x0 && !isRandom && !isVetoMultiplicity) errorMask += 4;

   //checking veto masks for self consistency
   int lastBin = fBeforeEventVetoMasks.size() - 1;

   if(lastBin >=0)
   {
      uint64_t vetoEventMask = fBeforeEventVetoMasks[lastBin];
      double   vetoEventTime = fBeforeEventVetoTimes[lastBin];

      // === bit3(8) no global trigger bit in the veto masks at time=0 ===
      if((CheckBit(vetoEventMask, 0) == 0x0 || CheckBit(vetoEventMask, 22) == 0x0) && vetoEventTime == 0) errorMask += 8;

      // === bit4(16) veto OR of all panels = 0 for veto-triggered event ===
      if(CheckBit(vetoEventMask, 23) == 0x0 && vetoEventTime == 0) errorMask += 16;

      // === bit5(32) = first veto OR of first 20 panels is not consistent with second OR of *first* 20 panels - only checked for VTMask20 
      if(CheckBit(vetoEventMask, 1) != CheckBit(vetoEventMask, 44)) errorMask += 32;
   }

   // === bit6(64) = veto buffer overflow occurred ===
   if(fVetoBufferOverflow) errorMask += 64;

   //for debugging only
//     if(errorMask != 0x0)
//     {
//        cout <<"\nNonzero error Mask  found! " << errorMask << endl;
     
//        for(int itr=0; itr<fNTowers; itr++) { cout <<"\ntrigger mask " << itr << " val = "; PrintBinary(fEventTriggerMasks[itr]); }
       
//        if(lastBin >=0)
//        {
//  	uint64_t vetoEventMask = fBeforeEventVetoMasks[lastBin];
//  	double   vetoEventTime = fBeforeEventVetoTimes[lastBin];
//  	cout <<"\nveto time = " << vetoEventTime <<"\nveto mask val = "; PrintBinary(vetoEventMask);
//        }
//     }

   return errorMask;
}

//no longer stored as an rq
double HistoryData::GetVTPostTime()
{
   //return the first time after trigger (convert from us to seconds)
   double postTime = BatRootTypes::kEmptyVariable;
   if(fAfterEventVetoTimes.size() > 0) postTime = fAfterEventVetoTimes[0]*1e-6;

   return postTime;
}

//no longer stored as an rq
double HistoryData::GetVTPreTime()
{
   //return the last entry, which is last record at or before trigger (convert from us to seconds)
   int lastEntry = fBeforeEventVetoTimes.size()-1;
   double preTime = (lastEntry >= 0 ? fBeforeEventVetoTimes[lastEntry]*1e-6 : BatRootTypes::kEmptyVariable);

   return preTime;
}

//no longer stored as an rq
double HistoryData::GetVT2PreTime()
{
   //return the second to last entry (convert from us to seconds)
   int nextToLastEntry = fBeforeEventVetoTimes.size()-2;
   double secondPreTime = (nextToLastEntry >= 0 ? fBeforeEventVetoTimes[nextToLastEntry]*1e-6 : BatRootTypes::kEmptyVariable);

   return secondPreTime;
}

//no longer  stored as an rq
uint64_t HistoryData::GetVTPreMask()
{
   //get the last entry, which is last record at or before trigger (convert from us to seconds)
   int lastEntry = fBeforeEventVetoTimes.size()-1;
   uint64_t preMask = (lastEntry >=0 ? fBeforeEventVetoMasks[lastEntry] : 0x0);
   double   preMaskTime = (lastEntry >=0 ? fBeforeEventVetoTimes[lastEntry] : 0);  //unecessary, there should be at least 1 entry

   //cout <<"lastEntry = " << lastEntry <<", last time = " << preMaskTime << endl;

   //now OR it with any other masks that occur within 2 seconds of last entry
   for(lastEntry-- ; lastEntry > 0; lastEntry--)
   {
      //cout <<"iterating backwards! lastEntry = " << lastEntry <<", time = " << fBeforeEventVetoTimes[lastEntry] << endl;

      //look for any entries within 2 us of the last one
      if( (preMaskTime - fBeforeEventVetoTimes[lastEntry]) < 2)
      {	 
	 preMask = preMask | fBeforeEventVetoMasks[lastEntry];
//	 cout <<"found an event within 2 seconds of last time, " << fBeforeEventVetoTimes[lastEntry] <<", lastEntry = " << lastEntry << endl;
      }
      else
      {
	 break;
      }
   }

   //now suppress bits 0,22 and 44 (I don't completely understand why this is done, archaic step from DarkPipe
   if(preMask & 0x1) 
      preMask = 0x1^preMask;
     
   if((preMask >> 22) & 0x1 ) 
      preMask = 0x400000UL^preMask;
  
   if((preMask >> 44) & 0x1) 
      preMask = 0x100000000000ULL^preMask;
  
   return preMask;
}

// ========================= Raw Data Read ======================================

//it is the responsibility of the calling routine to ensure
//the file ptr is positioned at the start of the history data block 
void HistoryData::ReadSoudanHistoryRecord(gzFile& localRawDataPtr, uint32_t recordLength, 
					  bool debugOn)
{
   //Extract the information in the data block
   uint32_t bufferLength = recordLength/BatRootTypes::kWordSize; //this better be an integer
   //uint32_t buffer[bufferLength]; //<this isn't legal C++! Not sure how it compiled...
   std::vector<uint32_t> buffer(bufferLength);
   int readCheck = gzread(localRawDataPtr,(char*)(&(buffer[0])), bufferLength*sizeof(uint32_t));

   if(readCheck < 0)
   {
      cerr <<"HistoryData::ERROR reading history data block!" << endl;
      exit(1);
   }
   
   // ========================== Setting up Word Counters ===============================
   
   //get number of veto times
   int nvtCtr = 0;
   int32_t nvt = (fFlipBytes ? EndianHelper::Swap4ByteWord(buffer[nvtCtr]) : buffer[nvtCtr]);

   //get number of veto words per time
   int nvwCtr = nvtCtr + nvt + 1;
   int32_t nvw = (fFlipBytes ? EndianHelper::Swap4ByteWord(buffer[nvwCtr]) : buffer[nvwCtr]);

   //get number of trigger times
   int nttCtr = nvwCtr + nvw*nvt + 1;
   int32_t ntt = (fFlipBytes ? EndianHelper::Swap4ByteWord(buffer[nttCtr]) : buffer[nttCtr]); 

   //get number of trigger words per time
   int ntwCtr = nttCtr + ntt + 1;
   int32_t ntw = (fFlipBytes ? EndianHelper::Swap4ByteWord(buffer[ntwCtr]) : buffer[ntwCtr]);  

   // ===================== doing veto history analysis ============================

   vector< double >   tempVetoTimes; //holder of times before compression
   vector< uint64_t > tempVetoMasks; //holder of masks before compression
   
   vector<double>  tempNoiseMonTimes;   //holder of noise monitor times before sorting
   vector<uint64_t> tempNoiseMonMasks;  //same for noise mon masks

   if(nvt == BatRootTypes::kVetoBufferOverflow)  fVetoBufferOverflow = true;

   //print veto words for each veto time
   for(int timeCtr=0; timeCtr<nvt; timeCtr++)
   {
     int32_t vetoTime =  ( fFlipBytes ? EndianHelper::Swap4ByteWord(buffer[nvtCtr + timeCtr + 1]) : buffer[nvtCtr + timeCtr + 1]);
     
     //temporary vectors to store veto masks for this time
     vector<uint32_t> pairMaskVector;

     //loop over veto words for this time (there should only be 2 for our present data format)
     for(int wordCtr=1; wordCtr<=nvw; wordCtr++)
     {
       uint32_t vetoWord =  ( fFlipBytes ? EndianHelper::Swap4ByteWord(buffer[nvwCtr + nvw*timeCtr + wordCtr]) : buffer[nvwCtr + nvw*timeCtr + wordCtr]); 
       
       if(debugOn)
       {
	  printf("word %d for time %d is %#x ", wordCtr, vetoTime, vetoWord);
	  cout <<"\nbufferCtr = " << (nvwCtr + nvw*timeCtr + wordCtr) << endl; 
       }

       pairMaskVector.push_back(vetoWord);

     } //end loop over veto masks
     
     //here we need to separate out noise monitor stuff. It's possible for there to be both noise mon and veto bits in a single mask
     //we'll keep with the assumption that there are only 2 veto mask words per time
     uint64_t bigmask = (uint64_t)pairMaskVector[0] + (((uint64_t)pairMaskVector[1])<<32);
     if(bigmask & fNoiseMonBits){
       tempNoiseMonTimes.push_back(vetoTime);
       tempNoiseMonMasks.push_back(bigmask & fNoiseMonBits);
       //if there are no veto bits here, then we don't want to put them in the veto vector
       const uint64_t vetoChansMask = 0x003FFFFC003FFFFCULL; //<bits 2-21 on both boards
       if(bigmask & vetoChansMask){
	 //subtract the noise mon bits from the veto masks
	 pairMaskVector[0] &= ~(fNoiseMonBits & 0xFFFFFFFF);
	 pairMaskVector[1] &= ~((fNoiseMonBits>>32) & 0xFFFFFFFF);
       }
       else{
	 //there was only noise mon bits, so nothing else to do
	 continue;
       }
     }
     tempVetoMasks.push_back(MaskCompressVeto(pairMaskVector)); //compress the veto masks into one large mask 
     tempVetoTimes.push_back(vetoTime);

   } //end loop over veto times

   //compress time-split veto history events
   MaskTimeCompressIII(tempVetoTimes, tempVetoMasks);

   //loop over times to sort results of veto analysis into before, and after vectors.  
   //Don't store mask/time pair if they are only 0x1 - this means global trigger happened
   //only the time t=0 should ever have this value since the buffer should reset then 
   int maxTimes = tempVetoMasks.size();
   for(int timeCtr=0; timeCtr < maxTimes; timeCtr++)
   {
      double tempTime = tempVetoTimes[timeCtr];
      uint64_t tempMask = tempVetoMasks[timeCtr];

       //fill BEFORE event trigger times and masks
       //the last pair will be the EventVetoTime/EventVetoMask
       if(tempTime <= 0 && tempMask != 0x1 && tempMask!= 0x400001UL && tempMask != 0x400000UL) 
       {
	  fBeforeEventVetoTimes.push_back(tempTime);
	  fBeforeEventVetoMasks.push_back(tempMask);
	  
 // 	  cout <<"storing before event time/mask = " << tempTime <<", " << tempMask <<", for timeCtr" << timeCtr<< endl;
//  	  printf("in hex = %#x", tempMask);
       }

       //fill AFTER event trigger times and masks
       //checking that tempMask != global trigger (0x1, 400001, 400000) even though this shouldn't happen
       if(tempTime > 0 && tempMask != 0x1 && tempMask != 0x400001UL && tempMask != 0x400000UL) 
       {
 	 fAfterEventVetoTimes.push_back(tempTime);
 	 fAfterEventVetoMasks.push_back(tempMask);
       }       

   } //end loop over veto times

   // ===================== doing trigger history analysis ============================

   vector< vector<double> >   tempTriggerTimes; //holder of times before compression
   vector< vector<uint64_t> > tempTriggerMasks; //holder of masks before compression
   
   //first create empty vectors to hold a place for each word (this is ok because all times have the same number of words in a single history record)
   for(int emptyCtr = 0; emptyCtr < ntw; emptyCtr++)
   {
     vector<double> emptyTimeVector;
     fBeforeEventTriggerTimes.push_back(emptyTimeVector);
     fAfterEventTriggerTimes.push_back(emptyTimeVector);

     vector<uint64_t> emptyMaskVector;
     fBeforeEventTriggerMasks.push_back(emptyMaskVector);
     fAfterEventTriggerMasks.push_back(emptyMaskVector);

     vector<uint64_t> emptyMaskVectorTest;
     tempTriggerTimes.push_back(emptyTimeVector);
     tempTriggerMasks.push_back(emptyMaskVectorTest);
   }

   //looping over trigger times
   for(int timeCtr=0; timeCtr<ntt; timeCtr++)
   {
     int32_t triggerTime =  ( fFlipBytes ? EndianHelper::Swap4ByteWord(buffer[nttCtr + timeCtr + 1]) : buffer[nttCtr + timeCtr + 1]); 
     //temp     cout <<"\ntrigger time " << timeCtr <<" is = " << triggerTime 
     //temp	  <<"\nbufferCtr = " << nttCtr + timeCtr + 1
     //temp	  << endl;

     //loop over trigger words (i.e. towers)
     //pair words to reposition bits 31 and 32, which are shared between odd and even words
     uint64_t oddMask = 0x0;
     uint64_t evenMask = 0x0;
     
     for(int wordCtr=1; wordCtr<=ntw; wordCtr++)
     {
       uint32_t triggerWord =  ( fFlipBytes ? EndianHelper::Swap4ByteWord(buffer[ntwCtr + ntw*timeCtr + wordCtr]) : buffer[ntwCtr + ntw*timeCtr + wordCtr]); 
       //cout <<"\nbufferCtr = " << (ntwCtr + ntw*timeCtr + wordCtr) << endl; 
       
       //printf("\nword %d for time %d is ", wordCtr, triggerTime);  PrintBinary(triggerWord);
       // printf("             bit 30(ST_TE/Glo) = %d,   bit 31(ISR/RAN) = %d \n", CheckBit(triggerWord,30), CheckBit(triggerWord,31));

       //store odd Mask and even Mask in order to form a pair
       if( (wordCtr%2) != 0) 
       {
	 //cout <<"Storing oddMask for word = " << wordCtr << endl;
	 oddMask = triggerWord;
       }

       //we've got a pair, now compress bits
       if( (wordCtr%2) == 0) 
       {
	 evenMask = triggerWord;
	 MaskCompressTrigger(oddMask, evenMask);

	 //cout <<"Storing oddMask for word = " << (wordCtr-1) <<", " << oddMask << endl;
	 //cout <<"Storing evenMask for word = " << (wordCtr) <<", " << evenMask << endl;
	 
	 //now store the pairs
	 //store trigger times and masks
	 (tempTriggerTimes[wordCtr-1]).push_back(triggerTime);
	 (tempTriggerMasks[wordCtr-1]).push_back(evenMask);

	 (tempTriggerTimes[wordCtr-2]).push_back(triggerTime);
	 (tempTriggerMasks[wordCtr-2]).push_back(oddMask);

       } 

     } //done looping over trigger words

   } //done looping over trigger times

   //Now loop through and compress times
   for(int wordCtr=1; wordCtr<=ntw; wordCtr++)
   {
     //cout <<"Now compressing word/tower = " << wordCtr << endl;
     
      MaskTimeCompressIII(tempTriggerTimes[wordCtr-1], tempTriggerMasks[wordCtr-1]);
     //MaskTimeCompressII(tempTriggerTimes[wordCtr-1], tempTriggerMasks[wordCtr-1]);
          
     //loop over times to sort results of trigger analysis into before, during and after vectors.  
     //Don't store mask/time pair if they are 0
     int maxTimes = (tempTriggerMasks[wordCtr-1]).size();
     for(int timeCtr=0; timeCtr < maxTimes; timeCtr++)
     {
       double tempTime = (tempTriggerTimes[wordCtr-1])[timeCtr];
       double tempTimeNext = ( (timeCtr+1) < maxTimes ? (tempTriggerTimes[wordCtr-1])[timeCtr+1] : 999999);
       uint64_t tempMask = (tempTriggerMasks[wordCtr-1])[timeCtr];

       //       cout << setprecision(10) <<"checking for word " << wordCtr <<", mask = " << tempMask <<", time = " <<  tempTime
       //    << endl;

       //fill BEFORE event trigger times and masks - only if a trigger happened on this tower
       if(tempTime < 0 && tempMask != 0x0) 
       {
	 //checking that time=0 is not skipped, if it is, then store current in EventTriggerTime/Mask
 	 if(tempTimeNext <=0)
	 {
	   (fBeforeEventTriggerTimes[wordCtr-1]).push_back(tempTime);
 	   (fBeforeEventTriggerMasks[wordCtr-1]).push_back(tempMask);
	 }
	 else
	 {
	   fEventTriggerTime = (double)tempTime;
	   fEventTriggerMasks.push_back((uint64_t)tempMask);
	 }
       }

       //fill DURING event trigger masks - regardless of whether trigger happened on this tower
       if(tempTime == 0) 
       { 
 	 fEventTriggerTime = (double)tempTime;
 	 fEventTriggerMasks.push_back((uint64_t)tempMask);
       }

       //fill AFTER event trigger times and masks - only if a trigger happened on this tower
       if(tempTime > 0 && tempMask != 0x0) 
       {
 	 (fAfterEventTriggerTimes[wordCtr-1]).push_back(tempTime);
 	 (fAfterEventTriggerMasks[wordCtr-1]).push_back(tempMask);
       }       

     } //end loop over times

   } //end loop over words


   // ============= Store all the RQs ======================
   if(!fStoreRQs) { return; } 
   
   
   // ============== Store the noise monitor RQs ================
   // we want to store the first time before and after each trigger for each noise mon bit
   for(size_t bit = 0; bit<64; ++bit){
     uint64_t mymask = (1ULL<<bit);
     if(fNoiseMonBits & mymask){
       //loop through the times and find the minimum
       double pretime, posttime;
       pretime = posttime = BatRootTypes::kEmptyVariable;
       for(size_t i=0; i<tempNoiseMonTimes.size(); ++i){
	 if(tempNoiseMonMasks[i] & mymask){
	   double t = tempNoiseMonTimes[i];
	   if(t <=0 && (pretime == BatRootTypes::kEmptyVariable || t>pretime))
	     pretime = t;
	   if(t > 0 && (posttime == BatRootTypes::kEmptyVariable || t<posttime))
	     posttime = t;
	 }
       }//end loop over stored times
       stringstream s;
       s<<"NM"<<bit<<"PreTime";
       fRQList[s.str()] = pretime;
       s.str("");
       s<<"NM"<<bit<<"PostTime";
       fRQList[s.str()] = posttime;
     }
   }


   // ============== Store results of veto analysis in RQ List ======================
   


   //Loop over the masks - the number of the mask should correspond to the tower
   
   //BEFORE time RQs - loop forward and cout up from 20 because they were stored in order of latest to most recent
   int nthToLast = 0;
   
   //for a given tower# the number of stored times and words should be the same
   for(int backCtr = fBeforeEventVetoTimes.size()-1; backCtr >=0; backCtr--)
   {

      if( (20 - nthToLast) > 13 )
      {
	 //towerCtr+1 corresponds to the tower number
	//FIXME check this once the history buffer is re-enabled!
	stringstream tempRQTime;
	stringstream tempRQMask;

	tempRQTime <<"VTTime" << (20 - nthToLast);
	tempRQMask <<"VTMask" << (20 - nthToLast);
	 

	fRQList[tempRQTime.str()] = fBeforeEventVetoTimes[backCtr];
	fRQList[tempRQMask.str()] = fBeforeEventVetoMasks[backCtr];
      }

      nthToLast++;
   } // done looping backwards through stored trigger times

   //AFTER time RQs - loop backwards and countdown from 19 because they were stored in order of latest to most recent
   int nthNext = 1;
   
   //for a given tower# the number of stored times and words should be the same
   for(uint forwardCtr = 0; forwardCtr < fAfterEventVetoTimes.size(); forwardCtr++)
   {

      if( (20 + nthNext) < 24 )
      {
	 //towerCtr+1 corresponds to the tower number
	//FIXME check this once the history buffer is re-enabled!
	stringstream tempRQTime;
	stringstream tempRQMask;
	 
	tempRQTime <<"VTTime" << (20 + nthNext); 
	tempRQMask <<"VTMask" << (20 + nthNext); 


	fRQList[tempRQTime.str()] = fAfterEventVetoTimes[forwardCtr];
	fRQList[tempRQMask.str()] = fAfterEventVetoMasks[forwardCtr];

      }

      nthNext++;
   } // done looping backwards through stored trigger times

   // ============== Store results of trigger analysis in RQ List ======================

   if(!fStoreRQs) { return; }

   //Loop over the masks - the number of the mask should correspond to the tower
   for(int towerCtr=0; towerCtr < ntw; towerCtr++)
   {

     if(fNTowers == BatRootTypes::kEmptyVariable) 
     {
       cout <<"HistoryData::ReadSoudanHistoryRecord ERROR!  fNTowers has not been set, yet attempting to store RQ's" << endl;
       exit(1);
     }

     //1.  Store BEFORE time RQs - loop forward and cout up from 21 because they were stored in order of latest to most recent
     int nthToLast = 1;

     //for a given tower# the number of stored times and words should be the same
     for(int backCtr = (fBeforeEventTriggerTimes[towerCtr]).size() - 1; backCtr >=0; backCtr--)
     {

       if( (20 - nthToLast) > 15 && towerCtr < fNTowers )
       {
	 //towerCtr+1 corresponds to the tower number
	 //FIXME check this once the history buffer is re-enabled!
	 stringstream tempRQTime;
	 stringstream tempRQMask; 

	 tempRQTime <<"T" << (towerCtr+1) <<"TGTime" << (20 - nthToLast);  
	 tempRQMask <<"T" << (towerCtr+1) <<"TGMask" << (20 - nthToLast);  

	 fRQList[tempRQTime.str()] = (fBeforeEventTriggerTimes[towerCtr])[backCtr];
	 fRQList[tempRQMask.str()] = (fBeforeEventTriggerMasks[towerCtr])[backCtr];
       }

       nthToLast++;
     } // done looping backwards through stored trigger times

     //2.  Store AFTER time RQs - loop backwards and countdown from 19 because they were stored in order of latest to most recent
     int nthNext = 1;

     //for a given tower# the number of stored times and words should be the same
     for(uint forwardCtr = 0; forwardCtr < (fAfterEventTriggerTimes[towerCtr]).size(); forwardCtr++)
     {

       if( (20 + nthNext) < 26 && towerCtr < fNTowers )
       {
	 //towerCtr+1 corresponds to the tower number
	 //FIXME check this once the history buffer is re-enabled!
	 stringstream tempRQTime; 
	 stringstream tempRQMask; 

	 tempRQTime <<"T" << (towerCtr+1) <<"TGTime" << (20 + nthNext);  
	 tempRQMask <<"T" << (towerCtr+1) <<"TGMask" << (20 + nthNext);  

	 fRQList[tempRQTime.str()] = (fAfterEventTriggerTimes[towerCtr])[forwardCtr];
	 fRQList[tempRQMask.str()] = (fAfterEventTriggerMasks[towerCtr])[forwardCtr];
       }

       nthNext++;
     } // done looping backwards through stored trigger times

     //3. Store DURING time RQs
     if( towerCtr < fNTowers )
     {
       //FIXME check this once the history buffer is re-enabled!
       stringstream tempRQTime;
       stringstream tempRQMask;
       
       tempRQTime <<"T" << (towerCtr+1) <<"TGTime20";
       tempRQMask <<"T" << (towerCtr+1) <<"TGMask20";

       fRQList[tempRQTime.str()] = fEventTriggerTime;
       fRQList[tempRQMask.str()] = fEventTriggerMasks[towerCtr];
     }

     //4. Store abbreviated masks containing summary plo and qlo bits
     if( towerCtr < fNTowers )
     {
	double ploMask = 0;
	double qloMask = 0;
	
	for(int zipItr = 0; zipItr < BatRootTypes::kSoudanNZipsPerTower; zipItr++)
	{
	   int detNum = zipItr+1 + towerCtr*(BatRootTypes::kSoudanNZipsPerTower);
	   
	   //we only need to know if it has a p(q)lo or not
	   ploMask += (GetNTrigP(detNum) >0);
	   qloMask += (GetNTrigQ(detNum) >0);
	}

	stringstream tempPstring;
	stringstream tempQstring;
	
	tempPstring <<"T" << towerCtr+1 <<"NTrigP";
	tempQstring <<"T" << towerCtr+1 <<"NTrigQ";


	fRQList[tempPstring.str()] = ploMask;
	fRQList[tempQstring.str()] = qloMask;

	//5. Store OR'd masks over specific time ranges

	stringstream tempMaskstring1;
	stringstream tempMaskstring2;
	stringstream tempMaskstring3;

	stringstream tempMaskstring8;
	stringstream tempMaskstring9;
	stringstream tempMaskstring10;
	stringstream tempMaskstring11;
	stringstream tempMaskstring12;
	stringstream tempMaskstring13;


	tempMaskstring1 <<"T" << towerCtr+1 <<"TGMask256to10";
	tempMaskstring2 <<"T" << towerCtr+1 <<"TGMask50to50";
	tempMaskstring3 <<"T" << towerCtr+1 <<"TGMask100to200";

	tempMaskstring8 <<"T" << towerCtr+1 <<"TGMaskminus300to200";
	tempMaskstring9 <<"T" << towerCtr+1 <<"TGMaskminus200to100";
	tempMaskstring10 <<"T" << towerCtr+1 <<"TGMaskminus100to0";
	tempMaskstring13 <<"T" << towerCtr+1 <<"TGMaskplus200to300";
	tempMaskstring12 <<"T" << towerCtr+1 <<"TGMaskplus100to200";
	tempMaskstring11 <<"T" << towerCtr+1 <<"TGMaskplus0to100";


	fRQList[tempMaskstring1.str()] = GetORMasksFrom(-256.0, 10.0, towerCtr+1) ;
	fRQList[tempMaskstring2.str()] = GetORMasksFrom(-50.0, 50.0, towerCtr+1) ;
	fRQList[tempMaskstring3.str()] = GetORMasksFrom(-100.0, 200.0, towerCtr+1) ;

	fRQList[tempMaskstring8.str()] = GetORMasksFrom(-300.0, -200.0, towerCtr+1) ;
	fRQList[tempMaskstring9.str()] = GetORMasksFrom(-200.0, -100.0, towerCtr+1) ;
	fRQList[tempMaskstring10.str()] = GetORMasksFrom(-100.0, -000.0, towerCtr+1) ;

	fRQList[tempMaskstring11.str()] = GetORMasksFrom(000.0, 100.0, towerCtr+1) ;
	fRQList[tempMaskstring12.str()] = GetORMasksFrom(100.0, 200.0, towerCtr+1) ;
	fRQList[tempMaskstring13.str()] = GetORMasksFrom(200.0, 300.0, towerCtr+1) ;


     }

   } //done looping over trigger words (i.e. towers)

   // ============== Store the ErrorMask in RQ List ======================

   fRQList["ErrorMask"] = GetErrorMask();



   // For debugging only!!
//   cout <<"\nStoring in History, tower 1 = "; PrintBinary(fRQList["T1TGMask20"]);
//   cout <<"\nStoring in History, tower 2 = "; PrintBinary(fRQList["T2TGMask20"]);
//   cout <<"\nStoring in History, tower 3 = "; PrintBinary(fRQList["T3TGMask20"]);
//   cout <<"\nStoring in History, tower 4 = "; PrintBinary(fRQList["T4TGMask20"]);
//   cout <<"\nStoring in History, tower 5 = "; PrintBinary(fRQList["T5TGMask20"]);

   return;
}

//it is the responsibility of the calling routine to ensure
//the file ptr is positioned at the start of the history data block 
void HistoryData::ReadSUFHistoryRecord(gzFile& localRawDataPtr, uint32_t recordLength, 
					  bool debugOn)
{
   //Extract the information in the data block
   uint32_t bufferLength = recordLength/BatRootTypes::kWordSize; //this better be an integer
   uint32_t buffer[bufferLength]; 
   int readCheck = gzread(localRawDataPtr,(char*)buffer, bufferLength*sizeof(uint32_t));

   if(readCheck < 0)
   {
      cerr <<"HistoryData::ERROR reading history data block!" << endl;
      exit(1);
   }

   //FIXME we still need to implement this!
   cout <<"HistoryData::ERROR!  Code not implemented yet!" 
	<< endl;
   exit(1);

//    if(fFlipBytes)
//    {
//    }
//    else
//    {
//    }

   return;
}

// ----------------- Supporting functions -------------------------

void HistoryData::PrintBinary(const uint64_t& word)
{
  int spaceCtr = 0;

  for (int i=63; i>=0; i--) 
    {
      if(spaceCtr == 4) { cout <<" "; spaceCtr = 0; }
      
      int bit = ((word >> i) & 1);
      if(i<64) cout <<bit;
      
      spaceCtr++;
    }
    
  return;

}

//attempting a straight port of the "slow" darkpipe algorithm
//this routine will not combine the third consecutive time if an event is tripley split
void HistoryData::MaskTimeCompressI(vector<double>& timeVector, vector<uint64_t>& maskVector)
{
  if(timeVector.size() != maskVector.size())
  {
    cout <<"HistoryData::MaskTimeCompress ERROR! Time and Mask vector lengths do not agree. Error in trigger analysis!"
	 << endl;
    exit(1);
  }

  //do nothing if the vectors are empty
  if(timeVector.size() == 0)
  {
    return;
  }


  //starting w/ *SECOND* element, check to see if the difference is <=1 us
  vector<double>::iterator timeItr = timeVector.begin();
  vector<uint64_t>::iterator maskItr = maskVector.begin();
  for(timeItr++, maskItr++ ; timeItr != timeVector.end(); timeItr++, maskItr++)
  {
    double time = *timeItr;
    double timePrev = *(timeItr-1);
    uint64_t mask = *(maskItr);
    uint64_t maskPrev = *(maskItr-1);

    //find two consecutive times
    if( (time - timePrev) <= 1)
    {
//       cout <<"time-1 = " << timePrev
// 	   <<", time = " << time
// 	   <<", mask-1 = " << maskPrev
// 	   <<", mask = " << mask
// 	   << endl;

      //check that there is no overlapping information
      if( (mask & maskPrev) == 0x0 )
      {
	uint64_t maskCompressed = maskPrev | mask;

//temp	cout <<"Compressed!  " << maskCompressed
//temp	     <<"\ntimeVector size = " << timeVector.size()
//temp	     <<"\nmaskVector size = " << maskVector.size()
//temp	     << endl;

	*(maskItr-1) = maskCompressed; //store compressed mask in earlier time

	//revert to previous counter so we can delete the current one
	timeItr = timeItr-1;
 	  maskItr = maskItr-1;

 	 timeVector.erase(timeItr+1);
	 maskVector.erase(maskItr+1);

//temp	cout  <<"\ntimeVector size = " << timeVector.size() <<", timeItr = " << *timeItr
//temp	      <<"\nmaskVector size = " << maskVector.size() <<", maskItr = " << *maskItr
//temp	      << endl;

      } //if & is 0x0
    }//if two consecutive events found

  }//end loop over times

  return;
}

//A straight port of the darkpipe algorithm labeled "kickass" routine in the original MaskTimeCompress.m
//for multiple consecutive times, the last time is not stored, but each consecutive pair is OR'd
//This means consecutive times between the first and last time are OR'd more than once
//For example, times 1, 2, 3, 4, 5 -> 1 OR 2, 2 OR 3, 3 OR 4, 4 OR 5.  
//These OR'd pairs are stored as times 1, 2, 3, 4
void HistoryData::MaskTimeCompressII(vector<double>& timeVector, vector<uint64_t>& maskVector)
{
  if(timeVector.size() != maskVector.size())
  {
    cout <<"HistoryData::MaskTimeCompressII ERROR! Time and Mask vector lengths do not agree. Error in trigger analysis!"
	 << endl;
    exit(1);
  }

  //do nothing if the vectors are empty
  if(timeVector.size() == 0)
  {
    return;
  }


  vector<int> dd;
  int ddVal = 0;
  int aaVal = 0;
  vector<uint64_t> tempMaskVector;
  vector<double> tempTimeVector;

  //starting with *second* element, loop through times and construct difference
  //store 1 if diff is <= 1 us and 0 if not
  dd.push_back(ddVal);

  for(int timeItr = 0; timeItr < ((int)timeVector.size()-1); timeItr++)
  {
    ddVal = ( (timeVector[timeItr+1] - timeVector[timeItr]) <= 1 ? 1 : 0);
    dd.push_back(ddVal); //this is entry timeItr+1 for dd

    aaVal = dd[timeItr+1] - dd[timeItr];

//    cout <<"Time = " << timeVector[timeItr] <<", dd = " << ddVal <<", aa = " << aaVal << endl;

    //if we found a consecutive pair of times, then OR the current mask with the next one
    //store it in the same position as the current mask
    if(ddVal == 1) 
    {
      //store compressed mask if the next event differs by 1 us
      tempMaskVector.push_back(maskVector[timeItr] | maskVector[timeItr+1]);  
      tempTimeVector.push_back(timeVector[timeItr]);
    }
    else
    {
      //don't store the later time/mask if aa = -1
      if(aaVal != -1)  
      {
	tempMaskVector.push_back(maskVector[timeItr]); 
	tempTimeVector.push_back(timeVector[timeItr]); 
      }
    }

  }//end loop over times

  //last iteration
  dd.push_back(0); //this is entry timeVector.size() for dd
  aaVal = dd[timeVector.size()] - dd[timeVector.size()-1];
 
  //  cout <<"Time = " << timeVector[timeVector.size()-1] <<", dd = " << 0 <<", aa = " << aaVal << endl;

  if(aaVal != -1) 
  {  
    tempMaskVector.push_back(maskVector[timeVector.size()-1]);
    tempTimeVector.push_back(timeVector[timeVector.size()-1]);
  }

  //overwrite the original maskVector and timeVector
  maskVector = tempMaskVector;
  timeVector = tempTimeVector;

  return;
}

//a modified version of MaskTimeCompressII.  We store consecutive events in pairs.   If there's
//an odd number of consecutive events then the last consecutive is stored as a triplet.
//For example, if times 1 and 2 are consective then this code combines: 1 | 2
//if times 1, 2, and 3 are consecutive then it combines: 1 | 2 | 3
//if times 1, 2, 3, and 4 are consecutive then it combines: 1 | 2, 3 | 4
//if times 1, 2, 3, 4, and 5 are consecutive then it combines: 1 | 2, 3 | 4 | 5
//Note most (>99%) split events show up as 2 or 3 consecutive entries in the history buffer
void HistoryData::MaskTimeCompressIII(vector<double>& timeVector, vector<uint64_t>& maskVector)
{
  if(timeVector.size() != maskVector.size())
  {
    cout <<"HistoryData::MaskTimeCompressIII ERROR! Time and Mask vector lengths do not agree. Error in trigger analysis!"
	 << endl;
    exit(1);
  }

  //do nothing if the vectors are empty
  if(timeVector.size() == 0)
  {
    return;
  }


  vector<int> dd;
  int consecutiveCtr = 0; //n consecutive entries = consecutiveCtr + 1
  int aaVal = 0;
  vector<uint64_t> tempMaskVector;
  vector<double> tempTimeVector;

  //starting with *second* element, loop through times and construct difference
  //store 1 if diff is <= 1 us and 0 if not
  dd.push_back(consecutiveCtr);
  
  for(int timeItr = 0; timeItr < ((int)timeVector.size()-1); timeItr++)
  {
    if( (timeVector[timeItr+1] - timeVector[timeItr]) <= 1 )
    {
       consecutiveCtr++;
    }
    else
    {
       consecutiveCtr = 0;
    }

    dd.push_back(consecutiveCtr); //this is entry timeItr+1 for dd

    aaVal = dd[timeItr+1] - dd[timeItr];

    //if we found a consecutive pair of times, then OR the current mask with the next one
    //store it in the same position as the current mask
    if(consecutiveCtr != 0) 
    {
       //only do OR for every other pair of consecutives to avoid OR'ing the same mask twice
       if(consecutiveCtr%2 != 0)
       {
	  tempMaskVector.push_back(maskVector[timeItr] | maskVector[timeItr+1]);  
	  tempTimeVector.push_back(timeVector[timeItr]);

// 	  cout << setprecision(15) <<"ORing times " << timeVector[timeItr] <<" and " << timeVector[timeItr+1] 
// 	       <<"\nand masks " << maskVector[timeItr] <<" and " << maskVector[timeItr+1]
// 	       <<"\n OR'd mask = " << (maskVector[timeItr] | maskVector[timeItr+1]) << endl;
       }

    }
    else
    {

      //store the nonconsecutives unaltered
      if(aaVal >= 0)  
      {
	 tempMaskVector.push_back(maskVector[timeItr]); 
	 tempTimeVector.push_back(timeVector[timeItr]); 
      }
      else  //dealing with the last consecutive
      {
	 //when there are an odd number of consecutives, store the last group as a triplet 
	 //note for the case of even number of consecutives, the last consecutive is already OR'd
	 if(aaVal%2 == 0)
	 {
	    //retrieve the last entry in temporary MaskVector
	    uint64_t oldMask = tempMaskVector[tempMaskVector.size()-1];

//	    cout <<"last mask entry = " << oldMask << endl;

	    tempMaskVector[tempMaskVector.size()-1] = oldMask | maskVector[timeItr];

// 	    cout <<"triple OR'd mask = " << tempMaskVector[tempMaskVector.size()-1] 
// 		 <<"\n added " << maskVector[timeItr]
// 		 << endl;
	    
	 }
      }
    }

  }//end loop over times

  //last iteration
  dd.push_back(0); //this is entry timeVector.size() for dd
  aaVal = dd[timeVector.size()] - dd[timeVector.size()-1];

  if(aaVal >= 0) 
  {  
     tempMaskVector.push_back(maskVector[timeVector.size()-1]);
     tempTimeVector.push_back(timeVector[timeVector.size()-1]);
  }
  else
  {
     if(aaVal%2 == 0)
     {
	//retrieve the last entry in temporary MaskVector
	uint64_t oldMask = tempMaskVector[tempMaskVector.size()-1];

//	cout <<"last mask entry = " << oldMask << endl;
	
	tempMaskVector[tempMaskVector.size()-1] = oldMask | maskVector[timeVector.size()-1];

//	cout <<"triple OR'd mask = " << tempMaskVector[tempMaskVector.size()-1] << endl;	
     }
  }

  //overwrite the original maskVector and timeVector
  maskVector = tempMaskVector;
  timeVector = tempTimeVector;

  return;
}

//rearranges bits 31 & 32 which are originally shared between odd and even masks
void HistoryData::MaskCompressTrigger(uint64_t& oddMask, uint64_t& evenMask)
{
  uint64_t tempOddMask = oddMask & 0x3fffffffUL;
  uint64_t tempEvenMask = evenMask & 0x3fffffffUL;

  //ST_TE (bit 30 on odd masks) remaps to bit 32
  if(CheckBit(oddMask,30) == 1)
  {
    tempOddMask = tempOddMask | 0x100000000ULL;
    tempEvenMask = tempEvenMask | 0x100000000ULL;
  }

  //VetoMult/ISR (bit 31 on odd masks) remaps to bit 33
  if(CheckBit(oddMask,31) == 1)
  {
    tempOddMask = tempOddMask | 0x200000000ULL;
    tempEvenMask = tempEvenMask | 0x200000000ULL;
  }

  //Global (bit 30 on even masks) remaps to bit 34
  if(CheckBit(evenMask,30) == 1)
  {
    tempOddMask = tempOddMask | 0x400000000ULL;
    tempEvenMask = tempEvenMask | 0x400000000ULL;
  }

  //Random (bit 31 on even masks) remaps to bit 35
  if(CheckBit(evenMask,31) == 1)
  {
    tempOddMask = tempOddMask | 0x800000000ULL;
    tempEvenMask = tempEvenMask | 0x800000000ULL;
  }

  oddMask = tempOddMask;
  evenMask = tempEvenMask;

//   cout <<"     oddMask = "; PrintBinary(oddMask);
//   cout <<"\n tempOddMask = "; PrintBinary(tempOddMask);

//   cout <<"\n    evenMask = "; PrintBinary(evenMask);
//   cout <<"\ntempEvenMask = "; PrintBinary(tempEvenMask);

  return;
}

//combines veto masks into one large mask
uint64_t HistoryData::MaskCompressVeto(vector<uint32_t>& vetoMasks)
{
   if(vetoMasks.size() != 2)
   {
      cout <<"HistoryData::MaskCompressVeto ERROR! Structure of veto history buffer is not as expected, please revise this function!" 
	   << endl;
      exit(1);
   }

//   cout <<"\nVeto Mask 1 "; PrintBinary(vetoMasks[0]);
//   cout <<"\nVeto Mask 2 "; PrintBinary(vetoMasks[1]);

   //map bits 0-21 of mask 1 to bits 0-21
   //map bits 0-21 of mask 2 to bits 22-43
   uint64_t combinedMask = (vetoMasks[0] & 0x3FFFFFULL) + ((vetoMasks[1]&0x3FFFFFULL)*0x400000ULL);

//   cout <<"\nAfter combining masks" << endl; PrintBinary(combinedMask);

   //map bit 28 in second word to bit 44
   if(CheckBit(vetoMasks[1], 28))
      combinedMask = combinedMask | 0x100000000000ULL;

//   cout <<"\nAfter remapping bit 28" << endl; PrintBinary(combinedMask);

   return combinedMask;
}

//checks whether specified bit is 1 or 0
bool HistoryData::CheckBit(const uint64_t& word, const int& bit)
{
  bool isOn = false;

//Temp  printf("\nIn CheckBit!\n");
//Temp  printf("word = %#x\n",        word);
//Temp  printf("after shift = %#x\n", (word >> (bit)) );
//Temp  printf(" & 0x01 = %#x\n",     (word >> (bit)) & 0x01);
  //  printf(" log2(word) = " << log(word,2));

  if( (word >> (bit)) & 0x01 )
  {
    isOn = true;
  }

  return isOn;
}
