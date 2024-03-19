///////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Name: DetectorConfigData
//Author: L. Hsu
//Description:  This is a container class which stores data from the Detector Configuration record.
//The detector configuration are filled directly by this class, which knows how to parse the raw record.
//
//
//File Import By: L. Hsu
//Creation Date: Nov. 22, 2010
//
//Modifications:
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include "zlib.h"

#include "BatRootTypes.h"
#include "DetectorConfigData.h"

using namespace std;

////////////////////////////////////////////////////////

DetectorConfigData::DetectorConfigData() :
  fMapIsFilled(false),
  fMapIsModified(false),
  fFlipBytes(false)
{
  //  cout <<"Hello from DetectorConfigData constructor" << endl;

  Reset();

}

DetectorConfigData::~DetectorConfigData()
{
//   cout <<"Goodbye from DetectorConfigData()" << endl;
}

void DetectorConfigData::Reset()
{
   //All data members to be filled from 
   //raw data records are reset here

   fDetectorConfigMap.clear();

   return;
}

// ========================= Raw Data Read ======================================

//it is the responsibility of the calling routine to ensure
//the file ptr is positioned at the start of the configuration block
void DetectorConfigData::ReadDetectorConfigRecord(gzFile& localRawDataPtr, 
						  uint32_t recordLength, bool debugOn)
{

  uint32_t bufferLength = recordLength/BatRootTypes::kWordSize; //this better be an integer
  int32_t buffer[bufferLength];  //these are signed values

  //returns -1 for error, and 0 for end of file
  int readCheck = gzread(localRawDataPtr, (char*)buffer, bufferLength*sizeof(uint32_t));

  if(readCheck < 0)
  {
    cerr <<"DetectorConfigData::ERROR reading trigger data block!" << endl;
    exit(1);
  }


  //Filling the map - begin loop through the sub-records
  //note that phonon and charge records have different lengths, which are
  //specified in the subheader
  uint linCtr = 0;

  while(linCtr < bufferLength)
  {
    //these are treated as uint to be consistent with pulse records
    uint32_t subHeader = (fFlipBytes ? EndianHelper::Swap4ByteWord(buffer[linCtr]) : buffer[linCtr]); 
    uint32_t subRecordLength = (fFlipBytes ? EndianHelper::Swap4ByteWord(buffer[linCtr+1]) : 
				buffer[linCtr+1])/BatRootTypes::kWordSize;
    uint32_t detCode = (fFlipBytes ? EndianHelper::Swap4ByteWord(buffer[linCtr+2]) : buffer[linCtr+2]);
    
    if(debugOn)
    {
      printf("subheader = %#x", subHeader);
      cout <<"\nFound config record for detector/channel = " << detCode 
	   << endl;
    }

    //add the config values
    map<string, double> configValMap;

    linCtr +=3;

     //parse the phonon record and convert to more mundane units
     if( subHeader == BatRootTypes::kPhononConfigID )
     {
       if( subRecordLength != BatRootTypes::kPhononConfigRecordSize )
       {
 	cout <<"ERROR! DetectorConfigData::ReadDetectorConfigRecord - Detecting incorrect size for charge config record." 
 	     << endl;
       }

       //tower number
       int32_t towernum       = (fFlipBytes ? EndianHelper::Swap4ByteWord(buffer[linCtr]) : 
 			       buffer[linCtr]);
       linCtr++;
       configValMap.insert(pair<string, double>("Tower", double(towernum)));

       //no units
       int32_t drivergain    = (fFlipBytes ? EndianHelper::Swap4ByteWord(buffer[linCtr]) : 
 			       buffer[linCtr]);
       linCtr++;
       configValMap.insert(pair<string, double>("driverGain", double(drivergain)/100.));  //remove X100 needed for storing as int

       //in amps (factor of 1e-6 converts from microamps to amps)
       int32_t qetbias        = (fFlipBytes ? EndianHelper::Swap4ByteWord(buffer[linCtr]) : 
 			       buffer[linCtr]);
       linCtr++;
       configValMap.insert(pair<string, double>("qetBias", 1e-6*double(qetbias)/100.)); //remove X100 needed for storing as int

       //in amps (factor of 1e-6 converts from microamps to amps)
       int32_t squidbias      = (fFlipBytes ? EndianHelper::Swap4ByteWord(buffer[linCtr]) : 
 			       buffer[linCtr]);
       linCtr++;
       configValMap.insert(pair<string, double>("squidBias", 1e-6*double(squidbias)/100.)); //remove X100 needed for storing as int

       //in volts (factor of 1e-6 converts from microvolts to volts)
       int32_t squidlockpoint = (fFlipBytes ? EndianHelper::Swap4ByteWord(buffer[linCtr]) : 
 			       buffer[linCtr]);
       linCtr++;
       configValMap.insert(pair<string, double>("squidLockPoint", 1e-6*double(squidlockpoint)/100.)); //remove X100 needed for storing as int

       //in volts (factor of 1e-6 converts from microvolts to volts)
       int32_t phononoffset   = (fFlipBytes ? EndianHelper::Swap4ByteWord(buffer[linCtr]) : 
 			       buffer[linCtr]);
       linCtr++;
       configValMap.insert(pair<string, double>("phononOffset", 1e-6*double(phononoffset)));

       //no units
       int32_t feedbackgain   = (fFlipBytes ? EndianHelper::Swap4ByteWord(buffer[linCtr]) : 
 			       buffer[linCtr]);
       linCtr++;
       configValMap.insert(pair<string, double>("variableGain", double(feedbackgain)));

       //in seconds (factor of 1e-9 converts from nanoseconds to seconds)
       int32_t timeperbin     = (fFlipBytes ? EndianHelper::Swap4ByteWord(buffer[linCtr]) : 
				 buffer[linCtr]); 
       linCtr++;
       configValMap.insert(pair<string, double>("timePerBin", double(timeperbin)/1e9)); 

       //in seconds (factor of 1e-9 converts from nanoseconds to seconds)
       int32_t triggertime    = (fFlipBytes ? EndianHelper::Swap4ByteWord(buffer[linCtr]) : 
 			       buffer[linCtr]);
       linCtr++;
       configValMap.insert(pair<string, double>("triggerTime", double(triggertime)/1e9)); 

       //in sampled bins
       int32_t binspertrace   = (fFlipBytes ? EndianHelper::Swap4ByteWord(buffer[linCtr]) : 
 			       buffer[linCtr]);
       linCtr++;
       configValMap.insert(pair<string, double>("binsPerTrace", double(binspertrace)));

     }
     else if( subHeader == BatRootTypes::kChargeConfigID )
     {
       if( subRecordLength != BatRootTypes::kChargeConfigRecordSize)
       {
 	cout <<"ERROR! DetectorConfigData::ReadDetectorConfigRecord -  Detecting incorrect size for charge config record." 
 	     << endl;
 	exit(1);
       }

       //tower number
       int32_t towernum       = (fFlipBytes ? EndianHelper::Swap4ByteWord(buffer[linCtr]) : 
 			       buffer[linCtr]);
       linCtr++;
       configValMap.insert(pair<string, double>("Tower", double(towernum)));

       //no units
       int32_t drivergain    = (fFlipBytes ? EndianHelper::Swap4ByteWord(buffer[linCtr]) : 
 			       buffer[linCtr]); 
       linCtr++;
       configValMap.insert(pair<string, double>("driverGain", double(drivergain)/100.)); //remove X100 needed for storing as int

       //in volts (factor of 1e-6 converts from microvolts to volts)
       int32_t channelbias    = (fFlipBytes ? EndianHelper::Swap4ByteWord(buffer[linCtr]) : 
				 buffer[linCtr]);  
       linCtr++;
       configValMap.insert(pair<string, double>("chargeBias", 1e-6*double(channelbias))); 

       //in volts (factor of 1e-6 converts from microvolts to volts 
       int32_t chargeoffset   = (fFlipBytes ? EndianHelper::Swap4ByteWord(buffer[linCtr]) : 
 			       buffer[linCtr]);
       linCtr++;
       configValMap.insert(pair<string, double>("chargeOffset", 1e-6*double(chargeoffset)));

       //in seconds (factor of 1e-9 converts from nanoseconds to seconds)
       int32_t timeperbin     = (fFlipBytes ? EndianHelper::Swap4ByteWord(buffer[linCtr]) : 
 			       buffer[linCtr]);
       linCtr++;
       configValMap.insert(pair<string, double>("timePerBin", double(timeperbin)/1e9));  

       //in seconds (factor of 1e-9 converts from nanoseconds to seconds)
       int32_t triggertime    = (fFlipBytes ? EndianHelper::Swap4ByteWord(buffer[linCtr]) : 
 			       buffer[linCtr]);
       linCtr++;
       configValMap.insert(pair<string, double>("triggerTime", double(triggertime)/1e9));

       //in sampled bins
       int32_t binspertrace   = (fFlipBytes ? EndianHelper::Swap4ByteWord(buffer[linCtr]) : 
 			       buffer[linCtr]);
       linCtr++;
       configValMap.insert(pair<string, double>("binsPerTrace", double(binspertrace)));
     }

     //for debugging only
     if(debugOn)
     {
       map<string, double>::iterator valItr = configValMap.begin();
       for( ; valItr != configValMap.end(); valItr++)
       {
	 cout <<"From DetectorConfigData::ReadDetectorConfigRecord dumping value " 
	      << valItr->first <<" = " << valItr->second << endl;
       }
     }       

     //Inserting one vector of config values per channel
     //its ok to cast detCode to int w/ the xxxyyyzzz format
     fDetectorConfigMap.insert(pair< int, map<string, double> >(detCode, configValMap));

    

  } //end while linCtr < bufferLength


  //record that fDetectorConfigMap is filled
  fMapIsFilled = true;

  // Set Modified config to false
  fMapIsModified = false;


  return;
}

