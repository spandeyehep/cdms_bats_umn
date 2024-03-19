///////////////////////////////////////////////////////////////////////////////// 
//Class Name: WedgeFitPhonon
//Authors: Oleg Kamaev (ported by Bruno Serfass into BatRoot code)
//Description: This class performs a 3-parameter fit on phonon pulses (below 20% rising edge amplitude)
// using a polynomial functional form 
//
//
//File Import By: B. Serfass
//Creation Date: Jan. 21, 2009
//
//Modifications:
//
////////////////////////////////////////////////////////////////////////////////// 

#ifndef WedgeFitPhonon_H
#define WedgeFitPhonon_H

#include <iostream>
#include <map>
#include <vector>

#include "TMinuit.h"
#include "TString.h"
#include "TMath.h"

#include "TCDMSAnalysis.h"

using namespace std;

class WedgeFitPhonon : public TCDMSAnalysis
{
   public:

      //do not modify the signature of this constructor
      //if you need a constructor with a different signature, than define a separate constructor
      WedgeFitPhonon();  
      ~WedgeFitPhonon(); //destructor 

     // === Get Paramters ===
      double GetPar1FitVal(){ return fParFitVal[0];};
      double GetPar2FitVal(){ return fParFitVal[1];};
      double GetPar3FitVal(){ return fParFitVal[2];}; 
      double GetPar1FitSig(){ return fParFitSig[0];};
      double GetPar2FitSig(){ return fParFitSig[1];};
      double GetPar3FitSig(){ return fParFitSig[2];}; 
      double GetFitChi2(){ return fChi2;};  
      int GetNdof(){ return fNdof;};  
      int GetFitErrFlag(){ return fFitErrFlag;}; 


      // === Set parameters === 
      void SetFitParameters(const vector<double>& aPulse,double sampleRate,int butterOrder,double butterCutoff, double rmsADC,double maxADC,int time10,int time20,int time60,int binSStart,int binAEnd,int binSt0,double par1Start);

      // == do the calculations ===
      void DoCalc(const vector<double>& aPulse); 

     

   private:

      void ConstructRQList();

      //Instances of minuit 
      TMinuit *fMyMinuitInstance;

      //constant values go here
      static const int kNumPars = 3; 
      static const int kMaxMIGRADCalls = 4000;
      static const int kStrategy = 2;
      static const int kNormFitStep = 1000;

      //private functions and data members
      double fRMS;
      double fChi2; 
      double fParStartVal[kNumPars];
      double fParFitVal[kNumPars];
      double fParFitSig[kNumPars]; 
      int fStartPt;
      int fEndPt;
      int fFitErrFlag;
      int fNdof; 
};

#endif /* WedgeFitPhonon_H */
