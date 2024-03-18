// Class:  TestAnalysis Class
// Author: LLH 

#ifndef TESTANALYSIS_H
#define TESTANALYSIS_H

#include <iostream>
#include <vector>
#include <map>

#include "TCDMSAnalysis.h"

using namespace std;

//!This is the TestAnalysis Class.  It serves as a template for all other analysis classes
class TestAnalysis : public TCDMSAnalysis
{
   public:

      //do not modify the signature of this constructor
      //if you need a constructor with a different signature, than define a separate constructor
      TestAnalysis();  
      ~TestAnalysis(); //destructor 

      //Set parameters
      //void InitializeParameters(); //optional function, do it here and not in the constructor

      //do the calculations
      void DoCalc(const vector<double>& aPulse);

      //define public functions here

   private:

      void ConstructRQList();

      //define private functions and data members here
      

};

#endif /* TESTANALYSIS_H */
