// Class:  TestAnalysisMinuit Class
// Author: LLH 

#ifndef TestAnalysisMinuit_H
#define TestAnalysisMinuit_H

#include <iostream>
#include <map>
#include <vector>

#include "TMinuit.h"
#include "TString.h"
#include "TMath.h"

#include "TCDMSAnalysis.h"

using namespace std;

class TestAnalysisMinuit : public TCDMSAnalysis
{
   public:

      //do not modify the signature of this constructor
      //if you need a constructor with a different signature, than define a separate constructor
      TestAnalysisMinuit();  
      ~TestAnalysisMinuit(); //destructor 

      //Set parameters
      //void InitializeParameters(); //optional function, do it here and not in the constructor

      //do the calculations
      void DoCalc(const vector<double>& aPulse); 

      //define public functions here

   private:

      void ConstructRQList();

      //Instances of minuit go here (rename as you feel)
      TMinuit *fMyMinuitInstance;

      //constant values go here
      static const int kNumPars = 5; //follow this example (note constants start with "k")
      static const int kMaxMIGRADCalls = 4000;

      //define private functions and data members here
      

};

#endif /* TestAnalysisMinuit_H */
