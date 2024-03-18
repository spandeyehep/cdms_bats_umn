///////////////////////////////////////////////////////////////////////////////// 
//Class Name: GenRRQDataCDMSII
//Authors: L. Hsu
//Description: This class is intended for the endcap detectors - old CDMSII style
//detectors with the phonon channels ganged 2 per side and charge ganged into one.
//Applies charge calibration (cross talk, amplitude scaling, position 
//correction).  Applies phonon calibration (relative amplitude scaling).  Generates 
//primary rrq's for analysis (xy-delays, partitions, yields).
//
//File Import By: L. Hsu
//Creation Date: Jan. 18, 2011 
//
//Modifications:
//
////////////////////////////////////////////////////////////////////////////////// 


#ifndef GENRRQDATAENDCAP_H
#define GENRRQDATAENDCAP_H

#include <iostream>
#include <vector>
#include <map>

#include "BatCalibIOManager.h"

using namespace std;

//!This is the GenRRQDataEndcap Class.
class GenRRQDataEndcap 
{
   public:

      GenRRQDataEndcap(BatCalibIOManager ioManager);  
      ~GenRRQDataEndcap(); //destructor 

      void DoCalibration(int maxEvents, int detNum, 
			 UserDataManager& myUserData);


   private:

      // ===== functions =====

      void ConstructRRQList();
      void ResetRRQValues();
      void ActivateRQs();

      //mandatory calculations

      void ApplyPhononCalibration();
      void CalcPhononDelays();
      void ApplyChargeCalibration();
      void CalcTotalEnergiesAndYields();
      void CalcPartitions();
      void CalcOFResolutions();

      // helpers
      

      // ===== data members =====

      map<string, double> fRRQList;
      BatCalibIOManager   fIOMan;
      UserDataManager     fUserData;

      //detector descriptions

      int               fDetNum;
      int               fDetType;
      bool              fIsGe;
      bool              fIsSi;
      vector<double>    fDelaySig;
      vector<double>    fAmpSig;


      //data descriptions

      
      //calibration

      vector<double> fChargeCal;
      vector<double> fChargePosX;
      vector<double> fChargePosY;
      vector<double> fXFitMinMax;
      double         fXFitMin;
      double         fXFitMax;
      vector<double> fYFitMinMax;
      double         fYFitMin;
      double         fYFitMax;

      vector<double> fPhononCal;
      vector<double> fPhononIntCal;
      double         fEpsilon;

      vector<double> fOFMaxTemplate;


};

#endif /* GENRRQDATAENDCAP_H */
