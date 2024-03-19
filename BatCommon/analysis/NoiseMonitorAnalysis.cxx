/////////////////////////////////////////////////////////////////////// 
///////////////////////////////
//Class Name: NoiseMonitorAnalysis
//Authors: B. Loer
//Description:  NoiseMonitor pulse analysis algorithm copied from veto 
//code.  
//
//File Import By: B. Loer
//Creation Date: Mar 2014
//
//Modifications:
//
//////////////////////////////////////////////////////////////////////// 
///////////////////////////////


#include <iostream>

#include "NoiseMonitorAnalysis.h"
#include "PulseTools.h"
#include "BatRootTypes.h"
////////////////////////////////////////////////////////

//do not modify the signature of this constructor
//instead use InitializeParameters() to pass in values to your class
NoiseMonitorAnalysis::NoiseMonitorAnalysis()
{
   //   cout <<"Hello from NoiseMonitorAnalysis()" << endl;

   //these members along with fRQlist are inherited from TCDMSAnalysis
   fClassName = "NoiseMonitorAnalysis"; 
   fStoreRQs = true;

   //Construct the RQ list
   ConstructRQList();
   
   //initialization of member data can go here

}

NoiseMonitorAnalysis::~NoiseMonitorAnalysis()
{
//   cout <<"Goodbye from NoiseMonitorAnalysis()" << endl;
}

//This method constructs the RQ list that is handed off to BatOutputManager
//It also sets the default value for the RQ to initVal.
void NoiseMonitorAnalysis::ConstructRQList()
{
  double initVal = BatRootTypes::kEmptyVariable;
  
  fRQList["std"] = initVal;
  fRQList["mean"] = initVal;  
}

//optional function, do it here and not in the constructor
void NoiseMonitorAnalysis::InitializeParameters()
{
}

//This is the main call for your analysis
void NoiseMonitorAnalysis::DoCalc(const vector<double>& aPulse)
{
  //check for null pulses
  if(aPulse.size() == 0)
    {
    cerr <<"NoiseMonitorAnalysis::DoCalc ERROR!  Pulse passed into DoCalc() has length 0, please check that pulse is valid."
	 << endl;
    exit(1);
  }
  if(fStoreRQs){
    fRQList["mean"] = PulseTools::Baseline(aPulse);
    fRQList["std"]  = PulseTools::Std(aPulse);
  }
  
}
