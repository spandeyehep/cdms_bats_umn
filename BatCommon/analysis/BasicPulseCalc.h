///////////////////////////////////////////////////////////////////////////////// 
//Class Name: BasicPulseCalc
//Authors: L. Hsu
//Description: This class performs some basic calculations on single pulses 
// (phonon or charge: baseline subtraction, normalization, calculate std 
//  baseline and pulse maximum
//
//File Import By: L. Hsu
//Creation Date: Nov. 17, 2008
//
//Modifications:
//  Nov 2017: N. Mast: Add option to use sloped baseline subtraction: SetSlopedBaselineSub
////////////////////////////////////////////////////////////////////////////////// 


#ifndef BasicPulseCalc_H
#define BasicPulseCalc_H

#include <iostream>
#include <vector>
#include <map>

#include "TCDMSAnalysis.h"

using namespace std;

//!This is the BasicPulseCalc Class.  It does: RMS, baseline, baseline-subtraction, peak height and saturation check on a pulse
class BasicPulseCalc : public TCDMSAnalysis
{
   public:

      //do not modify the signature of this constructor
      //if you need a constructor with a different signature, than define a separate constructor
      BasicPulseCalc(const string& sensorType);  
      ~BasicPulseCalc(); //destructor 

      //Set parameters
      void InitializeParameters(); //do it here and not in the constructor
      void SetSaturationVal(const int& satVal)         { fSatVal = satVal; return; }
      void SetMinADCVal(const int& minVal)             { fMinVal = minVal; return; }
      void SetGain(const double& gain)                 { fGain = gain; return; }
      void SetBias(const double& bias)                 { fBias = bias; return; }
      void SetBiasTime(const double& biastime)         { fBiasTime = biastime; return; }

      //Will be used for both pre-baseline calc
      void SetBaselineRange(const int& baselineMin, const int& baselineMax)   
      {  fBaselineMin = baselineMin; 
	 fBaselineMax = baselineMax;
	 return; }

      //Number of bins from end of trace for post-baseline calculation
      void SetPostBaselineRange(const int& postBaselineRange)
      {  fPostBaselineRange = postBaselineRange;
	 return; }

      void SetSlopedBaselineSub(const bool& doSlopedBsSub) {fDoSlopedBsSub=doSlopedBsSub; return;}

      void SetSensorType(const string& sensorType) { fSensorType = sensorType; return; }
      void SetPulseNorm(const double& norm)          { fPulseNorm = norm; return; }

      //do the calculations
      void DoCalc(const vector<double>& aPulse);

      //define public functions here
      double GetStd()              { return fStd; }
      double GetMeanBaseline()     { return fBaseline; }
      double GetMeanPostBaseline() { return fBaselinePost; }
      bool   IsSaturated()         { return fIsSat; }
      double GetMaximum()          { return fMaximum; }
      vector<double> GetBaselineSubPulse()     { return fBaselineSubPulse; }
      vector<double> GetBaselineSubNormPulse() { return fBaselineSubNormPulse; }  //if ISR not read, norm = 1

   private:

      void ConstructRQList();

      //define private functions and data members here
     
      double fSatVal;   //saturation point for pulse
      double fMinVal;   //minimum point for pulse
      double fGain;     //from DetectorConfigManager, no calc here
      double fBias;     //from DetectorConfigManager, no calc here
      double fBiasTime; //from DetectorConfigManager, no calc here
      double fPulseNorm;  //from DetectorConfigManager, no calc here

      int fBaselineMin; //min ADC value for calculating baseline mean, rms and subtraction
      int fBaselineMax; //max ADC value for calculating baseline mean, rms and subtraction
      int fPostBaselineRange; //number of bins from end of trace for post baseline calc
      bool fDoSlopedBsSub;  //whether to subtract a sloped baseline or just a flat one

      string fSensorType; //must be "charge" or "phonon" - initialized in constructor

      //results of this calculation
      double fBaseline;
      double fBaselinePost; //baseline at end of trace
      double fStd;
      bool   fIsSat;
      int    fNSat; //number of saturated bins
      double fMaximum; 
      vector<double> fBaselineSubPulse; //baseline subtracted pulse
      vector<double> fBaselineSubNormPulse; //baseline subtracted AND normalized pulse (if ISR is not read, norm = 1)

};

#endif /* BasicPulseCalc_H */
