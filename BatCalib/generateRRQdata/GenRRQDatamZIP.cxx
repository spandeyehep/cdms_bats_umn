///////////////////////////////////////////////////////////////////////////////// 
//Class Name: GenRRQDatamZIP
//Authors: L. Hsu
//Description: This class is intended for the 1-inch, single side phonon and single
//side charge ZIPs with "mercedes" style phonon sensor layout (4 channels) - the mZIP.
//Applies charge calibration (cross talk, amplitude scaling, position 
//correction).  Applies phonon calibration (relative amplitude scaling).  Generates 
//primary rrq's for analysis (xy-delays, partitions, yields, timing quantities).
//
//File Import By: L. Hsu
//Creation Date: Jan. 18, 2011 
//
//Modifications:
//
////////////////////////////////////////////////////////////////////////////////// 

#include <iostream>
#include <math.h>
#include <limits>

#include "TMath.h"

#include "GenRRQDatamZIP.h"
#include "BatCalibTypes.h"


GenRRQDatamZIP::GenRRQDatamZIP(BatCalibIOManager ioManager) :
   fIOMan(ioManager),
   fDetType(-999999),
   fPrimaryPhononChan(-999999),
   fPrimaryInnerPhononChan(-999999),
   fIsFirstProduction(0)
{
//   cout <<"Hello from GenRRQDatamZIP! " << endl;

   kThetaVect[0] = 150.*(TMath::Pi()/180.);
   kThetaVect[1] = 270.*(TMath::Pi()/180.);
   kThetaVect[2] = 30.*(TMath::Pi()/180.);

}

GenRRQDatamZIP::~GenRRQDatamZIP()
{
//   cout <<"Goodbye from GenRRQDatamZIP()" << endl;
}


//initialize desired output variables here (this happens before looping over events in the zip trees)
void GenRRQDatamZIP::ConstructRRQList()
{
     double initVal = -999999.;

     //construct the RRQ list here.  Note anything added outside of here 
     //is not going to be in the output

     fRRQList.insert(pair<string,double>("Empty", initVal));
     fRRQList.insert(pair<string,double>("DetType", initVal));

     fRRQList.insert(pair<string,double>("pprimechanOFWK", initVal));
     fRRQList.insert(pair<string,double>("pprimechan", initVal));  //in case there are other pprimechan defs

     // Optimal Filter

     if(fIOMan.CheckBatRootChargeAlg("OptimalFilterChargeX", fDetNum) ||
	fIOMan.CheckBatRootChargeAlg("OptimalFilterCharge", fDetNum))
     {
       fRRQList.insert(pair<string,double>("qiOF", initVal));
       fRRQList.insert(pair<string,double>("qoOF", initVal));
       fRRQList.insert(pair<string,double>("qiOF0", initVal));
       fRRQList.insert(pair<string,double>("qoOF0", initVal));
       fRRQList.insert(pair<string,double>("qsumOF", initVal));

       fRRQList.insert(pair<string,double>("qrpartOF", initVal)); 
     }


     if(fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon", fDetNum))
     {
       
       // Energies

       fRRQList.insert(pair<string,double>("paOF", initVal));
       fRRQList.insert(pair<string,double>("pbOF", initVal));
       fRRQList.insert(pair<string,double>("pcOF", initVal));
       fRRQList.insert(pair<string,double>("pdOF", initVal));

       fRRQList.insert(pair<string,double>("paOF0", initVal));
       fRRQList.insert(pair<string,double>("pbOF0", initVal));
       fRRQList.insert(pair<string,double>("pcOF0", initVal));
       fRRQList.insert(pair<string,double>("pdOF0", initVal));       

       fRRQList.insert(pair<string,double>("psumOF", initVal));
       fRRQList.insert(pair<string,double>("psumiOF", initVal));
       fRRQList.insert(pair<string,double>("psumoOF", initVal));
       
       fRRQList.insert(pair<string,double>("precoilsumOF", initVal));
       fRRQList.insert(pair<string,double>("precoilgOF", initVal));  
       fRRQList.insert(pair<string,double>("precoiliOF", initVal)); 

       // partitions

       fRRQList.insert(pair<string,double>("pxpartOF", initVal));
       fRRQList.insert(pair<string,double>("pypartOF", initVal));
       fRRQList.insert(pair<string,double>("prxypartOF", initVal));
       fRRQList.insert(pair<string,double>("prpartOF", initVal));

       // yields

       fRRQList.insert(pair<string,double>("ysumOF", initVal));
       fRRQList.insert(pair<string,double>("ygsumOF", initVal)); 
       fRRQList.insert(pair<string,double>("ysumiOF", initVal)); 
       fRRQList.insert(pair<string,double>("ygsumiOF", initVal));

     }


     // integral quantities

     if(fIOMan.CheckBatRootPhononAlg("PulseIntegral", fDetNum))
     {
       fRRQList.insert(pair<string,double>("paINT", initVal));
       fRRQList.insert(pair<string,double>("pbINT", initVal));
       fRRQList.insert(pair<string,double>("pcINT", initVal));
       fRRQList.insert(pair<string,double>("pdINT", initVal));
     
       fRRQList.insert(pair<string,double>("psumINT", initVal));
       fRRQList.insert(pair<string,double>("psumiINT", initVal));
       fRRQList.insert(pair<string,double>("psumoINT", initVal));

       fRRQList.insert(pair<string,double>("precoilsumINT", initVal)); 
       fRRQList.insert(pair<string,double>("precoilgsumINT", initVal)); 
       fRRQList.insert(pair<string,double>("precoilsumiINT", initVal)); 
       
       // partitions

       fRRQList.insert(pair<string,double>("pxpartINT", initVal));
       fRRQList.insert(pair<string,double>("pypartINT", initVal));
       fRRQList.insert(pair<string,double>("prxypartINT", initVal));
       fRRQList.insert(pair<string,double>("prpartINT", initVal));

       // yields

       fRRQList.insert(pair<string,double>("ysumINT", initVal));
       fRRQList.insert(pair<string,double>("ygsumINT", initVal)); 
       fRRQList.insert(pair<string,double>("ysumiINT", initVal)); 
       fRRQList.insert(pair<string,double>("ygsumiINT", initVal));

     }	


     //phonon delays

     if(fIOMan.CheckBatRootPhononAlg("ConstFreqRTFTWalkPhonon", fDetNum))
     {     
       fRRQList.insert(pair<string,double>("pxdelWK", initVal));
       fRRQList.insert(pair<string,double>("pydelWK", initVal));
     }

     //optimal filter resolutions

     fRRQList.insert(pair<string,double>("padelayres", initVal));
     fRRQList.insert(pair<string,double>("pbdelayres", initVal));
     fRRQList.insert(pair<string,double>("pcdelayres", initVal));
     fRRQList.insert(pair<string,double>("pddelayres", initVal));

     fRRQList.insert(pair<string,double>("paampres", initVal));
     fRRQList.insert(pair<string,double>("pbampres", initVal));
     fRRQList.insert(pair<string,double>("pcampres", initVal));
     fRRQList.insert(pair<string,double>("pdampres", initVal));

     fRRQList.insert(pair<string,double>("qiampres", initVal));
     fRRQList.insert(pair<string,double>("qoampres", initVal));
     fRRQList.insert(pair<string,double>("qidelayres", initVal));
     fRRQList.insert(pair<string,double>("qodelayres", initVal));


     // ---- RRQ's from optional calculations -----

     //Const Freq RTFTWalk
     if(fUserData.DoZipAlgorithm(fDetNum, "CalcConstFreqRTFTWalkRRQ"))
     {
	//for primary channel
	fRRQList.insert(pair<string,double>("pminrtWK_1040", initVal));
	fRRQList.insert(pair<string,double>("pminrtWK_4070", initVal));
	fRRQList.insert(pair<string,double>("pminrtWK_1030", initVal));
	fRRQList.insert(pair<string,double>("pminrtWK_3050", initVal));
	fRRQList.insert(pair<string,double>("pminrtWK_5080", initVal));

	fRRQList.insert(pair<string,double>("pqdelWK", initVal));
	fRRQList.insert(pair<string,double>("ptopwidthWK", initVal));
	fRRQList.insert(pair<string,double>("prdelWK", initVal)); //FIXME, def is not as shown in wiki!

	//for total phonon pulse
	if(fUserData.DoZipAlgorithm(fDetNum, "TotalPhonon"))
	{
	  fRRQList.insert(pair<string,double>("ptrtWK_1040", initVal));
	  fRRQList.insert(pair<string,double>("ptqdelWK", initVal));
	}
     }


    //Now tell the io manager to make an output tree based on this list    

    string treeName(Form("calibzip%d", fDetNum));
    fIOMan.ConstructOutputRRQTree(&fRRQList, treeName);

    return;
}

void GenRRQDatamZIP::ResetRRQValues()
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
void GenRRQDatamZIP::ActivateRQs()
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
	
   if(fDetType == -999999)
   {
     cout <<"WARNING! Ran through entire file and all entries are empty for this detector"
	  <<"\nThis is likely on a problem if you want to read this single dump in matlab"
	  <<"\nIf this is not selective read-out data, check file!" 
	  << endl; 
   }
	

   // --- Qbias, gain, norms ---

   //For TF and MC we currently don't have external files
   //this leaves the option to pull the values (prespecified) from the config files
   //this is less than ideal and should be fixed in the longer term by integrating
   //these values in the raw data stream
   if(fUserData.GetIntParameter("OVERRIDE_BIAS_WCONFIG") == 0)
   {
      fIOMan.Activate("QIbias"); 
      fIOMan.Activate("QObias");
   }

   fIOMan.Activate("QIgain");
   fIOMan.Activate("QOgain");

   fIOMan.Activate("QInorm");
   fIOMan.Activate("QOnorm");

   // --- Charge Optimal Filter ---

   if(fIOMan.CheckBatRootChargeAlg("OptimalFilterChargeX", fDetNum))
   {
     fIOMan.Activate("QIOFvolts");
     fIOMan.Activate("QOOFvolts");
     fIOMan.Activate("QIOFvolts0");
     fIOMan.Activate("QOOFvolts0");
     fIOMan.Activate("QSOFdelay");
    
   }

   // --- F5 ---
   
   if(fUserData.DoZipAlgorithm(fDetNum, "CalcF5SatEnergy") &&
      fIOMan.CheckBatRootChargeAlg("F5ChargeX", fDetNum) )
   {
      fIOMan.Activate("QIF5volts");
      fIOMan.Activate("QOF5volts");
   }

   // --- misc ---
   
   fIOMan.Activate("QIsat");
   fIOMan.Activate("QOsat");


   // ----- Phonon RQ's -----

   // --- Phonon Optimal Filter and Integral ---
   
   for(int chanItr = 0; chanItr < BatCalibTypes::kZIPFLIPNPhononChan; chanItr++)
   {
      // optimal filter
     if(fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon", fDetNum))
     {
       fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"OFamps");
       fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"OFamps0");
     }

     // integral routine
     if(fIOMan.CheckBatRootPhononAlg("PulseIntegral", fDetNum))
     {
       fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"INTall");
     }

     // gain
     fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr] + "gain");

     // norm
     fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr] + "norm");

   }
       
   // --- Const Freq rtftwalk ---

   if(fUserData.DoZipAlgorithm(fDetNum, "CalcConstFreqRTFTWalkRRQ")  &&
      fIOMan.CheckBatRootPhononAlg("ConstFreqRTFTWalkPhonon", fDetNum))
   {
     for(int chanItr = 0; chanItr < BatCalibTypes::kZIPFLIPNPhononChan; chanItr++)
     {
       fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"WKr10");
       fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"WKr20");
       fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"WKr40");
       fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"WKr50");
       fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"WKr70");
       fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"WKf80");

       if( !fIsFirstProduction ) //only stored in later production versions
       {
	 fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"WKr30");
	 fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"WKr80");
       }
     } // end loop over channels

     //total phonon walk times
     if(fUserData.DoZipAlgorithm(fDetNum, "TotalPhonon"))
     {
       fIOMan.Activate("PTWKr10");
       fIOMan.Activate("PTWKr20");
       fIOMan.Activate("PTWKr30");
       fIOMan.Activate("PTWKr40");
       fIOMan.Activate("PTWKr50");
       fIOMan.Activate("PTWKr70");
       fIOMan.Activate("PTWKr80");
       fIOMan.Activate("PTWKf80");	 
     }
     
   } //endif do Const Freq RTFTWalk


   return;

}

// ================== Calculations ======================
//
// DoCalibration controls looping over events!
//
// ======================================================

void GenRRQDatamZIP::DoCalibration(int maxEvents, int detNum, UserDataManager& myUserData)
{

   //FIXME - pass in the detector type from BatCalib main
   cout <<"Hello from GenRRQDatamZIP::DoCalibration!" << endl;

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
   fChargePosX = fUserData.GetVectDoubleParameter(fDetNum, "Q_POS_XCORRECTION");   
   fChargePosY = fUserData.GetVectDoubleParameter(fDetNum, "Q_POS_YCORRECTION");   
   fXFitMinMax = fUserData.GetVectDoubleParameter(fDetNum, "Q_XFIT_MINMAX"); //for position correction   
   fXFitMin = fXFitMinMax[0] - 10; //Note hardcoded shift! FIXME
   fXFitMax = fXFitMinMax[1] + 10; //Note hardcoded shift! FIXME
   fYFitMinMax = fUserData.GetVectDoubleParameter(fDetNum, "Q_YFIT_MINMAX"); //for position correction  
   fYFitMin = fYFitMinMax[0] - 6; //Note hardcoded shift! FIXME
   fYFitMax = fYFitMinMax[1] + 6; //Note hardcoded shift! FIXME

   fPhononCal = fUserData.GetVectDoubleParameter(fDetNum, "P_CALIBRATION_OF"); //for optimal filter
   fEpsilon = fUserData.GetDoubleParameter(fDetNum, "EPSILON"); 

   fOFMaxTemplate = fUserData.GetVectDoubleParameter(fDetNum, "OF_MAX_TEMPLATE");  //FIXME when this is available in RQ files

   //integral calibration only available in later production versions
   if( !fIsFirstProduction)
   {
      fPhononIntCal = fUserData.GetVectDoubleParameter(fDetNum, "P_CALIBRATION_INT"); 
   }


   // --- 5. Some checks before performing calculations ----

   if(fChargePosX.size() != 6 || fChargePosY.size() != 6) 
   {
      cout <<"CalibrationZipData::DoCalc ERROR! charge position correction vectors have the incorrect length." << endl;
   }

   if(fPhononCal.size() != (uint)BatCalibTypes::kZIPFLIPNPhononChan)
   {
      cout <<"GenRRQDatamZIP::DoCalibration ERROR! "
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
      

      // ---- optional calculations ----
      
      // find primary channel
      FindPrimaryPhononChannel();  //most optional calculations need this
      
      // ConstFreqRTFTWalk timing
      if(fUserData.DoZipAlgorithm(fDetNum, "CalcConstFreqRTFTWalkRRQ"))
 	 CalcConstFreqRTFTWalkRRQ(); 
      

      // --- Store some misc items ---
      
      fRRQList["DetType"] = fDetType; //inefficient, but needed for pull teeth right now
      fRRQList["pprimechanOFWK"] = fPrimaryPhononChan + 1; //1 to offset c++ and matlab conventions 
      fRRQList["pprimechan"] = fRRQList["pprimechanOFWK"]; 
      
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


void GenRRQDatamZIP::ApplyPhononCalibration()
{
   if(fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon", fDetNum))
   {
     //Optimal Filter energy
     fRRQList["paOF"] = fPhononCal[0]*fIOMan.Get("PAOFamps");
     fRRQList["pbOF"] = fPhononCal[1]*fIOMan.Get("PBOFamps");
     fRRQList["pcOF"] = fPhononCal[2]*fIOMan.Get("PCOFamps");
     fRRQList["pdOF"] = fPhononCal[3]*fIOMan.Get("PDOFamps");

     fRRQList["paOF0"] = fPhononCal[0]*fIOMan.Get("PAOFamps0");
     fRRQList["pbOF0"] = fPhononCal[1]*fIOMan.Get("PBOFamps0");
     fRRQList["pcOF0"] = fPhononCal[2]*fIOMan.Get("PCOFamps0");
     fRRQList["pdOF0"] = fPhononCal[3]*fIOMan.Get("PDOFamps0");
   }

   if(fIOMan.CheckBatRootPhononAlg("PulseIntegral", fDetNum))
   {
     //Phonon integral energy
     fRRQList["paINT"] = fPhononIntCal[0]*fIOMan.Get("PAINTall");
     fRRQList["pbINT"] = fPhononIntCal[1]*fIOMan.Get("PBINTall");
     fRRQList["pcINT"] = fPhononIntCal[2]*fIOMan.Get("PCINTall");
     fRRQList["pdINT"] = fPhononIntCal[3]*fIOMan.Get("PDINTall");
   }

   return;
}


void GenRRQDatamZIP::CalcPhononDelays()
{
   double pxdel = -999999;
   double pydel = -999999;


   if(fIOMan.CheckBatRootPhononAlg("ConstFreqRTFTWalkPhonon", fDetNum))
   {   
     pxdel = -(fIOMan.Get("PBWKr20")*cos(kThetaVect[0]) + fIOMan.Get("PCWKr20")*cos(kThetaVect[1]) 
	       + fIOMan.Get("PDWKr20")*cos(kThetaVect[2]))*1e6;
   
     pydel = -(fIOMan.Get("PBWKr20")*sin(kThetaVect[0]) + fIOMan.Get("PCWKr20")*sin(kThetaVect[1])
	       + fIOMan.Get("PDWKr20")*sin(kThetaVect[2]))*1e6;
   
      
     //  ===== Store the values =====
     
     fRRQList["pxdelWK"] = pxdel;
     fRRQList["pydelWK"] = pydel;

   }

   return;
} 

void GenRRQDatamZIP::ApplyChargeCalibration()
{

   // can't do this if no charge calibration
   if(!fIOMan.CheckBatRootChargeAlg("OptimalFilterChargeX", fDetNum) &&
      !fIOMan.CheckBatRootChargeAlg("OptimalFilterCharge", fDetNum))
   {
     return;
   }

   //to hold calibration constants
   double qia = fChargeCal[0];
   double qoa = fChargeCal[1];
   double qix = fChargeCal[2];
   double qox = fChargeCal[3];

   double qi = -999999;
   double qo = -999999;
   double qi0 = -999999;
   double qo0 = -999999;

   //1.  Just do simple cross talk calc for qi0 and qo0
   qo0 = qoa*(fIOMan.Get("QOOFvolts0") + qox*fIOMan.Get("QIOFvolts0"));
   qi0 = qia*(fIOMan.Get("QIOFvolts0") + qix*fIOMan.Get("QOOFvolts0"));


   //2.  Check for saturation (either QI or QO)  
   int isSat = ( (fIOMan.Get("QIsat")> 0 || fIOMan.Get("QOsat")>0) ? 1 : 0);


   //Note: the overall scale factor for unsaturated Ge events is applied by the position correction
   //FIXME - change this for R132 - its *very* confusing

   if(isSat != 1) 
   {
      // ==== Apply cross talk correction ====

      //QO for all
      qo = qoa*(fIOMan.Get("QOOFvolts") + qox*fIOMan.Get("QIOFvolts"));

      //If mercedes or silicon (i.e. no position correction correction)
      if(fDetType == BatCalibTypes::kmZIPDetType || fIsSi)
      {
	 //QI for silicon and mercedes applies amplitude here.   
	 qi = qia*(fIOMan.Get("QIOFvolts") + qix*fIOMan.Get("QOOFvolts"));
      }
      else //do the position correction only Ge CDMS II 
      {

	 //QI for Ge mZIP detectors applies amplitude in position correction
	 qi = (fIOMan.Get("QIOFvolts") + qix*fIOMan.Get("QOOFvolts"));


	 // ===== Apply charge position correction (only Ge CDMS II) =====
      
	 double xPosFactor = 356.;
	 double yPosFactor = 356.;
	 double pxdel = fRRQList["pxdelWK"];
	 double pydel = fRRQList["pydelWK"];

	 //short cut to using pow(), which is slow
	 double pxdel2 = pxdel*pxdel;
	 double pxdel3 = pxdel*pxdel2;
	 double pxdel4 = pxdel*pxdel3;
	 double pxdel5 = pxdel*pxdel4;

	 double pydel2 = pydel*pydel;
	 double pydel3 = pydel*pydel2;
	 double pydel4 = pydel*pydel3;
	 double pydel5 = pydel*pydel4;
	 
	 // --- compute the x correction factor ---

	 if( pxdel <= fXFitMax && pxdel >= fXFitMin)
	 {
//	    cout <<"In x range!" << endl;

	    xPosFactor /= (fChargePosX[0]*pxdel5 +  fChargePosX[1]*pxdel4 + fChargePosX[2]*pxdel3 + 
			   fChargePosX[3]*pxdel2 + fChargePosX[4]*pxdel + fChargePosX[5]);
	 }
	 else 
	 {
	    double val = ( pxdel < fXFitMin ? fXFitMin : fXFitMax);
	    double val2 = val*val;
	    double val3 = val*val2;
	    double val4 = val*val3;
	    double val5 = val*val4;

//	    cout <<"Out of x range!" << endl;

	    xPosFactor /= (fChargePosX[0]*val5 +  fChargePosX[1]*val4 + fChargePosX[2]*val3 + 
			   fChargePosX[3]*val2 + fChargePosX[4]*val + fChargePosX[5]);

	 }


	 // --- compute the y correction factor ---

	 //contains the amplitude norm also
	 if( pydel <= fYFitMax && pydel >= fYFitMin)
	 {
//	    cout <<"In y range!" << endl;

	    yPosFactor /= (fChargePosY[0]*pydel5 +  fChargePosY[1]*pydel4 + fChargePosY[2]*pydel3 + 
			   fChargePosY[3]*pydel2 + fChargePosY[4]*pydel + fChargePosY[5]); 
	 }
	 else
	 {
	    double val = ( pydel < fYFitMin ? fYFitMin : fYFitMax );
	    double val2 = val*val;
	    double val3 = val*val2;
	    double val4 = val*val3;
	    double val5 = val*val4;

//	    cout <<"Out of y range!" << endl;

	    yPosFactor /= (fChargePosY[0]*val5 +  fChargePosY[1]*val4 + fChargePosY[2]*val3 + 
			   fChargePosY[3]*val2 + fChargePosY[4]*val + fChargePosY[5]); 

	 }

	 // --- apply the position corrections ---

	 qi *= xPosFactor*yPosFactor;

      } //end if Ge mZIP

   }//end if not saturated

   else //now, take care of the saturated pulses 
   {
      //apply cross talk and overall scale factor (but no position correction)

      //do F5 by default, if it doesn't exist then enter -999999 w/ a warning!
      if(fUserData.DoZipAlgorithm(fDetNum, "CalcF5SatEnergy"))
      {
	 qi = qia*(fIOMan.Get("QIF5volts") + qix*fIOMan.Get("QOF5volts"));
	 qo = qoa*(fIOMan.Get("QOF5volts") + qox*fIOMan.Get("QIF5volts"));
      }
      else
      {
	 cout <<"WARNING from GenRRQDatamZIP::ApplyChargeCalibration!  There are no F5 rq's for calculating saturated charge energy, reverting to null value for energy!"
	      << endl;
      }

   }


   // ===== Store the values now =====

   fRRQList["qiOF"] = qi;
   fRRQList["qoOF"] = qo;

   fRRQList["qiOF0"] = qi0;
   fRRQList["qoOF0"] = qo0;

   return;
}

void GenRRQDatamZIP::CalcTotalEnergiesAndYields()
{

   //get qi/qo bias - they are used for all the detector below
   double qiBias = -999999;
   double qoBias = -999999;

   if(fUserData.GetIntParameter("OVERRIDE_BIAS_WCONFIG") == 0)
   {
      qiBias = fIOMan.Get("QIbias"); 
      qoBias = fIOMan.Get("QObias");
   }
   else
   {
      qiBias = fUserData.GetDoubleParameter(fDetNum, "QIBIAS");
      qiBias = fUserData.GetDoubleParameter(fDetNum, "QOBIAS");
   }

   double qifac = fabs(qiBias/fEpsilon); //should always be positive?

   // === total ionization energy ===
   int isChargeOn = 1;

   if(fIOMan.CheckBatRootChargeAlg("OptimalFilterChargeX", fDetNum) ||
      fIOMan.CheckBatRootChargeAlg("OptimalFilterCharge", fDetNum))
   {
     isChargeOn = 1;
     fRRQList["qsumOF"] = fRRQList["qiOF"] + fRRQList["qoOF"];
   }


   // === optimal filter quantities ===
   
   if(fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon", fDetNum))
   {

     // total phonon energy
   
     double psum = fRRQList["paOF"] + fRRQList["pbOF"] + fRRQList["pcOF"] + fRRQList["pdOF"];
     fRRQList["psumOF"] = psum;
     fRRQList["psumiOF"] = fRRQList["pbOF"] + fRRQList["pcOF"] + fRRQList["pdOF"]; 
     fRRQList["psumoOF"] = fRRQList["pbOF"] + fRRQList["pcOF"] + fRRQList["pdOF"]; 
      
     // recoil energies
   
     fRRQList["precoilgOF"] = psum/(1.0+qifac); 
     
     if(isChargeOn)
     {
       fRRQList["precoilsumOF"] = psum - (fRRQList["qsumOF"]*qifac);
       fRRQList["precoiliOF"] = psum - (fRRQList["qiOF"]*qifac); 
     }      

     // yields
     
     if(isChargeOn)
     {
       fRRQList["ysumOF"] = fRRQList["qsumOF"]/fRRQList["precoilsumOF"];
       fRRQList["ygsumOF"] = fRRQList["qsumOF"]/fRRQList["precoilgOF"];
       fRRQList["ysumiOF"] = fRRQList["qiOF"]/fRRQList["precoiliOF"];
       fRRQList["ygsumiOF"] = fRRQList["qiOF"]/fRRQList["precoilgOF"];
     }

   }


   // === integral quantities ===

   if(fIOMan.CheckBatRootPhononAlg("PulseIntegral", fDetNum))
   {
     // total phonon energy
   
     double psumINT = fRRQList["paINT"] + fRRQList["pbINT"] + fRRQList["pcINT"] + fRRQList["pdINT"];
     fRRQList["psumINT"] = psumINT;
     fRRQList["psumiINT"] = fRRQList["pbINT"] + fRRQList["pcINT"] + fRRQList["pdINT"]; 
     fRRQList["psumoINT"] = fRRQList["pbINT"] + fRRQList["pcINT"] + fRRQList["pdINT"]; 	 
     
     // recoil energies
     
     fRRQList["precoilgsumINT"] = psumINT/(1.0+qifac); 

     if(isChargeOn)
     {
       fRRQList["precoilsumINT"] = psumINT - (fRRQList["qsumOF"]*qifac);
       fRRQList["precoilsumiINT"] = psumINT - (fRRQList["qiOF"]*qifac); 
     }     

     // yields

     if(isChargeOn)
     {
       fRRQList["ysumINT"] = fRRQList["qsumOF"]/fRRQList["precoilsumINT"];
       fRRQList["ygsumINT"] = fRRQList["qsumOF"]/fRRQList["precoilgsumINT"];
       fRRQList["ysumiINT"] = fRRQList["qiOF"]/fRRQList["precoilsumiINT"];
       fRRQList["ygsumiINT"] = fRRQList["qiOF"]/fRRQList["precoilgsumINT"];
     }

   }
    
    
   return;
}


//mostly just for convenience
void GenRRQDatamZIP::CalcPartitions()
{

  // === Charge partitions ===

  if(fIOMan.CheckBatRootChargeAlg("OptimalFilterChargeX", fDetNum) ||
     fIOMan.CheckBatRootChargeAlg("OptimalFilterCharge", fDetNum))
  {
    fRRQList["qrpartOF"] =  fRRQList["qoOF"]/fRRQList["qsumOF"];
  }

  // === optimal filter quantities ===

  if(fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon", fDetNum) )
  {
    // partition for mercedes (triangle plots)
    fRRQList["pxpartOF"] = (fRRQList["pbOF"]*cos(kThetaVect[0]) + fRRQList["pcOF"]*cos(kThetaVect[1]) + 
			    fRRQList["pdOF"]*cos(kThetaVect[2])) 
                         / (fRRQList["pbOF"] + fRRQList["pcOF"] + fRRQList["pdOF"]);
  
    fRRQList["pypartOF"] = (fRRQList["pbOF"]*sin(kThetaVect[0]) + fRRQList["pcOF"]*sin(kThetaVect[1]) + 
			    fRRQList["pdOF"]*sin(kThetaVect[2])) 
                         / (fRRQList["pbOF"] + fRRQList["pcOF"] + fRRQList["pdOF"]);
  
    fRRQList["prxypartOF"] = sqrt(fRRQList["pxpartOF"]*fRRQList["pxpartOF"] + 
				  fRRQList["pypartOF"]*fRRQList["pypartOF"]);

    // inner/outer partition
    fRRQList["prpartOF"] = (fRRQList["paOF"] - (fRRQList["pbOF"] + fRRQList["pcOF"] + fRRQList["pdOF"]))                         / fRRQList["psumOF"];
    
  }

  // === integral quantities ===

  if(fIOMan.CheckBatRootPhononAlg("OptimalFilterPhonon", fDetNum) )
  {
    fRRQList["pxpartINT"] = (fRRQList["pbINT"]*cos(kThetaVect[0]) + fRRQList["pcINT"]*cos(kThetaVect[1]) + 
			     fRRQList["pdINT"]*cos(kThetaVect[2])) 
                          / (fRRQList["pbINT"] + fRRQList["pcINT"] + fRRQList["pdINT"]);
  
    fRRQList["pypartINT"] = (fRRQList["pbINT"]*sin(kThetaVect[0]) + fRRQList["pcINT"]*sin(kThetaVect[1]) + 
			     fRRQList["pdINT"]*sin(kThetaVect[2])) 
                          / (fRRQList["pbINT"] + fRRQList["pcINT"] + fRRQList["pdINT"]);
  
    fRRQList["prxypartINT"] = sqrt(fRRQList["pxpartINT"]*fRRQList["pxpartINT"] + 
				   fRRQList["pypartINT"]*fRRQList["pypartINT"]);


    fRRQList["prpartINT"] = (fRRQList["paINT"] - (fRRQList["pbINT"] + fRRQList["pcINT"] + fRRQList["pdINT"])) 
                           / fRRQList["psumINT"];

  }


  return;
}
     
//calibrate the OF resolution quantities
void GenRRQDatamZIP::CalcOFResolutions()
{

   // === calibrate the OFdelay and OFamplitude resolutions ===
   //      note values stored in order QI, QO, PA, PB, ...

   double tempQIMax = fOFMaxTemplate[0];
   double tempQOMax = fOFMaxTemplate[1];

   fRRQList["qidelayres"] = fChargeCal[0]*tempQIMax*fDelaySig[0];
   fRRQList["qodelayres"] = fChargeCal[1]*tempQOMax*fDelaySig[1];
      
   fRRQList["qiampres"] = fChargeCal[0]*tempQIMax*fAmpSig[0];
   fRRQList["qoampres"] = fChargeCal[1]*tempQOMax*fAmpSig[1];

   // loop over phonon channels and calculae for each
   for(int chanItr=0; chanItr < BatCalibTypes::kZIPFLIPNPhononChan; chanItr++)
   {
      string prefix = BatCalibTypes::kZIPFLIPPhononChan[chanItr];
      string prefixCal = BatCalibTypes::kZIPFLIPPhononCal[chanItr];

      fRRQList[prefixCal+"delayres"] = ( fIOMan.Get(prefix + "gain")/fIOMan.Get(prefix + "norm") )*fPhononCal[chanItr]*fDelaySig[chanItr+2];  
      fRRQList[prefixCal+"ampres"] = ( fIOMan.Get(prefix + "gain")/fIOMan.Get(prefix + "norm") )*fPhononCal[chanItr]*fAmpSig[chanItr+2]; 

   }

   return;
}

// ==============================  Optional RRQ calculations ==============================================

//Assume calculation is done w/ const freq rtftwalk, rrq name should reflect that
//FIXME this needs some revision to incorporate flexibility of OF/INT 
void GenRRQDatamZIP::FindPrimaryPhononChannel()
{

   //If these rq's don't exist, then skip the rest
   if(!fIOMan.CheckBatRootPhononAlg("ConstFreqRTFTWalkPhonon", fDetNum))
   {
     return;
   }

   // --- For mercedes, primary is the biggest if its one of the inner channels, ---
   // --- if the outer channel has the biggest amplitude, then take the largest inner channel if its delay is faster ---

   // 1.  First look for primary by the amplitude

   double maxAmp = -1.0*numeric_limits<double>::infinity();
   
   //loop over channels to find max amplitude
   for(int chanItr = 0; chanItr < BatCalibTypes::kZIPFLIPNPhononChan; chanItr++)
   {
     string tempVarName = BatCalibTypes::kZIPFLIPPhononCal[chanItr];
     double tempAmp =  fRRQList[tempVarName];

     if(tempAmp > maxAmp)
     {
       
       //save new value in first place
       maxAmp = tempAmp;
       fPrimaryPhononChan = chanItr;
     }
	 
   } //end loop over phonon channels


   // 2. loop over inner channels to find max amplitude - needed for prdel and primary by delay calc
   double secondaryAmp = -1.0*numeric_limits<double>::infinity(); //second to biggest amplitude
   
   for(int chanItr = 1; chanItr < BatCalibTypes::kZIPFLIPNPhononChan; chanItr++)
   {
     string tempVarName = BatCalibTypes::kZIPFLIPPhononCal[chanItr];
     double tempAmp =  fRRQList[tempVarName];
	    
     if(tempAmp > secondaryAmp)
     {
       //save new value 
       secondaryAmp = tempAmp;
       fPrimaryInnerPhononChan = chanItr;
     }
	 
   } //end loop over inner phonon channels


   // 3. Check whether max amplitude is the outer phonon channel, 
   // if so check whether largest of inner channels has faster delay than outer
   if(fPrimaryPhononChan == 0)
   {
     //retrieve the risetimes
     double primaryRT;
     double secondaryRT;
     
     primaryRT = fIOMan.Get(BatCalibTypes::kZIPFLIPPhononChan[fPrimaryPhononChan] + "WKr20"); 	 
     secondaryRT = fIOMan.Get(BatCalibTypes::kZIPFLIPPhononChan[fPrimaryInnerPhononChan] + "WKr20"); 

     //make the inner channel primary if it has a faster risetime
     if(secondaryRT < primaryRT)
     {
       fPrimaryPhononChan = fPrimaryInnerPhononChan;
     }

   }//end if max amplitude is outer


   return;
}
     
void GenRRQDatamZIP::CalcConstFreqRTFTWalkRRQ()
{

   if(fPrimaryPhononChan == -999999)
   {
      cout <<"ERROR GenRRQDatamZIP::CalcConstFreqRTFTWalkRRQ! Primary channel needs to be set first, please check code!" 
	   << endl;
      exit(1);
   }

   //If these rq's don't exist, then skip the rest
   if(!fIOMan.CheckBatRootPhononAlg("ConstFreqRTFTWalkPhonon", fDetNum))
   {
     return;
   }

   // --- primary channel rrq's  ---
   
   string primChanName = BatCalibTypes::kZIPFLIPPhononChan[fPrimaryPhononChan];

   fRRQList["pqdelWK"] = fIOMan.Get(primChanName+"WKr20")*1e6 - (511.5*0.8 + fIOMan.Get("QSOFdelay")*1e6);
   fRRQList["pminrtWK_1040"] = (fIOMan.Get(primChanName+"WKr40") - fIOMan.Get(primChanName+"WKr10"))*1e6; //in microseconds
   fRRQList["pminrtWK_4070"] = (fIOMan.Get(primChanName+"WKr70") - fIOMan.Get(primChanName+"WKr40"))*1e6; //in microseconds

   if( !fIsFirstProduction )
   {
      fRRQList["pminrtWK_1030"] = (fIOMan.Get(primChanName+"WKr30") - fIOMan.Get(primChanName+"WKr10"))*1e6; //in microseconds
      fRRQList["pminrtWK_3050"] = (fIOMan.Get(primChanName+"WKr50") - fIOMan.Get(primChanName+"WKr30"))*1e6; //in microseconds
      fRRQList["pminrtWK_5080"] = (fIOMan.Get(primChanName+"WKr80") - fIOMan.Get(primChanName+"WKr50"))*1e6; //in microseconds
      fRRQList["ptopwidthWK"] = (fIOMan.Get(primChanName+"WKf80") - fIOMan.Get(primChanName+"WKr80"))*1e6; //in microseconds
   }


   // --- total phonon pulse rrq's  ---
   
   if(fUserData.DoZipAlgorithm(fDetNum, "TotalPhonon"))
   {
     fRRQList["ptqdelWK"] = fIOMan.Get("PTWKr20")*1e6 - (511.5*0.8 + fIOMan.Get("QSOFdelay")*1e6);
     fRRQList["ptrtWK_1040"] = (fIOMan.Get("PTWKr40") - fIOMan.Get("PTWKr10"))*1e6; //in microseconds
   }

   //find the primary inner channel by amplitude
   fRRQList["prdelWK"] = (fIOMan.Get(BatCalibTypes::kZIPFLIPPhononChan[fPrimaryInnerPhononChan] + "WKr20") 
			  - fIOMan.Get("PAWKr20"))*1e6;


   return;
}




