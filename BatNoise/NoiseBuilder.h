///////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Name: NoiseBuilder
//Authors: L. Hsu, M. Kos, B. Serfass
//Description:  This class performs the analysis of traces for noise generation.  It controls the output generation of the noise files.
//It also interfaces the user commands and raw data reading.
//
//File Import By: L. Hsu
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef NOISEBUILDER_H
#define NOISEBUILDER_H

#include "zlib.h"
#include "TFile.h"
#include <vector>
#include <map>

#include "RawDataReader.h"
#include "PulseData.h"
#include "AdminData.h"
#include "HistoryData.h"
#include "UserDataManager.h"
#include "IsrDataManager.h"
#include "InfoDataManager.h"
#include "TemplateDataManager.h"
#include "NoiseData.h"
#include "CorrelationData.h"

#include "DetectorConfigManager.h"

//!This class performs analysis of traces for noise generation and outputs noise files.  Interfaces w/ raw data reader and user settings file.
class NoiseBuilder 
{
   public:

      NoiseBuilder(UserDataManager& myUserData, DetectorConfigManager& myDetConfigManager,
		   TemplateDataManager& myTemplateData);
      ~NoiseBuilder(); //destructor

      void ConfigureNoiseBuilder();
      void ConfigureCorrelationData(); //for special studies

      //For reading the raw data files
      void OpenRawFile(const string& inputRawDataFile);
      void ResetRawFile();
      int  ReadNextEvent();

      //For the output file
      void ConfigureOutputFile(const string& outputFileName);
      void WriteOutputFile();
      void StorePulses(int detNum);

      //getting data objects (all const functions)
      AdminData   GetAdmin() const    { return fAdminData; }

      //getting pulse collections - FIXME are these needed?
      void FillSingleZipPulseCollection(vector<PulseData>& pulseCollection, const int& detNum) const; 

      //getting general info about the event
      int      GetNZipPulses()    const { return fMapOfZipPulses.size(); }
      uint32_t GetEventCategory() const { return fRawReader.GetEventCategory(); }
      uint32_t GetEventType()     const { return fRawReader.GetEventType(); }

      //pulse calculations
      void CalcSumOfPulses(int detNum);

      //for noise selection
      void ConstructMinMaxDistribution(int detNum);
      void ConstructPileUpDistribution(int detNum);
      void ConstructPOFchisqDistribution(int detNum);
      void ConstructPTBSslopeDistribution(int detNum);
 
      void CalcMinMaxCut(int detNum); //based on filled minmax distribution
      void CalcPileupCut(int detNum, bool isTFData); //based on filled pileup distribution
      void CalcPOFchisqCut(int detNum); 

      int PassRandomTriggerCut();
      int PassSaturationCut(int detNum);  
      int PassMinMaxCut(int detNum);
      int PassPileupCut(int detNum);
      int PassPOFchisqCut(int detNum);
      int PassPTBSslopeCut(int detNum);
   
      // Optimal Filter calculation
      double DoOptimalFilter(vector<double> pulse, double sampleRate);

      //for noise calculations
      void BuildAveragePSD(int detNum);

      
      void CalcNoiseQuantities(const int detNum, const int totEvtCt); //also calculates NoiseFFTsq
      void CalcNoiseWithCrossTalk(vector<NoiseData>* noiseDataList, const int detNum, const int totEvtCtr,const string& side="");
      void CalcNoiseNoCrossTalk(vector<NoiseData>* noiseDataList, const int detNum, const int totEvtCtr);

      //for special studies
      void BuildQIQOCov(int detNum);

    


   private:

      //default constructor
      NoiseBuilder();

      //utility
      bool IsChosenType(const PulseData& aPulseData, const string& whichPulses) const;
      
      //Raw Data Reader
      RawDataReader fRawReader;

      //Output File 
      TFile* fOutputFile;

      //ExtDataManager
      UserDataManager fUserData;
      //      IsrDataManager fIsrData;
      //InfoDataManager fInfoData;
      TemplateDataManager fTemplateData;

      //Data Objects
      AdminData          fAdminData;

      //consider implementing a conglomerate class to hold these
      map< int, vector<PulseData> > fMapOfZipPulses;    //key is zip#
      map< int, vector<NoiseData> > fMapOfNoiseData;    //key is zip#
      map< int, vector<CorrelationData> > fMapOfCorrelationData; //key is zip#
      
      //External data
      bool     fReadIsr; //true by default
      bool     fReadInfo; //true by default

      //other      
      DetectorConfigManager fDetConfigManager;  

};

#endif /* NOISEBUILDER_H */
