///////////////////////////////////////////////////////////////////////////////// 
//Class Name: GenRRQDataCDMSliteI
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


#ifndef GENRRQDATACDMSLITEI_H
#define GENRRQDATACDMSLITEI_H

#include <iostream>
#include <vector>
#include <map>

#include "BatCalibIOManager.h"

using namespace std;

//!This is the GenRRQDataCDMSliteI Class.
class GenRRQDataCDMSliteI 
{
   public:

      GenRRQDataCDMSliteI(BatCalibIOManager ioManager);  
      ~GenRRQDataCDMSliteI(); //destructor 

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
  void Calc2TRadialParameter();
      void ApplyChargeCalibration();
      void CalcTotalEnergiesAndYields();
      void CalcPartitions();
      void CalcOFResolutions();
  
  int Sign(double x);//[wap]:fix me: is there not an existing cdmsbats function for this?

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
      int               fPrimaryPhononChanOF; //0 = PA, 1 = PB, 2 = PC, 3 = PD, -999999 (uninitialized)
      int               fPrimaryInnerPhononChanOF; //1 = PB, 2 = PC, 3 = PD, -999999 (uninitialized) - only for mercedes
      int               fPrimaryPhononChanWK; //0 = PA, 1 = PB, 2 = PC, 3 = PD, -999999 (uninitialized)
      int               fPrimaryPhononChanOFWK; //0 = PA, 1 = PB, 2 = PC, 3 = PD, -999999 (uninitialized)
      vector<double>    fDelaySig;
      vector<double>    fAmpSig;


      // RQ availability flag
      bool fCheckOFChargeXRQ;
      bool fCheckOFChargeRQ;

      //data descriptions
      bool              fIsFirstProduction; //some rq's not stored in first BatRoot production, so we can't compute rrq's

      // relative calibration
      vector<double> fPhononRelCal;      
    
      //calibration
      vector<double> fChargeCal;

      vector<double> fPhononIntCal;
      double fTotPhononOFCal;
      double fTotPhononNFCal;
      double fPhononTailCal;
      double fEpsilon;

  double fPhononOFCal;
  double fPhononOFCalCorr;
  double fPhononNFCal;
  double fPhonon2TCal;
  // individual channel slow amplitudes
  double fPhonon2TaCal;
  double fPhonon2TbCal;
  double fPhonon2TcCal;
  double fPhonon2TdCal;
  // individual channel fast amplitudes
  double fPhonon2TarCal;
  double fPhonon2TbrCal;
  double fPhonon2TcrCal;
  double fPhonon2TdrCal;

  double CorrOF2Tr;
  double CorrNF2Tr;
  // this is the total phonon 2T amplitude
  double Corr2T2Tr;
  // these are the individual channel 2T slow amp
  // note that the correction of the individual channels
  // are done with the individual channels' residual amplitudes
  double Corr2Ta2Tr;
  double Corr2Tb2Tr;
  double Corr2Tc2Tr;
  double Corr2Td2Tr;

  vector<double> fPhononOFCalVect;
  vector<double> fPhononNFCalVect;
  vector<double> fPhonon2TCalVect;
  vector<double> fPhonon2TaCalVect;
  vector<double> fPhonon2TbCalVect;
  vector<double> fPhonon2TcCalVect;
  vector<double> fPhonon2TdCalVect;
  vector<double> fPhonon2TarCalVect;
  vector<double> fPhonon2TbrCalVect;
  vector<double> fPhonon2TcrCalVect;
  vector<double> fPhonon2TdrCalVect;

  vector<double> fRadPhiCorrVect;

  vector<double> fPartitionCorrVect;

  double pa2t_delr;
  double pb2t_delr;
  double pc2t_delr;
  double pd2t_delr;
  
  double px2t_delr;
  double py2t_delr;

      vector<double> fOFMaxTemplate;

      //for mercedes delay calculation

      double kThetaVect[3]; //values defined in constructor

  // map baseTemp>0
  map<int,double> fGoodBaseTempMap;
  double fVnom;
  double fRb;
  

};

#endif /* GENRRQDATACDMSLITEI_H */
