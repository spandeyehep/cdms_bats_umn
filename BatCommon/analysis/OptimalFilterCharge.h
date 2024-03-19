///////////////////////////////////////////////////////////////////////////////// 
//Class Name: OptimalFilterCharge
//Author:  L. Hsu
//Description: This class perfoms an optimal filtering using noise fft and signal fft
//  on charge pulses.  It does *not* perform the cross talk calculation.  This code was
//  written for endcap charge channels
// 
// Original author of the darkpipe code:  R. Schnee
//
//File Import By: L. Hsu
//Creation Date: Jun. 6, 2009
//
//Modifications:
//
////////////////////////////////////////////////////////////////////////////////// 


#ifndef OptimalFilterCharge_H
#define OptimalFilterCharge_H

#include <iostream>
#include <map>
#include <vector>

#include "TComplex.h"

#include "TCDMSAnalysis.h"

using namespace std;

//!This is the OptimalFilterCharge Class.  It does the uncorrelated optimal filter for the Q pulses
class OptimalFilterCharge : public TCDMSAnalysis
{
   public:

      //do not modify the signature of this constructor
      //if you need a constructor with a different signature, than define a separate constructor
      OptimalFilterCharge();  
      ~OptimalFilterCharge(); //destructor 


      //do the calculations
      void DoCalc(const vector<double>& aPulse);

      //Get functions
      double GetVolts()  const { return fVolts;  }
      double GetVolts0() const { return fVolts0; }
      double GetDelay() const { return fDelay; }
      double GetChisq() const { return fChisq; }

      //Set parameters
      void SetSampleTime(double dt) { fdT = dt; return; }
      void SetWindows(int window1, int window2) 
      {  fwindow1 = window1; 
	 fwindow2 = window2; 
	 return; }
      void SetBiasVoltages(const double& bias) 
      {  fBias = bias;
	 fBiasVoltagesSet = true;
	 return; }

      //for loading in through extData manager
      void LoadTemplates(const vector<TComplex>& pulseTemplateFFT, const vector<TComplex>& optimalFilter);

      void LoadNormalizations(const double& normFFT, const double& sigToNoiseSq, const vector<double>& noiseFFTsq, const double& templateMax);

      
   private:

      void ConstructRQList();

      //constant values go here

      //define private functions and data members here
      
      //fit values
      double fVolts;
      double fVolts0;
      double fDelay;
      double fChisq;

      //parameters
      double fdT;
      int    fwindow1; //search for best delay from 0th bin to this point ...
      int    fwindow2; //...or from this point to the last bin      
      double fBias;

      bool   fTemplatesLoaded;
      bool   fNormalizationsLoaded;
      bool   fBiasVoltagesSet;

      //templates and normalizations
      double fNormFFT;
      double fSigToNoiseSq;
      double fTemplateMax;

      //noise fft sq 
      vector<double> fNoiseFFTSq;

      //noise templates
      vector<TComplex> fPulseTemplateFFT;

      //OptimalFilter templates
      vector<TComplex> fOptimalFilter; 

      //precaution against reading in the wrong templates
      int fNBinsTemplates;  

      // ==== only for debugging ======

      //pulse templates
      vector<double> fPulseTemplate;

};

#endif /* OptimalFilterCharge_H */
