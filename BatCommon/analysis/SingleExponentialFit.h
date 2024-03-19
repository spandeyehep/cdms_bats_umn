///////////////////////////////////////////////////////////////////////////////// 
//Class Name: SingleExponentialFit
//Authors:
//
//
//File Import By: 
//Creation Date: 
//
//Modifications:
//
////////////////////////////////////////////////////////////////////////////////// 

#ifndef SingleExponentialFit_H
#define SingleExponentialFit_H

#include <iostream>
#include <map>
#include <vector>

#include "TMinuit.h"
#include "TString.h"
#include "TMath.h"

#include "TCDMSAnalysis.h"

using namespace std;

class SingleExponentialFit : public TCDMSAnalysis
{
   public:

      //do not modify the signature of this constructor
      //if you need a constructor with a different signature, than define a separate constructor
      SingleExponentialFit(const string& className = "SingleExponentialFit");  
      ~SingleExponentialFit(); //destructor 

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
      void InitializeFitParameters(double par1Init, double par2Init, double par3Init, double RMS);
      void SetSampleTime(double dt) { fdT = dt; return; }
      void SetFitWindow(int startBin, int stopBin)
       {  fStartPt = startBin;
          fEndPt =  stopBin;
          return; }
   
     void SetFitParameterConstraintFlag(int parNumber, int flag) {fParConstraintFlag[parNumber-1] = flag; return;}
     void SetFitParameterRange(int parNumber, double parMin, double parMax) 
        { fParMin[parNumber-1] = parMin;
	  fParMax[parNumber-1] = parMax;
          return; }


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
      double fdT;
      double fRMS;
      double fChi2; 
      double fParStartVal[kNumPars];
      double fParMin[kNumPars];
      double fParMax[kNumPars];
      double fParFitVal[kNumPars];
      double fParFitSig[kNumPars]; 
      int fParConstraintFlag[kNumPars]; 
      int fStartPt;
      int fEndPt;
      int fFitErrFlag;
      int fNdof; 
};

#endif /* SingleExponentialFit_H */
