///////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Name: DetectorConfigManager
//Authors: L. Hsu, B. Serfass
//
//Description:  A class to manage retrieval of channel settings and normalizations.   The values may come 
//              from as many as three separate places: the detector configuration record inside the raw 
//              data files, the external data files (ISR or INFO), or the user settings files.   This 
//              class chooses which source based on availability and user specifications (such as whether 
//              or not to read the ISR file).  Unless over-ridden by the user, the default priority is to 
//              first take the information from the detecctor configuration record.  If that is not 
//              available, it will look in the external data files.  If that is not available, then it 
//              will take the values from the user settings file.   The last is least desirable because 
//              a number of these parameters can change depending on the DAQ settings.  In this case,  
//              the user must remember to keep those values consistent with those used by the DAQ. 
//
//
//File Import By: L. Hsu
//Creation Date: Dec. 9, 2010
//
//
//
//Modifications and Notes:
//
//L. Hsu (Jan 2011) - As of 2011 (start of R132) at Soudan, we no longer read the external ISR 
//                    and INFO files for channel settings.  Thus, Soudan processing config files 
//                    should have READ ISR_FILE and READ INFO_FILE flags set to FALSE (0).   In R132, we 
//                    added the detector config record to the raw data file.  This makes the retrieval 
//                    of these values from the ISR and INFO files redundant.   At the same time, the 
//                    mapping of channel to detector to hardware read-out chain changed dramatically 
//                    and non-intuitively due to the fact that we needed to use two FEBs to read out one 
//                    iZIP. Due to this complications, we decided to forgo upkeep of the ISR and INFO 
//                    file reading in this period.  Note that ISR and INFO reading still work fine for 
//                    Soudan runs prior to R132.  For these runs, the external files should be the 
//                    default source (READ flag set to true) because there is no detector config record 
//                    inside the raw data files from the earlier period.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "TTree.h"

#include "DetectorConfigManager.h"
#include "ChannelMapHelper.h"
#include "RawDataReader.h"

using namespace std;

// ======================================================================

DetectorConfigManager::DetectorConfigManager(UserDataManager& myUserData, string& inputRawDataFile) :
  fUserData(myUserData),
  fIsRawDataFilled(false),
  fIsIsrDataFilled(false),
  fIsInfoDataFilled(false),
  fDebugOn(false)
{

   // instantiatiate raw data reader for reading the detector config records
   RawDataReader rawReader;

   // register with RawDataReader so that detector config will be read
   rawReader.RegisterDetectorConfigData(&fDetectorConfigData);

   // open the raw data file

   rawReader.OpenRawDataFile(fUserData.GetPath("RAW_DATA"), inputRawDataFile);
   
   // reads the file header AND the detector configuration 
   rawReader.ReadFileHeader(false);

   // Allow modification of configuration if needed   
   if (myUserData.DoModifyRawData()) {
     
      // get detector -> modification map
      map<int, string> modificationMap = myUserData.GetRawDataModificationMap();
   
      if (!modificationMap.empty())
         rawReader.ModifyRawData(modificationMap);
      else {
        cerr <<"DetectorConfigurationManager: ERROR!  Cannot modify raw data without modification type!"  
	    <<"\nNeed to add a MODIFICATION_TYPE parameter in processing settings."
	    << endl;
        exit(1);
      }
   }


   // check whether the fDetectorConfigData object has been filled 
   //(this won't be filled if it doesn't exist in the file)
   fIsRawDataFilled = fDetectorConfigData.IsFilled();

   if(fIsRawDataFilled)
   {
     fRawConfigMap = fDetectorConfigData.GetDetectorConfigMap(); 
   }

   // close the raw data file because we don't need it here anymore 
   rawReader.CloseRawDataFile();

}


//default constructor
DetectorConfigManager::DetectorConfigManager()
{

}
  
DetectorConfigManager::~DetectorConfigManager() 
{ 
   //cout <<"Goodbye from DetectorConfigManager()" << endl; // destructor
}


// =====================================================================

// detectors to be processed:
//    detectors set by the DO_PROCESSING flag in configuration file
//            AND
//    available information in the raw data or ISR file (in that order of priority)
//

map<int, int> DetectorConfigManager::GetDetectorMap()
{

  // check whether the map has been filled, if so return filled map
  if( fDetectorMap.size() > 0)
  {
    return fDetectorMap;
  }

  // ----- fill the detector map, key = zip#, val = detector type -----

  // - if raw data config is available use that -
  if(fIsRawDataFilled)
  {

    // Loop over the raw data config map
    map< int, map<string, double>  >::iterator rawMapItr = fRawConfigMap.begin();
    for( ; rawMapItr != fRawConfigMap.end(); rawMapItr++)
    {

      uint32_t detectorCode = rawMapItr->first;
      int detType = ChannelMapHelper::GetDetTypeFromCode(detectorCode);
      int detNum = ChannelMapHelper::GetDetNumFromCode(detectorCode);

      //    if selected by user, store in the detector map 
      // - duplicate entries for each chan will be overwritten -
      
      if(fUserData.DoZipProcessing(detNum))
      {
	fDetectorMap.insert(pair< int, int >(detNum, detType)); 
      }

    }

  }
  // - if not raw data then check isr file -
  else if(fIsIsrDataFilled)
  {
    //note that isr file does not specify detector type.  We must take this from 
    //the processing config since its not available in the raw data
    
    for(int detNum = 1; detNum <= fUserData.GetMaxZIPs() ; detNum++)
    {
      if( fUserData.DoZipProcessing(detNum) && fIsrData.IsConfigured(detNum) )
      {
	int detType = fUserData.GetIntParameter(detNum, "DET_TYPE");
	fDetectorMap.insert(pair< int, int >(detNum, detType)); 
      }
    }
  }
  // - if no ext data or raw data record then use processing config files -
  else  
  {

    for(int detNum = 1; detNum <= fUserData.GetMaxZIPs() ; detNum++)
    {
      if( fUserData.DoZipProcessing(detNum) )
      {
	int detType = fUserData.GetIntParameter(detNum, "DET_TYPE");
	fDetectorMap.insert(pair< int, int >(detNum, detType)); 
      }
    }
  }

  //returns newly filled detector map
  return fDetectorMap;
}


// =====================================================================

//constructs list of available config information from what is available
map<string, double> DetectorConfigManager::GetDetectorConfiguration(int detNum)
{
  map<string, double> detectorConfigurationMap;

  // fill the detector map if it hasn't been filled already
  if( fDetectorMap.size() == 0)
  {
    GetDetectorMap();
  }


  // retrieve the detector type
  int detType = fDetectorMap[detNum];

  // Temp - these are just for debugging
  //fIsRawDataFilled = false;
  //fIsInfoDataFilled = false;
  //fIsIsrDataFilled = false;

  // === now fill the detector configuration map ===

  // - first try to fill from the detector config record from raw data file -
  if(fIsRawDataFilled)
  {

    // Loop over the raw data config map (one entry per channel)
    map< int, map<string, double>  >::iterator rawMapItr = fRawConfigMap.begin();
    for( ; rawMapItr != fRawConfigMap.end(); rawMapItr++)
    {
      uint32_t detectorCode = rawMapItr->first;
      int checkDetNum = ChannelMapHelper::GetDetNumFromCode(detectorCode);
      
      // if not the requested detector, then skip
      if(detNum != checkDetNum)  continue;

      // storing detector type
      //int detType = ChannelMapHelper::GetDetTypeFromCode(detectorCode);   
      detectorConfigurationMap.insert(pair<string,double>("DetType", detType));

      //retrieve the vector of config values and their names
      map<string, double> configValMap = rawMapItr->second;      
      map<string, double>::iterator valItr = configValMap.begin();

      // now loop over remaining quantities in the map and store them w/ their names
      // attach the channel name to distinguish values for different channels
      string chanName = ChannelMapHelper::GetChannelName(detectorCode);

      for( ; valItr != configValMap.end(); valItr++)
      {
	string valName = valItr->first;

	if(valName != "Tower")
	  valName = chanName + valName;

	detectorConfigurationMap.insert(pair<string,double>(valName, valItr->second));
      }

    }

  }

  // - if not raw data then check isr, info or configuration files -
  else 
  {
    //retrieve the channel list
    vector<string> channelNameList;
    ChannelMapHelper::FillAllChannelList(detType, channelNameList);

    //loop over channel names and retrieve values
    for(uint nameItr=0; nameItr < channelNameList.size(); nameItr++)
    {
      string chanName = channelNameList[nameItr];
      string sensorType = ChannelMapHelper::GetChannelType(chanName);
     
      // === store detector type and tower number (these must come from user config file) ===

      // store the detector type
      detectorConfigurationMap.insert(pair<string, double>("DetType", 
							   (double)fUserData.GetIntParameter(detNum, "DET_TYPE")));

      // store the tower number
      detectorConfigurationMap.insert(pair<string, double>("Tower", 
							   (double)fUserData.GetTowerNumber(detNum)));



      // === storing digitization parameters ===

      if(fIsInfoDataFilled)
      {
	//store the "timePerBin" in seconds
	double timePerBin = 1./fInfoData.GetSampleRate(detNum, sensorType);
	detectorConfigurationMap.insert(pair<string, double>(chanName+"timePerBin", timePerBin));
	
	//store the "triggerTime" in seconds
	double triggerTime = (double)fInfoData.GetPreTrigger(detNum, sensorType);
	detectorConfigurationMap.insert(pair<string, double>(chanName+"triggerTime", triggerTime*timePerBin));
	
	//store the "binsPerTrace"
	double traceLength = (double)fInfoData.GetPostTrigger(detNum, sensorType) + triggerTime;
	detectorConfigurationMap.insert(pair<string, double>(chanName+"binsPerTrace", traceLength));
      }
      else
      {

	//store the "timePerBin" in seconds
	string parNameBase = ChannelMapHelper::GetChannelNameBase(chanName);

	double timePerBin = 1./fUserData.GetDoubleParameter(detNum, parNameBase + "_SAMPLERATE");
	detectorConfigurationMap.insert(pair<string, double>(chanName+"timePerBin", timePerBin));
	
	//store the "triggerTime" in seconds
	double triggerTime = (double)fUserData.GetIntParameter(detNum, parNameBase + "_PRETRIGGER");
	detectorConfigurationMap.insert(pair<string, double>(chanName+"triggerTime", triggerTime*timePerBin));
	
	//store the "binsPerTrace"
	double traceLength = (double)fUserData.GetIntParameter(detNum, parNameBase + "_POSTTRIGGER")
	                     + triggerTime;
	detectorConfigurationMap.insert(pair<string, double>(chanName+"binsPerTrace", traceLength));

      } //end if Info is filled



      // === store bias and gains ===

      if(fIsIsrDataFilled)
      {
	//FIXME - figure out what to do about bias and gains - store values for start of file?
	//NOTE: I think we decided not to do this because the start time for bias is not well defined in ISR file
      }
      else
      {
	
	string parNameBase = ChannelMapHelper::GetChannelNameBase(chanName);

	//store the driver gain
	double driverGain = fUserData.GetDoubleParameter(detNum, parNameBase + "_DriverGain");
	detectorConfigurationMap.insert(pair<string, double>(chanName+"driverGain", driverGain));
	
	//store the channel bias (qet bias if phonon)
	int indexByChanType = ChannelMapHelper::GetChannelIndexByType(detType, chanName);
	vector<double> biasVector = fUserData.GetVectDoubleParameter(detNum, parNameBase + "_BIAS");
	
 	if(biasVector.size() <= (uint)indexByChanType)
 	{
 	  cout <<"ERROR! DetectorConfigurationManager::GetDetectorConfiguration()  Bias is not specified for all channels, check your analysis config file!"
 	       << endl;
 	  exit(1);
 	}

	double bias = biasVector[indexByChanType];
	detectorConfigurationMap.insert(pair<string, double>(chanName+"bias", bias));

      } //end if/else ISR  filled

    } //end loop over channels

  } //end if detector configuration exist in raw data file


  if(detectorConfigurationMap.size() == 0)
  {
    cout <<"ERROR! DetectorConfigurationManager::GetDetectorConfiguration "
	 <<"failed to find configuration for requested detector " << detNum 
	 <<"\nCheck consistency of code."
	 << endl;

    exit(1);
  }

  return detectorConfigurationMap;
}


// =====================================================================

double DetectorConfigManager::GetBias(int requestDetCode, int eventTime) const
{
  double bias = -999999;

  string chanName = ChannelMapHelper::GetChannelName(requestDetCode);
  string chanType = ChannelMapHelper::GetChannelType(chanName);
  int    detNum   = ChannelMapHelper::GetDetNumFromCode(requestDetCode);

  // Temp - these are just for debugging
  //fIsRawDataFilled = false;
  //fIsInfoDataFilled = false;
  //fIsIsrDataFilled = false;

  // === now fill the detector configuration map ===

  // - first try to fill from the detector config record from raw data file -
  if(fIsRawDataFilled)
  {

    // Loop over the raw data config map (one entry per channel)
    map< int, map<string, double>  >::const_iterator rawMapItr = fRawConfigMap.begin();
    for( ; rawMapItr != fRawConfigMap.end(); rawMapItr++)
    {
      int detCode = (int)rawMapItr->first;  
      
      // if not the requested detector and channel, then skip
      if(requestDetCode != detCode)  continue;

      //retrieve the vector of config values and their names
      map<string, double> configValMap = rawMapItr->second;      
      map<string, double>::const_iterator valItr = configValMap.begin();

      // now loop over remaining quantities in the map and look for the channel bias
      for( ; valItr != configValMap.end(); valItr++)
      {
	string valName = valItr->first;

	if(chanType == "phonon" && valName == "qetBias")
	{
	  bias = valItr->second; 
	}
	else if(chanType == "charge" && valName == "chargeBias")
	{
	  bias = valItr->second; 
	}
      } //end loop over config val map

    } //end loop over detectors

  }  // - if not raw data then check isr, info or configuration files -
  else 
  {
    if(fIsIsrDataFilled)
    {
      bias = fIsrData.GetBias(detNum, chanName, eventTime);
    }
    else
    {
	
      //      string parNameBase = ChannelMapHelper::GetChannelNameBase(chanName);
      // bias = fUserData.GetDoubleParameter(detNum, parNameBase + "_BIAS");  
      int detType = ChannelMapHelper::GetDetTypeFromCode(requestDetCode);
      string parNameBase = ChannelMapHelper::GetChannelNameBase(chanName);
      int indexByChanType = ChannelMapHelper::GetChannelIndexByType(detType, chanName);
      vector<double> biasVector = fUserData.GetVectDoubleParameter(detNum, parNameBase + "_BIAS");
	
      if(biasVector.size() <= (uint)indexByChanType)
      {
	cout <<"ERROR! DetectorConfigurationManager::GetBias()  Bias is not specified for all channels, check your analysis config file!"
	     << endl;
	exit(1);
      }

      bias = biasVector[indexByChanType];

    } //end if/else ISR is filled

  } //end if detector configuration exists in raw data file


  if(bias == -999999)
  {
    cout <<"ERROR! DetectorConfigurationManager::GetDetectorConfiguration "
	 <<"failed to find configuration for requested detector code" << requestDetCode
	 <<"\nCheck consistency of code."
	 << endl;

    exit(1);
  }

  return bias;
}



double DetectorConfigManager::GetBiasTime(int requestDetCode, int eventTime) const
{
  double biastime = -999999;

  string chanName = ChannelMapHelper::GetChannelName(requestDetCode);
  int    detNum   = ChannelMapHelper::GetDetNumFromCode(requestDetCode);

  
  // === now fill the detector configuration map ===
  if(fIsIsrDataFilled)
    {
      biastime = fIsrData.GetBiasTime(detNum, chanName, eventTime);
    } else {
      biastime = 0.0;
    }
 

  if(biastime == -999999)
  {
    cout <<"ERROR! DetectorConfigurationManager::GetDetectorConfiguration "
	 <<"failed to find configuration for requested detector code" << requestDetCode
	 <<"\nCheck consistency of code."
	 << endl;

    exit(1);
  }

  return biastime;
}




map<int,string> DetectorConfigManager::GetDetectorChargePolarityMap()
{

  map<int,string> polarityMap;


  // fill the detector map if it hasn't been filled already
  if( fDetectorMap.size() == 0)
  {
    GetDetectorMap();
  }


  // loop map
  map<int, int>::const_iterator it  = fDetectorMap.begin(); 

  for( ;it!=fDetectorMap.end(); it++)
  {
    int detNum = it->first;     
    int detType = it->second;   

    // iZIP only (detector types < 10 are not iZIP)
    if (detType < 10) {
       	polarityMap.insert(pair< int, string >(detNum, "unknown")); 
        continue;
    } 
    
    // get bias informations 
    int detCodeBase =  ChannelMapHelper::CalcDetCodeBase(detType, detNum);
    int detCodeQIS1 = detCodeBase;
    int detCodeQIS2 = detCodeBase+6;
   
    double QbiasS1 = -999999.;
    double QbiasS2 = -999999.; 
    string polarity = "unknown";

    if(fIsRawDataFilled)
      {

        // Loop over the raw data config map (one entry per channel)
        map< int, map<string, double>  >::const_iterator rawMapItr = fRawConfigMap.begin();
        for( ; rawMapItr != fRawConfigMap.end(); rawMapItr++)
          {
             int detCode = (int)rawMapItr->first;  
      
             if(detCode == detCodeQIS1) {
                 map<string, double> configValMap = rawMapItr->second;   
                 if (configValMap.find("chargeBias") != configValMap.end()) 
		    QbiasS1 = configValMap.find("chargeBias")->second; 
             }
       
             if(detCode == detCodeQIS2) {
                 map<string, double> configValMap = rawMapItr->second;   
                 if (configValMap.find("chargeBias") != configValMap.end()) 
		    QbiasS2 = configValMap.find("chargeBias")->second; 
             }
          }
        }
    

     if(QbiasS1 != -999999 && QbiasS2!=-999999)
       {
      
         if (QbiasS1>=0 && QbiasS2<=0)
              polarity = "pos";
         else 
              polarity = "neg";

       }
   
     polarityMap.insert(pair< int, string >(detNum, polarity)); 
       
   }

 return polarityMap;

}








// =====================================================================

//Note: If the detector bias is negative, then the driverGain sign is flipped.
//This is done to make normalized pulses always positive.  This works for most 
//cases, except with zero-bias which can result in postive or negative orientation
double DetectorConfigManager::GetDriverGain(int requestDetCode, int eventTime) const
{

  double driverGain = -999999;

  string chanName = ChannelMapHelper::GetChannelName(requestDetCode);
  int    detNum   = ChannelMapHelper::GetDetNumFromCode(requestDetCode);

  //Get the bias in order to change the sign of the driver gain if appropriate
  double bias = GetBias(requestDetCode, eventTime);
  int    polarity = (bias < 0 ? -1 : 1);

  // Temp - these are just for debugging
  //fIsRawDataFilled = false;
  //fIsInfoDataFilled = false;
  //fIsIsrDataFilled = false;

  // === now fill the detector configuration map ===

  // - first try to fill from the detector config record from raw data file -
  if(fIsRawDataFilled)
  {

    // Loop over the raw data config map (one entry per channel)
    map< int, map<string, double>  >::const_iterator rawMapItr = fRawConfigMap.begin();
    for( ; rawMapItr != fRawConfigMap.end(); rawMapItr++)
    {
      int detCode = (int)rawMapItr->first;
      
      // if not the requested detector and channel, then skip
      if(requestDetCode != detCode)  continue;

      //retrieve the vector of config values and their names
      map<string, double> configValMap = rawMapItr->second;      
      map<string, double>::const_iterator valItr = configValMap.begin();

      // now loop over remaining quantities in the map and look for the channel driverGain
      for( ; valItr != configValMap.end(); valItr++)
      {
	string valName = valItr->first;

	if(valName == "driverGain")
	{
	  driverGain = polarity*valItr->second;
	}

      } //end loop over config val map

    } //end loop over detectors

  }  // - if not raw data then check isr, info or configuration files -
  else 
  {
    if(fIsIsrDataFilled)
    {
      driverGain = polarity*fIsrData.GetDriverGain(detNum, chanName, eventTime);
    }
    else
    {
      //if not reading ISR then get this from the config file
      string parNameBase = ChannelMapHelper::GetChannelNameBase(chanName);
      driverGain = polarity*fUserData.GetDoubleParameter(detNum, parNameBase + "_DriverGain");
    } //end if ISR is filled

  } //end if detector configuration exists in raw data file


  if(driverGain == -999999)
  {
    cout <<"ERROR! DetectorConfigurationManager::GetDetectorConfiguration "
	 <<"failed to find configuration for requested detector code" << requestDetCode
	 <<"\nCheck consistency of code."
	 << endl;

    exit(1);
  }


  return driverGain;
}


// =====================================================================

double DetectorConfigManager::GetTraceLength(int requestDetCode) const
{
  double traceLength = -999999;

  string chanName = ChannelMapHelper::GetChannelName(requestDetCode);
  string chanType = ChannelMapHelper::GetChannelType(chanName);
  int    detNum   = ChannelMapHelper::GetDetNumFromCode(requestDetCode);

  // Temp - these are just for debugging
  //fIsRawDataFilled = false;
  //fIsInfoDataFilled = false;
  //fIsIsrDataFilled = false;

  // === now fill the detector configuration map ===

  // - first try to fill from the detector config record from raw data file -
  if(fIsRawDataFilled)
  {

    // Loop over the raw data config map (one entry per channel)
    map< int, map<string, double>  >::const_iterator rawMapItr = fRawConfigMap.begin();
    for( ; rawMapItr != fRawConfigMap.end(); rawMapItr++)
    {
      int detCode = (int)rawMapItr->first;
      
      // if not the requested detector and channel, then skip
      if(requestDetCode != detCode)  continue;

      //retrieve the vector of config values and their names
      map<string, double> configValMap = rawMapItr->second;      
      map<string, double>::const_iterator valItr = configValMap.begin();

      // now loop over remaining quantities in the map and look for the channel traceLength
      for( ; valItr != configValMap.end(); valItr++)
      {
	string valName = valItr->first;

	if(valName == "binsPerTrace")
	{
	  traceLength = valItr->second;
	}

      } //end loop over config val map

    } //end loop over detectors

  }  // - if not raw data then check isr, info or configuration files -
  else 
  {
    if(fIsInfoDataFilled)
    {
      traceLength = fInfoData.GetPreTrigger(detNum, chanType) + fInfoData.GetPostTrigger(detNum, chanType);
    }
    else
    {
      //if not reading ISR then get this from the config file
      string parNameBase = ChannelMapHelper::GetChannelNameBase(chanName);
      traceLength = (double)(fUserData.GetIntParameter(detNum, parNameBase + "_PRETRIGGER")
	            + fUserData.GetIntParameter(detNum, parNameBase + "_POSTTRIGGER"));
    } //end if Info is filled

  } //end if detector configuration exists in raw data file


  if(traceLength == -999999)
  {
    cout <<"ERROR! DetectorConfigurationManager::GetDetectorConfiguration "
	 <<"failed to find configuration for requested detector code" << requestDetCode
	 <<"\nCheck consistency of code."
	 << endl;

    exit(1);
  }

  return traceLength;
}      




//returns sample rate in hz
double DetectorConfigManager::GetTraceLength(int detNum, string chanType) const
{
  double traceLength = -999999;


  // === check first detector config record from raw data file  ===
  if(fIsRawDataFilled)
  {

    // Loop over the raw data config map (one entry per channel)
    map< int, map<string, double>  >::const_iterator rawMapItr = fRawConfigMap.begin();
    for( ; rawMapItr != fRawConfigMap.end(); rawMapItr++)
    {

      // check chanType
      int detCode = (int)rawMapItr->first;
      int currentDetNum = ChannelMapHelper::GetDetNumFromCode(detCode);
      string currentChanName = ChannelMapHelper::GetChannelName(detCode);
      string currentChanType = ChannelMapHelper::GetChannelType(currentChanName);


      // if not the requested detector type and detnum, then skip
      if(chanType != currentChanType || detNum!= currentDetNum)  continue;

      //retrieve the vector of config values and their names
      map<string, double> configValMap = rawMapItr->second;      
      map<string, double>::const_iterator valItr = configValMap.begin();

      // now loop over remaining quantities in the map and look for the channel traceLength
      for( ; valItr != configValMap.end(); valItr++)
      {
	string valName = valItr->first;

	if(valName == "binsPerTrace")  
        {

          double currentChanTraceLength  =  valItr->second;

          if (traceLength!=-999999 && currentChanTraceLength!=traceLength) 
	  {
             cout <<"ERROR in  DetectorConfigurationManager::GetTraceLength: "
	     <<"detector " << detNum << " has inconsistent trace length for '"<<  chanType << "'  type!"
 	     << endl;
              exit(1);
          } 
	  else 
	  {
	    traceLength=currentChanTraceLength;
          }
  
	}

      } //end loop over config val map

    } //end loop over detectors


  }  // =====  if not raw data then check isr, info or configuration files ====

  else 
  {
    if(fIsInfoDataFilled)
    {
      traceLength = fInfoData.GetPreTrigger(detNum, chanType) + fInfoData.GetPostTrigger(detNum, chanType);
    }
    else
    {
      //if not reading INFO then get this from the config file
      string parNameBase = ChannelMapHelper::GetChannelNameBaseByType(chanType);
      traceLength = (double)(fUserData.GetIntParameter(detNum, parNameBase + "_PRETRIGGER")
	            + fUserData.GetIntParameter(detNum, parNameBase + "_POSTTRIGGER"));
    } //end if Info is filled

  } //end if detector configuration exists in raw data file


  if(traceLength == -999999)
  {
    cout <<"ERROR in DetectorConfigurationManager::GetTraceLength"
	 <<"failed to find configuration for detector "<< detNum << " and channel type " << chanType
	 <<"\nCheck consistency of code."
	 << endl;

    exit(1);
  }

  return traceLength;

}







// =====================================================================


//returns sample rate in hz
double DetectorConfigManager::GetSampleRate(int requestDetCode) const
{
  double sampleRate = -999999;

  string chanName = ChannelMapHelper::GetChannelName(requestDetCode);
  string chanType = ChannelMapHelper::GetChannelType(chanName);
  int    detNum   = ChannelMapHelper::GetDetNumFromCode(requestDetCode);


  // Temp - these are just for debugging
  //fIsRawDataFilled = false;
  //fIsInfoDataFilled = false;
  //fIsIsrDataFilled = false;

  // === now fill the detector configuration map ===

  // - first try to fill from the detector config record from raw data file -
  if(fIsRawDataFilled)
  {

    // Loop over the raw data config map (one entry per channel)
    map< int, map<string, double>  >::const_iterator rawMapItr = fRawConfigMap.begin();
    for( ; rawMapItr != fRawConfigMap.end(); rawMapItr++)
    {
      int detCode = (int)rawMapItr->first;
      
      // if not the requested detector and channel, then skip
      if(requestDetCode != detCode)  continue;

      //retrieve the vector of config values and their names
      map<string, double> configValMap = rawMapItr->second;      
      map<string, double>::const_iterator valItr = configValMap.begin();

      // now loop over remaining quantities in the map and look for the channel sampleRate
      for( ; valItr != configValMap.end(); valItr++)
      {
	string valName = valItr->first;

	if(valName == "timePerBin")
	{
	  sampleRate = 1.0/valItr->second; //in hz
	}

      } //end loop over config val map

    } //end loop over detectors

  }  // - if not raw data then check isr, info or configuration files -
  else 
  {
    if(fIsInfoDataFilled)
    {
      sampleRate = fInfoData.GetSampleRate(detNum, chanType);
    }
    else
    {
      //if not reading ISR then get this from the config file
      string parNameBase = ChannelMapHelper::GetChannelNameBase(chanName);
      sampleRate = fUserData.GetDoubleParameter(detNum, parNameBase + "_SAMPLERATE");

    } //end if Info is filled

  } //end if detector configuration exists in raw data file


  if(sampleRate == -999999)
  {
    cout <<"ERROR! DetectorConfigurationManager::GetDetectorConfiguration "
	 <<"failed to find configuration for requested detector code" << requestDetCode
	 <<"\nCheck consistency of code."
	 << endl;

    exit(1);
  }

  return sampleRate;

}


//returns sample rate in hz
double DetectorConfigManager::GetSampleRate(int detNum, string chanType) const
{
  double sampleRate = -999999;


  // ====== check first detector config record from raw data file  =====

  if(fIsRawDataFilled)
  {

    // Loop over the raw data config map (one entry per channel)
    map< int, map<string, double>  >::const_iterator rawMapItr = fRawConfigMap.begin();
    for( ; rawMapItr != fRawConfigMap.end(); rawMapItr++)
    {

      // check chanType
      int detCode = (int)rawMapItr->first;
      int currentDetNum = ChannelMapHelper::GetDetNumFromCode(detCode);
      string currentChanName = ChannelMapHelper::GetChannelName(detCode);
      string currentChanType = ChannelMapHelper::GetChannelType(currentChanName);


      // if not the requested detector type and detnum, then skip
      if(chanType != currentChanType || detNum!= currentDetNum)  continue;

      //retrieve the vector of config values and their names
      map<string, double> configValMap = rawMapItr->second;      
      map<string, double>::const_iterator valItr = configValMap.begin();

      // now loop over remaining quantities in the map and look for the channel sampleRate
      for( ; valItr != configValMap.end(); valItr++)
      {
	string valName = valItr->first;

	if(valName == "timePerBin")  
        {

          double currentChanSampleRate  =  1.0/valItr->second; //in hz

          if (sampleRate!=-999999 && currentChanSampleRate!=sampleRate) 
	  {
             cout <<"ERROR in  DetectorConfigurationManager::GetSampleRate: "
	     <<"detector " << detNum << " has inconsistent sample rates for '"<<  chanType << "'  type!"
 	     << endl;
              exit(1);
          } 
	  else 
	  {
           sampleRate=currentChanSampleRate;
          }
  
	}

      } //end loop over config val map

    } //end loop over detectors

  }  // ====== if not raw data then check isr, info or configuration files ======

  else 
  {
 
    if(fIsInfoDataFilled)
    {
      sampleRate = fInfoData.GetSampleRate(detNum, chanType); 
    }
    else
    {
      //if not reading ISR then get this from the config file
      string parNameBase = ChannelMapHelper::GetChannelNameBaseByType(chanType);
      sampleRate = fUserData.GetDoubleParameter(detNum, parNameBase + "_SAMPLERATE");
    } //end if Info is filled

  } //end if detector configuration exists in raw data file


  if(sampleRate == -999999)
  {
    cout <<"ERROR in DetectorConfigurationManager::GetSampleRate"
	 <<"failed to find configuration for detector "<< detNum << " and channel type " << chanType
	 <<"\nCheck consistency of code."
	 << endl;

    exit(1);
  }

  return sampleRate;

}







// =====================================================================

//units for value returned is seconds  
double DetectorConfigManager::GetTriggerTime(int requestDetCode) const
{

  double triggerTime = -999999;

  string chanName = ChannelMapHelper::GetChannelName(requestDetCode);
  string chanType = ChannelMapHelper::GetChannelType(chanName);
  int    detNum   = ChannelMapHelper::GetDetNumFromCode(requestDetCode);

  // Temp - these are just for debugging
  //fIsRawDataFilled = false;
  //fIsInfoDataFilled = false;
  //fIsIsrDataFilled = false;

  // === now fill the detector configuration map ===

  // - first try to fill from the detector config record from raw data file -
  if(fIsRawDataFilled)
  {

    // Loop over the raw data config map (one entry per channel)
    map< int, map<string, double>  >::const_iterator rawMapItr = fRawConfigMap.begin();
    for( ; rawMapItr != fRawConfigMap.end(); rawMapItr++)
    {
      int detCode = rawMapItr->first;
      
      // if not the requested detector and channel, then skip
      if(requestDetCode != detCode)  continue;

      //retrieve the vector of config values and their names
      map<string, double> configValMap = rawMapItr->second;      
      map<string, double>::const_iterator valItr = configValMap.begin();

      // now loop over remaining quantities in the map and look for the channel triggerTime
      for( ; valItr != configValMap.end(); valItr++)
      {
	string valName = valItr->first;

	if(valName == "triggerTime")
	{
	  triggerTime = valItr->second; //in seconds
	}

      } //end loop over config val map

    } //end loop over detectors

  }  // - if not raw data then check isr, info or configuration files -
  else 
  {

    if(fIsInfoDataFilled)
    {
      double sampleRate = fInfoData.GetSampleRate(detNum, chanType); 
      triggerTime = fInfoData.GetPreTrigger(detNum, chanType)/sampleRate;
    }
    else
    {
      //if not reading ISR then get this from the config file
      string parNameBase = ChannelMapHelper::GetChannelNameBase(chanName);
      double sampleRate =  fUserData.GetDoubleParameter(detNum, parNameBase + "_SAMPLERATE");
      triggerTime = fUserData.GetIntParameter(detNum, parNameBase + "_PRETRIGGER")/sampleRate;

    } //end if Info is filled

  } //end if detector configuration exists in raw data file


  if(triggerTime == -999999)
  {
    cout <<"ERROR! DetectorConfigurationManager::GetDetectorConfiguration "
	 <<"failed to find configuration for requested detector code" << requestDetCode
	 <<"\nCheck consistency of code."
	 << endl;

    exit(1);
  }

  return triggerTime;

}



//returns sample rate in hz
double DetectorConfigManager::GetTriggerTime(int detNum, string chanType) const
{
  double triggerTime = -999999;


  // ====== check first detector config record from raw data file  ======

  if(fIsRawDataFilled)
  {

    // Loop over the raw data config map (one entry per channel)
    map< int, map<string, double>  >::const_iterator rawMapItr = fRawConfigMap.begin();
    for( ; rawMapItr != fRawConfigMap.end(); rawMapItr++)
    {

      // check chanType
      int detCode = (int)rawMapItr->first;
      int currentDetNum = ChannelMapHelper::GetDetNumFromCode(detCode);
      string currentChanName = ChannelMapHelper::GetChannelName(detCode);
      string currentChanType = ChannelMapHelper::GetChannelType(currentChanName);


      // if not the requested detector type and detnum, then skip
      if(chanType!=currentChanType || detNum!= currentDetNum)  continue;

      //retrieve the vector of config values and their names
      map<string, double> configValMap = rawMapItr->second;      
      map<string, double>::const_iterator valItr = configValMap.begin();

      // now loop over remaining quantities in the map and look for the channel triggerTime
      for( ; valItr != configValMap.end(); valItr++)
      {
	string valName = valItr->first;

	if(valName == "triggerTime")  
        {

          double currentChanTriggerTime  =  valItr->second; //in second

          if (triggerTime!=-999999 && currentChanTriggerTime!=triggerTime) 
	  {
             cout <<"ERROR in  DetectorConfigurationManager::GetTriggerTime: "
		  <<"detector " << detNum << " has different trigger time for '"<<  chanType << "'  type!"
		  << endl;
	     exit(1);
          } 
	  else 
	  {
	    triggerTime=currentChanTriggerTime;
          }
  
	}

      } //end loop over config val map

    } //end loop over detectors

  }  // ====== if not raw data then check isr, info or configuration files ======

  else 
  {

    if(fIsInfoDataFilled)
    {  
      double sampleRate = fInfoData.GetSampleRate(detNum, chanType); 
      triggerTime = fInfoData.GetPreTrigger(detNum, chanType)/sampleRate;
    }
    else
    {
      //if not reading INFO then get this from the config file
      string parNameBase = ChannelMapHelper::GetChannelNameBaseByType(chanType);
      double sampleRate =  fUserData.GetDoubleParameter(detNum, parNameBase + "_SAMPLERATE");
      triggerTime = fUserData.GetIntParameter(detNum, parNameBase + "_PRETRIGGER")/sampleRate;
    } //end if Info is filled

  } //end if detector configuration exists in raw data file


  if(triggerTime == -999999)
  {
    cout <<"ERROR in DetectorConfigurationManager::GetTriggerTime"
	 <<"failed to find configuration for detector "<< detNum << " and channel type " << chanType
	 <<"\nCheck consistency of code."
	 << endl;

    exit(1);
  }

  return triggerTime;

}











// =====================================================================

double DetectorConfigManager::GetTotalGain(int detCode, int eventTime) const
{

  double totalGain = -999999;

  string chanName = ChannelMapHelper::GetChannelName(detCode);
  string chanType = ChannelMapHelper::GetChannelType(chanName);
  int    detNum   = ChannelMapHelper::GetDetNumFromCode(detCode);

  if(chanType == "phonon")
  {
    double fbgain = fUserData.GetDoubleParameter(detNum, "P_FBgain");
    double digitizerbins = fUserData.GetDoubleParameter(detNum, 
							  "P_DigitizerBinsPerVolt");

    totalGain = GetDriverGain(detCode, eventTime)*fbgain*digitizerbins;
  
  }
  else if(chanType == "charge")
  {
    double gain1 = fUserData.GetDoubleParameter(detNum, "Q_Gain1");
    double digitizerbins = fUserData.GetDoubleParameter(detNum, 
							  "Q_DigitizerBinsPerVolt");

    totalGain = GetDriverGain(detCode, eventTime)*gain1*digitizerbins;
  }

  //a check that correct channel type is passed in
  if(totalGain == -999999)
  {
    cout <<"ERROR! DetectorConfigurationManager::GetPNormToAmps "
	 <<"invalid sensor type requested" << detCode
	 << endl;

    exit(1);
  }

  
  return totalGain;
}



// Use:  To normalize noise PSDs (always) and pulses (after 2011)
// Notes: The separate functions for retrieving phonon and charge normalization
// are not necessary.   They are made explicit for the purpose of helping the user
// understand the units.
double DetectorConfigManager::GetPNormADCToAmps(int detCode, int eventTime) const
{  
  double norm = GetTotalGain(detCode, eventTime);

  string chanName = ChannelMapHelper::GetChannelName(detCode);
  string chanType = ChannelMapHelper::GetChannelType(chanName);

  if(chanType != "phonon")
  {
    cout <<"ERROR! DetectorConfigManager::GetPNormADCToAmps - attempting to retrieve phonon"
	 <<"calibration for a non-phonon channel!"
	 <<endl;

    exit(1);
  }


  return norm;

}

// Use:  To normalized pulses for analysis and noise psd
double DetectorConfigManager::GetQNormADCToVolts(int detCode, int eventTime) const
{
  double norm = GetTotalGain(detCode, eventTime);

  string chanName = ChannelMapHelper::GetChannelName(detCode);
  string chanType = ChannelMapHelper::GetChannelType(chanName);

  if(chanType != "charge")
  {
    cout <<"ERROR! DetectorConfigManager::GetQNormADCToVolts - attempting to retrieve charge"
	 <<"calibration for a non-charge channel!"
	 <<endl;

    exit(1);
  }


  return norm;
}

// Use: convenience function
// Equivalent to GetQNormADCtoVolts() for charge and GetPanormADCtoAmps() for phonons
double DetectorConfigManager::GetPorQNorm(int detCode, int eventTime) const
{

  double norm = GetTotalGain(detCode, eventTime);
  
  string chanName = ChannelMapHelper::GetChannelName(detCode);
  string chanType = ChannelMapHelper::GetChannelType(chanName);

  if(chanType != "phonon" && chanType != "charge")
  {
    cout <<"ERROR! DetectorConfigManager::GetPorQNorm - attempting to retrieve channel"
	 <<"calibration for invalid channel type"
	 <<endl;

    exit(1);
  }


  return norm;

}


//Used to normalized phonon pulses for analysis, prior to 2011.
//This function exists largely only for historical purposes.
double DetectorConfigManager::GetPNormADCToWatts(int detCode, int eventTime) const
{
  int    detNum   = ChannelMapHelper::GetDetNumFromCode(detCode);

  string chanName = ChannelMapHelper::GetChannelName(detCode);
  string chanType = ChannelMapHelper::GetChannelType(chanName);

  double bias = GetBias(detCode, eventTime);
  double vbias = bias * fUserData.GetDoubleParameter(detNum, "P_Rshunt");
  double norm = GetTotalGain(detCode, eventTime)/vbias;

  if(chanType != "phonon")
  {
    cout <<"ERROR! DetectorConfigManager::GetPNormADCToWatts - attempting to retrieve phonon"
	 <<"calibration for a non-phonon channel!"
	 <<endl;

    exit(1);
  }
  
  return norm;
}




