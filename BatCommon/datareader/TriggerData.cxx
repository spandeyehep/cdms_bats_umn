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

#include <iostream>
#include "zlib.h"
#include <sstream>
#include <iomanip>

#include "BatRootTypes.h"
#include "TriggerData.h"

using namespace std;

////////////////////////////////////////////////////////

TriggerData::TriggerData() :
  fStoreRQs(true),
  fNTowers(-999999),
  fFlipBytes(false)
{
   Reset();
}

TriggerData::~TriggerData()
{
//   cout <<"Goodbye from TriggerData()" << endl;
}

void TriggerData::Reset()
{
   //All data members to be filled from 
   //raw data records are reset here

   fTriggerMasks.clear();

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
void TriggerData::ConstructRQList(const int nTowers)
{
   fNTowers = nTowers;
   double initVal = -999999.;

   //construct the RQ list here

   for(int towerCtr = 1; towerCtr <= fNTowers; towerCtr++)
   {
     //FIXME - check this when trigger is enabled!
     stringstream tempRQName1; 
     tempRQName1 <<"TrigInfo" << towerCtr;

     fRQList.insert(pair<string,double>(tempRQName1.str(), initVal));
   }

   //Any RQ that is included in the above list will be written out by BatRoot.  Add to this as you please.

   return;
}

// ========================= Get Functions ======================================

uint64_t TriggerData::GetTowerMask(const int towerNum)
{
  if(fTriggerMasks.size() == 0 || fTriggerMasks.size() < (uint)towerNum) 
  {
    cout <<"TriggerData::GetTowerMask ERROR! Attempting to get tower mask, but it appears event has not been read yet.  Check your code." << endl;
    exit(1);
  }  

  return fTriggerMasks[towerNum-1];
}

// ========================= Raw Data Read ======================================

//it is the responsibility of the calling routine to ensure
//the file ptr is positioned at the start of the trigger data block 
void TriggerData::ReadTrigger(gzFile& localRawDataPtr, uint32_t recordLength, bool debugOn)
{
   //Extract the information in the data block
   uint32_t bufferLength = recordLength/BatRootTypes::kWordSize; //this better be an integer
   uint32_t buffer[bufferLength]; 
   int readCheck = gzread(localRawDataPtr,(char*)buffer, bufferLength*sizeof(uint32_t));

   if(readCheck < 0)
   {
      cerr <<"TriggerData::ERROR reading trigger data block!" << endl;
      exit(1);
   }
   
   // get number of masks from the buffer length
   int32_t triggerTime = buffer[0]; //should always be zero
   int ntw = bufferLength - 1; //first word is trigger time
   
   //loop over trigger words (i.e. towers)
   //pair words to reposition bits 31 and 32, which are shared between odd and even words
   //this code is similar to what is done for the History Record
   uint64_t oddMask = 0x0;
   uint64_t evenMask = 0x0;
     
   for(int wordCtr=1; wordCtr<=ntw; wordCtr++)
   {
      uint32_t triggerWord =  ( fFlipBytes ? EndianHelper::Swap4ByteWord(buffer[wordCtr]) : buffer[wordCtr]); 
               
      if(debugOn)
      {
	 printf("\nword %d for time %d is, %u, ", wordCtr, triggerTime, triggerWord); PrintBinary(triggerWord);
	 printf("\n             bit 30(ST_TE/Glo) = %d,   bit 31(ISR/RAN) = %d \n", CheckBit(triggerWord,30), CheckBit(triggerWord,31));
      }

      //store odd Mask and even Mask in order to form a pair
      if( (wordCtr%2) != 0) 
      {
 	 oddMask = triggerWord;
      }

      //we've got a pair, now compress bits
      if( (wordCtr%2) == 0) 
      {
 	 evenMask = triggerWord;
 	 MaskCompress(oddMask, evenMask);

	 if(debugOn)
	 {
	    cout <<"Storing oddMask for word = " << (wordCtr-1) <<", " << oddMask << endl;
	    cout <<"Storing evenMask for word = " << (wordCtr) <<", " << evenMask << endl;
	 }

 	 //now store the pairs
 	 //store trigger times and masks, remembering that the odd mask is the earlier tower number
 	 fTriggerMasks.push_back(oddMask);
 	 fTriggerMasks.push_back(evenMask);

      } //done with pair 

   } //done looping over trigger words

   // ============== Store results of trigger analysis in RQ List ======================

   if(!fStoreRQs) { return; }

   //Loop over the masks - the number of the mask should correspond to the tower
   for(int towerCtr=0; towerCtr < ntw; towerCtr++)
   {

      if(fNTowers == -999999) 
      {
	 cout <<"TriggerData::ReadTriggerRecord ERROR!  fNTowers has not been set, yet attempting to store RQ's" << endl;
	 exit(1);
      }

      if( towerCtr < fNTowers )
      {
	//FIXME check this when trigger is enabled!
	stringstream tempRQMask;
	tempRQMask <<"TrigInfo" << towerCtr+1;

	fRQList[tempRQMask.str()] = fTriggerMasks[towerCtr];

//	 cout <<"\nStoring in TrigInfo = "; PrintBinary(fTriggerMasks[towerCtr]);
      }

   } //done looping over trigger words (i.e. towers)


   return;
}

//rearranges bits 31 & 32 which are originally shared between odd and even masks
void TriggerData::MaskCompress(uint64_t& oddMask, uint64_t& evenMask)
{
  uint64_t tempOddMask = oddMask & 0x3fffffffUL;
  uint64_t tempEvenMask = evenMask & 0x3fffffffUL;

  //ST_TE remaps to bit 32
  if(CheckBit(oddMask,30) == 1)
  {
    tempOddMask = tempOddMask | 0x100000000ULL;
    tempEvenMask = tempEvenMask | 0x100000000ULL;
  }

  //VetoMult/ISR remaps to bit 33
  if(CheckBit(oddMask,31) == 1)
  {
    tempOddMask = tempOddMask | 0x200000000ULL;
    tempEvenMask = tempEvenMask | 0x200000000ULL;
  }

  //Global remaps to bit 34
  if(CheckBit(evenMask,30) == 1)
  {
    tempOddMask = tempOddMask | 0x400000000ULL;
    tempEvenMask = tempEvenMask | 0x400000000ULL;
  }

  //Random remaps to bit 35
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

//checks whether specified bit is 1 or 0
bool TriggerData::CheckBit(const uint32_t& word, const int& bit)
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

void TriggerData::PrintBinary(const uint64_t& word)
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
 
char bit_to_plus(uint64_t test) 
{ return test ? '+' : '-'; }

string TriggerData::GetTriggerInfoTable()
{
  stringstream s;
  //write the header for the table
  s<<"Triggers: (Random,Global,ISR,ST_TE) or (Wisper,Plow,Phigh,Qlow,Qhigh)\n";
  s<<"Tower  | ";
  size_t tablewidth = 9;
  for(size_t tower=0; tower<fTriggerMasks.size(); ++tower){
    s<<setw(5)<<setfill(' ')<<tower+1<<" | ";
    tablewidth += 8;
  }
  s<<"\n";
  s<<"       |  ";
  for(size_t tower = 0; tower < fTriggerMasks.size(); ++tower){
    uint64_t towermask = fTriggerMasks[tower];
    s<< ((towermask & kRandom) ? 'R' : '-')
     << ((towermask & kGlobal) ? 'G' : '-')
     << ((towermask & kISR)    ? 'I' : '-')
     << ((towermask & kST_TE)  ? 'S' : '-')
     <<" |  ";
  }
  s<<'\n'<<setw(tablewidth)<<setfill('-')<<'\n';
  for(size_t dib = 0; dib < kMaxDibsPerTower; ++dib){
    s<<"DIB"<<setw(4)<<setfill(' ')<<left<<dib+1<<right<<"| ";
    for(size_t tower = 0; tower < fTriggerMasks.size(); ++tower){
      uint64_t towermask = fTriggerMasks[tower];
      uint8_t dibmask = (towermask>>(dib*kNTrigTypeBits));
      s << ((dibmask & kWisper) ? 'W' : '-')
	<< ((dibmask & kPlow)   ? 'p' : '-')
	<< ((dibmask & kPhigh)  ? 'P' : '-')
	<< ((dibmask & kQlow)   ? 'q' : '-')
	<< ((dibmask & kQhigh)  ? 'Q' : '-')
	<<" | ";
    }
    //s<<'\n'<<setw(tablewidth)<<setfill('-')<<'\n';
    s<<"\n";
  }
  return s.str();
}
