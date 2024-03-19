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
///////////////////////////////

#ifndef TCDMSANALYSIS_H
#define TCDMSANALYSIS_H

#include <iostream>
#include <map>

using namespace std;

typedef unsigned int uint;

//!TCDMSAnalysis class that all other analysis classes inherit from
class TCDMSAnalysis 
{
   public:

      TCDMSAnalysis();  //constructor (not inherited)
      ~TCDMSAnalysis(); //destructor (not inherited)

      //Get functions
      map<string,double>      GetRQList() const    { return fRQList; } 
      string                  GetClassName() const { return fClassName; }
      double                  GetRQVal(const string& rqName) const;

   protected:

      //For RQ management
      //virtual void ConstructRQList(); //to be overridden by derived classes

      map<string,double> fRQList;
      string                  fClassName; 
      string                  fAnalysisInitials; 
      bool                    fStoreRQs;

};

#endif /* TCDMSANALYSIS_H */
