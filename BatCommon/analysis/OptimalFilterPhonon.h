///////////////////////////////////////////////////////////////////////////////// 
//Class Name: OptimalFilterPhonon
//Author:  L. Hsu
//Description: This class perfoms an optimal filtering using noise fft and signal fft
//  on single pulse  (based on the DarkPipe code)
// 
// Original author of the darkpipe code:  R. Schnee
//
//File Import By: L. Hsu
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
////////////////////////////////////////////////////////////////////////////////// 


#ifndef OptimalFilterPhonon_H
#define OptimalFilterPhonon_H

#include <iostream>
#include <map>
#include <vector>

#include "TComplex.h"

#include "TCDMSAnalysis.h"

using namespace std;

//!This is the OptimalFilterPhonon Class.  This can be used on any single pulse, but most likely on phonon pulses
class OptimalFilterPhonon : public TCDMSAnalysis
{
   public:

      //do not modify the signature of this constructor
      //if you need a constructor with a different signature, than define a separate constructor
      OptimalFilterPhonon(const string& className =  "OptimalFilterPhonon");  
      ~OptimalFilterPhonon(); //destructor 

      //Set parameters
      //void InitializeParameters(); //do it here and not in the constructor

      //do the calculations
      void DoCalc(const vector<double>& aPulse); 

      //define public functions here
      double GeteV() const     { return fAmp; }
      double GetAmp() const     { return fAmp; }
      double GetChisq() const  { return fChisq; }
      double GetChisqlowFreq() const  { return fChisq; }
      double GetDelay() const  { return fDelay; }

      //Set parameters
      void SetSampleTime(double dt) { fdT = dt; return; }
      void SetWindows(int window1, int window2) 
      {  fwindow1 = window1; 
	 fwindow2 = window2; 
	 return; }


      //for loading in through extData manager
      void LoadTemplates(const vector<TComplex>& pulseTemplateFFT, const vector<TComplex>& optimalFilter);
      void LoadNormalizations(const double& normFFT, const double& sigToNoiseSq, const vector<double>& noiseFFTsq);
      void LoadCutoffFreq(const double& cutoffFreq);



   private:

      void ConstructRQList();

      //define private functions and data members here
      
      //fit values
      double fAmp;
      double fAmp0;
      double fChisq;
      double fChisqLF;
      double fDelay;

      //parameters
      double fdT;
      int    fwindow1; //in adc bins
      int    fwindow2; //in adc bins
      double fCutoffFreq;
      bool   fTemplatesLoaded; 
      bool   fNormalizationsLoaded;

      //templates
      double           fNormFFT;
      double           fSigToNoiseSq;
      vector<double>   fNoiseFFTSq;
      vector<TComplex> fPulseTemplateFFT;   
      vector<TComplex> fOptimalFilter; 


      //precaution against reading in the wrong templates
      int fNBinsTemplates;  

      //only for debugging
      void ConstructFakePulse(double norm, int delay);
      vector<double> fFakePulse;
      vector<double> fPulseTemplate;

};

#endif /* OptimalFilterPhonon_H */
