///////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Name: PulseData
//Author: L. Hsu, M. Kos, B. Serfass
//Description:  This is a container class which stores data from the Pulse record in the raw data file.
//In particular, this class stores the pulse from a single channel.  The pulse data are filled directly by this class, 
//which knows how to parse the raw record.  This class also stores modifications of the raw pulse and allows
//the user to retrieve them for later analysis.  Finally, this class stores instances of the analysis
//routines that are performed on this pulse. These are stored in the vector of TCDMSAnalyses for later
//retrieval of the RQ values.
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
#include "PulseData.h"
#include "EndianHelper.h"
#include "ChannelMapHelper.h"

using namespace std;

////////////////////////////////////////////////////////

//default constructor
PulseData::PulseData() 
{
   Reset();
}

PulseData::~PulseData()
{
}

void PulseData::Reset()
{
   //All data members are reset/initialized here

   fFlipBytes = false;

   // === Raw Data Quantities ===

   fDetCode      = 0;
   fDetChannel   = BatRootTypes::kEmptyVariable;
   fDetNum       = BatRootTypes::kEmptyVariable;
   fDetType      = BatRootTypes::kEmptyVariable;
   fNADCBins     = 0;
   fSampleDt     = 0;
   fTriggerT0    = BatRootTypes::kEmptyVariable;
   
   fIsPhonon = false;
   fIsCharge = false;
   fIsVeto = false;
   fIsZip = false;
   fIsNoiseMonitor = false;

   fChannelName = "";

   // === Calculated Quantities ===

   //the pulse in its various forms
   fPulseVector.clear(); 
   fBSPulseVector.clear(); 
   fBSNPulseVector.clear(); 
   fTestPulseVector.clear();
 
   //clear the fitters too
   fAnalysisCollection.clear();
   return;
}

//========================= Setters ========================================

//This class is currently only initialized in EventBuilder and RawDataReader
//no setters are currently activated

//======================= non-inline Getters  ==============================


string PulseData::GetChannelName() const
{

   //check that the name has been set
   if(fChannelName == "")
   {
      cout <<"ERROR!  In PulseData::GetChannelName - Channel name has not been set.  Check initialization of PulseData object"
	   << endl;

      exit(1);
   }


   return fChannelName;
}

string PulseData::GetChannelType() const
{
   string tempString = "";

   if(IsPhononPulse())
      tempString = "phonon";

   if(IsChargePulse())
      tempString = "charge";

   if(IsVetoPulse())
      tempString = "veto";

   if(IsNoiseMonitorPulse())
      tempString = "noise";


   return tempString;
}

//==========================================================================

//it is the responsibility of the calling routine to ensure
//the file ptr is positioned at the start of a trace data block 
void PulseData::ReadRawPulseRecord(gzFile& localRawDataPtr, uint32_t recordLength, 
				   uint32_t recordID, bool dispflag)
{
   

   //Extract the information in the data block
   uint32_t bufferLength = recordLength/BatRootTypes::kWordSize; //this better be an integer
   uint32_t buffer[bufferLength]; 
   int readCheck = gzread(localRawDataPtr,(char*)buffer, bufferLength*sizeof(uint32_t));  

   if(readCheck < 0)
   {
      cerr <<"PulseData::ERROR reading trace data block!" << endl;
      exit(1);
   }
   
   ReadRawPulseBuffer(buffer, recordLength, recordID, dispflag);
}

//load pulse from memory buffer (already unzipped from file)
void PulseData::ReadRawPulseBuffer(uint32_t* buffer, uint32_t recordLength, 
				   uint32_t recordID, bool dispflag)
{
  uint32_t detCodeHex = buffer[4];
  fTriggerT0 = buffer[7];
  fSampleDt = buffer[8];
  fNADCBins = buffer[11];
  
  if(fFlipBytes) 
    {
      detCodeHex = EndianHelper::Swap4ByteWord(detCodeHex);
      fTriggerT0 = EndianHelper::Swap4ByteWord(fTriggerT0);
      fSampleDt  = EndianHelper::Swap4ByteWord(fSampleDt);
      fNADCBins  = EndianHelper::Swap4ByteWord(fNADCBins); 
    }
  
  
  if(dispflag)
    {
      printf("Detector code, hex = %#x\n", detCodeHex); 
      printf("Detector code, decimal: %d \n", detCodeHex);  
      printf("record ID = %d", recordID);
    }
  
  
  //In 2010, we switched to pulse record ID type 0x11 (from 0x10) to expand the
  //detector code.   This allows for more channels and more detector types, which is
  //needed to accomodate the iZIP design.
  if(recordID == BatRootTypes::kPulseRecordID)
    {
      fDetChannel = detCodeHex%10;
      fDetNum     = ((detCodeHex%1000) - fDetChannel)/10;
      fDetType    = (detCodeHex - 10*fDetNum - fDetChannel) / 1000;  
      fDetCode    = ChannelMapHelper::CalcDetCodeBase(fDetType, fDetNum) + fDetChannel; 
    }   
  else //for kPulseRecordExpandedCodeID
    {
      fDetChannel = detCodeHex%1000;
      fDetNum     = ((detCodeHex%1000000) - fDetChannel)/1000;
      fDetType    = (detCodeHex - 1000*fDetNum - fDetChannel) / 1000000;
      fDetCode    = ChannelMapHelper::CalcDetCodeBase(fDetType, fDetNum) + fDetChannel; 
    }
  
  // === channel type and name assignments according to the detector code - these are dictated by the raw data format! ===
  
  // distiguish zip from veto traces, assign names
  if( fDetType != BatRootTypes::kVetoDetType && 
      fDetType != BatRootTypes::kMonitorNoiseFast && fDetType != BatRootTypes::kMonitorNoiseSlow)  
    {
      fIsZip = true;    
      
      //store the channel name for convenient access later - for ZIP pulses only!
      fChannelName = ChannelMapHelper::GetChannelName(fDetType, fDetChannel);
      
      //store the channel type for convenient access later
      if(fChannelName[0] == 'P')
	{
	  fIsPhonon = true;
	}
      
      if(fChannelName[0] == 'Q')
	{
	  fIsCharge = true;
	}
      
    } else if (fDetType == BatRootTypes::kVetoDetType) {
    
    fIsVeto = true;  
    fChannelName = "V";
    
    
  } else if (fDetType == BatRootTypes::kMonitorNoiseFast || fDetType == BatRootTypes::kMonitorNoiseSlow) {
    
    fIsNoiseMonitor = true;  
    char name[100];
    sprintf(name, "NM%dCh%d",fDetNum, fDetChannel+1);
    fChannelName = name;
    //fChannelName = ChannelMapHelper::GetChannelName(fDetType, fDetChannel);
    
  } else {
    
    fIsOther = true;
    fChannelName = "O";
  }
  
  
  // ==== ADC values are paired to make one word, separate them now ===
  
  for(int pairItr = 1; pairItr <= fNADCBins/2; pairItr++)
    {
      uint32_t twinBinValues = buffer[11+pairItr];
      if(fFlipBytes) twinBinValues = EndianHelper::Swap4ByteWord(twinBinValues);
      
      uint16_t adc1 = twinBinValues & 0x0000ffff; 
      uint16_t adc2 = (twinBinValues & 0xffff0000) >> 16;
      
      //       printf("word = %#x", twinBinValues);
      //       printf(", adc1 = %#x", adc1);
      //       printf(", adc2 = %#x", adc2);
      // 	 cout <<", bins " << (pairItr*2)-2 <<" and " << (pairItr*2)-1 << endl;
      
      fPulseVector.push_back(adc1);
      fPulseVector.push_back(adc2);
    }
  
  
  if(dispflag)
    {
      printf("Detector code, decimal, stored as: %d \n", fDetCode);  
      printf("Detector channel: %d \n", fDetChannel);  
      printf("Detector num: %d \n", fDetNum);
      printf("Detector type: %d \n", fDetType);  
      printf("nADC bins %d \n", fNADCBins);  
    }
  
  //   cout <<"Pulse Vector size = " << fPulseVector.size() << endl;
  
  return;
  
}




// =========== Set parameters =============
void PulseData::SetChannelConfig(uint32_t detCode)
{
  // set various channel parameters

  fDetCode = detCode;
  fDetChannel = ChannelMapHelper::GetChannelNumFromCode(fDetCode);
  fDetNum = ChannelMapHelper::GetDetNumFromCode(fDetCode);
  fDetType = ChannelMapHelper::GetDetTypeFromCode(fDetCode);
  

  if( fDetType != BatRootTypes::kVetoDetType && 
      fDetType != BatRootTypes::kMonitorNoiseFast && fDetType != BatRootTypes::kMonitorNoiseSlow)  
   {
      fIsZip = true;    

      //store the channel name for convenient access later - for ZIP pulses only!
      fChannelName = ChannelMapHelper::GetChannelName(fDetType, fDetChannel);

      //store the channel type for convenient access later
      if(fChannelName[0] == 'P')
 	 fIsPhonon = true;
       
      if(fChannelName[0] == 'Q')
 	 fIsCharge = true;

   } else if (fDetType == BatRootTypes::kVetoDetType) {
       fIsVeto = true;  
       fChannelName = "V";

   } else if (fDetType == BatRootTypes::kMonitorNoiseFast || fDetType == BatRootTypes::kMonitorNoiseSlow) {

       fIsNoiseMonitor = true;  
       fChannelName = ChannelMapHelper::GetChannelName(fDetType, fDetChannel);

   } else {

       fIsOther = true;
       fChannelName = "O";
   }

   return;
 }

// =========== Flip ADC bin values in pairs, for the DCRC revC boards  =============
void PulseData::FlipADCRawPulse()
{
  //This function should only be done before any calculates have been done on the pulse vector!

  if((fBSPulseVector.size() != 0 || fBSNPulseVector.size() != 0 || fTestPulseVector.size() != 0) && 
      fPulseVector.size() != 0)
  {
    cout <<"WARNING! It seems you are trying to flip adc values after deriving objects from the raw pulse!"
	 <<"\nAny changes you make now will not be automatically propagated to the derived pulses."
	 <<"\nPlease check that this is really what you want to do."
	 << endl;
  }

  vector<double> newPulseVector;

  for(uint binItr=0; binItr < fPulseVector.size(); binItr++)
  {

    if(binItr%2 == 0)
    {
      newPulseVector.push_back(fPulseVector[binItr+1]);
    }
    else
    {
      newPulseVector.push_back(fPulseVector[binItr-1]);
    }
  }

  //replace old pulse vector with the new one
  fPulseVector = newPulseVector;

  return;
}


// =========== Set Raw data record (if empty)  =============
void PulseData::SetRawPulseRecord(uint32_t detCode, const vector<double>& rawPulse, uint32_t sampleDt, int32_t triggerT0)
{

  // No modification allowed
  if (!fPulseVector.empty()) {
    cerr << "PulseData::SetRawPulseRecord:  ERROR: No modification of existing PulseData object allowed. " << endl;
    exit(1);
  }


  // Fill  Pulse Vector
  fPulseVector = rawPulse;

  // channel config (based on detCode)
  SetChannelConfig(detCode);

  // Digitizer information
  fTriggerT0 = triggerT0;
  fSampleDt = sampleDt;
  fNADCBins = fPulseVector.size();

}








// ============ For RQ management ======================

void PulseData::StorePulseAnalysis(TCDMSAnalysis& analysisClass)
{
   //cout <<"Storing virtual analysis!" << endl;

   fAnalysisCollection.push_back(analysisClass);

   return;
}

//provides read-only access!!
TCDMSAnalysis PulseData::GetPulseAnalysis(const string& analysisName)
{

   //cout <<"Getting virtual analysis!" << endl;

   TCDMSAnalysis tempAna;
   bool found = false;

   for(uint ctr=0; ctr < fAnalysisCollection.size(); ctr++)
   {
      if(fAnalysisCollection[ctr].GetClassName() == analysisName)
      {
	 tempAna = fAnalysisCollection[ctr];
	 found = true;
      }
   }      

   if(!found)
   {
      cerr <<"PulseData::ERROR! Attempting to retrieve analysis " << analysisName 
	   <<"\nThis hasn't been stored in PulseData for detector " 
	   << fDetNum <<", channel " << GetChannelName()
	   << endl;
      exit(1);

   }

   return tempAna;
}



bool PulseData::HasPulseAnalysis(const string& analysisName)
{
   bool found = false;

   for(uint ctr=0; ctr < fAnalysisCollection.size(); ctr++)
   {
      if(fAnalysisCollection[ctr].GetClassName() == analysisName)
  	               found = true;
   }      

   return found;
}
