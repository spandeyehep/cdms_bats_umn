////////////////////////////////////////////////////////////////////////
// Class:  PSDIntegralPhonon Class
// Author: Jianjie Zhang
// Description: This class calculates the power of phonon signals in frequency bands
// 4-60, 10-20, 20-30, 30-50, 50-70, and 70-100kHz.
//
// File imported by: Jianjie Zhang
// Creation date: Oct. 19, 2010
// Modifications:
//
////////////////////////////////////////////////////////////////////////

#ifndef PSDIntegralPhonon_H
#define PSDIntegralPhonon_H

#include <iostream>
#include <vector>
#include <map>

#include "TCDMSAnalysis.h"

using namespace std;

//!This is the PSDIntegralPhonon Class.  It serves as a template for all other analysis classes
class PSDIntegralPhonon : public TCDMSAnalysis
{
   public:

      //do not modify the signature of this constructor
      //if you need a constructor with a different signature, than define a separate constructor
      PSDIntegralPhonon();  
      ~PSDIntegralPhonon(); //destructor 

      //Set parameters
      //void InitializeParameters(); //optional function, do it here and not in the constructor

      //do the calculations
      void DoCalc(const vector<double>& aPulse, double sampleRate);

      //define public functions here

   private:

      void ConstructRQList();

      //define private functions and data members here
      

};

#endif /* PSDIntegralPhonon_H */
