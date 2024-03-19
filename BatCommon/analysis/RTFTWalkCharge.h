/////////////////////////////////////////////////////////////////////// 
///////////////////////////////
//Class Name: RTFTWalkCharge
//Authors: L. Hsu
//Description:  RTFT(RiseTimeFallTime)Walk algrorithm from darkpipe for charge 
//pulses.
//Original darkpipe authors are R. Schnee and R.W. Ogburn
//
//File Import By: L. Hsu
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
//////////////////////////////////////////////////////////////////////// 
///////////////////////////////

#ifndef RTFTWalkCharge_H
#define RTFTWalkCharge_H

#include <iostream>
#include <vector>
#include <map>

#include "TCDMSAnalysis.h"

using namespace std;

//!This is the RTFTWalkCharge Class.  It is originally from DarkPipe
class RTFTWalkCharge : public TCDMSAnalysis
{
   public:

      //do not modify the signature of this constructor
      //if you need a constructor with a different signature, than define a separate constructor
      RTFTWalkCharge();  
      ~RTFTWalkCharge(); //destructor 

      //do the calculations
      void DoCalc(const vector<double>& aPulse, const string& pulseType);

      //define public functions here
      void SetPeakWindowMin(const int& min)        { fPeakWindowMin = min;  fPeakWindowMinInitialized = true; return; }
      void SetSampleRate(const double& sampleRate) { fSampleRate = sampleRate; fSampleRateInitialized = true; return; }
      void SetSensorType(const string& sensorType) { fSensorType = sensorType; fSensorTypeInitialized = true; return; }
      void SetStd(const double& std)               { fStd = std; return; }
      void SetFilterParameters(const double& cutoff, const int& order) 
      { fFilterCutoff = cutoff;
	fFilterOrder = order;
	return; }

      //Get parameters
      map<int, double> GetRiseTimes() { return fRiseTimes; }
      
   private:

      void ConstructRQList();

      //define private functions and data members here
      
      //paramters needed for DoCalc
      int    fPeakWindowMin;         //range for pulse max restricted to be beyond this point
      bool   fPeakWindowMinInitialized;

      double fSampleRate;
      bool   fSampleRateInitialized;

      string fSensorType;
      bool   fSensorTypeInitialized;

      double fFilterCutoff;
      int    fFilterOrder;

      double fStd;

      //results
      vector<int>      fPercentages; //calculation points for RTFT walk - set in constructor
      map<int, double> fRiseTimes;   //key is percent, val is the time
      double fMaxAmp;
};

#endif /* RTFTWalkCharge_H */
