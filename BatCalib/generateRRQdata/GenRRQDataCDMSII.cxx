///////////////////////////////////////////////////////////////////////////////// 
//Class Name: GenRRQDataCDMSII
//Authors: L. Hsu
//Description: Applies charge calibration (cross talk, amplitude scaling, position 
//correction).  Applies phonon calibration (relative amplitude scaling).  Generates 
//primary rrq's for analysis (xy-delays, partitions, yields, timing quantities. This 
//is analogous to the cdmspipe "PC_first_pass.m" script.   Imported from cdmspipe 
//and upgraded to be compatible (and backwards compatible) with hybrid mercedes/CDMSII 
//running
//
//File Import By: L. Hsu
//Creation Date: Dec. 23, 2009 
//
//Modifications:
//
//Jan 2011 (LLH):  This file was renamed from GenerateRRQData to it current name, 
//                 GenRRQDataCDMSII.  Event-level rrq's removed from this class 
//                 and placed into new class GenRRQDataEvent
// 
////////////////////////////////////////////////////////////////////////////////// 

#include <iostream>
#include <math.h>
#include <limits>

#include "TMath.h"

#include "GenRRQDataCDMSII.h"
#include "BatCalibTypes.h"

////////////////////////////////////////////////////////
//
// Endcap Channel Mapping:
// top - qi, pc, pd
// bottom - qo, pa, pb
//
////////////////////////////////////////////////////////

GenRRQDataCDMSII::GenRRQDataCDMSII(BatCalibIOManager ioManager) :
   fIOMan(ioManager),
   fDetType(-999999),
   fPrimaryPhononChan(-999999),
   fPrimaryInnerPhononChan(-999999),
   fIsFirstProduction(0)
{
//   cout <<"Hello from GenRRQDataCDMSII! " << endl;

   kThetaVect[0] = 150.*(TMath::Pi()/180.);
   kThetaVect[1] = 270.*(TMath::Pi()/180.);
   kThetaVect[2] = 30.*(TMath::Pi()/180.);

}

GenRRQDataCDMSII::~GenRRQDataCDMSII()
{
//   cout <<"Goodbye from GenRRQDataCDMSII()" << endl;
}


//initialize desired output variables here (this happens before looping over events in the zip trees)
void GenRRQDataCDMSII::ConstructRRQList()
{
     double initVal = -999999.;

     //construct the RRQ list here.  Note anything added outside of here 
     //is not going to be in the output

     fRRQList.insert(pair<string,double>("Empty", initVal));
     fRRQList.insert(pair<string,double>("DetType", initVal));
     fRRQList.insert(pair<string,double>("prim_chan", initVal));

     //energies and yields

     fRRQList.insert(pair<string,double>("pa", initVal));
     fRRQList.insert(pair<string,double>("pb", initVal));
     fRRQList.insert(pair<string,double>("pc", initVal));
     fRRQList.insert(pair<string,double>("pd", initVal));

     fRRQList.insert(pair<string,double>("pa0", initVal));
     fRRQList.insert(pair<string,double>("pb0", initVal));
     fRRQList.insert(pair<string,double>("pc0", initVal));
     fRRQList.insert(pair<string,double>("pd0", initVal));

     if(fDetType != BatCalibTypes::kDualEndcapDetType)
     {
	fRRQList.insert(pair<string,double>("qi", initVal));
	fRRQList.insert(pair<string,double>("qo", initVal));
	fRRQList.insert(pair<string,double>("qi0", initVal));
	fRRQList.insert(pair<string,double>("qo0", initVal));
	fRRQList.insert(pair<string,double>("qsum", initVal));

	fRRQList.insert(pair<string,double>("pt", initVal));
	fRRQList.insert(pair<string,double>("prg", initVal)); 
	fRRQList.insert(pair<string,double>("pr", initVal));
	fRRQList.insert(pair<string,double>("pri", initVal));
	
	fRRQList.insert(pair<string,double>("y", initVal));
	fRRQList.insert(pair<string,double>("yg", initVal));
	fRRQList.insert(pair<string,double>("yi", initVal));
	fRRQList.insert(pair<string,double>("ygi", initVal));

	//integral quantities only for later production data
	if( !fIsFirstProduction )
	{
	   fRRQList.insert(pair<string,double>("pa_int", initVal));
	   fRRQList.insert(pair<string,double>("pb_int", initVal));
	   fRRQList.insert(pair<string,double>("pc_int", initVal));
	   fRRQList.insert(pair<string,double>("pd_int", initVal));
	   
	   fRRQList.insert(pair<string,double>("pt_int", initVal));
	   fRRQList.insert(pair<string,double>("prg_int", initVal));
	   fRRQList.insert(pair<string,double>("pr_int", initVal));
	   fRRQList.insert(pair<string,double>("pri_int", initVal));
	   
	   fRRQList.insert(pair<string,double>("y_int", initVal));
	   fRRQList.insert(pair<string,double>("yg_int", initVal));
	   fRRQList.insert(pair<string,double>("yi_int", initVal));
	   fRRQList.insert(pair<string,double>("ygi_int", initVal));
	}

     } //endif not endcap

     else //for endcaps
     {
	fRRQList.insert(pair<string,double>("qtop", initVal));
	fRRQList.insert(pair<string,double>("qbottom", initVal));
	fRRQList.insert(pair<string,double>("qtop0", initVal));
	fRRQList.insert(pair<string,double>("qbottom0", initVal));

	fRRQList.insert(pair<string,double>("pttop", initVal));
	fRRQList.insert(pair<string,double>("ptbottom", initVal));

	fRRQList.insert(pair<string,double>("prtop", initVal));
	fRRQList.insert(pair<string,double>("prbottom", initVal));
	
	fRRQList.insert(pair<string,double>("ytop", initVal));
	fRRQList.insert(pair<string,double>("ybottom", initVal));

	//no need to check for first production tag b/c 
	//we cannot process endcaps with old production versions

 	fRRQList.insert(pair<string,double>("pa_int", initVal));
 	fRRQList.insert(pair<string,double>("pb_int", initVal));
 	fRRQList.insert(pair<string,double>("pc_int", initVal));
 	fRRQList.insert(pair<string,double>("pd_int", initVal));

 	fRRQList.insert(pair<string,double>("pttop_int", initVal));
 	fRRQList.insert(pair<string,double>("ptbottom_int", initVal));

 	fRRQList.insert(pair<string,double>("prtop_int", initVal));
 	fRRQList.insert(pair<string,double>("prbottom_int", initVal));

 	fRRQList.insert(pair<string,double>("ytop_int", initVal));
 	fRRQList.insert(pair<string,double>("ybottom_int", initVal));

     } //endif endcap


     //phonon delays
     
     if(fDetType != BatCalibTypes::kDualEndcapDetType)
     {
	fRRQList.insert(pair<string,double>("xdel", initVal));
	fRRQList.insert(pair<string,double>("ydel", initVal));
     }
     else
     {
	fRRQList.insert(pair<string,double>("deltop", initVal));
	fRRQList.insert(pair<string,double>("delbottom", initVal));
     }

     //partitions

     if(fDetType != BatCalibTypes::kDualEndcapDetType)
     {
	fRRQList.insert(pair<string,double>("xppart", initVal));
	fRRQList.insert(pair<string,double>("yppart", initVal));
	fRRQList.insert(pair<string,double>("rppart", initVal));
	fRRQList.insert(pair<string,double>("qpart", initVal));

	if(fDetType == BatCalibTypes::kmZIPDetType)
	{
	   fRRQList.insert(pair<string,double>("ppart_io", initVal));
	   fRRQList.insert(pair<string,double>("ppart_io_int", initVal));
	}

     }
     else
     {
	fRRQList.insert(pair<string,double>("pparttop", initVal));
	fRRQList.insert(pair<string,double>("ppartbottom", initVal));
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

     if(fDetType != BatCalibTypes::kDualEndcapDetType)
     {
	fRRQList.insert(pair<string,double>("qiampres", initVal));
	fRRQList.insert(pair<string,double>("qoampres", initVal));
	fRRQList.insert(pair<string,double>("qidelayres", initVal));
	fRRQList.insert(pair<string,double>("qodelayres", initVal));
     }
     else
     {
	fRRQList.insert(pair<string,double>("qtopampres", initVal));
	fRRQList.insert(pair<string,double>("qbottomampres", initVal));
	fRRQList.insert(pair<string,double>("qtopdelayres", initVal));
	fRRQList.insert(pair<string,double>("qbottomdelayres", initVal));
     }

     // ---- RRQ's from optional calculations (don't perform these on endcaps) -----

     //RTFTWalk
     if(fUserData.DoZipAlgorithm(fDetNum, "CalcVarFreqRTFTWalkRRQ") && fDetType != BatCalibTypes::kDualEndcapDetType) 
     {
	//for primary channel
	fRRQList.insert(pair<string,double>("pminrt", initVal));
	fRRQList.insert(pair<string,double>("pdel", initVal));
	fRRQList.insert(pair<string,double>("pminrt4070", initVal));
	fRRQList.insert(pair<string,double>("pminrt1030", initVal));
	fRRQList.insert(pair<string,double>("pminrt3050", initVal));
	fRRQList.insert(pair<string,double>("pminrt5080", initVal));
	fRRQList.insert(pair<string,double>("ptopwidth", initVal));

	//for total phonon pulse (only for mercedes) 
	if(fDetType == BatCalibTypes::kmZIPDetType)
	{
	   fRRQList.insert(pair<string,double>("ptrt", initVal));
	   fRRQList.insert(pair<string,double>("ptdel", initVal));
	   fRRQList.insert(pair<string,double>("pdel_io", initVal));
	}

     }

     //ConstFreqRTFTWalk
     if(fUserData.DoZipAlgorithm(fDetNum, "CalcConstFreqRTFTWalkRRQ") && fDetType != BatCalibTypes::kDualEndcapDetType) 
     {
	//for primary channel
	fRRQList.insert(pair<string,double>("pminrtCF", initVal));
	fRRQList.insert(pair<string,double>("pdelCF", initVal));
	fRRQList.insert(pair<string,double>("pminrtCF4070", initVal));

	if( !fIsFirstProduction ) //only available in later production versions
	{
	   fRRQList.insert(pair<string,double>("pminrtCF1030", initVal));
	   fRRQList.insert(pair<string,double>("pminrtCF3050", initVal));
	   fRRQList.insert(pair<string,double>("pminrtCF5080", initVal));
	   fRRQList.insert(pair<string,double>("ptopwidthCF", initVal));
	}

	//for total phonon pulse, only needed for mercedes
	if(fDetType == BatCalibTypes::kmZIPDetType)
	{
	   fRRQList.insert(pair<string,double>("ptrtCF", initVal));
	   fRRQList.insert(pair<string,double>("ptdelCF", initVal));
	   fRRQList.insert(pair<string,double>("pdel_ioCF", initVal));
	}
     }

     //PipeFit
     if(fUserData.DoZipAlgorithm(fDetNum, "CalcPipeFitRRQ") && fDetType != BatCalibTypes::kDualEndcapDetType) 
     {
        
       fRRQList.insert(pair<string,double>("pcurv20", initVal));
       fRRQList.insert(pair<string,double>("pdelPF0", initVal));
       fRRQList.insert(pair<string,double>("pdelPF10", initVal));
       fRRQList.insert(pair<string,double>("pdelPF20", initVal));
       fRRQList.insert(pair<string,double>("prtPF020", initVal));
       fRRQList.insert(pair<string,double>("prtPF1040", initVal));
       
     }

     //Wedgefit
     if(fUserData.DoZipAlgorithm(fDetNum, "CalcWedgeFitRRQ") && fDetType != BatCalibTypes::kDualEndcapDetType) 
     {
	cout <<"Hello! please implement me!" << endl;
     }


    //Now tell the io manager to make an output tree based on this list    

    string treeName(Form("calibzip%d", fDetNum));
    fIOMan.ConstructOutputRRQTree(&fRRQList, treeName);

    return;
}

void GenRRQDataCDMSII::ResetRRQValues()
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
void GenRRQDataCDMSII::ActivateRQs()
{

   //Any RQ that is needed for calculation should be specified here
   //otherwise it is not read from the file
  
   //to first order, everything needs this
   fIOMan.Activate("Empty");

   //check whether this is older BatRoot data, some rq's non-existent for that data
   //no integral calibration constants for that data also
   fIsFirstProduction = fIOMan.DoesRQFilePredate("01-Dec-2009");
   
   //this RQ does not exist in data before R130
   if( fIsFirstProduction ) //prior to this, no DetType in RQ files
   {
      cout <<"WARNING: The RQ file predates a time when we stored the DetType!"
	   <<"\nDefaulting to DetType in the user settings file!" 
	   << endl;

      fDetType = fUserData.GetIntParameter(fDetNum, "DET_TYPE");
   }
   else
   {
      //Activate this branch
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

   if(fDetType == BatCalibTypes::kDualEndcapDetType || (fDetType == BatCalibTypes::kmZIPDetType && fDetNum == 3)) 
   {
      fIOMan.Activate("QIOFnoXvolts");
      fIOMan.Activate("QOOFnoXvolts");
      fIOMan.Activate("QIOFnoXvolts0");
      fIOMan.Activate("QOOFnoXvolts0");
   }

   //this extra if is to exclude cases where detType = -999999 for selective readout of endcaps
   if(fDetType == BatCalibTypes::kmZIPDetType || fDetType == BatCalibTypes::kZIPDetType || 
      fDetType == BatCalibTypes::kFLIPDetType || fDetType == BatCalibTypes::kBlipDetType)
   {
     fIOMan.Activate("QIOFvolts");
     fIOMan.Activate("QOOFvolts");
     fIOMan.Activate("QIOFvolts0");
     fIOMan.Activate("QOOFvolts0");
     fIOMan.Activate("QSOFdelay");
   }
   
   // --- F5 ---
   
   if(fUserData.DoZipAlgorithm(fDetNum, "CalcF5SatEnergy"))
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
      //optimal filter
      fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"OFamps");
      fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"OFamps0");

      //integral routine
      if( !fIsFirstProduction ) //only used for later production versions
	 fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"INTall");

      //gain
      fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr] + "gain");

      //norm
      fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr] + "norm");

   }
       

   // --- RTFTWalk ---
  

   for(int chanItr = 0; chanItr < BatCalibTypes::kZIPFLIPNPhononChan; chanItr++)
   {
      //the following two if statements are to ensure the right parameters are set for the delay calculation
      //user may choose to use either ConstFreqRTFTWalk (constant frequency) or RTFTWalk (variable frequency) 
      //the lines below effectively require that the user has stored at least one or the other of the two sets rtftwalk rq's
      if( fUserData.DoZipAlgorithm(fDetNum, "DefaultToConstFreqRTFTWalk") && !fUserData.DoZipAlgorithm(fDetNum, "CalcConstFreqRTFTWalkRRQ") )
      {
	 cout <<"ERROR! GenRRQDataCDMSII::ActivateRQs()   Inconsistent request for detector " << fDetNum
	      <<"!  \nRequesting to use ConstFreqRTFTWalk for default delay calculations but DoAlgorithm CalcConstFreqRTFTWalk is not activated for this detector!"
	      << endl;

	 exit(1);
      }

      if( !fUserData.DoZipAlgorithm(fDetNum, "DefaultToConstFreqRTFTWalk") && !fUserData.DoZipAlgorithm(fDetNum, "CalcVarFreqRTFTWalkRRQ") )
      {
	 cout <<"ERROR! GenRRQDataCDMSII::ActivateRQs()   Inconsistent request for detector " << fDetNum
	      <<"!  \nRequesting to use RTFTWalk for default delay calculations but DoAlgorithm CalcVarFreqRTFTWalk is not activated for this detector!"
	      << endl;

	 exit(1);
      }

      //optional for RTFTWalk timing paramters
      if(fUserData.DoZipAlgorithm(fDetNum, "CalcVarFreqRTFTWalkRRQ")) 
      {
	 fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"VWKr10");
	 fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"VWKr20");  
	 fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"VWKr30");
	 fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"VWKr40");
	 fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"VWKr50");
	 fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"VWKr70");
	 fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"VWKr80");
	 fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"VWKf80");
      }
	 
   } //end loop over phonon chan

   //only needed for mercedes and optional RTFTWalk calculation
   if(fUserData.DoZipAlgorithm(fDetNum, "CalcVarFreqRTFTWalkRRQ") && fDetType == BatCalibTypes::kmZIPDetType)
   {
      fIOMan.Activate("PTVWKr10");
      fIOMan.Activate("PTVWKr20");
      fIOMan.Activate("PTVWKr30");
      fIOMan.Activate("PTVWKr40");
      fIOMan.Activate("PTVWKr50");
      fIOMan.Activate("PTVWKr70");
      fIOMan.Activate("PTVWKr80");
      fIOMan.Activate("PTVWKf80");
   }


   // --- CFrtftwalk ---

   if(fUserData.DoZipAlgorithm(fDetNum, "CalcConstFreqRTFTWalkRRQ")) 
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
      }

      //only needed for mercedes
      if(fDetType == BatCalibTypes::kmZIPDetType)
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

   } //endif do ConstFreqRTFTWalk


   // --- PipeFitter (no endcap) ---

   if(fUserData.DoZipAlgorithm(fDetNum, "CalcPipeFitRRQ") && fDetType != BatCalibTypes::kDualEndcapDetType) 
   {
     for(int chanItr = 0; chanItr < BatCalibTypes::kZIPFLIPNPhononChan; chanItr++)
       {
	 fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"PFr0");
	 fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"PFr10");
	 fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"PFr20");
	 fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"PFr40");
	 fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"PFt0fit");
	 fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"PFtau");
	 fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"PFkappa");
	 fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"PFa1");
	 fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"PFa0");
	 fIOMan.Activate(BatCalibTypes::kZIPFLIPPhononChan[chanItr]+"PFrfeflag");
    
       }
   }


   // --- WedgeFit (no endcap) ---
   
   if(fUserData.DoZipAlgorithm(fDetNum, "CalcWedgeFitRRQ") && fDetType != BatCalibTypes::kDualEndcapDetType) 
   {
      cout <<"Hello!  please implement me!" << endl;
   }

   return;

}

// ================== Calculations ======================
//
// DoCalibration controls looping over events!
//
// ======================================================

void GenRRQDataCDMSII::DoCalibration(int maxEvents, int detNum, UserDataManager& myUserData)
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


   if(fDetType != BatCalibTypes::kZIPDetType && fDetType != BatCalibTypes::kmZIPDetType && 
      fDetType != BatCalibTypes::kDualEndcapDetType)
   {
     cout <<"GenRRQDataCDMSII::DoCalc ERROR!\n"  
	  <<"Detector type does not have a defined delay calculation"
	  << endl;
     
     exit(1);
   }

   if(fPhononCal.size() != (uint)BatCalibTypes::kZIPFLIPNPhononChan)
   {
      cout <<"GenRRQDataCDMSII::DoCalibration ERROR! "
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
      if(fDetType != BatCalibTypes::kDualEndcapDetType) 
	 FindPrimaryPhononChannel();  //most optional calculations need this, not necessary for endcaps
      

      // RTFTWalk timing
      if(fUserData.DoZipAlgorithm(fDetNum, "CalcVarFreqRTFTWalkRRQ") && fDetType != BatCalibTypes::kDualEndcapDetType) 
 	 CalcVarFreqRTFTWalkRRQ();   
      

      // ConstFreqRTFTWalk timing
      if(fUserData.DoZipAlgorithm(fDetNum, "CalcConstFreqRTFTWalkRRQ") && fDetType != BatCalibTypes::kDualEndcapDetType)
 	 CalcConstFreqRTFTWalkRRQ(); 
      

      // Pipefitter
      if(fUserData.DoZipAlgorithm(fDetNum, "CalcPipeFitRRQ") && fDetType != BatCalibTypes::kDualEndcapDetType)
 	 CalcPipeFitRRQ();
      

      // WedgeFit
      if(fUserData.DoZipAlgorithm(fDetNum, "CalcWedgeFitRRQ") && fDetType != BatCalibTypes::kDualEndcapDetType)
 	 CalcWedgeFitRRQ(); //FIXME no implementation yet  
      

      // --- Store some misc items ---
      
      fRRQList["DetType"] = fDetType; //inefficient, but needed for pull teeth right now
      fRRQList["prim_chan"] = fPrimaryPhononChan + 1; //1 to offset c++ and matlab conventions 
      
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


void GenRRQDataCDMSII::ApplyPhononCalibration()
{
   
   //Optimal Filter energy
   fRRQList["pa"] = fPhononCal[0]*fIOMan.Get("PAOFamps");
   fRRQList["pb"] = fPhononCal[1]*fIOMan.Get("PBOFamps");
   fRRQList["pc"] = fPhononCal[2]*fIOMan.Get("PCOFamps");
   fRRQList["pd"] = fPhononCal[3]*fIOMan.Get("PDOFamps");

   fRRQList["pa0"] = fPhononCal[0]*fIOMan.Get("PAOFamps0");
   fRRQList["pb0"] = fPhononCal[1]*fIOMan.Get("PBOFamps0");
   fRRQList["pc0"] = fPhononCal[2]*fIOMan.Get("PCOFamps0");
   fRRQList["pd0"] = fPhononCal[3]*fIOMan.Get("PDOFamps0");

   //Phonon integral energy (only in later production versions)
   if( !fIsFirstProduction )
   {
      fRRQList["pa_int"] = fPhononIntCal[0]*fIOMan.Get("PAINTall");
      fRRQList["pb_int"] = fPhononIntCal[1]*fIOMan.Get("PBINTall");
      fRRQList["pc_int"] = fPhononIntCal[2]*fIOMan.Get("PCINTall");
      fRRQList["pd_int"] = fPhononIntCal[3]*fIOMan.Get("PDINTall");
   }

   return;
}

//Note that the user can make a choice in whether constant frequency or variable frequency rtftwalk values are used for the delay calculations in this function.   
//This is done by setting the flag in the user settings file "DefaultToConstFreqRTFTWalk" 
void GenRRQDataCDMSII::CalcPhononDelays()
{
   double xdel = -999999;
   double ydel = -999999;

   // ===== for CDMSII style detectors =====

   if(fDetType == BatCalibTypes::kZIPDetType)
   {
      string minChan="";
      double minRT = numeric_limits<double>::infinity();

      //loop over channels to find minimum delay
      for(int chanItr = 0; chanItr < BatCalibTypes::kZIPFLIPNPhononChan; chanItr++)
      {
	 string tempChanName = BatCalibTypes::kZIPFLIPPhononChan[chanItr];
	 double tempRT;

	 //choosing which set of rtftwalk rq's to use
	 if( !fUserData.DoZipAlgorithm(fDetNum, "DefaultToConstFreqRTFTWalk"))
	 {
	    tempRT =  fIOMan.Get(tempChanName+"VWKr20");
	 }
	 else
	 {
	    tempRT =  fIOMan.Get(tempChanName+"WKr20");
	 }

	 if(tempRT < minRT)
	 {
	    minChan = tempChanName;
	    minRT = tempRT;
	 }
	 
      } //end loop over phonon channels

      // now calculate the delays

      if(minChan=="PA") 
      { 
	 if( !fUserData.DoZipAlgorithm(fDetNum, "DefaultToConstFreqRTFTWalk"))
	 {
	    xdel = (fIOMan.Get("PAVWKr20")-fIOMan.Get("PDVWKr20"))*1e6;
	    ydel = (-fIOMan.Get("PAVWKr20")+fIOMan.Get("PBVWKr20"))*1e6;
	 }
	 else
	 {
	    xdel = (fIOMan.Get("PAWKr20")-fIOMan.Get("PDWKr20"))*1e6;
	    ydel = (-fIOMan.Get("PAWKr20")+fIOMan.Get("PBWKr20"))*1e6;
	 }
      }
        
      if(minChan=="PB") 
      {  
	 if( !fUserData.DoZipAlgorithm(fDetNum, "DefaultToConstFreqRTFTWalk"))
	 {
	    xdel = (fIOMan.Get("PBVWKr20")-fIOMan.Get("PCVWKr20"))*1e6;
	    ydel = (fIOMan.Get("PBVWKr20")-fIOMan.Get("PAVWKr20"))*1e6; 
	 }
	 else
	 {
	    xdel = (fIOMan.Get("PBWKr20")-fIOMan.Get("PCWKr20"))*1e6;
	    ydel = (fIOMan.Get("PBWKr20")-fIOMan.Get("PAWKr20"))*1e6; 
	 }
      }
      
      if (minChan=="PC") 
      {
	 if( !fUserData.DoZipAlgorithm(fDetNum, "DefaultToConstFreqRTFTWalk"))
	 {
	    xdel = (-fIOMan.Get("PCVWKr20")+fIOMan.Get("PBVWKr20"))*1e6;
	    ydel = (fIOMan.Get("PCVWKr20")-fIOMan.Get("PDVWKr20"))*1e6; 	 
	 }
	 else
	 {
	    xdel = (-fIOMan.Get("PCWKr20")+fIOMan.Get("PBWKr20"))*1e6;
	    ydel = (fIOMan.Get("PCWKr20")-fIOMan.Get("PDWKr20"))*1e6; 	 
	 }
      }

      if (minChan=="PD") 
      {
	 if( !fUserData.DoZipAlgorithm(fDetNum, "DefaultToConstFreqRTFTWalk"))
	 {
	    xdel = (-fIOMan.Get("PDVWKr20")+fIOMan.Get("PAVWKr20"))*1e6;
	    ydel = (-fIOMan.Get("PDVWKr20")+fIOMan.Get("PCVWKr20"))*1e6; 
	 }
	 else
	 {
	    xdel = (-fIOMan.Get("PDWKr20")+fIOMan.Get("PAWKr20"))*1e6;
	    ydel = (-fIOMan.Get("PDWKr20")+fIOMan.Get("PCWKr20"))*1e6; 
	 }
      }
       
   } //end if ZIPFLIP


   // ===== for mercedes =====
   
   if(fDetType == BatCalibTypes::kmZIPDetType)
   {

      if( !fUserData.DoZipAlgorithm(fDetNum, "DefaultToConstFreqRTFTWalk"))
      {
	 xdel = -(fIOMan.Get("PBVWKr20")*cos(kThetaVect[0]) + fIOMan.Get("PCVWKr20")*cos(kThetaVect[1]) 
		  + fIOMan.Get("PDVWKr20")*cos(kThetaVect[2]))*1e6;
	 
	 ydel = -(fIOMan.Get("PBVWKr20")*sin(kThetaVect[0]) + fIOMan.Get("PCVWKr20")*sin(kThetaVect[1])
		  + fIOMan.Get("PDVWKr20")*sin(kThetaVect[2]))*1e6;
      }
      else
      {
	 xdel = -(fIOMan.Get("PBWKr20")*cos(kThetaVect[0]) + fIOMan.Get("PCWKr20")*cos(kThetaVect[1]) 
		  + fIOMan.Get("PDWKr20")*cos(kThetaVect[2]))*1e6;
	 
	 ydel = -(fIOMan.Get("PBWKr20")*sin(kThetaVect[0]) + fIOMan.Get("PCWKr20")*sin(kThetaVect[1])
		  + fIOMan.Get("PDWKr20")*sin(kThetaVect[2]))*1e6;
      }

   } //end if mercedes
   
   
   //  ===== for endcaps =====
   
   if(fDetType == BatCalibTypes::kDualEndcapDetType)
   {
      if( !fUserData.DoZipAlgorithm(fDetNum, "DefaultToConstFreqRTFTWalk"))
      {
	 fRRQList["deltop"] = (fIOMan.Get("PDVWKr20") - fIOMan.Get("PCVWKr20"))*1e6;
	 fRRQList["delbottom"] = (fIOMan.Get("PBVWKr20") - fIOMan.Get("PAVWKr20"))*1e6;
      }      
      else
      {
	 fRRQList["deltop"] = (fIOMan.Get("PDWKr20") - fIOMan.Get("PCWKr20"))*1e6;
	 fRRQList["delbottom"] = (fIOMan.Get("PBWKr20") - fIOMan.Get("PAWKr20"))*1e6;
      }      

   } //end if endcap
   
   
   //  ===== Store the values =====
   
   fRRQList["xdel"] = xdel;
   fRRQList["ydel"] = ydel;

   return;
} 

void GenRRQDataCDMSII::ApplyChargeCalibration()
{

   //to hold calibration constants
   double qia = fChargeCal[0];
   double qoa = fChargeCal[1];
   double qix = fChargeCal[2];
   double qox = fChargeCal[3];


   // ===== First do endcaps =====

   //same calculation whether saturated or not because F5 does not have modification to remove cross talk
   if(fDetType == BatCalibTypes::kDualEndcapDetType)
   {
      fRRQList["qbottom"] = qoa*fIOMan.Get("QOOFnoXvolts");
      fRRQList["qtop"] = qia*fIOMan.Get("QIOFnoXvolts");

      fRRQList["qbottom0"] = qoa*fIOMan.Get("QOOFnoXvolts0");
      fRRQList["qtop0"] = qia*fIOMan.Get("QIOFnoXvolts0");

      return;
   }


   // ===== Now do the other types of zips =====

   double qi = -999999;
   double qo = -999999;
   double qi0 = -999999;
   double qo0 = -999999;

   // === Special treatment for ST1z3 (R130/131) - for disconnected qouter  ===

   // FIXME, eventually we may need a smarter (time sensitive) algorithm to pick this out!
   // at the moment we don't have F5 w/o x-talk for this detector, so don't check for sat

   if((fDetType == BatCalibTypes::kmZIPDetType && fDetNum == 3) || (fDetType == BatCalibTypes::kDualEndcapDetType))
   {
      qo = 0.0;
      qi = qia*fIOMan.Get("QIOFnoXvolts");
      
      qo0 = 0.0;
      qi0 = qia*fIOMan.Get("QIOFnoXvolts0");

      // ===== Store the values and return here =====

      fRRQList["qi"] = qi;
      fRRQList["qo"] = qo;

      fRRQList["qi0"] = qi0;
      fRRQList["qo0"] = qo0;

      return;
   }


   //just do simple cross talk calc for qi0 and qo0
   qo0 = qoa*(fIOMan.Get("QOOFvolts0") + qox*fIOMan.Get("QIOFvolts0"));
   qi0 = qia*(fIOMan.Get("QIOFvolts0") + qix*fIOMan.Get("QOOFvolts0"));


   //check for saturation (either QI or QO)  
   int isSat = ( (fIOMan.Get("QIsat")> 0 || fIOMan.Get("QOsat")>0) ? 1 : 0);


   //Note: the overall scale factor for unsaturated Ge events is applied by the position correction

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

	 //QI for Ge CDMSII detectors applies amplitude in position correction
	 qi = (fIOMan.Get("QIOFvolts") + qix*fIOMan.Get("QOOFvolts"));


	 // ===== Apply charge position correction (only Ge CDMS II) =====
      
	 double xPosFactor = 356.;
	 double yPosFactor = 356.;
	 double xdel = fRRQList["xdel"];
	 double ydel = fRRQList["ydel"];

	 //short cut to using pow(), which is slow
	 double xdel2 = xdel*xdel;
	 double xdel3 = xdel*xdel2;
	 double xdel4 = xdel*xdel3;
	 double xdel5 = xdel*xdel4;

	 double ydel2 = ydel*ydel;
	 double ydel3 = ydel*ydel2;
	 double ydel4 = ydel*ydel3;
	 double ydel5 = ydel*ydel4;
	 
	 // --- compute the x correction factor ---

	 if( xdel <= fXFitMax && xdel >= fXFitMin)
	 {
//	    cout <<"In x range!" << endl;

	    xPosFactor /= (fChargePosX[0]*xdel5 +  fChargePosX[1]*xdel4 + fChargePosX[2]*xdel3 + 
			   fChargePosX[3]*xdel2 + fChargePosX[4]*xdel + fChargePosX[5]);
	 }
	 else 
	 {
	    double val = ( xdel < fXFitMin ? fXFitMin : fXFitMax);
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
	 if( ydel <= fYFitMax && ydel >= fYFitMin)
	 {
//	    cout <<"In y range!" << endl;

	    yPosFactor /= (fChargePosY[0]*ydel5 +  fChargePosY[1]*ydel4 + fChargePosY[2]*ydel3 + 
			   fChargePosY[3]*ydel2 + fChargePosY[4]*ydel + fChargePosY[5]); 
	 }
	 else
	 {
	    double val = ( ydel < fYFitMin ? fYFitMin : fYFitMax );
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

      } //end if Ge CDMSII

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
	 cout <<"WARNING from GenRRQDataCDMSII::ApplyChargeCalibration!  There are no F5 rq's for calculating saturated charge energy, reverting to null value for energy!"
	      << endl;
      }

   }


   // ===== Store the values now =====

   fRRQList["qi"] = qi;
   fRRQList["qo"] = qo;

   fRRQList["qi0"] = qi0;
   fRRQList["qo0"] = qo0;

   return;
}

void GenRRQDataCDMSII::CalcTotalEnergiesAndYields()
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


   // ===== First do endcaps =====
   if(fDetType == BatCalibTypes::kDualEndcapDetType)
   {
      // === total phonon energy ===
      
      double pttop = fRRQList["pc"] + fRRQList["pd"];
      double pttop_int = fRRQList["pc_int"] + fRRQList["pd_int"];

      double ptbottom = fRRQList["pa"] + fRRQList["pb"];
      double ptbottom_int = fRRQList["pa_int"] + fRRQList["pb_int"];
      
      fRRQList["pttop"] = pttop;
      fRRQList["pttop_int"] = pttop_int;

      fRRQList["ptbottom"] = ptbottom;
      fRRQList["ptbottom_int"] = ptbottom_int;
      
      // === recoil energy ===
      
      //should always be positive?
      double qtopfac = fabs(qiBias/fEpsilon); 
      double qbottomfac = fabs(qoBias/fEpsilon); 
      
      fRRQList["prtop"] = pttop - (fRRQList["qtop"]*qtopfac);
      fRRQList["prtop_int"] = pttop_int - (fRRQList["qtop"]*qtopfac);

      fRRQList["prbottom"] = ptbottom - (fRRQList["qbottom"]*qbottomfac);
      fRRQList["prbottom_int"] = ptbottom_int - (fRRQList["qbottom"]*qbottomfac);
      
      // === yield ===
      
      fRRQList["ytop"] = fRRQList["qtop"]/fRRQList["prtop"];
      fRRQList["ytop_int"] = fRRQList["qtop"]/fRRQList["prtop_int"];

      fRRQList["ybottom"] = fRRQList["qbottom"]/fRRQList["prbottom"];
      fRRQList["ybottom_int"] = fRRQList["qbottom"]/fRRQList["prbottom_int"];
      
   } //endif endcap


   // ===== Now take care of all other types of zips =====   

   else
   {

      // === total ionization energy ===

      fRRQList["qsum"] = fRRQList["qi"] + fRRQList["qo"];

      // === total phonon energy ===
      
      double pt = fRRQList["pa"] + fRRQList["pb"] + fRRQList["pc"] + fRRQList["pd"];
      fRRQList["pt"] = pt;
      
      // === recoil energies ===
      
      double qifac = fabs(qiBias/fEpsilon); //should always be positive?
      
      fRRQList["prg"] = pt/(1.0+qifac); 
      fRRQList["pr"] = pt - (fRRQList["qsum"]*qifac);
      fRRQList["pri"] = pt - (fRRQList["qi"]*qifac); 
      
      // === yields ===
      
      fRRQList["y"] = fRRQList["qsum"]/fRRQList["pr"];
      fRRQList["yg"] = fRRQList["qsum"]/fRRQList["prg"];
      fRRQList["yi"] = fRRQList["qi"]/fRRQList["pri"];
      fRRQList["ygi"] = fRRQList["qi"]/fRRQList["prg"];
      
      // === integral quantities ===
      
      if( !fIsFirstProduction )
      {
	 double pt_int = fRRQList["pa_int"] + fRRQList["pb_int"] + fRRQList["pc_int"] + fRRQList["pd_int"];
	 fRRQList["pt_int"] = pt_int;
	 
	 fRRQList["prg_int"] = pt_int/(1.0+qifac); 
	 fRRQList["pr_int"] = pt_int - (fRRQList["qsum"]*qifac);
	 fRRQList["pri_int"] = pt_int - (fRRQList["qi"]*qifac); 
	 
	 fRRQList["y_int"] = fRRQList["qsum"]/fRRQList["pr_int"];
	 fRRQList["yg_int"] = fRRQList["qsum"]/fRRQList["prg_int"];
	 fRRQList["yi_int"] = fRRQList["qi"]/fRRQList["pri_int"];
	 fRRQList["ygi_int"] = fRRQList["qi"]/fRRQList["prg_int"];
      }
      
   } //endif not endcap
   
   return;
}

//mostly just for convenience
void GenRRQDataCDMSII::CalcPartitions()
{

   // --- For all except endcap and mercedes ---

   if(fDetType != BatCalibTypes::kDualEndcapDetType )
   {
      fRRQList["qpart"] = (fRRQList["qi"] - fRRQList["qo"])/fRRQList["qsum"];
      
      //partition for CDMSII style detectors (box plot)
      if(fDetType != BatCalibTypes::kmZIPDetType)
      {
	 fRRQList["xppart"] = (fRRQList["pc"] + fRRQList["pd"] - fRRQList["pa"] - fRRQList["pb"]) /
	    ( fRRQList["pa"] + fRRQList["pb"] + fRRQList["pc"] + fRRQList["pd"] );

	 fRRQList["yppart"] = (fRRQList["pa"] + fRRQList["pd"] - fRRQList["pb"] - fRRQList["pc"]) /
	    ( fRRQList["pa"] + fRRQList["pb"] + fRRQList["pc"] + fRRQList["pd"] );

	 fRRQList["rppart"] = sqrt(fRRQList["xppart"]*fRRQList["xppart"] + fRRQList["yppart"]*fRRQList["yppart"]);
      }
   }     

   // --- for endcap ----

   if(fDetType == BatCalibTypes::kDualEndcapDetType)
   {
      fRRQList["pparttop"] = (fRRQList["pd"] - fRRQList["pc"])/(fRRQList["pd"] + fRRQList["pc"]);    //PC and PD belong to the top endcap
      fRRQList["ppartbottom"] = (fRRQList["pb"] - fRRQList["pa"])/(fRRQList["pb"] + fRRQList["pa"]); //PA and PB belong to the bottom endcap
   }     

   // --- only for mercedes ---

   if(fDetType == BatCalibTypes::kmZIPDetType)
   {
      //partition for mercedes (triangle plots)
      fRRQList["xppart"] = (fRRQList["pb"]*cos(kThetaVect[0]) + fRRQList["pc"]*cos(kThetaVect[1]) + fRRQList["pd"]*cos(kThetaVect[2])) 
	                   / (fRRQList["pb"] + fRRQList["pc"] + fRRQList["pd"]);

      fRRQList["yppart"] = (fRRQList["pb"]*sin(kThetaVect[0]) + fRRQList["pc"]*sin(kThetaVect[1]) + fRRQList["pd"]*sin(kThetaVect[2])) 
	                   / (fRRQList["pb"] + fRRQList["pc"] + fRRQList["pd"]);

      fRRQList["rppart"] = sqrt(fRRQList["xppart"]*fRRQList["xppart"] + fRRQList["yppart"]*fRRQList["yppart"]);

      //inner/outer partition
      fRRQList["ppart_io"] = (fRRQList["pa"] - (fRRQList["pb"] + fRRQList["pc"] + fRRQList["pd"]))/fRRQList["pt"];
      fRRQList["ppart_io_int"] = (fRRQList["pa_int"] - (fRRQList["pb_int"] + fRRQList["pc_int"] + fRRQList["pd_int"]))/fRRQList["pt_int"];
   }

   return;
}
     
//calibrate the OF resolution quantities
void GenRRQDataCDMSII::CalcOFResolutions()
{

   // === calibrate the OFdelay and OFamplitude resolutions ===
   //      note values stored in order QI, QO, PA, PB, ...

   double tempQIMax = fOFMaxTemplate[0];
   double tempQOMax = fOFMaxTemplate[1];

   if(fDetType == BatCalibTypes::kDualEndcapDetType)
   {
      fRRQList["qtopdelayres"] = fChargeCal[0]*tempQIMax*fDelaySig[0];
      fRRQList["qbottomdelayres"] = fChargeCal[1]*tempQOMax*fDelaySig[1];
      
      fRRQList["qtopampres"] = fChargeCal[0]*tempQIMax*fAmpSig[0];
      fRRQList["qbottomampres"] = fChargeCal[1]*tempQOMax*fAmpSig[1];
   }
   else
   {
      fRRQList["qidelayres"] = fChargeCal[0]*tempQIMax*fDelaySig[0];
      fRRQList["qodelayres"] = fChargeCal[1]*tempQOMax*fDelaySig[1];
      
      fRRQList["qiampres"] = fChargeCal[0]*tempQIMax*fAmpSig[0];
      fRRQList["qoampres"] = fChargeCal[1]*tempQOMax*fAmpSig[1];
   }

   //leaving this unchanged for endcaps until we have a naming convention for endcap phonon channels

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

//FIXME - should we fix the fact that internal primary chan number and stored are offset by 1
//Note that the user can make a choice in whether constant frequency or variable frequency rtftwalk values are used for the delay calculations in this function.   
//This is done by setting the flag in the user settings file "DefaultToConstFreqRTFTWalk" 
void GenRRQDataCDMSII::FindPrimaryPhononChannel()
{

   //Check whether algorithm is defined for this detector type

   if(fDetType != BatCalibTypes::kZIPDetType && fDetType != BatCalibTypes::kmZIPDetType)    
   {
      cout <<"GenRRQDataCDMSII::FindPrimaryPhononChannel ERROR!\n"  
	   <<"Detector type does not have a defined primary phonon channel calculation!"
	   << endl;

      exit(1);
   }


   const string kPhononEnergyNames[4] = {"pa", "pb", "pc", "pd"};


   // --- For CDMSII zips, primary is the channel with the biggest amplitude ---

   if(fDetType == BatCalibTypes::kZIPDetType)
   {
      double maxAmp = -1.0*numeric_limits<double>::infinity();

      //loop over channels to find minimum delay
      for(int chanItr = 0; chanItr < BatCalibTypes::kZIPFLIPNPhononChan; chanItr++)
      {
	 string tempVarName = kPhononEnergyNames[chanItr];
	 double tempAmp =  fRRQList[tempVarName];

	 if(tempAmp > maxAmp)
	 {
	    maxAmp = tempAmp;
	    fPrimaryPhononChan = chanItr;
	 }

      } //end loop over phonon channels

   } //end if CDMSII style of zip


   // --- For mercedes, primary is the biggest if its one of the inner channels, ---
   // --- if the outer channel has the biggest amplitude, then take the largest inner channel if its delay is faster ---

   if(fDetType == BatCalibTypes::kmZIPDetType)
   {

      // 1.  First look for primary by the amplitude

      double maxAmp = -1.0*numeric_limits<double>::infinity();

      //loop over channels to find max amplitude
      for(int chanItr = 0; chanItr < BatCalibTypes::kZIPFLIPNPhononChan; chanItr++)
      {
	 string tempVarName = kPhononEnergyNames[chanItr];
	 double tempAmp =  fRRQList[tempVarName];

	 if(tempAmp > maxAmp)
	 {

	    //save new value in first place
	    maxAmp = tempAmp;
	    fPrimaryPhononChan = chanItr;
	 }
	 
      } //end loop over phonon channels


      // 2. loop over inner channels to find max amplitude - needed for pdel_io and primary by delay calc
      double secondaryAmp = -1.0*numeric_limits<double>::infinity(); //second to biggest amplitude
      
      for(int chanItr = 1; chanItr < BatCalibTypes::kZIPFLIPNPhononChan; chanItr++)
      {
	 string tempVarName = kPhononEnergyNames[chanItr];
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
	 
	 if( !fUserData.DoZipAlgorithm(fDetNum, "DefaultToConstFreqRTFTWalk"))
	 {
	    primaryRT = fIOMan.Get(BatCalibTypes::kZIPFLIPPhononChan[fPrimaryPhononChan] + "VWKr20"); 	 
	    secondaryRT = fIOMan.Get(BatCalibTypes::kZIPFLIPPhononChan[fPrimaryInnerPhononChan] + "VWKr20"); 
	 }
	 else
	 {
	    primaryRT = fIOMan.Get(BatCalibTypes::kZIPFLIPPhononChan[fPrimaryPhononChan] + "WKr20"); 	 
	    secondaryRT = fIOMan.Get(BatCalibTypes::kZIPFLIPPhononChan[fPrimaryInnerPhononChan] + "WKr20"); 
	 }

	 //make the inner channel primary if it has a faster risetime
	 if(secondaryRT < primaryRT)
	 {
	    fPrimaryPhononChan = fPrimaryInnerPhononChan;
	 }

      }//end if max amplitude is outer


   }//end if mercedes style zip


   return;
}

void GenRRQDataCDMSII::CalcVarFreqRTFTWalkRRQ()
{
//   cout <<"Hello from CalcVarFreqRTFTWalkRRQ!" << endl;

   if(fPrimaryPhononChan == -999999)
   {
      cout <<"ERROR GenRRQDataCDMSII::CalcVarFreqRTFTWalkRRQ! Primary channel needs to be set first, please check code!" 
	   << endl;
      exit(1);
   }

   if(fDetType != BatCalibTypes::kZIPDetType && fDetType != BatCalibTypes::kmZIPDetType)    
   {
      cout <<"GenRRQDataCDMSII::CalcVarFreqRTFTWalkRRQ ERROR!\n"  
	   <<"Detector type does not have a set of RTFTWalk calculations!"
	   << endl;

      exit(1);
   }

   // --- primary channel rrq's  ---
   
   string primChanName = BatCalibTypes::kZIPFLIPPhononChan[fPrimaryPhononChan];

   fRRQList["pdel"] = fIOMan.Get(primChanName+"VWKr20")*1e6 - (511.5*0.8 + fIOMan.Get("QSOFdelay")*1e6);

   fRRQList["pminrt"] = (fIOMan.Get(primChanName+"VWKr40") - fIOMan.Get(primChanName+"VWKr10"))*1e6; //in microseconds

   fRRQList["pminrt4070"] = (fIOMan.Get(primChanName+"VWKr70") - fIOMan.Get(primChanName+"VWKr40"))*1e6; //in microseconds

   fRRQList["pminrt1030"] = (fIOMan.Get(primChanName+"VWKr30") - fIOMan.Get(primChanName+"VWKr10"))*1e6; //in microseconds

   fRRQList["pminrt3050"] = (fIOMan.Get(primChanName+"VWKr50") - fIOMan.Get(primChanName+"VWKr30"))*1e6; //in microseconds

   fRRQList["pminrt5080"] = (fIOMan.Get(primChanName+"VWKr80") - fIOMan.Get(primChanName+"VWKr50"))*1e6; //in microseconds

   fRRQList["ptopwidth"] = (fIOMan.Get(primChanName+"VWKf80") - fIOMan.Get(primChanName+"VWKr80"))*1e6; //in microseconds


   // --- total phonon pulse rrq's (only for mercedes) ---
   
   if(fDetType == BatCalibTypes::kmZIPDetType)
   {
      //all in microseconds
      
      fRRQList["ptdel"] = fIOMan.Get("PTVWKr20")*1e6 - (511.5*0.8 + fIOMan.Get("QSOFdelay")*1e6);
      fRRQList["ptrt"] = (fIOMan.Get("PTVWKr40") - fIOMan.Get("PTVWKr10"))*1e6; 

      //find the primary inner channel by amplitude
      fRRQList["pdel_io"] = (fIOMan.Get(BatCalibTypes::kZIPFLIPPhononChan[fPrimaryInnerPhononChan] + "VWKr20") 
			     - fIOMan.Get("PAVWKr20"))*1e6;

   }

   return;
}

     
void GenRRQDataCDMSII::CalcConstFreqRTFTWalkRRQ()
{
//   cout <<"Hello from CalcConstFreqRTFTWalkRRQ!" << endl;

   if(fPrimaryPhononChan == -999999)
   {
      cout <<"ERROR GenRRQDataCDMSII::CalcConstFreqRTFTWalkRRQ! Primary channel needs to be set first, please check code!" 
	   << endl;
      exit(1);
   }

   if(fDetType != BatCalibTypes::kZIPDetType && fDetType != BatCalibTypes::kmZIPDetType)    
   {
      cout <<"GenRRQDataCDMSII::CalcConstFreqRTFTWalkRRQ ERROR!\n"  
	   <<"Detector type does not have a set of ConstFreqRTFTWalk calculations!"
	   << endl;

      exit(1);
   }

   // --- primary channel rrq's  ---
   
   string primChanName = BatCalibTypes::kZIPFLIPPhononChan[fPrimaryPhononChan];

   fRRQList["pdelCF"] = fIOMan.Get(primChanName+"WKr20")*1e6 - (511.5*0.8 + fIOMan.Get("QSOFdelay")*1e6);
   fRRQList["pminrtCF"] = (fIOMan.Get(primChanName+"WKr40") - fIOMan.Get(primChanName+"WKr10"))*1e6; //in microseconds
   fRRQList["pminrtCF4070"] = (fIOMan.Get(primChanName+"WKr70") - fIOMan.Get(primChanName+"WKr40"))*1e6; //in microseconds

   if( !fIsFirstProduction )
   {
      fRRQList["pminrtCF1030"] = (fIOMan.Get(primChanName+"WKr30") - fIOMan.Get(primChanName+"WKr10"))*1e6; //in microseconds
      fRRQList["pminrtCF3050"] = (fIOMan.Get(primChanName+"WKr50") - fIOMan.Get(primChanName+"WKr30"))*1e6; //in microseconds
      fRRQList["pminrtCF5080"] = (fIOMan.Get(primChanName+"WKr80") - fIOMan.Get(primChanName+"WKr50"))*1e6; //in microseconds
      fRRQList["ptopwidthCF"] = (fIOMan.Get(primChanName+"WKf80") - fIOMan.Get(primChanName+"WKr80"))*1e6; //in microseconds
   }


   // --- total phonon pulse rrq's (only for mercedes) ---
   
   if(fDetType == BatCalibTypes::kmZIPDetType)
   {
      fRRQList["ptdelCF"] = fIOMan.Get("PTWKr20")*1e6 - (511.5*0.8 + fIOMan.Get("QSOFdelay")*1e6);
      fRRQList["ptrtCF"] = (fIOMan.Get("PTWKr40") - fIOMan.Get("PTWKr10"))*1e6; //in microseconds

      //find the primary inner channel by amplitude
      fRRQList["pdel_ioCF"] = (fIOMan.Get(BatCalibTypes::kZIPFLIPPhononChan[fPrimaryInnerPhononChan] + "WKr20") 
			     - fIOMan.Get("PAWKr20"))*1e6;
   }

   return;
}

     
void GenRRQDataCDMSII::CalcPipeFitRRQ()
{

   if(fPrimaryPhononChan == -999999)
   {
      cout <<"ERROR GenRRQDataCDMSII::CalcPipeFitRRQ! Primary channel needs to be set first, please check code!" 
	   << endl;
      exit(1);
   }

   if(fDetType != BatCalibTypes::kZIPDetType && fDetType != BatCalibTypes::kmZIPDetType)    
     {
       cout <<"GenRRQDataCDMSII::CalcPipeFitRRQ ERROR!\n"  
	    <<"Detector type does not have a set of PipFitRRQ calculations!"
	    << endl;
       
       exit(1);
     }
   

   // --- primary channel rrq's  ---
   
   string primChanName = BatCalibTypes::kZIPFLIPPhononChan[fPrimaryPhononChan];
   //make sure fit was done for this channel

   if (fIOMan.Get(primChanName+"PFrfeflag") == 0){
     
     fRRQList["prtPF020"] = (fIOMan.Get(primChanName+"PFr20") - fIOMan.Get(primChanName+"PFr0"))*0.8;
     fRRQList["prtPF1040"] = (fIOMan.Get(primChanName+"PFr40") - fIOMan.Get(primChanName+"PFr10"))*0.8;
     fRRQList["pdelPF0"] =  fIOMan.Get(primChanName+"PFr0")*0.8 - (511.5*0.8 + fIOMan.Get("QSOFdelay")*1e6);
     fRRQList["pdelPF10"] =  fIOMan.Get(primChanName+"PFr10")*0.8 - (511.5*0.8 + fIOMan.Get("QSOFdelay")*1e6);
     fRRQList["pdelPF20"] =  fIOMan.Get(primChanName+"PFr20")*0.8 - (511.5*0.8 + fIOMan.Get("QSOFdelay")*1e6);
   }

   //pcurv20 calculation,  a little more involved so defining some variables
   if (fIOMan.Get(primChanName+"PFrfeflag") == 0){
    
     double rt020 = fRRQList["prtPF020"];
     double rt1040 = fRRQList["prtPF1040"];
     double r0 = fIOMan.Get(primChanName+"PFr0");
     double t0 = fIOMan.Get(primChanName+"PFt0fit");
     double kappa = fIOMan.Get(primChanName+"PFkappa");
     double tau = fIOMan.Get(primChanName+"PFtau");
     double a1 = fIOMan.Get(primChanName+"PFa1");
     double a0 = fIOMan.Get(primChanName+"PFa0");
     if (kappa !=0 && tau != 0) {
       double t20p = r0 - t0 + rt020/0.8;
       double et20pk = exp(-t20p/kappa);
       double et20pt = exp(-t20p/tau);
       double uc1 = et20pk/(kappa*kappa);
       double uc2 = -exp(-t20p*(1.0/kappa+1.0/tau))*((1.0/kappa+1.0/tau)*(1.0/kappa+1.0/tau));
       double uc3 = -et20pt*a1/(tau*tau);
       double uc4 = exp(-2*t20p/tau)*4.0*a1/(tau*tau);
       double uncurv20 = a0*(uc1+uc2+uc3+uc4);
       
       double ppeak = 5.0*a0*(1-et20pt)*(et20pk-a1*et20pt);
       double pcurv20pn = uncurv20/ppeak;
       fRRQList["pcurv20"] = pcurv20pn*rt1040*rt1040;  
     }



   }
   return;
}

     
void GenRRQDataCDMSII::CalcWedgeFitRRQ()
{

   if(fPrimaryPhononChan == -999999)
   {
      cout <<"ERROR GenRRQDataCDMSII::CalcWedgeFitRRQ! Primary channel needs to be set first, please check code!" 
	   << endl;
      exit(1);
   }

   return;
}




