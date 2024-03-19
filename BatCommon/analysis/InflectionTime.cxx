///////////////////////////////////////////////////////////////////////////////// 
//Class Name: InflectionTime
//Author:  L. Hsu
//Description: This class is the equivalent of the PipeFitter
// code that finds the inflection time of a single pulse
//
// Original author of the PipeFitter code: J. Yoo 
//
//File Import By: L. Hsu
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
////////////////////////////////////////////////////////////////////////////////// 





#include <iostream>

#include "InflectionTime.h"
#include "PulseTools.h"

////////////////////////////////////////////////////////

//do not modify the signature of this constructor
InflectionTime::InflectionTime() :
   fMinWindow(-999999),
   fMaxWindow(-999999),
   fMaxDiff(-1e5),
   fMaxDiffTime(-999999),
   fMinDiff(1e10),
   fMinDiffTime(-999999)
{
   //   cout <<"Hello from InflectionTime()" << endl;

   //these members along with fRQlist are inherited from TCDMSAnalysis
   fClassName = "InflectionTime"; 
   fStoreRQs = true;

   //Construct the RQ list
   ConstructRQList();
   
   //initialization of member data can go here

}

InflectionTime::~InflectionTime()
{
//   cout <<"Goodbye from InflectionTime()" << endl;
}

//This method constructs the RQ list that is handed off to BatOutputManager
//It also sets the default value for the RQ to initVal.
void InflectionTime::ConstructRQList()
{
   double initVal = -999999.;

   //construct the RQ list here
   fRQList.insert(pair<string,double>("imaxt", initVal));
   fRQList.insert(pair<string,double>("imint", initVal));
   fRQList.insert(pair<string,double>("imax", initVal));
   fRQList.insert(pair<string,double>("imin", initVal));

   //Any RQ that is included in the above list will be written out by BatRoot.  Add to this as you please.

   return;
}

//This is the main call for your analysis
void InflectionTime::DoCalc(const vector<double>& aPulse)
{
   //check for null pulses
   if(aPulse.size() == 0)
   {
      cerr <<"InflectionTime::DoCalc ERROR!  Pulse passed into DoCalc() has length 0, please check that pulse is valid."
	   << endl;
      exit(1);
   }
   
   //check if calculation window is properly set
   if(fMinWindow == -999999 || fMaxWindow == -999999)
   {
      cerr <<"InflectionTime::DoCalc ERROR!  MinWindow and MaxWindow are not set!"
	   << endl;
      exit(1);
   }

   if(fMinWindow < 2)
   {
      cerr <<"InflectionTime::DoCalc ERROR!  MinWindow must be > 1 to allow for smoothing algorithm to work."
	   << endl;
      exit(1);
   }

   //do the calculation!

   //calculate derivative of the pulse
   vector<double> differentialPulse = PulseTools::Differentiate(aPulse);
   
   //subtract one from fMinWindow to account for c++ array convention
   fMinWindow -= 1;

   //now find the inflection points
   for (int binCtr = fMinWindow; binCtr < fMaxWindow; binCtr++)
   { 
      //calculate the average slope around this bin
      double diffAve = ( differentialPulse[binCtr-2]
			+differentialPulse[binCtr-1]
			+differentialPulse[binCtr]
			+differentialPulse[binCtr+1]
			+differentialPulse[binCtr+2])/5.;
      
      //max inflection
      if(diffAve > fMaxDiff)   
      {
	 fMaxDiff = diffAve;
	 fMaxDiffTime = double(binCtr)-0.5;
      }    

      //min inflection
      if(diffAve < fMinDiff)   
      {
	 fMinDiff = diffAve;
         fMinDiffTime = double(binCtr)-0.5;
      }
   }

   //add 1 to go from c++ bins to "physical" bins
   fMaxDiffTime += 1;
   fMinDiffTime += 1;

   //Next, store the results of this calculation as the RQ's.
   //These values will be included in the output of BatRoot.
   if(fStoreRQs) 
   {
      fRQList["imax"] = fMaxDiff;
      fRQList["imaxt"] = fMaxDiffTime;
      fRQList["imin"] = fMinDiff;
      fRQList["imint"] = fMinDiffTime;
   }

   return;
}
