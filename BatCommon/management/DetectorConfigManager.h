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
//Creation Date: Dec. 10, 2010
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

#ifndef DETECTORCONFIGMANAGER_H
#define DETECTORCONFIGMANAGER_H

#include <iostream>
#include <map>
#include <vector>
#include <list>
#include <cstdlib>

#include "DetectorConfigData.h"

#include "UserDataManager.h"
#include "IsrDataManager.h"
#include "InfoDataManager.h"


using namespace std;

//!This class manages the detector configuration information (specifically parameters related to the channel settings and their normalizations.
class DetectorConfigManager 
{
   public:
 
      // constructor - initialize with DetectorConfigData or just map of config data?
      DetectorConfigManager(UserDataManager& myUserData, string& inputRawDataFile);

      //default constructor
      DetectorConfigManager();

      // destructor
      ~DetectorConfigManager();      

      // register external data classes
      void RegisterInfo(InfoDataManager& infoData) { 
	fInfoData = infoData; 
	fIsInfoDataFilled = true; }

      void RegisterIsr(IsrDataManager& isrData)  { 
	fIsrData = isrData; 
        fIsIsrDataFilled = true; }


      // return the configured detector list
      map<int, int> GetDetectorMap(); //key = detector number, val = detector type


      // return all detector configuration parameters for that detector
      map<string, double> GetDetectorConfiguration(int detNum); //key = name, val = config parameter 


      // Get detector settings - eventTime only active for ISR reading

      double GetBias(int detCode, int eventTime) const; //units?
      double GetBiasTime(int detCode, int eventTime) const; 
      double GetDriverGain(int detCode, int eventTime) const;
      double GetTotalGain(int detCode, int eventTime) const;

      // get polarity configuration for each detectors ("pos" = +/-, "neg" = -/+, or "unknown)
      // only for raw data with bias information (and not form ISR file or user settings)
      map<int, string>  GetDetectorChargePolarityMap();
    

      // Get digitization settings

      double GetSampleRate(int detCode) const;  
      double GetSampleRate(int detNum, string chanType) const;  //in hz

      double GetTraceLength(int detCode) const;
      double GetTraceLength(int detNum, string chanType) const; //in adc bins - FIXME make this return int
   
      double GetTriggerTime(int detCode) const;
      double GetTriggerTime(int detNum, string chanType) const;//in seconds





      // === Get Normalizations  ===

      // Notes: Based on the way we now normalize phonon pulses (ADC to amps since 2011),
      // the separation of functions for retrieving phonon and charge normalization
      // is not really necessary.   They are made explicit for the purpose of helping the user
      // understand the units and provides the option for reverting back to old phonon pulse 
      // normalizations (ADC to watts), if ever that is needed. 
      // The eventTime is only active for ISR reading.

      double GetPNormADCToAmps(int detCode, int eventTime) const;
      double GetQNormADCToVolts(int detCode, int eventTime) const;
      double GetPorQNorm(int detCode, int eventTime) const;
      double GetPNormADCToWatts(int detCode, int eventTime) const;  //largely ony for historical use


    private:
      
      //the configured detector list
      map<int,int> fDetectorMap; //key = zip#, val = detector type

      //Objects related to raw data config
      DetectorConfigData fDetectorConfigData; 
      map< int, map<string, double> > fRawConfigMap; //from DetectorConfigData, key = det code, val = config list 
 

      //External data managers
      UserDataManager   fUserData;
      InfoDataManager   fInfoData;
      IsrDataManager    fIsrData;
      
      //for keeping tabs on which data is available
      bool fIsRawDataFilled;
      bool fIsIsrDataFilled;
      bool fIsInfoDataFilled;

      //for debugging
      bool fDebugOn;
};




#endif /* DETECTORCONFIGMANAGER_H */
