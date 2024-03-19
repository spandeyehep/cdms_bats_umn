/////////////////////////////////////////////////////////////////////// 
///////////////////////////////
//Class Name: PipeFitPhonon
//Authors: M. Kos
//Description:  This class is the equivalent of the 5 parameter Time Domain fit from the  
//PipeFitter code.  The fit is only applied to phonon pulses.
//Original authors of the PipeFitter code are L. Duong, J. Yoo, and E. Ramberg 
//
//File Import By: M. Kos
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
//////////////////////////////////////////////////////////////////////// 
///////////////////////////////

#ifndef PipeFitPhonon_H
#define PipeFitPhonon_H

#include <iostream>
#include <map>
#include <vector>

#include "TMinuit.h"
#include "TString.h"
#include "TMath.h"

#include "TCDMSAnalysis.h"
#include "PulseTools.h"


using namespace std;

//! A class containing functions related to the Time Domain fit.
class PipeFitPhonon : public TCDMSAnalysis
{
   public:

      //do not modify the signature of this constructor
      //if you need a constructor with a different signature, than define a separate constructor
      PipeFitPhonon();  
      ~PipeFitPhonon(); //destructor 

      //Set parameters
      void InitializeParameters(const vector<double> &pulsevector,const vector<double> &det_thresh, int channum, double RMS); //do it here and not in the constructor

      //do the calculations
      void DoCalc(); 

      //define public functions here

      //!Return Chi2 of fit
      double GetChi2RF(){if (IsSmall() || IsMedium()) return fchi2rf; else return -999999.0;}

      double GetChi2FF(){if (IsMedium() || IsLarge()) return fchi2ff; else return -999999.0;}

      double GetDOFRF(){if (IsSmall() || IsMedium()) return fndfrf; else return -999999.0;}

      double GetDOFFF(){if (IsMedium() || IsLarge()) return fndfff; else return -999999.0;}
      
      
      //!Return true if pulse is small pulse
      bool IsSmall() {return fIsSmall;}
      
      //!Return true if pulse is medium pulse
      bool IsMedium() {return fIsMedium;}

      //!Return true if pulse is large pulse
      bool IsLarge() {return fIsLarge;}

      //!Return true if pulse is saturated
      //This also tests for weird saturations, maybe should eventually be in PulseTools
      bool TEStest();

      //!fit using the rise function
      void FitRiseFunc();

      //!fit using the fall function
      void FitFallFunc();

      double GetRiseFuncStart() {return fstart + 1.0;}
      
      double GetRiseFuncEnd() {return fmidpt + 1.0;}
      
      double GetRiseFuncChi2() {return fchi2rf;}
      
      //Rise function return values
      //risefunc = a0 * {1-exp(-(t-t0fit)/tau)} * {exp(-(t-t0fit)/kappa) - a1*exp(-(t-t0fit)/tau)}

      double GetA0() {return fRiseFuncVal[0];}

      double GetT0(); //corrected t0 value
      
      double GetT0Fit() {if (fRiseFuncVal[1] != kFailValue) return fRiseFuncVal[1]+1; else return kFailValue;}
      
      double GetTau() {return fRiseFuncVal[2];}
      
      double GetKappa() {return fRiseFuncVal[3];}
      
      double GetA1() {return fRiseFuncVal[4];}
      
      double GetEA0() {return fRiseFuncSig[0];}
      
      double GetET0Fit() {return fRiseFuncSig[1];}
      
      double GetETau() {return fRiseFuncSig[2];}
      
      double GetEKappa() {return fRiseFuncSig[3];}
      
      double GetEA1() {return fRiseFuncSig[4];}

      
      
      //!Return start time of rising function fit
      double GetFallFuncStart() {return fmidpt + 1.0;}
      
      //!Return end time of rising function fit
      double GetFallFuncEnd() {return fsize + 1.0;}
      
      //Fall function return values
      //fall function is fallfunc = af * {exp(-t/tf1) - tfr*exp(-t/tf2)}

      double GetAf() {return fFallFuncVal[0];}
      
      double GetTf1() {return fFallFuncVal[1];}
      
      double GetTfr() {return fFallFuncVal[2];}
      
      double GetTf2() {return fFallFuncVal[3];}
      
      double GetEAf() {return fFallFuncSig[0];}
      
      double GetETf1() {return fFallFuncSig[1];}
      
      double GetETfr() {return fFallFuncSig[2];}
      
      double GetETf2() {return fFallFuncSig[3];}

      //=======================================================================
      // Functions from PipeRoot
      //total integral
      double GetInt();

      //rise function integral
      double GetIntRiseFunc();

      //fall function integral
      double GetIntFallFunc();

      double GetT10();

      double GetT20();
  
      double GetT30();
    
      double GetT40();

      double GetT50();
 
      double GetT60();

      double GetT70();

      double GetT80();

      double GetT90();
  
      double GetT030Chisq();

      double GetT3060Chisq();
  
      double GetT60100Chisq();

      //get time at peak
      double GetTPeak();


      //Newton-Raphson method
      double rtsafe(void (*funcd)(double *,double, double *, double *), double *pr, double x1, double x2, double xacc);

      //===============================================================================================================
      //Set functions, set values used in fit
      void SetStartWindowMin(int startmin) {fStartWindowMin = startmin;}

      void SetStartWindowMax(int startmax) {fStartWindowMax = startmax;}
 
      void SetStartRMSMultiplier(double startRMS) {fStartRMSMult = startRMS;}

      void SetStartWalkMultiplier(double startwalk) {fStartWalkMult = startwalk;}

      void SetStartThreshCheck(double startcheck) {fStartThreshCheck = startcheck;}
      
      void SetMaxThreshCheck(double maxcheck) {fMaxThreshCheck = maxcheck;}

      void SetLargeRMSMult(double largeRMS) {fStartLargeRMSMult = largeRMS;}

      void SetSmallRMSTest(double smallRMS) {fTestSmallRMS = smallRMS;}

      void SetStartSmallDefault(int smallStart) {fStartTimeDefaultSmall = smallStart;}
      
      void SetMaxADCBinStartDiff(double maxBinStartDiff) {fMaxBinStartDiff = maxBinStartDiff;}

      void SetMaxADCBinStartMult(double maxBinStartMult) {fMaxBinStartMult = maxBinStartMult;}

      void SetMaxADCBinStartAdd(double maxBinStartAdd) {fMaxBinStartAdd = maxBinStartAdd;}

      void SetMaxADCBinAdd(double maxBinAdd) {fMaxBinAdd = maxBinAdd;}

      void SetMidpointDefault(double midPointDefault) {fMidpointDefault = midPointDefault;}

      void SetFallFuncEnd(double fallFuncEnd) {fFallFuncEnd = fallFuncEnd;}

      void SetRiseFuncA0Default(double riseFuncA0Default) {fRiseFuncA0Default = riseFuncA0Default;}

      void SetRiseFuncT0Default(double riseFuncT0Default) {fRiseFuncT0Default = riseFuncT0Default;}

      void SetRiseFuncTauDefault(double riseFuncTauDefault) {fRiseFuncTauDefault = riseFuncTauDefault;}

      void SetRiseFuncKappaDefault(double riseFuncKappaDefault) {fRiseFuncKappaDefault = riseFuncKappaDefault;}

      void SetRiseFuncA1Default(double riseFuncA1Default) {fRiseFuncA1Default = riseFuncA1Default;}

      void SetRiseFuncPulseHeightMult(double riseFuncMaxMult) {fRiseFuncPulseHeightMult = riseFuncMaxMult;}

      void SetRiseFuncStartBinDiff(double riseFuncStartBinDiff) {fRiseFuncStartBinDiff = riseFuncStartBinDiff;}

      void SetRiseFuncTauMult(double riseFuncTauMult) {fRiseFuncTauMult = riseFuncTauMult;}
 
      void SetRiseFuncKappaMult(double riseFuncKappaMult) {fRiseFuncKappaMult = riseFuncKappaMult;}

      void SetFallFuncAfAdd(double fallFuncAfAdd) {fFallFuncAfAdd = fallFuncAfAdd;}

      void SetFallFuncTf1Start(double fallFuncTf1Start) {fFallFuncTf1Start = fallFuncTf1Start;}
 
      void SetFallFuncTf2Start(double fallFuncTf2Start) {fFallFuncTf2Start = fallFuncTf2Start;}

      void SetFallFuncTfrStart(double fallFuncTfrStart) {fFallFuncTfrStart = fallFuncTfrStart;}
      
      void SetFallFuncStepSizeAdd1(double fallFuncStepSize) {fFallFuncStepSize1 = fallFuncStepSize;}
  
      void SetFallFuncStepSizeAdd2(double fallFuncStepSize) {fFallFuncStepSize2 = fallFuncStepSize;}

      void SetMaxTraceStartSat(double maxTraceStartSat) {fMaxTraceStartSat = maxTraceStartSat;}

      void SetMaxTraceDiffSat(double maxTraceDiffSat) {fMaxTraceDiffSat = maxTraceDiffSat;}

      void SetPulseheightMaxSat(double pulseheightMaxSat) {fPulseheightMaxSat = pulseheightMaxSat;}

      void SetNumberSatBins(double numberSatBins) {fNumberSatBins = numberSatBins;}

   private:

      void ConstructRQList();

      static const int kNormFitStep = 200; //step size determined by start_par/kNormFitStep
      static const int kNumParsRF = 5;
      static const int kMaxMIGRADCalls = 4000;
      static const int kNumParsFF = 4;
      static const int kMaxIT = 100; //max iterations used for Netwon-Raphson method
      static const int kFailValue = -999999;

      //const double kAccuracy = 0.01;
      const double kAccuracy;

      //fit pre-set values
      int fStartWindowMin;
      int fStartWindowMax;
      double fStartRMSMult;
      double fStartWalkMult;
      double fStartThreshCheck;
      double fMaxThreshCheck;
      double fStartLargeRMSMult;
      double fTestSmallRMS;
      int fStartTimeDefaultSmall;
      double fMaxBinStartDiff;
      double fMaxBinStartMult;
      double fMaxBinStartAdd;
      double fMaxBinAdd;
      double fMidpointDefault;
      double fFallFuncEnd;
      double fRiseFuncA0Default;
      double fRiseFuncT0Default;
      double fRiseFuncTauDefault;
      double fRiseFuncKappaDefault;
      double fRiseFuncA1Default;
      double fRiseFuncPulseHeightMult;
      double fRiseFuncStartBinDiff;
      double fRiseFuncTauMult;
      double fRiseFuncKappaMult;
      double fFallFuncAfAdd;
      double fFallFuncTf1Start;
      double fFallFuncTf2Start;
      double fFallFuncTfrStart;
      double fFallFuncStepSize1;
      double fFallFuncStepSize2;
      double fMaxTraceStartSat;
      double fMaxTraceDiffSat;
      double fPulseheightMaxSat;
      double fNumberSatBins;
            

      void RiseFirstDev(double *, double, double *,double *);
      double RiseFuncInt(double *);
      double FallFuncInt(double *);
  
      //Instances of minuit go here (rename as you feel)
      TMinuit *fMinuitFallFunc;
      TMinuit *fMinuitRiseFunc;
     
      //pulse characteristics
      double fmaxbin;
      int fsize;
      double fpulseheight;
      double frise5;
      double frms;
      double fstart;
      double fmidpt;
      
      //For checking if fit fails     
      int ff_errflg;
      int frf_errflg;
      int fpflag;
      
      double fchi2;
      double fchi2rf;
      double fchi2ff;
      
      int fndf;
      int fndfrf;
      int fndfff;
            
      vector<double> fpulse;
      
      bool fIsInitialized; //check for fitter initialization
      bool fIsSmall;
      bool fIsMedium;
      bool fIsLarge;

      //values from fits
      double fRiseFuncVal[5];
      double fRiseFuncSig[5];
      double fFallFuncVal[4];
      double fFallFuncSig[4];
      
};

#endif /* PipeFitPhonon_H */
