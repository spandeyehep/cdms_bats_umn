///////////////////////////////////////////////////////////////////////////////// 
//Class Name: GenRRQDataiZIPSoudan
//Authors: L. Hsu
//Description: This class is intended for the 1-inch, 12-channel iZIP, with
//mercedes phonon channel layout. Generates primary rrq's for analysis (xy-delays, 
//partitions, yields, timing quantities).  
//
//File Import By: L. Hsu
//Creation Date: Jan. 18, 2011
//
//Modifications:
//
////////////////////////////////////////////////////////////////////////////////// 


#ifndef GENRRQDATAIZIPSOUDAN_H
#define GENRRQDATAIZIPSOUDAN_H

#include <iostream>
#include <vector>
#include <map>

#include "TGraph.h"

#include "BatCalibIOManager.h"

using namespace std;

//!This is the GenRRQDataiZIPSoudan Class.
class GenRRQDataiZIPSoudan 
{
   public:

      GenRRQDataiZIPSoudan(BatCalibIOManager ioManager);  
      ~GenRRQDataiZIPSoudan(); //destructor 

      void DoCalibration(int maxEvents, int detNum, 
			 UserDataManager& myUserData);


   private:

      // ===== functions =====

      void ConstructRRQList();
      void ResetRRQValues();
      void ActivateRQs();
      void CalcLindhardLookupTable(); //lookup table relating pt to pr (NR hypothesis, Lindhard yield)

      //mandatory calculations

      void ApplyPhononCalibration();
      void CalcPhononDelays();
      void ApplyChargeCalibration();
      void CalcTotalEnergies();
      void CalcYields();
      void CalcPartitions();
      void CalcOFResolutions();

      //optional timing calculations

      void CalcConstFreqRTFTWalkRRQ();

      // helpers
      
      void FindPrimaryPhononChannel();
      void CreateOnOffChannelSwitches();

      // ===== data members =====

      map<string, double> fRRQList;
      BatCalibIOManager   fIOMan;
      UserDataManager     fUserData;

      //detector descriptions

      int               fDetNum;
      int               fDetType;
      vector<double>    fDelaySig;
      vector<double>    fAmpSig;

     
      //data descriptions
      bool              fIsFirstProduction; //some rq's not stored in first BatRoot production, so we can't compute rrq's
      bool              fCheckOFChargeXRQ;  //if rq's for an OFChargeX routine exist
      bool              fCheckOFChargeRQ;   //if rq's for an OFCharge routine exist
      bool              fCheckF5ChargeXRQ;  //if rq's for a F5ChargeX routine exist
      double            fPreviousEventSeriesNumber;  

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
      
      vector<double> fChargeF5Cal;

      // relative calibration
      vector<double> fPhononRelCal;

      //overall phonon calibraton
      double fPhononOFCal;
      double fTotPhononOFCal;
      double fTotPhononNFCal;
      double fPhononIntCal;
      double fPhononTailCal;
      vector<double> fPhononOFCalVect;
      vector<double> fTotPhononOFCalVect;
      vector<double> fTotPhononNFCalVect;
      vector<double> fPhononIntCalVect;
      vector<double> fPhononTailCalVect;

      double         fEpsilon;

      vector<double> fOFMaxTemplate;

      map<string, int>    fPhononOnOffSwitches;
      map<string, int>    fChargeOnOffSwitches;
      int                 fNWorkingPhonon;
      vector<string>      fBrokenPhononChannels;

      //for NR hypthesis recoil energy calculations
      TGraph* fNRLookupTable;
      double  fMaxInterpolatedRecoil;
      double  fMaxInterpolatedpt;

      //for mercedes delay calculation

      double kTheta1Vect[3]; //values defined in constructor
      double kTheta2Vect[3]; //values defined in constructor

      // map baseTemp>0
      map<int,double> fGoodBaseTempMap;

};

#endif /* GENRRQDATAIZIPSOUDAN_H */
