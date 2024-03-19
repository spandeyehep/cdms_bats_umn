/////////////////////////////////////////////////////////////////////// 
///////////////////////////////
//Class Name: VarFreqRTFTWalkPhonon
//Authors: L. Hsu
//
//Description:  Variable Frequency RTFT(RiseTimeFallTime)Walk algrorithm 
//              for phonon pulses.
//              Original darkpipe authors are R. Schnee and R.W. Ogburn
//
//File Import By: L. Hsu
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
//Jan. 2009 - This was made into the variable frequency walk by Scott Hertel
//Feb. 2011 - Renaming the rq's of the class to VWKr* to distiguish
//            from the ConstFreqRTFTWalk rq's, which are WK [LLH]
//Feb. 2011 - Renaming this class from RTFTWalkPhonon to 
//            VarFreqRTFTWalkPhonon. RQ root names change from P*r%
//            to P*VWKr%. [LLH]
//
//////////////////////////////////////////////////////////////////////// 
///////////////////////////////

#ifndef VARFREQRTFTWALPHONON_H
#define VARFREQRTFTWALPHONON_H

#include <iostream>
#include <vector>
#include <map>

#include "TCDMSAnalysis.h"

using namespace std;

//!This is the VarFreqRTFTWalkPhonon Class.  It is originally from DarkPipe
class VarFreqRTFTWalkPhonon : public TCDMSAnalysis
{
   public:

      //do not modify the signature of this constructor
      //if you need a constructor with a different signature, than define a separate constructor
      VarFreqRTFTWalkPhonon();  
      ~VarFreqRTFTWalkPhonon(); //destructor 

      //do the calculations
      void DoCalc(const vector<double>& aPulse);

      //define public functions here
      void SetPeakWindowMin(const int& min)        { fPeakWindowMin = min;  fPeakWindowMinInitialized = true; return; }
      void SetPeakWindowMax(const int& max)        { fPeakWindowMax = max;  fPeakWindowMaxInitialized = true; return; }
      void SetSampleRate(const double& sampleRate) { fSampleRate = sampleRate; fSampleRateInitialized = true; return; }
      void SetSensorType(const string& sensorType) { fSensorType = sensorType; fSensorTypeInitialized = true; return; }
 
      //filter cutoff varies with signal to noise
      void SetVariableFilterParameters(int& channum,
				       vector<double> cutoff1,
				       vector<double> cutoff2,
				       int order,
				       double lowerLimit,
				       double upperLimit,
				       double scalingFactor,
				       double pulseSTD,
				       double pulseOFeV,
				       double templateMax,
	                               double defaultCutoff);

      //single cutoff frequency for all sizes of pulse
      void SetFixedFilterParameters(const double& cutoff, const int& order) 
      { fFilterCutoff = cutoff;
	fFilterOrder = order;
	fDoFilter = true;
	return; }

      //Get parameters
      map<int, double> GetRiseTimes() { return fRiseTimes; }
      map<int, double> GetFallTimes() { return fFallTimes; }
      
   private:

      void ConstructRQList();

      //define private functions and data members here
      
      //paramters needed for DoCalc
      int    fPeakWindowMin;         //range for pulse max
      int    fPeakWindowMax;         //range for pulse max
      bool   fPeakWindowMinInitialized;
      bool   fPeakWindowMaxInitialized;

      double fSampleRate;
      bool   fSampleRateInitialized;

      string fSensorType;
      bool   fSensorTypeInitialized;

      //for filtering
      double fCutoffParameter1;
      double fCutoffParameter2;
      int    fFilterOrder;
      double fFilterCutoff;
      bool   fDoFilter;        //filtering is off by default until filter parameter functions are called

      //results
      vector<int>      fPercentages; //calculation points for RTFT walk - set in constructor
      map<int, double> fRiseTimes;   //key is percent, val is the time
      map<int, double> fFallTimes;   //key is percent, val is the time
      double fMaxAmp;
};

#endif /* VARFREQRTFTWALPHONON_H */
