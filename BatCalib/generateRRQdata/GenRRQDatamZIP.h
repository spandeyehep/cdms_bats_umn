///////////////////////////////////////////////////////////////////////////////// 
//Class Name: GenRRQDatamZIP
//Authors: L. Hsu
//Description: This class is intended for the 1-inch, single side phonon and single
//side charge ZIPs with "mercedes" style phonon sensor layout (4 channels) - the mZIP.
//Generates primary rrq's for analysis (xy-delays, partitions, yields, timing 
//quantities).  
//
//File Import By: L. Hsu
//Creation Date: Jan. 18, 2011
//
//Modifications:
//
////////////////////////////////////////////////////////////////////////////////// 


#ifndef GENRRQDATAMZIP_H
#define GENRRQDATAMZIP_H

#include <iostream>
#include <vector>
#include <map>

#include "BatCalibIOManager.h"

using namespace std;

//!This is the GenRRQDataMZIP Class.
class GenRRQDatamZIP 
{
   public:

      GenRRQDatamZIP(BatCalibIOManager ioManager);  
      ~GenRRQDatamZIP(); //destructor 

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

      //optional timing calculations

      void CalcConstFreqRTFTWalkRRQ();

      // helpers
      
      void FindPrimaryPhononChannel();

      // ===== data members =====

      map<string, double> fRRQList;
      BatCalibIOManager   fIOMan;
      UserDataManager     fUserData;

      //detector descriptions

      int               fDetNum;
      int               fDetType;
      bool              fIsGe;
      bool              fIsSi;
      int               fPrimaryPhononChan; //0 = PA, 1 = PB, 2 = PC, 3 = PD, -999999 (uninitialized)
      int               fPrimaryInnerPhononChan; //1 = PB, 2 = PC, 3 = PD, -999999 (uninitialized) - only for mercedes
      vector<double>    fDelaySig;
      vector<double>    fAmpSig;


      //data descriptions
      bool              fIsFirstProduction; //some rq's not stored in first BatRoot production, so we can't compute rrq's

      
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

      //for mercedes delay calculation

      double kThetaVect[3]; //values defined in constructor

};

#endif /* GENRRQDATAMZIP_H */
