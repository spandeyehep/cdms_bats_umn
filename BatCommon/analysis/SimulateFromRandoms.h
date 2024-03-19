// Class:  SimulateFromRandoms Class
// Author: LLH 

#ifndef SimulateFromRandoms_H
#define SimulateFromRandoms_H

#include <iostream>
#include <vector>
#include <map>

#include "TCDMSAnalysis.h"

using namespace std;

//!This is the SimulateFromRandoms Class.  It serves as a template for all other analysis classes
class SimulateFromRandoms : public TCDMSAnalysis
{
   public:

      //do not modify the signature of this constructor
      //if you need a constructor with a different signature, than define a separate constructor
      SimulateFromRandoms();  
      ~SimulateFromRandoms(); //destructor 

      //Set parameters
      //void InitializeParameters(); //optional function, do it here and not in the constructor

      //do the calculations (no doCalc in this class, instead there are different "generators"
      //which choose different energy distributions to simulate
      //min and max E in keV
      void SimPTwFlatSpec(vector<double>& aPulse, double minE, double maxE, 
			  double ptcal, const int seed=0); //dumb example
      void SimPMonoenergetic(vector<double>& aPulse, double pulseE, 
			     double ptcal);
      void SimQMonoenergetic(vector<double>& aPulse, map<string, double>& pulseE, 
			     double qcal, string chanName);
      void SimPwDataTemplate(vector<double>& noisePulse, vector<double>& templatePulse,
			     double pulseE, double ptcal, map<string, double> dataAmp,
			     string chanName, int libNum, double evnum, double sernum, 
			     double DMCAvgX, double DMCAvgY, double DMCAvgZ, double DMCRecoilEnergy);

      //define public functions here
      void LoadPTemplate(const vector<double>& aTemplate)  { fPTemplate = aTemplate; return; }
      void LoadQTemplates(const vector<double> aQTemplate, const vector<double> aQTemplateX);

   private:

      void ConstructRQList();

      //define private functions and data members here
      
      //pulse templates
      vector<double> fPTemplate;
      vector<double> fQTemplate;
      vector<double> fQTemplateX;
};

#endif /* SimulateFromRandoms_H */
