///////////////////////////////////////////////////////////////////////////////// 
//Class Name: OptimalFilterCharge
//Author:  L. Hsu
//Description: This class perfoms an optimal filtering using noise fft and signal fft
//  on charge pulses, taking into account cross-talks  (based on the DarkPipe code)
// 
// Original author of the darkpipe code:  R. Schnee, w/ modifications by J.A. Cooley
//                                        and R.W. Ogburn
//
//File Import By: L. Hsu
//Creation Date: Nov. 17, 2008
//
//Modifications:
//Feb. 2009 - Initial import.  This first import behaves identially to old DP implementation
//Feb. 2010 - Added full chisq minimization calculation and the ability to choose
//            between the chisq minimization (slow) or the old amplitude max (fast)
//            routines, based on a threshold that is implemented through the config
//            interface (LLH).
//Feb. 2010 - Added delay and chisq interpolation based on simple parabolic fit to 
//            delay that yields minimum chisq and the two bins on either side of it.
//            This routine is only performed for the case of full chisq minimization.
//            There is also a separate flag to toggle the interpolation on and off.
//            Delays that are at the edge of the allowed window are not interpolated (LLH).
//
////////////////////////////////////////////////////////////////////////////////// 


#ifndef OptimalFilterChargeX_H
#define OptimalFilterChargeX_H

#include <iostream>
#include <map>
#include <vector>

#include "TComplex.h"

#include "TCDMSAnalysis.h"

using namespace std;

//!This is the OptimalFilterChargeX Class.  It does the correlated optimal filter for the QI and QO pulses
class OptimalFilterChargeX : public TCDMSAnalysis
{
   public:

      //do not modify the signature of this constructor
      //if you need a constructor with a different signature, than define a separate constructor
      OptimalFilterChargeX();  
      ~OptimalFilterChargeX(); //destructor 


      //do the calculations
      void DoCalc(const vector<double>& aPulseQI, const vector<double>& aPulseQO);

      //define public functions here
      void StoreAs(const string& chanType);

      //Get functions
      double GetQOvolts()  const { return fQOvolts;  }
      double GetQOvolts0() const { return fQOvolts0; }

      double GetQIvolts()  const { return fQIvolts;  }
      double GetQIvolts0() const { return fQIvolts0; }

      double GetDelay() const { return fDelay; }
      double GetChisq() const { return fChisq; }

      //Set parameters
      void SetSampleTime(double dt) { fdT = dt; return; }
      void SetQxWindows(int qxwindow1, int qxwindow2) 
      {  fQxwindow1 = qxwindow1; 
	 fQxwindow2 = qxwindow2; 
	 return; }
      void SetBiasVoltages(const double& biasQI, const double& biasQO) 
      {  fBiasQI = biasQI;
	 fBiasQO = biasQO;
	 fBiasVoltagesSet = true;
	 return; }

      //for loading in through extData manager
      void LoadTemplates(const vector<TComplex>& pulseTemplateFFT, const vector<TComplex>& optimalFilter, const string& channelName);

      void LoadNormalizations(const double& normFFT, const double& sigToNoiseSq, const vector<double>& noiseFFTsq, const double& templateMax, 
			      const string& channelName);

      void LoadQInverse(const vector<double>& qInverse);

      void LoadChisqThresholds(const double& minQI, const double& minQO)
      {
	fMinQI = minQI;
	fMinQO = minQO;
	return;
      }

      void IsRandom(const bool& isRandom)
      {
	fIsRandom = (isRandom ? 1 : 0);
	return;
      }

      void SetDelayInterpolateFlag(const int& doDelayInt)
      {
	fDoDelayInterpolation = doDelayInt;
	return;
      }

      //for special OFX studies
      void DelayResCalc(const vector<double>& aNoisePulseQI, const vector<double>& aNoisePulseQO, int detNum);
      
   private:

      void ConstructRQList();
      void CalcDelayInterpolation(const int delay, const double chisq);

      //constant values go here

      //define private functions and data members here
      
      //fit values
      double fQOvolts;
      double fQOvolts0;
      double fQIvolts;
      double fQIvolts0;

      double fDelay;
      double fChisq;
      double fInterDelay; //interpolated delay 
      double fInterChisq; //interpolated chisq
      double fDiscreteDelay; //original delay
      double fDiscreteChisq; //original chisq

      map<int, double> fDelayChisqMap; //key is delay, val is chisq

      //parameters
      double fdT;
      int    fQxwindow1; //search for best delay from 0th bin to this point ...
      int    fQxwindow2; //...or from this point to the last bin      
      double fBiasQI;
      double fBiasQO;
      double fMinQI;   //threshold for performing full chisq minimization
      double fMinQO;   //threshold for performaing full chisq minimization
      bool   fDoFullChisqMin; //this is set internally

      //set by the user, -999999 if not initialized
      int    fIsRandom;
      int    fDoDelayInterpolation; 

      bool   fTemplatesLoaded;
      bool   fNormalizationsLoaded;
      bool   fMatrixLoaded;
      bool   fBiasVoltagesSet;

      //templates and normalizations
      vector<double> fQInverse;
      
      double fNormFFTQI;
      double fNormFFTQO;

      double fSigToNoiseSqQI;
      double fSigToNoiseSqQO;

      double fTemplateMaxQI;
      double fTemplateMaxQO;

      //noise fft sq 
      vector<double> fNoiseFFTSqQI;
      vector<double> fNoiseFFTSqQO;
      vector<double> fNoiseFFTSqQIX; //not needed
      vector<double> fNoiseFFTSqQOX; //not needed

      //noise templates
      vector<TComplex> fPulseTemplateFFTQI;
      vector<TComplex> fPulseTemplateFFTQO;
      vector<TComplex> fPulseTemplateFFTQIX;
      vector<TComplex> fPulseTemplateFFTQOX;

      //OptimalFilter templates
      vector<TComplex> fOptimalFilterQI; 
      vector<TComplex> fOptimalFilterQO; 
      vector<TComplex> fOptimalFilterQIX; 
      vector<TComplex> fOptimalFilterQOX; 

      //precaution against reading in the wrong templates
      int fNBinsTemplates;  

      // ==== only for debugging ======

      //pulse templates
      vector<double> fPulseTemplateQI;
      vector<double> fPulseTemplateQO;
      vector<double> fPulseTemplateQIX;
      vector<double> fPulseTemplateQOX;

      void ConstructFakePulse(double normQI, double normQO, double delay, int detNum,
			      const vector<double>& aNoisePulseQI, const vector<double>& aNoisePulseQO);
      vector<double> fFakePulseQI;
      vector<double> fFakePulseQO;

};

#endif /* OptimalFilterChargeX_H */
