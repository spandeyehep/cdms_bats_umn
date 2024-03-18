///////////////////////////////////////////////////////////////////////////////// 
//Class Name: GenRRQDataEvent
//Authors: L. Hsu
//Description:  This class was split off from the original GenerateRRQData class,
//which has now morphed into GenRRQDataCDMSII. Stores event number, series and 
//some trig info
//
//File Import By: L. Hsu
//Creation Date: Jan. 18, 2011 
//
//Modifications:
//
////////////////////////////////////////////////////////////////////////////////// 

#include <iostream>
#include <sstream>
#include <math.h>

//utilities for getting unix time
#include <time.h>

#include "TMath.h"

#include "GenRRQDataEvent.h"
#include "BatCalibTypes.h"
#include "BatRootTypes.h"

GenRRQDataEvent::GenRRQDataEvent(BatCalibIOManager ioManager) :
   fIOMan(ioManager),
   fIsFirstProduction(0)
{
//   cout <<"Hello from GenRRQDataEvent! " << endl;

}

GenRRQDataEvent::~GenRRQDataEvent()
{
//   cout <<"Goodbye from GenRRQDataEvent()" << endl;
}


// ================== Calculations ======================
//
// controls looping over events!
//
// ======================================================


void GenRRQDataEvent::DoEventCalc(int maxEvents, UserDataManager& myUserData)
{

   fIsFirstProduction = fIOMan.DoesRQFilePredate("01-Dec-2009");


   // --- 1.  Activate the rqs ---
   
   fIOMan.LoadTree("rqDir", "eventTree");

   fIOMan.Activate("SeriesNumber");
   fIOMan.Activate("EventNumber");
   
   //cryocooler noise triggers
   fIOMan.Activate("NM55PreTime");
   fIOMan.Activate("NM55PostTime");
   fIOMan.Activate("TimeBetween");
   
   int maxTowers = myUserData.GetIntParameter("MAX_TOWERS");

   if( !fIsFirstProduction && myUserData.DoTriggerProcessing())
   {
      for(int towerCtr=0; towerCtr < maxTowers; towerCtr++)
      {
	 fIOMan.Activate(Form("T%dNTrigP", towerCtr+1));
	 fIOMan.Activate(Form("T%dNTrigQ", towerCtr+1));
      }
   }
   
   // --- 2. Setup the output rrq's ---

   double initval = BatRootTypes::kEmptyVariable;

   fRRQList.insert(pair<string,double>("EventNumber", initval));
   fRRQList.insert(pair<string,double>("SeriesNumber", initval));
   // ==========  time from series string converted into unix time  ==========
   fRRQList.insert(pair<string,double>("SeriesTime", initval));
   // ========== cryocooler noise triggers =========== //
   fRRQList["CryocoolerPreTime"] = initval;
   fRRQList["CryocoolerPostTime"] = initval;
   
   if( !fIsFirstProduction )
   {
      fRRQList.insert(pair<string,double>("ntrigp", initval));
      fRRQList.insert(pair<string,double>("ntrigq", initval));
   }

   string treeName(Form("calibevent"));
   fIOMan.ConstructOutputRRQTree(&fRRQList, treeName);

   // ---- 3. do some calculations that need not be done for all iterations ------- [ANV]
    
   //convert the series string from fIOMan
   double seriesTime;
   //string strSeries;
   string strSeries = fIOMan.GetSeriesStringWithoutUnderscore();

   //change to ostringstream and get from SeriesNumber because
   //potentially faster than above call to GetSeriesStringWithoutUnderscore() 
   //but won't currently work because fIOMan.ReadNextEntry hasn't been run[ANV]
   //ostringstream streamSeries;
   //streamSeries << fIOMan.Get("SeriesNumber");
   //strSeries = streamSeries.str();


   int year=0,mon=0,mday=0,hour=0,min=0;

   int value;
   char buff=0;
   int length = std::distance(strSeries.begin(),strSeries.end());
   for(string::reverse_iterator itr = strSeries.rbegin();itr!=strSeries.rend();++itr){
     int count = std::distance(strSeries.rbegin(),itr);
    
     //ten digits from the end.
     if(count%2==1 && count<10){ 
       ostringstream twodigits;
       if(count == 9 && length==10)
         twodigits << buff;
       else
         twodigits << *itr << buff;
       istringstream sametwodigits(twodigits.str());
       sametwodigits >> value; 
       switch(count){
         case 9:
	   year = value;
	   break;

         case 7:
	   mon = value;
	   break;

         case 5:
	   mday = value;
	   break;

         case 3:
	   hour = value;
	   break;

         case 1:
	   min = value;
	   break;

       }
     }
     else
       buff=*itr;
   }

   //this is slightly tricky because of daylight savings so use the time_t object [ANV]
   //http://www.cplusplus.com/reference/ctime/mktime/
   time_t rawtime;
   struct tm *timeinfo;
   timeinfo = gmtime ( &rawtime );

   timeinfo->tm_year = year + 100;
   timeinfo->tm_mon = mon - 1;
   timeinfo->tm_mday = mday;
   timeinfo->tm_hour = hour;
   timeinfo->tm_min = min;
   timeinfo->tm_sec = 0;
   timeinfo->tm_isdst = -1;

   rawtime = mktime(timeinfo);

   ctime(&rawtime);
   string timezone;
   if(timeinfo->tm_isdst==0)
     timezone="CST";
   else if(timeinfo->tm_isdst==1)
     timezone="CDT";
    else
      timezone="unknown";

   seriesTime = (double) rawtime; //shouldn't loose precision [ANV]

   // ----- 4. Loop over all events and do the calculations! -----

   cout <<"In DoCalc, maxEvents = " << maxEvents << endl;
   cout <<"Size of this tree is = " << fIOMan.GetMaxEntries() << endl;

   int maxEntries = fIOMan.GetMaxEntries();

   for(int eventCtr = 0; (eventCtr < maxEvents && eventCtr < maxEntries) ; eventCtr++)
   {
      //cout <<"eventCtr = " << eventCtr << endl;

      //read the next entry from the file
      fIOMan.ReadNextEntry(eventCtr);
      
      //"calculations"

      //just copy the event and series number of indexing purposes
      fRRQList["EventNumber"] = fIOMan.Get("EventNumber");
      fRRQList["SeriesNumber"] = fIOMan.Get("SeriesNumber");
      
      //simple calculation for ntrigp and ntrigq (for glitch cut) - only for later datasets
      if( !fIsFirstProduction && myUserData.DoTriggerProcessing())
      {
	 int ntrigp = 0;
	 int ntrigq = 0;

	 for(int towerCtr=0; towerCtr < maxTowers; towerCtr++)
	 {
	    ntrigp += (int)fIOMan.Get(Form("T%dNTrigP", towerCtr+1));
	    ntrigq += (int)fIOMan.Get(Form("T%dNTrigQ", towerCtr+1));
	 }

	 fRRQList["ntrigp"] = ntrigp;
	 fRRQList["ntrigq"] = ntrigq;
      }
    
      fRRQList["SeriesTime"] = seriesTime; //calculated once outside the loop [ANV]
      
      //  ------- Cryocooler noise triggers ------- //
      
      // Look back into previous events to find valid cryocooler noise trigs
      double rawtime = fIOMan.Get("NM55PreTime");
      double oldpost = fRRQList["CryocoolerPostTime"];
      double oldpre = fRRQList["CryocoolerPreTime"];
      double TimeBetween = fIOMan.Get("TimeBetween");
      
      if(rawtime != BatRootTypes::kEmptyVariable){
	fRRQList["CryocoolerPreTime"] = rawtime;
      }
      else if(TimeBetween != BatRootTypes::kEmptyVariable){
	TimeBetween = -TimeBetween*1000; //convert to microsec
	if(oldpost != BatRootTypes::kEmptyVariable)
	  fRRQList["CryocoolerPreTime"] = oldpost + TimeBetween;
	else if(oldpre != BatRootTypes::kEmptyVariable)
	  fRRQList["CryocoolerPreTime"] = oldpre + TimeBetween;
	else
	  fRRQList["CryocoolerPreTime"] = BatRootTypes::kEmptyVariable;
      }
      else
	fRRQList["CryocoolerPreTime"] = BatRootTypes::kEmptyVariable;
      
      //Just copy the post time
      fRRQList["CryocoolerPostTime"] = fIOMan.Get("NM55PostTime");
      
      
      
      // ---- Store the data and reset the RRQ list values ----
      
      fIOMan.FillOutputRRQTree();       
      
   } //Done looping over events
   

   // --- 4. Write the tree ---
   
   fIOMan.WriteOutputRRQTree();

   // --- Done! ---

   fIOMan.DeleteActiveTree();

   return;
}



