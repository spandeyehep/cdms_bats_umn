///////////////////////////////////////////////////////////////////////////////// 
//Class Name: F5ChargeX
//Authors: M. Kos
//Description: This class performs 2-parameter fit with delay ("F5") for charge channels.
// This fit should give good results even for sturating pulses.
//
// Original author of the darkpipe code:  R. Schnee
//
//File Import By: M. Kos
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
////////////////////////////////////////////////////////////////////////////////// 

#ifndef F5ChargeX_H
#define F5ChargeX_H

#include <iostream>
#include <map>
#include <vector>

#include "TMinuit.h"
#include "TString.h"
#include "TMath.h"

#include "TCDMSAnalysis.h"

using namespace std;

//!This is the F5ChargeX Class.  It does the correlated 4-parameter time domain fit for QI baseline, QI amplitude, QO baseline and QO amplitude.
class F5ChargeX : public TCDMSAnalysis
{
   public:

      //do not modify the signature of this constructor
      //if you need a constructor with a different signature, than define a separate constructor
      F5ChargeX();  
      ~F5ChargeX(); //destructor 

      //do the calculations
      void DoCalc(const vector<double> &rawPulseQI, const vector<double> &rawPulseQO); 

      //define public functions here
      void StoreAs(const string &chanType);

      //Get functions
      double GetQIvolts()  const { return fQIVolts;  }
      double GetQOvolts()  const { return fQOVolts;  }

      double GetQIBase() const { return fQIBase; }
      double GetQOBase() const { return fQOBase; }

      double GetQIChisq() const { return fQIChisq; }
      double GetQOChisq() const { return fQOChisq; }

      //Functions required for F5 fit
      void LUDecomp();
      vector<double> LUBackSub(double **LUmatrix, int dim, const vector<double>& indx, const vector<double>& sol_vector);
      vector<double> ImproveFit(double **MNSMatrix, double **LUmatrix,int dim,const vector<double>& indx,  const vector<double>& rhsVector, const vector<double>& SolVector);
      //void LUIndex(); Probably not needed
      void ChisqFitX();
      void InitChisqFit(const vector<double>& templateQI, const vector<double>& templateQIX, const vector<double>& templateQO,  const vector<double>& templateQOX);

      //Set parameters and templates
      void SetSampleRate(double sampleRate) { fSampleRate = sampleRate; return; }
      void SetOFDelay(double OFDelay = 512.) {fOFDelay = OFDelay; return; }
      void SetTemplateStart(int tempStart = 512) {fTemplateStart = tempStart; return; }
      void SetQIStd(double qiStd) {fSTDQI = qiStd; return; }
      void SetQOStd(double qoStd) {fSTDQO = qoStd; return; }
      void SetQIBaseline(double qiBaseline) {fBSQI = qiBaseline; return; }
      void SetQOBaseline(double qoBaseline) {fBSQO = qoBaseline; return; }
      void SetSaturationValue(double satValue = 4091.) {fSatValue = satValue; return; }
      void SetTemplateLowBin(int templowbin) {fTemplateLowBin = templowbin; return; }
      void SetTemplateHiBin(int temphibin) {fTemplateHiBin = temphibin; return; }
      void SetNoiseQI(double noiseQI = 1.0e-7) {fnoiseQI = noiseQI*noiseQI; return;}
      void SetNoiseQO(double noiseQO = 1.0e-7) {fnoiseQO = noiseQO*noiseQO; return;}
      void SetGainQI(double gainQI) {fScaleQI = gainQI; return;}
      void SetGainQO(double gainQO) {fScaleQO = gainQO; return;}
      void SetGoodQStart(int goodQStr) {fGoodQStart = goodQStr; return;}
      void SetGoodQEnd(int goodQEnd) {fGoodQEnd = goodQEnd; return;}
      void LoadTemplates(const vector<double>& templateQI, const vector<double>& templateQIX, const vector<double>& templateQO, const vector<double>& templateQOX,
			 const double& templateMaxQI, const double& templateMaxQO);

      // ==== only for debugging ======

      void ConstructFakePulse(double normQI, double normQO, int delay);
      vector<double> fFakePulseQI;
      vector<double> fFakePulseQO;

   private:

      void ConstructRQList();

      //Instances of minuit go here (rename as you feel)
      TMinuit *fMyMinuitInstance;

      //define private functions and data members here

      //Fit values
      double fQIVolts;
      double fQIChisq;
      double fQIBase;
      double fQIAmpl; //amplitude

      double fQOVolts;
      double fQOChisq;
      double fQOBase;
      double fQOAmpl; //amplitude
      
      double fResSumQI;//sum of the residual for both channels
      double fResSumQO;
   
      vector<double> fSolutionVector;

      //parameters
      double fSampleRate;
      double fOFDelay; //this may or may not be set depending on whether pulses are saturated
      double fSatDelay;
      double fSTDQI;
      double fSTDQO;
      double fBSQI;
      double fBSQO; 
      double fSatValue;
      double fd; //used by LUdecomp
      double fScaleQI; //scale to volts
      double fScaleQO; //scale to volts
      double fnoiseQI;
      double fnoiseQO;
   

      //templates
      vector<double> fTemplateQI;
      vector<double> fTemplateQO;
      vector<double> fTemplateQIX;
      vector<double> fTemplateQOX;

      //baseline un-subtracted 
      vector<double> fPulseQI;
      vector<double> fPulseQO;

      //internal vectors
      vector<double> findx;
    
      //matrices for LUDecomp
      double **fpMns2; //the 2 is for 2D
      double **fpMnsLU2;

      //pulses to be fitted
      vector<double> fQIFit;
      vector<double> fQOFit;

      double fTemplateMaxQI;
      double fTemplateMaxQO;

      bool fTemplatesLoaded;

      //precaution against reading in the wrong templates
      int fNBinsTemplates;  
      int fNBinsTemplatesQO;
      int fNBinsTemplatesQIX;
      int fNBinsTemplatesQOX;

      //template start position, defined from 5*std above basline
      int fTemplateStart;
      //template window
      int fGoodQStart;
      int fGoodQEnd;
      int fTemplateLowBin;
      int fTemplateHiBin;
 
};

#endif /* F5ChargeX_H */
