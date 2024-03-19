/////////////////////////////////////////////////////////////////////// 
///////////////////////////////
//Class Name: ConstFreqRTFTWalkPhonon
//Authors: L. Hsu
//Description:  RTFT(RiseTimeFallTime)Walk algrorithm from darkpipe for phonon 
//pulses.  Uses a single filtering frequency for all pulses on a given detector
//Original darkpipe authors are R. Schnee and R.W. Ogburn
//
//File Import By: L. Hsu
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
//Feb. 2011 - RQ root names change from P*r% to P*WKr%. [LLH]
//
//////////////////////////////////////////////////////////////////////// 
///////////////////////////////

#ifndef ConstFreqRTFTWalkPhonon_H
#define ConstFreqRTFTWalkPhonon_H

#include <iostream>
#include <vector>
#include <map>

#include "TCDMSAnalysis.h"

using namespace std;

//!This is the ConstFreqRTFTWalkPhonon Class.  It is originally from DarkPipe
class ConstFreqRTFTWalkPhonon : public TCDMSAnalysis
{
   public:

      //do not modify the signature of this constructor
      //if you need a constructor with a different signature, than define a separate constructor
      ConstFreqRTFTWalkPhonon();  
      ~ConstFreqRTFTWalkPhonon(); //destructor 

      //do the calculations
      void DoCalc(const vector<double>& aPulse, const string& pulseType);

      //define public functions here
      void SetPeakWindowMin(const int& min)        { fPeakWindowMin = min;  fPeakWindowMinInitialized = true; return; }
      void SetPeakWindowMax(const int& max)        { fPeakWindowMax = max;  fPeakWindowMaxInitialized = true; return; }
      void SetSampleRate(const double& sampleRate) { fSampleRate = sampleRate; fSampleRateInitialized = true; return; }
      void SetSensorType(const string& sensorType) { fSensorType = sensorType; fSensorTypeInitialized = true; return; }
      void SetFilterParameters(const double& cutoff, const int& order) 
      { fFilterCutoff = cutoff;
	fFilterOrder = order;
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

      double fFilterCutoff;
      int    fFilterOrder;

      //results
      vector<int>      fPercentages; //calculation points for RTFT walk - set in constructor
      map<int, double> fRiseTimes;   //key is percent, val is the time
      map<int, double> fFallTimes;   //key is percent, val is the time

};

#endif /* ConstFreqRTFTWalkPhonon_H */
