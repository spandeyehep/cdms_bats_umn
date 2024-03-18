///////////////////////////////////////////////////////////////////////////////// 
//Class Name: GenRRQDataEvent
//Authors: L. Hsu
//Description: This class was split off from the original GenRRQDataEvent class,
//which has now morphed into GenRRQDataCDMSII.
//
//File Import By: L. Hsu
//Creation Date: Jan. 18, 2011
//
//Modifications:
//
////////////////////////////////////////////////////////////////////////////////// 


#ifndef GENRRQDATAEVENT_H
#define GENRRQDATAEVENT_H

#include <iostream>
#include <vector>
#include <map>

#include "BatCalibIOManager.h"

using namespace std;

//!This is the GenRRQDataEvent Class.
class GenRRQDataEvent 
{
   public:

      GenRRQDataEvent(BatCalibIOManager ioManager);  
      ~GenRRQDataEvent(); //destructor 

      void DoEventCalc(int maxEvents, UserDataManager& myUserData);

   private:

      // ===== functions =====



      // ===== data members =====

      map<string, double> fRRQList;
      BatCalibIOManager   fIOMan;
      UserDataManager     fUserData;


      //data descriptions
      bool              fIsFirstProduction; //some rq's not stored in first BatRoot production, so we can't compute rrq's
     
};

#endif /* GENRRQDATAEVENT_H */
