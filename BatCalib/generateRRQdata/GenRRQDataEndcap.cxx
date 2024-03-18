///////////////////////////////////////////////////////////////////////////////// 
//Class Name: GenRRQDataCDMSII
//Authors: L. Hsu
//Description: This class is intended for the endcap detectors - old CDMSII style
//detectors with the phonon channels ganged 2 per side and charge ganged into one.
//Applies charge calibration (cross talk, amplitude scaling, position 
//correction).  Applies phonon calibration (relative amplitude scaling).  Generates 
//primary rrq's for analysis (xy-delays, partitions, yields).
//
//File Import By: L. Hsu
//Creation Date: Jan. 18, 2011 
//
//Modifications:
//
////////////////////////////////////////////////////////////////////////////////// 

#include <iostream>
#include <math.h>

#include "TMath.h"

#include "GenRRQDataEndcap.h"
#include "BatCalibTypes.h"

////////////////////////////////////////////////////////
//
// Endcap Channel Mapping:
// top - qi, pc, pd
// bottom - qo, pa, pb
//
////////////////////////////////////////////////////////

GenRRQDataEndcap::GenRRQDataEndcap(BatCalibIOManager ioManager) :
   fIOMan(ioManager),
   fDetType(-999999)
{
//   cout <<"Hello from GenRRQDataEndcap! " << endl;

}

GenRRQDataEndcap::~GenRRQDataEndcap()
{
//   cout <<"Goodbye from GenRRQDataEndcap()" << endl;
}


//initialize desired output variables here (this happens before looping over events in the zip trees)
void GenRRQDataEndcap::ConstructRRQList()
{
     double initVal = -999999.;

     //construct the RRQ list here.  Note anything added outside of here 
     //is not going to be in the output

     fRRQList.insert(pair<string,double>("Empty", initVal));
     fRRQList.insert(pair<string,double>("DetType", initVal));


     // === optimal filter quantities ===

     if(fIOMan.CheckBatRootChargeAlg("OptimalFilterCharge", fDetNum))
     {
       fRRQList.insert(pair<string,double>("qOF", initVal));
       fRRQList.insert(pair<string,double>("qOF0", initVal));
       fRRQList.insert(pair<string,double>("qsumOF", initVal)); //the same as qOF
     }


     if(fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon", fDetNum))
     {

       fRRQList.insert(pair<string,double>("paOF", initVal));
       fRRQList.insert(pair<string,double>("pbOF", initVal));

       fRRQList.insert(pair<string,double>("paOF0", initVal));
       fRRQList.insert(pair<string,double>("pbOF0", initVal));

       fRRQList.insert(pair<string,double>("psumOF", initVal));
       fRRQList.insert(pair<string,double>("precoilgOF", initVal)); 
       fRRQList.insert(pair<string,double>("precoilsumOF", initVal));
	
       fRRQList.insert(pair<string,double>("ysumOF", initVal));
       fRRQList.insert(pair<string,double>("ygsumOF", initVal));

       fRRQList.insert(pair<string,double>("pxpartOF", initVal));
     }

     // === integral quantities ===


     if(fIOMan.CheckBatRootPhononAlg("PulseIntegral", fDetNum))
     {
       fRRQList.insert(pair<string,double>("paINT", initVal));
       fRRQList.insert(pair<string,double>("pbINT", initVal));
	   
       fRRQList.insert(pair<string,double>("psumINT", initVal));
       fRRQList.insert(pair<string,double>("precoilgINT", initVal));
       fRRQList.insert(pair<string,double>("precoilsumINT", initVal));
	   
       fRRQList.insert(pair<string,double>("ysumINT", initVal));
       fRRQList.insert(pair<string,double>("ygsumINT", initVal));
       
       fRRQList.insert(pair<string,double>("pxpartINT", initVal));
     }


     //phonon delays

     if(fUserData.DoZipAlgorithm(fDetNum, "CalcConstFreqRTFTWalkRRQ") &&
	fIOMan.CheckBatRootPhononAlg("ConstFreqRTFTWalkPhonon", fDetNum))
     {
       fRRQList.insert(pair<string,double>("pxdelWK", initVal));
     }


     //optimal filter resolutions

     fRRQList.insert(pair<string,double>("padelayres", initVal));
     fRRQList.insert(pair<string,double>("pbdelayres", initVal));

     fRRQList.insert(pair<string,double>("paampres", initVal));
     fRRQList.insert(pair<string,double>("pbampres", initVal));

     fRRQList.insert(pair<string,double>("qampres", initVal));
     fRRQList.insert(pair<string,double>("qdelayres", initVal));


     //Now tell the io manager to make an output tree based on this list    

     string treeName(Form("calibzip%d", fDetNum));
     fIOMan.ConstructOutputRRQTree(&fRRQList, treeName);

     return;
}

void GenRRQDataEndcap::ResetRRQValues()
{
   double initVal = -999999.;

   map<string, double>::iterator rrqListItr = fRRQList.begin();

   for( ; rrqListItr!=fRRQList.end(); rrqListItr++)
   {
      rrqListItr->second = initVal;
   }

   return;
}

// set branch addresses before looping over events
// this reads the first entry to guarantee that the DetType is filled, which we need to setup the output RRQ's
void GenRRQDataEndcap::ActivateRQs()
{

   //Any RQ that is needed for calculation should be specified here
   //otherwise it is not read from the file
  
   //to first order, everything needs this
   fIOMan.Activate("Empty");
   fIOMan.Activate("DetType");

   //Read the entries until one gets to the first non-empty value - b/c detType is not stored for empty events
   int maxEntries = fIOMan.GetMaxEntries();
   
   for(int eventCtr = 0; eventCtr < maxEntries; eventCtr++)
   {
     fIOMan.ReadNextEntry(eventCtr);
     
     if(fIOMan.Get("Empty") == 0.0) 
     {
       //Store the Det_Type variable
       fDetType = (int)fIOMan.Get("DetType");
       break;
     }

   }
	

   // --- Qbias, gain, norms ---

   //For TF and MC we currently don't have external files
   //this leaves the option to pull the values (prespecified) from the config files
   //this is less than ideal and should be fixed in the longer term by integrating
   //these values in the raw data stream
   if(fUserData.GetIntParameter("OVERRIDE_BIAS_WCONFIG") == 0)
   {
      fIOMan.Activate("Qbias"); 
   }

   fIOMan.Activate("Qgain");
   fIOMan.Activate("Qnorm");

   // --- Charge Optimal Filter ---

   if(fIOMan.CheckBatRootChargeAlg("OptimalFilterCharge", fDetNum))
   {
     fIOMan.Activate("QOFnoXvolts");
     fIOMan.Activate("QOFnoXvolts0");
   }

   // --- F5 ---

   //non-cross talk version not avaiable right now   
//    if(fUserData.DoZipAlgorithm(fDetNum, "CalcF5SatEnergy"))
//    {
//       fIOMan.Activate("QF5volts");
//    }

   // --- misc ---
   
   fIOMan.Activate("Qsat");


   // ----- Phonon RQ's -----
   
   for(int chanItr = 0; chanItr < BatCalibTypes::kEndcapNPhononChan; chanItr++)
   {
      //optimal filter
      if(fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon", fDetNum))
      {
	fIOMan.Activate(BatCalibTypes::kEndcapPhononChan[chanItr]+"OFamps");
	fIOMan.Activate(BatCalibTypes::kEndcapPhononChan[chanItr]+"OFamps0");
      }

      //integral routine
      if(fIOMan.CheckBatRootPhononAlg("PulseIntegral", fDetNum))
      {
	fIOMan.Activate(BatCalibTypes::kEndcapPhononChan[chanItr]+"INTall");
      }	

      //gain
      fIOMan.Activate(BatCalibTypes::kEndcapPhononChan[chanItr] + "gain");

      //norm
      fIOMan.Activate(BatCalibTypes::kEndcapPhononChan[chanItr] + "norm");
	 
   } //end loop over phonon chan


   // --- ConstFreqRTFTwalk ---

   if(fUserData.DoZipAlgorithm(fDetNum, "CalcConstFreqRTFTWalkRRQ") &&
      fIOMan.CheckBatRootPhononAlg("ConstFreqRTFTWalkPhonon", fDetNum)) 
   {
      for(int chanItr = 0; chanItr < BatCalibTypes::kEndcapNPhononChan; chanItr++)
      {
	 fIOMan.Activate(BatCalibTypes::kEndcapPhononChan[chanItr]+"WKr20");
      }

   } //endif do ConstFreqRTFTWalk


   return;

}

// ================== Calculations ======================
//
// DoCalibration controls looping over events!
//
// ======================================================

void GenRRQDataEndcap::DoCalibration(int maxEvents, int detNum, UserDataManager& myUserData)
{

   // --- 1. Store description of detector for local access ---

   fUserData = myUserData;
   fDetNum = detNum;
   fIsGe = fUserData.IsGe(fDetNum);
   fIsSi = fUserData.IsSi(fDetNum);

   if(fUserData.DoZipAlgorithm(fDetNum, "CalibrateOFRes"))
      fIOMan.FillOFResolution(fDetNum, fDelaySig, fAmpSig);  //FIXME, only temporary until merging script bug is fixed


   // --- 2.  Activate the rqs ---
   
   string zipTreeName(Form("zip%d", fDetNum));
   int isValidTree = fIOMan.LoadTree("rqDir", zipTreeName);

   //return to main loop if the tree doesn't exist (as is case for Hybrid running conditions)
   if(isValidTree == 0)
   {
      return;
   }
   
   ActivateRQs();  //fDetType is read from rq file and set in ActivateRQs()


   // --- 3. Setup the output rrq's ---

   ConstructRRQList();


   // --- 4. Retrieve calibration constants (outside loop over events to be efficient) ---
   
   fChargeCal = fUserData.GetVectDoubleParameter(fDetNum, "Q_CALIBRATION_OF");   
   fPhononCal = fUserData.GetVectDoubleParameter(fDetNum, "P_CALIBRATION_OF"); //for optimal filter
   fEpsilon = fUserData.GetDoubleParameter(fDetNum, "EPSILON"); 

   fOFMaxTemplate = fUserData.GetVectDoubleParameter(fDetNum, "OF_MAX_TEMPLATE");  //FIXME when this is available in RQ files
   fPhononIntCal = fUserData.GetVectDoubleParameter(fDetNum, "P_CALIBRATION_INT"); 


   // --- 5. Some checks before performing calculations ----

   if(fPhononCal.size() != (uint)BatCalibTypes::kEndcapNPhononChan)
   {
     cout <<"GenRRQDataEndcap::DoCalibration ERROR! "
	  <<"Number of phonon calibration constants does not match number of phonon channels, please check"
	  << endl;
     exit(1);
   }

   // ----- 6. Loop over all events and do the calculations! -----

   cout <<"In DoCalc, maxEvents = " << maxEvents << endl;
   cout <<"Size of this tree is = " << fIOMan.GetMaxEntries() << endl;

   int maxEntries = fIOMan.GetMaxEntries();

   for(int eventCtr = 0; (eventCtr < maxEvents && eventCtr < maxEntries) ; eventCtr++)
   {
      //cout <<"eventCtr = " << eventCtr << endl;

      //read the next entry from the file
      fIOMan.ReadNextEntry(eventCtr);
      
      //skip this entry if it was not read out (for selective readout)
      if(fIOMan.Get("Empty") != 0.0) 
      {
 	 fIOMan.FillOutputRRQTree(); //fills with empty "default" of -999999, no need to reset     
 	 continue;
      }
      else
      {
	 fRRQList["Empty"] = 0; //not empty
      }

      // ---- mandatory calculations ---- 
      
      //the order in which these are called matters !

      ApplyPhononCalibration();     //1. apply phonon relative calibration
      
      CalcPhononDelays();           //2. calculate xy delays (needs relative phonon cal)
      
      ApplyChargeCalibration();     //3. apply charge calibration and position correction (needs phonon delays)
      
      CalcTotalEnergiesAndYields(); //4. calculate qsum and recoil energies
      
      CalcPartitions();             //5. calculate phonon and charge partitions

      if(fUserData.DoZipAlgorithm(fDetNum, "CalibrateOFRes"))
	 CalcOFResolutions();          //6. calculate optimal filter resolutions - FIXME, only temporary until merge script bug is fixed
      

      // --- Store some misc items ---
      
      fRRQList["DetType"] = fDetType; //inefficient, but needed for pull teeth right now

      
      // ---- Store the data and reset the RRQ list values ----


      fIOMan.FillOutputRRQTree();      
      ResetRRQValues();
 
     
   } //Done looping over events
 

   // --- 7. Write the tree ---
   
   fIOMan.WriteOutputRRQTree();


   // --- Done! ---

   fIOMan.DeleteActiveTree();

   return;
}

// ==================== the following routines are called from within a loop over events =====================


void GenRRQDataEndcap::ApplyPhononCalibration()
{
   
   //Optimal Filter energy
   if(fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon", fDetNum))
   {
     fRRQList["paOF"] = fPhononCal[0]*fIOMan.Get("PAOFamps");
     fRRQList["pbOF"] = fPhononCal[1]*fIOMan.Get("PBOFamps");

     fRRQList["paOF0"] = fPhononCal[0]*fIOMan.Get("PAOFamps0");
     fRRQList["pbOF0"] = fPhononCal[1]*fIOMan.Get("PBOFamps0");
   }

   //Integral energy
   if(fIOMan.CheckBatRootPhononAlg("PulseIntegral", fDetNum))
   {
     fRRQList["paINT"] = fPhononIntCal[0]*fIOMan.Get("PAINTall");
     fRRQList["pbINT"] = fPhononIntCal[1]*fIOMan.Get("PBINTall");
   }

   return;
}

//Note that the user can make a choice in whether constant frequency or variable frequency rtftwalk values are used for the delay calculations in this function.   
//This is done by setting the flag in the user settings file "DefaultToConstFreqRTFTWalk" 
void GenRRQDataEndcap::CalcPhononDelays()
{
   if(fIOMan.CheckBatRootPhononAlg("ConstFreqRTFTWalkPhonon", fDetNum))
   {
     fRRQList["pxdelWK"] = (fIOMan.Get("PBWKr20") - fIOMan.Get("PAWKr20"))*1e6;
   }

  return;
} 

void GenRRQDataEndcap::ApplyChargeCalibration()
{

   //to hold calibration constants
   double qa = fChargeCal[0];

   if(fIOMan.CheckBatRootChargeAlg("OptimalFilterCharge", fDetNum))
   { 
     //same calculation whether saturated or not because F5 does not have modification to remove cross talk
     fRRQList["qOF"] = qa*fIOMan.Get("QOFnoXvolts");
     fRRQList["qOF0"] = qa*fIOMan.Get("QOFnoXvolts0");
     fRRQList["qsumOF"] = fRRQList["qOF"];
   }


   return;
}

void GenRRQDataEndcap::CalcTotalEnergiesAndYields()
{

   //get qi/qo bias - they are used for all the detector below
   double qiBias = -999999;
   // double qoBias = -999999;

   if(fUserData.GetIntParameter("OVERRIDE_BIAS_WCONFIG") == 0)
   {
      qiBias = fIOMan.Get("Qbias"); 
   }
   else
   {
     qiBias = fUserData.GetDoubleParameter(fDetNum, "QBIAS"); //FIXME - this will not work right now
   }

   double qifac = fabs(qiBias/fEpsilon); //should always be positive?

   // === total phonon energy ===
   
   if(fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon", fDetNum))
   {
     double psum = fRRQList["paOF"] + fRRQList["pbOF"];
     fRRQList["psumOF"] = psum;
      

     // === recoil energies ===
   
     fRRQList["precoilgOF"] = psum/(1.0+qifac); 
     fRRQList["precoilsumOF"] = psum - (fRRQList["qOF"]*qifac);   //note, q and qsum are the same for endcaps
   
     // === yields ===
   
     fRRQList["ysumOF"] = fRRQList["qOF"]/fRRQList["precoilsumOF"];
     fRRQList["ygsumOF"] = fRRQList["qOF"]/fRRQList["precoilgOF"];

   }
      

   // === integral quantities ===
   if(fIOMan.CheckBatRootPhononAlg("PulseIntegral", fDetNum))
   {   
     double psumINT = fRRQList["paINT"] + fRRQList["pbINT"];
     fRRQList["psumINT"] = psumINT;
	 
     fRRQList["precoilgINT"] = psumINT/(1.0+qifac); 
     fRRQList["precoilsumINT"] = psumINT - (fRRQList["qOF"]*qifac);
     
     fRRQList["ysumINT"] = fRRQList["qOF"]/fRRQList["precoilsumINT"];
     fRRQList["ygsumINT"]   = fRRQList["qOF"]/fRRQList["precoilgINT"];
   }      


   return;
}

//mostly just for convenience
void GenRRQDataEndcap::CalcPartitions()
{
   
  if(fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon", fDetNum))
  {
    fRRQList["pxpartOF"] = (fRRQList["pbOF"] - fRRQList["paOF"]) / (fRRQList["paOF"] + fRRQList["pbOF"]);
  }


  if(fIOMan.CheckBatRootPhononAlg("PulseIntegral", fDetNum))
  {
    fRRQList["pxpartINT"] = (fRRQList["pbINT"] - fRRQList["paINT"]) / (fRRQList["paINT"] + fRRQList["pbINT"]);
  }

  return;
}
     
//calibrate the OF resolution quantities
void GenRRQDataEndcap::CalcOFResolutions()
{

   // === calibrate the OFdelay and OFamplitude resolutions ===
   //      note values stored in order Q, PA, PB

  // calibrate charge resolution

   double tempQMax = fOFMaxTemplate[0];

   fRRQList["qdelayres"] = fChargeCal[0]*tempQMax*fDelaySig[0];      
   fRRQList["qampres"] = fChargeCal[0]*tempQMax*fAmpSig[0];


   // calibrate phonon resolutions

   for(int chanItr=0; chanItr < BatCalibTypes::kEndcapNPhononChan; chanItr++)
   {
     string prefix = BatCalibTypes::kEndcapPhononChan[chanItr];
     string prefixCal = BatCalibTypes::kEndcapPhononCal[chanItr];

     fRRQList[prefixCal+"delayres"] = ( fIOMan.Get(prefix + "gain")/fIOMan.Get(prefix + "norm") )*fPhononCal[chanItr]*fDelaySig[chanItr+1];  
     fRRQList[prefixCal+"ampres"] = ( fIOMan.Get(prefix + "gain")/fIOMan.Get(prefix + "norm") )*fPhononCal[chanItr]*fAmpSig[chanItr+1]; 

   }

   return;
}


     




