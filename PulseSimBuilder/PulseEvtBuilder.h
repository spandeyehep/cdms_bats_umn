///////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Name: PulseEvtBuilder
//Authors: L. Hsu, M. Kos, B. Serfass
//Description:  This class interfaces the user commands, output generation, raw data reading and analysis.
//All quantities describing the event can be obtained from this class.
//
//File Import By: L. Hsu
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef EVENTBUILDER_H
#define EVENTBUILDER_H

#include "zlib.h"
#include "TTree.h"
#include <vector>
#include <map>

#include "SimulationDataManager.h"
//#include "SimulationPulseLibraryManager.h"
#include "RawDataReader.h"
#include "PulseData.h"
#include "AdminData.h"
#include "HistoryData.h"
#include "TriggerData.h"
#include "GPSData.h"

#include "UserDataManager.h"
#include "DmmDataManager.h"
#include "IsrDataManager.h"
#include "InfoDataManager.h"
#include "GpibDataManager.h"
#include "FilterDataManager.h"
#include "DatabaseManager.h"
#include "DetectorConfigManager.h"

//!Interface for user commands, output generation, raw data reading and analysis (see header file for more details) 
class PulseEvtBuilder 
{
   public:

      PulseEvtBuilder(UserDataManager& myUserData, DetectorConfigManager& myDetectorConfigManager, 
		   string& inputRawDataFile);  

      ~PulseEvtBuilder(); //destructor

      //use RawDataReader to get event, returns 0 if end of file, <0 if read error
      int ReadNextEvent();
      int ReadEventN(int eventN);

      //getting data objects (all const functions)
      
      AdminData    GetAdmin()   const  { return fAdminData;    }
      HistoryData  GetHistory() const  { return fHistoryData;  }
      TriggerData  GetTrigger() const  { return fTriggerData;  }
      GPSData      GetGPS()     const  { return fGPSData; }
      
      uint32_t     GetEventCategory();

      //registering external managers
      
      void RegisterInfo(InfoDataManager& myInfo) { 
	fInfoData = myInfo;
	fInfoIsRegistered = true;
      }

      void RegisterIsr(IsrDataManager& myIsr) { 
	fIsrData = myIsr;
	fIsrIsRegistered = true;
      }

      void RegisterDmm(DmmDataManager& myDmm) { 
	fDmmData = myDmm;
	fDmmIsRegistered = true;
      }
      
      void RegisterGpib(GpibDataManager& myGpib) { 
	fGpibData = myGpib;
	fGpibIsRegistered = true;
      }

      void RegisterFilter(FilterDataManager& myFilter) { 
	fFilterData = myFilter;
	fFilterIsRegistered = true;
      }
      
      void RegisterDatabase(CdmsDB::DatabaseManager& mydb) { 
	fDatabaseManager = mydb;
	fDatabaseIsRegistered = true;
      }
      
      
      
      //getting external files data (all const functions)
      InfoDataManager GetInfoData()  const    { return fInfoData;   }
      IsrDataManager GetIsrData()  const      { return fIsrData;    }
      DmmDataManager GetDmmData()  const      { return fDmmData;    } 
      GpibDataManager GetGpibData() const     { return fGpibData;   }
      FilterDataManager GetFilterData() const { return fFilterData; }
      const CdmsDB::DatabaseManager& GetDatabaseManager() const 
      { return fDatabaseManager; }

      //getting pulse collections
      void FillSingleZipPulseCollection(vector<PulseData>& pulseCollection, const int& detNum) const;
      void FillVetoPulseCollection(vector<PulseData>& pulseCollection) const;

      //getting general info about the event
      int      GetNZipPulses()    const { return fMapOfZipPulses.size(); }
      int      GetNVetoPulses()   const { return fVectorOfVetoPulses.size(); }
      uint32_t GetEventCategory() const { return fRawReader.GetEventCategory(); }
      uint32_t GetEventType()     const { return fRawReader.GetEventType(); }

      // Pulse simulation settings
      void SetSimDataManager(string input_filename);
      void ReadSimEvent();
      //void SetPulseLibManager(string input_filename);

      //Veto
      void DoVetoAnalysis();

      // GPIB timing calculations (Flash times)
      void DoGpibTimingCalc();
      
      // ISR timing calculations (LastISRTime)
      void DoIsrTimingCalc();
  
      // DMM calculations
      void DoDmmCalc(int detNum);

      // ISR  calculations
      void DoIsrCalc(int detNum);
      
      // Database stuff
      void DoDatabaseEventCalc();
      //void DoDatabasePulseCalc(int detNum); // commented out because of compilation issues [AJA]
      
      //Basic Pulse Analysis
      void DoBasicPulseCalc(int detNum);

      //Phonon Algorithms
      void DoPulseIntegral(int detNum, const string& sensorType, const string& pulseType = "filtered"); //filtered if not specified
      void DoInflectionTime(int detNum, const string& sensorType);
      void DoVarFreqRTFTWalkPhonon(int detNum, const string& sensorType, const string& pulseType = "filtered"); //filtered if not specified
      void DoConstFreqRTFTWalkPhonon(int detNum, const string& sensorType,  const string& pulseType); //filtered if not specified
      void DoOptimalFilterPhonon(int detNum, const string& sensorType);  
      void DoOptimalFilterPhononGlitch1(int detNum, const string& sensorType);      
      void DoOptimalFilterPhononLFnoise1(int detNum, const string& sensorType);     
      void DoNoiseSelector(int detNum, const string& sensorType);
      void DoPipeFitPhonon(int detNum, const string& sensorType);
      void DoWedgeFitPhonon(int detNum, const string& sensorType);
      void DoTailFitPhonon(int detNum, const string& sensorType);

      //Charge Algorithms
      void DoRTFTWalkCharge(int detNum, const string& sensorType, const string& pulseType = "filtered"); //filtered if not specified
      void DoOptimalFilterChargeX(int detNum,const string& side=""); // side = "S1", "S2"
      void DoOptimalFilterCharge(int detNum);
      void DoF5ChargeX(int detNum,const string& side="");// side = "S1", "S2"

      //User Analysis Classes - DO NOT modify or copy this comment (for auto_analysis)
      void DoSimulatePhononFromRandoms(int detNum);
      //void DoSimulateFromPulse(int detNum);
      void DoSimulateChargeFromRandoms(int detNum);
      void DoOptimalFilterPhononNS(int detNum, const string& sensorType);
      void DoOptimalFilterCharge2X2(int detNum);
      void DoPSDIntegralPhonon(int detNum, const string& sensorType);

 
   private:

      //default constructor
      PulseEvtBuilder();

      //utility
      bool IsChosenType(const PulseData& aPulseData, const string& whichPulses) const;
      
      //Readers
      RawDataReader fRawReader;
      SimulationDataManager fSimDataReader;
      //SimulationPulseLibraryManager fSimLibManager;

      //Data Objects
      AdminData          fAdminData;
      HistoryData        fHistoryData;
      TriggerData        fTriggerData;
      GPSData            fGPSData;
      vector<PulseData> fVectorOfVetoPulses;
      map< int, vector<PulseData> > fMapOfZipPulses;  //key is zip#
       
      //External data 

      UserDataManager   fUserData;
      InfoDataManager   fInfoData;
      IsrDataManager    fIsrData;
      DmmDataManager    fDmmData;
      GpibDataManager   fGpibData;
      FilterDataManager fFilterData;
      CdmsDB::DatabaseManager fDatabaseManager;

      //possibly defunct now, replaced by the "IsRegistered" bools
      bool     fReadIsr; //true by default
      bool     fReadInfo; //true by default

      bool     fInfoIsRegistered;
      bool     fIsrIsRegistered;
      bool     fDmmIsRegistered;
      bool     fGpibIsRegistered;
      bool     fFilterIsRegistered;
      bool     fDatabaseIsRegistered;
      //other
      
      DetectorConfigManager fDetectorConfigManager;  

};

#endif /* EVENTBUILDER_H */
