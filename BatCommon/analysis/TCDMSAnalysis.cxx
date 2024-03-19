/////////////////////////////////////////////////////////////////////// 
///////////////////////////////
//Class Name: TCDMSAnalysis
//Authors: L. Hsu
//Description:  The TCMDSAnalysis class is the base class that all other analysis  
//classes inherit from.
//
//File Import By: L. Hsu
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
//////////////////////////////////////////////////////////////////////// 

#include <iostream>
#include <cstdlib>

#include "TCDMSAnalysis.h"

////////////////////////////////////////////////////////

TCDMSAnalysis::TCDMSAnalysis() :
   fClassName("TCDMSAnalysis") ,
   fAnalysisInitials(""),
   fStoreRQs(false)
{
//   cout <<"Hello from TCDMSAnalysis()" << endl;
}

TCDMSAnalysis::~TCDMSAnalysis()
{
//   cout <<"Goodbye from TCDMSAnalysis()" << endl;
}

double TCDMSAnalysis::GetRQVal(const string& rqName) const
{
   double val = -999999.;

   if(fRQList.count(rqName) == 0)
   {
      cout <<"TCDMSAnalysis::GetRQVal ERROR!  Requested RQ, " << rqName
	   <<", is not found, please check your code" << endl;
      exit(1);
   }

   map<string, double>::const_iterator rqItr = fRQList.begin();
   for( ; rqItr != fRQList.end(); rqItr++)
   {
      if(rqItr->first == rqName)
	 return rqItr->second;
   }

   return val;
}
