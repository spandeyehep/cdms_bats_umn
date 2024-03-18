#include <iostream>

#include "TestAnalysis.h"
#include "PulseTools.h"

////////////////////////////////////////////////////////

//do not modify the signature of this constructor
//instead use InitializeParameters() to pass in values to your class
TestAnalysis::TestAnalysis()
{
   //   cout <<"Hello from TestAnalysis()" << endl;

   //these members along with fRQlist are inherited from TCDMSAnalysis
   fClassName = "TestAnalysis"; 
   fStoreRQs = true;

   //Construct the RQ list
   ConstructRQList();
   
   //initialization of member data can go here

}

TestAnalysis::~TestAnalysis()
{
//   cout <<"Goodbye from TestAnalysis()" << endl;
}

//This method constructs the RQ list that is handed off to BatOutputManager
//It also sets the default value for the RQ to initVal.
void TestAnalysis::ConstructRQList()
{
   double initVal = -999999.;

   //construct the RQ list here
   fRQList.insert(pair<string,double>("alpha", initVal));
   fRQList.insert(pair<string,double>("beta",  initVal));
   fRQList.insert(pair<string,double>("gamma", initVal));

   //Any RQ that is included in the above list will be written out by BatRoot.  Add to this as you please.

   return;
}

//optional function, do it here and not in the constructor
// void TestAnalysis::InitializeParameters()
// {
//
//    return;
// }

//This is the main call for your analysis
void TestAnalysis::DoCalc(const vector<double>& aPulse)
{
  //check for null pulses
  if(aPulse.size() == 0)
  {
    cerr <<"TestAnalysis::DoCalc ERROR!  Pulse passed into DoCalc() has length 0, please check that pulse is valid."
	 << endl;
    exit(1);
  }

   //do your calculation here!

   //Next, store the results of this calculation as the RQ's.
   //These values will be included in the output of BatRoot.
   if(fStoreRQs) {
      fRQList["alpha"] = 1.0;
      fRQList["beta"]  = 2.0;
      fRQList["gamma"] = 3.0;
   }

   return;
}
