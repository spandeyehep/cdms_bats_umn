///////////////////////////////////////////////////////////////////////////////// 
//Class Name: GenRRQDataCDMSII
//Authors: L. Hsu
//Description: Applies charge calibration (cross talk, amplitude scaling, 
//position correction).  Applies phonon calibration (relative amplitude scaling).  
//Generates primary rrq's for analysis (xy-delays, partitions, yields, timing 
//quantities.  This is analogous to the cdmspipe "PC_first_pass.m" script.   
//Imported from cdmspipe and upgraded to be compatible (and backwards compatible) 
//with hybrid mercedes/CDMSII running
//
//File Import By: L. Hsu
//Creation Date: Dec. 23, 2009
//
//Modifications:
//
//Jan 2011 (LLH):  This file was renamed from GenerateRRQData to it current name, 
//                 GenRRQDataCDMSII.  Event-level rrq's removed from this class 
//                 and placed into new class GenRRQDataEvent
//
////////////////////////////////////////////////////////////////////////////////// 


#ifndef GENRRQDATACDMSII_H
#define GENRRQDATACDMSII_H

#include <iostream>
#include <vector>
#include <map>

#include "BatCalibIOManager.h"

using namespace std;

//!This is the GenRRQDataCDMSII Class.
class GenRRQDataCDMSII 
{
   public:

      GenRRQDataCDMSII(BatCalibIOManager ioManager);  
      ~GenRRQDataCDMSII(); //destructor 

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

      void CalcVarFreqRTFTWalkRRQ();
      void CalcConstFreqRTFTWalkRRQ();
      void CalcPipeFitRRQ();
      void CalcWedgeFitRRQ();

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

#endif /* GENRRQDATACDMSII_H */
